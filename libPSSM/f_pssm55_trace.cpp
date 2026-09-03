/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2015-11-10
Description:	 电渣锭计划跟踪函数。
Update:
**************************************************************************************************************/
#include "stdafx.h"

//程序用头文件



int f_pssm55_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount, samprow;
	int blkNum = 0;
	int i = 0;
	int rows = 0;

	CString userid = " ";                  /* 登陆用户 */
	int v_cnt = 0;
	CDecimal v_cnt1 = 0;
	int v_cnt2 = 0;
	CDecimal v_cnt3 = 0;
	int v_mat_num = 0;
	CDecimal v_mat_wt = 0;
	CString v_errmsg = " ";         /* 错误信息 */
	CDecimal v_plan_exec_seq_no_max = 0;      /* 新顺序号 */
	int v_seq = 0;
	CString v_backlog_code = "";
	CDecimal v_mat_seq_no = 0;
	CString v_plan_no_pre = "";


	EIClass inBlock;
	EIClass outBlock;

	CModel tpssm05("TPSSM05");
	CModel hpssm05("HPSSM05");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm05_inq(conn);
	CDbCommand cmd_tqmtjf1_upd(conn);
	CDbCommand cmd_tep0002_inq(conn);

	CString sqlstr;

	CTracer log(__FUNCTION__);
	try
	{
		userid = s.userid;

		//获取输入参数；
		rows = bcls_rec->Tables["PSSM"].Rows.get_Count();

		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm05["FACTORY_DIV"] = bcls_rec->Tables["PSSM"].Rows[i]["FACTORY_DIV"];
			tpssm05["PLAN_BACKLOG_CODE"] = bcls_rec->Tables["PSSM"].Rows[i]["PLAN_BACKLOG_CODE"];
			tpssm05["PLAN_NO"] = bcls_rec->Tables["PSSM"].Rows[i]["PLAN_NO"];
			tpssm05["IN_MAT_NO"] = bcls_rec->Tables["PSSM"].Rows[i]["MAT_NO"];

			////Log::Trace("", __FUNCTION__, "tpssm05.FACTORY_DIV[{0}],[{1}],[{2}],[{3}]", tpssm05["FACTORY_DIV"].ToString(), tpssm05["PLAN_BACKLOG_CODE"].ToString(), tpssm05["PLAN_NO"].ToString(), tpssm05["IN_MAT_NO"].ToString());

			if (tpssm05.Query("FACTORY_DIV, PLAN_BACKLOG_CODE, PLAN_NO, IN_MAT_NO") == false)
			{
				CFormattable arguments[] = { tpssm05["IN_MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在计划中不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tpssm05["PLAN_STATUS"].ToString() < "06")
			{
				CFormattable arguments[] = { tpssm05["IN_MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]命令状态已经未下发。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm05.TrimOrBlank();

			hpssm05.CopyFrom(tpssm05);
			hpssm05.Insert();
			tpssm05.Delete("FACTORY_DIV, PLAN_BACKLOG_CODE, IN_MAT_NO");
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

	return (doFlag);

}
