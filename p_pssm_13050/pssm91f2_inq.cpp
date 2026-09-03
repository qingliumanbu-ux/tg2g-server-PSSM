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
BM2F_ENTERACE(pssm91f2_inq)

int f_pssm91f2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer LOG(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_order = "";
	int		TotalRecordCount = 0;
	CString run_signal = ""; //运转信号
	CString cast_no = "";
	CString cast_div_no = "";
	int cut_num = 0; //切断数	
	int prcut_num = 0;//实绩切断数
	CString heat_no = "";
	CString pono = "";
	CString factory_div = "";
	CDecimal moltiron_wt = 0; //钢水重量
	CDecimal cast_steel_wt = 0; //浇铸钢水重量
	CString dev_code_3 = "";
	CString dev_code_5 = "";
	CDecimal area_id = 0;
	CString history_flag = "";

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm11("TPSSM11");
	CModel tpssm03("TPSSM03");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm34_inq(conn);
	CDbCommand cmd_tmmsm_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tmmsm21_inq(conn);
	CDbCommand cmd_tmmsm31_inq(conn);

	try
	{
		try
		{//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;
		}


		//--------------------------------
		//获取传入参数
		tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		history_flag = bcls_rec->Tables[0].Rows[0]["HISTORY_FLAG"].ToString();
		/* ***** 打印输入参数 ***** */
		//LOG::Info("", __FUNCTION__, "tpssm01.PONO  =[{0}]", tpssm11["PONO"].ToString());
		//LOG::Info("", __FUNCTION__, "factory_div = [{0}]", tpssm11["FACTORY_DIV"].ToString());
		//LOG::Info("", __FUNCTION__, "history_flag = [{0}]", history_flag);
		//LOG::Info("", __FUNCTION__, "pono_status       = [{0}]", tpssm11["PONO_STATUS"].ToDecimal().ToInt32());


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			if (history_flag == "false" || history_flag == "0")
			{
				sqlstr_count = " SELECT COUNT(1) "
					"  FROM TPSSM11 A   "
					"  LEFT JOIN TPSSM12 C ON A.SM_PLAN_NO = C.SM_PLAN_NO AND C.AREA_ID = '5'  "
					"  WHERE A.PONO_STATUS>= 18 "
					;
				sqlstr = "  SELECT A.*, C.START_TIME_REAL  FROM TPSSM11 A   "
					"  LEFT JOIN TPSSM12 C ON A.SM_PLAN_NO = C.SM_PLAN_NO AND C.AREA_ID = '5'  "
					"  WHERE A.PONO_STATUS >= 18 "
					;
			}
			else
			{
				sqlstr_count = "SELECT COUNT(1) "
					"  FROM TPSSM41 A   "
					"  LEFT JOIN TPSSM42 C ON A.SM_PLAN_NO = C.SM_PLAN_NO AND C.AREA_ID = '5'  "
					"  WHERE 1=1 "
					;
				sqlstr = " SELECT A.*, C.START_TIME_REAL  "
					"   FROM TPSSM41 A "
					"  LEFT JOIN TPSSM42 C ON A.SM_PLAN_NO = C.SM_PLAN_NO AND C.AREA_ID = '5'  "
					"  WHERE 1=1"
					;
			}
			if (tpssm11["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr += " AND A.FACTORY_DIV	= @tpssm11.FACTORY_DIV";
			}
			if (tpssm11["PONO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND A.PONO LIKE '%'||@tpssm11.PONO||'%' ";
			}
			if (tpssm11["HEAT_NO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND A.HEAT_NO	LIKE '%'||@tpssm11.HEAT_NO||'%' ";
			}


			if (tpssm11["PONO_STATUS"].ToDecimal() > 0)
			{
				sqlstr_temp += " AND A.PONO_STATUS	= @tpssm11.PONO_STATUS";
			}
			if (history_flag == "false" || history_flag == "0")
			{
				if (sqlstr_temp.Trim() == "")
				{
					sqlstr_temp += " AND A.PONO_STATUS	<91";
				}
			}


			sqlstr_order += " ORDER BY  A.PONO_STATUS ";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_order;
			break;
		}
		cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
		cmd_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
		cmd_inq.Parameters.Set("tpssm11.PONO_STATUS", tpssm11["PONO_STATUS"].ToDecimal());


		cmd_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_inq.ExecuteScalar().ToInt32();
		//分页获取
		//LOG::Info("", __FUNCTION__, "sqlstr       = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_inq.Close();

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "RUN_SIGNAL"); //增加运转信号查询  chejs/20151012
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_NO_2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CUT_NUM"); //切断数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRCUT_NUM"); //实绩切断数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MOLTIRON_WT"); //钢水重量
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CAST_STEEL_WT"); //浇铸钢水重量

		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			//LOG::Info("", __FUNCTION__, "i = [{0}]", i);
			cast_no = bcls_ret->Tables[0].Rows[i]["CAST_NO"].ToString();
			cast_div_no = bcls_ret->Tables[0].Rows[i]["CAST_DIV_NO"].ToString();
			factory_div = bcls_ret->Tables[0].Rows[i]["FACTORY_DIV"].ToString();
			pono = bcls_ret->Tables[0].Rows[i]["PONO"].ToString();
			heat_no = bcls_ret->Tables[0].Rows[i]["HEAT_NO"].ToString();

			//LOG::Info("", __FUNCTION__, "pono = [{0}]", pono);
			//LOG::Info("", __FUNCTION__, "heat_no = [{0}]", heat_no);

			//add whm 20170823 初始化变量
			moltiron_wt = 0;
			cast_steel_wt = 0;

			sqlstr = "SELECT MAX(RUN_SIGNAL) RUN_SIGNAL FROM TPSSM34 WHERE FACTORY_DIV = @FACTORY_DIV AND PONO = @PONO ";
			//sqlstr = "SELECT RUN_SIGNAL FROM TPSSM34 WHERE FACTORY_DIV = @FACTORY_DIV AND PONO = @PONO ORDER BY REC_CREATE_TIME DESC ";
			cmd_tpssm34_inq.SetCommandText(sqlstr);
			cmd_tpssm34_inq.Parameters.Set("FACTORY_DIV", factory_div);
			cmd_tpssm34_inq.Parameters.Set("PONO", pono);
			cmd_tpssm34_inq.ExecuteReader();
			if (cmd_tpssm34_inq.Read())
			{
				run_signal = cmd_tpssm34_inq.GetString(1);
				//LOG::Info("", __FUNCTION__, "run_signal = [{0}]", run_signal);
			}
			cmd_tpssm34_inq.Close();

			//切断数
			tpssm03["PONO"] = pono;
			tpssm03["FACTORY_DIV"] = factory_div;
			cut_num = tpssm03.QueryCount("PONO,FACTORY_DIV");


			//LOG::Info("", __FUNCTION__, "cut_num = [{0}]", cut_num);
			//实绩切断数
			sqlstr = "select count(*) from tmmsm33 where heat_no = @heat_no ";

			cmd_tmmsm_inq.SetCommandText(sqlstr);
			cmd_tmmsm_inq.Parameters.Set("heat_no", heat_no);
			cmd_tmmsm_inq.ExecuteReader();
			if (cmd_tmmsm_inq.Read())
			{
				prcut_num = cmd_tmmsm_inq.GetInt32(1);
			}
			cmd_tmmsm_inq.Close();

			//LOG::Info("", __FUNCTION__, "prcut_num = [{0}]", prcut_num);
			if (history_flag == "false" || history_flag == "0")
			{
				sqlstr = "select dev_code,area_id from tpssm12 where heat_no = @heat_no and factory_div = @factory_div ";
			}
			else{
				sqlstr = "select dev_code,area_id from tpssm42 where heat_no = @heat_no and factory_div = @factory_div ";
			}


			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("heat_no", heat_no);
			cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
			cmd_tpssm12_inq.ExecuteReader();
			while (cmd_tpssm12_inq.Read())
			{
				area_id = cmd_tpssm12_inq.GetDecimal(2);

				if (area_id == 3)
				{
					dev_code_3 = cmd_tpssm12_inq.GetString(1);
				}

				if (area_id == 5)
				{
					dev_code_5 = cmd_tpssm12_inq.GetString(1);
				}
			}
			cmd_tpssm12_inq.Close();

			//LOG::Info("", __FUNCTION__, "area_id = [{0}]", area_id);
			Log::Info("", __FUNCTION__, "dev_code_3 = [{0}]", dev_code_3);
			Log::Info("", __FUNCTION__, "dev_code_5 = [{0}]", dev_code_5);
			//铁水重量
			if (dev_code_3.SubstringNE(0, 1) == "E")
			{
				sqlstr = "select moltiron_wt from tmmsm20 where heat_no = @heat_no ";
			}
			else if (dev_code_3.SubstringNE(0, 1) == "B")
			{
				sqlstr = "select moltiron_wt from tmmsm21 where heat_no = @heat_no ";
			}
			else if (dev_code_3.SubstringNE(0, 1) == "A")
			{
				sqlstr = "select TO_NUMBER(DECODE(LADLE_PRE_LIQUID_WT,' ','0',LADLE_PRE_LIQUID_WT)) from tmmsm27 where heat_no = @heat_no ";
			}
			else
			{
				CFormattable arguments[] = { heat_no, dev_code_3 };
				CMessageFormat::Format(s.msg, "熔炼号[{0}]冶炼设备代码[{1}]有误，请联系系统开发人员。", arguments, 2);
				throw CApplicationException(-1, s.msg, LOG.Location);
			}

			cmd_tmmsm21_inq.SetCommandText(sqlstr);
			cmd_tmmsm21_inq.Parameters.Set("heat_no", heat_no);
			cmd_tmmsm21_inq.ExecuteReader();
			if (cmd_tmmsm21_inq.Read())
			{
				moltiron_wt = cmd_tmmsm21_inq.GetDecimal(1);
			}
			cmd_tmmsm21_inq.Close();


			//LOG::Info("", __FUNCTION__, "moltiron_wt = [{0}]", moltiron_wt);

			//浇铸钢水重量
			if (dev_code_5.SubstringNE(0, 1) == "C" || dev_code_5.SubstringNE(0, 1) == "M")
			{
				sqlstr = "select LADLE_ARRIVE_WT - LADLE_LEAVE_WT from tmmsm31 where heat_no = @heat_no ";
			}
			else if (dev_code_5.SubstringNE(0, 1) == "I")
			{
				sqlstr = "select PUR_WT from tmmsm41 where heat_no = @heat_no ";
			}
			else
			{
				CFormattable arguments[] = { heat_no, dev_code_5 };
				CMessageFormat::Format(s.msg, "熔炼号[{0}]连铸设备代码[{1}]有误，请联系系统开发人员。", arguments, 2);
				throw CApplicationException(-1, s.msg, LOG.Location);
			}

			cmd_tmmsm31_inq.SetCommandText(sqlstr);
			cmd_tmmsm31_inq.Parameters.Set("heat_no", heat_no);
			cmd_tmmsm31_inq.ExecuteReader();
			if (cmd_tmmsm31_inq.Read())
			{
				cast_steel_wt = cmd_tmmsm31_inq.GetDecimal(1);
			}
			cmd_tmmsm31_inq.Close();


			//LOG::Info("", __FUNCTION__, "cast_steel_wt = [{0}]", cast_steel_wt);


			//LOG::Info("", __FUNCTION__, "cast_no = [{0}]", cast_no);
			bcls_ret->Tables[0].Rows[i]["RUN_SIGNAL"] = run_signal;
			bcls_ret->Tables[0].Rows[i]["CAST_NO_2"] = cast_no + "-" + cast_div_no;
			bcls_ret->Tables[0].Rows[i]["CUT_NUM"] = cut_num;
			bcls_ret->Tables[0].Rows[i]["PRCUT_NUM"] = prcut_num;
			bcls_ret->Tables[0].Rows[i]["MOLTIRON_WT"] = moltiron_wt;
			bcls_ret->Tables[0].Rows[i]["CAST_STEEL_WT"] = cast_steel_wt;
		}

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;


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
