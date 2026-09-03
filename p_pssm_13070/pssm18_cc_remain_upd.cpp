/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    weichenxiang
Version:    3.1
Date:      2014-11-24
Description: 出钢计划铸余维护
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
BM2F_ENTERACE(pssm18_cc_remain_upd)


int f_pssm18_cc_remain_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;
	CString date_time = "";

	/* 业务变量 */
	CDecimal v_sm_plan_no = 0;	//炼钢计划号
	CString  v_pono = "";

	CString sqlstr = "";

	CDbCommand cmd(conn);

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");


	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------------------
		//定义函数调用信息块
		blkseq = 0;  //第一块 "PONO"块，f_pssm12_time_calc()函数用
		//---------------------------------------------------
		//获得输入参数，并进行相应解析（调用f_pssm12_plan_upd函数用）
		blkseq = bcls_rec->Tables.IndexOf("TPSSM11");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有传入TPSSM11信息数据块[TPSSM12A]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [TPSSM11] NOT EXIST in pssm18_cc_remain_upd().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		//循环读取信息，多记录
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"].ToDecimal();
			v_pono = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString();

			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString();
			tpssm11["PONO"] = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString();
			tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"].ToString();

			tpssm11["CC_REMAIN_RET"] = bcls_rec->Tables[blkseq].Rows[i]["CC_REMAIN_RET"].ToString();
			tpssm11["CC_REMAIN_USE"] = bcls_rec->Tables[blkseq].Rows[i]["CC_REMAIN_USE"].ToString();
			

			//TPSSM11字段修改
			//更新表
			tpssm11.Update("CC_REMAIN_RET,CC_REMAIN_USE", "SM_PLAN_NO");


		}//for 传入参数

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
