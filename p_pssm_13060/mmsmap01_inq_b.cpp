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
BM2F_ENTERACE(mmsmap01_inq_b)

int f_mmsmap01_inq_b(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_B1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_B1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_B1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_B1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRON_IN_B1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_B1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_B1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "OUT_STEEL_TEMP_B1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "ALLOY_DROP_B1");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_B2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_B2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_B2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_B2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRON_IN_B2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_B2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_B2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "OUT_STEEL_TEMP_B2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "ALLOY_DROP_B2");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_B3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_B3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_B3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_B3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRON_IN_B3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_B3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_B3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "OUT_STEEL_TEMP_B3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "ALLOY_DROP_B3");


		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["HEAT_NO_B1"] = "2110002";
		row["ST_NO_B1"] = "ALTU400";
		row["LADLE_NO_B1"] = "1#";
		row["LADLE_LEVEL_B1"] = "B";
		row["IRON_IN_B1"] = 20000;
		row["START_TIME_B1"] = "20210825";
		row["END_TIME_B1"] = "20210826";
		row["OUT_STEEL_TEMP_B1"] = 799;
		row["ALLOY_DROP_B1"] = 20000;

		row["HEAT_NO_B2"] = "2120002";
		row["ST_NO_B2"] = "ALTU300";
		row["LADLE_NO_B2"] = "1#";
		row["LADLE_LEVEL_B2"] = "B";
		row["IRON_IN_B2"] = 20000;
		row["START_TIME_B2"] = "20210825";
		row["END_TIME_B2"] = "20210826";
		row["OUT_STEEL_TEMP_B2"] = 799;
		row["ALLOY_DROP_B2"] = 20000;

		row["HEAT_NO_B3"] = "2130002";
		row["ST_NO_B3"] = "ALTU500";
		row["LADLE_NO_B3"] = "1#";
		row["LADLE_LEVEL_B3"] = "B";
		row["IRON_IN_B3"] = 20000;
		row["START_TIME_B3"] = "20210825";
		row["END_TIME_B3"] = "20210826";
		row["OUT_STEEL_TEMP_B3"] = 799;
		row["ALLOY_DROP_B3"] = 20000;





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
