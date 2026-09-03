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

//程序用头文件


void SendMessage(const std::string &module, const std::string &topic, const std::string& msg);//消息推送
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);
int f_pssm_run_proc_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
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
BM2F_ENTERACE(pssm18_ot_run)
//-EP_SYSTEM_HEAD_END
int f_pssm18_ot_run(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	
	//程序用变量
	int doFlag = 0, ret = 0;
	CString	v_proc_time = "", v_proc_no = "", v_dev_code = "", v_run_signal = "", proc_no = "", sm_plan_no = "", v_check_flag = "";
	CDecimal  v_charge_no = 0,v_charge_no_min=0,v_area_id = 0;
	CDecimal v_flag = 0;//2-开始，3-结束
	CDecimal v_srp_seq = 0;
	CDecimal treatment_count = 0;
	CDecimal    iproc_no = 0;
	CString sqlstr = "";
	CString   simul_flag = "1";              /* 模拟标记: 3- 模拟*/
	CString  datetime, datetime1;
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm99("TPSSM99");//履历
	CModel tpssm25("TPSSM25");
	EIClass inblk;
	EIClass inBlock;
	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssms1_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);

	inblk.Tables[0].set_TableName("PLAN");  //
	inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	inblk.Tables[0].Rows.Add();

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		datetime1 = CDateTime::Now().AddDays(-3).ToString("yyyyMMddHHmmss");
		//获取传入参数
		tpssm11["FACTORY_DIV"] =  bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NO"];
		tpssm11["PONO"] =  bcls_rec->Tables[0].Rows[0]["PONO"];
		v_dev_code =  bcls_rec->Tables[0].Rows[0]["DEV_CODE"];
		v_proc_time =  bcls_rec->Tables[0].Rows[0]["PROC_TIME"];
		v_charge_no  =  bcls_rec->Tables[0].Rows[0]["CHARGE_NO"];
		v_flag = bcls_rec->Tables[0].Rows[0]["PROC_FLAG"];
		v_proc_no = bcls_rec->Tables[0].Rows[0]["PROC_NO"].ToString().TrimOrBlank();

		if (bcls_rec->Tables[0].Columns.Contains("CHECK_FLAG"))
		{
			v_check_flag = bcls_rec->Tables[0].Rows[0]["CHECK_FLAG"].ToString();
			Log::Trace("", __FUNCTION__, "清空标记v_check_flag=[{0}],[{1}],[{2}]", v_check_flag, tpssm11["PONO"].ToString(), v_dev_code);
		}
		if (v_proc_time > datetime)
		{
			sprintf(s.msg, "模拟信号时间超过了当前时间，请确认！");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (v_proc_time < datetime1)
		{
			sprintf(s.msg, "模拟信号时间超过了前三天，请确认！");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		
		//打印传入参数
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>HEAT_NO=[{0}]", tpssm11["HEAT_NO"].ToString());
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>PONO=[{0}]", tpssm11["PONO"].ToString());
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_dev_code=[{0}]", v_dev_code);
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_proc_time=[{0}]", v_proc_time);
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_charge_no=[{0}]", v_charge_no);
		////Log::Trace("", __FUNCTION__, "pssm21_ot_run>v_flag=[{0}]", v_flag);

		//调用函数用
		inBlock.Tables[0].Columns.Add(DT_STRING,"FACTORY_DIV");		
		inBlock.Tables[0].Columns.Add(DT_STRING,"SIMUL_FLAG");//模拟标记
		inBlock.Tables[0].Columns.Add(DT_STRING,"PROC_NO");//处理号
		inBlock.Tables[0].Columns.Add(DT_STRING,"PROC_TIME");//处理时刻
		inBlock.Tables.Add();
		inBlock.Tables[1].Columns.Add(DT_STRING,"PONO");
		inBlock.Tables.Add();
		inBlock.Tables[2].Columns.Add(DT_STRING,"RUN_SIGNAL");//运转信号
		inBlock.Tables[2].Columns.Add(DT_STRING,"AREA_ID");//炼钢区域标识
		inBlock.Tables[2].Columns.Add(DT_STRING,"SRP_SEQ");//精炼重数
		inBlock.Tables[2].Columns.Add(DT_STRING, "CHARGE_NO_2");//预处理重数
		inBlock.Tables[2].Columns.Add(DT_DECIMAL, "TREATMENT_COUNTER");

		if (v_dev_code.SubstringNE(0,1)=="A")
		{
			sqlstr = " SELECT PRE_PROC_NO,AREA_ID,PROC_NO,TREATMENT_COUNTER,SM_PLAN_NO FROM TPSSM12 \
					 					    WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) \
																	AND AREA_ID = 3 ";
		}
		else
		{
			sqlstr = " SELECT PRE_PROC_NO,AREA_ID,PROC_NO,TREATMENT_COUNTER,SM_PLAN_NO FROM TPSSM12 \
					 					    WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) \
																	AND CHARGE_NO = @v_charge_no ";
		}
		
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm12_inq.Parameters.Set("v_dev_code",v_dev_code);
		cmd_tpssm12_inq.Parameters.Set("v_charge_no", v_charge_no);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
			//v_proc_no = cmd_tpssm12_inq.GetString(1);	
			v_area_id = cmd_tpssm12_inq.GetDecimal(2);	
			proc_no = cmd_tpssm12_inq.GetString(3);
			treatment_count = cmd_tpssm12_inq.GetDecimal(4);
			sm_plan_no = cmd_tpssm12_inq.GetString(5);
		}
		cmd_tpssm12_inq.Close();

		//模拟信号工序校验 wcy
		tpssm12.Reset();
		tpssm12["CHARGE_NO"] = v_charge_no;
		tpssm12["SM_PLAN_NO"] = sm_plan_no;
		if (tpssm12.Query("SM_PLAN_NO,CHARGE_NO"))
		{
			if (tpssm12["DEV_CODE"].ToString().Substring(0, 1) != v_dev_code.Substring(0, 1))
			{
				sprintf(s.msg, "计划号[{0}],制造命令号[{1}]的计划路径已变更，请刷新后重新接管。", tpssm12["SM_PLAN_NOL2"].ToString(), tpssm11["PONO"].ToString());
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (v_check_flag.Trim() == "1")
			{
				if (tpssm12["END_TIME_REAL"].ToString().Trim() != "")
				{
					tpssm12["END_TIME_REAL"] = " ";
					tpssm12.Update("END_TIME_REAL", "SM_PLAN_NO,CHARGE_NO");
				}
			}
		}
		//////////////////
		//计算精炼重数
		if (v_area_id != 4)
		{
			v_srp_seq = 0;
		}
		else
		{
#if 1
			switch(conn->DatabaseKind)
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
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		    cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();

			if(cmd_tpssm12_inq.Read())
			{
				v_charge_no_min = cmd_tpssm12_inq.GetDecimal(1);
			}

			v_srp_seq = v_charge_no -v_charge_no_min + 1;//精炼重数
#endif
			
			
		}
		//3、查询run_signal
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
				
			sqlstr = " SELECT RUN_SIGNAL "
					"   FROM TPSSMS1 "
					"  WHERE DEV_CODE = @v_dev_code "
					"    AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"    AND substr(RUN_SIGNAL,1,1)=@v_area_id  "
					"    AND START_OR_END  = @v_flag "
					"    AND IS_IMPORT = 1 "
					"  ORDER BY RUN_SIGNAL ";
			break;
		}

		cmd_tpssms1_inq.SetCommandText(sqlstr);
		cmd_tpssms1_inq.Parameters.Set("v_dev_code", v_dev_code);
		cmd_tpssms1_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssms1_inq.Parameters.Set("v_area_id",v_area_id.ToString().SubstringNE(0,1));
		cmd_tpssms1_inq.Parameters.Set("v_flag",v_flag);
		cmd_tpssms1_inq.ExecuteReader();

		if(cmd_tpssms1_inq.Read())
		{
			v_run_signal = cmd_tpssms1_inq.GetString(1);
			////Log::Info("", __FUNCTION__, "v_run_signal = [{0}]", v_run_signal);
		}
        else
        {
			Log::Info("", __FUNCTION__, "取运转信号参数 DEV_CODE[{0}], FACTORY_DIV[{1}], RUN_SIGNAL[{2}], START_OR_END[{3}]", v_dev_code, tpssm11["FACTORY_DIV"].ToString(), v_area_id.ToString().SubstringNE(0, 1), v_flag);
        }
		cmd_tpssms1_inq.Close();

		Log::Info("", __FUNCTION__, "模拟运转信号v_run_signal = [{0}]", v_run_signal);


		if (proc_no.Trim() == "")
		{
			if (v_area_id == 4 || v_area_id == 5)
			{
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
					Log::Info("", __FUNCTION__, "模拟精炼连铸处理号v_proc_no = [{0}]", v_proc_no);
				}
			}
		}
		if (v_proc_no.Trim() == "")
		{
			if (tpssm11["HEAT_NO"].ToString().Trim() != "")
			{
				if (v_area_id==3)
				{
					v_proc_no = tpssm11["HEAT_NO"].ToString();
				}
			}
		}
		inBlock.Tables[0].Rows.Add();
		inBlock.Tables[0].Rows[0]["FACTORY_DIV"]=tpssm11["FACTORY_DIV"];
		inBlock.Tables[0].Rows[0]["PROC_NO"]=v_proc_no;
		inBlock.Tables[0].Rows[0]["PROC_TIME"]=v_proc_time;	
		inBlock.Tables[0].Rows[0]["SIMUL_FLAG"]="3"; //甘特图模拟

		inBlock.Tables[1].Rows.Add();
		inBlock.Tables[1].Rows[0]["PONO"]=tpssm11["PONO"];

		inBlock.Tables[2].Rows.Add();
		inBlock.Tables[2].Rows[0]["RUN_SIGNAL"] = v_run_signal;
		inBlock.Tables[2].Rows[0]["AREA_ID"]= v_area_id;
		inBlock.Tables[2].Rows[0]["SRP_SEQ"]=v_srp_seq;
		inBlock.Tables[2].Rows[0]["CHARGE_NO_2"] = v_charge_no;
		inBlock.Tables[2].Rows[0]["TREATMENT_COUNTER"] = treatment_count;

		////Log::Trace("", __FUNCTION__, "pono=[{0}],proc_time={1},v_charge_no={2},v_flag={3},v_proc_no={4},v_run_signal={5},v_area_id={6},v_srp_seq={7}", 
			//tpssm11["PONO"].ToString(),v_proc_time,v_charge_no,v_flag,v_proc_no,v_run_signal,v_area_id,v_srp_seq);

		//tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		//tpssm99["PONO"] = tpssm11["PONO"];
		//tpssm99["EVENT_ID"] = "16";
		//tpssm11.Query("FACTORY_DIV,PONO");
		//tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		//tpssm99["RUN_SIGNAL"] = v_run_signal;
		//tpssm99["SIMUL_FLAG"] = simul_flag;
		//tpssm99["PROC_NO"] = v_proc_no;
		//tpssm99["EVENT_DATETIME"] = v_proc_time;//模拟时间

		//////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
		//信号模拟
		ret = f_pssm_run_proc_n(&inBlock, bcls_ret,conn);
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
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//tpssm99["VALID_FLAG"] = "1";
		//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		//////Log::Trace("", __FUNCTION__, "记录成功履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		////记录编入计划成功的履历
		//ret = 0;
		//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//=======================================
		//消息推送测试 （目前规则先按 转炉异钢种吹炼）

		//{
		//	CString module = "PSSM";
		//	CString topic = "EGEGPPSSMB10001";
		//	CString msg = "注意： 工序[3#BOF] 当前炉[22300002],冶炼钢种[Q3552D2T]与上一炉[22300001],冶炼钢种[Q355B1]不同，注意！！！";

		//	Log::Trace("", __FUNCTION__, "==SendMessage begin-------");

		//	SendMessage((const char*)module, (const char*)topic, (const char*)msg);

		//	Log::Trace("", __FUNCTION__, "==SendMessage end---------");
		//}
		ret = f_pssm12_sequ_calc_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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
	cmd_tpssm12_inq.Close();
	cmd_tpssms1_inq.Close();
	return doFlag;

}
