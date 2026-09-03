/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   songwei
Version:    1.0
Date:
Description: 模板画面维护
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */

// service入口
BM2F_ENTERACE(pssmd6x_upd)

int f_pssmd6x_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int blkNum = 0;
	int doFlag = 0;
	CString s_formname = "";
	CString v_proc_div = "";
	CString sqlstr = "";
	CString sqlstr_temp = "";
	CString v_table_name = "";//表名称。
	CString v_mat_code = "";
	CString v_mat_id = "";
	CString v_dev_code = "";
	CString v_mat_name = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_sql(conn);
	CString  nowTime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	try
	{
		CModel tpssmd6_x("TPSSMD6_X");
		if (bcls_rec->Tables.Contains("DEL"))
		{
			Log::Trace("", "", "--------------DEL-------------");
			for (int i = 0; i < bcls_rec->Tables["DEL"].Rows.get_Count(); i++)
			{
				tpssmd6_x.Reset();
				tpssmd6_x.MergeFrom(bcls_rec->Tables["DEL"].Rows[i]);
				tpssmd6_x.TrimOrBlank();
				tpssmd6_x.Delete("FACTORY_DIV,ST_NO,DEV_MOVE_START,DEV_MOVE_END");

			}
		}
		if (bcls_rec->Tables.Contains("ADD"))
		{
			Log::Trace("", "", "--------------ADD-------------");
			for (int i = 0; i < bcls_rec->Tables["ADD"].Rows.get_Count(); i++)
			{
				tpssmd6_x.Reset();
				tpssmd6_x.MergeFrom(bcls_rec->Tables["ADD"].Rows[i]);

				tpssmd6_x["REC_CREATE_TIME"] = nowTime;
				tpssmd6_x["REC_CREATOR"] = s.userid;
				tpssmd6_x["FACTORY_DIV"] = "LG1";
				tpssmd6_x.TrimOrBlank();
				tpssmd6_x.Insert();
			}
		}
		if (bcls_rec->Tables.Contains("UPD"))
		{
			Log::Trace("", "", "--------------UPD-------------");
			for (int i = 0; i < bcls_rec->Tables["UPD"].Rows.get_Count(); i++)
			{
				tpssmd6_x.Reset();
				tpssmd6_x.MergeFrom(bcls_rec->Tables["UPD"].Rows[i]);
				tpssmd6_x.Delete("FACTORY_DIV,ST_NO,DEV_MOVE_START,DEV_MOVE_END");
				tpssmd6_x.MergeFrom(bcls_rec->Tables["UPD"].Rows[i]);
				tpssmd6_x["REC_REVISE_TIME"] = nowTime;
				tpssmd6_x["REC_REVISOR"] = s.userid;
				tpssmd6_x["FACTORY_DIV"] = "LG1";
				tpssmd6_x.TrimOrBlank();
				tpssmd6_x.Insert();
			}
		}

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
