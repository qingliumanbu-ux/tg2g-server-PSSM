/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2016-01-14 19:13:56
Description: 炼钢计划履历一览查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/




/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 故障信息一览查询
/// <para>
/// 1.根据factory_div,时间等条件进行出钢计划查询。
///
/// </para>
/// <para>数据库表：TPSSM99履历信息记录表)     </para>
/// <para>主调用函数：前台PSSM99画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm35f4_inq)

int f_pssm35f4_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString str = "";
	CString sqlstr_count = "";
	CString cs_column1 = "", cs_column2 = "";
	CString sqlstr_temp_where = "";
	CString	prod_date_from = "";
	CString	prod_date_to = "";
	CString	start_time = "";
	CString	confrm_time = "";
	CString colname = " ";
	CString colname2 = " ";
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag = "0";
	CString cast_no_show = "";
	CString proc_no = "";    /* 生产处理号 */
	CString online_flag = "";
	int		TotalRecordCount = 0;
	int fetchRowCount = 0;
	int first_srf = 0; //精炼工序的第一个charge_no

	CDecimal i_count =0;


	//系统的分页类信息。
	CModel tpssm35("TPSSM35");
	CDbCommand cmd_inq(conn);
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO1");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE1");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO2");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE2");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO3");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE3");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO4");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE4");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO5");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE5");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO6");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE6");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO7");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE7");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO8");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE8");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO9");
	bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATE9");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "RET_TIME");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "REMARK");
	try
	{
		tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		sqlstr = "  SELECT *  FROM TPSSM35 WHERE HEAT_NO = @cs_heat_no  order by RET_HEAT_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("cs_heat_no", tpssm35["HEAT_NO"].ToString().Trim());
		cmd_inq.ExecuteReader();
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"].ToString().Trim();
		while (cmd_inq.Read())
		{
			tpssm35.Reset();
			cmd_inq.Fetch(tpssm35);
			if (i_count==0)
			{
				bcls_ret->Tables[0].Rows[0]["RET_HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString().Trim();
				bcls_ret->Tables[0].Rows[0]["RATE"] = tpssm35["RATE"].ToDecimal()*100;
				bcls_ret->Tables[0].Rows[0]["RET_TIME"] = tpssm35["RET_TIME"].ToString().Trim();
				bcls_ret->Tables[0].Rows[0]["REMARK"] = tpssm35["REMARK"].ToString().Trim();
			}
			else
			{
				cs_column1 = "RET_HEAT_NO" + (i_count).ToString();
				cs_column2 = "RATE"+ (i_count).ToString();
				bcls_ret->Tables[0].Rows[0][cs_column1] = tpssm35["RET_HEAT_NO"].ToString().Trim();
				bcls_ret->Tables[0].Rows[0][cs_column2] = tpssm35["RATE"].ToDecimal() * 100;
			}

			i_count=i_count+1;
		}
		cmd_inq.Close();
		


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
