/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2015-1-5
Version:  3.1
Description: 出钢计划炉次CAST号全部重算
**************************************************/
#include "stdafx.h"




  //处理号维护表, CAST记录

/*<remark>=========================================================
/// <summary>
/// 出钢计划炉次CAST号全部重算
/// <para>根据出钢计划的浇铸顺，对生成各炉次的CAST号。</para>
/// <para>读取各连铸机下的当前浇铸的CAST号；读取出钢计划中各连铸机下最大的CAST号；
///  确定新增计划的基准CAST后,按浇铸顺,生成CAST号.
/// </para>
/// <para>数据库表：TPSSM11(炼钢出钢计划主表)</para>
/// <para>主调用函数：pssm11_opti</para>
/// </summary>
/// <param name="sm_unit_no"></param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm11t_cast_recalc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int   blkseq=0;

	int k= 0;
	CDecimal	   dummy;
	CString     cc_mach_no = "";
	CString     cast_no = "";             /* 计算CAST号用 */
	CDecimal    cast_div_no = 0;          /* CAST分割号 */
	CDecimal    cc_seq;                   /* 浇铸顺序号 */
	CDecimal    cast_stream_no;           /* cast流水号 */

	CString dateNow14 = "";

	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm26("TPSSM26");
	
	CString sqlstr;

	CDbCommand cmd_tpssm26_inq(conn); 
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);

	try
	{
		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");

		tpssm26["FACTORY_DIV"] = bcls_rec->Tables["PONO"].Rows[0]["FACTORY_DIV"].ToString().Trim();
		////Log::Trace("", __FUNCTION__, "--- tpssm26["FACTORY_DIV"] = [{0}] ---", tpssm26["FACTORY_DIV"].ToString());

		//----------------------------------------------------------------------------------
		//获得输入参数
		//1.读取编入计划的炼钢单元号
		//blkseq = bcls_rec->Tables.IndexOf("PLAN"); //浇铸信息
		//if (blkseq < 0) 
		//{
		//	sprintf(s.msg, "CAST号生成逻辑中，没有找到计划的浇铸信息数据块[PLAN]，请联系系统维护人员。");
		//	sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm11_cast_create().");
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		tpssm26["REC_CREATOR"] = s.userid;
		tpssm26["REC_CREATE_TIME"] = dateNow14;
		
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = " SELECT STATION_NO,DEV_CODE FROM TPSSMD1 \
					   WHERE FACTORY_DIV = @tpssm26.FACTORY_DIV \
						 AND AREA_ID = 5 \
						ORDER BY STATION_NO ASC ";
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssm26.FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while (cmd_tpssmd1_inq.Read())
		{
			tpssm26["CC_MACH_NO"] = cmd_tpssmd1_inq.GetString(1).Trim();
			tpssm26["DEV_CODE"] = cmd_tpssmd1_inq.GetString(2).Trim();
			dummy = 0;
			dummy = tpssm26.QueryCount("CC_MACH_NO,FACTORY_DIV,DEV_CODE");
			if (dummy == 0)
			{
				//扩位：年末（1）+铸机号（1）+ 4位流水
				tpssm26["CAST_NO"] = (const char*)dateNow14.Trim().SubstringNE(3, 1) + tpssm26["CC_MACH_NO"].ToString() + "0000";
				tpssm26["CAST_DIV_NO"] = 1;//从第1开始			
				////Log::Info("", __FUNCTION__, "CAST_NO=[{0}]", (const char*)tpssm26["CAST_NO"].ToString());
				tpssm26.Insert();
			}
		}
		cmd_tpssmd1_inq.Close();

		////Log::Trace("", __FUNCTION__, "循环读取tpssm26表中的计划编制数据，以计算CAST号!");
		//循环读取tpssm26表中的计划编制数据，以计算CAST号
		//数据必须是连铸而非模铸的，模铸CAST号在插入模铸的程序中单独计算 HYF20130606
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				"  SELECT * FROM TPSSM26 "
				"  WHERE FACTORY_DIV = @tpssm26.FACTORY_DIV "
				"  ORDER BY DEV_CODE ASC"
				);
			break;
		}

		cmd_tpssm26_inq.SetCommandText(sqlstr);
		cmd_tpssm26_inq.Parameters.Set("tpssm26.FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());  //C-连铸
		cmd_tpssm26_inq.ExecuteReader();
		while ( cmd_tpssm26_inq.Read() )
		{
			cmd_tpssm26_inq.Fetch(tpssm26);
			tpssm26.TrimOrBlank();
			
			////Log::Trace("", __FUNCTION__, "起始cast_no=[{0}], cast_div_no=[{1}]", (const char*)tpssm26["CAST_NO"].ToString(), tpssm26["CAST_DIV_NO"].ToDecimal().ToInt32());

			cast_no = tpssm26["CAST_NO"].ToString().Trim();
			cast_div_no = tpssm26["CAST_DIV_NO"];
			cc_mach_no = tpssm26["DEV_CODE"].ToString().SubstringNE(1);
			
			//------------------------------------
			//基于确定的 cast_no 进行后续新增炉次的CAST计算
			//1）判断从TPSSM26表中读取的cast_no是否为空，空则表示是年度第一个计划
			if (cast_no == "")
			{

				cast_no = cc_mach_no + "0000";
				cast_div_no = 0; //后续炉次 + 1 计算

			}//if (cast_no == "")

			////Log::Trace("", __FUNCTION__, "起始cast_no=[{0}], cast_div_no=[{1}]", (const char*)cast_no, cast_div_no.ToInt32() );

			tpssm10["FACTORY_DIV"] = tpssm26["FACTORY_DIV"];
			//------------------------------------
			//顺序读取连铸预计划各炉次，进行CAST计算
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT PONO, RESTRAND_FLG, CC_SEQ, TD_CHG_FLG "
					"   FROM TPSSM10 "
					"  WHERE CC_MACH_NO  = @cc_mach_no " //指定连铸机
					"    AND FACTORY_DIV  = @tpssm10.FACTORY_DIV "
					"    AND PONO_STATUS < 83 "  //82-开浇  必须从开浇炉计算
					"    AND PONO_STATUS > 16 "  //16-命令接收
					" ORDER BY CC_SEQ ASC "
					);
				break;
			}
			cmd_tpssm10_inq.SetCommandText( sqlstr );
			cmd_tpssm10_inq.Parameters.Set("cc_mach_no", cc_mach_no); 
			cmd_tpssm10_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
			cmd_tpssm10_inq.ExecuteReader();
			while (cmd_tpssm10_inq.Read())
			{
				tpssm10["PONO"] = cmd_tpssm10_inq.GetString(1);
				tpssm10["RESTRAND_FLG"] = cmd_tpssm10_inq.GetString(2);
				tpssm10["CC_SEQ"] = cmd_tpssm10_inq.GetDecimal(3).ToInt32();
				tpssm10["TD_CHG_FLG"] = cmd_tpssm10_inq.GetDecimal(4); //中间包更换标志
				//tpssm10.INS_FE_FLAG = cmd_tpssm10_inq.GetString(5).Trim(); //插铁板标记

				////Log::Trace("", __FUNCTION__, "获取PONO=[{0}], CC_SEQ=[{1}], RESTRAND_FLG=[{2}], TD_CHG_FLG=[{3}]",
				//	tpssm10["PONO"].ToString(), tpssm10["CC_SEQ"].ToDecimal(), tpssm10["RESTRAND_FLG"].ToString(), tpssm10["TD_CHG_FLG"].ToDecimal());

				if (tpssm10["PONO"].ToString() == "") continue;
				
				tpssm11["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssm11["PONO"] = tpssm10["PONO"];
				sqlstr = "tpssm11.Query(PONO)";
				bool has11 = tpssm11.Query("PONO, FACTORY_DIV");

				//若计划中无该PONO，跳过（浇铸顺做过调整，PONO没有编入计划）  2015-12-01 增加
				if (has11 == false) continue;


				//校验开浇第一炉次的CAST号是否记录的一致（状态回退的要修改）
				if (tpssm11["PONO_STATUS"].ToDecimal().ToInt32() == 82) //82-开浇
				{
					if (tpssm10["CC_SEQ"].ToDecimal().ToInt32() == 1 &&
						(tpssm11["CAST_NO"].ToString().Trim() != tpssm26["CAST_NO"].ToString().Trim() ||
						tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32() != tpssm26["CAST_DIV_NO"].ToDecimal().ToInt32())
						)
					{
						CString cast_11 = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();
						CString cast_25 = tpssm26["CAST_NO"].ToString().Trim() + "-" + tpssm26["CAST_DIV_NO"].ToDecimal().ToString();
						CFormattable arguments[] = { tpssm10["PONO"].ToString(), cast_11, cast_25 }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "开浇炉[{0}]的CAST号[{1}]与当前记录的CAST号[{2}]不一致，请修正后继续。", arguments, 3); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//对已开浇的炉次，跳过后续计算
					continue;
				}


				//当出钢计划(TPSSM11)的重引锭标记与浇铸顺(TPSSM10)的不同时，浇铸准备时间有不同
				if (tpssm10["RESTRAND_FLG"].ToString() != tpssm11["RESTRAND_FLG"].ToString().Trim() ||
					tpssm10["TD_CHG_FLG"]   != tpssm11["TD_CHG_FLG"].ToDecimal()
					)
				{
					tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
					tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm12["AREA_ID"] = 5;  //连铸工序
					tpssm12["PREP_TIME"] = tpssm10["CC_PREP_TIME"].ToDecimal().ToInt32();   //炉间准备时间
					tpssm12["PROC_TIME"] = tpssm10["POUR_TIME"].ToDecimal().ToInt32();      //浇铸时间, 4舍5入
					//tpssm12["PRE_PROC_TIME"] = tpssmc1.TT_PREP_2CH;        //连铸前处理时间，包到提前时刻，不用改，没变
					sqlstr = "tpssm12.Update()";
					tpssm12.Update(
						" PREP_TIME"       //准备时间
						",PRE_PROC_TIME"   //前处理时间
						",PROC_TIME",      //处理时间
						"FACTORY_DIV, SM_PLAN_NO, AREA_ID");
				}

				tpssm11["RESTRAND_FLG"] = tpssm10["RESTRAND_FLG"];
				tpssm11["TD_CHG_FLG"]   = tpssm10["TD_CHG_FLG"];

				if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T") //重引锭
				{					
					cast_stream_no = cast_stream_no.Parse(cast_no.Substring(2));//流水号
					cast_stream_no = cast_stream_no + 1;
					//sprintf( cast_no, "%c%.4d", tpssm26.cc_mach_no[0], cast_stream_no);//连连浇号--根据连铸机号的代码生成
					cast_no = cast_no.Format("%s%.4d", (const char*)cast_no.SubstringNE(0, 2), cast_stream_no.ToInt32());
					cast_div_no = 1;

				}
				else //不重引锭--连浇, cast_no不变
				{
					cast_div_no = cast_div_no + 1;
				}

				//~~~~~将cast_no, cast_div_no写入记录~~~~~
				////Log::Trace("", __FUNCTION__, "cast_no=[{0}], cast_div_no=[{1}]", (const char*)cast_no, cast_div_no.ToInt32() );

				tpssm11["CC_MACH_NO"] = cc_mach_no;
				tpssm11["CAST_NO"] = cast_no;
				tpssm11["CAST_DIV_NO"] = cast_div_no;
				if (tpssm10["CC_REQ_TIME_FLAG"].ToString().Trim() == "1") //人工指定时
				{
					tpssm11["CC_REQ_TIME"] = tpssm10["CC_REQ_TIME"];
				}
				else if(tpssm10["CC_REQ_TIME_FLAG"].ToString().Trim() == "3") //已开浇
				{
					//保留原记录
					tpssm11["CC_REQ_TIME"] = tpssm11["CC_REQ_TIME"];
				}
				else //否则置空
				{
					tpssm11["CC_REQ_TIME"] = " ";
				}

				sqlstr = "tpssm11.Update()";
				tpssm11.Update(
					"CC_MACH_NO,"
					"RESTRAND_FLG,"
					"TD_CHG_FLG,"   //中间包更换标志
					"INS_FE_FLAG,"   //插铁板标记
					"CC_REQ_TIME,"   //同步TPSSM10表结果
					"CAST_NO,"
					"CAST_DIV_NO",
					"FACTORY_DIV, SM_PLAN_NO");

				////修改TPSSM12表的连铸设备 2015-9-23 增加
				//tpssm12["DEV_CODE"] = "C" + cc_mach_no;  //设备代码
				//tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				//tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				//tpssm12["AREA_ID"] = 5;  //连铸工序
				//sqlstr = "tpssm12.Update()";
				//tpssm12.Update("DEV_CODE", "FACTORY_DIV, SM_PLAN_NO,AREA_ID");
			}
			cmd_tpssm10_inq.Close();
		
		}
		cmd_tpssm26_inq.Close();

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
