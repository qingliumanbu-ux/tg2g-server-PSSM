/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 处理号查询
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
BM2F_ENTERACE(mmsmap02_inq_proc)

int f_mmsmap02_inq_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("");
	CString v_station_id("");
	CString v_station_no("");
	CString v_date_time("");
	//CDecimal  cmd_flag = 0;
	int i = 0;
	CString table_end("");//实绩表后缀

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
		v_station_id = bcls_rec->Tables[0].Rows[0]["STATION_ID"].ToString().Trim();
		v_station_no = bcls_rec->Tables[0].Rows[0]["STATION_NO"].ToString().Trim();
		v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();

		if (v_station_id == "B") //转炉生产实绩
		{
			table_end = "21";
			v_date_time = " and PROD_DATE = '" + v_date_time + "' ";
			v_station_id = " and station_id = 'B'";
		}
		else if (v_station_id == "C") //连铸生产实绩
		{
			table_end = "31";
			v_date_time = " and PROD_DATE = '" + v_date_time + "' ";
			v_station_id = " and station_id = 'C'";
		}
		else if (v_station_id == "L") //LF生产实绩
		{
			table_end = "24";	
			v_date_time = " and PROD_TIME = '" + v_date_time + "' ";
			v_station_id = " and station_id = 'L'";
		}
		else if (v_station_id == "R") //RH生产实绩
		{
			table_end = "23";
		    v_date_time = " and PROD_DATE = '" + v_date_time + "' ";
			v_station_id = " and station_id = 'R'";
		}
		else if (v_station_id == "S") //脱硫生产实绩
		{
			table_end = "14";
			v_date_time = " and PROD_DATE = '" + v_date_time + "' ";
			v_station_id = "";
		}
		else 
		{
			table_end = "21";
			v_date_time = " and PROD_DATE = '" + v_date_time + "' ";
			v_station_id = " and station_id = 'B'";
		}
		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_station_id[{1}],v_station_no[{2}],v_date_time[{3}] =======  ", v_factory_div, v_station_id, v_station_no, v_date_time);
		Log::Trace("", __FUNCTION__, "table_end = [{0}]", table_end);

		//---------------------------------------------------
		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROC_NO");

		sqlstr = " select HEAT_NO,PROC_NO from tmmsm" + table_end + " where FACTORY_DIV = '" + v_factory_div + "' and STATION_NO = '" + v_station_no + "' " + v_date_time + v_station_id;
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.Parameters.Set("v_station_id", v_station_id);
		cmd_inq.Parameters.Set("v_station_no", v_station_no);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["HEAT_NO"] = cmd_inq.GetString(1);
			bcls_ret->Tables[0].Rows[i]["PROC_NO"] = cmd_inq.GetString(2);
			i++;
		}
		//打印块的数目
		//Log::Trace("", __FUNCTION__, "返回块中的数据[{0}]", bcls_rec->Tables["PROC_NO"].Rows.get_Count());
		Log::Trace("", __FUNCTION__, "====== 打印结束 =======  ");
		cmd_inq.Close();

		//先取默认值，后续sql取数据源
		//CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		//row["PROC_NO"] = "21300870";







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
