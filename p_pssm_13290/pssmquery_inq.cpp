/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      KE2111
Version:     1.0
Date:        2022-11-24 15:08:16
Description: Í¨ÓÃ²éÑ¯
**************************************************/

#include "stdafx.h"

BM2F_ENTERACE(pssmquery_inq)


int f_pssmquery_inq(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);
	try
	{
		// -----Begin IPLAT4C::IPLAT4CServiceCompositeStatementObj()----- //
		sqlstr = bcls_rec->Tables[0].Rows[0][0];
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd.SetCommandText(sqlstr);
		cmd.ExecuteQuery(bcls_ret->Tables[0]);
		// -----End IPLAT4C::IPLAT4CServiceCompositeStatementObj()----- //
	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


