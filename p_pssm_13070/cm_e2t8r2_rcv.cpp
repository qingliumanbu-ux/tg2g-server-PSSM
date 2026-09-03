/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-10-12
Version:1.0
Description: 甘特图信号模拟
Update:
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h"
#include "CUtils.h"

//程序用头文件


void SendMessage(const std::string &module, const std::string &topic, const std::string& msg);//消息推送
int f_pssm_call_tps_n(CString main_backlog_code, int mode, CDbConnection * conn);//
int f_pssm_run_proc_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_qmts_23_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_qmts_23_init(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm_deal_pre_plan(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssmss_insert(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_plan_delete_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm12z_combine(CString ladle_no, CString heat_no, CString sm_plan_no, CDbConnection * conn);
//int f_pssm_chg_in(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//甘特图炉次生成CAST号
//int f_tmsm_mag(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 炉次状态查询
/// <para>甘特图信号模拟。                            </para>
/// <para>数据库表：tpssm11                    </para>
/// <para>主调用函数：PSSM21O画面调用。                </para>
/// </summary>
/// <param name="pono">制造命令          </param>
/// <param name="dev_code">设备代码     </param>
/// <param name="proc_time">处理时间     </param>
/// <param name="signal">信号代码        </param>
/// <param name="charge_no">工序号        </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE_TELE(cm_e2t8r2_rcv)
//-EP_SYSTEM_HEAD_END
int f_cm_e2t8r2_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0, ret = 0;
	CString	v_proc_time = "", v_proc_no = "", v_dev_code = "", v_run_signal = "", v_heat_no = "";
	CDecimal  v_charge_no = 0, v_charge_no_min = 0, v_area_id = 0, dev_count = 0;
	CDecimal v_flag = 0;//2-开始，3-结束
	CDecimal resume_seq_no = 0;
	int		dummy;
	CString v_factory_div = "LG1";
	CDecimal v_srp_seq = 0;
	CDecimal treatment_count = 0;
	CDecimal    iproc_no = 0;
	CDecimal RN = 0;
	CString sqlstr = "";
	CString cs_time_column = "";
	CString sqlstr_tpssm12 = "";
	CString sqlstr_count = "";
	CString   simul_flag = "0";              /* 模拟标记: 1- 模拟*/
	CString   event_id = "";
	CString   status_name = "";
	CString   run_status = "";
	CString sys_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString mx_flag = "";
	CString test_pono = "";
	CString test_plan_no = "";
	CString user = "";
	CString planl2 = "";
	CString cal_flag = "";
	CString ladle_no = "";
	CString  datetime;
	CString cs_st_no = " ";
	CDateTime eventtime;
	CDateTime eventtime2;
	CDateTime eventtime3;
	CModel tpssmss("TPSSMSS");
	CString func_back_mess;

	int mode = 1;
	CModel tpssm11("TPSSM11");
	//CModel tpssm11_chg("TPSSM11");
	CModel tpssms1("TPSSMS1");
	CModel tpssm12_check("TPSSM12");
	CModel tpssm12("TPSSM12");
	CModel tpssm12z("TPSSM12Z");
	CModel tpssm12zt("TPSSM12ZT");
	CModel tpssm25("TPSSM25");
	CModel tpssm33("TPSSM33");
	CModel tpssm34("TPSSM34");
	CModel tpssm99("TPSSM99");//履历
	EIClass tb_tpssm11;
	EIClass inBlock;
	//EIClass inBlock2;
	//调用函数用
	inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inBlock.Tables[0].Columns.Add(DT_STRING, "SIMUL_FLAG");//模拟标记
	inBlock.Tables[0].Columns.Add(DT_STRING, "PROC_NO");//处理号
	inBlock.Tables[0].Columns.Add(DT_STRING, "PROC_TIME");//处理时刻
	inBlock.Tables.Add();
	inBlock.Tables[1].Columns.Add(DT_STRING, "PONO");
	inBlock.Tables.Add();
	inBlock.Tables[2].Columns.Add(DT_STRING, "RUN_SIGNAL");//运转信号
	inBlock.Tables[2].Columns.Add(DT_STRING, "AREA_ID");//炼钢区域标识
	inBlock.Tables[2].Columns.Add(DT_STRING, "SRP_SEQ");//精炼重数
	inBlock.Tables[2].Columns.Add(DT_STRING, "CHARGE_NO_2");//预处理重数
	inBlock.Tables[2].Columns.Add(DT_STRING, "HEAT_NO");//覆盖炉号
	inBlock.Tables[2].Columns.Add(DT_STRING, "LADLE_NO");
	inBlock.Tables[2].Columns.Add(DT_DECIMAL, "TREATMENT_COUNTER");//覆盖炉号

	//inBlock2.Tables[0].set_TableName("PLAN");  //
	//inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	//inBlock2.Tables[0].Rows.Add();
	//inBlock2.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;

	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);

	EIClass inblocktmsm;//调用履历函数
	inblocktmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblocktmsm.Tables[0].Columns.Add(DT_STRING, "MSG");
	inblocktmsm.Tables[0].Columns.Add(DT_STRING, "STATE_TIME");
	inblocktmsm.Tables[0].set_TableName("TMSM_MSG");

	EIClass inblockqm;//调用质量函数
	inblockqm.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblockqm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblockqm.Tables[0].Columns.Add(DT_STRING, "ST_NO");
	inblockqm.Tables[0].Columns.Add(DT_STRING, "PONO");
	//inblockqm.Tables[0].set_TableName("TMSM_MSG");

	EIClass inblockchgin;
	inblockchgin.Tables[0].set_TableName("HEAT_CHG");  //钢水对换
	inblockchgin.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inblockchgin.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");

	EIClass inblockdel;
	inblockdel.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblockdel.Tables[0].Rows.Add();
	//inblockdel.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	/*inblockchgin.Tables[0].Rows.Add();
	inblockchgin.Tables[0].Rows[0]["SM_PLAN_NO"] = (string)args[1];
	inblockchgin.Tables[0].Rows[0]["FACTORY_DIV"] = factory_div;
	inblockchgin.Tables[0].Rows.Add();
	inblockchgin.Tables[0].Rows[1]["SM_PLAN_NO"] = (string)args[2];
	inblockchgin.Tables[0].Rows[1]["FACTORY_DIV"] = factory_div;*/

	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);
	CDbCommand cmd_tpssm25_upd(conn);
	CDbCommand cmd_tpssms1_inq(conn);
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_count(conn);
	CDbCommand cmd_exe(conn);
	try
	{
		bcls_rec->Tables[0].Columns.Add(DT_STRING, "NOTE");
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获取传入参数
		tpssm11["FACTORY_DIV"] = "LG1";
		tpssm11["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"].ToString();
		//tpssm11_chg["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"];
		tpssm11["SM_PLAN_NOL2"] = bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"].ToString();
		tpssm11["SPLIT_INDICATION"] = bcls_rec->Tables[0].Rows[0]["SPLIT_INDICATION"].ToString();
		v_dev_code = bcls_rec->Tables[0].Rows[0]["AGGREGATE_NAME"].ToString();
		v_proc_time = bcls_rec->Tables[0].Rows[0]["EVENT_TIME"].ToString();
		event_id = bcls_rec->Tables[0].Rows[0]["EVENT"].ToString();
		status_name = bcls_rec->Tables[0].Rows[0]["HEAT_STATUS"].ToString();
		v_heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"].ToString();
		planl2 = bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"].ToString().TrimOrBlank();
		treatment_count = bcls_rec->Tables[0].Rows[0]["TREATMENT_COUNTER"].ToDecimal();
		ladle_no = bcls_rec->Tables[0].Rows[0]["LADLE_NUMBER"].ToString().TrimOrBlank();
		cs_st_no = bcls_rec->Tables[0].Rows[0]["GRADE"].ToString().TrimOrBlank();
		bcls_rec->Tables[0].Rows[0]["NOTE"] = "";
		if ((v_dev_code.SubstringNE(0, 1) == "B") && (event_id.Trim() == "6") && (status_name.Trim() == "BLOW"))
		{
			tpssmss["DEV_CODE"] = bcls_rec->Tables[0].Rows[0]["AGGREGATE_NAME"];
			tpssmss["EVENT_ID"] = bcls_rec->Tables[0].Rows[0]["EVENT"];
			tpssmss["STATUS_NAME"] = bcls_rec->Tables[0].Rows[0]["HEAT_STATUS"];
			tpssmss["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"];
			Log::Trace("", __FUNCTION__, "取信号第一条SM_PLAN_NO[{0}]", tpssmss["SM_PLAN_NO"].ToString());
			if (tpssmss.QueryCount("SM_PLAN_NO,STATUS_NAME,EVENT_ID,DEV_CODE") > 0)
			{
				return 0;
			}
		}
		if ((v_dev_code.SubstringNE(0, 1) == "F") && (event_id.Trim() == "6") && (status_name.Trim() == "StirST"))
		{
			tpssmss["DEV_CODE"] = bcls_rec->Tables[0].Rows[0]["AGGREGATE_NAME"];
			tpssmss["EVENT_ID"] = bcls_rec->Tables[0].Rows[0]["EVENT"];
			tpssmss["STATUS_NAME"] = bcls_rec->Tables[0].Rows[0]["HEAT_STATUS"];
			tpssmss["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"];
			if (tpssmss.QueryCount("SM_PLAN_NO,STATUS_NAME,EVENT_ID,DEV_CODE") > 0)
			{
				return 0;
			}
		}
		ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
		/*if (ret < 0)
		{
		func_back_mess = s.msg;
		Log::Trace("", __FUNCTION__, "func_back_mess = [{0}]", func_back_mess);

		if (func_back_mess == "数据库处理出错，sqlcode=[12899]。请稍后再试或联系系统维护人员。")
		{
		throw CApplicationException(-9, s.msg, log.Location);
		}
		}*/

		//tpabort(0);
		//tpbegin(0, 0);
		sqlstr = "  SELECT DA_SCHEDULE_UPDATE_SEQ.nextval FROM dual  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			resume_seq_no = cmd_inq.GetDecimal(1);
		}

		eventtime = CDateTime::Parse(bcls_rec->Tables[0].Rows[0]["EVENT_TIME"].ToString());
		eventtime2 = CDateTime::Parse(bcls_rec->Tables[0].Rows[0]["REQUIRED_LADLE_OPEN"].ToString());
		eventtime3 = CDateTime::Parse(bcls_rec->Tables[0].Rows[0]["PLANNED_LF_END_TIME"].ToString());

		sqlstr = " INSERT INTO DA_SCHEDULE_UPDATE "
			" (ID,AGGREGATECODE,EVENT,EVENTTIME,HEATSTATUS,HEATNUMBER,TREATMENTCOUNTER,PLANID,SPLITINDICATION,GRADE,LADLENUMBER,REQUIREDLADLEOPEN,PLANNEDLFENDTIME,NEXTAGGREGATE,TIMESTAMP,PPREADTIME,PPREAD,DMREADTIME,DMREAD) "
			" VALUES(@resume_seq_no, @v_dev_code, @event_id, @v_proc_time, @status_name, @heat_no, @treatmentcount, @plan_no, @splitno, @st_no, @ladle_no, @ladleopen, @lfend, @next_dev_no, SYSDATE, @PPREADTIME, @PPREAD, @DMREADTIME, @DMREAD) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("resume_seq_no", resume_seq_no);
		cmd_inq.Parameters.Set("v_dev_code", bcls_rec->Tables[0].Rows[0]["AGGREGATE_NAME"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("event_id", bcls_rec->Tables[0].Rows[0]["EVENT"].ToDecimal());
		cmd_inq.Parameters.Set("v_proc_time", eventtime);
		cmd_inq.Parameters.Set("status_name", bcls_rec->Tables[0].Rows[0]["HEAT_STATUS"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("heat_no", bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("treatmentcount", bcls_rec->Tables[0].Rows[0]["TREATMENT_COUNTER"].ToDecimal());
		cmd_inq.Parameters.Set("plan_no", bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("splitno", bcls_rec->Tables[0].Rows[0]["SPLIT_INDICATION"].ToDecimal());
		cmd_inq.Parameters.Set("st_no", bcls_rec->Tables[0].Rows[0]["GRADE"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("ladle_no", bcls_rec->Tables[0].Rows[0]["LADLE_NUMBER"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("ladleopen", eventtime2);
		cmd_inq.Parameters.Set("lfend", eventtime3);
		cmd_inq.Parameters.Set("next_dev_no", bcls_rec->Tables[0].Rows[0]["NEXT_AGGREGATE"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("PPREADTIME", "");
		cmd_inq.Parameters.Set("PPREAD", "N");
		cmd_inq.Parameters.Set("DMREADTIME", "");
		cmd_inq.Parameters.Set("DMREAD", "N");
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		tpcommit(0);
		tpbegin(0, 0);

		//计划号超长处理
		if (tpssm11["SM_PLAN_NOL2"].ToString().Trim().GetLength() > 8)
		{
			tpssm11["SM_PLAN_NOL2"] = planl2.Substring(0, 8);
		}

		sqlstr = " SELECT * FROM TPSSM11 WHERE SM_PLAN_NOL2 = @SM_PLAN_NOL2 FOR UPDATE ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("SM_PLAN_NOL2", tpssm11["SM_PLAN_NOL2"].ToString());
		cmd_inq.ExecuteReader();
		cmd_inq.Close();

		sqlstr = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NOL2 = @SM_PLAN_NOL2 FOR UPDATE ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("SM_PLAN_NOL2", tpssm11["SM_PLAN_NOL2"].ToString());
		cmd_inq.ExecuteReader();
		cmd_inq.Close();

		/////////////////////中频炉补丁///////////////////

		if (bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"].ToString() == "11111111")
		{
			tpssm12z.Reset();
			if (v_dev_code.SubstringNE(0, 1) == "Z")
			{
				if (event_id == "3" && status_name == "HeatStart")
				{
					tpssm12z["DEV_CODE"] = v_dev_code;
					tpssm12z["PROC_NO"] = v_heat_no;
					tpssm12z["GRADE_ID"] = bcls_rec->Tables[0].Rows[0]["GRADE"].ToString().TrimOrBlank();
					tpssm12z["LADLE_NO"] = ladle_no;
					tpssm12z["START_TIME"] = v_proc_time;
					tpssm12z["START_TIME_REAL"] = v_proc_time;
					tpssm12z["END_TIME"] = CDateTime::Parse(v_proc_time).AddMinutes(80).ToString("yyyyMMddHHmmss");
					tpssm12z["AREA_ID"] = 2;

					if (tpssm12z.QueryCount("PROC_NO") == 0)
					{
						tpssm12z["REC_CREATOR"] = "XCOM";
						tpssm12z["REC_CREATE_TIME"] = sys_time;
						tpssm12z["ID_SJ"] = "SJ" + bcls_rec->Tables[0].Rows[0]["ID"].ToString();
						tpssm12z.Insert();
					}
					else
					{
						tpssm12z["REC_REVISOR"] = "XCOM";
						tpssm12z["REC_REVISE_TIME"] = sys_time;
						tpssm12z.Update("DEV_CODE,GRADE_ID,LADLE_NO,START_TIME_REAL,END_TIME,REC_REVISOR,REC_REVISE_TIME", "PROC_NO");
					}

					tpssm25["REC_CREATOR"] = s.userid;
					tpssm25["REC_CREATE_TIME"] = datetime;
					tpssm25["FACTORY_DIV"] = "LG1";
					tpssm25["STATION_ID"] = v_dev_code.Substring(0, 1);
					tpssm25["STATION_NO"] = v_dev_code.Substring(1, 1);

					if (tpssm12z["PROC_NO"].ToString().Trim() != "")
					{
						tpssm25["CURR_PROC_NO"] = "Z" + tpssm12z["PROC_NO"].ToString().Substring(1);
					}
					tpssm25.TrimOrBlank();
					//查询炼钢作业计划工位运行信息表(tpssm25)中信息
					dummy = 0;
					////Log::Trace("", __FUNCTION__, "25表处理号[{0}]赋值", tpssm25["CURR_PROC_NO"].ToString());
					dummy = tpssm25.QueryCount("FACTORY_DIV,STATION_ID,STATION_NO");

					if (dummy <= 0)//不存在该工序设备的, 新增该记录, 并赋设备处理号的值
					{
						tpssm25.Insert();
					}
					else
					{
						if (tpssm25["CURR_PROC_NO"].ToString().Trim() != "")
						{
							//tpssm25.Print();
							////Log::Trace("", __FUNCTION__, "开始更新25表处理号[{0}]", tpssm25["CURR_PROC_NO"].ToString());
							//更新对应工序设备的处理号
							sqlstr = " UPDATE TPSSM25 "
								" SET CURR_PROC_NO      = @tpssm25.CURR_PROC_NO "
								" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
								" AND STATION_ID        = @tpssm25.STATION_ID "
								" AND STATION_NO        = @tpssm25.STATION_NO "
								" AND CURR_PROC_NO      < @tpssm25.CURR_PROC_NO ";
							cmd_tpssm25_upd.SetCommandText(sqlstr);
							cmd_tpssm25_upd.Parameters.Set("tpssm25.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString());
							cmd_tpssm25_upd.Parameters.Set("tpssm25.STATION_ID", tpssm25["STATION_ID"].ToString());
							cmd_tpssm25_upd.Parameters.Set("tpssm25.STATION_NO", tpssm25["STATION_NO"].ToString());
							cmd_tpssm25_upd.Parameters.Set("tpssm25.CURR_PROC_NO", tpssm25["CURR_PROC_NO"].ToString());
							cmd_tpssm25_upd.ExecuteNonQuery();
						}
					}
				}
				else if (event_id == "4" && status_name == "HeatEnd")
				{
					tpssm12z["DEV_CODE"] = v_dev_code;
					tpssm12z["PROC_NO"] = v_heat_no;
					tpssm12z["GRADE_ID"] = bcls_rec->Tables[0].Rows[0]["GRADE"].ToString().TrimOrBlank();
					tpssm12z["LADLE_NO"] = ladle_no;
					tpssm12z["END_TIME"] = v_proc_time;
					tpssm12z["END_TIME_REAL"] = v_proc_time;
					tpssm12z["AREA_ID"] = 2;

					if (tpssm12z.QueryCount("PROC_NO") == 0)
					{
						tpssm12z["REC_CREATOR"] = "XCOM";
						tpssm12z["REC_CREATE_TIME"] = sys_time;
						tpssm12z["ID_SJ"] = bcls_rec->Tables[0].Rows[0]["ID"].ToString();
						tpssm12z.Insert();
					}
					else
					{
						tpssm12z["REC_REVISOR"] = "XCOM";
						tpssm12z["REC_REVISE_TIME"] = sys_time;
						tpssm12z.Update("DEV_CODE,GRADE_ID,LADLE_NO,END_TIME_REAL,REC_REVISOR,REC_REVISE_TIME", "PROC_NO");
					}

					//tpssm12zt.Reset();
					//tpssm12zt["LADLE_NO"] = ladle_no;
					//if (tpssm12zt.QueryCount("LADLE_NO") > 0)
					//{
					//	tpssm12zt.Query("LADLE_NO");
					//	tpssm12zt.TrimOrBlank();
					//	if (tpssm12zt["PROC_NO"].ToString() == " ")
					//	{
					//		tpssm12zt["PROC_NO"] = v_heat_no;
					//		tpssm12zt.Update("PROC_NO", "LADLE_NO");
					//	}
					//	if (tpssm12zt["PROC_NO"].ToString() != " "&&tpssm12zt["PROC_NO2"].ToString() == " "&&tpssm12zt["PROC_NO"].ToString() != v_heat_no)
					//	{
					//		tpssm12zt["PROC_NO2"] = v_heat_no;
					//		tpssm12zt.Update("PROC_NO2", "LADLE_NO");
					//	}
					//	if (tpssm12zt["PROC_NO"].ToString() != " "&&tpssm12zt["PROC_NO2"].ToString() != " "&&tpssm12zt["PROC_NO3"].ToString() == " "&&tpssm12zt["PROC_NO"].ToString() != v_heat_no && tpssm12zt["PROC_NO2"].ToString() != v_heat_no)
					//	{
					//		tpssm12zt["PROC_NO3"] = v_heat_no;
					//		tpssm12zt.Update("PROC_NO3", "LADLE_NO");
					//	}
					//	if (tpssm12zt["PROC_NO"].ToString() != " "&&tpssm12zt["PROC_NO2"].ToString() != " "&&tpssm12zt["PROC_NO3"].ToString() != " "&&tpssm12zt["PROC_NO4"].ToString() == " "&&tpssm12zt["PROC_NO"].ToString() != v_heat_no && tpssm12zt["PROC_NO2"].ToString() != v_heat_no && tpssm12zt["PROC_NO3"].ToString() != v_heat_no)
					//	{
					//		tpssm12zt["PROC_NO4"] = v_heat_no;
					//		tpssm12zt.Update("PROC_NO4", "LADLE_NO");
					//	}
					//	if (tpssm12zt["PROC_NO"].ToString() != " "&&tpssm12zt["PROC_NO2"].ToString() != " "&&tpssm12zt["PROC_NO3"].ToString() != " "&&tpssm12zt["PROC_NO4"].ToString() != " "&&tpssm12zt["PROC_NO"].ToString() != v_heat_no && tpssm12zt["PROC_NO2"].ToString() != v_heat_no && tpssm12zt["PROC_NO3"].ToString() != v_heat_no && tpssm12zt["PROC_NO4"].ToString() != v_heat_no)
					//	{
					//		tpssm12zt["REMARK"] = tpssm12zt["REMARK"].ToString() + v_heat_no + ",";
					//		tpssm12zt.Update("REMARK", "LADLE_NO");
					//	}

					//}
					//else
					//{
					//	//tpssm12zt["SMELT_MODE2"] = code;
					//	tpssm12zt["PROC_NO"] = v_heat_no;
					//	tpssm12zt.Insert();
					//}

				}
			}
		}

		//////////////////////////////////////////////////
		/////////////////////并行补丁//////////////////

		//sqlstr = " SELECT USER FROM DUAL ";
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.ExecuteReader();
		//if (cmd_inq.Read())
		//{
		//	Log::Trace("", __FUNCTION__, "USER=[{0}]", cmd_inq.GetString(1));
		//	user = cmd_inq.GetString(1);
		//}

		//if (user == "TGT8Z1")
		//{
		//	sqlstr = " SELECT PPS_ORDER_ID FROM TPSSM_PLAN_TEST WHERE PLAN_NUMBER = @SM_PLAN_NOL2 ";
		//	cmd_inq.SetCommandText(sqlstr);
		//	cmd_inq.Parameters.Set("SM_PLAN_NOL2", tpssm11["SM_PLAN_NOL2"].ToString());
		//	cmd_inq.ExecuteReader();
		//	if (cmd_inq.Read())
		//	{
		//		test_pono = cmd_inq.GetString(1);
		//		test_plan_no = tpssm11["SM_PLAN_NOL2"];

		//		Log::Trace("", __FUNCTION__, "test_pono=[{0}]", test_pono);
		//		tpssm11["PONO"] = test_pono;
		//		//tpssm11["PONO"] = "23000001";
		//		if (tpssm11.QueryCount("PONO") == 1)
		//		{
		//			tpssm11["SM_PLAN_NOL2_TEST"] = test_plan_no;
		//			//tpssm11["SM_PLAN_NOL2_TEST"] = "230000001";
		//			tpssm11.Update("SM_PLAN_NOL2_TEST", "PONO");
		//			tpssm11.Query("PONO");
		//		}
		//	}
		//	else//因对应关系表会每分钟重新写入，所以找不到对应关系时，查找备用表的数据，用来弥补 by 王建征 20240504
		//	{
		//		sqlstr = " SELECT PPS_ORDER_ID FROM TPSSM_PLAN_TEST_BACKUP WHERE PLAN_NUMBER = @SM_PLAN_NOL2 ";
		//		cmd_inq.SetCommandText(sqlstr);
		//		cmd_inq.Parameters.Set("SM_PLAN_NOL2", tpssm11["SM_PLAN_NOL2"].ToString());
		//		cmd_inq.ExecuteReader();
		//		if (cmd_inq.Read())
		//		{
		//			test_pono = cmd_inq.GetString(1);
		//			test_plan_no = tpssm11["SM_PLAN_NOL2"];

		//			Log::Trace("", __FUNCTION__, "test_pono=[{0}]", test_pono);
		//			tpssm11["PONO"] = test_pono;
		//			//tpssm11["PONO"] = "23000001";
		//			if (tpssm11.QueryCount("PONO") == 1)
		//			{
		//				tpssm11["SM_PLAN_NOL2_TEST"] = test_plan_no;
		//				//tpssm11["SM_PLAN_NOL2_TEST"] = "230000001";
		//				tpssm11.Update("SM_PLAN_NOL2_TEST", "PONO");
		//				tpssm11.Query("PONO");
		//			}
		//		}
		//	}
		//	cmd_inq.Close();
		//}

		//////////////////////////////////////////////
		tpssm12["DEV_CODE"] = v_dev_code;
		// 此段IF逻辑判断是否能读取到计划，未能读取到则进行写tpssm33，tpssm34表，写完 结束
		if (!tpssm11.Query("SM_PLAN_NOL2,FACTORY_DIV"))
		{
			//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["SM_PLAN_NOL2"].ToString());
			Log::Trace("", __FUNCTION__, "计划号[{0}]，制造命令[{1}]未排入计划，进入记履历流程", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["PONO"].ToString());
			CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划号[{0}]，制造命令[{1}]未排入计划，进入记履历流程", arguments, 2); //格式化字符串
			bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
			ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);

			tpssms1["EVENT_ID"] = event_id;
			tpssms1["STATUS_NAME"] = status_name;
			tpssms1["DEV_CODE"] = v_dev_code;
			//tpssms1.Print();
			//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			// 传入计划号无法查询到直接写入33和34表
			tpssm34["FACTORY_DIV"] = "LG1";
			tpssm34["PONO"] = tpssm11["SM_PLAN_NOL2"].ToString().TrimOrBlank();
			tpssm34["DEV_CODE"] = v_dev_code;
			if (v_dev_code.Substring(0, 1) == "Z" || v_dev_code.Substring(0, 1) == "D")
			{
				tpssm34["PONO"] = v_heat_no;
			}
			tpssm34["EVENT_ID"] = event_id;
			tpssm34["STATUS_NAME"] = status_name;
			tpssm34["SIMUL_FLAG"] = "0";
			tpssm34["REC_CREATOR"] = s.userid;
			tpssm34["REC_CREATE_TIME"] = sys_time;
			tpssm34["REC_REVISOR"] = s.userid;
			tpssm34["START_TIME"] = v_proc_time;
			sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
				" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
				" AND PONO                = @tpssm34.PONO ";

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
			cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
				Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
			}
			else
			{
				tpssm34["RUN_SEQ"] = 0;
			}
			tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
			tpssm34["PRACT_PONO"] = tpssm34["PONO"];
			//取HEAT_NO
			tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
			tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
			tpssm34["SM_PLAN_NOL2"] = planl2;
			sqlstr = "tpssm34.Insert()";
			tpssm34.TrimOrBlank();
			tpssm34.Insert();

			//33表  update
			tpssm33["PONO"] = " ";
			//更新作业计划监控表的部分字段内容(接口表送入数据)
			tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
			tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
			tpssm33.Query("STATION_ID,STATION_NO");
			tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
			tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
			tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
			tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
			//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
			tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
			tpssm33["START_TIME"] = v_proc_time;				//处理时刻
			tpssm33["ST_NO"] = cs_st_no;
			tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
			tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
			tpssm33["EVENT_ID"] = event_id;
			tpssm33["STATUS_NAME"] = status_name;
			if (v_dev_code.SubstringNE(0, 1) == "V")
			{
				if (status_name == "VACSTART"&&event_id == "3")
				{
					tpssm33["START_TIME_REAL"] = v_proc_time;
				}
				if (event_id == "4")
				{
					tpssm33["END_TIME_REAL"] = v_proc_time;
				}
			}
			else if (v_dev_code.SubstringNE(0, 1) == "F")
			{
				if (status_name == "StirST"&&event_id == "6")
				{
					tpssm33["START_TIME_REAL"] = v_proc_time;
				}
				if (event_id == "4")
				{
					tpssm33["END_TIME_REAL"] = v_proc_time;
				}
			}
			else  if (v_dev_code.SubstringNE(0, 1) == "C")
			{
				if (event_id == "3")
				{
					tpssm33["START_TIME_REAL"] = v_proc_time;
				}
				if (event_id == "8")
				{
					tpssm33["END_TIME_REAL"] = v_proc_time;
				}
			}
			else
			{
				if (event_id == "3")
				{
					tpssm33["START_TIME_REAL"] = v_proc_time;
					tpssm33["END_TIME_REAL"] = "";
				}
				if (event_id == "4")
				{
					tpssm33["END_TIME_REAL"] = v_proc_time;
				}
				if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
				{
					tpssm33["START_TIME_REAL"] = v_proc_time;
					tpssm33["END_TIME_REAL"] = "";
				}
			}
			tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
			tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
			tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
			tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
			//tpssm33.Print();
			tpssm33.TrimOrBlank();
			tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
				, "STATION_ID,STATION_NO");
			return 0;
		}

		/*tpssm11_chg.Query("HEAT_NO");
		if (tpssm11_chg["SM_PLAN_NO"].ToString().Trim() != tpssm11["SM_PLAN_NO"].ToString().Trim())
		{
		Log::Trace("", __FUNCTION__, "信号触发钢种变更");
		inblockchgin.Tables[0].Rows.Add();
		inblockchgin.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11_chg["SM_PLAN_NO"].ToString().Trim();
		inblockchgin.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"].ToString().Trim();
		inblockchgin.Tables[0].Rows.Add();
		inblockchgin.Tables[0].Rows[1]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"].ToString().Trim();
		inblockchgin.Tables[0].Rows[1]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"].ToString().Trim();

		ret = f_pssm_chg_in(&inblockchgin, bcls_ret, conn);
		if (ret < 0)
		{
		throw CApplicationException(-1, s.msg, log.Location);
		}
		}*/

		// 根据设备状态获取12表根据start time real的是否为空判定怎么取数据
		tpssms1["EVENT_ID"] = event_id;
		tpssms1["STATUS_NAME"] = status_name;
		tpssms1["DEV_CODE"] = v_dev_code;
		Log::Trace("", __FUNCTION__, "事件[{0}]状态[{1}]设备[{2}]", event_id, status_name, v_dev_code);

		if (v_dev_code.Substring(0, 1) == "R" || v_dev_code.Substring(0, 1) == "S")
		{
			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
				" AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			cmd_count.Parameters.Set("treatment_count", treatment_count);
			dev_count = cmd_count.ExecuteScalar();

			tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			run_status = tpssms1["RUN_STATUS"];

			if (dev_count > 1)
			{
				if (run_status == "41")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "42")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4   ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "43")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "44")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL <> ' ' AND LEAVE_REAL_TIME = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					// 处理精炼不重要信号
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						"  START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
				}
			}
			else if (dev_count == 1)
			{
				if (run_status == "41")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND  " //TREATMENT_COUNTER = @treatment_count AND
						" START_TIME_REAL = ' '  AND  AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "42")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND  " //TREATMENT_COUNTER = @treatment_count AND
						" START_TIME_REAL = ' '  AND  AREA_ID = 4   ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "43")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND  "//TREATMENT_COUNTER = @treatment_count AND
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "44")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND  "//TREATMENT_COUNTER = @treatment_count AND
						" START_TIME_REAL <> ' ' AND END_TIME_REAL <> ' ' AND LEAVE_REAL_TIME = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					// 处理精炼不重要信号
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND  "//TREATMENT_COUNTER = @treatment_count AND
						"  START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				//throw CApplicationException(-1, s.msg, log.Location);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}

				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}

		}
		else if (v_dev_code.Substring(0, 1) == "V")
		{
			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
				" AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			cmd_count.Parameters.Set("treatment_count", treatment_count);
			dev_count = cmd_count.ExecuteScalar();

			tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			run_status = tpssms1["RUN_STATUS"];

			if (dev_count > 1)
			{
				if (run_status == "41")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "42")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4   ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "43")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "44")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL <> ' ' AND LEAVE_REAL_TIME = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					// 处理精炼不重要信号
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						"  START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
				}
			}
			else if (dev_count == 1)
			{
				if (run_status == "41")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "42")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND
						" START_TIME_REAL = ' '  AND  AREA_ID = 4   ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "43")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "44")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL <> ' ' AND LEAVE_REAL_TIME = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					// 处理精炼不重要信号
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND
						"  START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				//throw CApplicationException(-1, s.msg, log.Location);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}
		}
		else if (v_dev_code.Substring(0, 1) == "F")
		{
			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
				" AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			cmd_count.Parameters.Set("treatment_count", treatment_count);
			dev_count = cmd_count.ExecuteScalar();

			tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			run_status = tpssms1["RUN_STATUS"];

			if (dev_count > 1)
			{
				if (run_status == "41")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "42")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4   ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "43")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "44")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL <> ' ' AND LEAVE_REAL_TIME = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					// 处理精炼不重要信号
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						"  START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
				}
			}
			else if (dev_count == 1)
			{
				if (run_status == "41")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL = ' '  AND  AREA_ID = 4  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "42")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND
						" START_TIME_REAL <> ' '  AND END_TIME_REAL = ' ' AND  AREA_ID = 4   ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "43")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "44")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND 
						" START_TIME_REAL <> ' ' AND END_TIME_REAL <> ' ' AND LEAVE_REAL_TIME = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					// 处理精炼不重要信号
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "//TREATMENT_COUNTER = @treatment_count AND
						"  START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' AND  AREA_ID = 4 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				//throw CApplicationException(-1, s.msg, log.Location);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}
		}
		else if (v_dev_code.Substring(0, 1) == "C")
		{
			// 写入事件的钢种在TPSSM11表
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm11["ST_NO_1"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
			tpssm11.Update("ST_NO_1", "SM_PLAN_NO,FACTORY_DIV");

			if (event_id == "8")
			{
				tpssms1.Query("EVENT_ID,DEV_CODE");
			}
			else tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			run_status = tpssms1["RUN_STATUS"];
			Log::Trace("", __FUNCTION__, "L3信号[{0}]L3状态[{1}]", tpssms1["RUN_SIGNAL"].ToString(), run_status);

			if (event_id == "1")
			{
				run_status = "51";
				sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE = @dev_code AND "
					"  FACTORY_DIV = @factory_div  AND  AREA_ID = 5  ORDER BY CHARGE_NO ASC  ";//START_TIME_REAL = ' '
			}
			else if (event_id == "3")
			{
				run_status = "52";
				sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE = @dev_code AND "
					"  FACTORY_DIV = @factory_div  AND AREA_ID = 5  ORDER BY CHARGE_NO ASC  ";//START_TIME_REAL = ' '
			}
			else if (event_id == "8")
			{
				run_status = "53";
				sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE = @dev_code AND "
					"  FACTORY_DIV = @factory_div  AND  AREA_ID = 5   ORDER BY CHARGE_NO ASC  ";//START_TIME_REAL <> ' '
			}
			else if (event_id == "6" &&status_name == "CUT_F")
			{
				run_status = "54";
				sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE = @dev_code AND "
					"  FACTORY_DIV = @factory_div  AND  AREA_ID = 5   ORDER BY CHARGE_NO ASC  ";//START_TIME_REAL <> ' '
				tpssm11["CUT_FIN_FLAG"] = "1";
				tpssm11.Update("CUT_FIN_FLAG", "SM_PLAN_NO");
			}
			else
			{
				// 处理连铸不重要信号
				sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE = @dev_code AND "
					"  FACTORY_DIV = @factory_div  AND  AREA_ID = 5 ORDER BY CHARGE_NO ASC  ";//AND  START_TIME_REAL <> ' '
			}
		}
		else if (v_dev_code.Substring(0, 1) == "A")
		{
			tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			run_status = tpssms1["RUN_STATUS"];

			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND "
				" AREA_ID = 3  ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			dev_count = cmd_count.ExecuteScalar();

			if (dev_count == 1)
			{
				sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  AREA_ID = 3 AND "
					"  FACTORY_DIV = @factory_div   ORDER BY CHARGE_NO ASC  ";
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]", arguments, 3); //格式化字符串
				//throw CApplicationException(-1, s.msg, log.Location);
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}
		}

		else if (v_dev_code.Substring(0, 1) == "B")
		{
			// 需要判断是预溶液还是脱c
			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count "
				" ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			cmd_count.Parameters.Set("treatment_count", treatment_count);
			dev_count = cmd_count.ExecuteScalar();


			if (dev_count > 1)
			{
				if (event_id == "1" || event_id == "3")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND START_TIME_REAL = ' '  ORDER BY CHARGE_NO ASC  ";
				}
				else if (event_id == "4")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div   AND START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND START_TIME_REAL <> ' ' AND END_TIME_REAL = ' '  ORDER BY CHARGE_NO ASC  ";
				}
			}
			else if (dev_count == 1)
			{
				if (event_id == "1" || event_id == "3")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  ORDER BY CHARGE_NO ASC  ";
				}
				else if (event_id == "4")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  ORDER BY CHARGE_NO ASC  ";
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				//throw CApplicationException(-1, s.msg, log.Location);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");


				return 0;
			}

		}
		else if (v_dev_code.Substring(0, 1) == "E")
		{
			// 需要判断是预溶液还是脱c
			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count "
				" ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			cmd_count.Parameters.Set("treatment_count", treatment_count);
			dev_count = cmd_count.ExecuteScalar();


			if (dev_count > 1)
			{
				if (event_id == "1" || event_id == "3")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND START_TIME_REAL = ' '  ORDER BY CHARGE_NO ASC  ";
				}
				else if (event_id == "4")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div   AND START_TIME_REAL <> ' ' AND END_TIME_REAL = ' ' ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND START_TIME_REAL <> ' ' AND END_TIME_REAL = ' '  ORDER BY CHARGE_NO ASC  ";
				}
			}
			else if (dev_count == 1)
			{
				if (event_id == "1" || event_id == "3")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  ORDER BY CHARGE_NO ASC  ";
				}
				else if (event_id == "4")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  ORDER BY CHARGE_NO DESC  ";
				}
				else
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  ORDER BY CHARGE_NO ASC  ";
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				//throw CApplicationException(-1, s.msg, log.Location);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}

		}
		else if (v_dev_code.Substring(0, 1) == "Z")
		{

			sqlstr_count = " SELECT count(*) FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and  DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
				" AREA_ID = 2  ORDER BY CHARGE_NO ASC  ";
			cmd_count.SetCommandText(sqlstr_count);
			cmd_count.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_count.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_count.Parameters.Set("dev_code", v_dev_code);
			cmd_count.Parameters.Set("treatment_count", treatment_count);
			dev_count = cmd_count.ExecuteScalar();

			tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
			run_status = tpssms1["RUN_STATUS"];

			if (dev_count > 1)
			{
				if (run_status == "21")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and   DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND  START_TIME_REAL = ' ' AND  AREA_ID = 2 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "24")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and   DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND START_TIME_REAL = ' '  AND  AREA_ID = 2 ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "27")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and   DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND  START_TIME_REAL <> ' '  AND END_TIME_REAL = ' ' AND  AREA_ID = 2 ORDER BY CHARGE_NO DESC  ";
				}
			}
			else if (dev_count == 1)
			{
				if (run_status == "21")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and   DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND  AREA_ID = 2  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "24")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and   DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND AREA_ID = 2  ORDER BY CHARGE_NO ASC  ";
				}
				else if (run_status == "27")
				{
					sqlstr_tpssm12 = " SELECT * FROM TPSSM12 WHERE SM_PLAN_NO = @sm_plan_no  and   DEV_CODE LIKE SUBSTR(@dev_code,1,1)||'%' AND TREATMENT_COUNTER = @treatment_count AND "
						"  FACTORY_DIV = @factory_div  AND  AREA_ID = 2  ORDER BY CHARGE_NO DESC  ";
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				CMessageFormat::Format(s.msg, "计划[{0}]未安排路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				//throw CApplicationException(-1, s.msg, log.Location);

				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = v_heat_no;
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}
		}
		if (sqlstr_tpssm12.Trim() != "")
		{

			cmd_tpssm12_inq.SetCommandText(sqlstr_tpssm12);
			Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_dev_code=[{0}]", sqlstr_tpssm12);

			cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("dev_code", v_dev_code);
			cmd_tpssm12_inq.Parameters.Set("treatment_count", treatment_count);
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
			}
			else
			{
				Log::Trace("", __FUNCTION__, "计划[{0}]已处理路径[{1}]上的设备[{2}]设备处理次数[{3}]", tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count);
				CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), v_dev_code.Substring(0, 1), v_dev_code, treatment_count }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]已处理路径[{1}]上的设备[{2}]设备处理次数[{3}]", arguments, 4); //格式化字符串
				//throw CApplicationException(-1, s.msg, log.Location);
				bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
				ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
				//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
				tpssms1["EVENT_ID"] = event_id;
				tpssms1["STATUS_NAME"] = status_name;
				tpssms1["DEV_CODE"] = v_dev_code;
				//tpssms1.Print();
				//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
				// 传入计划号无法查询到直接写入33和34表
				tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
				tpssm34["DEV_CODE"] = v_dev_code;
				tpssm34["EVENT_ID"] = event_id;
				tpssm34["STATUS_NAME"] = status_name;
				tpssm34["SIMUL_FLAG"] = "0";
				tpssm34["REC_CREATOR"] = s.userid;
				tpssm34["REC_CREATE_TIME"] = sys_time;
				tpssm34["REC_REVISOR"] = s.userid;
				tpssm34["START_TIME"] = v_proc_time;
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
					" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
					" AND PONO                = @tpssm34.PONO ";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
					Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
				}
				else
				{
					tpssm34["RUN_SEQ"] = 0;
				}
				tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
				tpssm34["PRACT_PONO"] = tpssm34["PONO"];
				//取HEAT_NO
				tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
				tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
				tpssm34["SM_PLAN_NOL2"] = planl2;
				sqlstr = "tpssm34.Insert()";
				tpssm34.TrimOrBlank();
				tpssm34.Insert();

				//33表  update
				tpssm33["PONO"] = tpssm11["PONO"];
				tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
				dummy = tpssm33.QueryCount("PONO");
				if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
				{
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy == 1)
				{
					tpssm33.Query("PONO");
					tpssm33["TAP_END_TIME"] = v_proc_time;
				}
				else if (dummy > 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
					tpssm33.Query("PONO,VIEW_POS");
				}

				//更新作业计划监控表的部分字段内容(接口表送入数据)
				tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
				tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
				tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
				tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
				//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
				tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
				tpssm33["START_TIME"] = v_proc_time;				//处理时刻
				tpssm33["ST_NO"] = cs_st_no;
				tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
				tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm33["EVENT_ID"] = event_id;
				tpssm33["STATUS_NAME"] = status_name;
				if (v_dev_code.SubstringNE(0, 1) == "V")
				{
					if (status_name == "VACSTART"&&event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else if (v_dev_code.SubstringNE(0, 1) == "F")
				{
					if (status_name == "StirST"&&event_id == "6")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else  if (v_dev_code.SubstringNE(0, 1) == "C")
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "8")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
				}
				else
				{
					if (event_id == "3")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;
					}
					if (event_id == "4")
					{
						tpssm33["END_TIME_REAL"] = v_proc_time;
					}
					if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
					{
						tpssm33["START_TIME_REAL"] = v_proc_time;

					}
				}
				if (dummy >= 1)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
						sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
						break;
					}
					cmd_exe.SetCommandText(sqlstr);
					cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
					cmd_exe.ExecuteNonQuery();
				}

				tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
				//tpssm33.Print();
				tpssm33.TrimOrBlank();
				tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
					, "STATION_ID,STATION_NO");

				return 0;
			}
			cmd_tpssm12_inq.Close();
			//tpssm12.Print();
			if (v_dev_code.Substring(0, 1) == "E" || v_dev_code.Substring(0, 1) == "B")
			{
				sqlstr = " SELECT *     FROM TPSSMS1  WHERE DEV_CODE = @v_dev_code    AND FACTORY_DIV = @tpssm11.FACTORY_DIV    AND  RUN_STATUS LIKE @run_status   AND STATUS_NAME = @status_name AND EVENT_ID = @event_id  ORDER BY RUN_SIGNAL ";


				cmd_tpssms1_inq.SetCommandText(sqlstr);
				cmd_tpssms1_inq.Parameters.Set("v_dev_code", v_dev_code);
				cmd_tpssms1_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssms1_inq.Parameters.Set("run_status", tpssm12["AREA_ID"].ToString() + "%");
				cmd_tpssms1_inq.Parameters.Set("status_name", status_name);
				cmd_tpssms1_inq.Parameters.Set("event_id", event_id);
				cmd_tpssms1_inq.ExecuteReader();

				if (cmd_tpssms1_inq.Read())
				{
					cmd_tpssms1_inq.Fetch(tpssms1);
				}
				else
				{
					Log::Trace("", __FUNCTION__, "未定义信号[{0}],[{1}],[{2}]", event_id, status_name, v_dev_code);
				}
				cmd_tpssms1_inq.Close();
			}
		}
		else
		{
			Log::Trace("", __FUNCTION__, "未定义信号[{0}],[{1}],[{2}]", event_id, status_name, v_dev_code);
		}

		//查询处理号，区域标识
		v_charge_no = tpssm12["CHARGE_NO"];
		Log::Trace("", __FUNCTION__, "pssm21_ot_run>sqlstr_tpssm12=[{0}]", sqlstr_tpssm12);
		v_area_id = tpssm12["AREA_ID"];

		if (v_area_id < 4)
		{
			//Log::Trace("", __FUNCTION__, "aaaaaaaaaaa");
			v_proc_no = bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"].ToString();
		}
		else
		{
			if ((v_area_id == 4 || v_area_id == 5) && tpssm12["PROC_NO"].ToString().Trim() == "")
			{
				//Log::Trace("", __FUNCTION__, "bbbbb");
				sqlstr = CString(
					" SELECT CURR_PROC_NO FROM TPSSM25 "
					"  WHERE FACTORY_DIV   = @tpssmd1.FACTORY_DIV "
					"    AND STATION_ID   = @tpssmd1.STATION_ID "
					"    AND STATION_NO   = @tpssmd1.STATION_NO "
					);
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_ID", v_dev_code.Substring(0, 1));
				cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_NO", v_dev_code.Substring(1, 1));
				cmd_tpssm25_inq.ExecuteReader();
				if (cmd_tpssm25_inq.Read())
				{
					tpssm25["CURR_PROC_NO"] = cmd_tpssm25_inq.GetString(1);
					iproc_no = iproc_no.Parse(tpssm25["CURR_PROC_NO"].ToString().Substring(3));
					iproc_no = (iproc_no.ToInt32() + 1) % 100000; // 5位流水
					v_proc_no = tpssm25["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)v_dev_code.Trim().SubstringNE(0, 1), (const char*)v_dev_code.Trim().SubstringNE(1, 1), (const char*)datetime.Trim().SubstringNE(3, 1), iproc_no.ToInt32());
					Log::Info("", __FUNCTION__, "生成精炼连铸处理号v_proc_no = [{0}]", v_proc_no);
				}
				cmd_tpssm25_inq.Close();
			}
			else v_proc_no = tpssm12["PROC_NO"].ToString().Trim();
			//Log::Trace("", __FUNCTION__, "cccccccccc");
		}

		// 根据电文获取到的设备号拿到对应的信号，并且将对应设备的信息拿到

		//打印传入参数
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>HEAT_NO=[{0}]", tpssm11["HEAT_NO"].ToString());
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>PONO=[{0}]", tpssm11["PONO"].ToString());
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_dev_code=[{0}]", v_dev_code);
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_proc_time=[{0}]", v_proc_time);
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_charge_no=[{0}]", v_charge_no);
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_flag=[{0}]", v_flag);


		//计算精炼重数
		//Log::Trace("", __FUNCTION__, "eeeeeeeeeee");
		if (v_area_id != 4 && v_area_id != 2)
		{
			v_srp_seq = 0;
		}
		else if (v_area_id == 4)
		{
#if 1
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = " SELECT MIN(CHARGE_NO) "
					"   FROM TPSSM12 "
					"  WHERE  SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) "
					"    AND AREA_ID = 4 ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();

			if (cmd_tpssm12_inq.Read())
			{
				v_charge_no_min = cmd_tpssm12_inq.GetDecimal(1);
			}

			v_srp_seq = v_charge_no - v_charge_no_min + 1;//精炼重数
#endif


		}
		else if (v_area_id == 2)
		{
			v_srp_seq = tpssm12["CHARGE_NO"];
		}
		v_run_signal = tpssms1["RUN_SIGNAL"];
		////Log::Info("", __FUNCTION__, "v_run_signal = [{0}]", v_run_signal);
		mx_flag = tpssms1["COMPANY_NAME"];
		cs_time_column = tpssms1["COMPANY_CODE"];


		//校验浇次顺序
		//Log::Trace("", __FUNCTION__, "dddd");
		if (v_run_signal.Trim() != "")
		{
			if (v_run_signal.Trim().Substring(0, 1) == "5" && v_run_signal.Trim().Substring(2, 1) == "2")
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
				if (countT >= 2)
				{
					Log::Trace("", __FUNCTION__, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前有大于等于两个开浇炉，判定为错误信号，记录履历", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString());
					CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前有大于等于两个开浇炉，判定为错误信号，记录履历", arguments, 3); //格式化字符串
					bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
					ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
					//throw CApplicationException(-1, s.msg, log.Location);

					//Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
					tpssms1["EVENT_ID"] = event_id;
					tpssms1["STATUS_NAME"] = status_name;
					tpssms1["DEV_CODE"] = v_dev_code;
					//tpssms1.Print();
					//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
					// 传入计划号无法查询到直接写入33和34表
					tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
					tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
					tpssm34["DEV_CODE"] = v_dev_code;
					tpssm34["EVENT_ID"] = event_id;
					tpssm34["STATUS_NAME"] = status_name;
					tpssm34["SIMUL_FLAG"] = "0";
					tpssm34["REC_CREATOR"] = s.userid;
					tpssm34["REC_CREATE_TIME"] = sys_time;
					tpssm34["REC_REVISOR"] = s.userid;
					tpssm34["START_TIME"] = v_proc_time;
					sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
						" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
						" AND PONO                = @tpssm34.PONO ";

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
					cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
						Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
					}
					else
					{
						tpssm34["RUN_SEQ"] = 0;
					}
					tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
					tpssm34["PRACT_PONO"] = tpssm34["PONO"];
					//取HEAT_NO
					tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
					tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
					tpssm34["SM_PLAN_NOL2"] = planl2;
					sqlstr = "tpssm34.Insert()";
					tpssm34.TrimOrBlank();
					tpssm34.Insert();

					//33表  update
					tpssm33["PONO"] = tpssm11["PONO"];
					tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
					dummy = tpssm33.QueryCount("PONO");
					if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
					{
						tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

						tpssm33["TAP_END_TIME"] = v_proc_time;
					}
					else if (dummy == 1)
					{
						tpssm33.Query("PONO");
						tpssm33["TAP_END_TIME"] = v_proc_time;
					}
					else if (dummy > 1)
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
							break;
						}

						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
						tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
						tpssm33.Query("PONO,VIEW_POS");
					}

					//更新作业计划监控表的部分字段内容(接口表送入数据)
					tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
					tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
					tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
					tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
					//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
					tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
					tpssm33["START_TIME"] = v_proc_time;				//处理时刻
					tpssm33["ST_NO"] = cs_st_no;
					tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
					tpssm33["EVENT_ID"] = event_id;
					tpssm33["STATUS_NAME"] = status_name;
					if (v_dev_code.SubstringNE(0, 1) == "V")
					{
						if (status_name == "VACSTART"&&event_id == "3")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "4")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
					}
					else if (v_dev_code.SubstringNE(0, 1) == "F")
					{
						if (status_name == "StirST"&&event_id == "6")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "4")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
					}
					else  if (v_dev_code.SubstringNE(0, 1) == "C")
					{
						if (event_id == "3")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "8")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
					}
					else
					{
						if (event_id == "3")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "4")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
						if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;

						}
					}
					if (dummy >= 1)
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
							sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
							break;
						}
						cmd_exe.SetCommandText(sqlstr);
						cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
						cmd_exe.ExecuteNonQuery();
					}

					tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
					tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
					tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
					tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
					//tpssm33.Print();
					tpssm33.TrimOrBlank();
					tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
						, "STATION_ID,STATION_NO");

					return 0;
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
						Log::Trace("", __FUNCTION__, "计划[{0}]安排在[{1}]浇次上的第[{2}]个，之前有本浇次开浇炉，需要调整顺序0912UPD", tpssm11["SM_PLAN_NOL2"].ToString(), tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToString());

						sqlstr = " UPDATE TPSSM11 SET RESTRAND_FLG = ' ' WHERE CAST_NO = @cast_no AND RESTRAND_FLG = 'T' ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
						cmd_inq.ExecuteNonQuery();

						sqlstr = " UPDATE TPSSM11 SET RESTRAND_FLG = 'T',TD_CHG_FLG=0, CAST_DIV_NO = 1 WHERE CAST_NO = @cast_no AND SM_PLAN_NO = @sm_plan_no ";
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
						bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
						ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
						throw CApplicationException(-1, s.msg, log.Location);

						tpssms1["EVENT_ID"] = event_id;
						tpssms1["STATUS_NAME"] = status_name;
						tpssms1["DEV_CODE"] = v_dev_code;
						//tpssms1.Print();
						//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
						// 传入计划号无法查询到直接写入33和34表
						tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
						tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
						tpssm34["DEV_CODE"] = v_dev_code;
						tpssm34["EVENT_ID"] = event_id;
						tpssm34["STATUS_NAME"] = status_name;
						tpssm34["SIMUL_FLAG"] = "0";
						tpssm34["REC_CREATOR"] = s.userid;
						tpssm34["REC_CREATE_TIME"] = sys_time;
						tpssm34["REC_REVISOR"] = s.userid;
						tpssm34["START_TIME"] = v_proc_time;
						sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
							" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
							" AND PONO                = @tpssm34.PONO ";

						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
						cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
							Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
						}
						else
						{
							tpssm34["RUN_SEQ"] = 0;
						}
						tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
						tpssm34["PRACT_PONO"] = tpssm34["PONO"];
						//取HEAT_NO
						tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
						tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
						tpssm34["SM_PLAN_NOL2"] = planl2;
						sqlstr = "tpssm34.Insert()";
						tpssm34.TrimOrBlank();
						tpssm34.Insert();

						//33表  update
						tpssm33["PONO"] = tpssm11["PONO"];
						tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
						dummy = tpssm33.QueryCount("PONO");
						if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
						{
							tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

							tpssm33["TAP_END_TIME"] = v_proc_time;
						}
						else if (dummy == 1)
						{
							tpssm33.Query("PONO");
							tpssm33["TAP_END_TIME"] = v_proc_time;
						}
						else if (dummy > 1)
						{
							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:	        // MS SQL Server数据库
							case DB_KIND_ORACLE:	        // Oracle 数据库
							default: // 所有数据库适用，通用SQL语句
								sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
								break;
							}

							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
							tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
							tpssm33.Query("PONO,VIEW_POS");
						}

						//更新作业计划监控表的部分字段内容(接口表送入数据)
						tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
						tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
						tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
						tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
						//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
						tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
						tpssm33["START_TIME"] = v_proc_time;				//处理时刻
						tpssm33["ST_NO"] = cs_st_no;
						tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
						tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
						tpssm33["EVENT_ID"] = event_id;
						tpssm33["STATUS_NAME"] = status_name;
						if (v_dev_code.SubstringNE(0, 1) == "V")
						{
							if (status_name == "VACSTART"&&event_id == "3")
							{
								tpssm33["START_TIME_REAL"] = v_proc_time;
							}
							if (event_id == "4")
							{
								tpssm33["END_TIME_REAL"] = v_proc_time;
							}
						}
						else if (v_dev_code.SubstringNE(0, 1) == "F")
						{
							if (status_name == "StirST"&&event_id == "6")
							{
								tpssm33["START_TIME_REAL"] = v_proc_time;
							}
							if (event_id == "4")
							{
								tpssm33["END_TIME_REAL"] = v_proc_time;
							}
						}
						else  if (v_dev_code.SubstringNE(0, 1) == "C")
						{
							if (event_id == "3")
							{
								tpssm33["START_TIME_REAL"] = v_proc_time;
							}
							if (event_id == "8")
							{
								tpssm33["END_TIME_REAL"] = v_proc_time;
							}
						}
						else
						{
							if (event_id == "3")
							{
								tpssm33["START_TIME_REAL"] = v_proc_time;
							}
							if (event_id == "4")
							{
								tpssm33["END_TIME_REAL"] = v_proc_time;
							}
							if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
							{
								tpssm33["START_TIME_REAL"] = v_proc_time;

							}
						}
						if (dummy >= 1)
						{
							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:	        // MS SQL Server数据库
							case DB_KIND_ORACLE:	        // Oracle 数据库
							default: // 所有数据库适用，通用SQL语句
								sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
								sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
								break;
							}
							cmd_exe.SetCommandText(sqlstr);
							cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
							cmd_exe.ExecuteNonQuery();
						}

						tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
						tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
						tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
						tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
						//tpssm33.Print();
						tpssm33.TrimOrBlank();
						tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
							, "STATION_ID,STATION_NO");

						return 0;
					}
				}

			}
		}

		//校验异常信号
		if (v_run_signal.Trim() != "")
		{
			if (v_run_signal.Trim().Substring(0, 1) == "2" || v_run_signal.Trim().Substring(0, 1) == "3" || v_run_signal.Trim().Substring(0, 1) == "4")
			{
				if (tpssm11["RUN_STATUS"].ToString() >= "50" && tpssm11["HEAT_NO"].ToString() != v_heat_no)
				{
					Log::Trace("", __FUNCTION__, "计划[{0}]在开浇后，被非连铸设备接管并且炉号不一致，判定为错误信号", tpssm11["SM_PLAN_NOL2"].ToString());
					CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划[{0}]在开浇后，被非连铸设备接管并且炉号不一致，判定为错误信号", arguments, 1); //格式化字符串
					bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
					ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
					throw CApplicationException(-1, s.msg, log.Location);

					tpssms1["EVENT_ID"] = event_id;
					tpssms1["STATUS_NAME"] = status_name;
					tpssms1["DEV_CODE"] = v_dev_code;
					//tpssms1.Print();
					//tpssms1.Query("EVENT_ID,STATUS_NAME,DEV_CODE");
					// 传入计划号无法查询到直接写入33和34表
					tpssm34["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
					tpssm34["PONO"] = tpssm11["PONO"].ToString().TrimOrBlank();
					tpssm34["DEV_CODE"] = v_dev_code;
					tpssm34["EVENT_ID"] = event_id;
					tpssm34["STATUS_NAME"] = status_name;
					tpssm34["SIMUL_FLAG"] = "0";
					tpssm34["REC_CREATOR"] = s.userid;
					tpssm34["REC_CREATE_TIME"] = sys_time;
					tpssm34["REC_REVISOR"] = s.userid;
					tpssm34["START_TIME"] = v_proc_time;
					sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
						" WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
						" AND PONO                = @tpssm34.PONO ";

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
					cmd_inq.Parameters.Set("tpssm34.PONO", tpssm34["PONO"].ToString());
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
						Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}],PONO=[{1}]", tpssm34["RUN_SEQ"].ToDecimal(), tpssm34["PONO"].ToString());
					}
					else
					{
						tpssm34["RUN_SEQ"] = 0;
					}
					tpssm34["RUN_SEQ"] = tpssm34["RUN_SEQ"].ToDecimal() + 1;
					tpssm34["PRACT_PONO"] = tpssm34["PONO"];
					//取HEAT_NO
					tpssm34["HEAT_NO"] = tpssm11["HEAT_NO"].ToString().TrimOrBlank();
					tpssm34["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
					tpssm34["SM_PLAN_NOL2"] = planl2;
					sqlstr = "tpssm34.Insert()";
					tpssm34.TrimOrBlank();
					tpssm34.Insert();

					//33表  update
					tpssm33["PONO"] = tpssm11["PONO"];
					tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
					dummy = tpssm33.QueryCount("PONO");
					if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
					{
						tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];

						tpssm33["TAP_END_TIME"] = v_proc_time;
					}
					else if (dummy == 1)
					{
						tpssm33.Query("PONO");
						tpssm33["TAP_END_TIME"] = v_proc_time;
					}
					else if (dummy > 1)
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = "SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
							break;
						}

						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
						tpssm33["VIEW_POS"] = cmd_inq.ExecuteScalar();
						tpssm33.Query("PONO,VIEW_POS");
					}

					//更新作业计划监控表的部分字段内容(接口表送入数据)
					tpssm33["STATION_ID"] = v_dev_code.SubstringNE(0, 1);		//PK1
					tpssm33["STATION_NO"] = v_dev_code.SubstringNE(1, 1);		//PK2
					tpssm33["PONO"] = tpssm11["SM_PLAN_NO"];				//PI号
					tpssm33["CURR_PROC_NO"] = tpssm11["SM_PLAN_NO"];					//当前处理号
					//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
					tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
					tpssm33["START_TIME"] = v_proc_time;				//处理时刻
					tpssm33["ST_NO"] = cs_st_no;
					tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
					tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
					tpssm33["EVENT_ID"] = event_id;
					tpssm33["STATUS_NAME"] = status_name;
					if (v_dev_code.SubstringNE(0, 1) == "V")
					{
						if (status_name == "VACSTART"&&event_id == "3")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "4")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
					}
					else if (v_dev_code.SubstringNE(0, 1) == "F")
					{
						if (status_name == "StirST"&&event_id == "6")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "4")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
					}
					else  if (v_dev_code.SubstringNE(0, 1) == "C")
					{
						if (event_id == "3")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "8")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
					}
					else
					{
						if (event_id == "3")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;
						}
						if (event_id == "4")
						{
							tpssm33["END_TIME_REAL"] = v_proc_time;
						}
						if (status_name == "BLOW"&&event_id == "6"&&v_dev_code.SubstringNE(0, 1) == "B")
						{
							tpssm33["START_TIME_REAL"] = v_proc_time;

						}
					}
					if (dummy >= 1)
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = "UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
							sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
							break;
						}
						cmd_exe.SetCommandText(sqlstr);
						cmd_exe.Parameters.Set("tpssm33.PONO", tpssm33["PONO"].ToString());
						cmd_exe.ExecuteNonQuery();
					}

					tpssm33["CAST_NO"] = tpssm11["CAST_NO"];
					tpssm33["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
					tpssm33["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
					tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];
					//tpssm33.Print();
					tpssm33.TrimOrBlank();
					tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
						, "STATION_ID,STATION_NO");

					return 0;
				}
			}
		}

		//3、查询run_signal

		Log::Info("", __FUNCTION__, "模拟运转信号v_run_signal = [{0}]", v_run_signal);

		inBlock.Tables[0].Rows.Add();
		inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		inBlock.Tables[0].Rows[0]["PROC_NO"] = v_proc_no;
		inBlock.Tables[0].Rows[0]["PROC_TIME"] = v_proc_time;
		inBlock.Tables[0].Rows[0]["SIMUL_FLAG"] = "0"; //甘特图模拟

		inBlock.Tables[1].Rows.Add();
		inBlock.Tables[1].Rows[0]["PONO"] = tpssm11["PONO"];

		inBlock.Tables[2].Rows.Add();
		inBlock.Tables[2].Rows[0]["RUN_SIGNAL"] = v_run_signal;
		inBlock.Tables[2].Rows[0]["AREA_ID"] = v_area_id;
		inBlock.Tables[2].Rows[0]["SRP_SEQ"] = v_srp_seq;
		inBlock.Tables[2].Rows[0]["CHARGE_NO_2"] = v_srp_seq;
		inBlock.Tables[2].Rows[0]["HEAT_NO"] = v_heat_no;
		inBlock.Tables[2].Rows[0]["LADLE_NO"] = ladle_no;
		inBlock.Tables[2].Rows[0]["TREATMENT_COUNTER"] = treatment_count;

		Log::Trace("", __FUNCTION__, "ffffffffffffff");

	
		tpssms1["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssms1["RUN_SIGNAL"] = v_run_signal;
		dummy = tpssms1.QueryCount("FACTORY_DIV,RUN_SIGNAL");
		if (dummy == 0)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000149")/*运转信号[{0}]不存在，请联系维护人员。*/, arguments, 1); //格式化字符串
			//throw CApplicationException(-1, s.msg, log.Location);
			Log::Info("", __FUNCTION__, "运转信号[{0}]不存在，请联系维护人员", tpssms1["RUN_SIGNAL"].ToString());
			return 0;
		}
		//信号模拟
		ret = f_pssm_run_proc_n(&inBlock, bcls_ret, conn);
		if (ret < 0)
		{
			//tpssm99["VALID_FLAG"] = "0";
			//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
			//////Log::Trace("", __FUNCTION__, "记录失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());

			//tpabort(0);
			//tpbegin(0, 0);
			////记录编入计划成功的履历
			//ret = 0;
			//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			//tpcommit(0);
			//tpbegin(0, 0);

			func_back_mess = s.msg;
			Log::Trace("", __FUNCTION__, "func_back_mess = [{0}]", func_back_mess);

			if (func_back_mess == "数据库处理出错，sqlcode=[60]。请稍后再试或联系系统维护人员。" || func_back_mess == "数据库处理出错，sqlcode=[2049]。请稍后再试或联系系统维护人员。")
			{
				throw CApplicationException(-9, s.msg, log.Location);
			}
			else
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		if (cs_time_column.Trim() != "")
		{
			tpssm12[cs_time_column] = v_proc_time;
			tpssm12.Update(cs_time_column, "FACTORY_DIV,SM_PLAN_NO,SPLIT_INDICATION,CHARGE_NO");
		}
		//20250928  lizhen 连铸时更新质量表的炉号信息，原先是5*2,5*3状态，现在改为5**
		if (v_run_signal.Trim().Substring(0, 1) == "5")
		{

			inblockqm.Tables[0].Rows.Add();

			inblockqm.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			inblockqm.Tables[0].Rows[0]["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"];
			inblockqm.Tables[0].Rows[0]["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
			inblockqm.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];

			ret = f_qmts_23_init(&inblockqm, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			ret = f_qmts_23_upd(&inblockqm, bcls_ret, conn);
			/*if (ret < 0)
			{
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
		}

		if (v_run_signal.Trim().Substring(0, 1) == "5" && v_run_signal.Trim().Substring(2, 1) == "4")
		{
			tpssm11["CUT_FIN_FLAG"] = "1";
			tpssm11.Update("CUT_FIN_FLAG", "SM_PLAN_NOL2");
		}
		Log::Trace("", __FUNCTION__, "f_pssm12z_combine = [{0}],event_id=[{1}]", v_dev_code, event_id);

		if (v_dev_code.Trim().SubstringNE(0, 1) == "A" && event_id.Trim() == "3")
		{
			Log::Trace("", __FUNCTION__, "开始f_pssm12z_combine ladle_no= [{0}],v_heat_no=[{1}],SM_PLAN_NO=[{2}]", ladle_no, v_heat_no, tpssm11["SM_PLAN_NO"].ToString());
			ret = f_pssm12z_combine(ladle_no, v_heat_no, tpssm11["SM_PLAN_NO"].ToString(), conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		//20240515 卫迪提出逻辑
		if (v_run_signal.Trim().Substring(0, 1) == "5" && v_run_signal.Trim().Substring(2, 1) == "G")
		{
			inblockdel.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			ret = f_plan_delete_snd2(&inblockdel, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}


		sqlstr = " SELECT CODE_DESC_1_CONTENT FROM TEP0002  "
			"	WHERE CODE_CLASS = 'PS302N' "
			"	  AND CODE = 'A' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			cal_flag = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
		if (cal_flag == "1")
		{
			if ((v_run_signal.Trim().Substring(0, 1) == "5" && v_run_signal.Trim().Substring(2, 1) == "2") || (v_run_signal.Trim().Substring(0, 1) == "5" && v_run_signal.Trim().Substring(2, 1) == "3"))
			{
				sqlstr = "SELECT * FROM TPSSM11 WHERE FACTORY_DIV=@v_factory_div AND PONO_STATUS<83 ";
				sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
				cmd_inq.ExecuteQuery(tb_tpssm11.Tables[0]);
				Log::Trace("", __FUNCTION__, "1=[{0}]", tb_tpssm11.Tables[0].Rows.get_Count());
				//校验可编计划数如果大于0才需要优化
				if (tb_tpssm11.Tables[0].Rows.get_Count() > 0)
				{
					ret = f_pssm_call_tps_n(v_factory_div, mode, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				cmd_inq.Close();
			}
		}


		///套娃
		tpssm12_check["CHARGE_NO"] = v_charge_no;
		tpssm12_check["SM_PLAN_NOL2"] = planl2;
		tpssm12_check.Query("SM_PLAN_NOL2,CHARGE_NO");
		if (tpssm12_check["DEV_CODE"].ToString() != v_dev_code)
		{
			Log::Trace("", __FUNCTION__, "计划更新设备[{0}]与电文接收设备[{1}]不符，进行套娃", tpssm12_check["DEV_CODE"].ToString(), v_dev_code);
			sleep(5);

			ret = f_pssm_run_proc_n(&inBlock, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		bcls_rec->Tables[0].Rows[0]["NOTE"] = s.msg;
		ret = f_pssmss_insert(bcls_rec, bcls_ret, conn);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚

		/*func_back_mess = s.msg;
		Log::Trace("", __FUNCTION__, "func_back_mess = [{0}]", func_back_mess);

		if (func_back_mess == "数据库处理出错，sqlcode=[60]。请稍后再试或联系系统维护人员。" || func_back_mess == "数据库处理出错，sqlcode=[2049]。请稍后再试或联系系统维护人员。")
		{
		throw CApplicationException(-9, s.msg, log.Location);
		}
		else
		{
		throw CApplicationException(-1, s.msg, log.Location);
		}*/
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
	cmd_tpssm12_inq.Close();
	cmd_tpssms1_inq.Close();
	return doFlag;

}
