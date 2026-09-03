/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      zhengqiangqiang
Version:     1.0
Date:        2020/7/6 16:08:19
Description: 一系列脱硫生产实绩查询
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
BM2F_ENTERACE(mmsmap02_inq_s)

int f_mmsmap02_inq_s(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	CModel tmmsm14("TMMSM14");


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

		//if (v_date != "")
		//{
		//	v_date = " and PROD_DATE = '" + v_date + "' ";
		//}


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_date[{0}],v_station_no[{1}],v_proc_no[{2}] =======  ", v_date, v_station_no, v_proc_no);


		//---------------------------------------------------
		//设置返回块参数
		//bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C4");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROD_TIME_S");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRCO_NO_S");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "IRON_LADLE_S");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "IRON_NO_S");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TEMP_IN_S");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TEMP_OUT_S");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "START_TIME_S");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "END_TIME_S");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROC_TIME_S");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PRT_S");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PRE_IN_S");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PRE_OUT_S");
		

		//先取默认值，后续sql取数据源
		//CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		//row["PROD_TIME_S"] = "21100001";
		//row["PRCO_NO_S"] = "12345";
		//row["IRON_LADLE_S"] = "666";
		//row["IRON_NO_S"] = "20210826";
		//row["TEMP_IN_S"] = 800;
		//row["TEMP_OUT_S"] = 30;
		//row["START_TIME_S"] = "20210827";
		//row["END_TIME_S"] = "20210828";
		//row["PROC_TIME_S"] = "20210829";
		//row["PRT_S"] = 1000;
		//row["PRE_IN_S"] = 12;
		//row["PRE_OUT_S"] = 13;




		sqlstr = "select PROD_DATE AS PROD_TIME_S,PROC_NO AS PRCO_NO_S,IRON_LADLE_NO AS IRON_LADLE_S,"
			"IRON_NO AS IRON_NO_S, S_PREV_TEMP AS TEMP_IN_S, S_REP_TEMP AS TEMP_OUT_S,"
			"LADLE_ARRIVE_TIME AS START_TIME_S, LADLE_LEAVE_TIME AS END_TIME_S, DES_DURATION AS PROC_TIME_S,"
			"TS_N2_FLOW AS PRT_S, ENTRY_S AS PRE_IN_S, AFT_S AS PRE_OUT_S from TMMSM14 where PROD_DATE = '" + v_date + "' AND STATION_NO = '" + v_station_no + "' AND PROC_NO = '" + v_proc_no + "'";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_date", v_date);
		cmd_inq.Parameters.Set("v_station_no", v_station_no);
		cmd_inq.Parameters.Set("v_proc_no", v_proc_no);

		Log::Trace("", __FUNCTION__, "SQL[{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		//打印块的数目
		//Log::Trace("", __FUNCTION__, "返回块中的数据[{0}]", bcls_ret->Tables[0].Rows.get_Count());
		Log::Trace("", __FUNCTION__, "====== 打印结束 =======  ");

		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		//	sqlstr =
		//		"select PROD_DATE AS PROD_TIME_S,PROC_NO AS PRCO_NO_S,IRON_LADLE_NO AS IRON_LADLE_S,"
		//	"IRON_NO AS IRON_NO_S, S_PREV_TEMP AS TEMP_IN_S, S_REP_TEMP AS TEMP_OUT_S,"
		//	"LADLE_ARRIVE_TIME AS START_TIME_S, LADLE_LEAVE_TIME AS END_TIME_S, DES_DURATION AS PROC_TIME_S,"
		//	"TS_N2_FLOW AS PRT_S, ENTRY_S AS PRE_IN_S, AFT_S AS PRE_OUT_S from TMMSM14 where PROD_DATE = @v_date "
		//		" AND STATION_NO = @v_station_no and PROC_NO =@v_proc_no"
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
		//cmd_inq.Parameters.Set("v_date", v_date);
		//cmd_inq.Parameters.Set("v_station_no", v_station_no);
		//cmd_inq.Parameters.Set("v_proc_no", v_proc_no);
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
	cmd_inq.Close();
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}
