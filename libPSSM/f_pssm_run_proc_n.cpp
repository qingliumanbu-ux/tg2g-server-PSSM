/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-12-26
Description:	 炼钢计划运转信号处理函数。
Update: 2015-04-07 lijie 更新数据表
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件





int f_pssm33_run_upd_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm11_run_upd_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm34_ins_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm_call_tps_n(CString main_backlog_code, int mode, CDbConnection * conn);//
int f_tmsm_mag(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_mmsm009e_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//甘特图炉次生成CAST号
int f_pssm_deal_pre_plan(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm_deal_pre_plan2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_qmts_23_init(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
/*<remark>=========================================================
/// <summary>
/// 出钢计划运转信号处理函数
/// <para>根据选择的PONO, 将传入的设备运转信号进行处理，更新计划状态。</para>
/// <para>1.调用作业计划监控处理函数f_pssm33_run_upd()，更新TPSSM33的表内容。</para>
/// <para>2.调用作业计划运转状态处理函数 f_pssm13_run_status_upd()。 </para>
/// <para>3.调用事项信号履历记录函数 f_pssm34_ins()。                </para>
/// <para>数据库表：tpssm13/14(炼钢出钢计划跟踪表)，tpssms1          </para>
/// <para>主调用函数：pssm51_run()调用。                             </para>
/// </summary>
/// <param name="pono">制造命令          </param>
/// <param name="proc_no">设备处理号     </param>
/// <param name="proc_time">处理时间     </param>
/// <param name="signal">信号代码        </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm_run_proc_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int fetchRowCount;
	int i;
	int ret;
	CString message;
	CDecimal srp_seq = 0;
	CDecimal v_charge_no_3 = 0;
	CDecimal v_charge_no_2 = 0;
	CDecimal v_charge_no_4 = 0;
	CDecimal RN = 0;
	EIClass inBlock;
	EIClass  outBlock1;
	CString	date_time;            /* 记录创建时刻 */
	int		dummy;
	CString	proc_no;				/* 处理号 */
	CString	proc_time;				/* 处理时刻 */
	CString    simul_flag;              /* 模拟标记 */
	CString   start_flag=" ";                  /* 校验结果 */
	CString		do_flag = " ";
	CTimeSpan proc_time_dif;
	CDecimal  c_dif = 0;
	CDateTime start_time;
	CDateTime end_time;
	CString		v_pono = " ";
	CString		v_heat_no = " ";
	CString		ladle_no = " ";
	CString		v_sm_plan_no = " ";
	CString		v_proc_no = " ";
	CString		v_start_time_real = " ";
	CString		v_end_time_real = " ";
	CString		v_end_time = " ";
	CString		v_heat_no_back = " ";
	CString		v_start_time_real_back = " ";
	CString		v_end_time_real_back = " ";
	int mode = 1;
	CString v_factory_div = "LG1";
	CString     v_run_signal = " ";
	int v_pono_proc_time = 0;
	CString v_run_status = " ";
	CString v_cc_req_time = " ";
	CString v_heat_no_3 = "";
	CString v_heat_no_5 = "";
	CString v_heat_no_bef = "";
	CString v_sm_plan_no_back = "";
	CString	v_start_time_4 = " ";
	CString	v_start_time_real_4 = " ";
	CString	v_end_time_real_4 = " ";
	CString	v_end_time_4 = " ";
	CDecimal treatment_count = 0;
	CDecimal v_proc_time_4 = 0;
	CDateTime tmp_time;
	EIClass tb_tpssm11;
	CModel tpssmd1("TPSSMD1");
	CModel tpssms1("TPSSMS1");
	CModel tpssm11("TPSSM11");
	CModel tpssm11_send("TPSSM11");
	CModel tpssm12("TPSSM12");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm41_inq(conn);

	CString sqlstr;
	bool b_area4_time_upd = false; //当精炼生产时间推迟的时候，结束时间是否自动延迟

	EIClass inblocktmsm;//调用履历函数
	inblocktmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblocktmsm.Tables[0].Columns.Add(DT_STRING, "MSG");
	inblocktmsm.Tables[0].Columns.Add(DT_STRING, "STATE_TIME");
	inblocktmsm.Tables[0].set_TableName("TMSM_MSG");

	EIClass inblockmmsm;//调用实绩函数
	inblockmmsm.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NOL2");
	inblockmmsm.Tables[0].Columns.Add(DT_STRING, "TABLE_NAME");
	inblockmmsm.Tables[0].Columns.Add(DT_STRING, "PONO");
	inblockmmsm.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblockmmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblockmmsm.Tables[0].Columns.Add(DT_STRING, "PROC_NO");
	inblockmmsm.Tables[0].Columns.Add(DT_DECIMAL, "TREATMENT_COUNTER");
	//inblockmmsm.Tables[0].Columns.Add(DT_STRING, "STATE_TIME");

	EIClass inBlock2;
	inBlock2.Tables[0].set_TableName("PLAN");  //
	inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	inBlock2.Tables[0].Rows.Add();
	inBlock2.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;

	EIClass inBlockdealtime;
	inBlockdealtime.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");  //炼钢单元号
	inBlockdealtime.Tables[0].Rows.Add();

	EIClass inblockqm;//调用质量函数
	inblockqm.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblockqm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblockqm.Tables[0].Columns.Add(DT_STRING, "ST_NO");
	inblockqm.Tables[0].Columns.Add(DT_STRING, "PONO");
	
	try
	{
		date_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		//获得输入参数
		tpssms1["FACTORY_DIV"]=bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();		
		proc_no    =bcls_rec->Tables[0].Rows[0]["PROC_NO"].ToString();		
		proc_time  =bcls_rec->Tables[0].Rows[0]["PROC_TIME"].ToString();
		simul_flag =bcls_rec->Tables[0].Rows[0]["SIMUL_FLAG"].ToString();
		tpssm11["PONO"]=bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
		tpssms1["RUN_SIGNAL"]=bcls_rec->Tables[2].Rows[0]["RUN_SIGNAL"].ToString();
		tpssmd1["AREA_ID"] = srp_seq.Parse(bcls_rec->Tables[2].Rows[0]["AREA_ID"].ToString());

		if (bcls_rec->Tables[2].Columns.Contains("HEAT_NO"))
		{
			v_heat_no = bcls_rec->Tables[2].Rows[0]["HEAT_NO"].ToString();
		}

		if (bcls_rec->Tables[2].Columns.Contains("LADLE_NO"))
		{
			ladle_no = bcls_rec->Tables[2].Rows[0]["LADLE_NO"].ToString();
		}

		if (bcls_rec->Tables[2].Rows[0]["SRP_SEQ"].ToString().Trim() != "")
		{
			srp_seq = srp_seq.Parse(bcls_rec->Tables[2].Rows[0]["SRP_SEQ"].ToString());
		}

		if (bcls_rec->Tables[2].Rows[0]["TREATMENT_COUNTER"].ToString().Trim() != "")
		{
			treatment_count = bcls_rec->Tables[2].Rows[0]["TREATMENT_COUNTER"].ToDecimal();
		}
		if (bcls_rec->Tables[2].Rows[0]["CHARGE_NO_2"].ToString().Trim() != "")
		{
			v_charge_no_2 = bcls_rec->Tables[2].Rows[0]["CHARGE_NO_2"].ToDecimal();
		}

		Log::Info("", __FUNCTION__,  "factory_div=[{0}][{1}], pono=[{2}], run_signal=[{3}], area_id=[{4}], proc_no=[{5}],proc_time=[{6}]",
			tpssms1["FACTORY_DIV"].ToString(), simul_flag, tpssm11["PONO"].ToString(), tpssms1["RUN_SIGNAL"].ToString(), tpssmd1["AREA_ID"].ToDecimal(), proc_no, proc_time);
		
		////Log::Trace("", __FUNCTION__, "srp_seq = [{0}]", srp_seq);

		//1.运转信号转换为PONO状态
		//输入: 信号(sign), 制造命令(PONO), 处理号(proc_no), 处理时刻(proc_time)
		//1)校验有无该信号和制造命令
		dummy=tpssms1.QueryCount("FACTORY_DIV,RUN_SIGNAL");
		if (dummy == 0)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000149")/*运转信号[{0}]不存在，请联系维护人员。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		else if (dummy > 1)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000059")/*运转信号[{0}]重复定义，请联系维护人员。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		tpssms1.Query("FACTORY_DIV,RUN_SIGNAL");
		tpssms1.TrimOrBlank();

		if (tpssms1["SUB_PROC_SEQ"].ToDecimal() == 0) //子处理序号
		{
			sprintf(s.sysmsg, "制造命令[%s]的运转信号[%s]子处理序号为0，不处理.",(const char*)tpssm11["PONO"].ToString(),(const char*)tpssms1["RUN_SIGNAL"].ToString());
			throw CApplicationException(-1,s.msg,log.Location);
		}
		tpssm11["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
		dummy=tpssm11.QueryCount("FACTORY_DIV,PONO");
		if (dummy == 0)
		{
			CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000088")/*制造命令号[{0}]在出钢计划中不存在。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		tpssm11.Query("FACTORY_DIV,PONO");
		tpssm11.TrimOrBlank();

		tpssmd1["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
		tpssmd1["DEV_CODE"]=tpssms1["DEV_CODE"];
		dummy=tpssmd1.QueryCount("FACTORY_DIV,DEV_CODE,AREA_ID");
		if (dummy == 0)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString(), tpssms1["DEV_CODE"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000150")/*运转信号[{0}]对应的设备[{1}]不存在，请联系维护人员。*/, arguments, 2); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
		tpssmd1.TrimOrBlank();

		if (simul_flag == "3")
		{
			//校验浇次顺序
			if (tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "5" && tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(2, 1) == "2")
			{
				sqlstr = " SELECT count(1) FROM( "
					" SELECT a.SM_PLAN_NO, a.pono, a.RESTRAND_FLG, a.CAST_NO, a.CAST_DIV_NO,ROW_NUMBER() OVER (ORDER BY Trim(b.START_TIME_REAL) NULLS LAST, b.START_TIME ASC) AS RN FROM TPSSM11 a JOIN  TPSSM12 b	"
					" ON(a.FACTORY_DIV = b.FACTORY_DIV AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 5)					"
					" WHERE a.run_status < '52'																			"
					" AND a.FACTORY_DIV = @v_factory_div																	"
					" AND a.CC_MACH_NO = @v_dev_code																		"
					" AND(b.START_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.START_TIME_REAL = ' ')			"
					" AND a.CAST_NO <= @cast_no                                            "
					" ORDER BY trim(b.START_TIME_REAL) NULLS LAST, b.START_TIME ASC)										"
					" WHERE RESTRAND_FLG = 'T'																				";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("v_factory_div", tpssm11["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("v_dev_code", tpssm11["CC_MACH_NO"].ToString());
				cmd_inq.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
				//cmd_inq.Parameters.Set("cast_div_no", tpssm11["CAST_DIV_NO"].ToString());
				CDecimal countT = cmd_inq.ExecuteScalar();
				cmd_inq.Close();

				if (countT >= 2)
				{
					Log::Trace("", __FUNCTION__, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前有大于等于两个开浇炉，判定为错误信号，记录履历", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString());
					CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前有大于等于两个开浇炉，判定为错误信号，记录履历", arguments, 3); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else if (countT == 0)
				{
					Log::Trace("", __FUNCTION__, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前没有开浇炉，不需调整顺序", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString());
				}
				else if (countT == 1)
				{
					sqlstr = " SELECT RN FROM( "
						" SELECT a.SM_PLAN_NO, a.pono, a.RESTRAND_FLG, a.CAST_NO, a.CAST_DIV_NO,ROW_NUMBER() OVER (ORDER BY Trim(b.START_TIME_REAL) NULLS LAST, b.START_TIME ASC) AS RN FROM TPSSM11 a JOIN  TPSSM12 b	"
						" ON(a.FACTORY_DIV = b.FACTORY_DIV AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 5)					"
						" WHERE a.run_status < '52'																			"
						" AND a.FACTORY_DIV = @v_factory_div																	"
						" AND a.CC_MACH_NO = @v_dev_code																		"
						" AND(b.START_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.START_TIME_REAL = ' ')			"
						" AND a.CAST_NO <= @cast_no                                            "
						" ORDER BY trim(b.START_TIME_REAL) NULLS LAST, b.START_TIME ASC)										"
						" WHERE RESTRAND_FLG = 'T'																				";
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("v_factory_div", tpssm11["FACTORY_DIV"].ToString());
					cmd_inq.Parameters.Set("v_dev_code", tpssm11["CC_MACH_NO"].ToString());
					cmd_inq.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						RN = cmd_inq.GetDecimal(1);
					}
					cmd_inq.Close();

					if (RN == 1)
					{
						Log::Trace("", __FUNCTION__, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前有本浇次开浇炉，需要调整顺序", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString());

						sqlstr = " UPDATE TPSSM11 SET RESTRAND_FLG = ' ' WHERE CAST_NO = @cast_no AND RESTRAND_FLG = 'T' ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
						cmd_inq.ExecuteNonQuery();

						sqlstr = " UPDATE TPSSM11 SET RESTRAND_FLG = 'T', CAST_DIV_NO = 1 WHERE CAST_NO = @cast_no AND SM_PLAN_NO = @sm_plan_no ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
						cmd_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
						cmd_inq.ExecuteNonQuery();
						cmd_inq.Close();
					}
					else
					{
						Log::Trace("", __FUNCTION__, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前浇次未结束，判定为错误信号", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString());
						CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前浇次未结束，判定为错误信号", arguments, 3); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}
		}
		//Log::Trace("", __FUNCTION__, "1111111111111");
		if (tpssmd1["AREA_ID"].ToDecimal() == 4)
		{
			//1)读取转炉的charge_no

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				//SQL 语句标准化修改:去掉NVL。 xuwen 2013-4-10
				sqlstr = " SELECT MAX(CHARGE_NO) FROM TPSSM12 "
					"  WHERE FACTORY_DIV = @tpssms1.FACTORY_DIV "
					"    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					"    AND AREA_ID           = 3 ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV", tpssms1["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			//tpssm14.CHARGE_NO = cmd_tpssm14_inq.ExecuteScalar();
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				v_charge_no_3 = cmd_tpssm12_inq.GetDecimal(1);
				v_charge_no_4 = v_charge_no_3 + srp_seq;
			}
			else //未读到
			{
				//读取第一重精炼的charge_no
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					//SQL 语句标准化修改:去掉NVL。 xuwen 2013-4-10
					sqlstr = " SELECT MIN(CHARGE_NO) "
						"   FROM TPSSM12 "
						"  WHERE FACTORY_DIV = @tpssms1.FACTORY_DIV "
						"    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
						"    AND AREA_ID           = 4 ";
					break;
				}

				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV", tpssms1["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				//tpssm14.CHARGE_NO = cmd_tpssm14_inq.ExecuteScalar();
				cmd_tpssm12_inq.ExecuteReader();

				if (cmd_tpssm12_inq.Read())
				{
					v_charge_no_3 = cmd_tpssm12_inq.GetDecimal(1);
				}
				else //未读到
				{
					strcpy(s.msg, _RES("PSSMS0000155")/*第一重精炼的工序号不正确。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				
				v_charge_no_3 = v_charge_no_3 - 1;
				v_charge_no_4 = v_charge_no_3 + srp_seq;
			}
		}
		
		//Log::Trace("", __FUNCTION__, "v_charge_no_3 = [{0}], v_charge_no_4 = [{1}]", v_charge_no_3, v_charge_no_4);

		//模拟连铸信号时，转炉必须有信号
		if (tpssmd1["AREA_ID"].ToDecimal() == 5)
		{
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm12["AREA_ID"] = 3;
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];

			////Log::Info("", __FUNCTION__, "tpssm12["SM_PLAN_NO"] = [{0}]", tpssm12["SM_PLAN_NO"].ToString());

			dummy = tpssm12.QueryCount("FACTORY_DIV,AREA_ID,SM_PLAN_NO");
			if (dummy == 0)
			{
				CFormattable arguments[] = { tpssm12["SM_PLAN_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "查不到计划号[{0}]信息", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			sqlstr = "tpssm12.Query(FACTORY_DIV, AREA_ID,SM_PLAN_NO)";
			tpssm12.Query("FACTORY_DIV,AREA_ID,SM_PLAN_NO");
			tpssm12.TrimOrBlank();

			if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "熔炼号[{0}]没有转炉结束信号，无法模拟连铸信号", arguments, 1); //格式化字符串
				//throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		//Log::Trace("", __FUNCTION__, "2222222222222222");
		if (tpssms1["IS_IMPORT"].ToDecimal() == 1)//重要标记, 说明对照表中有
		{
			do_flag = "1";
			//Log::Trace("", __FUNCTION__, "3333333333333333");
			//魏晨祥 2023-06-06提取公共逻辑
			//校验连铸顺序不能混乱
			if ((tpssmd1["AREA_ID"].ToDecimal() == 5)) //&& (tpssm11["RESTRAND_FLG"].ToString() != "T") 卡不住快换；改为41表找不到同浇次上一分割号PONO状态，取上一浇次号的
			{
				v_run_status = "";
				v_cc_req_time = "20150101000000";

				if (tpssm11["CAST_DIV_NO"].ToDecimal() > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
													(SELECT RUN_STATUS,CC_REQ_TIME \
														FROM TPSSM11 T \
														WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
														AND T.CAST_NO = @tpssm11.CAST_NO \
														AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
														ORDER BY T.CAST_DIV_NO DESC) \
													FETCH FIRST 1 ROWS ONLY ";
						break;
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库

					default: // 所有数据库适用，通用SQL语句			
						sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
													(SELECT RUN_STATUS,CC_REQ_TIME \
														FROM TPSSM11 T \
														WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
														AND T.CAST_NO = @tpssm11.CAST_NO \
														AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
														ORDER BY T.CAST_DIV_NO DESC) \
													WHERE ROWNUM = 1 ";
						break;
					}
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
					cmd_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
					cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

					cmd_inq.ExecuteReader();

					if (cmd_inq.Read())
					{
						v_run_status = cmd_inq.GetString(1);
						v_cc_req_time = cmd_inq.GetString(2);
					}
					else
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
							sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
														(SELECT RUN_STATUS,CC_REQ_TIME \
														FROM TPSSM41 T \
														WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
														AND T.CAST_NO = @tpssm11.CAST_NO \
														AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
														ORDER BY T.CAST_DIV_NO DESC) \
														FETCH FIRST 1 ROWS ONLY ";
							break;
						case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	    // Oracle 数据库

						default: // 所有数据库适用，通用SQL语句			
							sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
														(SELECT RUN_STATUS,CC_REQ_TIME \
														FROM TPSSM41 T \
														WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
														AND T.CAST_NO = @tpssm11.CAST_NO \
														AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
														ORDER BY T.CAST_DIV_NO DESC) \
														WHERE ROWNUM = 1 ";
							break;
						}
						cmd_tpssm41_inq.SetCommandText(sqlstr);
						cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
						cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
						cmd_tpssm41_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

						cmd_tpssm41_inq.ExecuteReader();

						if (cmd_tpssm41_inq.Read())
						{
							v_run_status = cmd_tpssm41_inq.GetString(1);
							v_cc_req_time = cmd_tpssm41_inq.GetString(2);
						}
						cmd_tpssm41_inq.Close();
					}
					cmd_inq.Close();
				}
				//上述逻辑没找到v_run_status
				if (v_run_status.Trim() == "")
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
													(SELECT RUN_STATUS,CC_REQ_TIME \
														FROM TPSSM11 T \
														WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
														AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
														ORDER BY T.CAST_DIV_NO DESC) \
													FETCH FIRST 1 ROWS ONLY ";
						break;
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库

					default: // 所有数据库适用，通用SQL语句			
						sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
													(SELECT RUN_STATUS,CC_REQ_TIME \
														FROM TPSSM11 T \
														WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
														AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
														ORDER BY T.CAST_DIV_NO DESC) \
													WHERE ROWNUM = 1 ";
						break;
					}
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
					cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

					cmd_inq.ExecuteReader();

					if (cmd_inq.Read())
					{
						v_run_status = cmd_inq.GetString(1);
						v_cc_req_time = cmd_inq.GetString(2);
					}
					else
					{
						//进历史表默认已经浇铸结束
					}
					cmd_inq.Close();
				}

				if (v_run_status.Trim() != "")
				{
					//判断浇次上一炉是否开浇
					if (v_run_status < "52")
					{
						if (simul_flag == "0")
						{
							CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇！", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
						else
						{
							CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇,请依次模拟信号！", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
					}

					if (v_cc_req_time > proc_time)
					{
						if (simul_flag == "0")
						{
							CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}]。", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
						else
						{
							CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}],请依次模拟信号。", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
					}
				}	
			}
			//----------------校验连铸顺序不能混乱 end-----------

			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			////Log::Trace("", __FUNCTION__, "tpssms1["START_OR_END"] = [{0}]", tpssms1["START_OR_END"].ToDecimal());
			//Log::Trace("", __FUNCTION__, "3333333333333333");
			if ((tpssms1["START_OR_END"].ToDecimal() == 2) && (tpssmd1["AREA_ID"].ToDecimal() == 4))
			{
				//校验当前信号开始时间不能大于设备最后结束时间
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库

				default: // 所有数据库适用，通用SQL语句			
					sqlstr = "	SELECT T.START_TIME, T.END_TIME, PROC_TIME, T.START_TIME_REAL, T.END_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.CHARGE_NO = @v_charge_no_4 \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE  \
								AND T.HEAT_NO = @tpssm11.HEAT_NO \
								AND T.FACTORY_DIV = @tpssm11.FACTORY_DIV  ";
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("v_charge_no_4", v_charge_no_4);
				cmd_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
				cmd_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
				cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				cmd_inq.ExecuteReader();

				if (cmd_inq.Read())
				{
					v_start_time_4 = cmd_inq.GetString(1);
					v_end_time_4 = cmd_inq.GetString(2);
					v_proc_time_4 = cmd_inq.GetDecimal(3);
					v_start_time_real_4 = cmd_inq.GetString(4);
					v_end_time_real_4 = cmd_inq.GetString(5);
				}
				cmd_inq.Close();

				if ((v_end_time_real_4 <= proc_time) && (v_end_time_real_4.Trim() != ""))
				{
					CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(), v_end_time_real_4 }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "该精炼工序已结束,不能大于当前结束时间[{1}]", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
				}

				//如果实绩开始时间比计划结束时间还要晚
				if ((v_end_time_4 <= proc_time) && (v_end_time_real_4.Trim() == "") && (b_area4_time_upd == true))
				{
					//Log::Trace("", __FUNCTION__, "4444444444444444444444");
					tmp_time = CDateTime::Parse(proc_time);
					tpssm12["END_TIME"] = tmp_time.AddMinutes(v_proc_time_4.ToDouble()).ToString("yyyyMMddHHmmss");
					////Log::Trace("", __FUNCTION__, "tpssm12["END_TIME"] = [{0}]", tpssm12["END_TIME"].ToString());

					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库

					default: // 所有数据库适用，通用SQL语句			
						sqlstr = "	UPDATE TPSSM12 T \
									SET START_TIME = @proc_time, \
										END_TIME = @tpssm12.END_TIME \
									WHERE T.CHARGE_NO = @v_charge_no_4 \
										AND T.AREA_ID = @tpssmd1.AREA_ID  \
										AND T.DEV_CODE = @tpssms1.DEV_CODE  \
										AND T.HEAT_NO = @tpssm11.HEAT_NO  \
										AND T.FACTORY_DIV = @tpssm11.FACTORY_DIV  ";
						break;
					}
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("proc_time", proc_time);
					cmd_inq.Parameters.Set("tpssm12.END_TIME", tpssm12["END_TIME"].ToString());
					cmd_inq.Parameters.Set("v_charge_no_4", v_charge_no_4);
					cmd_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
					cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
					cmd_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
					cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

					cmd_inq.ExecuteNonQuery();
				}
			}
			else if ((tpssms1["START_OR_END"].ToDecimal() == 2) && (tpssmd1["AREA_ID"].ToDecimal() != 4))
			{
				//如果是L2上来信号
				if (simul_flag == "0")
				{
					////转炉上一炉没有开始信号
					//if (tpssmd1["AREA_ID"].ToDecimal() == 3)
					//{
					//	v_heat_no_bef = " ";
					//	switch (conn->DatabaseKind)
					//	{
					//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
					//	case DB_KIND_ORACLE:	    // Oracle 数据库

					//	default: // 所有数据库适用，通用SQL语句			
					//		sqlstr = " SELECT SUBSTR(@tpssm11.HEAT_NO, 1, 3)|| LPAD(TO_CHAR(TO_NUMBER(SUBSTR(@tpssm11.HEAT_NO, 4, 5)) - 1),5,'0') \
										//				 									 										   FROM DUAL T ";
					//		break;
					//	}
					//	cmd_inq.SetCommandText(sqlstr);
					//	cmd_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
					//	cmd_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());

					//	cmd_inq.ExecuteReader();

					//	if (cmd_inq.Read())
					//	{
					//		v_heat_no_bef = cmd_inq.GetString(1);
					//	}
					//	cmd_inq.Close();

					//	if (v_heat_no_bef.Trim() != "")
					//	{
					//		v_run_status = "00";
					//		switch (conn->DatabaseKind)
					//		{
					//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					//		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
					//		case DB_KIND_ORACLE:	    // Oracle 数据库

					//		default: // 所有数据库适用，通用SQL语句			
					//			sqlstr = " SELECT RUN_STATUS \
										//						FROM TPSSM11 T \
										//					   WHERE HEAT_NO = @tpssm11.HEAT_NO ";
					//			break;
					//		}
					//		cmd_inq.SetCommandText(sqlstr);
					//		cmd_inq.Parameters.Set("tpssm11.HEAT_NO", v_heat_no_bef);

					//		cmd_inq.ExecuteReader();

					//		if (cmd_inq.Read())
					//		{
					//			v_run_status = cmd_inq.GetString(1);

					//			if (v_run_status == "00")
					//			{
					//				CFormattable arguments[] = { v_heat_no_bef }; // 定义参数列表的数组
					//				CMessageFormat::Format(s.msg, "上一炉号[{0}]未开始生产！", arguments, 1); //格式化字符串
					//				throw CApplicationException(-1, s.msg, log.Location);
					//				cmd_inq.Close();
					//			}
					//		}
					//		cmd_inq.Close();

					//	}
					//}


				}

				////Log::Info("", __FUNCTION__, "校验连铸顺序不能混乱!!!");

				//魏晨祥 2023-06-06提取公共逻辑
				////校验连铸顺序不能混乱
				//if ((tpssmd1["AREA_ID"].ToDecimal() == 5) && (tpssm11["RESTRAND_FLG"].ToString() != "T"))
				//{
				//	v_run_status = "";
				//	v_cc_req_time = "20150101000000";
				//	switch (conn->DatabaseKind)
				//	{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM11 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
				//						AND T.CAST_NO = @tpssm11.CAST_NO \
				//						AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//					FETCH FIRST 1 ROWS ONLY ";
				//		break;
				//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	    // Oracle 数据库

				//	default: // 所有数据库适用，通用SQL语句			
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM11 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
				//						AND T.CAST_NO = @tpssm11.CAST_NO \
				//						AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//					WHERE ROWNUM = 1 ";
				//		break;
				//	}
				//	cmd_inq.SetCommandText(sqlstr);
				//	cmd_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//	cmd_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
				//	cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//	cmd_inq.ExecuteReader();

				//	if (cmd_inq.Read())
				//	{
				//		v_run_status = cmd_inq.GetString(1);
				//		v_cc_req_time = cmd_inq.GetString(2);
				//	}
				//	else
				//	{
				//		switch (conn->DatabaseKind)
				//		{
				//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//			sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//						(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM41 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = @tpssm11.CAST_NO \
				//						AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//						FETCH FIRST 1 ROWS ONLY ";
				//			break;
				//		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//		case DB_KIND_ORACLE:	    // Oracle 数据库

				//		default: // 所有数据库适用，通用SQL语句			
				//			sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//						(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM41 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = @tpssm11.CAST_NO \
				//						AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//						WHERE ROWNUM = 1 ";
				//			break;
				//		}
				//		cmd_tpssm41_inq.SetCommandText(sqlstr);
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//		cmd_tpssm41_inq.ExecuteReader();

				//		if (cmd_tpssm41_inq.Read())
				//		{
				//			v_run_status = cmd_tpssm41_inq.GetString(1);
				//			v_cc_req_time = cmd_tpssm41_inq.GetString(2);
				//		}
				//		cmd_tpssm41_inq.Close();
				//	}
				//	cmd_inq.Close();

				//	if (v_run_status.Trim() != "")
				//	{
				//		//判断浇次上一炉是否开浇
				//		if (v_run_status < "52")
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇,请依次模拟信号！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}

				//		if (v_cc_req_time > proc_time)
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}]。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}],请依次模拟信号。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}
				//	}	
				//}

				////校验连铸顺序不能混乱
				//if ((tpssmd1["AREA_ID"].ToDecimal() == 5) && (tpssm11["RESTRAND_FLG"].ToString() == "T"))
				//{
				//	v_run_status = "";
				//	v_cc_req_time = "20150101000000";
				//	switch (conn->DatabaseKind)
				//	{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM11 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
				//						AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//					FETCH FIRST 1 ROWS ONLY ";
				//		break;
				//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	    // Oracle 数据库

				//	default: // 所有数据库适用，通用SQL语句			
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM11 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
				//						AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//					WHERE ROWNUM = 1 ";
				//		break;
				//	}
				//	cmd_inq.SetCommandText(sqlstr);
				//	cmd_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//	cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//	cmd_inq.ExecuteReader();

				//	if (cmd_inq.Read())
				//	{
				//		v_run_status = cmd_inq.GetString(1);
				//		v_cc_req_time = cmd_inq.GetString(2);
				//	}
				//	else
				//	{
				//		//switch (conn->DatabaseKind)
				//		//{
				//		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		//	sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//		//				(SELECT RUN_STATUS,CC_REQ_TIME \
				//		//				FROM TPSSM41 T \
				//		//				WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//		//				AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//		//				ORDER BY T.CAST_DIV_NO DESC) \
				//		//				FETCH FIRST 1 ROWS ONLY ";
				//		//	break;
				//		//case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//		//case DB_KIND_ORACLE:	    // Oracle 数据库

				//		//default: // 所有数据库适用，通用SQL语句			
				//		//	sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//		//				(SELECT RUN_STATUS,CC_REQ_TIME \
				//		//				FROM TPSSM41 T \
				//		//				WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//		//				AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//		//				ORDER BY T.CAST_DIV_NO DESC) \
				//		//				WHERE ROWNUM = 1 ";
				//		//	break;
				//		//}
				//		//cmd_tpssm41_inq.SetCommandText(sqlstr);
				//		//cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//		//cmd_tpssm41_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//		//cmd_tpssm41_inq.ExecuteReader();

				//		//if (cmd_tpssm41_inq.Read())
				//		//{
				//		//	v_run_status = cmd_tpssm41_inq.GetString(1);
				//		//	v_cc_req_time = cmd_tpssm41_inq.GetString(2);
				//		//}
				//		//cmd_tpssm41_inq.Close();
				//	}
				//	cmd_inq.Close();

				//	if (v_run_status.Trim() != "")
				//	{
				//		//判断浇次上一炉是否开浇
				//		if (v_run_status < "52")
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇,请依次模拟信号！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}

				//		if (v_cc_req_time > proc_time)
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}]。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}],请依次模拟信号。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}
				//	}
				//}

				////Log::Info("", __FUNCTION__, "校验当前信号开始时间不能大于设备最后结束时间!!!");
				if ((tpssmd1["AREA_ID"].ToDecimal() == 3) || (tpssmd1["AREA_ID"].ToDecimal() == 5))
				{
					//校验当前信号开始时间不能大于设备最后结束时间
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						sqlstr = "	SELECT *  \
									FROM (SELECT T.SM_PLAN_NO,T.END_TIME_REAL \
									FROM TPSSM12 T \
									WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
									AND T.END_TIME_REAL != ' ' \
									AND T.AREA_ID = @tpssmd1.AREA_ID  \
									AND T.DEV_CODE = @tpssms1.DEV_CODE \
									ORDER BY T.END_TIME_REAL DESC) \
									FETCH FIRST 1 ROWS ONLY ";
						break;
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库

					default: // 所有数据库适用，通用SQL语句			
						sqlstr = "	SELECT *  \
									FROM (SELECT T.SM_PLAN_NO,T.END_TIME_REAL \
									FROM TPSSM12 T \
									WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
									AND T.END_TIME_REAL != ' ' \
									AND T.AREA_ID = @tpssmd1.AREA_ID  \
									AND T.DEV_CODE = @tpssms1.DEV_CODE \
									ORDER BY T.END_TIME_REAL DESC) \
									WHERE ROWNUM = 1 ";
						break;
					}
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
					cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
					cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

					cmd_inq.ExecuteReader();

					if (cmd_inq.Read())
					{
						v_sm_plan_no_back = cmd_inq.GetString(1);
						v_end_time_real_back = cmd_inq.GetString(2);

						if (v_sm_plan_no_back != tpssm11["SM_PLAN_NO"].ToString())
						{
							if ((v_end_time_real_back > proc_time) && (simul_flag == "0"))
							{
								CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
								CMessageFormat::Format(s.msg, "信号时间[{0}]不能小于上个结束时间[{1}]。", arguments, 2); //格式化字符串
								//throw CApplicationException(-1, s.msg, log.Location);
							}
						}
					}
					cmd_inq.Close();
				}

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					sqlstr = "	SELECT *  \
								FROM (SELECT T.SM_PLAN_NO,T.START_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
								AND T.END_TIME_REAL = ' ' \
								AND T.START_TIME_REAL != ' ' \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE \
								ORDER BY T.START_TIME_REAL DESC) \
								FETCH FIRST 1 ROWS ONLY ";

					break;
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库

				default: // 所有数据库适用，通用SQL语句			
					sqlstr = "	SELECT *  \
								FROM (SELECT T.SM_PLAN_NO,T.START_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
								AND T.END_TIME_REAL = ' ' \
								AND T.START_TIME_REAL != ' ' \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE \
								ORDER BY T.START_TIME_REAL DESC) \
								WHERE ROWNUM = 1 ";

					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
				cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				cmd_inq.ExecuteReader();

				if (cmd_inq.Read())
				{
					v_sm_plan_no_back = cmd_inq.GetString(1);
					v_start_time_real_back = cmd_inq.GetString(2);

					if (v_sm_plan_no_back != tpssm11["SM_PLAN_NO"].ToString())
					{
						if ((v_start_time_real_back > proc_time) && (simul_flag == "0"))
						{
							//do_flag = "0";
							//两个开始信号小于5分钟，忽略信号
							CFormattable arguments[] = { proc_time, v_start_time_real_back }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "信号时间[{0}]不能小于上个开始时间[{1}]。", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
						else
						{
							//Log::Trace("", __FUNCTION__, "555555555555555555");
							start_time = CDateTime::Parse(v_start_time_real_back);
							end_time = CDateTime::Parse(proc_time);
							proc_time_dif = end_time - start_time;
							c_dif = proc_time_dif.TotalMinutes();

							//两个开始信号小于5分钟，忽略信号
							if ((c_dif <= 5) && (simul_flag == "0"))
							{
								CFormattable arguments[] = { proc_time, v_start_time_real_back }; // 定义参数列表的数组
								CMessageFormat::Format(s.msg, "信号时间[{0}]与上个开始时间[{1}]不能小于5分钟。", arguments, 2); //格式化字符串
								//throw CApplicationException(-1, s.msg, log.Location);
							}
						}
					}
					else
					{
						//switch (conn->DatabaseKind)
						//{
						//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						//case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
						//case DB_KIND_MSSQL:	        // MS SQL Server数据库
						//case DB_KIND_ORACLE:	    // Oracle 数据库

						//default: // 所有数据库适用，通用SQL语句			
						//	sqlstr = "	SELECT *  \
						//				FROM (SELECT RUN_SIGNAL \
						//				FROM TPSSMS1  \
						//				WHERE START_OR_END = @tpssms1.START_OR_END \
						//				AND IS_IMPORT = 1  \
						//				AND DEV_CODE = @tpssms1.DEV_CODE \
						//				ORDER BY RUN_SIGNAL DESC) \
						//				WHERE ROWNUM = 1 ";
						//	break;
						//}
						//cmd_inq.SetCommandText(sqlstr);
						//cmd_inq.Parameters.Set("tpssms1.START_OR_END", tpssms1["START_OR_END"].ToDecimal());
						//cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());

						//cmd_inq.ExecuteReader();

						//if (cmd_inq.Read())
						//{
						//	v_run_signal = cmd_inq.GetString(1);

						//	if ((v_run_signal != tpssms1["RUN_SIGNAL"].ToString()) && (simul_flag == "0"))
						//	{
						//		do_flag = "0";
						//	}
						//}
						//cmd_inq.Close();
					}
				}
				cmd_inq.Close();

				////Log::Trace("", __FUNCTION__, "tpssm12["FACTORY_DIV"] = [{0}]", tpssm12["FACTORY_DIV"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssm12["AREA_ID"] = [{0}]", tpssm12["AREA_ID"].ToDecimal());
				////Log::Trace("", __FUNCTION__, "tpssm12["DEV_CODE"] = [{0}]", tpssm12["DEV_CODE"].ToString());
				////Log::Trace("", __FUNCTION__, "proc_time = [{0}]", proc_time);
				//Log::Trace("", "RUN_SIGNAL = [{0}]", tpssms1["RUN_SIGNAL"].ToString());
				if ((tpssms1["RUN_SIGNAL"].ToString().SubstringNE(0, 1) == "3") && (do_flag == "1"))
				{
					//////Log::Trace("", __FUNCTION__, "tpssm13.PONO_STATUS = [{0}]", tpssm11["PONO_STATUS"].ToDecimal());

					//if (tpssm11["PONO_STATUS"].ToDecimal() == 20)
					//{
					//	//do_flag = "0";
					//}
					//else
					//{
					//	switch (conn->DatabaseKind)
					//	{
					//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					//	case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
					//	case DB_KIND_ORACLE:	        // Oracle 数据库
					//	default:
					//		sqlstr = " SELECT SM_PLAN_NO,PROC_NO,START_TIME_REAL,PROC_TIME "
					//			"	FROM TPSSM12  "
					//			"	WHERE FACTORY_DIV = TRIM(@tpssms1["FACTORY_DIV"].ToString())  "
					//			"		AND START_TIME_REAL <= @proc_time "
					//			"		AND START_TIME_REAL != ' '	"
					//			"		AND END_TIME_REAL = ' ' "
					//			"		AND AREA_ID = @tpssmd1.AREA_ID  "
					//			"		AND DEV_CODE = @tpssms1.DEV_CODE ";
					//		break;
					//	}

					//	cmd_tpssm12_inq.SetCommandText(sqlstr);
					//	cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV", tpssms1["FACTORY_DIV"].ToString());
					//	cmd_tpssm12_inq.Parameters.Set("proc_time", proc_time);
					//	cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
					//	cmd_tpssm12_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
					//	cmd_tpssm12_inq.ExecuteReader();

					//	if (cmd_tpssm12_inq.Read())
					//	{
					//		v_sm_plan_no = cmd_tpssm12_inq.GetString(1);
					//		v_proc_no = cmd_tpssm12_inq.GetString(2);
					//		v_start_time_real = cmd_tpssm12_inq.GetString(3);
					//		v_pono_proc_time = cmd_tpssm12_inq.GetInt32(4);

					//		////Log::Trace("", __FUNCTION__, "设备一炉处理号v_proc_no = [{0}]", v_proc_no);

					//		sqlstr = "SELECT PONO FROM TPSSM11 WHERE SM_PLAN_NO = @SM_PLAN_NO AND FACTORY_DIV = @FACTORY_DIV ";
					//		cmd_tpssm11_inq.SetCommandText(sqlstr);
					//		cmd_tpssm11_inq.Parameters.Set("SM_PLAN_NO", v_sm_plan_no);
					//		cmd_tpssm11_inq.Parameters.Set("FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					//		cmd_tpssm11_inq.ExecuteReader();

					//		if (cmd_tpssm11_inq.Read())
					//		{
					//			v_pono = cmd_tpssm11_inq.GetString(1);
					//			////Log::Info("", __FUNCTION__, "v_pono = [{0}]", v_pono);
					//		}
					//		cmd_tpssm11_inq.Close();

					//		////Log::Trace("", __FUNCTION__, "v_pono = [{0}]", v_pono);
					//		////Log::Trace("", __FUNCTION__, "v_proc_no = [{0}]", v_proc_no);

					//		if (v_start_time_real == proc_time)
					//		{
					//			do_flag = "0";
					//		}
					//		else if (v_sm_plan_no == tpssm11["SM_PLAN_NO"].ToString())
					//		{
					//			//不做任何操作
					//		}
					//		else
					//		{
					//			start_time = CDateTime::Parse(v_start_time_real);
					//			end_time = CDateTime::Parse(proc_time);
					//			proc_time_dif = end_time - start_time;
					//			c_dif = proc_time_dif.TotalMinutes();

					//			//小于开始时间与结束时间小于10分钟，忽略结束信号
					//			if (c_dif <= 10)
					//			{
					//				do_flag = "0";
					//			}
					//			else
					//			{
					//				if (c_dif >= 60)
					//				{
					//					v_end_time_real = CDateTime::Parse(v_start_time_real).AddMinutes(v_pono_proc_time).ToString("yyyyMMddHHmmss");
					//				}
					//				else
					//				{
					//					v_end_time_real = proc_time;
					//				}

					//				if (tpssms1["DEV_CODE"].ToString() == "B1")
					//				{
					//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "316";
					//				}
					//				else if (tpssms1["DEV_CODE"].ToString() == "B2")
					//				{
					//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "326";
					//				}
					//				else if (tpssms1["DEV_CODE"].ToString() == "B3")
					//				{
					//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "336";
					//				}

					//				inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
					//				inBlock.Tables[0].Rows[0]["PONO"] = v_pono;
					//				inBlock.Tables[0].Rows[0]["PROC_NO"] = v_proc_no;
					//				inBlock.Tables[0].Rows[0]["PROC_TIME"] = v_end_time_real;
					//				inBlock.Tables[0].Rows[0]["DEV_CODE"] = tpssms1["DEV_CODE"];
					//				inBlock.Tables[0].Rows[0]["SIMUL_FLAG"] = simul_flag;
					//				inBlock.Tables[0].Rows[0]["AREA_ID"] = tpssmd1["AREA_ID"];
					//				inBlock.Tables[0].Rows[0]["SRP_SEQ"] = 1;
					//				EDLog(1, 1, "f_pssmy1_run_proc>dev_code=[%s]", (const char*)tpssms1["DEV_CODE"].ToString());

					//				//作业计划运转状态处理函数
					//				ret = f_pssm11_run_upd_n(&inBlock, bcls_ret, conn);
					//				if (ret != 0)
					//				{
					//					throw CApplicationException(-1, s.msg, log.Location);
					//				}
					//			}
					//		}
					//	}
					//	cmd_tpssm12_inq.Close();
					//}
				}
				//else if (tpssms1["RUN_SIGNAL"].ToString().SubstringNE(0, 1) == "5")
				//{
				//	tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				//	tpssm12["DEV_CODE"] = tpssms1["DEV_CODE"];
				//	tpssm12["AREA_ID"] = tpssmd1["AREA_ID"];

				//	tpssm12.Query("SM_PLAN_NO, DEV_CODE, AREA_ID");

				//	if (tpssm12["START_TIME_REAL"].ToString().TrimOrBlank() != " ")
				//	{
				//		if (tpssm12["END_TIME_REAL"].ToString().TrimOrBlank() != " ")
				//		{
				//			CFormattable arguments[] = { tpssm12["HEAT_NO"].ToString(), tpssm12["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
				//			CMessageFormat::Format(s.msg, "炉号已经存在[{0}]结束信号[{1}]。", arguments, 2); //格式化字符串
				//			throw CApplicationException(-1, s.msg, log.Location);
				//		}
				//	}
				//	else
				//	{
				//		switch (conn->DatabaseKind)
				//		{
				//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//		case DB_KIND_ORACLE:	        // Oracle 数据库
				//		default:
				//			sqlstr = " SELECT SM_PLAN_NO,PROC_NO,START_TIME_REAL,PROC_TIME "
				//				"	FROM TPSSM12  "
				//				"	WHERE FACTORY_DIV = TRIM(@tpssm12["FACTORY_DIV"].ToString())  "
				//				"		AND START_TIME_REAL <= @proc_time "
				//				"		AND START_TIME_REAL != ' '	"
				//				"		AND END_TIME_REAL = ' ' "
				//				"		AND AREA_ID = @tpssm12.AREA_ID  "
				//				"		AND DEV_CODE = @tpssm12.DEV_CODE ";
				//			break;
				//		}

				//		cmd_tpssm12_inq.SetCommandText(sqlstr);
				//		cmd_tpssm12_inq.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString());
				//		cmd_tpssm12_inq.Parameters.Set("proc_time", proc_time);
				//		cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
				//		cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE", tpssm12["DEV_CODE"].ToString());

				//		cmd_tpssm12_inq.ExecuteReader();

				//		if (cmd_tpssm12_inq.Read())
				//		{
				//			v_sm_plan_no = cmd_tpssm12_inq.GetString(1);
				//			v_proc_no = cmd_tpssm12_inq.GetString(2);
				//			v_start_time_real = cmd_tpssm12_inq.GetString(3);
				//			v_pono_proc_time = cmd_tpssm12_inq.GetInt32(4);

				//			////Log::Trace("", __FUNCTION__, "设备上一炉处理号v_proc_no = [{0}]", v_proc_no);

				//			if (v_start_time_real == proc_time)
				//			{
				//				do_flag = "0";
				//			}
				//			else if (v_sm_plan_no == tpssm11["SM_PLAN_NO"].ToString())
				//			{
				//				//不做任何操作
				//			}
				//			else
				//			{
				//				start_time = CDateTime::Parse(v_start_time_real);
				//				end_time = CDateTime::Parse(proc_time);
				//				proc_time_dif = end_time - start_time;
				//				c_dif = proc_time_dif.TotalMinutes();

				//				//小于开始时间与结束时间小于10分钟，忽略结束信号
				//				if (c_dif <= 10)
				//				{
				//					do_flag = "0";
				//				}
				//				else
				//				{
				//					if (c_dif >= 60)
				//					{
				//						v_end_time_real = CDateTime::Parse(v_start_time_real).AddMinutes(v_pono_proc_time).ToString("yyyyMMddHHmmss");
				//					}
				//					else
				//					{
				//						v_end_time_real = proc_time;
				//					}
				//				}

				//				sqlstr = "SELECT PONO FROM TPSSM11 WHERE SM_PLAN_NO = @SM_PLAN_NO AND FACTORY_DIV = @FACTORY_DIV ";
				//				cmd_tpssm11_inq.SetCommandText(sqlstr);
				//				cmd_tpssm11_inq.Parameters.Set("SM_PLAN_NO", v_sm_plan_no);
				//				cmd_tpssm11_inq.Parameters.Set("FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				//				cmd_tpssm11_inq.ExecuteReader();

				//				if (cmd_tpssm11_inq.Read())
				//				{
				//					v_pono = cmd_tpssm11_inq.GetString(1);
				//					////Log::Info("", __FUNCTION__, "v_pono = [{0}]", v_pono);
				//				}
				//				cmd_tpssm11_inq.Close();

				//				////Log::Trace("", __FUNCTION__, "v_pono = [{0}]", v_pono);

				//				if (tpssms1["DEV_CODE"].ToString() == "C1")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "514";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "C2")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "524";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "C3")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "534";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "C4")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "544";
				//				}

				//				inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
				//				inBlock.Tables[0].Rows[0]["PONO"] = v_pono;
				//				inBlock.Tables[0].Rows[0]["PROC_NO"] = v_proc_no;
				//				inBlock.Tables[0].Rows[0]["PROC_TIME"] = v_end_time_real;
				//				inBlock.Tables[0].Rows[0]["DEV_CODE"] = tpssms1["DEV_CODE"];
				//				inBlock.Tables[0].Rows[0]["SIMUL_FLAG"] = simul_flag;
				//				inBlock.Tables[0].Rows[0]["AREA_ID"] = tpssmd1["AREA_ID"];
				//				inBlock.Tables[0].Rows[0]["SRP_SEQ"] = 1;
				//				EDLog(1, 1, "f_pssmy1_run_proc>dev_code=[%s]", (const char*)tpssms1["DEV_CODE"].ToString());

				//				//作业计划运转状态处理函数
				//				ret = f_pssm11_run_upd_n(&inBlock, bcls_ret, conn);
				//				if (ret != 0)
				//				{
				//					throw CApplicationException(-1, s.msg, log.Location);
				//				}
				//			}
				//		}
				//		cmd_tpssm12_inq.Close();
				//	}
				//}
			}
			else if ((tpssms1["START_OR_END"].ToDecimal() == 3) && (tpssmd1["AREA_ID"].ToDecimal() == 4))
			{
				////Log::Info("", __FUNCTION__, "判断开始时间与结束时间差值!!!");
				//判断开始时间与结束时间差值
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["DEV_CODE"] = tpssms1["DEV_CODE"];
				tpssm12["AREA_ID"] = tpssmd1["AREA_ID"];
				tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm12["CHARGE_NO"] = v_charge_no_4;
				////Log::Info("", __FUNCTION__, "tpssm12["CHARGE_NO"] = [{0}]", tpssm12["CHARGE_NO"].ToDecimal());

				tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, DEV_CODE, AREA_ID, CHARGE_NO");

				if (tpssm12["START_TIME_REAL"].ToString().TrimOrBlank() == " ")
				{
					CFormattable arguments[] = { tpssm12["START_TIME_REAL"].ToString(), proc_time }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "精炼开始时间为空[{0}]不能接受结束信号[{1}]。", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
				}

				if ((tpssm12["START_TIME_REAL"].ToString() > proc_time) && (simul_flag == "0"))
				{
					CFormattable arguments[] = { proc_time, tpssm12["START_TIME_REAL"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于开始时间[{1}]。", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					//Log::Trace("", __FUNCTION__, "666666666666666666");
					//Log::Trace("", __FUNCTION__, "666666666666666666[{0}]", tpssm12["START_TIME_REAL"].ToString());
					//start_time = CDateTime::Parse(tpssm12["START_TIME_REAL"].ToString());
					//Log::Trace("", __FUNCTION__, "666666666666666666444");
					//end_time = CDateTime::Parse(proc_time);
					//Log::Trace("", __FUNCTION__, "666666666666655555");
					//proc_time_dif = end_time - start_time;
					//c_dif = proc_time_dif.TotalMinutes();

					//小于开始时间与结束时间小于5分钟，忽略结束信号
					//if ((c_dif <= 5) && (simul_flag == "0"))
					//{
						//CFormattable arguments[] = { proc_time, tpssm12["START_TIME_REAL"].ToString() }; // 定义参数列表的数组
						//CMessageFormat::Format(s.msg, "信号结束时间[{0}]与开始时间[{1}]不能小于5分钟。", arguments, 2); //格式化字符串
						//throw CApplicationException(-1, s.msg, log.Location);
					//}
				}
			}
			else if ((tpssms1["START_OR_END"].ToDecimal() == 3) && (tpssmd1["AREA_ID"].ToDecimal() != 4) && (tpssmd1["AREA_ID"].ToDecimal() != 2))
			{
				////Log::Info("", __FUNCTION__, "2校验连铸顺序不能混乱!!!");
				//魏晨祥 2023-06-06提取公共逻辑
				////校验连铸顺序不能混乱
				//if ((tpssmd1["AREA_ID"].ToDecimal() == 5) && (tpssm11["RESTRAND_FLG"].ToString() != "T"))
				//{
				//	v_run_status = "";
				//	v_cc_req_time = "20150101000000";
				//	switch (conn->DatabaseKind)
				//	{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//					FROM TPSSM11 T \
				//					WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//					AND T.CAST_NO = @tpssm11.CAST_NO \
				//					AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//					ORDER BY T.CAST_DIV_NO DESC) \
				//					FETCH FIRST 1 ROWS ONLY ";
				//		break;
				//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	    // Oracle 数据库

				//	default: // 所有数据库适用，通用SQL语句			
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//					FROM TPSSM11 T \
				//					WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//					AND T.CAST_NO = @tpssm11.CAST_NO \
				//					AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//					ORDER BY T.CAST_DIV_NO DESC) \
				//					WHERE ROWNUM = 1 ";
				//		break;
				//	}
				//	cmd_inq.SetCommandText(sqlstr);
				//	cmd_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//	cmd_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
				//	cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//	cmd_inq.ExecuteReader();

				//	if (cmd_inq.Read())
				//	{
				//		v_run_status = cmd_inq.GetString(1);
				//		v_cc_req_time = cmd_inq.GetString(2);
				//	}
				//	else
				//	{
				//		switch (conn->DatabaseKind)
				//		{
				//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//			sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//						(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM41 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = @tpssm11.CAST_NO \
				//						AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//						FETCH FIRST 1 ROWS ONLY  ";
				//			break;
				//		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//		case DB_KIND_ORACLE:	    // Oracle 数据库

				//		default: // 所有数据库适用，通用SQL语句			
				//			sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//						(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM41 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = @tpssm11.CAST_NO \
				//						AND T.CAST_DIV_NO = @tpssm11.CAST_DIV_NO - 1 \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//						WHERE ROWNUM = 1 ";
				//			break;
				//		}
				//		cmd_tpssm41_inq.SetCommandText(sqlstr);
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//		cmd_tpssm41_inq.ExecuteReader();

				//		if (cmd_tpssm41_inq.Read())
				//		{
				//			v_run_status = cmd_tpssm41_inq.GetString(1);
				//			v_cc_req_time = cmd_tpssm41_inq.GetString(2);
				//		}
				//		cmd_tpssm41_inq.Close();
				//	}
				//	cmd_inq.Close();

				//	if (v_run_status.Trim() != "")
				//	{
				//		//判断浇次上一炉是否开浇
				//		if (v_run_status < "52")
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇,请依次模拟信号！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}

				//		if (v_cc_req_time > proc_time)
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}]。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}],请依次模拟信号。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}
				//	}
				//}

				//if ((tpssmd1["AREA_ID"].ToDecimal() == 5) && (tpssm11["RESTRAND_FLG"].ToString() == "T"))
				//{
				//	v_run_status = "";
				//	v_cc_req_time = "20150101000000";
				//	switch (conn->DatabaseKind)
				//	{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//					FROM TPSSM11 T \
				//					WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//					ORDER BY T.CAST_DIV_NO DESC) \
				//					FETCH FIRST 1 ROWS ONLY ";
				//		break;
				//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	    // Oracle 数据库

				//	default: // 所有数据库适用，通用SQL语句			
				//		sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//					(SELECT RUN_STATUS,CC_REQ_TIME \
				//					FROM TPSSM11 T \
				//					WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//					ORDER BY T.CAST_DIV_NO DESC) \
				//					WHERE ROWNUM = 1 ";
				//		break;
				//	}
				//	cmd_inq.SetCommandText(sqlstr);
				//	cmd_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//	cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//	cmd_inq.ExecuteReader();

				//	if (cmd_inq.Read())
				//	{
				//		v_run_status = cmd_inq.GetString(1);
				//		v_cc_req_time = cmd_inq.GetString(2);
				//	}
				//	else
				//	{
				//		switch (conn->DatabaseKind)
				//		{
				//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//			sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//						(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM41 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//						FETCH FIRST 1 ROWS ONLY  ";
				//			break;
				//		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//		case DB_KIND_ORACLE:	    // Oracle 数据库

				//		default: // 所有数据库适用，通用SQL语句			
				//			sqlstr = "	SELECT RUN_STATUS,CC_REQ_TIME FROM \
				//						(SELECT RUN_STATUS,CC_REQ_TIME \
				//						FROM TPSSM41 T \
				//						WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
				//						AND T.CAST_NO = substr(@tpssm11.CAST_NO, 1, 2) || lpad(to_char((to_number(substr(@tpssm11.CAST_NO,3,4)-1))),4,0) \
				//						ORDER BY T.CAST_DIV_NO DESC) \
				//						WHERE ROWNUM = 1 ";
				//			break;
				//		}
				//		cmd_tpssm41_inq.SetCommandText(sqlstr);
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				//		cmd_tpssm41_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				//		cmd_tpssm41_inq.ExecuteReader();

				//		if (cmd_tpssm41_inq.Read())
				//		{
				//			v_run_status = cmd_tpssm41_inq.GetString(1);
				//			v_cc_req_time = cmd_tpssm41_inq.GetString(2);
				//		}
				//		cmd_tpssm41_inq.Close();
				//	}
				//	cmd_inq.Close();

				//	if (v_run_status.Trim() != "")
				//	{
				//		//判断浇次上一炉是否开浇
				//		if (v_run_status < "52")
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal() }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "浇次号[{0}][{1}]上一炉未开浇,请依次模拟信号！", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}

				//		if (v_cc_req_time > proc_time)
				//		{
				//			if (simul_flag == "0")
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}]。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//			else
				//			{
				//				CFormattable arguments[] = { proc_time, v_cc_req_time }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "连铸处理开始时间[{0}]不能早于上一炉[{1}],请依次模拟信号。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}
				//		}
				//	}
				//}

				//判断开始时间与结束时间差值
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["DEV_CODE"] = tpssms1["DEV_CODE"];
				tpssm12["AREA_ID"] = tpssmd1["AREA_ID"];
				tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				////Log::Info("", __FUNCTION__, "tpssm12.CHARGE_No = [{0}]", tpssm12["CHARGE_NO"].ToDecimal());
				tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, DEV_CODE, AREA_ID");
				////Log::Info("", __FUNCTION__, "tpssm12["START_TIME_REAL"] = [{0}]", tpssm12["START_TIME_REAL"].ToString());

				if (tpssm12["START_TIME_REAL"].ToString().TrimOrBlank() == " ")
				{
					tpssm12["START_TIME_REAL"] = "20150101000000";
				}

				if ((tpssm12["START_TIME_REAL"].ToString() > proc_time) && (simul_flag == "0"))
				{
					CFormattable arguments[] = { proc_time, tpssm12["START_TIME_REAL"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于开始时间[{1}]。", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					//Log::Trace("", __FUNCTION__, "777777777777777");
					//start_time = CDateTime::Parse(tpssm12["START_TIME_REAL"].ToString());
					//end_time = CDateTime::Parse(proc_time);
					//proc_time_dif = end_time - start_time;
					//c_dif = proc_time_dif.TotalMinutes();

					////小于开始时间与结束时间小于10分钟，忽略结束信号
					//if ((c_dif <= 5) && (simul_flag == "0"))
					//{
					//	CFormattable arguments[] = { proc_time, tpssm12["START_TIME_REAL"].ToString() }; // 定义参数列表的数组
					//	CMessageFormat::Format(s.msg, "信号结束时间[{0}]与开始时间[{1}]不能小于5分钟。", arguments, 2); //格式化字符串
					//	//throw CApplicationException(-1, s.msg, log.Location);
					//}
				}

				////Log::Info("", __FUNCTION__, "-----------与前一炉结束时间做校验--------------");
				//与前一炉做校验（结束时间）
				//两炉结束时间不能相差小于5分钟
				//v_count = 0;
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					sqlstr = "	SELECT *  \
								FROM (SELECT T.SM_PLAN_NO,T.END_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
								AND T.END_TIME_REAL != ' ' \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE \
								ORDER BY T.END_TIME_REAL DESC) \
								FETCH FIRST 1 ROWS ONLY  ";

					break;
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库

				default: // 所有数据库适用，通用SQL语句			
					sqlstr = "	SELECT *  \
								FROM (SELECT T.SM_PLAN_NO,T.END_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
								AND T.END_TIME_REAL != ' ' \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE \
								ORDER BY T.END_TIME_REAL DESC) \
								WHERE ROWNUM = 1 ";

					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("proc_time", proc_time);
				cmd_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
				cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				cmd_inq.ExecuteReader();

				if (cmd_inq.Read())
				{
					v_sm_plan_no_back = cmd_inq.GetString(1);
					v_end_time_real_back = cmd_inq.GetString(2);

					if (v_sm_plan_no_back != tpssm12["SM_PLAN_NO"].ToString())
					{
						if ((v_end_time_real_back > proc_time) && (simul_flag == "0"))
						{
							CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于上炉结束时间[{1}]。", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
						else
						{
							//Log::Trace("", __FUNCTION__, "8888888888888");
							//start_time = CDateTime::Parse(v_end_time_real_back);
							//end_time = CDateTime::Parse(proc_time);
							//proc_time_dif = end_time - start_time;
							//c_dif = proc_time_dif.TotalMinutes();

							////小于开始时间与结束时间小于5分钟，忽略结束信号
							//if ((c_dif <= 5) && (simul_flag == "0"))
							//{
							//	CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
							//	CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能与上炉结束时间[{1}]相差小于5分钟。", arguments, 2); //格式化字符串
							//	//throw CApplicationException(-1, s.msg, log.Location);
							//}
						}
					}
					//else
					//{
					//		start_time	= CDateTime::Parse(v_end_time_real_back);
					//		end_time	= CDateTime::Parse(proc_time);
					//		proc_time_dif	= end_time - start_time;
					//		c_dif = proc_time_dif.TotalMinutes();
					//		
					//		//同一炉两个结束信号相差30分钟
					//		if(c_dif > 30)
					//		{
					//			do_flag = "0";
					//		}								
					//}
				}
				cmd_inq.Close();

				////Log::Info("", __FUNCTION__, "-----------与前一炉开始时间做校验--------------");
				//与前一炉做校验（开始时间）
				//两炉结束时间不能相差小于5分钟
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					sqlstr = "	SELECT *  \
								FROM (SELECT T.SM_PLAN_NO,T.START_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
								AND T.END_TIME_REAL = ' ' \
								AND T.START_TIME_REAL != ' ' \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE \
								ORDER BY T.START_TIME_REAL DESC) \
								FETCH FIRST 1 ROWS ONLY ";

					break;
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库

				default: // 所有数据库适用，通用SQL语句			
					sqlstr = "	SELECT *  \
								FROM (SELECT T.SM_PLAN_NO,T.START_TIME_REAL \
								FROM TPSSM12 T \
								WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV \
								AND T.END_TIME_REAL = ' ' \
								AND T.START_TIME_REAL != ' ' \
								AND T.AREA_ID = @tpssmd1.AREA_ID  \
								AND T.DEV_CODE = @tpssms1.DEV_CODE \
								ORDER BY T.START_TIME_REAL DESC) \
								WHERE ROWNUM = 1 ";

					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("proc_time", proc_time);
				cmd_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_inq.Parameters.Set("tpssms1.DEV_CODE", tpssms1["DEV_CODE"].ToString());
				cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());

				cmd_inq.ExecuteReader();

				if (cmd_inq.Read())
				{
					v_sm_plan_no_back = cmd_inq.GetString(1);
					v_start_time_real_back = cmd_inq.GetString(2);

					if (v_sm_plan_no_back != tpssm12["SM_PLAN_NO"].ToString())
					{
						if ((v_start_time_real_back > proc_time) && (simul_flag == "0"))
						{
							//由于L2上来结束信号过晚，暂时放开注释
							CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于上炉开始时间[{1}]。", arguments, 2); //格式化字符串
							//throw CApplicationException(-1, s.msg, log.Location);
						}
						else
						{
							//Log::Trace("", __FUNCTION__, "9999999999999999");
							//start_time = CDateTime::Parse(v_start_time_real_back);
							//end_time = CDateTime::Parse(proc_time);
							//proc_time_dif = end_time - start_time;
							//c_dif = proc_time_dif.TotalMinutes();

							////小于开始时间与结束时间小于10分钟，忽略结束信号
							//if ((c_dif <= 10) && (simul_flag == "0"))
							//{
							//	CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
							//	CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能与上炉开始时间[{1}]相差小于10分钟。", arguments, 2); //格式化字符串
							//	//throw CApplicationException(-1, s.msg, log.Location);
							//}

							//if (tpssm12["START_TIME_REAL"].ToString() != "20150101000000")
							//{
							//	if (tpssm12["START_TIME_REAL"].ToString() <= v_start_time_real_back)
							//	{
							//		do_flag = "0";
							//	}
							//}
						}
					}
				}
				cmd_inq.Close();

				////如果炉次开始时间为空
				//if (((do_flag == "1") && (tpssm12["START_TIME_REAL"].ToString() == "20150101000000"))
				//	&& ((tpssm12["AREA_ID"].ToDecimal() == 3) || (tpssm12["AREA_ID"].ToDecimal() == 5)))
				//{
				//	////Log::Info("", __FUNCTION__, "-----------如果开始时间为空--------------");
				//	v_end_time_real = " ";
				//	switch (conn->DatabaseKind)
				//	{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//	case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	        // Oracle 数据库
				//	default:
				//		sqlstr =
				//			" SELECT * "
				//			"   FROM (SELECT * "
				//			"           FROM (SELECT SM_PLAN_NO, "
				//			"                        PROC_NO, "
				//			"                        START_TIME_REAL, "
				//			"                        DECODE(END_TIME_REAL, "
				//			"                               ' ', "
				//			"                               '90140130000000', "
				//			"                               END_TIME_REAL) AS END_TIME_REAL, "
				//			"                        END_TIME,          "
				//			"                        PROC_TIME          "
				//			"                   FROM TPSSM12 A "
				//			"                  WHERE FACTORY_DIV = TRIM(@tpssm12["FACTORY_DIV"].ToString()) "
				//			"                    AND START_TIME_REAL < @proc_time "
				//			"                    AND START_TIME_REAL != ' ' "
				//			"                    AND SM_PLAN_NO != @tpssm11.SM_PLAN_NO   "
				//			"                    AND END_TIME_REAL = ' '  "
				//			//"                      ( OR  END_TIME_REAL <= @proc_time) "
				//			"                    AND AREA_ID = @tpssm12.AREA_ID "
				//			"                    AND DEV_CODE = @tpssm12["DEV_CODE"].ToString()) "
				//			"          ORDER BY END_TIME_REAL DESC) "
				//			"  WHERE ROWNUM = 1 ";
				//		break;
				//	}

				//	cmd_inq.SetCommandText(sqlstr);
				//	cmd_inq.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString());
				//	cmd_inq.Parameters.Set("proc_time", proc_time);
				//	cmd_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				//	cmd_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
				//	cmd_inq.Parameters.Set("tpssm12.DEV_CODE", tpssm12["DEV_CODE"].ToString());

				//	cmd_inq.ExecuteReader();

				//	if (cmd_inq.Read())
				//	{
				//		v_sm_plan_no = cmd_inq.GetString(1);
				//		v_proc_no = cmd_inq.GetString(2);
				//		v_start_time_real = cmd_inq.GetString(3);
				//		v_end_time_real = cmd_inq.GetString(4);
				//		v_end_time = cmd_inq.GetString(5);
				//		v_pono_proc_time = cmd_inq.GetInt32(6);

				//		////Log::Trace("", __FUNCTION__, "设备上[{0}],上一炉处理号v_proc_no = [{1}]实际结束时间[{2}]", tpssm12["DEV_CODE"].ToString(), v_proc_no, v_end_time_real);

				//		//if (v_end_time_real == proc_time)
				//		//{
				//		//	do_flag = "0";
				//		//}

				//		//若前一炉生产没有结束，则将前一炉模拟生产结束信号
				//		if ((v_end_time_real.TrimOrBlank() == "90140130000000") && (do_flag == "1"))
				//		{

				//			sqlstr = "SELECT PONO FROM TPSSM11 WHERE SM_PLAN_NO = @SM_PLAN_NO AND FACTORY_DIV = @FACTORY_DIV ";
				//			cmd_tpssm11_inq.SetCommandText(sqlstr);
				//			cmd_tpssm11_inq.Parameters.Set("SM_PLAN_NO", v_sm_plan_no);
				//			cmd_tpssm11_inq.Parameters.Set("FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString());
				//			cmd_tpssm11_inq.ExecuteReader();

				//			if (cmd_tpssm11_inq.Read())
				//			{
				//				v_pono = cmd_tpssm11_inq.GetString(1);
				//				////Log::Info("", __FUNCTION__, "v_pono = [{0}]", v_pono);
				//			}
				//			cmd_tpssm11_inq.Close();

				//			v_end_time_real = CDateTime::Parse(v_start_time_real).AddMinutes(v_pono_proc_time).ToString("yyyyMMddHHmmss");

				//			if (v_end_time_real > proc_time)
				//			{
				//				CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
				//				CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于上炉结束时间[{1}]。", arguments, 2); //格式化字符串
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}

				//			////Log::Trace("", __FUNCTION__, "v_pono = [{0}],v_end_time_real = [{1}]", (const char*)v_pono, (const char*)v_end_time_real);

				//			if (tpssmd1["AREA_ID"].ToDecimal() == 3)
				//			{
				//				if (tpssms1["DEV_CODE"].ToString() == "B1")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "316";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "B2")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "326";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "B3")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "336";
				//				}
				//			}
				//			else if (tpssmd1["AREA_ID"].ToDecimal() == 5)
				//			{
				//				if (tpssms1["DEV_CODE"].ToString() == "C1")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "514";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "C2")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "524";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "C3")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "534";
				//				}
				//				else if (tpssms1["DEV_CODE"].ToString() == "C4")
				//				{
				//					inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "544";
				//				}
				//			}


				//			inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
				//			inBlock.Tables[0].Rows[0]["PONO"] = v_pono;
				//			inBlock.Tables[0].Rows[0]["PROC_NO"] = v_proc_no;
				//			inBlock.Tables[0].Rows[0]["PROC_TIME"] = v_end_time_real;
				//			inBlock.Tables[0].Rows[0]["DEV_CODE"] = tpssms1["DEV_CODE"];
				//			inBlock.Tables[0].Rows[0]["SIMUL_FLAG"] = simul_flag;
				//			inBlock.Tables[0].Rows[0]["AREA_ID"] = tpssmd1["AREA_ID"];
				//			inBlock.Tables[0].Rows[0]["SRP_SEQ"] = 1;

				//			////Log::Trace("", __FUNCTION__, "v_pono = [{0}]", v_pono);
				//			////Log::Trace("", __FUNCTION__, "v_end_time_real = [{0}]", v_end_time_real);

				//			//作业计划运转状态处理函数
				//			ret = f_pssm11_run_upd_n(&inBlock, bcls_ret, conn);
				//			if (ret != 0)
				//			{
				//				throw CApplicationException(-1, s.msg, log.Location);
				//			}

				//			v_end_time_real = v_end_time;
				//		}
				//	}
				//	cmd_inq.Close();

				//	//如果结束时间不为空
				//	if (v_end_time_real == " ")
				//	{
				//		v_end_time_real = CDateTime::Parse(proc_time).AddMinutes(-1 * tpssm12["PROC_TIME"].ToDecimal().ToDouble()).ToString("yyyyMMddHHmmss");
				//	}
				//	else
				//	{
				//		start_time = CDateTime::Parse(v_end_time_real);
				//		end_time = CDateTime::Parse(proc_time);
				//		proc_time_dif = end_time - start_time;
				//		c_dif = proc_time_dif.TotalMinutes();

				//		//小于开始时间与结束时间小于5分钟，忽略结束信号
				//		if (c_dif >= 60)
				//		{
				//			v_end_time_real = CDateTime::Parse(proc_time).AddMinutes(-1 * tpssm12["PROC_TIME"].ToDecimal().ToDouble()).ToString("yyyyMMddHHmmss");
				//		}
				//	}

				//	if (v_end_time_real > proc_time)
				//	{
				//		CFormattable arguments[] = { proc_time, v_end_time_real_back }; // 定义参数列表的数组
				//		CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于开始时间[{1}]。", arguments, 2); //格式化字符串
				//		throw CApplicationException(-1, s.msg, log.Location);
				//	}

				//	//由于该炉次的开始时间为空，故模拟炉次的开始时间
				//	if (do_flag == "1")
				//	{
				//		if (tpssmd1["AREA_ID"].ToDecimal() == 3)
				//		{
				//			if (tpssms1["DEV_CODE"].ToString() == "B1")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "311";
				//			}
				//			else if (tpssms1["DEV_CODE"].ToString() == "B2")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "321";
				//			}
				//			else if (tpssms1["DEV_CODE"].ToString() == "B3")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "331";
				//			}
				//		}
				//		else if (tpssmd1["AREA_ID"].ToDecimal() == 5)
				//		{
				//			if (tpssms1["DEV_CODE"].ToString() == "C1")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "512";
				//			}
				//			else if (tpssms1["DEV_CODE"].ToString() == "C2")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "522";
				//			}
				//			else if (tpssms1["DEV_CODE"].ToString() == "C3")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "532";
				//			}
				//			else if (tpssms1["DEV_CODE"].ToString() == "C4")
				//			{
				//				inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = "542";
				//			}
				//		}

				//		inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
				//		inBlock.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
				//		inBlock.Tables[0].Rows[0]["PROC_NO"] = proc_no;
				//		inBlock.Tables[0].Rows[0]["PROC_TIME"] = v_end_time_real;
				//		inBlock.Tables[0].Rows[0]["DEV_CODE"] = tpssms1["DEV_CODE"];
				//		inBlock.Tables[0].Rows[0]["SIMUL_FLAG"] = simul_flag;
				//		inBlock.Tables[0].Rows[0]["AREA_ID"] = tpssmd1["AREA_ID"];
				//		inBlock.Tables[0].Rows[0]["SRP_SEQ"] = 1;

				//		////Log::Trace("", __FUNCTION__, "tpssm11["PONO"] = [{0}]", tpssm11["PONO"].ToString());
				//		////Log::Trace("", __FUNCTION__, "v_end_time_real = [{0}]", v_end_time_real);

				//		//作业计划运转状态处理函数
				//		ret = f_pssm11_run_upd_n(&inBlock, bcls_ret, conn);
				//		if (ret != 0)
				//		{
				//			throw CApplicationException(-1, s.msg, log.Location);
				//		}
				//	}
				//}

				////判断连铸结束时候后已经有开始的炉次
				//if ((do_flag == "1") && (tpssm12["AREA_ID"].ToDecimal() == 5))
				//{
				//	////Log::Trace("", __FUNCTION__, "tpssm11["PONO_STATUS"] = [{0}]", tpssm11["PONO_STATUS"].ToDecimal());

				//	if ((tpssm11["PONO_STATUS"].ToDecimal() == 20) || (tpssm11["PONO_STATUS"].ToDecimal() == 83))
				//	{
				//		switch (conn->DatabaseKind)
				//		{
				//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//		case DB_KIND_ORACLE:	        // Oracle 数据库
				//		default:
				//			sqlstr = " SELECT COUNT(1) "
				//				"	FROM TPSSM12 A "
				//				"	WHERE FACTORY_DIV = TRIM(@tpssm12["FACTORY_DIV"].ToString())  "
				//				"		AND START_TIME_REAL < @proc_time "
				//				"		AND START_TIME_REAL != ' ' "
				//				"      AND SM_PLAN_NO != @tpssm11.SM_PLAN_NO  "
				//				"		AND (END_TIME_REAL = ' ' "
				//				"			OR END_TIME_REAL > @proc_time)   "
				//				"		AND AREA_ID = @tpssm12.AREA_ID  "
				//				"		AND DEV_CODE = @tpssm12.DEV_CODE ";
				//			break;
				//		}

				//		cmd_inq.SetCommandText(sqlstr);
				//		cmd_inq.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString());
				//		cmd_inq.Parameters.Set("proc_time", proc_time);
				//		cmd_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				//		cmd_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
				//		cmd_inq.Parameters.Set("tpssm12.DEV_CODE", tpssm12["DEV_CODE"].ToString());

				//		if (cmd_inq.ExecuteScalar().ToInt32() > 0)
				//		{
				//			do_flag = "0";
				//		}
				//	}

				//	////Log::Trace("", __FUNCTION__, "do_flag = [{0}]", (const char*)do_flag);
				//}
			}
			else if ((tpssms1["START_OR_END"].ToDecimal() == 3) && (tpssmd1["AREA_ID"].ToDecimal() == 2))
			{
				////Log::Info("", __FUNCTION__, "判断开始时间与结束时间差值!!!");
				//判断开始时间与结束时间差值
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["DEV_CODE"] = tpssms1["DEV_CODE"];
				tpssm12["AREA_ID"] = tpssmd1["AREA_ID"];
				tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm12["CHARGE_NO"] = v_charge_no_2;
				////Log::Info("", __FUNCTION__, "tpssm12.CHARGE_No = [{0}]", tpssm12["CHARGE_NO"].ToDecimal());

				tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, DEV_CODE, AREA_ID, CHARGE_NO");

				if (tpssm12["START_TIME_REAL"].ToString().TrimOrBlank() == " ")
				{
					CFormattable arguments[] = { tpssm12["START_TIME_REAL"].ToString(), proc_time }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "精炼开始时间为空[{0}]不能接受结束信号[{1}]。", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
				}

				if ((tpssm12["START_TIME_REAL"].ToString() > proc_time) && (simul_flag == "0"))
				{
					CFormattable arguments[] = { proc_time, tpssm12["START_TIME_REAL"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "信号结束时间[{0}]不能小于开始时间[{1}]。", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					//Log::Trace("", __FUNCTION__, "666666666666666666");
					//Log::Trace("", __FUNCTION__, "666666666666666666[{0}]", tpssm12["START_TIME_REAL"].ToString());
					//start_time = CDateTime::Parse(tpssm12["START_TIME_REAL"].ToString());
					//Log::Trace("", __FUNCTION__, "666666666666666666444");
					//end_time = CDateTime::Parse(proc_time);
					//Log::Trace("", __FUNCTION__, "666666666666655555");
					//proc_time_dif = end_time - start_time;
					//c_dif = proc_time_dif.TotalMinutes();

					//小于开始时间与结束时间小于5分钟，忽略结束信号
					//if ((c_dif <= 5) && (simul_flag == "0"))
					//{
					//CFormattable arguments[] = { proc_time, tpssm12["START_TIME_REAL"].ToString() }; // 定义参数列表的数组
					//CMessageFormat::Format(s.msg, "信号结束时间[{0}]与开始时间[{1}]不能小于5分钟。", arguments, 2); //格式化字符串
					//throw CApplicationException(-1, s.msg, log.Location);
					//}
				}
			}
			//inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			//inBlock.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
			//inBlock.Tables[0].Rows[0]["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
			//inBlock.Tables[0].Rows[0]["PROC_NO"] = proc_no;
			//inBlock.Tables[0].Rows[0]["PROC_TIME"] = proc_time;
			//inBlock.Tables[0].Rows[0]["DEV_CODE"] = tpssms1["DEV_CODE"];
			//inBlock.Tables[0].Rows[0]["SIMUL_FLAG"] = simul_flag;
			//inBlock.Tables[0].Rows[0]["AREA_ID"] = tpssmd1["AREA_ID"];
			//inBlock.Tables[0].Rows[0]["SRP_SEQ"] = srp_seq;

			//////Log::Trace("", __FUNCTION__, "dev_code = [{0}]", tpssms1["DEV_CODE"].ToString());

			//if (do_flag == "1")
			//{
			//	//作业计划运转状态处理函数
			//	ret = f_pssm11_run_upd_n(&inBlock, bcls_ret, conn);
			//	if (ret != 0)
			//	{
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			//}
		}

		//2)通过运转信号查询PONO状态
		
		inBlock.Tables[0].Columns.Add(DT_STRING,"FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING,"PONO");
		inBlock.Tables[0].Columns.Add(DT_STRING,"RUN_SIGNAL");//运转信号
		inBlock.Tables[0].Columns.Add(DT_STRING,"PROC_NO");//处理号
		inBlock.Tables[0].Columns.Add(DT_STRING,"PROC_TIME");//处理时刻
		inBlock.Tables[0].Columns.Add(DT_STRING,"DEV_CODE");//设备代码
		inBlock.Tables[0].Columns.Add(DT_STRING,"SIMUL_FLAG");//模拟标记
		inBlock.Tables[0].Columns.Add(DT_STRING,"AREA_ID");//炼钢区域标识
		inBlock.Tables[0].Columns.Add(DT_STRING,"SRP_SEQ");//精炼重数
		inBlock.Tables[0].Columns.Add(DT_STRING, "CHARGE_NO_2");//预处理重数
		inBlock.Tables[0].Columns.Add(DT_STRING, "EVENT_ID");
		inBlock.Tables[0].Columns.Add(DT_STRING, "STATUS_NAME");
		inBlock.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "LADLE_NO");
		inBlock.Tables[0].Rows.Add();

		inBlock.Tables[0].Rows[0]["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
		inBlock.Tables[0].Rows[0]["PONO"]=tpssm11["PONO"];
		inBlock.Tables[0].Rows[0]["RUN_SIGNAL"]=tpssms1["RUN_SIGNAL"];
		inBlock.Tables[0].Rows[0]["PROC_NO"]=proc_no;
		inBlock.Tables[0].Rows[0]["PROC_TIME"]=proc_time;
		inBlock.Tables[0].Rows[0]["DEV_CODE"]=tpssms1["DEV_CODE"];
		inBlock.Tables[0].Rows[0]["SIMUL_FLAG"]=simul_flag;
		inBlock.Tables[0].Rows[0]["AREA_ID"]= tpssmd1["AREA_ID"];
		inBlock.Tables[0].Rows[0]["SRP_SEQ"] = srp_seq;
		inBlock.Tables[0].Rows[0]["STATUS_NAME"] = tpssms1["STATUS_NAME"];
		inBlock.Tables[0].Rows[0]["EVENT_ID"] = tpssms1["EVENT_ID"];
		inBlock.Tables[0].Rows[0]["CHARGE_NO_2"] = bcls_rec->Tables[2].Rows[0]["CHARGE_NO_2"].ToDecimal();
		inBlock.Tables[0].Rows[0]["HEAT_NO"] = v_heat_no;
		inBlock.Tables[0].Rows[0]["LADLE_NO"] = ladle_no;

		////Log::Trace("", __FUNCTION__, "f_pssm_run_proc_n>dev_code=[%s]",(const char*)tpssms1["DEV_CODE"].ToString());
		if (tpssms1["IS_IMPORT"].ToDecimal() == 1)//重要标记, 说明对照表中有
		{
			//作业计划运转状态处理函数
			ret = f_pssm11_run_upd_n(&inBlock, bcls_ret,conn);
			if (ret != 0)
			{
				throw CApplicationException(-1,s.msg,log.Location);
			}
			
		}

		//1.更新运转状态监控表
		ret = f_pssm33_run_upd_n(bcls_rec, bcls_ret,conn);
		if (ret != 0)
		{
			throw CApplicationException(-1,s.msg,log.Location);
		}

		//事项信号履历记录函数  upd by lj不管是否重要标记，都写34表
		ret = f_pssm34_ins_n(&inBlock, bcls_ret,conn);
		if (ret != 0)
		{
			throw CApplicationException(-1,s.msg,log.Location);
		}
		//if (tpssms1["COMPANY_NAME"].ToString().Trim() == "1")
		//{

		//	sqlstr = "SELECT * FROM TPSSM11 WHERE FACTORY_DIV=@v_factory_div AND PONO_STATUS<83 ";
		//	sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		//	cmd_inq.SetCommandText(sqlstr);
		//	cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		//	cmd_inq.ExecuteQuery(tb_tpssm11.Tables[0]);
		//	Log::Trace("", __FUNCTION__, "1=[{0}]", tb_tpssm11.Tables[0].Rows.get_Count());
		//	//校验可编计划数如果大于0才需要优化
		//	if (tb_tpssm11.Tables[0].Rows.get_Count() > 0)
		//	{
		//		ret = f_pssm_call_tps_n(v_factory_div, mode, conn);
		//		if (ret < 0)
		//		{
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}
		//	cmd_inq.Close();
		//}
		// 4.发送事件给前台, 进行数据刷新;
		//sprintf(message, "接收到运转信号[%s][%s]：[%s]",(const char*)tpssms1["DEV_CODE"].ToString(),(const char*)tpssms1["RUN_SIGNAL"].ToString(),(const char*)tpssms1["RUN_SIGNAL"].ToString()_desc);
		//f_epen_route_event(1000, message);
		//Log::Trace("", "RUN_SIGNAL = [{0}]", tpssms1["RUN_SIGNAL"].ToString());
		if (tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "5" && tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(2, 1) == "2")
		{
			ret = f_pssm21_cast_cre_n(&inBlock2, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		if ((tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "3" && tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(2, 1) == "7") || (tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "5" && tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(2, 1) == "3"))
		{
			inblocktmsm.Tables["TMSM_MSG"].Rows.Add();
			tpssm11_send["PONO"] = bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
			tpssm11_send.Query("PONO");
			inblocktmsm.Tables["TMSM_MSG"].Rows[0]["HEAT_NO"] = tpssm11_send["HEAT_NO"];
			inblocktmsm.Tables["TMSM_MSG"].Rows[0]["MSG"] = tpssms1["RUN_SIGNAL"].ToString().Trim();
			inblocktmsm.Tables["TMSM_MSG"].Rows[0]["STATE_TIME"] = proc_time;
			ret = f_tmsm_mag(&inblocktmsm, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		//if (tpssms1["START_OR_END"].ToDecimal() == 2 || tpssms1["START_OR_END"].ToDecimal() == 3)//重要标记, 说明对照表中有
		//{
		//	inblockmmsm.Tables[0].Rows.Add();
		//	tpssm11_send["PONO"] = bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
		//	tpssm11_send.Query("PONO");
		//	if (tpssm11_send["SM_PLAN_NOL2_TEST"].ToString().Trim() != "")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["SM_PLAN_NOL2"] = tpssm11_send["SM_PLAN_NOL2_TEST"];
		//	}
		//	else inblockmmsm.Tables[0].Rows[0]["SM_PLAN_NOL2"] = tpssm11_send["SM_PLAN_NOL2"];
		//	inblockmmsm.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11_send["SM_PLAN_NO"];
		//	inblockmmsm.Tables[0].Rows[0]["HEAT_NO"] = tpssm11_send["HEAT_NO"];
		//	inblockmmsm.Tables[0].Rows[0]["PONO"] = tpssm11_send["PONO"];
		//	inblockmmsm.Tables[0].Rows[0]["PROC_NO"] = proc_no;
		//	inblockmmsm.Tables[0].Rows[0]["TREATMENT_COUNTER"] = treatment_count;
		//	//Log::Trace("", "DEV_CODE = [{0}]", tpssms1["DEV_CODE"].ToString());
		//	if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "Z")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM19";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "E")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM20";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "B")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM21";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "R")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM23";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "F")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM24";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "V")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM25";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "S")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM26";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "A")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM27";
		//	}
		//	else if (tpssms1["DEV_CODE"].ToString().Trim().Substring(0, 1) == "C")
		//	{
		//		inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"] = "TMMSM31";
		//	}
		//	//inblockmmsm.Tables[0].Rows[0]["STATE_TIME"] = proc_time;
		//	Log::Trace("", __FUNCTION__, "SM_PLAN_NOL2=[{0}] TABLE_NAME=[{1}]", inblockmmsm.Tables[0].Rows[0]["SM_PLAN_NOL2"].ToString(), inblockmmsm.Tables[0].Rows[0]["TABLE_NAME"].ToString());
		//	Log::Trace("", __FUNCTION__, "SM_PLAN_NO=[{0}] HEAT_NO=[{1}]", tpssm11_send["SM_PLAN_NO"].ToString(), tpssm11_send["HEAT_NO"].ToString());
		//	Log::Trace("", __FUNCTION__, "PONO=[{0}] PROC_NO=[{1}]", tpssm11_send["PONO"].ToString(), proc_no);
		//	ret = f_mmsm009e_snd(&inblockmmsm, bcls_ret, conn);
		//	if (ret < 0)
		//	{
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

		if ((tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "3" && tpssms1["START_OR_END"].ToDecimal() == 2) || (tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "3" && tpssms1["START_OR_END"].ToDecimal() == 3))
		{
			tpssm11_send["PONO"] = bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
			tpssm11_send.Query("PONO");
			inBlockdealtime.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11_send["SM_PLAN_NO"].ToString();

			ret = f_pssm_deal_pre_plan(&inBlockdealtime, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}


			inblockqm.Tables[0].Rows.Add();
			inblockqm.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11_send["SM_PLAN_NO"].ToString();
			inblockqm.Tables[0].Rows[0]["HEAT_NO"] = tpssm11_send["HEAT_NO"].ToString();
			inblockqm.Tables[0].Rows[0]["ST_NO"] = tpssm11_send["ST_NO"].ToString();
			inblockqm.Tables[0].Rows[0]["PONO"] = tpssm11_send["PONO"].ToString();

			ret = f_qmts_23_init(&inblockqm, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		if (((tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "4" && tpssms1["START_OR_END"].ToDecimal() == 2) || (tpssms1["RUN_SIGNAL"].ToString().Trim().Substring(0, 1) == "4" && tpssms1["START_OR_END"].ToDecimal() == 3)) && v_heat_no.SubstringNE(1,1) == "9")
		{
			tpssm11_send["PONO"] = bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
			tpssm11_send.Query("PONO");
			inBlockdealtime.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11_send["SM_PLAN_NO"].ToString();

			ret = f_pssm_deal_pre_plan2(&inBlockdealtime, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}
