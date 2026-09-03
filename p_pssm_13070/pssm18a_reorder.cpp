/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Date:     2015-07-16
Version:  3.1.0
Description:  PES系统接受对MMS系统下发的板坯制造命令进行接收
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

int f_pssm_reranking(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

// service入口
BM2F_ENTERACE(pssm18a_reorder)

int f_pssm18a_reorder(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int doFlag = 0;
	CString sqlstr = "";
	int ret = 0;
	EIClass inblk;
	CDbCommand cmd_tep0002_inq(conn);
	try
	{
		string p = "";

		sqlstr = "SELECT CODE_DESC_1_CONTENT "
			"  FROM TEP0002 "
			"  WHERE CODE_CLASS  =  'PSZZ'";
		cmd_tep0002_inq.SetCommandText(sqlstr);
		cmd_tep0002_inq.ExecuteReader();
		if (cmd_tep0002_inq.Read())
		{
			p = cmd_tep0002_inq.GetString(1);
		}
		if (p == "1")
		{
			ret = f_pssm_reranking(&inblk, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

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