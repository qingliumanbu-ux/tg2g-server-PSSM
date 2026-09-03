/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 二炼钢方坯日发运信息查询
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
BM2F_ENTERACE(mmlgap04_inq)

int f_mmlgap04_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A2");
	CString v_date_time = "";
	CString v_date_time_1 = "";
	CString v_date_time_31 = "";

	CDecimal v_day_hot_num12 = 0;
	CDecimal v_total_hot_num12 = 0;
	CDecimal v_day_cold_num12 = 0;
	CDecimal v_total_cold_num12 = 0;
	CDecimal v_day_sum12 = 0;
	CDecimal v_total_sum12 = 0;

	CDecimal v_day_hot_num11 = 0;
	CDecimal v_total_hot_num11 = 0;
	CDecimal v_day_cold_num11 = 0;
	CDecimal v_total_cold_num11 = 0;
	CDecimal v_day_sum11 = 0;
	CDecimal v_total_sum11 = 0;

	CDecimal v_day_hot_num13 = 0;
	CDecimal v_total_hot_num13 = 0;
	CDecimal v_day_cold_num13 = 0;
	CDecimal v_total_cold_num13 = 0;
	CDecimal v_day_sum13 = 0;
	CDecimal v_total_sum13 = 0;

	CDecimal v_day_hot_num20 = 0;
	CDecimal v_total_hot_num20 = 0;
	CDecimal v_day_cold_num20 = 0;
	CDecimal v_total_cold_num20 = 0;
	CDecimal v_day_sum20 = 0;
	CDecimal v_total_sum20 = 0;

	CDecimal v_day_hot_sum = 0;
	CDecimal v_total_hot_sum = 0;
	CDecimal v_day_cold_sum = 0;
	CDecimal v_total_cold_sum = 0;
	CDecimal v_day_sum = 0;
	CDecimal v_total_sum = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sql("");
	CString sqlwhere("");
	CString code_class("");

	/* 实体类定义 */
	CModel tpssm10("TPSSM10");


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		//v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();
		v_date_time_1 = CDateTime::Parse(v_date_time + "000000").AddMinutes(-1).ToString("yyyyMMdd");

		v_date_time_31 = CDateTime::Parse(CDateTime::Now().AddDays(-1).ToString("yyyyMM") + "01000000").AddMinutes(-1).ToString("yyyyMMdd");

		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_date_time[{1}] =======  ", v_factory_div, v_date_time);

		//---------------------------------------------------
		//设置返回块参数
		//普棒产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM12");

		//双高棒产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM11");

		//线材产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM13");

		//外卖产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM20");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM20");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM20");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM20");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM20");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM20");

		//汇总
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_SUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_SUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_SUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_SUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr = " SELECT SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('C11','C12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM12, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('D11','D12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM11, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('E11','E12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM13, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('C11','C12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM12, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('D11','D12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM11, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('E11','E12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM13, "
				"       SUM(DECODE(STOCK_OPER_ORDER, '2E', T.MAT_ACT_WT, 0)) AS DAY_SUM20 "
				"  FROM TWMA4 T "
				" WHERE T.STOCK_OPER_ORDER LIKE '2%' "
				"   AND T.EVENT_TIME >= @v_date_time_1 || '210000' "
				"   AND T.EVENT_TIME < @v_date_time || '210000' ";

			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_date_time_1", v_date_time_1);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);

		cmd_inq.ExecuteReader();

		if (cmd_inq.Read())
		{
			v_day_hot_num12 = cmd_inq.GetDecimal(1);
			v_day_hot_num11 = cmd_inq.GetDecimal(2);
			v_day_hot_num13 = cmd_inq.GetDecimal(3);
			v_day_cold_num12 = cmd_inq.GetDecimal(4);
			v_day_cold_num11 = cmd_inq.GetDecimal(5);
			v_day_cold_num13 = cmd_inq.GetDecimal(6);
			v_day_sum20 = cmd_inq.GetDecimal(7);

			v_day_sum12 = v_day_hot_num12 + v_day_cold_num12;
			v_day_sum11 = v_day_hot_num11 + v_day_cold_num11;
			v_day_sum13 = v_day_hot_num13 + v_day_cold_num13;

			v_day_hot_sum = v_day_hot_num12 + v_day_hot_num11 + v_day_hot_num13;
			v_day_cold_sum = v_day_cold_num12 + v_day_cold_num11 + v_day_cold_num13;
			v_day_sum = v_day_sum12 + v_day_sum11 + v_day_sum13 + v_day_sum20;
		}
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr = " SELECT SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('C11','C12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM12, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('D11','D12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM11, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('E11','E12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM13, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('C11','C12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM12, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('D11','D12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM11, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('E11','E12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM13, "
				"       SUM(DECODE(STOCK_OPER_ORDER, '2E', T.MAT_ACT_WT, 0)) AS DAY_SUM20 "
				"  FROM TWMA4 T "
				" WHERE T.STOCK_OPER_ORDER LIKE '2%' "
				"   AND T.EVENT_TIME >= @v_date_time_31 || '210000' "
				"   AND T.EVENT_TIME < @v_date_time || '210000' ";

			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_date_time_31", v_date_time_31);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);

		cmd_inq.ExecuteReader();

		if (cmd_inq.Read())
		{
			v_total_hot_num12 = cmd_inq.GetDecimal(1);
			v_total_hot_num11 = cmd_inq.GetDecimal(2);
			v_total_hot_num13 = cmd_inq.GetDecimal(3);
			v_total_cold_num12 = cmd_inq.GetDecimal(4);
			v_total_cold_num11 = cmd_inq.GetDecimal(5);
			v_total_cold_num13 = cmd_inq.GetDecimal(6);
			v_total_sum20 = cmd_inq.GetDecimal(7);

			v_total_sum12 = v_total_hot_num12 + v_total_cold_num12;
			v_total_sum11 = v_total_hot_num11 + v_total_cold_num11;
			v_total_sum13 = v_total_hot_num13 + v_total_cold_num13;

			v_total_hot_sum = v_total_hot_num12 + v_total_hot_num11 + v_total_hot_num13;
			v_total_cold_sum = v_total_cold_num12 + v_total_cold_num11 + v_total_cold_num13;
			v_total_sum = v_total_sum12 + v_total_sum11 + v_total_sum13 + v_total_sum20;
		}
		cmd_inq.Close();

		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["DAY_HOT_NUM12"] = v_day_hot_num12;
		row["TOTAL_HOT_NUM12"] = v_total_hot_num12;
		row["DAY_COLD_NUM12"] = v_day_cold_num12;
		row["TOTAL_COLD_NUM12"] = v_total_cold_num12;
		row["DAY_SUM12"] = v_day_sum12;
		row["TOTAL_SUM12"] = v_total_sum12;

		row["DAY_HOT_NUM11"] = v_day_hot_num11;
		row["TOTAL_HOT_NUM11"] = v_total_hot_num11;
		row["DAY_COLD_NUM11"] = v_day_cold_num11;
		row["TOTAL_COLD_NUM11"] = v_total_cold_num11;
		row["DAY_SUM11"] = v_day_sum11;
		row["TOTAL_SUM11"] = v_total_sum11;

		row["DAY_HOT_NUM13"] = v_day_hot_num13;
		row["TOTAL_HOT_NUM13"] = v_total_hot_num13;
		row["DAY_COLD_NUM13"] = v_day_cold_num13;
		row["TOTAL_COLD_NUM13"] = v_total_cold_num13;
		row["DAY_SUM13"] = v_day_sum13;
		row["TOTAL_SUM13"] = v_total_sum13;

		row["DAY_HOT_NUM20"] = v_day_hot_num20;
		row["TOTAL_HOT_NUM20"] = v_total_hot_num20;
		row["DAY_COLD_NUM20"] = v_day_cold_num20;
		row["TOTAL_COLD_NUM20"] = v_total_cold_num20;
		row["DAY_SUM20"] = v_day_sum20;
		row["TOTAL_SUM20"] = v_total_sum20;

		row["DAY_HOT_SUM"] = v_day_hot_sum;
		row["TOTAL_HOT_SUM"] = v_total_hot_sum;
		row["DAY_COLD_SUM"] = v_day_cold_sum;
		row["TOTAL_COLD_SUM"] = v_total_cold_sum;
		row["DAY_SUM"] = v_day_sum;
		row["TOTAL_SUM"] = v_total_sum;

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
