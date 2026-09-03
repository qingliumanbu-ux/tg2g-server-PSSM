/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    chejs
Version:   1.0
Date:      2015-05-22
Description: 炼钢计划日备注信息下发
**************************************************************************************************************/
/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/
#include "tpssm24.h"
#include "epex.h"

BM2_FUNCTION_EXPORT
int f_cm_002024_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	//程序用变量
	int doFlag = 0;
	int ret = 0;

	/* 业务变量 */
	CString sqlstr("");
	CString	lpsz_tc_no("");

	// 创建电文处理对象
	EPEX epex(&s, conn);

	/* 实体类定义 */
	CTPSSM24 tpssm24(conn);

	/* 数据库操作类定义 */

	try
	{
		/* ***** 获取输入参数 ***** */
		tpssm24.MergeFrom(bcls_rec->Tables["H3H424"].Rows[0]);

		Log::Trace("", __FUNCTION__, "f_cm_h3h424_snd>FACTORY_DIV = [{0}]", tpssm24.FACTORY_DIV);
		Log::Trace("", __FUNCTION__, "f_cm_h3h424_snd>BACKLOG_CODE = [{0}]", tpssm24.BACKLOG_CODE);
		Log::Trace("", __FUNCTION__, "f_cm_h3h424_snd>REMARK_DESC = [{0}]", tpssm24.REMARK_DESC);

		/* 检查输入参数合法性 */

		if (tpssm24.FACTORY_DIV.Trim() == "")
		{
			strcpy(s.sysmsg, "FACTORY_DIV不能为空!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpssm24.BACKLOG_CODE.Trim() == "")
		{
			strcpy(s.sysmsg, "BACKLOG_CODE不能为空.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		lpsz_tc_no = "H3H424";

		if (epex.Initialize(lpsz_tc_no) < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}


		if (epex.SetValue(0, tpssm24) < 0)
		{
			sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		Log::Trace("", __FUNCTION__, "发送H3H424电文开始");
		/*发送电文 */
		if (epex.SendTele() < 0)
		{
			strcpy(s.msg, "电文发送失败。");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		Log::Trace("", __FUNCTION__, "发送H3H424电文结束");

		/* 释放 */
		epex.Uninitialize();

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


