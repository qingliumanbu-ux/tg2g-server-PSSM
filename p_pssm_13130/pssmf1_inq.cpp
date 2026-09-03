
/***** C/C++ 的标准头文件部分 *****/
#include "stdafx.h"


// service入口

BM2F_ENTERACE(pssmf1_inq)

int f_pssmf1_inq(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int doFlag = 0;
	int fetchRowCount = 0;
	int row_count = 0;

	CModel tsmpe00("TSMPE00");

	/* ***** 程序变量 ***** */
	CString c_user = s.userid, c_mat_kind = " ", datetime = " ";
	//CString c_stock_no=" ",c_stock_no_end=" ",c_query_type=" ";

	CString c_stock_no = " ";
	CString c_confm_plan_no = " ";
	CString c_ready_bill_no = " ";
	CString c_order_no_from = " ";
	CString c_order_no_to = " ";
	CString c_confm_status_from = " ";
	CString c_confm_status_to = " ";
	CString c_prg_send_time_from = " ";
	CString c_prg_send_time_to = " ";
	CString c_mat_no_from = " ";
	CString c_mat_no_to = " ";
	CString c_heat_no = " ";
	CString c_factory_div = " ";
	CString c_code = " ";
	/* ***** 数据库SQL操作字符串 ***** */
	CString	sqlstr(""), sqlstr1(""), sqlstr2(""), sqlstr3("");

	/* ***** 数据库操作类定义 ***** */
	CDbCommand cmd_inq(conn);

	/* ***** 应用程序开始处理 ***** */
	try
	{
		/* ***** 获取前台参数  ***** */
		sqlstr = " select F1.STATION_NAME, T3.CURR_PROC_NO, T3.STATUS_NAME, T3.HEAT_NO\
			from TPSSMF1 F1,\
			TPSSM33 T3\
			WHERE F1.STATION_ID = T3.STATION_ID\
			AND F1.STATION_NO = T3.STATION_NO ";
		Log::Trace("", "", "", "", "sqlstr[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
  

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;
	}
	catch (const CApplicationException& ex)
	{
		//	strncpy(s.msg, (const char*)ex.GetMsg(), 399); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应 
		Log::Error("", __FUNCTION__, "error=[{0}]", s.msg);
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (const CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	return doFlag;
}
