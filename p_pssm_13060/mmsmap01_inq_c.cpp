/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      zhengqiangqiang
Version:     1.0
Date:        2020/7/6 16:08:19
Description: 一系列生产状态查询
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
BM2F_ENTERACE(mmsmap01_inq_c)

int f_mmsmap01_inq_c(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C1");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C2");


		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C3");


		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C4");




		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["HEAT_NO_C1"] = "21400001";
		row["ST_NO_C1"] = "AI170300";
		row["LADLE_NO_C1"] = "10#";
		row["STEEL_WT_C1"] = 99.8;
		row["CAST_LOT_NO_C1"] = "120000";
		row["CAST_DIV_NO_C1"] = "111";
		row["SPEC_C1"] = "200*250";
		row["START_TIME_C1"] = "20210827";
		row["END_TIME_C1"] = "20210828";
		row["MID_TEMP_C1"] = 999;
		row["N1_SPEED_C1"] = 1;
		row["N2_SPEED_C1"] = 1;
		row["NEXT_LOAD_TIME_C1"] = "20210712";
		row["NEXT_LOAD_TEMP_C1"] = 900;

		row["HEAT_NO_C2"] = "21300001";
		row["ST_NO_C2"] = "AI170300";
		row["LADLE_NO_C2"] = "10#";
		row["STEEL_WT_C2"] = 99.8;
		row["CAST_LOT_NO_C2"] = "120000";
		row["CAST_DIV_NO_C2"] = "111";
		row["SPEC_C2"] = "200*250";
		row["START_TIME_C2"] = "20210827";
		row["END_TIME_C2"] = "20210828";
		row["MID_TEMP_C2"] = 999;
		row["N1_SPEED_C2"] = 1;
		row["N2_SPEED_C2"] = 1;
		row["NEXT_LOAD_TIME_C2"] = "20210712";
		row["NEXT_LOAD_TEMP_C2"] = 900;


		row["HEAT_NO_C3"] = "21200001";
		row["ST_NO_C3"] = "AI170300";
		row["LADLE_NO_C3"] = "10#";
		row["STEEL_WT_C3"] = 99.8;
		row["CAST_LOT_NO_C3"] = "120000";
		row["CAST_DIV_NO_C3"] = "111";
		row["SPEC_C3"] = "200*250";
		row["START_TIME_C3"] = "20210825";
		row["END_TIME_C3"] = "20210828";
		row["MID_TEMP_C3"] = 999;
		row["N1_SPEED_C3"] = 1;
		row["N2_SPEED_C3"] = 1;
		row["NEXT_LOAD_TIME_C3"] = "20210712";
		row["NEXT_LOAD_TEMP_C3"] = 900;

		row["HEAT_NO_C4"] = "211ww00001";
		row["ST_NO_C4"] = "AI170300";
		row["LADLE_NO_C4"] = "10#";
		row["STEEL_WT_C4"] = 99.8;
		row["CAST_LOT_NO_C4"] = "120000";
		row["CAST_DIV_NO_C4"] = "111";
		row["SPEC_C4"] = "200*250";
		row["START_TIME_C4"] = "20210825";
		row["END_TIME_C4"] = "20210828";
		row["MID_TEMP_C4"] = 999;
		row["N1_SPEED_C4"] = 1;
		row["N2_SPEED_C4"] = 1;
		row["NEXT_LOAD_TIME_C4"] = "20210712";
		row["NEXT_LOAD_TEMP_C4"] = 900;





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
