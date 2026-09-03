/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      zhengqiangqiang
Version:     1.0
Date:        2020/7/6 16:08:19
Description: 二系列连铸生产实绩查询
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
BM2F_ENTERACE(mmlgap02_inq_c)

int f_mmlgap02_inq_c(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	//CString v_factory_div("A1");
	CString v_date("");
	CString v_station_no("");
	CString v_proc_no("");
	//CDecimal  cmd_flag = 0;

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
		v_date = bcls_rec->Tables[0].Rows[0]["DATE"].ToString().Trim();
		v_station_no = bcls_rec->Tables[0].Rows[0]["STATION_NO"].ToString().Trim();
		v_proc_no = bcls_rec->Tables[0].Rows[0]["PROC_NO"].ToString().Trim();


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_date[{0}],v_station_no[{1}],v_proc_no[{2}] =======  ", v_date, v_station_no, v_proc_no);


		//---------------------------------------------------
		//设置返回块参数
		//bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C4");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_LEVEL_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_G_C");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_NO_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AVG_MID_TEMP_C");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AVG_N1_SPEED_C");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AVG_N2_SPEED_C");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CAST_CYCLE_C");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_C");



		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["HEAT_NO_C"] = "21100001";
		row["ST_NO_C"] = "A1";
		row["LADLE_NO_C"] = "211005601";
		row["LADLE_LEVEL_C"] = "1级";
		row["LADLE_G_C"] = "否";
		row["STEEL_WT_C"] = 200;
		row["CAST_NO_C"] = "15644";
		row["START_TIME_C"] = "20210826";
		row["END_TIME_C"] = "20210826";
		row["AVG_MID_TEMP_C"] = 150;
		row["AVG_N1_SPEED_C"] = 36.5;
		row["AVG_N2_SPEED_C"] = 40.5;
		row["CAST_CYCLE_C"] = 5;
		row["PROD_WT_C"] = 2000;






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
