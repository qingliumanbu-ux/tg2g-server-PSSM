/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一炼钢板坯产量信息查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** 头文件部分 *****/

//#include "AppFunc.h"

/*<remark>=========================================================
/// <summary>
///  一系列生产计划查询
/// <para>
/// </para>
/// <para>数据库表：</para>
/// </summary>
/// <param name="">  </param>
/// <returns>返回参数：材料数据</returns>
===========================================================</remark>*/
// Service 入口
BM2F_ENTERACE(mmsmap05_mon_inq)

int f_mmsmap05_mon_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_date_time("");
	CString time_year("");
	CString time_year1("");
	//CDecimal  cmd_flag = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sqlstr1("");

	/* 实体类定义 */


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();
		time_year = v_date_time.Substring(0, 4);
		time_year1 = time_year + "0";
		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "v_date_time[{0}]", v_date_time);
		Log::Trace("", __FUNCTION__, "time_year[{0}]", time_year);
		bcls_ret->Tables.Add("A");
		bcls_ret->Tables.Add("B");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = "select  substr(START_TIME,1,6) as MONTH,sum(OUT_STEEL_WT) AS CONS_PROD_WT FROM (select case START_TIME WHEN  "
				"to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day "
				"AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN "
				"SUBSTRING(START_TIME, 1, 8)when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS START_TIME, "
				"OUT_STEEL_WT from TMMSM21 where  FACTORY_DIV = 'A1') where START_TIME like @TIME_YEAR || '%' group by substr(START_TIME, 1, 6)";

			sqlstr1 = "select substr(START_TIME, 1, 6) as MONTH, sum(SLAB_WT) AS SLAB_PROD_WT FROM(select case START_TIME WHEN  "
				"to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day "
				"AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN "
				"SUBSTRING(START_TIME, 1, 8) when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS START_TIME, SLAB_WT "
				"from(select a.SLAB_WT, a.STATION_NO, a.START_TIME from TMMSM33 a, TMMSM31 b where a.HEAT_NO = b.HEAT_NO and a.FACTORY_DIV = 'A1')) "
				"where START_TIME like @TIME_YEAR || '%' group by substr(START_TIME, 1, 6)";

			break;
		}
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr);
		cmd_inq.Parameters.Clear();
		cmd_inq.Parameters.Set("TIME_YEAR", time_year);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["A"]);
		cmd_inq.Close();
		cmd_inq.SetCommandText(sqlstr1);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["B"]);
		cmd_inq.Close();
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MONTH");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_PROD_WT");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CONS_PROD_WT");
		for (int i = 1; i <= 12; i++){
			bcls_ret->Tables[0].Rows.Add();
			if (i < 10){
				string str = to_string(i);
				bcls_ret->Tables[0].Rows[i - 1]["MONTH"] = time_year1 + str;
				bcls_ret->Tables[0].Rows[i - 1]["SLAB_PROD_WT"] = 0;
				bcls_ret->Tables[0].Rows[i - 1]["CONS_PROD_WT"] = 0;
			}
			else {
				string str = to_string(i);
				bcls_ret->Tables[0].Rows[i - 1]["MONTH"] = time_year + str;
				bcls_ret->Tables[0].Rows[i - 1]["SLAB_PROD_WT"] = 0;
				bcls_ret->Tables[0].Rows[i - 1]["CONS_PROD_WT"] = 0;
			}
		}
		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++){
			for (int j = 0; j < bcls_ret->Tables["A"].Rows.get_Count(); j++){
				if (bcls_ret->Tables["A"].Rows[j]["MONTH"].ToString() == bcls_ret->Tables[0].Rows[i]["MONTH"].ToString()){
					bcls_ret->Tables[0].Rows[i]["CONS_PROD_WT"] = bcls_ret->Tables["A"].Rows[j]["CONS_PROD_WT"].ToString();
				}
			}
		}
		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++){
			for (int j = 0; j < bcls_ret->Tables["B"].Rows.get_Count(); j++){
				if (bcls_ret->Tables["B"].Rows[j]["MONTH"].ToString() == bcls_ret->Tables[0].Rows[i]["MONTH"].ToString()){
					bcls_ret->Tables[0].Rows[i]["SLAB_PROD_WT"] = bcls_ret->Tables["B"].Rows[j]["SLAB_PROD_WT"].ToString();
				}
			}
		}
		bcls_ret->Tables["A"].Delete();
		bcls_ret->Tables["B"].Delete();

	}
	catch (CDbException& ex)					//捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)	//捕获应用错误
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

	return(doFlag);
}