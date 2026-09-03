/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一炼钢板坯日发运信息查询
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
BM2F_ENTERACE(mmsmap04_inq)

int f_mmsmap04_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A1");
	CString v_date_time = "";
	CString v_date_time_1 = "";
	CString v_date_time_31 = "";

	CDecimal v_day_hot_num15 = 0;
	CDecimal v_total_hot_num15 = 0;
	CDecimal v_day_cold_num15 = 0;
	CDecimal v_total_cold_num15 = 0;
	CDecimal v_day_sum15 = 0;
	CDecimal v_total_sum15 = 0;

	CDecimal v_day_hot_num16 = 0;
	CDecimal v_total_hot_num16 = 0;
	CDecimal v_day_cold_num16 = 0;
	CDecimal v_total_cold_num16 = 0;
	CDecimal v_day_sum16 = 0;
	CDecimal v_total_sum16 = 0;

	CDecimal v_day_hot_num14 = 0;
	CDecimal v_total_hot_num14 = 0;
	CDecimal v_day_cold_num14 = 0;
	CDecimal v_total_cold_num14 = 0;
	CDecimal v_day_sum14 = 0;
	CDecimal v_total_sum14 = 0;

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
		//4100mm产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM15");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM15");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM15");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM15");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM15");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM15");

		//2700mm产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM16");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM16");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM16");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM16");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM16");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM16");

		//1780mm产线
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_HOT_NUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_HOT_NUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_COLD_NUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_COLD_NUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DAY_SUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_SUM14");

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
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('H11','H12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM15, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('G11','G12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM16, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('F11','F12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM14, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('H11','H12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM15, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('G11','G12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM16, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('F11','F12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM14, "
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
			v_day_hot_num15 = cmd_inq.GetDecimal(1);
			v_day_hot_num16 = cmd_inq.GetDecimal(2);
			v_day_hot_num14 = cmd_inq.GetDecimal(3);
			v_day_cold_num15 = cmd_inq.GetDecimal(4);
			v_day_cold_num16 = cmd_inq.GetDecimal(5);
			v_day_cold_num14 = cmd_inq.GetDecimal(6);
			v_day_sum20 = cmd_inq.GetDecimal(7);

			v_day_sum15 = v_day_hot_num15 + v_day_cold_num15;
			v_day_sum16 = v_day_hot_num16 + v_day_cold_num16;
			v_day_sum14 = v_day_hot_num14 + v_day_cold_num14;

			v_day_hot_sum = v_day_hot_num15 + v_day_hot_num16 + v_day_hot_num14;
			v_day_cold_sum = v_day_cold_num15 + v_day_cold_num16 + v_day_cold_num14;
			v_day_sum = v_day_sum15 + v_day_sum16 + v_day_sum14 + v_day_sum20;
		}
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr = " SELECT SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('H11','H12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM15, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('G11','G12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM16, "
				"       SUM(CASE "
				"             WHEN STOCK_OPER_ORDER = '2X' AND STOCK_NO_TO IN ('F11','F12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_HOT_NUM14, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('H11','H12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM15, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('G11','G12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM16, "
				"       SUM(CASE "
				"             WHEN (STOCK_OPER_ORDER = '2G') AND STOCK_NO_TO IN ('F11','F12') THEN "
				"              T.MAT_ACT_WT "
				"             ELSE "
				"              0 "
				"           END) AS DAY_COLD_NUM14, "
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
			v_total_hot_num15 = cmd_inq.GetDecimal(1);
			v_total_hot_num16 = cmd_inq.GetDecimal(2);
			v_total_hot_num14 = cmd_inq.GetDecimal(3);
			v_total_cold_num15 = cmd_inq.GetDecimal(4);
			v_total_cold_num16 = cmd_inq.GetDecimal(5);
			v_total_cold_num14 = cmd_inq.GetDecimal(6);
			v_total_sum20 = cmd_inq.GetDecimal(7);

			v_total_sum15 = v_total_hot_num15 + v_total_cold_num15;
			v_total_sum16 = v_total_hot_num16 + v_total_cold_num16;
			v_total_sum14 = v_total_hot_num14 + v_total_cold_num14;

			v_total_hot_sum = v_total_hot_num15 + v_total_hot_num16 + v_total_hot_num14;
			v_total_cold_sum = v_total_cold_num15 + v_total_cold_num16 + v_total_cold_num14;
			v_total_sum = v_total_sum15 + v_total_sum16 + v_total_sum14 + v_total_sum20;
		}
		cmd_inq.Close();

		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["DAY_HOT_NUM15"] = v_day_hot_num15;
		row["TOTAL_HOT_NUM15"] = v_total_hot_num15;
		row["DAY_COLD_NUM15"] = v_day_cold_num15;
		row["TOTAL_COLD_NUM15"] = v_total_cold_num15;
		row["DAY_SUM15"] = v_day_sum15;
		row["TOTAL_SUM15"] = v_total_sum15;

		row["DAY_HOT_NUM16"] = v_day_hot_num16;
		row["TOTAL_HOT_NUM16"] = v_total_hot_num16;
		row["DAY_COLD_NUM16"] = v_day_cold_num16;
		row["TOTAL_COLD_NUM16"] = v_total_cold_num16;
		row["DAY_SUM16"] = v_day_sum16;
		row["TOTAL_SUM16"] = v_total_sum16;

		row["DAY_HOT_NUM14"] = v_day_hot_num14;
		row["TOTAL_HOT_NUM14"] = v_total_hot_num14;
		row["DAY_COLD_NUM14"] = v_day_cold_num14;
		row["TOTAL_COLD_NUM14"] = v_total_cold_num14;
		row["DAY_SUM14"] = v_day_sum14;
		row["TOTAL_SUM14"] = v_total_sum14;

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
