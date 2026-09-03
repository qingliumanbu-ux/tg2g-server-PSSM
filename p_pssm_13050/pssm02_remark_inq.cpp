/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-11-12
Description: 备注信息查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/


// service入口
BM2F_ENTERACE(pssm02_remark_inq)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssm02_remark_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__); 	//系统日志类定义

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;

	/* 业务变量 */

	/* 数据库SQL操作字符串 */
	CString sqlstr = "";


	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm24_inq(conn);

	CModel tpssm24("TPSSM24");

	try
	{
		////获取传入参数
		//tpssm24["BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["BACKLOG_CODE"].ToString();

		////打印传入参数
		//Log::Trace("", __FUNCTION__, "pssm02l_slab_inq>tpssm24.BACKLOG_CODE = [{0}]", tpssm24["BACKLOG_CODE"]);

		////校验
		//if (tpssm24["BACKLOG_CODE"].ToString() == "")
		//{
		//	strcpy(s.msg, "工序代码BACKLOG_CODE 传入为空");
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		///* 逻辑处理 */
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	    // Oracle 数据库
		//default:

		//	sqlstr = " SELECT * FROM TPSSM24 "
		//		" WHERE BACKLOG_CODE = @BACKLOG_CODE ";

		//	break;
		//}
		//cmd_tpssm24_inq.SetCommandText(sqlstr);
		//cmd_tpssm24_inq.Parameters.Set("BACKLOG_CODE", tpssm24["BACKLOG_CODE"]);
		//ret = cmd_tpssm24_inq.ExecuteQuery(bcls_ret->Tables[0]);
		//cmd_tpssm24_inq.Close();
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
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}




