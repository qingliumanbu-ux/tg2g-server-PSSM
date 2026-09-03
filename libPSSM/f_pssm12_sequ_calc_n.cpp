/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:  3.1.0
Description: 出钢计划各工序作业顺序号生成（包括HEAT_NO）
Update：  2014-11-13  xuwen  炉次条件合并
**************************************************/
#include "stdafx.h"

//程序用头文件





/*<remark>=========================================================
/// <summary>
/// 出钢计划各工序作业顺序计算（包括HEAT_NO。不需要各工序时刻的作法）
/// 根据配置的设备为单位，搜索计划信息，并排序赋号
/// <para>数据库表：TPSSM11/12/16/25/d1            </para>
/// <para>主调用函数：pssm11_add(出钢计划编入调用)              </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int fetchRowCount = 0;
	int  blkseq;//i, rows,

	CString  datetime;

	CDecimal    dummy=0;
	CDecimal    proc_seq_no=0;                    /* 处理顺序号 */
	CString   pre_proc_no="";                /* 预定处理号 */
	CDecimal    iproc_no=0;

	//CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm25("TPSSM25");
	//CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssm29("TPSSM29");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm16_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CString sqlstr;
	CString special_flag = "";

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");


		//----------------------------------------------------------------------------------
		//获得输入参数
		//读取编入计划的炼钢单元号，单记录方式
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm12_sequ_calc_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (bcls_rec->Tables[blkseq].Columns.Contains("SPECIAL_FLAG"))
		{
			special_flag = bcls_rec->Tables[blkseq].Rows[0]["SPECIAL_FLAG"].ToString();
		}

		if (special_flag != "1")
		{
			/*tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
			tpssm25["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];*/
			tpssm25["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
			tpssm25["REC_CREATE_TIME"] = datetime;
			tpssm25["REC_CREATOR"] = s.userid;


			//---------------------------------------------------------
			//循环读取各工序设备的信息
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM TPSSMD1 "
					"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"  ORDER BY STATION_ID, STATION_NO ASC "
					);
				break;
			}
			cmd_tpssmd1_inq.SetCommandText(sqlstr);
			cmd_tpssmd1_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString());
			cmd_tpssmd1_inq.ExecuteReader();
			fetchRowCount = 0;
			while (cmd_tpssmd1_inq.Read())
			{
				cmd_tpssmd1_inq.Fetch(tpssmd1);
				fetchRowCount++;
				tpssmd1.TrimOrBlank();

				//查看新增出钢计划中是否有该设备的作业
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT COUNT(1) FROM TPSSM12"
						"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
						"    AND AREA_ID    = @tpssmd1.AREA_ID "
						"    AND ( START_TIME_REAL = ' ' and END_TIME_REAL = ' ' and  ARRIVE_REAL_TIME=' ' and  LEAVE_REAL_TIME = ' ') "
						"    AND SM_PLAN_NO IN "
						"        ( "
						"           SELECT SM_PLAN_NO FROM TPSSM11 "
						"            WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"              AND PONO_STATUS < 83 "
						"        ) "
						);
					break;
				}

				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				dummy = cmd_tpssm12_inq.ExecuteScalar();
				if (dummy == 0) //没有则不再做计算
				{
					continue;
				}

				//读取该设备下的最大顺序号
				//switch(conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:         // MS SQL Server数据库
				//case DB_KIND_ORACLE:        // Oracle 数据库
				//default:  // 所有数据库适用，通用SQL语句
				//	sqlstr = CString(
				//		" SELECT MAX(PRE_PROC_NO) "
				//		"   FROM TPSSM12 "
				//		"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
				//		"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
				//		"    AND AREA_ID    = @tpssmd1.AREA_ID "
				//		);
				//	break;
				//}

				//cmd_tpssm12_inq.SetCommandText(sqlstr);
				//cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV",tpssmd1["FACTORY_DIV"].ToString());
				//cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE",tpssmd1["DEV_CODE"].ToString());
				//cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID",tpssmd1["AREA_ID"].ToDecimal());
				//cmd_tpssm12_inq.ExecuteReader();
				//if(cmd_tpssm12_inq.Read())
				//{				
				//	pre_proc_no = cmd_tpssm12_inq.GetString(1).TrimOrBlank();
				//}
				//else
				//{
				//	pre_proc_no = " ";
				//}
				//cmd_tpssm12_inq.Close();

				//////Log::Trace("", __FUNCTION__, "dev_code[{0}]:  pre_proc_no=[{1}]",(const char*)tpssmd1["DEV_CODE"].ToString(),  (const char*)pre_proc_no);

				////Log::Trace("", __FUNCTION__, "tpssmd1["FACTORY_DIV"] =[{0}]", tpssmd1["FACTORY_DIV"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssmd1["STATION_ID"] =[{0}]", tpssmd1["STATION_ID"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssmd1["STATION_NO"] =[{0}]", tpssmd1["STATION_NO"].ToString());

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT CURR_PROC_NO FROM TPSSM25 "
						"  WHERE FACTORY_DIV   = @tpssmd1.FACTORY_DIV "
						"    AND STATION_ID   = @tpssmd1.STATION_ID "
						"    AND STATION_NO   = @tpssmd1.STATION_NO "
						);
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_ID", tpssmd1["STATION_ID"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_NO", tpssmd1["STATION_NO"].ToString());
				cmd_tpssm25_inq.ExecuteReader();
				if (cmd_tpssm25_inq.Read())
				{
					tpssm25["CURR_PROC_NO"] = cmd_tpssm25_inq.GetString(1);
				}
				else
				{
					//tpssm25["CURR_PROC_NO"] = pre_proc_no;
					tpssm25["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssm25["STATION_ID"] = tpssmd1["STATION_ID"];
					tpssm25["STATION_NO"] = tpssmd1["STATION_NO"];

					//Log::Trace("", __FUNCTION__, " tpssm25[CURR_PROC_NO]");

					if (tpssmd1["AREA_ID"].ToDecimal() == 3) //转炉 wcy 太钢工序标志 + 工位号 + 年末一位 + 5位流水号
					{
						tpssm25["CURR_PROC_NO"] = tpssm25["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1),
							(const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), 0);
					}
					else
					{
						tpssm25["CURR_PROC_NO"] = tpssm25["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1),
							(const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), 0);
					}

					Log::Trace("", __FUNCTION__, " tpssm25[CURR_PROC_NO] =[{0}]", (const char*)tpssm25["CURR_PROC_NO"].ToString());
					tpssm25.Insert();
				}
				cmd_tpssm25_inq.Close();

				//iproc_no = atol(&tpssm25["CURR_PROC_NO"].ToString()[3]); //从第5位开始流水, 5位流水--比如16C100001
				if (tpssm25["CURR_PROC_NO"].ToString().Trim().GetLength() > 0)
				{
					if (tpssmd1["AREA_ID"].ToDecimal() == 3) //转炉
					{
						iproc_no = iproc_no.Parse(tpssm25["CURR_PROC_NO"].ToString().Substring(3));
					}
					else
					{
						iproc_no = iproc_no.Parse(tpssm25["CURR_PROC_NO"].ToString().Substring(3));
					}
				}
				else
				{
					iproc_no = 0;
				}
				//Log::Trace("", __FUNCTION__, "当前流水号iproc_no=[{0}]", iproc_no);

				//----------------------------------------------------------
				//按开始时刻排序，循环读取各设备计划，并赋顺序号
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = CString(
						" SELECT * FROM TPSSM12 "
						"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
						"    AND AREA_ID    = @tpssmd1.AREA_ID "
						"    AND ( START_TIME_REAL=' ' and END_TIME_REAL=' ' and  ARRIVE_REAL_TIME=' ' and  LEAVE_REAL_TIME=' ') "
						"    AND SM_PLAN_NO IN "
						"        ( "
						"           SELECT SM_PLAN_NO FROM TPSSM11 "
						"            WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"              AND PONO_STATUS < 83 "  //83-浇铸完毕
						"        ) "
						"  ORDER BY START_TIME ASC "
						);
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				//cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				while (cmd_tpssm12_inq.Read())
				{
					cmd_tpssm12_inq.Fetch(tpssm12);
					tpssm12.TrimOrBlank();

					iproc_no = (iproc_no.ToInt32() + 1) % 100000; // 5位流水
					//HYF 20130427 SubstringNE---dclian---年末两位+工序标志+工位号+5位流水号---2016-02-26
					//sprintf(tpssm12["PRE_PROC_NO"].ToString(), "%c%c%c%.5d",(const char*)tpssmd1["STATION_ID"].ToString()[0],(const char*)tpssmd1["STATION_NO"].ToString()[0], date_time[3], iproc_no);


					if (tpssmd1["AREA_ID"].ToDecimal() == 3)
					{
						tpssm12["PRE_PROC_NO"] = tpssm12["PRE_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1), (const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), iproc_no.ToInt32());
					}
					else
					{
						tpssm12["PRE_PROC_NO"] = tpssm12["PRE_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1), (const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), iproc_no.ToInt32());
					}

					//Log::Trace("", __FUNCTION__, "sm_plan_no=[{0}],  pre_proc_no=[{1}]",(const char*)tpssm12["SM_PLAN_NO"].ToString(),(const char*)tpssm12["PRE_PROC_NO"].ToString());

					//赋值 
					sqlstr = "tpssm12.Update()";
					tpssm12.Update(
						"PRE_PROC_NO",   //预定处理号
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

					//if(tpssmd1["AREA_ID"].ToDecimal() == 3) //转炉、电炉时，写入主表
					//{					
					//	tpssm11["HEAT_NO"] = tpssm12["PRE_PROC_NO"];
					//	tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					//	tpssm11["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];

					//	sqlstr = "tpssm11.Update()";
					//	tpssm11.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");
					//	tpssm12["HEAT_NO"] = tpssm11["HEAT_NO"];
					//	sqlstr = "tpssm12.Update()";
					//	tpssm12.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");
					//}

				}
				cmd_tpssm12_inq.Close();

			}
			cmd_tpssmd1_inq.Close();
		}
		else
		{
			/*tpssm15["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
			tpssm29["FACTORY_DIV"] = tpssm15["FACTORY_DIV"];*/
			tpssm29["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
			tpssm29["REC_CREATE_TIME"] = datetime;
			tpssm29["REC_CREATOR"] = s.userid;


			//---------------------------------------------------------
			//循环读取各工序设备的信息
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM TPSSMD1 "
					"  WHERE FACTORY_DIV = @tpssm15.FACTORY_DIV "
					"  ORDER BY STATION_ID, STATION_NO ASC "
					);
				break;
			}
			cmd_tpssmd1_inq.SetCommandText(sqlstr);
			cmd_tpssmd1_inq.Parameters.Set("tpssm15.FACTORY_DIV", tpssm29["FACTORY_DIV"].ToString());
			cmd_tpssmd1_inq.ExecuteReader();
			fetchRowCount = 0;
			while (cmd_tpssmd1_inq.Read())
			{
				cmd_tpssmd1_inq.Fetch(tpssmd1);
				fetchRowCount++;
				tpssmd1.TrimOrBlank();

				//查看新增出钢计划中是否有该设备的作业
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT COUNT(1) FROM TPSSM16"
						"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
						"    AND AREA_ID    = @tpssmd1.AREA_ID "
						"    AND ( START_TIME_REAL = ' ' and END_TIME_REAL = ' ' and  ARRIVE_REAL_TIME=' ' and  LEAVE_REAL_TIME = ' ') "
						"    AND SM_PLAN_NO IN "
						"        ( "
						"           SELECT SM_PLAN_NO FROM TPSSM15 "
						"            WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"              AND PONO_STATUS < 83 "
						"        ) "
						);
					break;
				}

				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				dummy = cmd_tpssm12_inq.ExecuteScalar();
				if (dummy == 0) //没有则不再做计算
				{
					continue;
				}

				//读取该设备下的最大顺序号
				//switch(conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:         // MS SQL Server数据库
				//case DB_KIND_ORACLE:        // Oracle 数据库
				//default:  // 所有数据库适用，通用SQL语句
				//	sqlstr = CString(
				//		" SELECT MAX(PRE_PROC_NO) "
				//		"   FROM TPSSM16 "
				//		"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
				//		"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
				//		"    AND AREA_ID    = @tpssmd1.AREA_ID "
				//		);
				//	break;
				//}

				//cmd_tpssm12_inq.SetCommandText(sqlstr);
				//cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV",tpssmd1["FACTORY_DIV"].ToString());
				//cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE",tpssmd1["DEV_CODE"].ToString());
				//cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID",tpssmd1["AREA_ID"].ToDecimal());
				//cmd_tpssm12_inq.ExecuteReader();
				//if(cmd_tpssm12_inq.Read())
				//{				
				//	pre_proc_no = cmd_tpssm12_inq.GetString(1).TrimOrBlank();
				//}
				//else
				//{
				//	pre_proc_no = " ";
				//}
				//cmd_tpssm12_inq.Close();

				//////Log::Trace("", __FUNCTION__, "dev_code[{0}]:  pre_proc_no=[{1}]",(const char*)tpssmd1["DEV_CODE"].ToString(),  (const char*)pre_proc_no);

				////Log::Trace("", __FUNCTION__, "tpssmd1["FACTORY_DIV"] =[{0}]", tpssmd1["FACTORY_DIV"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssmd1["STATION_ID"] =[{0}]", tpssmd1["STATION_ID"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssmd1["STATION_NO"] =[{0}]", tpssmd1["STATION_NO"].ToString());

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT CURR_PROC_NO FROM TPSSM29 "
						"  WHERE FACTORY_DIV   = @tpssmd1.FACTORY_DIV "
						"    AND STATION_ID   = @tpssmd1.STATION_ID "
						"    AND STATION_NO   = @tpssmd1.STATION_NO "
						);
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_ID", tpssmd1["STATION_ID"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_NO", tpssmd1["STATION_NO"].ToString());
				cmd_tpssm25_inq.ExecuteReader();
				if (cmd_tpssm25_inq.Read())
				{
					tpssm29["CURR_PROC_NO"] = cmd_tpssm25_inq.GetString(1);
				}
				else
				{
					//tpssm29["CURR_PROC_NO"] = pre_proc_no;
					tpssm29["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssm29["STATION_ID"] = tpssmd1["STATION_ID"];
					tpssm29["STATION_NO"] = tpssmd1["STATION_NO"];

					//Log::Trace("", __FUNCTION__, " tpssm29[CURR_PROC_NO]");

					if (tpssmd1["AREA_ID"].ToDecimal() == 3) //转炉 wcy 太钢工序标志 + 工位号 + 年末一位 + 5位流水号
					{
						tpssm29["CURR_PROC_NO"] = tpssm29["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1),
							(const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), 0);
					}
					else
					{
						tpssm29["CURR_PROC_NO"] = tpssm29["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1),
							(const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), 0);
					}

					Log::Trace("", __FUNCTION__, " tpssm29[CURR_PROC_NO] =[{0}]", (const char*)tpssm29["CURR_PROC_NO"].ToString());
					tpssm29.Insert();
				}
				cmd_tpssm25_inq.Close();

				//iproc_no = atol(&tpssm29["CURR_PROC_NO"].ToString()[3]); //从第5位开始流水, 5位流水--比如16C100001
				if (tpssm29["CURR_PROC_NO"].ToString().Trim().GetLength() > 0)
				{
					if (tpssmd1["AREA_ID"].ToDecimal() == 3) //转炉
					{
						iproc_no = iproc_no.Parse(tpssm29["CURR_PROC_NO"].ToString().Substring(3));
					}
					else
					{
						iproc_no = iproc_no.Parse(tpssm29["CURR_PROC_NO"].ToString().Substring(3));
					}
				}
				else
				{
					iproc_no = 0;
				}
				//Log::Trace("", __FUNCTION__, "当前流水号iproc_no=[{0}]", iproc_no);

				//----------------------------------------------------------
				//按开始时刻排序，循环读取各设备计划，并赋顺序号
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = CString(
						" SELECT * FROM TPSSM16 "
						"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
						"    AND AREA_ID    = @tpssmd1.AREA_ID "
						"    AND ( START_TIME_REAL=' ' and END_TIME_REAL=' ' and  ARRIVE_REAL_TIME=' ' and  LEAVE_REAL_TIME=' ') "
						"    AND SM_PLAN_NO IN "
						"        ( "
						"           SELECT SM_PLAN_NO FROM TPSSM15 "
						"            WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"              AND PONO_STATUS < 83 "  //83-浇铸完毕
						"        ) "
						"  ORDER BY START_TIME ASC "
						);
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				//cmd_tpssm12_inq.Parameters.Set("tpssm15.FACTORY_DIV",tpssm15["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				while (cmd_tpssm12_inq.Read())
				{
					cmd_tpssm12_inq.Fetch(tpssm16);
					tpssm16.TrimOrBlank();

					iproc_no = (iproc_no.ToInt32() + 1) % 100000; // 5位流水
					//HYF 20130427 SubstringNE---dclian---年末两位+工序标志+工位号+5位流水号---2016-02-26
					//sprintf(tpssm16["PRE_PROC_NO"].ToString(), "%c%c%c%.5d",(const char*)tpssmd1["STATION_ID"].ToString()[0],(const char*)tpssmd1["STATION_NO"].ToString()[0], date_time[3], iproc_no);


					if (tpssmd1["AREA_ID"].ToDecimal() == 3)
					{
						tpssm16["PRE_PROC_NO"] = tpssm16["PRE_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1), (const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), iproc_no.ToInt32());
					}
					else
					{
						tpssm16["PRE_PROC_NO"] = tpssm16["PRE_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1), (const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), iproc_no.ToInt32());
					}

					//Log::Trace("", __FUNCTION__, "sm_plan_no=[{0}],  pre_proc_no=[{1}]",(const char*)tpssm16["SM_PLAN_NO"].ToString(),(const char*)tpssm16["PRE_PROC_NO"].ToString());

					//赋值 
					sqlstr = "tpssm16.Update()";
					tpssm16.Update(
						"PRE_PROC_NO",   //预定处理号
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

					//if(tpssmd1["AREA_ID"].ToDecimal() == 3) //转炉、电炉时，写入主表
					//{					
					//	tpssm15["HEAT_NO"] = tpssm16["PRE_PROC_NO"];
					//	tpssm15["FACTORY_DIV"] = tpssm16["FACTORY_DIV"];
					//	tpssm15["SM_PLAN_NO"] = tpssm16["SM_PLAN_NO"];

					//	sqlstr = "tpssm15.Update()";
					//	tpssm15.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");
					//	tpssm16["HEAT_NO"] = tpssm15["HEAT_NO"];
					//	sqlstr = "tpssm16.Update()";
					//	tpssm16.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");
					//}

				}
				cmd_tpssm12_inq.Close();

			}
			cmd_tpssmd1_inq.Close();
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

	cmd_tpssm12_inq.Close();
	cmd_tpssm25_inq.Close();
	cmd_tpssmd1_inq.Close();

	return doFlag;
}
