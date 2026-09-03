/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2014-11-14
Description:月出钢计划维护送信至EMS系统
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tpssm63.h"

//月计划维护至EMS系统电文
int f_pssm_kbkm21_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// <para>数据库表：tpssm11(炼钢作业命令编制主表)</para>
/// <para>主调用函数：前台PSSM11 F5下达。 </para>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm38_snd)

int f_pssm38_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;
	int ret = 0;

	EIClass inBlock1;//出钢计划月维护下达输入块
	EIClass outBlock1;//出钢计划月维护下达返回块

	// 定义表的实体对象
	CTPSSM63 tpssm63(conn);
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);

	try
	{

		ret = f_pssm_kbkm21_snd(bcls_rec, &outBlock1, conn);

		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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

