/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2015-11-10
Description:	 写炼钢计划履历表。
Update:
**************************************************************************************************************/
#include "stdafx.h"

//程序用头文件




int f_mmsm99(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

int f_pssm81_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount, samprow;
	int blkNum = 0;
	int i = 0;
	int fetchRowCount1 = 0;
	int fetchRowCount2 = 0;
	int rows = 0;

	int v_rownum_pm99 = 0;
	int v_rownum_mm99 = 0;

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

	CModel tpssm81("TPSSM81");
	CModel hpssm81("HPSSM81");
	CModel tmmsm01("TMMSM01");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm81_inq(conn);
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
			tpssm81["FACTORY_DIV"] = bcls_rec->Tables["PSSM"].Rows[0]["FACTORY_DIV"];
			tpssm81["PLAN_BACKLOG_CODE"] = bcls_rec->Tables["PSSM"].Rows[0]["PLAN_BACKLOG_CODE"];
			tpssm81["PLAN_NO"] = bcls_rec->Tables["PSSM"].Rows[0]["PLAN_NO"];
			tpssm81["MAT_NO"] = bcls_rec->Tables["PSSM"].Rows[i]["MAT_NO"];

			////Log::Trace("", __FUNCTION__, "tpssm81.FACTORY_DIV[{0}],[{1}],[{2}],[{3}]", tpssm81["FACTORY_DIV"].ToString(), tpssm81["PLAN_BACKLOG_CODE"].ToString(), tpssm81["PLAN_NO"].ToString(), tpssm81["MAT_NO"].ToString());

			if (tpssm81.Query("FACTORY_DIV, PLAN_BACKLOG_CODE, PLAN_NO, MAT_NO") == false)
			{
				CFormattable arguments[] = { tpssm81["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在计划中不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tpssm81["PLAN_STATUS"].ToString() < "06")
			{
				CFormattable arguments[] = { tpssm81["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]命令状态已经未下发。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm81.TrimOrBlank();

			if (tpssm81["REPAIR_FLAG"].ToString() == "1")
			{
				//修改返修处置表的记录状态：1,计划中
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:				// MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " UPDATE TQMTJF1 SET STATUS_FLAG = '2' "
						"  WHERE MAT_NO = @tpssm81.MAT_NO  "
						"	 AND STATUS_FLAG = '1' ";
					break;
				}

				cmd_tqmtjf1_upd.SetCommandText(sqlstr);
				cmd_tqmtjf1_upd.Parameters.Set("tpssm81.MAT_NO", tpssm81["MAT_NO"].ToString());
				cmd_tqmtjf1_upd.ExecuteNonQuery();
			}

			hpssm81.CopyFrom(tpssm81);
			hpssm81.Insert();
			tpssm81.Delete("FACTORY_DIV, PLAN_BACKLOG_CODE, PLAN_NO, MAT_NO");
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
