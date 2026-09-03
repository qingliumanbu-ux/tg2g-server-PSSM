/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-17
Version:  3.1
Description:  出钢计划各工序作业时刻计算（只对新增炉次、修改炉次计算）
Update：   2014-11-13  xuwen  炉次条件合并，增加转炉区子阶段计算
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


//#include "tpssmd1.h"




/*<remark>=========================================================
/// <summary>
/// 出钢计划指定炉次各工序作业时刻计算（只对新增炉次、修改炉次计算，旧炉次由模型或人工指定调整）
/// <para>处理流程：倒序工艺路径，由后向前推算：                    </para>
/// <para>1)由本工序到下工序的移行时间，推算本工序包离开时刻；      </para>
/// <para>2)由等待时间和包离开时刻，    推算本工序处理结束时刻；    </para>
/// <para>3)由处理时间和处理结束时刻，  推算本工序处理开始时刻；    </para>
/// <para>4)由准备时间和处理开始时刻，  推算本工序包到达时刻；      </para>
/// <para>数据库表：TPSSM11/12/16/d1/d6           </para>
/// <para>主调用函数：pssm11_add(出钢计划编入),pssm12_upd(计划修改)调用 </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/
int f_pssm12_time_calc(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int ret=0;
	//int fetchRowCount=0;
	int i, rows, blkseq;

	CDateTime tmp_time;
	double  diff_time = 0;
	CString base_time = "";     //主工序的计算基准
	CString sub_base_time = ""; //子工序的计算基准
	CString dev_code = "";      //上一设备号

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	//CTPSSM16 tpssm16(conn);
	CModel tpssmd3_ds("TPSSMD3");
	CModel tpssmd3_pi("TPSSMD3");
	CModel tpssmd6("TPSSMD6");

	CDbCommand cmd_tpssm12_inq(conn);

	CString sqlstr;

	try
	{
		//----------------------------------------------------
		//读取脱硫设备公共条件
		tpssmd3_ds["DEV_CODE"] = "S1";
		sqlstr = "tpssmd3_ds.Query()";
		tpssmd3_ds.Query("DEV_CODE, FACTORY_DIV");

		tpssmd3_pi["DEV_CODE"] = "D1";
		sqlstr = "tpssmd3_pi.Query()";
		tpssmd3_pi.Query("DEV_CODE, FACTORY_DIV");


		//----------------------------------------------------
		//获得输入参数
		blkseq = bcls_rec->Tables.IndexOf("PONO"); //计划信息
		if (blkseq < 0) 
		{
			strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//读取编入计划的PONO(新增炉次)，多记录方式
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{
			//tpssm10.MergeFrom(bcls_rec->Tables[0].Rows[0]);
			tpssm11["PONO"] = bcls_rec->Tables[blkseq].Rows[i]["PONO"];
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];

			////Log::Info("", __FUNCTION__, "PONO[{0}] =[{1}]", i, tpssm11["PONO"].ToString());

			//循环读取各工序的信息
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					" AND SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV AND PONO = @tpssm11.PONO) "
					"  ORDER BY CHARGE_NO DESC, SUB_CHARGE_NO ASC "  //倒推各工序时刻，顺推转炉区子阶段
					);
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString().Trim());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString().Trim());
			cmd_tpssm12_inq.ExecuteReader();
			while(cmd_tpssm12_inq.Read())
			{

				cmd_tpssm12_inq.Fetch(tpssm12);
				tpssm12.TrimOrBlank();

				/*----取传搁时间:设备移行时间---------------*/
				//1.查设备代码 
				//tpssmd1.DEV_CODE   = tpssm12["DEV_CODE"];
				//tpssmd1.AREA_ID    = tpssm12["AREA_ID"];
				//sqlstr = "tpssmd1.Query()";
				//bool hasd1 = tpssmd1.Query("SM_UNIT_NO,DEV_CODE,AREA_ID");
				//if (hasd1 == false) //没找到对应记录
				//{
				//	CFormattable arguments[] = { tpssmd1.DEV_CODE }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "各工序时刻计算时，没找到设备[{0}]的配置信息。", arguments, 1);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}
				//tpssmd1.TrimOrBlank();
				

				if (tpssm12["AREA_ID"].ToDecimal() == 5) //连铸工序
				{
					//记录包到连铸时刻
					if (tpssm12["LADLE_ARRIVE_TIME"].ToString().Trim() != "")
					{
						base_time = tpssm12["LADLE_ARRIVE_TIME"];
					}
					else
					{
						base_time = tpssm12["START_TIME"];
					}

					dev_code = tpssm12["DEV_CODE"];
					continue;
				}


				//2.查询设备间移行时间
				tpssmd6["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				tpssmd6["DEV_MOVE_START"] = tpssm12["DEV_CODE"];
				tpssmd6["DEV_MOVE_END"]   = dev_code;
				sqlstr = "tpssmd6.Query()";
				bool hasd6 = tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
				if (hasd6 == false) //没找到对应记录
				{
					tpssmd6["MOVE_TIME"] = 5;  //系统默认工序间移动时间为5分钟，不报错
				}
				tpssmd6.TrimOrBlank();
				
				////Log::Trace("", __FUNCTION__, "base_time=[{0}]",(const char*)base_time);
				////Log::Trace("", __FUNCTION__, "tpssmd6["MOVE_TIME"] =[{0}]",tpssmd6["MOVE_TIME"].ToDecimal().ToInt32());


				/*----计算各工序的时间----------------------------------------------*/
				if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 0)  //主工序计算
				{
					//2种算法，只取其一：

					//----------------------------------------------
					////算法一：工序作业时间中包含包到、包离
					//// 1)由本工序到下工序的移行时间，推算本工序包离开时刻;
					//diff_time = (tpssmd6["MOVE_TIME"].ToDecimal() + tpssm12["REST_TIME"].ToDecimal()).ToDouble(); //传搁时间 + 休辅时间
					//tmp_time = CDateTime::Parse(base_time);
					//tpssm12["LADLE_LEAVE_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");

					////2)计算结束时刻 end_time : 由后处理时间，推算本工序处理结束时刻;
					//diff_time = tpssm12["POST_PROC_TIME"].ToDecimal().ToDouble(); //等待时间
					//tmp_time = CDateTime::Parse(tpssm12["LADLE_LEAVE_TIME"].ToString());
					//tpssm12["END_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");

					////3)计算开始时刻 start_time : 由处理时间和处理结束时刻，  推算本工序处理开始时刻;
					//diff_time = tpssm12["PROC_TIME"].ToDecimal().ToDouble();
					//tmp_time = CDateTime::Parse(tpssm12["END_TIME"].ToString());
					//tpssm12["START_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");

					////4)由准备时间和处理开始时刻，  推算本工序包到达时刻;
					//diff_time = tpssm12["PRE_PROC_TIME"].ToDecimal().ToDouble();
					//tmp_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
					//tpssm12["LADLE_ARRIVE_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");


					//----------------------------------------------
					//算法二：工序作业时间中只计算处理时间，不包含包到、包离（包到、包离纳入到传搁时间里）
					//1)计算结束时刻 end_time : 由本工序到下工序的移行时间，推算本工序结束时刻;
					diff_time = (tpssmd6["MOVE_TIME"].ToDecimal() + tpssm12["REST_TIME"].ToDecimal()).ToDouble(); //传搁时间 + 休辅时间
					tmp_time = CDateTime::Parse(base_time);   //下工序开始
					tpssm12["END_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");

					//2)计算开始时刻 start_time : 由处理时间和处理结束时刻，推算本工序处理开始时刻;
					diff_time = tpssm12["PROC_TIME"].ToDecimal().ToDouble();
					tmp_time = CDateTime::Parse(tpssm12["END_TIME"].ToString());
					tpssm12["START_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");

					//3)由本工序后处理时间，推算本工序包离开时刻;
					diff_time = tpssm12["POST_PROC_TIME"].ToDecimal().ToDouble(); //后处理(等待)时间
					tmp_time = CDateTime::Parse(tpssm12["END_TIME"].ToString());
					tpssm12["LADLE_LEAVE_TIME"] = tmp_time.AddMinutes(diff_time).ToString("yyyyMMddHHmmss");

					//4)由准备时间和处理开始时刻，  推算本工序包到达时刻;
					diff_time = tpssm12["PRE_PROC_TIME"].ToDecimal().ToDouble();
					tmp_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
					tpssm12["LADLE_ARRIVE_TIME"] = tmp_time.AddMinutes(-diff_time).ToString("yyyyMMddHHmmss");


					////Log::Trace("", __FUNCTION__, "charge_no[{0}]: Arrive=[{1}], Start=[{2}], End=[{3}], Leave=[{4}]",
						//tpssm12["CHARGE_NO"].ToInt32(), (const char *)tpssm12["LADLE_ARRIVE_TIME"].ToString(), (const char *)tpssm12["START_TIME"].ToString(), (const char *)tpssm12["END_TIME"].ToString(), (const char *)tpssm12["LADLE_LEAVE_TIME"].ToString());


					//5.写入作业计划子表
					sqlstr = "tpssm12.Update(SUB_CHARGE_NO=0)";
					tpssm12.Update(
						"LADLE_ARRIVE_TIME,"
						"START_TIME,"
						"END_TIME,"
						"LADLE_LEAVE_TIME",
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO");


					//-------------------------------
					//6.转炉工序时，写入炼钢开始时刻
					if (tpssm12["AREA_ID"].ToDecimal() == 3 || tpssm12["AREA_ID"].ToDecimal() == 2) //脱磷算是炼钢开始
					{
						if (tpssm12["AREA_ID"].ToDecimal() == 3) tpssm11["STEEL_START_TIME"] = tpssm12["START_TIME"];

						//20160905 周平 目前没脱硫，倒灌
						////----------------------------------------------------
						////根据转炉开始时刻，推算脱S、倒罐结束时刻
						//diff_time = 6;   //铁水站到转炉的移动时间
						//tmp_time = CDateTime::Parse(tpssm11["STEEL_START_TIME"].ToString());
						//tpssm11.DS_END_TIME = tmp_time.AddMinutes(0 - diff_time).ToString("yyyyMMddHHmmss");  //预处理结束时刻

						//diff_time = tpssmd3_ds.PROC_TIME.ToDouble();   //铁水脱硫处理时间
						//tmp_time = CDateTime::Parse(tpssm11.DS_END_TIME);
						//tpssm11["TPD_END_TIME"] = tmp_time.AddMinutes(0 - diff_time).ToString("yyyyMMddHHmmss");  //预处理结束时刻
						////----------------------------------------------------

						//结果写入主表
						sqlstr = "tpssm11.Update(STEEL_START_TIME)";
						tpssm11.Update(
							"TPD_END_TIME,"
							"STEEL_START_TIME",
							"FACTORY_DIV, PONO");  //新增炉次按PONO操作(传入参数如此)，其他修改点的操作用主键炼钢计划号
					}


					dev_code = tpssm12["DEV_CODE"];

					//算法一：工序作业时间中包含包到、包离
					//base_time = tpssm12["LADLE_ARRIVE_TIME"]; //记录上次计算时间（以包到达为工序作业起点）

					//算法二：工序作业时间中不包含包到、包离，只考虑处理时间
					base_time = tpssm12["START_TIME"];    //记录上次计算时间（以工序作业开始为起点，因甘特图中用开始时刻）

					sub_base_time = base_time;         //为子工序计算用

				}
				else if(tpssm12["SUB_CHARGE_NO"].ToDecimal() >= 1)  //子工序计算，顺推转炉/电炉的3个子阶段
				{

					//此循环必须是先处理主工序（SUB_CHARGE_NO=0），再进入到子工序（SUB_CHARGE_NO >0 ）。否则算法错误
					//顺推子工序开始、结束时刻
					tpssm12["START_TIME"] = sub_base_time;

					diff_time = tpssm12["PROC_TIME"].ToDecimal().ToDouble();
					tmp_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
					tpssm12["END_TIME"] = tmp_time.AddMinutes(diff_time).ToString("yyyyMMddHHmmss");

					sqlstr = "tpssm12.Update(SUB_CHARGE_NO>0)";
					tpssm12.Update(
						"START_TIME,"
						"END_TIME",
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO");

					sub_base_time = tpssm12["END_TIME"];

				}//if SUB_CHARGE_NO == 0
			}// while
			cmd_tpssm12_inq.Close();


		}//for 

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
	cmd_tpssm12_inq.Close();

	return doFlag;
}
