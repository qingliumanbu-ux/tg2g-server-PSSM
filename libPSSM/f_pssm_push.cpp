/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-16
Version:1.0
Description:
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件



BM2_FUNCTION_EXPORT
int f_pssm_push(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int bofnexttime = 9 * 60;
	int bofnexttime1 = 9 * 60;
	int bofnexttime2 = 9 * 60;
	int bofnexttime3 = 9 * 60;
	int casnexttime = 1 * 60;
	int ccnexttime = 2 * 60;
	int castime = 10 * 60;
	int i = 0;
	int j = 0;
	int proc_time = 0;
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";

	CDecimal diff_time = 0;
	CString pono = "";
	CString sm_plan_no = " ";
	CString sm_plan_no2 = " ";
	CString sm_plan_no_nextbof_first = " ";
	CString sqlstr = "";
	CString start_time = "";
	CString end_time = "";
	CString start_time2 = "";
	CString end_time2 = "";
	CString start_time_next = "";
	CString end_time_next = "";
	CString start_time_cas = "";
	CString end_time_cas = "";
	CString start_time_cc = "";
	CString end_time_cc = "";

	CString end_time_last = "";

	CDateTime start_timex;
	CDateTime end_timex;
	CDateTime end_time_lastx;
	CTimeSpan proc_time_dif;

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssmd3_inq(conn);
	CDbCommand cmd_tpssm12_inq2(conn);
	try
	{
		sqlstr = " select std_prep_time from tpssmd3 where dev_code = 'B5'";
		cmd_tpssmd3_inq.SetCommandText(sqlstr);
		cmd_tpssmd3_inq.ExecuteReader();
		if (cmd_tpssmd3_inq.Read())
		{
			bofnexttime1 = cmd_tpssmd3_inq.GetUInt32(1);
			bofnexttime1 = bofnexttime1 * 60;
			//Log::Trace("", __FUNCTION__, "bofnexttime1 = {0},bofnexttime2 = {1},bofnexttime3 = {2}", bofnexttime1, bofnexttime2, bofnexttime3);
		}
		cmd_tpssmd3_inq.Close();

		sqlstr = " select std_prep_time from tpssmd3 where dev_code = 'B6'";
		cmd_tpssmd3_inq.SetCommandText(sqlstr);
		cmd_tpssmd3_inq.ExecuteReader();
		if (cmd_tpssmd3_inq.Read())
		{
			bofnexttime2 = cmd_tpssmd3_inq.GetUInt32(1);
			bofnexttime2 = bofnexttime2 * 60;
		}
		cmd_tpssmd3_inq.Close();

		/*sqlstr = " select std_prep_time from tpssmd3 where dev_code = 'B3'";
		cmd_tpssmd3_inq.SetCommandText(sqlstr);
		cmd_tpssmd3_inq.ExecuteReader();
		if (cmd_tpssmd3_inq.Read())
		{
		bofnexttime3 = cmd_tpssmd3_inq.GetUInt32(1);
		bofnexttime3 = bofnexttime3 * 60;
		}
		cmd_tpssmd3_inq.Close();*/
		bofnexttime1 = 120;
		bofnexttime2 = 120;
		Log::Trace("", __FUNCTION__, "bofnexttime1 = {0},bofnexttime2 = {1},bofnexttime3 = {2}", bofnexttime1, bofnexttime2, bofnexttime3);

		Log::Trace("", __FUNCTION__, "B5");

		sqlstr = " select a.sm_plan_no, a.start_time_real, a.start_time, a.end_time, a.proc_time, b.steel_start_time, b.steel_end_time, b.pono, b.cast_no, b.cast_div_no from tpssm12 a, tpssm11 b where a.dev_code = 'B5' and b.pono_status >= 20 and a.sm_plan_no = b.sm_plan_no and rownum = 1 order by a.start_time_real desc ";
		cmd_tpssm12_inq2.SetCommandText(sqlstr);
		cmd_tpssm12_inq2.ExecuteReader();
		if (cmd_tpssm12_inq2.Read())
		{
			end_time_last = cmd_tpssm12_inq2.GetString(4);
		}
		cmd_tpssm12_inq2.Close();

		sqlstr = " select a.sm_plan_no, a.start_time, a.end_time, a.proc_time, b.steel_start_time, b.steel_end_time, b.pono, b.cast_no, b.cast_div_no from tpssm12 a, tpssm11 b where a.dev_code = 'B5' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and rownum = 1 order by a.start_time ";
		cmd_tpssm12_inq2.SetCommandText(sqlstr);
		cmd_tpssm12_inq2.ExecuteReader();
		if (cmd_tpssm12_inq2.Read())
		{
			sm_plan_no_nextbof_first = cmd_tpssm12_inq2.GetString(1);
		}
		cmd_tpssm12_inq2.Close();

		sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,a.proc_time,b.steel_start_time,b.steel_end_time,b.pono,b.cast_no,b.cast_div_no from tpssm12 a,tpssm11 b where a.dev_code = 'B5' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and rownum = 1 order by cast_no,cast_div_no ";
		cmd_tpssm12_inq2.SetCommandText(sqlstr);
		cmd_tpssm12_inq2.ExecuteReader();
		if (cmd_tpssm12_inq2.Read())
		{
			sm_plan_no = cmd_tpssm12_inq2.GetString(1);
			if (end_time_last.Trim() != "" && sm_plan_no_nextbof_first != sm_plan_no)
			{

				proc_time = cmd_tpssm12_inq2.GetInt32(4);

				start_time = (CDateTime::Parse(end_time_last).AddSeconds(120).ToString("yyyyMMddHHmmss"));
				if (proc_time == 0)
				{
					end_time = (CDateTime::Parse(start_time).AddMinutes(30).ToString("yyyyMMddHHmmss"));
				}
				else
				{
					end_time = (CDateTime::Parse(start_time).AddMinutes(proc_time).ToString("yyyyMMddHHmmss"));
				}

				tpssm12["START_TIME"] = start_time;
				tpssm12["END_TIME"] = end_time;
				tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["DEV_CODE"] = "B5";
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11["STEEL_START_TIME"] = start_time;

				v_update = "START_TIME,END_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

				if (tpssm12.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm12 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				v_update = "STEEL_START_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO"; //查询条件

				if (tpssm11.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm11 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				end_time_last = " ";
				sm_plan_no_nextbof_first = " ";
			}
		}

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,a.proc_time,b.steel_start_time,b.steel_end_time,b.pono,b.cast_no,b.cast_div_no from tpssm12 a,tpssm11 b where a.dev_code = 'B5' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no order by cast_no,cast_div_no "
				;
			Log::Trace("", __FUNCTION__, "sqlstr = {0}", sqlstr);
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_tpssm11_inq.Close();
		diff_time = 0;
		for (i = 0; i < bcls_ret->Tables[0].Rows.get_Count() - 1; i++)
		{
			start_time = bcls_ret->Tables[0].Rows[i]["START_TIME"].ToString();
			end_time = bcls_ret->Tables[0].Rows[i]["END_TIME"].ToString();
			if (tpssm12["START_TIME"].ToString().Trim() != "" && tpssm12["END_TIME"].ToString().Trim() != "")
			{
				start_time = tpssm12["START_TIME"];
				end_time = tpssm12["END_TIME"];
			}

			start_time_next = bcls_ret->Tables[0].Rows[i + 1]["START_TIME"].ToString();
			end_time_next = bcls_ret->Tables[0].Rows[i + 1]["END_TIME"].ToString();
			if (end_time > start_time_next)
			{
				//diff_time = 0;
				start_timex = CDateTime::Parse(start_time_next);
				end_timex = CDateTime::Parse(end_time);
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalSeconds();
				diff_time = diff_time + bofnexttime1;
				Log::Trace("", __FUNCTION__, "diff_time = {0}", diff_time);
				Log::Trace("", __FUNCTION__, "pono = {0},pono2 = {1}", bcls_ret->Tables[0].Rows[i]["PONO"].ToString(), bcls_ret->Tables[0].Rows[i + 1]["PONO"].ToString());
				j = i + 1;
				/*for (j = i + 1; j < bcls_ret->Tables[0].Rows.get_Count(); j++)
				{*/
				Log::Trace("", __FUNCTION__, "i = {0},j = {1}", i, j);

				start_time2 = bcls_ret->Tables[0].Rows[j]["START_TIME"].ToString();
				end_time2 = bcls_ret->Tables[0].Rows[j]["END_TIME"].ToString();
				sm_plan_no = bcls_ret->Tables[0].Rows[j]["SM_PLAN_NO"].ToString();
				pono = bcls_ret->Tables[0].Rows[j]["PONO"].ToString();
				tpssm12["START_TIME"] = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
				tpssm12["END_TIME"] = (CDateTime::Parse(end_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
				tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["DEV_CODE"] = "B5";
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11["STEEL_START_TIME"] = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");

				Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", tpssm12["SM_PLAN_NO"].ToString());
				Log::Trace("", __FUNCTION__, "pono = {0}", pono);
				Log::Trace("", __FUNCTION__, "start_time2 = {0}", tpssm12["START_TIME"].ToString());
				Log::Trace("", __FUNCTION__, "end_time2 = {0}", tpssm12["END_TIME"].ToString());


				v_update = "START_TIME,END_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

				if (tpssm12.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm12 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//tpssm12["DEV_CODE"] = "A1";
				//if (tpssm12.QueryCount("DEV_CODE,SM_PLAN_NO") > 0)
				//{
				//	tpssm12.Query("DEV_CODE,SM_PLAN_NO");
				//	tpssm12["START_TIME"] = (CDateTime::Parse(end_time2).AddSeconds((diff_time + casnexttime).ToDouble())).ToString("yyyyMMddHHmmss");
				//	tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12["START_TIME"].ToString()).AddSeconds(castime)).ToString("yyyyMMddHHmmss");
				//	v_update = "START_TIME,END_TIME";//修改字段信息。
				//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

				//	if (tpssm12.Update(v_update, v_condi) != true)
				//	{
				//		strcpy(s.msg, "Update tpssm12,c failed.");
				//		throw CApplicationException(-1, s.msg, log.Location);
				//	}
				//}

				v_update = "STEEL_START_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO"; //查询条件

				if (tpssm11.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm11 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				/*if (j != bcls_ret->Tables[0].Rows.get_Count() - 1)
				{
				if (tpssm12["END_TIME"].ToString() < bcls_ret->Tables[0].Rows[j + 1]["START_TIME"].ToString())
				{
				break;
				}
				}
				}*/
			}
		}

		Log::Trace("", __FUNCTION__, "B6");

		sqlstr = " select a.sm_plan_no, a.start_time_real, a.start_time, a.end_time, a.proc_time, b.steel_start_time, b.steel_end_time, b.pono, b.cast_no, b.cast_div_no from tpssm12 a, tpssm11 b where a.dev_code = 'B6' and b.pono_status >= 20 and a.sm_plan_no = b.sm_plan_no and rownum = 1 order by a.start_time_real desc ";
		cmd_tpssm12_inq2.SetCommandText(sqlstr);
		cmd_tpssm12_inq2.ExecuteReader();
		if (cmd_tpssm12_inq2.Read())
		{
			end_time_last = cmd_tpssm12_inq2.GetString(4);
		}
		cmd_tpssm12_inq2.Close();

		sqlstr = " select a.sm_plan_no, a.start_time, a.end_time, a.proc_time, b.steel_start_time, b.steel_end_time, b.pono, b.cast_no, b.cast_div_no from tpssm12 a, tpssm11 b where a.dev_code = 'B6' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and rownum = 1 order by a.start_time ";
		cmd_tpssm12_inq2.SetCommandText(sqlstr);
		cmd_tpssm12_inq2.ExecuteReader();
		if (cmd_tpssm12_inq2.Read())
		{
			sm_plan_no_nextbof_first = cmd_tpssm12_inq2.GetString(1);
		}
		cmd_tpssm12_inq2.Close();

		sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,a.proc_time,b.steel_start_time,b.steel_end_time,b.pono,b.cast_no,b.cast_div_no from tpssm12 a,tpssm11 b where a.dev_code = 'B6' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and rownum = 1 order by cast_no,cast_div_no ";
		cmd_tpssm12_inq2.SetCommandText(sqlstr);
		cmd_tpssm12_inq2.ExecuteReader();
		if (cmd_tpssm12_inq2.Read())
		{
			sm_plan_no = cmd_tpssm12_inq2.GetString(1);
			if (end_time_last.Trim() != "" && sm_plan_no_nextbof_first != sm_plan_no)
			{
				proc_time = cmd_tpssm12_inq2.GetInt32(4);

				start_time = (CDateTime::Parse(end_time_last).AddSeconds(120).ToString("yyyyMMddHHmmss"));
				if (proc_time == 0)
				{
					end_time = (CDateTime::Parse(start_time).AddMinutes(30).ToString("yyyyMMddHHmmss"));
				}
				else
				{
					end_time = (CDateTime::Parse(start_time).AddMinutes(proc_time).ToString("yyyyMMddHHmmss"));
				}

				tpssm12["START_TIME"] = start_time;
				tpssm12["END_TIME"] = end_time;
				tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["DEV_CODE"] = "B6";
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11["STEEL_START_TIME"] = start_time;

				v_update = "START_TIME,END_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

				if (tpssm12.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm12 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				v_update = "STEEL_START_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO"; //查询条件

				if (tpssm11.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm11 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				end_time_last = " ";
				sm_plan_no_nextbof_first = " ";
			}
		}


		bcls_ret->Tables.Add();
		sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,a.proc_time,b.steel_start_time,b.steel_end_time,b.pono,b.cast_no,b.cast_div_no from tpssm12 a,tpssm11 b where a.dev_code = 'B6' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no order by cast_no,cast_div_no ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[1]);
		cmd_tpssm11_inq.Close();
		diff_time = 0;
		tpssm12.Reset();
		for (i = 0; i < bcls_ret->Tables[1].Rows.get_Count() - 1; i++)
		{
			start_time = bcls_ret->Tables[1].Rows[i]["START_TIME"].ToString();
			end_time = bcls_ret->Tables[1].Rows[i]["END_TIME"].ToString();
			if (tpssm12["START_TIME"].ToString().Trim() != "" && tpssm12["END_TIME"].ToString().Trim() != "")
			{
				start_time = tpssm12["START_TIME"];
				end_time = tpssm12["END_TIME"];
			}
			start_time_next = bcls_ret->Tables[1].Rows[i + 1]["START_TIME"].ToString();
			end_time_next = bcls_ret->Tables[1].Rows[i + 1]["END_TIME"].ToString();
			if (end_time > start_time_next)
			{
				Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
				Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
				Log::Trace("", __FUNCTION__, "start_time_next = {0}", start_time_next);
				Log::Trace("", __FUNCTION__, "end_time_next = {0}", end_time_next);
				//diff_time = 0;
				start_timex = CDateTime::Parse(start_time_next);
				end_timex = CDateTime::Parse(end_time);
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalSeconds();
				diff_time = diff_time + bofnexttime2;
				Log::Trace("", __FUNCTION__, "diff_time = {0}", diff_time);
				Log::Trace("", __FUNCTION__, "pono = {0},pono2 = {1}", bcls_ret->Tables[1].Rows[i]["PONO"].ToString(), bcls_ret->Tables[1].Rows[i + 1]["PONO"].ToString());
				j = i + 1;
				/*for (j = i + 1; j < bcls_ret->Tables[1].Rows.get_Count(); j++)
				{*/
				Log::Trace("", __FUNCTION__, "i = {0},j = {1}", i, j);

				start_time2 = bcls_ret->Tables[1].Rows[j]["START_TIME"].ToString();
				end_time2 = bcls_ret->Tables[1].Rows[j]["END_TIME"].ToString();
				sm_plan_no = bcls_ret->Tables[1].Rows[j]["SM_PLAN_NO"].ToString();
				pono = bcls_ret->Tables[1].Rows[j]["PONO"].ToString();
				tpssm12["START_TIME"] = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
				tpssm12["END_TIME"] = (CDateTime::Parse(end_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
				tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["DEV_CODE"] = "B6";
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11["STEEL_START_TIME"] = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");

				Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", tpssm12["SM_PLAN_NO"].ToString());
				Log::Trace("", __FUNCTION__, "pono = {0}", pono);
				Log::Trace("", __FUNCTION__, "start_time2 = {0}", tpssm12["START_TIME"].ToString());
				Log::Trace("", __FUNCTION__, "end_time2 = {0}", tpssm12["END_TIME"].ToString());

				v_update = "START_TIME,END_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

				if (tpssm12.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm12 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//tpssm12["DEV_CODE"] = "A2";
				//if (tpssm12.QueryCount("DEV_CODE,SM_PLAN_NO") > 0)
				//{
				//	tpssm12.Query("DEV_CODE,SM_PLAN_NO");
				//	tpssm12["START_TIME"] = (CDateTime::Parse(end_time2).AddSeconds((diff_time + casnexttime).ToDouble())).ToString("yyyyMMddHHmmss");
				//	tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12["START_TIME"].ToString()).AddSeconds(castime)).ToString("yyyyMMddHHmmss");
				//	v_update = "START_TIME,END_TIME";//修改字段信息。
				//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

				//	if (tpssm12.Update(v_update, v_condi) != true)
				//	{
				//		strcpy(s.msg, "Update tpssm12,c failed.");
				//		throw CApplicationException(-1, s.msg, log.Location);
				//	}
				//}

				v_update = "STEEL_START_TIME";//修改字段信息。
				v_condi = "SM_PLAN_NO"; //查询条件

				if (tpssm11.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm11 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//}
			}
		}

		//Log::Trace("", __FUNCTION__, "B3");
		//bcls_ret->Tables.Add();
		//sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,a.proc_time,b.steel_start_time,b.steel_end_time,b.pono from tpssm12 a,tpssm11 b where a.dev_code = 'B3' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no order by start_time ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[2]);
		//cmd_tpssm11_inq.Close();
		//diff_time = 0;
		//tpssm12.Reset();
		//for (i = 0; i < bcls_ret->Tables[2].Rows.get_Count() - 1; i++)
		//{
		//	start_time = bcls_ret->Tables[2].Rows[i]["START_TIME"].ToString();
		//	end_time = bcls_ret->Tables[2].Rows[i]["END_TIME"].ToString();
		//	if (tpssm12["START_TIME"].ToString().Trim() != "" && tpssm12["END_TIME"].ToString().Trim() != "")
		//	{
		//		start_time = tpssm12["START_TIME"];
		//		end_time = tpssm12["END_TIME"];
		//	}
		//	start_time_next = bcls_ret->Tables[2].Rows[i + 1]["START_TIME"].ToString();
		//	end_time_next = bcls_ret->Tables[2].Rows[i + 1]["END_TIME"].ToString();

		//	if (end_time > start_time_next)
		//	{
		//		//diff_time = 0;
		//		start_timex = CDateTime::Parse(start_time_next);
		//		end_timex = CDateTime::Parse(end_time);
		//		proc_time_dif = end_timex - start_timex;
		//		diff_time = proc_time_dif.TotalSeconds();
		//		diff_time = diff_time + bofnexttime3;
		//		Log::Trace("", __FUNCTION__, "diff_time = {0}", diff_time);
		//		Log::Trace("", __FUNCTION__, "pono = {0},pono2 = {1}", bcls_ret->Tables[2].Rows[i]["PONO"].ToString(), bcls_ret->Tables[2].Rows[i + 1]["PONO"].ToString());
		//		j = i + 1;
		//		/*for (j = i + 1; j < bcls_ret->Tables[2].Rows.get_Count(); j++)
		//		{*/
		//			Log::Trace("", __FUNCTION__, "i = {0},j = {1}", i, j);

		//			start_time2 = bcls_ret->Tables[2].Rows[j]["START_TIME"].ToString();
		//			end_time2 = bcls_ret->Tables[2].Rows[j]["END_TIME"].ToString();
		//			sm_plan_no = bcls_ret->Tables[2].Rows[j]["SM_PLAN_NO"].ToString();
		//			pono = bcls_ret->Tables[2].Rows[j]["PONO"].ToString();
		//			tpssm12["START_TIME"] = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		//			tpssm12["END_TIME"] = (CDateTime::Parse(end_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		//			tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//			tpssm12["DEV_CODE"] = "B3";
		//			tpssm11["SM_PLAN_NO"] = sm_plan_no;
		//			tpssm11["STEEL_START_TIME"] = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");

		//			Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", tpssm12["SM_PLAN_NO"].ToString());
		//			Log::Trace("", __FUNCTION__, "pono = {0}", pono);
		//			Log::Trace("", __FUNCTION__, "start_time2 = {0}", tpssm12["START_TIME"].ToString());
		//			Log::Trace("", __FUNCTION__, "end_time2 = {0}", tpssm12["END_TIME"].ToString());

		//			v_update = "START_TIME,END_TIME";//修改字段信息。
		//			v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//			if (tpssm12.Update(v_update, v_condi) != true)
		//			{
		//				strcpy(s.msg, "Update tpssm12 failed.");
		//				throw CApplicationException(-1, s.msg, log.Location);
		//			}
		//tpssm12["DEV_CODE"] = "A3";
		//if (tpssm12.QueryCount("DEV_CODE,SM_PLAN_NO") > 0)
		//{
		//	tpssm12["PROC_TIME"] = 10;
		//	tpssm12["START_TIME"] = (CDateTime::Parse(end_time2).AddSeconds((diff_time + casnexttime).ToDouble())).ToString("yyyyMMddHHmmss");
		//	tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12["START_TIME"].ToString()).AddSeconds(castime)).ToString("yyyyMMddHHmmss");
		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.Update(v_update, v_condi) != true)
		//	{
		//		strcpy(s.msg, "Update tpssm12,c failed.");
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

		//			v_update = "STEEL_START_TIME";//修改字段信息。
		//			v_condi = "SM_PLAN_NO"; //查询条件

		//			if (tpssm11.Update(v_update, v_condi) != true)
		//			{
		//				strcpy(s.msg, "Update tpssm11 failed.");
		//				throw CApplicationException(-1, s.msg, log.Location);
		//			}
		//		}
		//	/*}*/
		//}

		//Log::Trace("", __FUNCTION__, "A1");
		//bcls_ret->Tables.Add();
		//tpssm12.Reset();
		//sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,c.proc_time,b.steel_start_time,b.steel_end_time,b.pono from tpssm12 a,tpssm11 b,tpssm12 c where c.dev_code = 'A1' and a.dev_code = 'B1' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by start_time ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[3]);
		//cmd_tpssm11_inq.Close();
		//for (i = 0; i < bcls_ret->Tables[3].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[3].Rows[i]["START_TIME"].ToString();
		//	end_time = bcls_ret->Tables[3].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[3].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[3].Rows[i]["SM_PLAN_NO"].ToString();

		//	sqlstr = "select start_time,end_time from tpssm12 where dev_code = 'A1' and sm_plan_no = @sm_plan_no ";
		//	cmd_tpssm12_inq.SetCommandText(sqlstr);
		//	cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//	cmd_tpssm12_inq.ExecuteReader();
		//	if (cmd_tpssm12_inq.Read())
		//	{
		//		start_time2 = cmd_tpssm12_inq.GetString(1);
		//		end_time2 = cmd_tpssm12_inq.GetString(2);
		//	}
		//	else
		//	{
		//		start_time2 = " ";
		//		end_time2 = " ";
		//	}
		//	cmd_tpssm12_inq.Close();

		//	start_time = (CDateTime::Parse(end_time).AddSeconds(60)).ToString("yyyyMMddHHmmss");
		//	end_time = (CDateTime::Parse(end_time).AddSeconds(diff_time.ToDouble() + 60)).ToString("yyyyMMddHHmmss");

		//	if (start_time < start_time2)
		//	{
		//		start_time = start_time2;
		//		end_time = end_time2;
		//	}
		//	
		//	tpssm12["START_TIME"] = start_time;
		//	tpssm12["END_TIME"] = end_time;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "A1";
		//	

		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", tpssm12["SM_PLAN_NO"].ToString());
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", tpssm12["START_TIME"].ToString());
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", tpssm12["END_TIME"].ToString());

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}
		//}

		//Log::Trace("", __FUNCTION__, "A2");
		//tpssm12.Reset();
		//bcls_ret->Tables.Add();
		//sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,c.proc_time,b.steel_start_time,b.steel_end_time,b.pono from tpssm12 a,tpssm11 b,tpssm12 c where c.dev_code = 'A2' and a.dev_code = 'B2' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by start_time ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[4]);
		//cmd_tpssm11_inq.Close();
		//for (i = 0; i < bcls_ret->Tables[4].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[4].Rows[i]["START_TIME"].ToString();
		//	end_time = bcls_ret->Tables[4].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[4].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[4].Rows[i]["SM_PLAN_NO"].ToString();

		//	sqlstr = "select start_time,end_time from tpssm12 where dev_code = 'A2' and sm_plan_no = @sm_plan_no ";
		//	cmd_tpssm12_inq.SetCommandText(sqlstr);
		//	cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//	cmd_tpssm12_inq.ExecuteReader();
		//	if (cmd_tpssm12_inq.Read())
		//	{
		//		start_time2 = cmd_tpssm12_inq.GetString(1);
		//		end_time2 = cmd_tpssm12_inq.GetString(2);
		//	}
		//	else
		//	{
		//		start_time2 = " ";
		//		end_time2 = " ";
		//	}
		//	cmd_tpssm12_inq.Close();

		//	start_time = (CDateTime::Parse(end_time).AddSeconds(60)).ToString("yyyyMMddHHmmss");
		//	end_time = (CDateTime::Parse(end_time).AddSeconds(diff_time.ToDouble() + 60)).ToString("yyyyMMddHHmmss");

		//	if (start_time < start_time2)
		//	{
		//		start_time = start_time2;
		//		end_time = end_time2;
		//	}

		//	tpssm12["START_TIME"] = start_time;
		//	tpssm12["END_TIME"] = end_time;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "A2";


		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", tpssm12["SM_PLAN_NO"].ToString());
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", tpssm12["START_TIME"].ToString());
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", tpssm12["END_TIME"].ToString());

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}
		//}

		//Log::Trace("", __FUNCTION__, "A3");
		//tpssm12.Reset();
		//bcls_ret->Tables.Add();
		//sqlstr = " select a.sm_plan_no,a.start_time,a.end_time,c.proc_time,b.steel_start_time,b.steel_end_time,b.pono from tpssm12 a,tpssm11 b,tpssm12 c where c.dev_code = 'A3' and a.dev_code = 'B3' and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by start_time ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[5]);
		//cmd_tpssm11_inq.Close();
		//for (i = 0; i < bcls_ret->Tables[5].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[5].Rows[i]["START_TIME"].ToString();
		//	end_time = bcls_ret->Tables[5].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[5].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[5].Rows[i]["SM_PLAN_NO"].ToString();

		//	sqlstr = "select start_time,end_time from tpssm12 where dev_code = 'A3' and sm_plan_no = @sm_plan_no ";
		//	cmd_tpssm12_inq.SetCommandText(sqlstr);
		//	cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//	cmd_tpssm12_inq.ExecuteReader();
		//	if (cmd_tpssm12_inq.Read())
		//	{
		//		start_time2 = cmd_tpssm12_inq.GetString(1);
		//		end_time2 = cmd_tpssm12_inq.GetString(2);
		//	}
		//	else
		//	{
		//		start_time2 = " ";
		//		end_time2 = " ";
		//	}
		//	cmd_tpssm12_inq.Close();

		//	start_time = (CDateTime::Parse(end_time).AddSeconds(60)).ToString("yyyyMMddHHmmss");
		//	end_time = (CDateTime::Parse(end_time).AddSeconds(diff_time.ToDouble() + 60)).ToString("yyyyMMddHHmmss");

		//	if (start_time < start_time2)
		//	{
		//		start_time = start_time2;
		//		end_time = end_time2;
		//	}

		//	tpssm12["START_TIME"] = start_time;
		//	tpssm12["END_TIME"] = end_time;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "A3";


		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", tpssm12["SM_PLAN_NO"].ToString());
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", tpssm12["START_TIME"].ToString());
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", tpssm12["END_TIME"].ToString());

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}
		//}


		//Log::Trace("", __FUNCTION__, "C1");
		//bcls_ret->Tables.Add();
		//sqlstr = " select c.sm_plan_no, "
		//	" c.start_time, "
		//	" c.end_time, "
		//	" c.proc_time, "
		//	" d.steel_start_time,"
		//	" d.steel_end_time,"
		//	" d.pono,"
		//	" d.lf_end_time,"
		//	" d.lf_end_time_real"
		//	" , decode(trim(d.lf_end_time_real), '', d.lf_end_time, d.lf_end_time_real)end_time1"
		//	" from tpssm12 c"
		//	" right join（select a.sm_plan_no, a.end_time as lf_end_time, a.end_time_real as lf_end_time_real, b.steel_start_time, b.steel_end_time, b.pono"
		//	" from tpssm12 a, tpssm11 b"
		//	" where a.area_id in (3, 4)"
		//	" and b.run_status < 40"
		//	" and a.sm_plan_no = b.sm_plan_no"
		//	" and(a.charge_no, a.sm_plan_no) in"
		//	" (select max(charge_no) - 1 as charge_no, sm_plan_no "
		//	" from tpssm12"
		//	" group by sm_plan_no) ）d on c.sm_plan_no = d.sm_plan_no"
		//    " where c.dev_code = 'C1'"
		//	" order by end_time1 ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[2]);
		//cmd_tpssm11_inq.Close();
		////Log::Trace("", __FUNCTION__, "count = {0}", bcls_ret->Tables[6].Rows.get_Count());
		//tpssm12.Reset();
		//for (i = 0; i < bcls_ret->Tables[2].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[2].Rows[i]["START_TIME"].ToString();//当前炉时间
		//	end_time = bcls_ret->Tables[2].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[2].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[2].Rows[i]["SM_PLAN_NO"].ToString();

		//	if (i == 0)
		//	{
		//		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C1' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C1' and start_time < (select start_time from tpssm12 where dev_code = 'C1' and sm_plan_no = @sm_plan_no))";
		//		cmd_tpssm12_inq.SetCommandText(sqlstr);
		//		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//		cmd_tpssm12_inq.ExecuteReader();
		//		if (cmd_tpssm12_inq.Read())
		//		{
		//			sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		//			start_time2 = cmd_tpssm12_inq.GetString(2);
		//			end_time2 = cmd_tpssm12_inq.GetString(3);
		//		}
		//		else
		//		{
		//			sm_plan_no2 = " ";
		//			start_time2 = " ";
		//			end_time2 = " ";
		//		}
		//		cmd_tpssm12_inq.Close();
		//	}
		//	else
		//	{
		//		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		//		start_time2 = tpssm12["START_TIME"];
		//		end_time2 = tpssm12["END_TIME"];
		//	}

		//	if (start_time2.Trim() == "" || end_time2.Trim() == "")
		//	{
		//		start_time_cc = start_time;
		//		end_time_cc = end_time;
		//	}
		//	else
		//	{
		//		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		//		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		//		if (start_time >= start_time2)
		//		{
		//			start_time_cc = start_time;
		//			end_time_cc = end_time;
		//		}
		//		else
		//		{
		//			start_time_cc = start_time2;
		//			end_time_cc = end_time2;
		//		}
		//	}
		//	Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		//	Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		//	Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		//	Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		//	tpssm12["START_TIME"] = start_time_cc;
		//	tpssm12["END_TIME"] = end_time_cc;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "C1";
		//	tpssm11["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm11["STEEL_END_TIME"] = end_time_cc;

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}

		//	v_update = "STEEL_END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO"; //查询条件

		//	if (tpssm11.Update(v_update, v_condi) != true)
		//	{
		//		strcpy(s.msg, "Update tpssm11 failed.");
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

		//Log::Trace("", __FUNCTION__, "C2");
		//bcls_ret->Tables.Add();
		//sqlstr = " select c.sm_plan_no, "
		//	" c.start_time, "
		//	" c.end_time, "
		//	" c.proc_time, "
		//	" d.steel_start_time,"
		//	" d.steel_end_time,"
		//	" d.pono,"
		//	" d.lf_end_time,"
		//	" d.lf_end_time_real"
		//	" , decode(trim(d.lf_end_time_real), '', d.lf_end_time, d.lf_end_time_real)end_time1"
		//	" from tpssm12 c"
		//	" right join（select a.sm_plan_no, a.end_time as lf_end_time, a.end_time_real as lf_end_time_real, b.steel_start_time, b.steel_end_time, b.pono"
		//	" from tpssm12 a, tpssm11 b"
		//	" where a.area_id in (3, 4)"
		//	" and b.run_status < 40"
		//	" and a.sm_plan_no = b.sm_plan_no"
		//	" and(a.charge_no, a.sm_plan_no) in"
		//	" (select max(charge_no) - 1 as charge_no, sm_plan_no "
		//	" from tpssm12"
		//	" group by sm_plan_no) ）d on c.sm_plan_no = d.sm_plan_no"
		//	" where c.dev_code = 'C2'"
		//	" order by end_time1 ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[3]);
		//cmd_tpssm11_inq.Close();
		////Log::Trace("", __FUNCTION__, "count = {0}", bcls_ret->Tables[6].Rows.get_Count());
		//tpssm12.Reset();
		//for (i = 0; i < bcls_ret->Tables[3].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[3].Rows[i]["START_TIME"].ToString();//当前炉时间
		//	end_time = bcls_ret->Tables[3].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[3].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[3].Rows[i]["SM_PLAN_NO"].ToString();

		//	if (i == 0)
		//	{
		//		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C2' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C2' and start_time < (select start_time from tpssm12 where dev_code = 'C2' and sm_plan_no = @sm_plan_no))";
		//		cmd_tpssm12_inq.SetCommandText(sqlstr);
		//		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//		cmd_tpssm12_inq.ExecuteReader();
		//		if (cmd_tpssm12_inq.Read())
		//		{
		//			sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		//			start_time2 = cmd_tpssm12_inq.GetString(2);
		//			end_time2 = cmd_tpssm12_inq.GetString(3);
		//		}
		//		else
		//		{
		//			sm_plan_no2 = " ";
		//			start_time2 = " ";
		//			end_time2 = " ";
		//		}
		//		cmd_tpssm12_inq.Close();
		//	}
		//	else
		//	{
		//		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		//		start_time2 = tpssm12["START_TIME"];
		//		end_time2 = tpssm12["END_TIME"];
		//	}

		//	if (start_time2.Trim() == "" || end_time2.Trim() == "")
		//	{
		//		start_time_cc = start_time;
		//		end_time_cc = end_time;
		//	}
		//	else
		//	{
		//		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		//		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		//		if (start_time >= start_time2)
		//		{
		//			start_time_cc = start_time;
		//			end_time_cc = end_time;
		//		}
		//		else
		//		{
		//			start_time_cc = start_time2;
		//			end_time_cc = end_time2;
		//		}
		//	}
		//	Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		//	Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		//	Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		//	Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		//	tpssm12["START_TIME"] = start_time_cc;
		//	tpssm12["END_TIME"] = end_time_cc;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "C2";
		//	tpssm11["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm11["STEEL_END_TIME"] = end_time_cc;

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}

		//	v_update = "STEEL_END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO"; //查询条件

		//	if (tpssm11.Update(v_update, v_condi) != true)
		//	{
		//		strcpy(s.msg, "Update tpssm11 failed.");
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

		//Log::Trace("", __FUNCTION__, "C3");
		//bcls_ret->Tables.Add();
		//sqlstr = " select c.sm_plan_no, "
		//	" c.start_time, "
		//	" c.end_time, "
		//	" c.proc_time, "
		//	" d.steel_start_time,"
		//	" d.steel_end_time,"
		//	" d.pono,"
		//	" d.lf_end_time,"
		//	" d.lf_end_time_real"
		//	" , decode(trim(d.lf_end_time_real), '', d.lf_end_time, d.lf_end_time_real)end_time1"
		//	" from tpssm12 c"
		//	" right join（select a.sm_plan_no, a.end_time as lf_end_time, a.end_time_real as lf_end_time_real, b.steel_start_time, b.steel_end_time, b.pono"
		//	" from tpssm12 a, tpssm11 b"
		//	" where a.area_id = 4"
		//	" and b.run_status < 40"
		//	" and a.sm_plan_no = b.sm_plan_no"
		//	" and(a.charge_no, a.sm_plan_no) in"
		//	" (select max(charge_no) - 1, sm_plan_no as charge_no"
		//	" from tpssm12"
		//	" group by sm_plan_no) ）d on c.sm_plan_no = d.sm_plan_no"
		//	" where c.dev_code = 'C3'"
		//	" order by end_time1 ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[8]);
		//cmd_tpssm11_inq.Close();
		////Log::Trace("", __FUNCTION__, "count = {0}", bcls_ret->Tables[8].Rows.get_Count());
		//tpssm12.Reset();
		//for (i = 0; i < bcls_ret->Tables[8].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[8].Rows[i]["START_TIME"].ToString();//当前炉时间
		//	end_time = bcls_ret->Tables[8].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[8].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[8].Rows[i]["SM_PLAN_NO"].ToString();

		//	if (i == 0)
		//	{
		//		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C3' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C3' and start_time < (select start_time from tpssm12 where dev_code = 'C3' and sm_plan_no = @sm_plan_no))";
		//		cmd_tpssm12_inq.SetCommandText(sqlstr);
		//		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//		cmd_tpssm12_inq.ExecuteReader();
		//		if (cmd_tpssm12_inq.Read())
		//		{
		//			sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		//			start_time2 = cmd_tpssm12_inq.GetString(2);
		//			end_time2 = cmd_tpssm12_inq.GetString(3);
		//		}
		//		else
		//		{
		//			sm_plan_no2 = " ";
		//			start_time2 = " ";
		//			end_time2 = " ";
		//		}
		//		cmd_tpssm12_inq.Close();
		//	}
		//	else
		//	{
		//		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		//		start_time2 = tpssm12["START_TIME"];
		//		end_time2 = tpssm12["END_TIME"];
		//	}

		//	if (start_time2.Trim() == "" || end_time2.Trim() == "")
		//	{
		//		start_time_cc = start_time;
		//		end_time_cc = end_time;
		//	}
		//	else
		//	{
		//		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		//		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		//		if (start_time >= start_time2)
		//		{
		//			start_time_cc = start_time;
		//			end_time_cc = end_time;
		//		}
		//		else
		//		{
		//			start_time_cc = start_time2;
		//			end_time_cc = end_time2;
		//		}
		//	}
		//	Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		//	Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		//	Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		//	Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		//	tpssm12["START_TIME"] = start_time_cc;
		//	tpssm12["END_TIME"] = end_time_cc;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "C3";
		//	tpssm11["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm11["STEEL_END_TIME"] = end_time_cc;

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}

		//	v_update = "STEEL_END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO"; //查询条件

		//	if (tpssm11.Update(v_update, v_condi) != true)
		//	{
		//		strcpy(s.msg, "Update tpssm11 failed.");
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

		//Log::Trace("", __FUNCTION__, "C4");
		//bcls_ret->Tables.Add();
		//sqlstr = " select c.sm_plan_no, "
		//	" c.start_time, "
		//	" c.end_time, "
		//	" c.proc_time, "
		//	" d.steel_start_time,"
		//	" d.steel_end_time,"
		//	" d.pono,"
		//	" d.lf_end_time,"
		//	" d.lf_end_time_real"
		//	" , decode(trim(d.lf_end_time_real), '', d.lf_end_time, d.lf_end_time_real)end_time1"
		//	" from tpssm12 c"
		//	" right join（select a.sm_plan_no, a.end_time as lf_end_time, a.end_time_real as lf_end_time_real, b.steel_start_time, b.steel_end_time, b.pono"
		//	" from tpssm12 a, tpssm11 b"
		//	" where a.area_id = 4"
		//	" and b.run_status < 40"
		//	" and a.sm_plan_no = b.sm_plan_no"
		//	" and(a.charge_no, a.sm_plan_no) in"
		//	" (select max(charge_no) - 1, sm_plan_no as charge_no"
		//	" from tpssm12"
		//	" group by sm_plan_no) ）d on c.sm_plan_no = d.sm_plan_no"
		//	" where c.dev_code = 'C4'"
		//	" order by end_time1 ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[9]);
		//cmd_tpssm11_inq.Close();
		////Log::Trace("", __FUNCTION__, "count = {0}", bcls_ret->Tables[9].Rows.get_Count());
		//tpssm12.Reset();
		//for (i = 0; i < bcls_ret->Tables[9].Rows.get_Count(); i++)
		//{
		//	start_time = bcls_ret->Tables[9].Rows[i]["START_TIME"].ToString();//当前炉时间
		//	end_time = bcls_ret->Tables[9].Rows[i]["END_TIME"].ToString();
		//	diff_time = bcls_ret->Tables[9].Rows[i]["PROC_TIME"].ToDouble() * 60;
		//	sm_plan_no = bcls_ret->Tables[9].Rows[i]["SM_PLAN_NO"].ToString();

		//	if (i == 0)
		//	{
		//		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C4' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C4' and start_time < (select start_time from tpssm12 where dev_code = 'C4' and sm_plan_no = @sm_plan_no))";
		//		cmd_tpssm12_inq.SetCommandText(sqlstr);
		//		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		//		cmd_tpssm12_inq.ExecuteReader();
		//		if (cmd_tpssm12_inq.Read())
		//		{
		//			sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		//			start_time2 = cmd_tpssm12_inq.GetString(2);
		//			end_time2 = cmd_tpssm12_inq.GetString(3);
		//		}
		//		else
		//		{
		//			sm_plan_no2 = " ";
		//			start_time2 = " ";
		//			end_time2 = " ";
		//		}
		//		cmd_tpssm12_inq.Close();
		//	}
		//	else
		//	{
		//		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		//		start_time2 = tpssm12["START_TIME"];
		//		end_time2 = tpssm12["END_TIME"];
		//	}

		//	if (start_time2.Trim() == "" || end_time2.Trim() == "")
		//	{
		//		start_time_cc = start_time;
		//		end_time_cc = end_time;
		//	}
		//	else
		//	{
		//		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		//		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		//		if (start_time >= start_time2)
		//		{
		//			start_time_cc = start_time;
		//			end_time_cc = end_time;
		//		}
		//		else
		//		{
		//			start_time_cc = start_time2;
		//			end_time_cc = end_time2;
		//		}
		//	}
		//	Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		//	Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		//	Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		//	Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		//	Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		//	Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		//	Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		//	tpssm12["START_TIME"] = start_time_cc;
		//	tpssm12["END_TIME"] = end_time_cc;
		//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm12["DEV_CODE"] = "C4";
		//	tpssm11["SM_PLAN_NO"] = sm_plan_no;
		//	tpssm11["STEEL_END_TIME"] = end_time_cc;

		//	v_update = "START_TIME,END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		//	if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		//	{
		//		if (tpssm12.Update(v_update, v_condi) != true)
		//		{
		//			strcpy(s.msg, "Update tpssm12 failed.");
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}

		//	v_update = "STEEL_END_TIME";//修改字段信息。
		//	v_condi = "SM_PLAN_NO"; //查询条件

		//	if (tpssm11.Update(v_update, v_condi) != true)
		//	{
		//		strcpy(s.msg, "Update tpssm11 failed.");
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

		/*Log::Trace("", __FUNCTION__, "C1");
		bcls_ret->Tables.Add();
		sqlstr = " select a.sm_plan_no,a.proc_time,a.start_time,a.end_time,b.steel_start_time,b.steel_end_time,b.pono,c.start_time as timeorder,c.end_time from tpssm12 a,tpssm11 b,tpssm12 c where a.dev_code = 'C1' and c.dev_code in ('B1','B2','B3') and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by timeorder ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[6]);
		cmd_tpssm11_inq.Close();
		tpssm12.Reset();
		for (i = 0; i < bcls_ret->Tables[6].Rows.get_Count(); i++)
		{
		start_time = bcls_ret->Tables[6].Rows[i]["START_TIME"].ToString();//当前炉时间
		end_time = bcls_ret->Tables[6].Rows[i]["END_TIME"].ToString();
		diff_time = bcls_ret->Tables[6].Rows[i]["PROC_TIME"].ToDouble() * 60;
		sm_plan_no = bcls_ret->Tables[6].Rows[i]["SM_PLAN_NO"].ToString();

		if (i == 0)
		{
		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C1' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C1' and start_time < (select start_time from tpssm12 where dev_code = 'C1' and sm_plan_no = @sm_plan_no))";
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
		sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		start_time2 = cmd_tpssm12_inq.GetString(2);
		end_time2 = cmd_tpssm12_inq.GetString(3);
		}
		else
		{
		sm_plan_no2 = " ";
		start_time2 = " ";
		end_time2 = " ";
		}
		cmd_tpssm12_inq.Close();
		}
		else
		{
		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		start_time2 = tpssm12["START_TIME"];
		end_time2 = tpssm12["END_TIME"];
		}

		if (start_time2.Trim() == "" || end_time2.Trim() == "")
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		if (start_time >= start_time2)
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time_cc = start_time2;
		end_time_cc = end_time2;
		}
		}
		Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		tpssm12["START_TIME"] = start_time_cc;
		tpssm12["END_TIME"] = end_time_cc;
		tpssm12["SM_PLAN_NO"] = sm_plan_no;
		tpssm12["DEV_CODE"] = "C1";
		tpssm11["SM_PLAN_NO"] = sm_plan_no;
		tpssm11["STEEL_END_TIME"] = end_time_cc;

		v_update = "START_TIME,END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		{
		if (tpssm12.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm12 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		v_update = "STEEL_END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO"; //查询条件

		if (tpssm11.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm11 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		Log::Trace("", __FUNCTION__, "C2");
		bcls_ret->Tables.Add();
		sqlstr = " select a.sm_plan_no,a.proc_time,a.start_time,a.end_time,b.steel_start_time,b.steel_end_time,b.pono,c.start_time as timeorder,c.end_time from tpssm12 a,tpssm11 b,tpssm12 c where a.dev_code = 'C2' and c.dev_code in ('B1','B2','B3') and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by timeorder ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[7]);
		cmd_tpssm11_inq.Close();
		tpssm12.Reset();
		for (i = 0; i < bcls_ret->Tables[7].Rows.get_Count(); i++)
		{
		start_time = bcls_ret->Tables[7].Rows[i]["START_TIME"].ToString();//当前炉时间
		end_time = bcls_ret->Tables[7].Rows[i]["END_TIME"].ToString();
		diff_time = bcls_ret->Tables[7].Rows[i]["PROC_TIME"].ToDouble() * 60;
		sm_plan_no = bcls_ret->Tables[7].Rows[i]["SM_PLAN_NO"].ToString();

		if (i == 0)
		{
		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C2' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C2' and start_time < (select start_time from tpssm12 where dev_code = 'C2' and sm_plan_no = @sm_plan_no))";
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
		sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		start_time2 = cmd_tpssm12_inq.GetString(2);
		end_time2 = cmd_tpssm12_inq.GetString(3);
		}
		else
		{
		sm_plan_no2 = " ";
		start_time2 = " ";
		end_time2 = " ";
		}
		cmd_tpssm12_inq.Close();
		}
		else
		{
		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		start_time2 = tpssm12["START_TIME"];
		end_time2 = tpssm12["END_TIME"];
		}

		if (start_time2.Trim() == "" || end_time2.Trim() == "")
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		if (start_time >= start_time2)
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time_cc = start_time2;
		end_time_cc = end_time2;
		}
		}
		Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		tpssm12["START_TIME"] = start_time_cc;
		tpssm12["END_TIME"] = end_time_cc;
		tpssm12["SM_PLAN_NO"] = sm_plan_no;
		tpssm12["DEV_CODE"] = "C2";
		tpssm11["SM_PLAN_NO"] = sm_plan_no;
		tpssm11["STEEL_END_TIME"] = end_time_cc;

		v_update = "START_TIME,END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		{
		if (tpssm12.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm12 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		v_update = "STEEL_END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO"; //查询条件

		if (tpssm11.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm11 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		Log::Trace("", __FUNCTION__, "C3");
		bcls_ret->Tables.Add();
		sqlstr = " select a.sm_plan_no,a.proc_time,a.start_time,a.end_time,b.steel_start_time,b.steel_end_time,b.pono,c.start_time as timeorder,c.end_time from tpssm12 a,tpssm11 b,tpssm12 c where a.dev_code = 'C3' and c.dev_code in ('B1','B2','B3') and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by timeorder ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[8]);
		cmd_tpssm11_inq.Close();
		tpssm12.Reset();
		for (i = 0; i < bcls_ret->Tables[8].Rows.get_Count(); i++)
		{
		start_time = bcls_ret->Tables[8].Rows[i]["START_TIME"].ToString();//当前炉时间
		end_time = bcls_ret->Tables[8].Rows[i]["END_TIME"].ToString();
		diff_time = bcls_ret->Tables[8].Rows[i]["PROC_TIME"].ToDouble() * 60;
		sm_plan_no = bcls_ret->Tables[8].Rows[i]["SM_PLAN_NO"].ToString();

		if (i == 0)
		{
		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C3' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C3' and start_time < (select start_time from tpssm12 where dev_code = 'C3' and sm_plan_no = @sm_plan_no))";
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
		sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		start_time2 = cmd_tpssm12_inq.GetString(2);
		end_time2 = cmd_tpssm12_inq.GetString(3);
		}
		else
		{
		sm_plan_no2 = " ";
		start_time2 = " ";
		end_time2 = " ";
		}
		cmd_tpssm12_inq.Close();
		}
		else
		{
		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		start_time2 = tpssm12["START_TIME"];
		end_time2 = tpssm12["END_TIME"];
		}

		if (start_time2.Trim() == "" || end_time2.Trim() == "")
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		if (start_time >= start_time2)
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time_cc = start_time2;
		end_time_cc = end_time2;
		}
		}
		Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		tpssm12["START_TIME"] = start_time_cc;
		tpssm12["END_TIME"] = end_time_cc;
		tpssm12["SM_PLAN_NO"] = sm_plan_no;
		tpssm12["DEV_CODE"] = "C3";
		tpssm11["SM_PLAN_NO"] = sm_plan_no;
		tpssm11["STEEL_END_TIME"] = end_time_cc;

		v_update = "START_TIME,END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		{
		if (tpssm12.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm12 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		v_update = "STEEL_END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO"; //查询条件

		if (tpssm11.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm11 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		Log::Trace("", __FUNCTION__, "C4");
		bcls_ret->Tables.Add();
		sqlstr = " select a.sm_plan_no,a.proc_time,a.start_time,a.end_time,b.steel_start_time,b.steel_end_time,b.pono,c.start_time as timeorder,c.end_time from tpssm12 a,tpssm11 b,tpssm12 c where a.dev_code = 'C4' and c.dev_code in ('B1','B2','B3') and b.pono_status < 20 and a.sm_plan_no = b.sm_plan_no and a.sm_plan_no = c.sm_plan_no order by timeorder ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[9]);
		cmd_tpssm11_inq.Close();
		tpssm12.Reset();
		for (i = 0; i < bcls_ret->Tables[9].Rows.get_Count(); i++)
		{
		start_time = bcls_ret->Tables[9].Rows[i]["START_TIME"].ToString();//当前炉时间
		end_time = bcls_ret->Tables[9].Rows[i]["END_TIME"].ToString();
		diff_time = bcls_ret->Tables[9].Rows[i]["PROC_TIME"].ToDouble() * 60;
		sm_plan_no = bcls_ret->Tables[9].Rows[i]["SM_PLAN_NO"].ToString();

		if (i == 0)
		{
		sqlstr = "select sm_plan_no,start_time,end_time from tpssm12 where dev_code = 'C4' and start_time = (select max(start_time) from tpssm12 where dev_code = 'C4' and start_time < (select start_time from tpssm12 where dev_code = 'C4' and sm_plan_no = @sm_plan_no))";
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
		sm_plan_no2 = cmd_tpssm12_inq.GetString(1);//前一炉时间
		start_time2 = cmd_tpssm12_inq.GetString(2);
		end_time2 = cmd_tpssm12_inq.GetString(3);
		}
		else
		{
		sm_plan_no2 = " ";
		start_time2 = " ";
		end_time2 = " ";
		}
		cmd_tpssm12_inq.Close();
		}
		else
		{
		sm_plan_no2 = tpssm12["SM_PLAN_NO"];
		start_time2 = tpssm12["START_TIME"];
		end_time2 = tpssm12["END_TIME"];
		}

		if (start_time2.Trim() == "" || end_time2.Trim() == "")
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time2 = (CDateTime::Parse(end_time2).AddSeconds(ccnexttime)).ToString("yyyyMMddHHmmss");
		end_time2 = (CDateTime::Parse(start_time2).AddSeconds(diff_time.ToDouble())).ToString("yyyyMMddHHmmss");
		if (start_time >= start_time2)
		{
		start_time_cc = start_time;
		end_time_cc = end_time;
		}
		else
		{
		start_time_cc = start_time2;
		end_time_cc = end_time2;
		}
		}
		Log::Trace("", __FUNCTION__, "start_time = {0}", start_time);
		Log::Trace("", __FUNCTION__, "end_time = {0}", end_time);
		Log::Trace("", __FUNCTION__, "start_time2 = {0}", start_time2);
		Log::Trace("", __FUNCTION__, "end_time2 = {0}", end_time2);
		Log::Trace("", __FUNCTION__, "start_timecc = {0}", start_time_cc);
		Log::Trace("", __FUNCTION__, "end_timecc = {0}", end_time_cc);
		Log::Trace("", __FUNCTION__, "sm_plan_no = {0}", sm_plan_no);

		tpssm12["START_TIME"] = start_time_cc;
		tpssm12["END_TIME"] = end_time_cc;
		tpssm12["SM_PLAN_NO"] = sm_plan_no;
		tpssm12["DEV_CODE"] = "C4";
		tpssm11["SM_PLAN_NO"] = sm_plan_no;
		tpssm11["STEEL_END_TIME"] = end_time_cc;

		v_update = "START_TIME,END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO,DEV_CODE"; //查询条件

		if (tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE") > 0)
		{
		if (tpssm12.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm12 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}

		v_update = "STEEL_END_TIME";//修改字段信息。
		v_condi = "SM_PLAN_NO"; //查询条件

		if (tpssm11.Update(v_update, v_condi) != true)
		{
		strcpy(s.msg, "Update tpssm11 failed.");
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}*/
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
