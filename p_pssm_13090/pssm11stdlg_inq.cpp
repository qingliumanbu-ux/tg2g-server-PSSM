/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-12-26 17:13:56
Version:  3.1.0
Description: 钢种变更对话框初始化查询
**************************************************/
//框架头文件
#include "stdafx.h"

//业务头文件


//函数申明

/*<remark>=========================================================
///<summary>
///钢种变更对话框初始化查询
///<para>查询连铸设备</para>
///</summary>
/// <param name="sm_plan_no">2个炼钢计划号  </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11stdlg_inq);

int f_pssm11stdlg_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;
	CString v_factory_div = " ";

	EIClass inBlock;

	/* 实体类定义 */
	CModel tpssmd1("TPSSMD1");

	/* 数据库SQL操作字符串 */
	CString sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);


	try
	{
		//---------------------------------------------------
		//设定返回参数表
		bcls_ret->Tables[0].set_TableName("CC_DEV");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_NO");  //连铸机号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_DESC");

		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();

		//---------------------------------------------------
		//查询连铸设备信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT * FROM TPSSMD1 "
				"  WHERE FACTORY_DIV = @FACTORY_DIV "
				"    AND AREA_ID = 5 "
				" ORDER BY DEV_CODE ASC "
				);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);

			CDataRow &row = bcls_ret->Tables["CC_DEV"].Rows.Add();
			row["DEV_NO"] = tpssmd1["STATION_NO"].ToString().Trim();
			row["DEV_CODE"] = tpssmd1["DEV_CODE"].ToString().Trim();
			row["DEV_DESC"] = tpssmd1["STATION_NAME"].ToString().Trim();
		}
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
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

	return doFlag;
}
