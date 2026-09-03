#include "stdafx.h"
BM2F_ENTERACE(pssmdhv_F3_ins)

int f_pssmdhv_F3_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	CString sqlstr = "";
	CString s_formname = "";
	CModel tpssmdh("TPSSMDH");
	CDbCommand cmd_inq(conn);
	try
	{
		s_formname = s.formname;
		//Log::Trace("", "", "formname=[{0}]", s_formname);
		
			tpssmdh.MergeFrom(bcls_rec->Tables[0].Rows[0]);

			
			
			
			
			tpssmdh["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss").Trim();
			tpssmdh["REC_CREATOR"] = s.userid;
			tpssmdh.Insert();

			//Log::Trace("", "", "执行状态：tmmsmwtb.Insert Finish");
		}

	
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
