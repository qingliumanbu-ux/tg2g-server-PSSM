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
BM2F_ENTERACE(mmlgap01_inq_c)

int f_mmlgap01_inq_c(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N3_SPEED_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N4_SPEED_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N5_SPEED_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N6_SPEED_C5");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C5");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N3_SPEED_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N4_SPEED_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N5_SPEED_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N6_SPEED_C6");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C6");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "STEEL_WT_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SPEC_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MID_TEMP_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N1_SPEED_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N2_SPEED_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N3_SPEED_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N4_SPEED_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N5_SPEED_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N6_SPEED_C7");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NEXT_LOAD_TIME_C7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NEXT_LOAD_TEMP_C7");






		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["HEAT_NO_C5"] = "21500001";
		row["ST_NO_C5"] = "AI170300";
		row["LADLE_NO_C5"] = "10#";
		row["STEEL_WT_C5"] = 99.8;
		row["CAST_LOT_NO_C5"] = "120000";
		row["CAST_DIV_NO_C5"] = "111";
		row["SPEC_C5"] = "200*250";
		row["START_TIME_C5"] = "20210506";
		row["END_TIME_C5"] = "20210507";
		row["MID_TEMP_C5"] = 999;
		row["N1_SPEED_C5"] = 1;
		row["N2_SPEED_C5"] = 1;
		row["N3_SPEED_C5"] = 1;
		row["N4_SPEED_C5"] = 1;
		row["N5_SPEED_C5"] = 1;
		row["N6_SPEED_C5"] = 1;
		row["NEXT_LOAD_TIME_C5"] = "20210712";
		row["NEXT_LOAD_TEMP_C5"] = 900;

		row["HEAT_NO_C6"] = "21600001";
		row["ST_NO_C6"] = "AI170300";
		row["LADLE_NO_C6"] = "10#";
		row["STEEL_WT_C6"] = 99.8;
		row["CAST_LOT_NO_C6"] = "120000";
		row["CAST_DIV_NO_C6"] = "111";
		row["SPEC_C6"] = "200*250";
		row["START_TIME_C6"] = "20210506";
		row["END_TIME_C6"] = "20210508";
		row["MID_TEMP_C6"] = 999;
		row["N1_SPEED_C6"] = 1;
		row["N2_SPEED_C6"] = 1;
		row["N3_SPEED_C6"] = 1;
		row["N4_SPEED_C6"] = 1;
		row["N5_SPEED_C6"] = 1;
		row["N6_SPEED_C6"] = 1;
		row["NEXT_LOAD_TIME_C6"] = "20210712";
		row["NEXT_LOAD_TEMP_C6"] = 900;


		row["HEAT_NO_C7"] = "21700001";
		row["ST_NO_C7"] = "AI170300";
		row["LADLE_NO_C7"] = "10#";
		row["STEEL_WT_C7"] = 99.8;
		row["CAST_LOT_NO_C7"] = "120000";
		row["CAST_DIV_NO_C7"] = "111";
		row["SPEC_C7"] = "200*250";
		row["START_TIME_C7"] = "20210506";
		row["END_TIME_C7"] = "20210509";
		row["MID_TEMP_C7"] = 999;
		row["N1_SPEED_C7"] = 1;
		row["N2_SPEED_C7"] = 1;
		row["N3_SPEED_C7"] = 1;
		row["N4_SPEED_C7"] = 1;
		row["N5_SPEED_C7"] = 1;
		row["N6_SPEED_C7"] = 1;
		row["NEXT_LOAD_TIME_C7"] = "20210712";
		row["NEXT_LOAD_TEMP_C7"] = 900;



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
