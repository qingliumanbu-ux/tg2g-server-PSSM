/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM09画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12z_save2)

int f_pssm12z_save2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString deal_flag = "";
	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	CString datetime = "";
	int		TotalRecordCount = 0;
	int		row_count = 0;
	int		row_count2 = 0;
	int		fetchRowCount = 0;
	CDecimal	dummy = 0;
	CDecimal    iproc_no = 0;
	CModel tpssm12z("TPSSM12Z");
	CModel tpssm25("TPSSM25");
	CModel tpssmd1("TPSSMD1");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssm12z_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);

	//表0是INS 表1是DEL
	try
	{
		//deal_flag = bcls_rec->Tables[0].Rows[0]["DEAL_FLAG"].ToString().Trim();
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		row_count = bcls_rec->Tables[0].Rows.get_Count();
		row_count2 = bcls_rec->Tables[1].Rows.get_Count();
		Log::Trace("", __FUNCTION__, "row_count=[{0}]", row_count);
		Log::Trace("", __FUNCTION__, "row_count2=[{0}]", row_count2);

		sqlstr = " DELETE FROM TPSSM12Z WHERE START_TIME_REAL = ' ' AND END_TIME_REAL = ' ' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		if (row_count > 0)
		{
			for (int i = 0; i < row_count; i++)
			{
				tpssm12z.Reset();
				tpssm12z.MergeFrom(bcls_rec->Tables[0].Rows[i]);

				if (tpssm12z.QueryCount("ID_SJ") == 0)
				{
					tpssm12z["REC_CREATOR"] = s.userid;
					tpssm12z["REC_CREATE_TIME"] = datetime;
					tpssm12z.Insert();
				}
				else
				{
					tpssm12z["REC_REVISOR"] = s.userid;
					tpssm12z["REC_REVISE_TIME"] = datetime;
					tpssm12z.Update("ID_SJ");
				}
			}
		}

		if (row_count2 > 0)
		{
			for (int i = 0; i < row_count2; i++)
			{
				tpssm12z.Reset();
				tpssm12z.MergeFrom(bcls_rec->Tables[1].Rows[i]);
				tpssm12z.Delete("ID_SJ");
			}
		}

		//计算处理号
		tpssm25["FACTORY_DIV"] = "LG1";
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
				"  WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
				"  AND STATION_ID = 'Z' "
				"  ORDER BY STATION_ID, STATION_NO ASC "
				);
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssm25.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString());
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
					" SELECT COUNT(1) FROM TPSSM12Z"
					"  WHERE DEV_CODE   = @tpssmd1.DEV_CODE "
					"    AND ( START_TIME_REAL = ' ' and END_TIME_REAL = ' ') "
					);
				break;
			}

			cmd_tpssm12z_inq.SetCommandText(sqlstr);
			cmd_tpssm12z_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
			cmd_tpssm12z_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			cmd_tpssm12z_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
			dummy = cmd_tpssm12z_inq.ExecuteScalar();
			if (dummy == 0) //没有则不再做计算
			{
				continue;
			}

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

				tpssm25["CURR_PROC_NO"] = tpssm25["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", "F",
					(const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), 0);


				Log::Trace("", __FUNCTION__, " tpssm25[CURR_PROC_NO] =[{0}]", (const char*)tpssm25["CURR_PROC_NO"].ToString());
				tpssm25.Insert();
			}
			cmd_tpssm25_inq.Close();

			//iproc_no = atol(&tpssm25["CURR_PROC_NO"].ToString()[3]); //从第5位开始流水, 5位流水--比如16C100001
			if (tpssm25["CURR_PROC_NO"].ToString().Trim().GetLength() > 0)
			{
				iproc_no = iproc_no.Parse(tpssm25["CURR_PROC_NO"].ToString().Substring(3));
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
					" SELECT * FROM TPSSM12Z "
					"  WHERE DEV_CODE   = @tpssmd1.DEV_CODE "
					"    AND ( START_TIME_REAL=' ' and END_TIME_REAL=' ') "
					"  ORDER BY START_TIME ASC "
					);
				break;
			}
			cmd_tpssm12z_inq.SetCommandText(sqlstr);
			cmd_tpssm12z_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			cmd_tpssm12z_inq.ExecuteReader();
			while (cmd_tpssm12z_inq.Read())
			{
				tpssm12z.Reset();
				cmd_tpssm12z_inq.Fetch(tpssm12z);
				tpssm12z.TrimOrBlank();

				iproc_no = (iproc_no.ToInt32() + 1) % 100000; // 5位流水

				tpssm12z["PROC_NO"] = tpssm12z["PROC_NO"].ToString().Format("%s%s%s%.5d", "F", (const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), (const char*)datetime.Trim().SubstringNE(3, 1), iproc_no.ToInt32());


				//Log::Trace("", __FUNCTION__, "sm_plan_no=[{0}],  pre_proc_no=[{1}]",(const char*)tpssm12["SM_PLAN_NO"].ToString(),(const char*)tpssm12["PRE_PROC_NO"].ToString());

				//赋值 
				sqlstr = "tpssm12z.Update()";
				tpssm12z.Update(
					"PROC_NO",   //预定处理号
					"ID_SJ");

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
			cmd_tpssm12z_inq.Close();

		}
		cmd_tpssmd1_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}


	return doFlag;

}
