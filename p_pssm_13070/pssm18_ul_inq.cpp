/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:chejs
Date:2015-10-12
Version:1.0
Description: 转炉处理号查询
Update:
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
//#include "tpssm25.h"

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 转炉处理号查询
/// <para>查询CURR_PROC_NO                            </para>
/// <para>数据库表：tpssm25                   </para>
/// <para>主调用函数：PSSM21O2画面调用。                </para>
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别区分     </param>
/// <returns>run_status</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_ul_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm18_ul_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CString dev_code = "", factory_div = ""; //DEV_CODE

	CDbCommand cmd_tpssm25_inq(conn);

	try
	{
		//获得输入参数
		dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"];
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		//打印传入参数
		////Log::Info("", __FUNCTION__, "dev_code = [{0}] ,factory_div = [{1}] ", dev_code, factory_div);

		//设定返回参数表

		//转炉处理号
		bcls_ret->Tables[0].set_TableName("PROC_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROC_NO");

		//转炉处理号
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT CURR_PROC_NO FROM TPSSM25 WHERE STATION_ID = @STATION_ID AND STATION_NO = @STATION_NO  \
				 AND  FACTORY_DIV =@FACTORY_DIV ";
			break;
		}
		cmd_tpssm25_inq.SetCommandText(sqlstr);
		cmd_tpssm25_inq.Parameters.Set("STATION_ID", dev_code.SubstringNE(0,1));
		cmd_tpssm25_inq.Parameters.Set("STATION_NO", dev_code.SubstringNE(1,1));
		cmd_tpssm25_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_tpssm25_inq.ExecuteReader();
		while (cmd_tpssm25_inq.Read())
		{
			CDataRow & row = bcls_ret->Tables[0].Rows.Add();
			row["PROC_NO"] = cmd_tpssm25_inq.GetString(1);
		}
		cmd_tpssm25_inq.Close();

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
