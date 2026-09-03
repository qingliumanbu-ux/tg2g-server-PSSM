/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2012-1-6
Description:新增月计划信息
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tpssm63.h"

/*<remark>=========================================================
/// <summary>
/// 新增月计划信息
/// <para>数据库表：tpssm63炼钢生产计划表        </para>
/// <para>主调用函数：前台PSSMR1 F3新增。 </para>
/// <param name="tpssm63.SUM_MONTH">月度生产总炉数 </param>
/// 1、根据传入生产月份date_time计算当月共有多少天days</param>
/// 2、按天循环插入tpssm63表每天每班生产炉数</param>
/// 3、获取每天生产炉数sum_month =tpssm63.SUM_MONTH/days，若除不尽获取余数res_month</param>
/// 4、若res_month大于0每天生产炉数sum_day=sum_month+1；res_month -1以此循环每天加1直至res_month =0</param>
/// 5、每班生产炉数sum_shift=sum_day/3,若除不尽获取余数res_day
/// 6、若res_day大于0第一班生产炉数tpssm63.SUM_SHIFT1 = sum_shift+1；res_day -1；以此循环直至第三班
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm38_ins)

int f_pssm38_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int i;
	int doFlag = 0;
	int months, year, days;
	CDecimal sum_month,res_month, sum_day, res_day, sum_shift;
	CString date_time = "";
	// 定义表的实体对象
	CTPSSM63 tpssm63(conn);
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);


	try
	{

		// 获取前台传入参数
		date_time = bcls_rec->Tables[0].Rows[0]["PROD_DATE"].ToString().Trim();
		tpssm63.SUM_MONTH = bcls_rec->Tables[0].Rows[0]["SUM_MONTH"].ToDecimal();
		tpssm63.PROD_DATE = date_time.SubstringNE(0,6);

		Log::Trace("", __FUNCTION__, "月总炉数: {0}", tpssm63.SUM_MONTH);

		int count = tpssm63.QueryCount("PROD_DATE");

		if (count > 0)
		{
			sprintf(s.msg, "Record is already exist!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		tpssm63.REC_CREATOR = CString(s.userid);
		tpssm63.REC_CREATE_TIME = CDateTime::Now().ToString("yyyyMMddHHmmss");

		Log::Trace("", __FUNCTION__, "date_time: {0}", (const char*)date_time);
		year = CDateTime::Parse(date_time).Year();
		months = CDateTime::Parse(date_time).Month();
		days = CDateTime::Parse(date_time).DaysInMonth(year, months);

		sum_month = (tpssm63.SUM_MONTH / double(days)).Floor();

		res_month = tpssm63.SUM_MONTH.ToInt32() % days;


		for (i = 1; i <= days; i++)
		{
			tpssm63.DAY = CConvert::ToString(i, "%02d");
			if (res_month > 0)
			{
				sum_day = sum_month + 1;
				res_month = res_month - 1;
			}
			else
			{
				sum_day = sum_month;
			}

			sum_shift = sum_day.ToInt32() / 2;   //二班制
			//sum_shift = sum_day.ToInt32() / 3;  //三班制

			Log::Trace("", __FUNCTION__, "班组炉数sum_shift: {0}", sum_shift);

			res_day = sum_day.ToInt32() % 2;   //二班制
			//res_day = sum_day.ToInt32() % 3;   //三班制
			Log::Trace("", __FUNCTION__, "班组炉数余数res_day: {0}", res_day);

			if (res_day > 0)
			{
				tpssm63.SUM_SHIFT1 = sum_shift + 1;
				res_day = res_day - 1;
				Log::Trace("", __FUNCTION__, "余数: {0}", res_day);
			}
			else
			{
				tpssm63.SUM_SHIFT1 = sum_shift;
			}
			if (res_day > 0)
			{
				tpssm63.SUM_SHIFT2 = sum_shift + 1;
				res_day = res_day - 1;
			}
			else
			{
				tpssm63.SUM_SHIFT2 = sum_shift;
			}
			if (res_day > 0)
			{
				tpssm63.SUM_SHIFT3 = sum_shift + 1;
				res_day = res_day - 1;
			}
			else
			{
				tpssm63.SUM_SHIFT3 = sum_shift;
			}
			//如果分摊班组炉数小于0，班组炉数为0
			if (tpssm63.SUM_SHIFT1 < 0)
			{
				tpssm63.SUM_SHIFT1 = 0;
			}
			if (tpssm63.SUM_SHIFT2 < 0)
			{
				tpssm63.SUM_SHIFT2 = 0;
			}
			if (tpssm63.SUM_SHIFT3 < 0)
			{
				tpssm63.SUM_SHIFT3 = 0;
			}
			// 执行新增
			tpssm63.Insert();

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

