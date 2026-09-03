/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhengqiangqiang
Version:  1.0
Date:     2023-05-12
Description: 查询炼钢作业符合率表
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "math.h"
//程序用头文件


// service入口
BM2F_ENTERACE(pssmt_ccd_inq)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssmt_ccd_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{

	CTracer log(__FUNCTION__);

	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount;
	int cd_count = 0;
	CDecimal month_num = 0;
	CDecimal month_fh_num = 0;

	CString	factory_div = "";
	CString	station_id = "C";
	CString	shift_group = "";
	CString	st_no = "";

	CString	date_time_begin = "";
	CString	date_time_end = "";
	CString	date_time = "";
	CString	cs_start_time = "";
	CString	cs_end_time = "";
	CString	cs_dev_code = "";
	CString prod_shift_no = "";
	CString prod_shift_group = "";


	CString sqlstr("");

	/* 实体类定义 */
	CModel tpssmb0("TPSSMB0");


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	try
	{
		//设置返回块
		//bcls_ret->Tables[0].set_TableName("D");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROD_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STD_PROC_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROC_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FH_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRO_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FH_PERCENT_D");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MONTH_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MONTH_FH_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FH_PERCENT_M");



		//测试用
		date_time = CDateTime::Now().AddDays(0).ToString("yyyyMMddHHmmss");//默认当天

		//获得输入参数
		if (bcls_rec->Tables[0].Rows[0]["DATE_TIME_BEGIN"].ToString().Trim() != "")
		{
			cs_start_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME_BEGIN"].ToString().Substring(0, 8);
		}
		else
		{
			cs_start_time = date_time.Substring(0, 6) + "01";
		}

		if (bcls_rec->Tables[0].Rows[0]["DATE_TIME_END"].ToString().Trim() != "")
		{
			cs_end_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME_END"].ToString().Substring(0, 8);
		}
		else
		{
			cs_end_time = date_time.Substring(0, 8);
		}

		/*if (bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().Trim() != "")
		{
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString();
		}*/


		if (bcls_rec->Tables[0].Rows[0]["SHIFT_GROUP"].ToString().Trim() != "")
		{
			shift_group = bcls_rec->Tables[0].Rows[0]["SHIFT_GROUP"].ToString();
		}

		//if (bcls_rec->Tables[0].Rows[0]["STATION_ID"].ToString().Trim() != "")
		//{
		//	station_id = bcls_rec->Tables[0].Rows[0]["STATION_ID"].ToString();
		//}

		//if (bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim() != "")
		//{
		//	factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		//}


		Log::Trace("", __FUNCTION__, "...打印传入参数...");
		Log::Trace("", __FUNCTION__, "...factory_div=[{0}]", factory_div);
		Log::Trace("", __FUNCTION__, "...cs_start_time=[{0}]", cs_start_time);
		Log::Trace("", __FUNCTION__, "...cs_end_time=[{0}]", cs_end_time);
		Log::Trace("", __FUNCTION__, "...station_id=[{0}]", station_id);
		Log::Trace("", __FUNCTION__, "...shift_group=[{0}]", shift_group);




		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
			sqlstr = " SELECT STATION_ID															"
				" , PROD_TIME																		"
				" , COUNT(*)  PRO_NUM																"
				" , SUM(CASE WHEN FH_FLAG = '1' THEN 1 ELSE 0 END) FH_NUM							"
				" , CAST(SUM(STD_PROC_TIME) / double(COUNT(*)) as DECIMAL(10, 2)) STD_PROC_TIME		"
				" , CAST(SUM(PROC_TIME) / double(COUNT(*)) as DECIMAL(10, 2)) PROC_TIME				"
				"	FROM TPSSMB0	WHERE 1=1															"
				;
			if (factory_div != "")
			{
				sqlstr += " AND FACTORY_DIV = @factory_div ";
			}
			if (shift_group != "")
			{
				sqlstr += " AND SHIFT_GROUP = @shift_group ";
			}
		/*	if (st_no != "")
			{
				sqlstr += " AND ST_NO = @st_no ";
			}*/

			if (station_id != "")
			{
				sqlstr += " AND STATION_ID = @station_id ";
			}

			if (cs_start_time.Trim() != "")
			{
				sqlstr += " AND PROD_TIME>=@cs_start_time ";
			}
			if (cs_end_time.Trim() != "")
			{
				sqlstr += " AND PROD_TIME<=@cs_end_time ";
			}
			sqlstr += " group by STATION_ID,PROD_TIME ";
			break;
		}

		Log::Trace("", __FUNCTION__, "...sqlstr=[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.Parameters.Set("shift_group", shift_group);
		/*cmd_inq.Parameters.Set("st_no", st_no);*/
		cmd_inq.Parameters.Set("station_id", station_id);
		cmd_inq.Parameters.Set("cs_start_time", cs_start_time);
		cmd_inq.Parameters.Set("cs_end_time", cs_end_time);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			CDataRow& row = bcls_ret->Tables[0].Rows.Add();
			row["PROD_TIME"] = cmd_inq.GetString(2);
			row["PRO_NUM"] = cmd_inq.GetDecimal(3);
			row["FH_NUM"] = cmd_inq.GetDecimal(4);
			row["STD_PROC_TIME"] = cmd_inq.GetDecimal(5);
			row["PROC_TIME"] = cmd_inq.GetDecimal(6);
			row["FH_PERCENT_D"] = (cmd_inq.GetDecimal(4).ToDouble() / cmd_inq.GetDecimal(3).ToDouble()) * 100;

			//当月炉数
			if (cmd_inq.GetString(2).Substring(0, 6) = cs_end_time.Substring(0, 6))
			{
				month_num = month_num + cmd_inq.GetDecimal(3);
				month_fh_num = month_fh_num + cmd_inq.GetDecimal(4);
			}

			row["MONTH_NUM"] = month_num;
			row["MONTH_FH_NUM"] = month_fh_num;
			row["FH_PERCENT_M"] = month_fh_num > 0 ? (month_fh_num.ToDouble() / month_num.ToDouble()) * 100 : 0;

		}
		cmd_inq.Close();



		Log::Trace("", __FUNCTION__, "==记录条数=[{0}]", bcls_ret->Tables[0].Rows.get_Count());


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)
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
