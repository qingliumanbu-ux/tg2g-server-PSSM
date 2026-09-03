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
BM2F_ENTERACE(mmsmap01_inq_l)

int f_mmsmap01_inq_l(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A1");
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

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_A1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_A1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_A1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_A1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_A2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_A2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_A2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_A2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_A3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_A3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_A3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_A3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_L1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_L1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_L1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_L1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_L2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_L2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_L2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_L2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_R1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_R1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_R1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_R1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_R2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "START_TEMP_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_R2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "END_TEMP_R2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_R2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_BACKLOG_R2");


		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();

		row["HEAT_NO_A1"] = "21100002";
		row["ST_NO_A1"] = "AJ20110A";
		row["LADLE_NO_A1"] = "10#";
		row["LADLE_LEVEL_A1"] = "A";
		row["START_TIME_A1"] = "20210828";
		row["START_TEMP_A1"] = 700;
		row["END_TIME_A1"] = "20210830";
		row["END_TEMP_A1"] = 600;
		row["STEEL_WT_A1"] = 300;
		row["LADLE_G_A1"] = "Y";
		row["BACKLOG_A1"] = "BLC";
		row["NEXT_BACKLOG_A1"] = "L2";

		row["HEAT_NO_A2"] = "21100002";
		row["ST_NO_A2"] = "AJ20110A";
		row["LADLE_NO_A2"] = "10#";
		row["LADLE_LEVEL_A2"] = "A";
		row["START_TIME_A2"] = "20210828";
		row["START_TEMP_A2"] = 700;
		row["END_TIME_A2"] = "20210830";
		row["END_TEMP_A2"] = 600;
		row["STEEL_WT_A2"] = 300;
		row["LADLE_G_A2"] = "Y";
		row["BACKLOG_A2"] = "BLC";
		row["NEXT_BACKLOG_A2"] = "L2";

		row["HEAT_NO_A3"] = "21100002";
		row["ST_NO_A3"] = "AJ20110A";
		row["LADLE_NO_A3"] = "10#";
		row["LADLE_LEVEL_A3"] = "A";
		row["START_TIME_A3"] = "20210828";
		row["START_TEMP_A3"] = 700;
		row["END_TIME_A3"] = "20210830";
		row["END_TEMP_A3"] = 600;
		row["STEEL_WT_A3"] = 300;
		row["LADLE_G_A3"] = "Y";
		row["BACKLOG_A3"] = "BLC";
		row["NEXT_BACKLOG_A3"] = "L2";

		row["HEAT_NO_L1"] = "21100002";
		row["ST_NO_L1"] = "AJ20110A";
		row["LADLE_NO_L1"] = "10#";
		row["LADLE_LEVEL_L1"] = "A";
		row["START_TIME_L1"] = "20210828";
		row["START_TEMP_L1"] = 700;
		row["END_TIME_L1"] = "20210830";
		row["END_TEMP_L1"] = 600;
		row["STEEL_WT_L1"] = 300;
		row["LADLE_G_L1"] = "Y";
		row["BACKLOG_L1"] = "BLC";
		row["NEXT_BACKLOG_L1"] = "L2";

		row["HEAT_NO_L2"] = "21100002";
		row["ST_NO_L2"] = "AJ20110A";
		row["LADLE_NO_L2"] = "10#";
		row["LADLE_LEVEL_L2"] = "A";
		row["START_TIME_L2"] = "20210828";
		row["START_TEMP_L2"] = 700;
		row["END_TIME_L2"] = "20210830";
		row["END_TEMP_L2"] = 600;
		row["STEEL_WT_L2"] = 300;
		row["LADLE_G_L2"] = "Y";
		row["BACKLOG_L2"] = "BLC";
		row["NEXT_BACKLOG_L2"] = "L2";

		row["HEAT_NO_R1"] = "21100002";
		row["ST_NO_R1"] = "AJ20110A";
		row["LADLE_NO_R1"] = "10#";
		row["LADLE_LEVEL_R1"] = "A";
		row["START_TIME_R1"] = "20210828";
		row["START_TEMP_R1"] = 700;
		row["END_TIME_R1"] = "20210830";
		row["END_TEMP_R1"] = 600;
		row["STEEL_WT_R1"] = 300;
		row["LADLE_G_R1"] = "Y";
		row["BACKLOG_R1"] = "BLC";
		row["NEXT_BACKLOG_R1"] = "R1";

		row["HEAT_NO_R2"] = "21100002";
		row["ST_NO_R2"] = "AJ20110A";
		row["LADLE_NO_R2"] = "10#";
		row["LADLE_LEVEL_R2"] = "A";
		row["START_TIME_R2"] = "20210828";
		row["START_TEMP_R2"] = 700;
		row["END_TIME_R2"] = "20210830";
		row["END_TEMP_R2"] = 600;
		row["STEEL_WT_R2"] = 300;
		row["LADLE_G_R2"] = "Y";
		row["BACKLOG_R2"] = "BLC";
		row["NEXT_BACKLOG_R2"] = "R2";


	


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
