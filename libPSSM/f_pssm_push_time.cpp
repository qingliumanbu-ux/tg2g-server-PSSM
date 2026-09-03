/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-16
Version:1.0
Description:
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件



BM2_FUNCTION_EXPORT
int f_pssm_push_time(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int i = 0;
	int j = 0;
	int proc_time = 0;
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";

	CDecimal diff_time = 0;
	CString pono = "";
	CString sm_plan_no = " ";
	CString sm_plan_no2 = " ";

	CString sqlstr = "";
	CString time_now = "";
	CString start_time_min = "";
	CString end_time_min = "";

	CDateTime start_timex;
	CDateTime end_timex;

	CTimeSpan proc_time_dif;

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssmd3_inq(conn);
	CDbCommand cmd_tpssm12_inq2(conn);
	try
	{
		time_now = CDateTime::Now().ToString("yyyyMMddHHmmss");

		sqlstr = "  SELECT MIN(a.START_TIME) FROM tpssm12 a,tpssm11 b WHERE a.START_TIME_REAL = ' ' AND a.END_TIME_REAL = ' ' AND a.SM_PLAN_NO = b.SM_PLAN_NO  AND b.PONO_STATUS < 83 ";
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
			start_time_min = cmd_tpssm12_inq.GetString(1);
		}
		cmd_tpssm12_inq.Close();

		if (time_now > start_time_min)
		{
			start_timex = CDateTime::Parse(start_time_min);
			end_timex = CDateTime::Parse(time_now);

			proc_time_dif = end_timex - start_timex;
			diff_time = proc_time_dif.TotalSeconds();
			Log::Trace("", __FUNCTION__, "diff_time = {0}", diff_time);

			sqlstr = "  SELECT a.* FROM tpssm12 a,tpssm11 b WHERE a.START_TIME_REAL = ' ' AND a.END_TIME_REAL = ' ' AND a.SM_PLAN_NO = b.SM_PLAN_NO  AND b.PONO_STATUS < 83 ORDER BY a.SM_PLAN_NO,a.CHARGE_NO ";
			cmd_tpssm12_inq2.SetCommandText(sqlstr);
			cmd_tpssm12_inq2.ExecuteReader();
			while (cmd_tpssm12_inq2.Read())
			{
				tpssm12.Reset();
				cmd_tpssm12_inq2.Fetch(tpssm12);
				//Log::Trace("", __FUNCTION__, "DEV_CODE = {0}", tpssm12["DEV_CODE"].ToString());
				//Log::Trace("", __FUNCTION__, "START_TIME = {0}", tpssm12["START_TIME"].ToString());
				//Log::Trace("", __FUNCTION__, "END_TIME = {0}", tpssm12["END_TIME"].ToString());
				tpssm12["START_TIME"] = (CDateTime::Parse(tpssm12["START_TIME"].ToString()).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
				tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12["END_TIME"].ToString()).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
				//Log::Trace("", __FUNCTION__, "START_TIME = {0}", tpssm12["START_TIME"].ToString());
				//Log::Trace("", __FUNCTION__, "END_TIME = {0}", tpssm12["END_TIME"].ToString());
				tpssm12.Update("START_TIME,END_TIME", "SM_PLAN_NO,CHARGE_NO");
			}
			cmd_tpssm12_inq2.Close();
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
