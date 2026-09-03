/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2012-01-16
Version:1.0
Description: 状态回退画面熔炼号查询
Update: 2015-03-19 lijie  数据表调整
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 状态回退画面熔炼号查询
/// <para>查询制造命令号。                            </para>
/// <para>数据库表：tpssm11                    </para>
/// <para>主调用函数：PSSM18S画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>制造命令号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_sk_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm18_sk_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	
	//程序用变量
	int doFlag = 0; 
	CModel tpssm11("TPSSM11");
	CString sqlstr = "";
	CDbCommand cmd_tpssm11_inq(conn);

	try
	{
		//获得输入参数
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
		{
			tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"];
		}
		//设定返回参数表
		//1)熔炼号
		bcls_ret->Tables[0].set_TableName("HTNO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"HTNO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NOL2");
		
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT HEAT_NO,SM_PLAN_NO,SM_PLAN_NOL2 FROM TPSSM11 \
					    WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV \
					      AND RUN_STATUS       >= 00 \
					      AND RUN_STATUS        < 91 ";
			break;
		}
		if (tpssm11["PONO"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " AND PONO = @tpssm11.PONO ";
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		while(cmd_tpssm11_inq.Read())
		{
			//tpssm11["HEAT_NO"] = cmd_tpssm11_inq.GetString(1);	
			CDataRow & row = bcls_ret->Tables[0].Rows.Add();
			row["HTNO"] = cmd_tpssm11_inq.GetString(1);
			row["SM_PLAN_NO"] = cmd_tpssm11_inq.GetString(2);
			row["SM_PLAN_NOL2"] = cmd_tpssm11_inq.GetString(3);
		}
		cmd_tpssm11_inq.Close();
		
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	cmd_tpssm11_inq.Close();	
	return doFlag;

}
