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
/// <param name="tpssm63.SUM_MONTH">月度生产总炉数 </param>
/// 1、根据传入生产月份date_time计算当月共有多少天month_days</param>
/// 2、更新前台传入的修改信息
/// 3、需要计算的修改天数month_days = month_days-修改的天数rows-已经生产的天数counts（包括今天）
/// 2、需要生产的总炉数heat_sum_month = heat_sum_month-修改过天数的炉数heat_sums-已经生产的炉数sum_shift1+sum_shift2+sum_shift3（包括今天）
/// 3、获取每天生产炉数sum_month =heat_sum_month/days，若除不尽获取余数res_month</param>
/// 4、若res_month大于0每天生产炉数sum_day=sum_month+1；res_month -1以此循环每天加1直至res_month =0</param>
/// 5、每班生产炉数sum_shift=sum_day/3,若除不尽获取余数res_day
/// 6、若res_day大于0第一班生产炉数tpssm63.SUM_SHIFT1 = sum_shift+1；res_day -1；以此循环直至第三班
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm38_upt)


int f_pssm38_upt(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;
	int months, year, month_days;
	CDecimal sum_month, res_month, sum_day, res_day, sum_shift;

	CString date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString prod_time = "";
	CDecimal heat_sum_month = 0;
	CDecimal heat_sum_month_1 = 0;
	CDecimal sum_shift1 = 0;
	CDecimal sum_shift2 = 0;
	CDecimal sum_shift3 = 0;
	CDecimal counts = 0;
	CString days = "";
	CDbCommand cmd_inq(conn);

	// 定义表的实体对象
	CTPSSM63 tpssm63(conn);
	CString sqlstr = "";
	CDecimal heat_sums = 0;
	try
	{
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Trace(" ", __FUNCTION__, "rows =[{0}]", rows);
		heat_sum_month = bcls_rec->Tables[0].Rows[0]["HEAT_SUM_MONTH"].ToDecimal();//前台传入修改总炉数
		heat_sum_month_1 = heat_sum_month;
		//获取修改的总炉数、总天数、并修改传入值
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm63.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			Log::Trace(" ", __FUNCTION__, "DAY =[{0}]", tpssm63.DAY);

			prod_time = tpssm63.PROD_DATE + tpssm63.DAY;

			if (prod_time.Compare(date_time.SubstringNE(0, 8))<=0)
			{
				sprintf(s.msg, "不能修改已经生产的计划信息");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			heat_sums = heat_sums + tpssm63.SUM_SHIFT1 + tpssm63.SUM_SHIFT2 + tpssm63.SUM_SHIFT3;//修改过的总炉数累加
			Log::Trace(" ", __FUNCTION__, "heat_sums修改过总炉数 =[{0}]", heat_sums);
			if (i < rows-1 )
			{
				days = days + "'" + tpssm63.DAY + "',";
			}
			if (i == rows -1)
			{
				days = days + "'" + tpssm63.DAY +"'";
			}

			tpssm63.REC_REVISOR = CString(s.userid);
			tpssm63.REC_REVISE_TIME = date_time;

			if (heat_sum_month > 0)
			{
				tpssm63.SUM_MONTH = heat_sum_month_1;
			}
			// 执行修改
			tpssm63.Update("REC_REVISOR,REC_REVISE_TIME,SUM_SHIFT1,SUM_SHIFT2,SUM_SHIFT3,SUM_MONTH", "PROD_DATE,DAY");
		}//for循环结束
		//计算一天生产的总炉数、余数
		if (heat_sum_month <= 0)
		{
			heat_sum_month = tpssm63.SUM_MONTH;
		}
		Log::Trace(" ", __FUNCTION__, "heat_sum_month月度总炉数 =[{0}]", heat_sum_month);

		sqlstr = "SELECT SUM(SUM_SHIFT1),SUM(SUM_SHIFT2),SUM(SUM_SHIFT3),COUNT(*) FROM TPSSM63 WHERE PROD_DATE=@prod_date "
			     "AND DAY <= @today_time "
				 "AND PROD_DATE = @daty_current";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date", tpssm63.PROD_DATE);
		cmd_inq.Parameters.Set("today_time", date_time.SubstringNE(6, 2));
		cmd_inq.Parameters.Set("daty_current", date_time.SubstringNE(0, 6));
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			sum_shift1 = cmd_inq.GetDecimal(1);
			sum_shift2 = cmd_inq.GetDecimal(2);
			sum_shift3 = cmd_inq.GetDecimal(3);
			counts = cmd_inq.GetDecimal(4);
		}
		cmd_inq.Close();

		Log::Trace(" ", __FUNCTION__, "今天之前sum_shift1 =[{0}]", sum_shift1);
		Log::Trace(" ", __FUNCTION__, "今天之前sum_shift2 =[{0}]", sum_shift2);
		Log::Trace(" ", __FUNCTION__, "今天之前sum_shift3 =[{0}]", sum_shift3);
		Log::Trace(" ", __FUNCTION__, "今天之前总行数counts =[{0}]", counts);

		if ((heat_sums + sum_shift1 + sum_shift2 + sum_shift3) > heat_sum_month)
		{
			sprintf(s.msg, "修改后的炉数和已经生产的炉数，超过月度总炉数，请核实修改信息");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		heat_sum_month = heat_sum_month - heat_sums - sum_shift1 - sum_shift2 - sum_shift3;//本月中除了修改后的总炉数；
		year = CDateTime::Parse(tpssm63.PROD_DATE + "01000000").Year();
		months = CDateTime::Parse(tpssm63.PROD_DATE + "01000000").Month();
		month_days = CDateTime::Parse(tpssm63.PROD_DATE + "01000000").DaysInMonth(year, months);

		Log::Trace(" ", __FUNCTION__, "month_days =[{0}]", month_days);

		month_days = month_days - rows - counts.ToInt32(); //本月中今天以后除了修改后的天数；

		Log::Trace(" ", __FUNCTION__, "需要修改的总天数month_days =[{0}]", month_days);

		sum_month = (heat_sum_month / double(month_days)).Floor();
		Log::Trace(" ", __FUNCTION__, "每天共生产多少炉sum_month =[{0}]", sum_month);
		res_month = heat_sum_month.ToInt32() % month_days;
		Log::Trace(" ", __FUNCTION__, "每天共生产多少炉余数sum_month =[{0}]", res_month);

		Log::Trace(" ", __FUNCTION__, "res_month(sql前) =[{0}]", res_month);
		//更新还未生产的炉数变更，不包括今天
		if (tpssm63.PROD_DATE.Compare(date_time.SubstringNE(0, 6)) > 0)
		{
			sqlstr = "SELECT * FROM TPSSM63 WHERE PROD_DATE=@prod_date "
				     "AND DAY NOT IN ( " + days + " ) ";
		}
		else if (tpssm63.PROD_DATE.Compare(date_time.SubstringNE(0, 6)) == 0)
		{
			sqlstr = "SELECT * FROM TPSSM63 WHERE PROD_DATE=@prod_date "
				"AND DAY > @today_time "
				"AND DAY NOT IN ( " + days + " ) ";
		}
		

		Log::Trace(" ", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
		Log::Trace(" ", __FUNCTION__, "tpssm63.PROD_DATE =[{0}]", tpssm63.PROD_DATE);
		Log::Trace(" ", __FUNCTION__, "date_time =[{0}]", date_time);
		Log::Trace(" ", __FUNCTION__, "days =[{0}]", days);

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date", tpssm63.PROD_DATE);
		cmd_inq.Parameters.Set("today_time", date_time.SubstringNE(6,2));
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm63);
			tpssm63.TrimOrBlank();
			tpssm63.SUM_MONTH = heat_sum_month_1;
			Log::Trace(" ", __FUNCTION__, "tpssm63.DAY =[{0}]", tpssm63.DAY);
			if (res_month > 0)
			{
				sum_day = sum_month + 1;
				res_month = res_month - 1;
			}
			else
			{
				sum_day = sum_month;
			}

			sum_shift = (sum_day / 3).Floor();  //二班制
			res_day = sum_day.ToInt32() % 3;    //二班制

			//sum_shift = (sum_day / 3).Floor();  //三班制
			//res_day = sum_day.ToInt32() % 3;    //三班制
			if (res_day > 0)
			{
				tpssm63.SUM_SHIFT1 = sum_shift + 1;
				res_day = res_day - 1;
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
			tpssm63.REC_REVISOR = CString(s.userid);
			tpssm63.REC_REVISE_TIME = date_time;

			//执行修改
			tpssm63.Update("REC_REVISOR,REC_REVISE_TIME,SUM_SHIFT1,SUM_SHIFT2,SUM_SHIFT3,SUM_MONTH", "PROD_DATE,DAY");

		}//while 循环结束
		cmd_inq.Close();
		//更新今天之前（包括今天）的生产总炉数
		sqlstr = "SELECT * FROM TPSSM63 WHERE PROD_DATE=@prod_date "
			"AND DAY<= @today_time ";

		Log::Trace(" ", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
		Log::Trace(" ", __FUNCTION__, "tpssm63.PROD_DATE =[{0}]", tpssm63.PROD_DATE);
		Log::Trace(" ", __FUNCTION__, "date_time =[{0}]", date_time);
		Log::Trace(" ", __FUNCTION__, "days =[{0}]", days);

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date", tpssm63.PROD_DATE);
		cmd_inq.Parameters.Set("today_time", date_time.SubstringNE(6, 2));
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm63);
			tpssm63.SUM_MONTH = heat_sum_month_1;
			tpssm63.REC_REVISE_TIME = date_time;
			tpssm63.Update("REC_REVISOR,REC_REVISE_TIME,SUM_MONTH", "PROD_DATE,DAY");
		
		}
		cmd_inq.Close();
		
	
        
		
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

