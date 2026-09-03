/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    weichenxiang
Version:    3.1
Date:      2023-5-24
Description: 保留炉数统计
**************************************************/
#include "stdafx.h"


//调用TPS模型
//int f_pssm_call_tps(CString main_backlog_code, int mode, int bof_cc_matching, CDbConnection * conn);//

/*<remark>=========================================================
/// <summary>
/// 出钢计划11表修改
/// <para>修改内容包括: 用铸余、返回铸余</para>
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm28_yy_inq)


int f_pssm28_yy_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;
	CString date_time = "", v_end_time, v_start_time;
	int		yyCount = 0;

	/* 业务变量 */
	CDecimal v_sm_plan_no = 0;	//炼钢计划号
	CString  v_factory_div = "";

	CString sqlstr = "";

	CDbCommand cmd(conn);

	// 定义表的实体对象
	//CModel tpssm11("TPSSM11");

	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
		v_end_time = date_time.Substring(0, 8) + "225959";
		v_start_time = CDateTime::Now().AddDays(-1).ToString("yyyyMMddHHmmss").Substring(0, 8) + "225959";

		//--------------------------------------------
		//定义函数调用信息块
		blkseq = 0;  //第一块 "PONO"块，f_pssm12_time_calc()函数用
		//---------------------------------------------------
		//获得输入参数，并进行相应解析（调用f_pssm12_plan_upd函数用）
		v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString();

		sqlstr = "SELECT count(PONO) AS COUNT1 FROM TQMTS23 WHERE DECI_ST_NO = 'YY000000' "
			" AND SM_PLAN_NO in(SELECT SM_PLAN_NO FROM(SELECT SM_PLAN_NO FROM TPSSM12 WHERE AREA_ID = '5' "
			" AND(END_TIME_REAL BETWEEN @end_time AND @start_time "
			" OR(END_TIME_REAL = ' ' and END_TIME BETWEEN @end_time AND @start_time)) "
			" UNION "
			" SELECT SM_PLAN_NO FROM TPSSM42 WHERE AREA_ID = '5' "
			" AND(END_TIME_REAL BETWEEN @end_time AND @start_time "
			" OR(END_TIME_REAL = ' ' and END_TIME BETWEEN @end_time AND @start_time)) "
			" ))";

		cmd.SetCommandText(sqlstr);
		cmd.Parameters.Set("FACTORY_DIV", v_factory_div);
		cmd.Parameters.Set("end_time", v_end_time);
		cmd.Parameters.Set("start_time", v_start_time);
		yyCount = cmd.ExecuteScalar().ToInt32();
		cmd.Close();

		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "YY_COUNT");
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[0]["YY_COUNT"] = yyCount;

	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = -1;
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
