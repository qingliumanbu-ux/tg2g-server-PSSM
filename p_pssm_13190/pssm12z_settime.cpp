/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"
/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM09画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12z_settime)

int f_pssm12z_settime(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString deal_flag = "";
	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	CString maxseq = "";
	int		TotalRecordCount = 0;
	int		row_count = 0;
	int		row_count2 = 0;
	CDecimal heatz_count = 0;
	CDecimal heatb_count = 0;
	CDecimal heate_count = 0;
	CModel tpssm12z("TPSSM12Z");
	CModel tpssm12("TPSSM12");

	CDbCommand cmd_inq(conn);
	CDataTable tb_tep0003("TEP0002");

	try
	{
		//deal_flag = bcls_rec->Tables[0].Rows[0]["DEAL_FLAG"].ToString().Trim();
		row_count = bcls_rec->Tables["TIME"].Rows.get_Count();
		Log::Trace("", __FUNCTION__, "row_count=[{0}]", row_count);

		sqlstr = " SELECT * from  tep0002 where CODE_CLASS='PSAL2N' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(tb_tep0003);
		cmd_inq.Close();

		//tpssm12z.MergeFrom(bcls_rec->Tables[0].Rows[i]);

		if (row_count > 0)
		{
			for (int i = 0; i < row_count; i++)
			{
				tpssm12z.Reset();
				tpssm12z["ID_SJ"] = bcls_rec->Tables["TIME"].Rows[i]["ID_SJ"].ToString().TrimOrBlank();
				tpssm12z["PROC_NO"] = bcls_rec->Tables["TIME"].Rows[i]["PROC_NO"].ToString().TrimOrBlank();
				tpssm12z["GRADE_ID"] = bcls_rec->Tables["TIME"].Rows[i]["GRADE_ID"].ToString().TrimOrBlank();
				tpssm12z["START_TIME_REAL"] = bcls_rec->Tables["TIME"].Rows[i]["START_TIME_REAL"].ToString().TrimOrBlank();
				tpssm12z["END_TIME_REAL"] = bcls_rec->Tables["TIME"].Rows[i]["END_TIME_REAL"].ToString().TrimOrBlank();
				tpssm12z["HEAT_NO1"] = bcls_rec->Tables["TIME"].Rows[i]["HEAT_NO1"].ToString().TrimOrBlank();
				tpssm12z["HEAT_NO2"] = bcls_rec->Tables["TIME"].Rows[i]["HEAT_NO2"].ToString().TrimOrBlank();
				tpssm12z["HEAT_NO3"] = bcls_rec->Tables["TIME"].Rows[i]["HEAT_NO3"].ToString().TrimOrBlank();

				Log::Trace("", __FUNCTION__, "START_TIME_REAL=[{0}]", tpssm12z["START_TIME_REAL"]);
				Log::Trace("", __FUNCTION__, "END_TIME_REAL=[{0}]", tpssm12z["END_TIME_REAL"]);
				Log::Trace("", __FUNCTION__, "ID_SJ=[{0}]", tpssm12z["ID_SJ"]);
				Log::Trace("", __FUNCTION__, "HEAT_NO1=[{0}]", tpssm12z["HEAT_NO1"]);
				Log::Trace("", __FUNCTION__, "HEAT_NO2=[{0}]", tpssm12z["HEAT_NO2"]);
				Log::Trace("", __FUNCTION__, "HEAT_NO3=[{0}]", tpssm12z["HEAT_NO3"]);
				
				tpssm12z.Update("START_TIME_REAL,END_TIME_REAL,HEAT_NO1,HEAT_NO2,HEAT_NO3", "ID_SJ");
			}
		}

		sqlstr = " SELECT * FROM TPSSM12Z WHERE END_TIME_REAL > TO_CHAR(SYSDATE - 1 , 'YYYYMMDDHH24MISS') OR ( END_TIME_REAL = ' ' AND START_TIME_REAL > TO_CHAR(SYSDATE - 1 , 'YYYYMMDDHH24MISS')) OR START_TIME_REAL = ' ' order by dev_code asc, PROC_NO desc "
			;

		//分页获取
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SMELT_MODE2_DESC");

		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			tpssm12z.Reset();
			tpssm12.Reset();
			heatz_count = 0;
			heatb_count = 0;
			heate_count = 0;
			tpssm12z.MergeFrom(bcls_ret->Tables[0].Rows[i]);
			if (tpssm12z["HEAT_NO"].ToString().Trim() == "")
			{
				continue;
			}
			heatz_count = tpssm12z.QueryCount("HEAT_NO");

			tpssm12["SM_PLAN_NO"] = tpssm12z["SM_PLAN_NO"];
			sqlstr = " SELECT count(1) FROM TPSSM12 WHERE SM_PLAN_NO = @SM_PLAN_NO AND DEV_CODE LIKE 'B%' order by CHARGE_NO asc";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString().Trim());
			heatb_count = cmd_inq.ExecuteScalar();
			cmd_inq.Close();

			sqlstr = " SELECT count(1) FROM TPSSM12 WHERE SM_PLAN_NO = @SM_PLAN_NO AND DEV_CODE LIKE 'E%' order by CHARGE_NO asc";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString().Trim());
			heate_count = cmd_inq.ExecuteScalar();
			cmd_inq.Close();

			if (heatz_count == 1 && heate_count >= 1 && heatb_count == 0)
			{
				for (int i_step = 0; i_step < tb_tep0003.Rows.get_Count(); i_step++)
				{
					if (tb_tep0003.Rows[i_step]["CODE"].ToString() == "2")
					{
						bcls_ret->Tables[0].Rows[i]["SMELT_MODE2_DESC"] = tb_tep0003.Rows[i_step]["CODE_DESC_1_CONTENT"].ToString();
					}
				}
				bcls_ret->Tables[0].Rows[i]["SMELT_MODE2"] = "2";
			}

			else if (heatz_count == 1 && heate_count == 0 && heatb_count >= 1)
			{
				for (int i_step = 0; i_step < tb_tep0003.Rows.get_Count(); i_step++)
				{
					if (tb_tep0003.Rows[i_step]["CODE"].ToString() == "6")
					{
						bcls_ret->Tables[0].Rows[i]["SMELT_MODE2_DESC"] = tb_tep0003.Rows[i_step]["CODE_DESC_1_CONTENT"].ToString();
					}
				}
				bcls_ret->Tables[0].Rows[i]["SMELT_MODE2"] = "6";
			}

			else if (heatz_count == 2)// && heate_count == 0 && heatb_count == 0
			{
				for (int i_step = 0; i_step < tb_tep0003.Rows.get_Count(); i_step++)
				{
					if (tb_tep0003.Rows[i_step]["CODE"].ToString() == "3")
					{
						bcls_ret->Tables[0].Rows[i]["SMELT_MODE2_DESC"] = tb_tep0003.Rows[i_step]["CODE_DESC_1_CONTENT"].ToString();
					}
				}
				bcls_ret->Tables[0].Rows[i]["SMELT_MODE2"] = "3";
			}

			else if (heatz_count >= 3)// && heate_count == 0 && heatb_count == 0
			{
				for (int i_step = 0; i_step < tb_tep0003.Rows.get_Count(); i_step++)
				{
					if (tb_tep0003.Rows[i_step]["CODE"].ToString() == "7")
					{
						bcls_ret->Tables[0].Rows[i]["SMELT_MODE2_DESC"] = tb_tep0003.Rows[i_step]["CODE_DESC_1_CONTENT"].ToString();
					}
				}
				bcls_ret->Tables[0].Rows[i]["SMELT_MODE2"] = "7";
			}

			else
			{
				bcls_ret->Tables[0].Rows[i]["SMELT_MODE2_DESC"] = "未维护路径";
				bcls_ret->Tables[0].Rows[i]["SMELT_MODE2"] = " ";
			}

		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
