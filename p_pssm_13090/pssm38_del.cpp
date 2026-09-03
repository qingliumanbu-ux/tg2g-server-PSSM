/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2012-1-6
Description:修改月计划信息
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
#include "tpssm63.h"

/*<remark>=========================================================
/// <summary>
/// 修改月计划信息
/// <para>数据库表：tpssm63炼钢生产计划表        </para>
/// <para>主调用函数：前台PSSMR1 F3新增。 </para>
/// <param name="tpssm63.PROD_DATE">生产日期 </param>
/// 1、只能删除尚未开始的月计划</param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm38_del)


int f_pssm38_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;

	CString date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CDbCommand cmd_inq(conn);

	// 定义表的实体对象
	CTPSSM63 tpssm63(conn);
	CString sqlstr = "";
	CDecimal heat_sums = 0;
	try
	{
		tpssm63.PROD_DATE = bcls_rec->Tables[0].Rows[0]["PROD_DATE"].ToString();//前台传入修改总炉数
		if (tpssm63.PROD_DATE.Compare(date_time.SubstringNE(0, 6)) <= 0)
		{
			sprintf(s.msg, "不能删除本月之前的计划信息");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		tpssm63.Delete("PROD_DATE");
        

	}//try结束
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

