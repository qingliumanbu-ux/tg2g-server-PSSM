/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2012-01-16
Version:1.0
Description: 钢水返送
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
int f_plan_delete_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 钢水返送
/// <para>钢水返送                            </para>
/// <para>数据库表：tpssm35                    </para>
/// <para>主调用函数：PSSM18R画面调用。                </para>
/// </summary>
/// <param name="factory_div">炼钢厂别区分代码     </param>
/// <returns></returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18b_rt_upd)
//-EP_SYSTEM_HEAD_END
int f_pssm18b_rt_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0; 
	int dummy = 0;
	int ret = 0;
	CModel tpssm35("TPSSM35");
	CModel tpssm11("TPSSM11");
	CModel tpssm13("TPSSM13");
	CModel tpssm10("TPSSM10");
	CModel tpssm01("TPSSM01");
	CModel tpssm12("TPSSM12");
	CModel tpssm99("TPSSM99");
	CModel tpssmd1("TPSSMD1");
	CString sqlstr = "";
	CString heat_no				= "";
	CString factory_div	= "";
	CString plan_no = "";
	//当前被回退炉的回退代码
	CString steel_return_code = "";
	CString run_status = "";
	CString sm_plan_no = "";
	CString ret_return_code = "";
	CString ret_run_status = "";
	CDbCommand cmd_tpssm11_inq(conn);

	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);

	EIClass inblock;
	inblock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblock.Tables[0].Rows.Add();
	try
	{	
		//------------------------------
		//获取传入的返送炉次信息
		tpssm35["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString();
		tpssm35["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm35["STEEL_RETURN_CODE"] = bcls_rec->Tables[0].Rows[0]["STEEL_RETURN_CODE"].ToString();

		Log::Trace("", __FUNCTION__, "=HEAT_NO = [{0}]", tpssm35["HEAT_NO"].ToString());
		//针对三种回炉方式的说明
		//1.回炉，包壁穿了(漏钢)时,现场一般用周转包倒包或倒出透红、漏钢部位以上钢水,当倒出钢水太多，不能继续精炼时，可能需要回炉，对应PONO无铸机计划
		//2.兑包/折包，包壁穿了（漏钢）时,现场一般用周转包倒包或倒出透红、漏钢部位以上钢水，当倒出钢水太多，不能继续精炼时，可能需要折包或兑包（倒入其他钢包），对应PONO无铸机计划
		//3.分割，包壁穿了（漏钢），钢水被目标炉次消耗，认为目标炉次出钢结束，目标炉次继续进行下一工序生产


		tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
		heat_no = tpssm35["HEAT_NO"];
		factory_div = tpssm35["FACTORY_DIV"];

		if (tpssm35.QueryCount("HEAT_NO") > 0)
		{
			CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
			CMessageFormat::Format(s.msg, _RES("PSSMS0000199")/*熔炼号={0}的计划已经回炉。*/, arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//wcy 复制表数据处理
		//tpssm13["HEAT_NO"] = tpssm35["HEAT_NO"];
		//tpssm13["PLAN_EDIT_FLAG"] = "C";
		//tpssm13.Update("PLAN_EDIT_FLAG", "HEAT_NO");

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT RUN_STATUS,PONO,STEEL_RETURN_CODE,SM_PLAN_NO  FROM  TPSSM11 \
					 WHERE    HEAT_NO = @tpssm11.HEAT_NO ";
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);

		cmd_tpssm11_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());		
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			run_status = cmd_tpssm11_inq.GetString(1);
			tpssm35["PONO"] = cmd_tpssm11_inq.GetString(2);
			steel_return_code = cmd_tpssm11_inq.GetString(3);
			plan_no = cmd_tpssm11_inq.GetString(4);
		}
		cmd_tpssm11_inq.Close();


		//------------------------------
		//校验
		//if(tpssm13.STEEL_RETURN_CODE.Trim() == "1")
		//{
		//	CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
		//	CMessageFormat::Format(s.msg,  _RES("PSSMS0000199")/*熔炼号={0}的计划已经回炉。*/, arguments, 1);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		if (run_status.Trim() <= "30")
		{
			CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
			CMessageFormat::Format(s.msg,  _RES("PSSMS0000171")/*熔炼号={0}的计划未开始生产，不能回炉。*/, arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		inblock.Tables[0].Rows[0]["SM_PLAN_NO"] = plan_no; //wcy 回炉删除计划
		ret = f_plan_delete_snd2(&inblock, bcls_ret, conn); 
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//------------------------------
		//更新TPSSM11 - 源侧
		tpssm11["STEEL_RETURN_CODE"] = tpssm35["STEEL_RETURN_CODE"];
		tpssm11["PONO_STATUS"] = 83;
		if (tpssm35["STEEL_RETURN_CODE"].ToString() == '2')
		{
			tpssm11["RUN_STATUS"] = "84";
		}
		else
		{
			tpssm11["RUN_STATUS"] = "83";
		}	
		tpssm11["CURR_WP_NO"] = 9;
		tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
		if (tpssm35["RET_TIME"].ToString().Trim() == "")
		{
			tpssm35["RET_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
		}
		sqlstr = "tpssm11.Update";
		//for循环中一起更新 tpssm11.Update("STEEL_RETURN_CODE,PONO_STATUS,RUN_STATUS,CURR_WP_NO,RET_TIME","HEAT_NO");

		////Log::Trace("", __FUNCTION__, "---- tpssm11.update");
		////Log::Trace("", __FUNCTION__, "tpssm35["STEEL_RETURN_CODE"] = [{0}]", tpssm35["STEEL_RETURN_CODE"].ToString());
		////Log::Trace("", __FUNCTION__, "tpssm11["STEEL_RETURN_CODE"] = [{0}]", tpssm11["STEEL_RETURN_CODE"].ToString());
		////Log::Trace("", __FUNCTION__, "tpssm11["RUN_STATUS"] = [{0}]", tpssm11["RUN_STATUS"].ToString());

		////更新TPSSM10
		//tpssm10["PONO_STATUS"] = 83;
		//tpssm10["PONO"] = tpssm35["PONO"];
		//sqlstr = "tpssm10.Update";
		//tpssm10.Update("PONO_STATUS","PONO");
		//
	
		////更新TPSSM01
		//tpssm01["PONO_STATUS"] = 83;
		//tpssm01["PONO"] = tpssm35["PONO"];
		//sqlstr = "tpssm01.Update";
		//tpssm01.Update("PONO_STATUS", "PONO");
		
	
		//获得输入参数
		for (int i = 0; i <  bcls_rec->Tables[0].Rows.get_Count() ; i++ )
		{
			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据
			tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			////Log::Trace("", __FUNCTION__, "tpssm35["RET_DEST"] = [{0}]", tpssm35["RET_DEST"].ToString());
			tpssm35["FACTORY_DIV"]	= factory_div;
			tpssm35["HEAT_NO"]				= heat_no;
			tpssm35["REC_CREATOR"]			= s.userid;
			tpssm35["REC_CREATE_TIME"]		= CDateTime::Now().ToString("yyyyMMddHHmmss"); 

			////Log::Trace("", __FUNCTION__, "tpssm35["STEEL_RETURN_CODE"] = [{0}]", tpssm35["STEEL_RETURN_CODE"].ToString());

			if (tpssm35["RET_PONO"].ToString().Trim() == ""&&tpssm35["STEEL_RETURN_CODE"].ToString() == "3")
			{
				sprintf(s.msg, "炉次分割未指定目标制造命令号！");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (tpssm35.QueryCount("RET_PONO") > 0)
			{
				sprintf(s.msg, "指定目标制造命令号已经做过其它的炉次返送，不可重复接受返送！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//更新甘特图上的标志，1：回炉，2：兑包/折包
			tpssm11["HEAT_NO"] = heat_no;
			tpssm11["FACTORY_DIV"] = factory_div;
			tpssm11["STEEL_RETURN_CODE"] = tpssm35["STEEL_RETURN_CODE"];
			tpssm11["RET_HEAT_NO"] = tpssm35["RET_HEAT_NO"];
			tpssm11["RET_PONO"] = tpssm35["RET_PONO"];
			tpssm11["RET_TIME"] = tpssm35["RET_TIME"];
			if (tpssm35["STEEL_RETURN_CODE"].ToString() == "3")
			{
				//炉次分割
				if (tpssm35["RET_HEAT_NO"].ToString().Trim() == "")
				{
					//tpssm35["RET_HEAT_NO"] = heat_no + "A";
					tpssm35["RET_HEAT_NO"] = heat_no.SubstringNE(0, 4) + "A" + heat_no.SubstringNE(5);
				}				
				tpssm11["STEEL_RETURN_CODE"] = "5";
			}
			
			sqlstr = "---tpssm11.Update";
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);				
			tpssm11.Update("STEEL_RETURN_CODE,PONO_STATUS,RUN_STATUS,CURR_WP_NO, RET_HEAT_NO,RET_PONO,RET_TIME", "HEAT_NO,FACTORY_DIV");

			//更新TPSSM10 -- 源侧
			tpssm10["PONO_STATUS"] = 83;
			tpssm10["PONO"] = tpssm35["PONO"];
			sqlstr = "tpssm10.Update";
			tpssm10.Update("PONO_STATUS", "PONO");


			//更新TPSSM01 -- 源侧
			tpssm01["PONO_STATUS"] = 83;
			tpssm01["PONO"] = tpssm35["PONO"];
			sqlstr = "tpssm01.Update";
			tpssm01.Update("PONO_STATUS", "PONO");
			
			//------------------  目标侧  -------------------------

			//查询目的计划信息
			sm_plan_no = "";
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = " SELECT RUN_STATUS ,SM_PLAN_NO,STEEL_RETURN_CODE FROM TPSSM11 \
						 	WHERE FACTORY_DIV = @tpssm35.FACTORY_DIV \
							AND PONO           = @tpssm35.RET_PONO ";
				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm35.FACTORY_DIV", tpssm35["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm35.RET_PONO", tpssm35["RET_PONO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			if (cmd_tpssm11_inq.Read())
			{
				//tpssm11["RUN_STATUS"] = cmd_tpssm11_inq.GetString(1);
				ret_run_status = cmd_tpssm11_inq.GetString(1);
				sm_plan_no = cmd_tpssm11_inq.GetString(2).Trim();
				ret_return_code = cmd_tpssm11_inq.GetString(3);
			}
			cmd_tpssm11_inq.Close();
			////Log::Trace("", __FUNCTION__, "返送炉号对应的sm_plan_no{0}-返送代码[{1}]", sm_plan_no,ret_return_code);
			////Log::Trace("", __FUNCTION__, "返送炉号对应的FACTORY_DIV{0}-RET_PONO[{1}]", tpssm35["FACTORY_DIV"].ToString(), tpssm35["RET_PONO"].ToString());
			
			
			if (ret_return_code.Trim() != "")
			{
				////Log::Trace("", __FUNCTION__, "ret_return_code!=空", ret_return_code);
				CFormattable arguments[] = { sm_plan_no }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "返送计划号[{0}]已做过回炉和返送，不能再操作。", arguments, 1); //格式化字符串
				////Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			


			if (tpssm35["STEEL_RETURN_CODE"].ToString() == "3")
			{
				//------------------------
				//炉次分割满足条件
				//1.返送炉次要浇完  返送炉次不能是已经回炉的状态
				//2.目标炉次 不能开始生产
				//这里的炉次分割意思指  转炉工序下一炉与上一炉，
				if (tpssm35["RET_HEAT_NO"].ToString().Trim() == "")
				{
					//tpssm35["RET_HEAT_NO"] = heat_no + "A";
					tpssm35["RET_HEAT_NO"] = heat_no.SubstringNE(0, 4) + "A" + heat_no.SubstringNE(5);
				}
				
				if (steel_return_code.Trim() == "1")
				{
					CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
					CMessageFormat::Format(s.msg,  _RES("PSSMS0000199")/*熔炼号={0}的计划已经回炉。*/, arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				////Log::Trace("", __FUNCTION__, "run_status{0}", run_status);
				if (run_status.Trim() < "53")
				{
					sprintf(s.msg, "熔炼号[%s]尚未浇铸结束，不能作为炉次分割！", (const char*)tpssm35["HEAT_NO"].ToString());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				////Log::Trace("", __FUNCTION__, "返送炉号对应的ret_run_status{0}", ret_run_status);
				if (ret_run_status.Trim() >= "30")
				{
					sprintf(s.msg, "炉订号[%s]已生产，不能作为炉次分割目标！", (const char*)tpssm35["RET_PONO"].ToString());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				////Log::Trace("", __FUNCTION__, "返送炉号对应的STEEL_RETURN_CODE{0}", tpssm11["STEEL_RETURN_CODE"].ToString());
				
				//----------------------------------
				//更新数据表  以目标炉次为主键更新
				//更新TPSSM10
				tpssm10["PONO_STATUS"] = 20;
				tpssm10["PONO"] = tpssm35["RET_PONO"];
				tpssm10.Update("PONO_STATUS", "PONO");

				//更新TPSSM01
				tpssm01["PONO_STATUS"] = 20;
				tpssm01["PONO"] = tpssm35["RET_PONO"];
				tpssm01.Update("PONO_STATUS", "PONO");


				//如果已经排入计划
				if (sm_plan_no != "")
				{
					//更新TPSSM11
					tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"];
					tpssm11["PONO_STATUS"] = 20;
					tpssm11["RUN_STATUS"] = "36";
					tpssm11["CURR_WP_NO"] = 1;
					tpssm11["PONO"] = tpssm35["RET_PONO"];
					tpssm11["STEEL_RETURN_CODE"] = "6";
					tpssm11["RET_HEAT_NO"] = heat_no;
					tpssm11["RET_PONO"] = tpssm35["PONO"];
					tpssm11["RET_TIME"] = tpssm35["RET_TIME"];
					tpssm11.Update("HEAT_NO,PONO_STATUS,RUN_STATUS,CURR_WP_NO,STEEL_RETURN_CODE,RET_HEAT_NO,RET_PONO,RET_TIME", "PONO");


					//更新TPSSM12表
					tpssm11["PONO"] = tpssm35["PONO"];
					tpssm11.Query("PONO");
					tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					////Log::Trace("", __FUNCTION__, "tpssm12.SM_PLAN_NO{0}", tpssm12["SM_PLAN_NO"].ToString());
					tpssm12["AREA_ID"] = 3;
					if (tpssm12.Query("SM_PLAN_NO,AREA_ID"))
					{
						////Log::Trace("", __FUNCTION__, "sm_plan_no-{0}AREA_ID[{1}]", tpssm12["SM_PLAN_NO"].ToString(), tpssm12["AREA_ID"].ToDecimal());
						////Log::Trace("", __FUNCTION__, "sm_plan_no-{0}", sm_plan_no);
						tpssm12["SM_PLAN_NO"] = sm_plan_no;
						tpssm12["AREA_ID"] = 3;
						tpssm12["PROC_NO"] = tpssm35["RET_HEAT_NO"];
						tpssm12["HEAT_NO"] = tpssm35["RET_HEAT_NO"];
						tpssm12.Print();
						sqlstr = "tpssm12.Update";
						tpssm12.Update("DEV_CODE,PROC_NO,HEAT_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME", "SM_PLAN_NO,AREA_ID");
						////Log::Trace("", __FUNCTION__, "sm_plan_no-{0}AREA_ID[{1}]", sm_plan_no, tpssm12["AREA_ID"].ToDecimal());
						tpssm12.Update("HEAT_NO", "SM_PLAN_NO");

					}
				}

			}

			////Log::Trace("", __FUNCTION__, "tpssm35["HEAT_NO"] = [{0}]", tpssm35["HEAT_NO"].ToString());
			//插入一条新记录到TPSSM35
			tpssm35.Insert();
			
			//写履历表
			tpssm99["FACTORY_DIV"] = factory_div;
			tpssm99["PONO"] = tpssm35["PONO"];
			tpssm99["HEAT_NO"] = tpssm11["HEAT_NO"];
			tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
			tpssm99["EVENT_ID"] = "G1";
			////Log::Trace("", __FUNCTION__, "doFlag=[{0}]", doFlag);
			tpssm99["RET_POS"] = tpssm35["RET_DEST"];
			tpssm99["RETURN_MLSL"] = tpssm35["RETURN_MLSL"];

			if (doFlag == -1)
			{//发生异常
				tpssm99["VALID_FLAG"] = "0";

			}
			else
			{
				tpssm99["VALID_FLAG"] = "1";
			}

			
			tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
			////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
			////Log::Trace("", __FUNCTION__, "记录失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
			//记录编入计划成功的履历
			ret = 0;
			ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
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
	cmd_tpssm11_inq.Close();	
	return doFlag;

}

	
