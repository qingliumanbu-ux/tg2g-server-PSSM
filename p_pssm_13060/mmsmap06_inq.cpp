/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一炼钢能耗信息查询
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
BM2F_ENTERACE(mmsmap06_inq)

int f_mmsmap06_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A1");
	CString v_stat_date("");
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
		v_stat_date = bcls_rec->Tables[0].Rows[0]["STAT_DATE"].ToString().Trim();


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_stat_date[{1}] =======  ", v_factory_div, v_stat_date);


		//---------------------------------------------------
		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT5");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT6");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT7");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM8");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT8");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM8");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT8");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM9");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT9");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM9");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT9");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM10");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT10");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM10");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT10");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT11");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT12");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT13");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_NUM_SUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MON_USE_UNIT14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_NUM14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOT_USE_UNIT14");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "OUTPUT_MON");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "OUTPUT_TOTAL");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CHARGE_NAME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STAT_NAME");

		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["MON_NUM_SUM1"] = 600;
		row["MON_USE_UNIT1"] = 600;
		row["TOT_NUM1"] = 600;
		row["TOT_USE_UNIT1"] = 600;
		row["MON_NUM_SUM2"] = 600;
		row["MON_USE_UNIT2"] = 600;
		row["TOT_NUM2"] = 600;
		row["TOT_USE_UNIT2"] = 600;
		row["MON_NUM_SUM3"] = 600;
		row["MON_USE_UNIT3"] = 600;
		row["TOT_NUM3"] = 600;
		row["TOT_USE_UNIT3"] = 600;
		row["MON_NUM_SUM4"] = 600;
		row["MON_USE_UNIT4"] = 600;
		row["TOT_NUM4"] = 600;
		row["TOT_USE_UNIT4"] = 600;
		row["MON_NUM_SUM5"] = 600;
		row["MON_USE_UNIT5"] = 600;
		row["TOT_NUM5"] = 600;
		row["TOT_USE_UNIT5"] = 600;
		row["MON_NUM_SUM6"] = 600;
		row["MON_USE_UNIT6"] = 600;
		row["TOT_NUM6"] = 600;
		row["TOT_USE_UNIT6"] = 600;
		row["MON_NUM_SUM7"] = 600;
		row["MON_USE_UNIT7"] = 600;
		row["TOT_NUM7"] = 600;
		row["TOT_USE_UNIT7"] = 600;
		row["MON_NUM_SUM8"] = 600;
		row["MON_USE_UNIT8"] = 600;
		row["TOT_NUM8"] = 600;
		row["TOT_USE_UNIT8"] = 600;
		row["MON_NUM_SUM9"] = 600;
		row["MON_USE_UNIT9"] = 600;
		row["TOT_NUM9"] = 600;
		row["TOT_USE_UNIT9"] = 600;
		row["MON_NUM_SUM10"] = 600;
		row["MON_USE_UNIT10"] = 600;
		row["TOT_NUM10"] = 600;
		row["TOT_USE_UNIT10"] = 600;
		row["MON_NUM_SUM11"] = 600;
		row["MON_USE_UNIT11"] = 600;
		row["TOT_NUM11"] = 600;
		row["TOT_USE_UNIT11"] = 600;
		row["MON_NUM_SUM12"] = 600;
		row["MON_USE_UNIT12"] = 600;
		row["TOT_NUM12"] = 600;
		row["TOT_USE_UNIT12"] = 600;
		row["MON_NUM_SUM13"] = 600;
		row["MON_USE_UNIT13"] = 600;
		row["TOT_NUM13"] = 600;
		row["TOT_USE_UNIT13"] = 600;
		row["MON_NUM_SUM14"] = 600;
		row["MON_USE_UNIT14"] = 600;
		row["TOT_NUM14"] = 600;
		row["TOT_USE_UNIT14"] = 600;
		row["OUTPUT_MON"] = 600;
		row["OUTPUT_TOTAL"] = 600;
		row["CHARGE_NAME"] = "何维祥";
		row["STAT_NAME"] = "卢熙";








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
