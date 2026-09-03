/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-11-28
功能: 炼钢指标查询
修改历史：
日期:________；修改人：________; 需求提出人________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <para > 
/// 1. 读取前台传入参数；
/// 2. 建立查询语句，查询PONO信息；
/// 3. 执行查询操作；
/// 4. 返回查询结果。
/// </para > 
/// </summary > 
/// <param name = "sql_hp39" > 查询模连铸炉次信息</param > 
/// <param name = "sql_hp30" > 查询制造命令有关信息</param > 
/// <returns > 查询结果集</returns > 
=========================================================== </remark > */

BM2F_ENTERACE(pssm21_inq);

int f_pssm21_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*系统日志类定义*/
	CTracer log(__FUNCTION__);	

	int  doFlag = 0;
	int  dateCount = 0;
	int  rc = 0;
	int  rowCount = 0;
	int  i,m = 0;
	CDateTime date_date;
	CDateTime date_start_date;
	CDateTime date_end_date;
	CTimeSpan m_timespan;
	CString factory_div = "";
	CString factory_div_temp = "";
	CString cc_mach_no = "";
	CString cc_mach_no_temp = "";
	CString refine_route_code = "";
	CString plan_date = "";
	CString hot_send_flag = "";
	CString flame_clean = "";
	CString sel_fc = "";
	CString sel_third_flag = "";
	CString sql = "";
	CString sql_order = "";
	CString status_to = "";
	CString status_from = "";
	CDecimal v_lf_num = 0;
	CDecimal v_rh_num = 0;
	CDecimal v_vd_num = 0;
	CDecimal v_xiax_num = 0;
	CDecimal v_jq_num = 0;
	CDecimal v_tuo_num = 0;
	CDecimal v_pono_num = 0;
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";	

	CDbCommand cmd_sql(conn);
	CDbCommand cmd_sel(conn);

	try
	{
		/*传入参数*/
		CString start_date = bcls_rec->Tables[0].Rows[0]["START_DATE"].ToString().Trim();
		CString end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString().Trim(); 
		CString check_flag = bcls_rec->Tables[0].Rows[0]["CHECK_FLAG"].ToString().Trim();
		status_from = bcls_rec->Tables[0].Rows[0]["STATUS_FROM"].ToString();
		status_to = bcls_rec->Tables[0].Rows[0]["STATUS_TO"].ToString();

		date_start_date=CDateTime::Parse(start_date);
		date_end_date = CDateTime::Parse(end_date);
		if (date_start_date>date_end_date)
		{
			sprintf(s.msg,_RES("PSSMS0000188")/*起始日期晚于结束日期。*/);
			throw CApplicationException(-1,s.msg,log.Location);
		}
		m_timespan = date_end_date.Subtract(date_start_date);
		dateCount = m_timespan.Days();
		if (dateCount > 100)
		{
			CFormattable arguments[] = { date_start_date,date_end_date };
			CMessageFormat::Format(s.msg,  "起始日期[{0}]与结束日期[{1}]之间大于100天，请减小时间范围", arguments, 2);
			throw CApplicationException(-1,s.msg,log.Location);
		}

		bcls_ret->Tables[0].set_TableName("TPSSM21");

		bcls_ret->Tables["TPSSM21"].Columns.Add(DT_STRING,"FACTORY_DIV");
		bcls_ret->Tables["TPSSM21"].Columns["FACTORY_DIV"].set_Caption("炼钢区分");
		bcls_ret->Tables["TPSSM21"].Columns.Add(DT_STRING,"CC_MACH_NO");
		bcls_ret->Tables["TPSSM21"].Columns["CC_MACH_NO"].set_Caption("连铸机号");

		sqlstr = " SELECT DISTINCT FACTORY_DIV,CC_MACH_NO FROM TPSSMD8 "
				" ORDER BY FACTORY_DIV, CC_MACH_NO ";
		CDbCommand cmd_sel(sqlstr, conn);
		cmd_sel.ExecuteReader();
		while (cmd_sel.Read())
		{
			factory_div = cmd_sel.GetString(1);
			cc_mach_no = cmd_sel.GetString(2);
			
			bcls_ret->Tables["TPSSM21"].Rows.Add();

			bcls_ret->Tables["TPSSM21"].Rows[m]["FACTORY_DIV"] = factory_div;
			bcls_ret->Tables["TPSSM21"].Rows[m]["CC_MACH_NO"] = cc_mach_no;
			m++;
		}
		cmd_sel.Close();

		if(check_flag == "1")
		{
			sql = " SELECT SUM(DECODE(INSTR(A.REFINE_DIV,'L'),0,0,1)) AS LF_NUM,  "
				" SUM(DECODE(INSTR(A.REFINE_DIV,'R'),0,0,1)) AS RH_NUM,  "
				" SUM(DECODE(INSTR(A.REFINE_DIV,'V'),0,0,1)) AS VD_NUM,  "
				" SUM(DECODE(HOT_SEND_FLAG,'0',1,0)) AS XIAX_NUM,  "
				" SUM(DECODE(LENGTH(A.FLAME_CLEAN_DIV),0,0,1)) AS JQ_NUM,  "
				" SUM(DECODE(B.THIRD_OFF_FLAG,NULL,0,1)) AS TUO_NUM,  "
				" COUNT(1) AS PONO_NUM  "
				" FROM TPSSM01 A LEFT JOIN TPMOES11 B ON A.ST_NO = B.ST_NO  "
				" WHERE A.FACTORY_DIV	= @factory_div "
				"   AND A.PLAN_DATE		= @plan_date "
				"   AND A.CC_MACH_NO	= @cc_mach_no ";
			if(status_from.Trim()!= "")
			{
				sql  += " AND  A.PONO_STATUS >= @status_from ";  
			}
			if(status_to.Trim()!= "")
			{
				sql  += " AND  A.PONO_STATUS <= @status_to ";  
			}
		}
		else
		{
			sql = " SELECT SUM(DECODE(INSTR(A.REFINE_DIV,'L'),0,0,1)) AS LF_NUM,  "
				" SUM(DECODE(INSTR(A.REFINE_DIV,'R'),0,0,1)) AS RH_NUM,  "
				" SUM(DECODE(INSTR(A.REFINE_DIV,'V'),0,0,1)) AS VD_NUM,  "
				" SUM(DECODE(HOT_SEND_FLAG,'0',1,0)) AS XIAX_NUM,  "
				" SUM(DECODE(LENGTH(A.FLAME_CLEAN_DIV),0,0,1)) AS JQ_NUM,  "
				" SUM(DECODE(B.THIRD_OFF_FLAG,NULL,0,1)) AS TUO_NUM,  "
				" COUNT(1) AS PONO_NUM "
				" FROM TPSSM01 A LEFT JOIN TPMOES11 B ON A.ST_NO = B.ST_NO  "
				" WHERE A.FACTORY_DIV	= @factory_div "
				"   AND A.PLAN_DATE		= @plan_date "
				"   AND A.CC_MACH_NO	= @cc_mach_no ";
			if(status_from.Trim()!= "")
			{
				sql  += " AND  PONO_STATUS >= @status_from ";  
			}
			if(status_to.Trim()!= "")
			{
				sql  += " AND  PONO_STATUS <= @status_to ";  
			}
		}

		for (int i = 0; i < bcls_ret->Tables["TPSSM21"].Rows.get_Count(); i++)
		{
			factory_div = bcls_ret->Tables["TPSSM21"].Rows[i]["FACTORY_DIV"].ToString();
			cc_mach_no = bcls_ret->Tables["TPSSM21"].Rows[i]["CC_MACH_NO"].ToString();

			////Log::Debug("", __FUNCTION__, "factory_div = [{0}]", factory_div);
			////Log::Debug("", __FUNCTION__, "cc_mach_no = [{0}]", cc_mach_no);

			date_date = date_start_date;
			while (date_end_date.Subtract(date_date).Days() >= 0)
			{
				plan_date = date_date.ToString("yyyyMMdd");

				////Log::Debug("", __FUNCTION__, "plan_date = [{0}]", plan_date);

				v_lf_num = 0;
				v_rh_num = 0;
				v_xiax_num	= 0;
				v_jq_num	= 0;
				v_tuo_num	= 0;
				v_pono_num	= 0;

				cmd_sql.SetCommandText(sql);
				cmd_sql.Parameters.Set("factory_div", factory_div);
				cmd_sql.Parameters.Set("plan_date", plan_date);
				cmd_sql.Parameters.Set("cc_mach_no", cc_mach_no);
				cmd_sql.Parameters.Set("status_from", status_from);
				cmd_sql.Parameters.Set("status_to", status_to);
				
				cmd_sql.ExecuteReader();
				if (cmd_sql.Read())
				{
					v_lf_num = cmd_sql.GetDecimal(1);
					v_rh_num = cmd_sql.GetDecimal(2);
					v_vd_num = cmd_sql.GetDecimal(3);
					v_xiax_num	= cmd_sql.GetDecimal(4);
					v_jq_num	= cmd_sql.GetDecimal(5);
					v_tuo_num	= cmd_sql.GetDecimal(6);
					v_pono_num	= cmd_sql.GetDecimal(7);
				}
				cmd_sql.Close();

				////Log::Debug("", __FUNCTION__, "v_pono_num = [{0}]", v_pono_num);
				if (i == 0)
				{
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "LF");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "LF"].set_Caption("LF");
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "RH");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "RH"].set_Caption("RH");
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "VD");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "VD"].set_Caption("VD");
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "3TUO");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "3TUO"].set_Caption("三脱");
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "UNLINE");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "UNLINE"].set_Caption("下线");
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "FC");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "FC"].set_Caption("机清");
					bcls_ret->Tables["TPSSM21"].Columns.Add(DT_INT32, plan_date + "COUNT");
					bcls_ret->Tables["TPSSM21"].Columns[plan_date + "COUNT"].set_Caption("总数");
				}

				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "COUNT"] = v_pono_num;
				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "LF"] = v_lf_num;
				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "RH"] = v_rh_num;
				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "VD"] = v_vd_num;
				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "UNLINE"] = v_xiax_num;
				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "FC"] = v_jq_num;
				bcls_ret->Tables["TPSSM21"].Rows[i][plan_date + "3TUO"] = v_tuo_num;

				date_date = date_date.AddDays(1);
			}
		}
		////Log::Trace("",__FUNCTION__,"plan_date=[{0}]--------END",plan_date);
		
		/*处理成功*/
		strcpy(s.msg,_RES("GCRSS0000002"));
	}


	/*捕获数据库操作异常*/
	catch(CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = ex.GetMsg() + "\r\n" + sqlstr;
		/*返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应*/
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);	
		s.flag = -1;
		/*数据库异常时返回-1，事务将被回滚*/
		doFlag = -1;   
	}
	/*捕获应用错误*/
	catch(CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;

	return(doFlag);
}
