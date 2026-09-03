/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一二系列精炼生产日报
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
BM2F_ENTERACE(mmsmap10_inq)

int f_mmsmap10_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A1,A2");
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
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_stat_date = bcls_rec->Tables[0].Rows[0]["STAT_DATE"].ToString().Trim();


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_stat_date[{1}] =======  ", v_factory_div, v_stat_date);


		//---------------------------------------------------
		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRONTURN_DAY_TIMEA1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRONTURN_DAY_TIMEA2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRONTURN_TOTAL_TIMEA1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "IRONTURN_TOTAL_TIMEA2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_DAY_PACKA1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_DAY_PACKA2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_TOTAL_PACKA1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_TOTAL_PACKA2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_DAY_NUMA1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_DAY_NUMA2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_TOTAL_NUMA1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CASTIRON_TOTAL_NUMA2");


		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["IRONTURN_DAY_TIMEA1"] = 80;
		row["IRONTURN_DAY_TIMEA2"] = 80;
		row["IRONTURN_TOTAL_TIMEA1"] = 80;
		row["IRONTURN_TOTAL_TIMEA2"] = 80;
		row["CASTIRON_DAY_PACKA1"] = 80;
		row["CASTIRON_DAY_PACKA2"] = 80;
		row["CASTIRON_TOTAL_PACKA1"] = 80;
		row["CASTIRON_TOTAL_PACKA2"] = 80;
		row["CASTIRON_DAY_NUMA1"] = 80;
		row["CASTIRON_DAY_NUMA2"] = 80;
		row["CASTIRON_TOTAL_NUMA1"] = 80;
		row["CASTIRON_TOTAL_NUMA2"] = 80;



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
