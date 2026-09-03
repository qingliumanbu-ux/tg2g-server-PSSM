/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      zhengqiangqiang
Version:     1.0
Date:        2020/7/6 16:08:19
Description: 一系列生产计划查询
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
BM2F_ENTERACE(mmlgap01_inq_l)

int f_mmlgap01_inq_l(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A2");
	CString v_date_time("");
	CDecimal  cmd_flag = 0;

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


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_date_time[{1}] =======  ", v_factory_div, v_date_time);


		//---------------------------------------------------
		//设置返回块参数
		//bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_WT_C1");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_A5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_A5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_A5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_A5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_A5");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_A6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_A6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_A6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_A6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_A6");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_A7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_A7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_A7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_A7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_A7");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_L3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_L3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LAST_BACKLOG_L3");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_L4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_L4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LAST_BACKLOG_L4");



		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();

		row["HEAT_NO_A5"] = "21100002";
		row["ST_NO_A5"] = "AJ20110A";
		row["LADLE_NO_A5"] = "10#";
		row["LADLE_LEVEL_A5"] = "A";
		row["START_TIME_A5"] = "20210828";
		row["START_TEMP_A5"] = 700;
		row["END_TIME_A5"] = "20210830";
		row["END_TEMP_A5"] = 600;
		row["STEEL_WT_A5"] = 300;
		row["LADLE_G_A5"] = "Y";
		row["BACKLOG_A5"] = "BLC";
		row["NEXT_BACKLOG_A5"] = "L4";

		row["HEAT_NO_A6"] = "21100002";
		row["ST_NO_A6"] = "AJ20110A";
		row["LADLE_NO_A6"] = "10#";
		row["LADLE_LEVEL_A6"] = "A";
		row["START_TIME_A6"] = "20210828";
		row["START_TEMP_A6"] = 700;
		row["END_TIME_A6"] = "20210830";
		row["END_TEMP_A6"] = 600;
		row["STEEL_WT_A6"] = 300;
		row["LADLE_G_A6"] = "Y";
		row["BACKLOG_A6"] = "BLC";
		row["NEXT_BACKLOG_A6"] = "L4";

		row["HEAT_NO_A7"] = "21100002";
		row["ST_NO_A7"] = "AJ20110A";
		row["LADLE_NO_A7"] = "10#";
		row["LADLE_LEVEL_A7"] = "A";
		row["START_TIME_A7"] = "20210828";
		row["START_TEMP_A7"] = 700;
		row["END_TIME_A7"] = "20210830";
		row["END_TEMP_A7"] = 600;
		row["STEEL_WT_A7"] = 300;
		row["LADLE_G_A7"] = "Y";
		row["BACKLOG_A7"] = "BLC";
		row["NEXT_BACKLOG_A7"] = "L4";

		row["HEAT_NO_L3"] = "21100002";
		row["ST_NO_L3"] = "AJ20110A";
		row["LADLE_NO_L3"] = "10#";
		row["LADLE_LEVEL_L3"] = "A";
		row["START_TIME_L3"] = "20210828";
		row["END_TIME_L3"] = "20210830";
		row["STEEL_WT_L3"] = 300;
		row["BACKLOG_L3"] = "BLC";
		row["NEXT_BACKLOG_L3"] = "L4";
		row["LAST_BACKLOG_L3"] = "L4";

		row["HEAT_NO_L4"] = "21100002";
		row["ST_NO_L4"] = "AJ20110A";
		row["LADLE_NO_L4"] = "10#";
		row["LADLE_LEVEL_L4"] = "A";
		row["START_TIME_L4"] = "20210828";
		row["END_TIME_L4"] = "20210830";
		row["STEEL_WT_L4"] = 300;
		row["BACKLOG_L4"] = "BLC";
		row["LAST_BACKLOG_L4"] = "L4";


		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		//	sqlstr =
		//		" select * from tep0002 where code_class = @code_class "
		//		//" AND B.STOCK_NO = @stock_no "
		//		;
		//	
		//	break;
		//case DB_KIND_MSSQL:			// MS SQL Server数据库
		//	break;
		//case DB_KIND_ORACLE:		// Oracle 数据库
		//	break;
		//}
		//
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("code_class", code_class);
		//
		//Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		//cmd_inq.ExecuteQuery(bcls_ret->Tables[0], 0, -1);  //0,-1：非翻页查询
		//cmd_inq.Close();

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
