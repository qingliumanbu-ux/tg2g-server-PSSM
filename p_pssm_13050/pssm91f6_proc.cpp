/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-06-03 17:13:56  
Description: 炉次确定
**************************************************/

/***** C++ 的标准头文件部分 *****/ 
#include "stdafx.h"

int f_pssm_heat_confirm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/***** C++ 的业务头文件部分 *****/ 

/* ***** 静态函数申明 ***** */

/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 炉次确定
/// <para>
/// 1.炉次确定。
/// 
/// </para>
/// <para>数据库表：TPSSM11(出钢计划表)					</para>
/// <para>主调用函数：前台PSSM91画面F3(新增)按钮				</para>
/// </summary>
/// <param name=" ">     </param>
/// <param name=" ">                </param>
/// <returns>  </returns>
===========================================================</remark>*/
// service入口


BM2F_ENTERACE(pssm91f6_proc)

int f_pssm91f6_proc(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{

	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr;
	

	try
	{
		/* 设置块名 */

		/* 设置块名 */
		bcls_rec->Tables.Add("OP_FLAG");
		bcls_rec->Tables["OP_FLAG"].Columns.Add(DT_STRING, "FLAG");
		bcls_rec->Tables["OP_FLAG"].Rows.Add();
		bcls_rec->Tables["OP_FLAG"].Rows[0]["FLAG"] = 1;		
		
		doFlag = f_pssm_heat_confirm(bcls_rec, bcls_ret,conn);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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


	return doFlag;

}
