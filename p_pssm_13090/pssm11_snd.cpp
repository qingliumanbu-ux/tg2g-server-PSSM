/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2014-11-14
Description:出钢计划下发
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tpssm11.h"
#include "tpssm12.h"

int f_pssm13_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//计划写入下达表

int f_pssm31_ins(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);//计划写入炉次钢种管理表

int f_pssm19_plan_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划下达时铁水计划写入

//计划下达接口
int f_pssm_plan_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// <para>数据库表：tpssm11(炼钢作业命令编制主表)</para>
/// <para>主调用函数：前台PSSM11 F5下达。 </para>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_snd)

int f_pssm11_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;
	int ret = 0;

	EIClass inBlock1;//出钢计划下达输入块
	EIClass outBlock1;//出钢计划下达返回块

	// 定义表的实体对象
	CTPSSM11 tpssm11(conn);

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);

	try
	{

		//1.计划下达给跟踪表
		ret = f_pssm13_ins(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			//sprintf(s.msg, "调用计划下达给跟踪表f_pssm13_ins出错");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//2.将新的出钢计划表写入炼钢计划炉次钢种管理表(TPSSM31)中
		ret = f_pssm31_ins(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			//sprintf(s.msg, "调用计划下达给跟炉次钢种管理表f_pssm31_ins出错");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//3.出钢计划下达时，铁水计划写入
		ret = f_pssm19_plan_ins(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			//sprintf(s.msg, "调用计划下达给跟踪表f_pssm13_ins出错");
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//4.向各级L2发送电文
		ret = f_pssm_plan_snd(bcls_rec, bcls_ret, conn);
		ret = 0;
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

