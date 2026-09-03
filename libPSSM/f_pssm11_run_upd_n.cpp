/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-17
Version:1.0
Description: 炼钢计划PONO状态处理函数
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件









int f_t8z_23m_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);//发送专家系统数据
int f_pssm27_upd_plno_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表

#if defined _SYS_PES
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //向L4送炉次确定状态 
#endif

/*<remark>=========================================================
/// <summary>
/// 炼钢运转信号更新计划表
/// <para>处理内容：更新 TPSSM11/12 的表内容，计划状态及时刻        </para>
/// <para> 1.修改计划状态(TPSSM11/12)中记录内容；                   </para>
/// <para> 2.根据PONO, charge_no, 填入各工序处理时刻；              </para>
/// <para> 3.对于浇注完毕信号, 还需修改浇次计划的浇注顺序；         </para>
/// <para> 4.对于浇注完毕信号, 还需修改连铸公共条件表的计划数量；   </para>
/// <para> 5.修改处理号和浇次号                                     </para>
/// <para>数据库表：TPSSM11/12(出钢计划主子表)                  </para>
/// <para>主调用函数：被 f_pssm_run_proc() 函数调用。               </para>
/// </summary>
/// <param name="pono">制造命令          </param>
/// <param name="heat_no">熔炼号         </param>
/// <param name="proc_no">设备处理号     </param>
/// <param name="proc_time">处理时间     </param>
/// <param name="run_signal">信号代码    </param>
/// <param name="dev_code">设备代码      </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm11_run_upd_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret, blkseq;
	EIClass inBlock;
	EIClass inBlock2;
	CString	datetime="";            /* 记录创建时刻 */
	CDecimal dummy=0;
	CString	proc_no="";				/* 处理号 */
	CString	v_heat_no = "";				/* 炉号 */
	CString	ladle_no = "";
	CString	proc_time="";				/* 处理时刻 */
	int     area_id = 0;                    /* 炼钢区域标识 */
	CDecimal     srp_seq = 0;                /* 精炼重数 */
	CDecimal     max_proc_seq=0;
	CDecimal 	now_proc_seq=0;
	//BOOL ishave = false;
	bool ishave = false;
	CString simul_flag = "";            /* 模拟标记 */
	int v_count = 0;
	int mm_count = 0;
	CString tablename = "";
	CString update_flag = "0";

	CModel tpssmd1("TPSSMD1");
	CModel tpssms1("TPSSMS1");
	CModel tpssm01("TPSSM01");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm13("TPSSM13");
	CModel tpssm12("TPSSM12");
	CModel tpssm12_upd("TPSSM12");
	CModel tpssm14("TPSSM14");
	CModel tpssm25("TPSSM25");
	CModel tpssm26("TPSSM26");
	CModel tpssm99("TPSSM99");
	CModel tpssm12_HT("TPSSM12");
	CModel tpssm14_HT("TPSSM14");
	CDbCommand cmd_tpssm_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_tpssm12_upd(conn);
	CDbCommand cmd_tpssm25_upd(conn);
	CDbCommand cmd_tmmsm_inq(conn);
	CString sqlstr;
	CString sqlstr_del;
	CString sqlstr_ins;
	EIClass in_pssm99trace;  //调用履历函数
	CDbCommand cmd_tpssm_del(conn);
	CDbCommand cmd_tpssm_ins(conn);
	try
	{
		datetime=CDateTime::Now().ToString("yyyyMMddHHmmss");

		//设置函数调用输入块参数(inBlock)
		//1)计划顺序号更新
		blkseq = 1;
		inBlock.Tables[blkseq-1].set_TableName("PLAN_NO");
		inBlock.Tables[blkseq-1].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[blkseq-1].Columns.Add(DT_STRING, "PONO");

		//2)炉次钢种管理
		inBlock.Tables.Add();		
		inBlock.Tables[1].set_TableName("TPSSM31");
		inBlock.Tables[1].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[1].Columns.Add(DT_STRING, "PONO");
		inBlock.Tables[1].Columns.Add(DT_STRING, "RUN_SIGNAL");	//运转信号
		inBlock.Tables[1].Columns.Add(DT_STRING, "AREA_ID");	    //运转信号
		inBlock.Tables[1].Columns.Add(DT_STRING, "PROC_NO");		//处理号
		inBlock.Tables[1].Columns.Add(DT_STRING, "CHARGE_NO");

		//3)状态电文参数 HYF 20130401
		inBlock2.Tables[0].set_TableName("X200009");
		inBlock2.Tables[0].Columns.Add(DT_STRING,"FACTORY_DIV");
		inBlock2.Tables[0].Columns.Add(DT_STRING,"PONO");
		inBlock2.Tables[0].Columns.Add(DT_STRING,"PONO_STATUS");

		//5、计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);

		EIClass in_23m;
		in_23m.Tables[0].Columns.Add(DT_STRING, "TC_NO");
		in_23m.Tables[0].Rows.Add();
		in_23m.Tables[0].Rows[0]["TC_NO"] = "T82325";
		in_23m.Tables.Add();
		in_23m.Tables[1].Columns.Add(tpssm11);

		//获得输入参数
		
		tpssms1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().TrimOrBlank();
		tpssms1["RUN_SIGNAL"] = bcls_rec->Tables[0].Rows[0]["RUN_SIGNAL"]; //运转信号
		area_id = bcls_rec->Tables[0].Rows[0]["AREA_ID"];    //炼钢区域标识, 用于区分脱P,脱C
		srp_seq = bcls_rec->Tables[0].Rows[0]["SRP_SEQ"];  //精炼重数
		proc_no = bcls_rec->Tables[0].Rows[0]["PROC_NO"];
		proc_time = bcls_rec->Tables[0].Rows[0]["PROC_TIME"];
		simul_flag = bcls_rec->Tables[0].Rows[0]["SIMUL_FLAG"].ToString();
		v_heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString();
		ladle_no = bcls_rec->Tables[0].Rows[0]["LADLE_NO"].ToString();

		Log::Info("", __FUNCTION__,  "status_upd>factory_div=[{0}], proc_no=[{1}], pono=[{2}], sign=[{3}], v_heat_no=[{4}]", 
			(const char*)tpssms1["FACTORY_DIV"].ToString(), (const char*)proc_no, (const char*)tpssm11["PONO"].ToString(), (const char*)tpssms1["RUN_SIGNAL"].ToString(), v_heat_no);

		tpssmd1["AREA_ID"] = area_id;

		//输入: 信号(sign), 制造命令(PONO), 处理号(proc_no), 处理时刻(proc_time)
		//1)校验有无该运转信号和制造命令	
		ishave = false;
		ishave = tpssms1.Query("FACTORY_DIV,RUN_SIGNAL");
		tpssms1.TrimOrBlank();
		if (ishave == false)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000149")/*运转信号[{0}]不存在，请联系维护人员。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		if (tpssms1["START_OR_END"].ToDecimal() < 1 || tpssms1["START_OR_END"].ToDecimal() > 5)
		{
			sprintf(s.sysmsg, "运转信号[%s]对应start_or_end[%d]不做处理.",(const char*)tpssms1["RUN_SIGNAL"].ToString(),tpssms1["START_OR_END"].ToDecimal().ToInt32());
			return 0;
		}

		ishave = false;
		tpssm11["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
		ishave = tpssm11.Query("FACTORY_DIV,PONO");
		if (ishave == false)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000088")/*制造命令号[{0}]在出钢计划中不存在。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}
		tpssm11.TrimOrBlank();
		tpssm14_HT["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		tpssm14_HT["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm12_HT["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		tpssm12_HT["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		////Log::Trace("", __FUNCTION__, "查询tpssm11.cast_no=[{0}],[{1}]", tpssm11["CAST_NO"].ToString(),tpssm11["CAST_DIV_NO"].ToDecimal());


		tpssmd1["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
		tpssmd1["DEV_CODE"] = tpssms1["DEV_CODE"];
		tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
		tpssmd1.TrimOrBlank();

		
		//---------------------------------------------------------
		//确定charge_no
		if (tpssmd1["AREA_ID"].ToDecimal() == 4)
		{
			//校验信号送入的srp_seq是否正确
			if (srp_seq <= 0)
			{
				//sprintf(s.sysmsg, "同一精炼设备经过2次以上处理时, 信号送入的精炼重数srp_seq=[%d]不正确.", srp_seq);
				CFormattable arguments[] = { srp_seq.ToInt32() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000163")/*精炼重数[{0}]不正确，请联系维护人员。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//1)读取转炉的charge_no

			switch(conn->DatabaseKind)
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
			cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO",tpssm11["SM_PLAN_NO"].ToString());
			//tpssm14.CHARGE_NO = cmd_tpssm14_inq.ExecuteScalar();
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				tpssm12["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(1);
			}
			else //未读到
			{
				//读取第一重精炼的charge_no
				switch(conn->DatabaseKind)
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
				cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				//tpssm14.CHARGE_NO = cmd_tpssm14_inq.ExecuteScalar();
				cmd_tpssm12_inq.ExecuteReader();

				if(cmd_tpssm12_inq.Read())
				{
					tpssm12["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(1);
				}
				else //未读到
				{
					strcpy(s.msg, _RES("PSSMS0000155")/*第一重精炼的工序号不正确。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tpssm12["CHARGE_NO"] = tpssm12["CHARGE_NO"].ToDecimal() -1;
			}

			tpssm12["CHARGE_NO"] = tpssm12["CHARGE_NO"].ToDecimal() + srp_seq;

		}
		else if (tpssmd1["AREA_ID"].ToDecimal() == 2)
		{
			tpssm12["CHARGE_NO"] = bcls_rec->Tables[0].Rows[0]["CHARGE_NO_2"];
		}
		else //非精炼区域的可直接获取charge_no
		{			
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT MAX(CHARGE_NO) FROM TPSSM12 "
						 "  WHERE FACTORY_DIV = @tpssms1.FACTORY_DIV "
						 "    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO  "
						 "    AND AREA_ID           = @tpssmd1.AREA_ID ";
				break;  
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO",tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID",tpssmd1["AREA_ID"].ToDecimal());
			tpssm12["CHARGE_NO"] = cmd_tpssm12_inq.ExecuteScalar();
		}


		//---------------------------------------------------------
		//2.读取 charge_no 下的信息，用于 proc_seq_no 的更新
		tpssm12["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
		tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		sqlstr = "tpssm12.Query";
		tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
		tpssm12.TrimOrBlank();

		//---------------------------------------------------------
		//3.运转信号转换为PONO状态
		//取PONO_STATUS
		//包到、包离不影响proc_no、curr_wp_no
		//冶炼区域取熔炼号
		////Log::Trace("", __FUNCTION__, " tpssms1["RUN_STATUS"] = [{0}]", tpssms1["RUN_STATUS"].ToString());
		if (tpssms1["RUN_STATUS"].ToString()[0] == '3')
		{

			tpssm11["HEAT_NO"] = proc_no;
			if (simul_flag == "3")
			{
				Log::Trace("", __FUNCTION__, " 模拟炉号tpssm11[HEAT_NO] = [{0}]", tpssm11["HEAT_NO"].ToString());
			}

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT COUNT(1) FROM TPSSM11 "
					"  WHERE FACTORY_DIV = @tpssms1.FACTORY_DIV "
					"    AND PONO != @tpssm11.PONO "
					"    AND HEAT_NO = @tpssm11.HEAT_NO ";
				break;
			}

			cmd_tpssm_inq.SetCommandText(sqlstr);
			cmd_tpssm_inq.Parameters.Set("tpssms1.FACTORY_DIV", tpssms1["FACTORY_DIV"].ToString());
			cmd_tpssm_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
			v_count = cmd_tpssm_inq.ExecuteScalar().ToInt32();
			
			if (v_count != 0)
			{
				strcpy(s.msg, "该炉号已经存在，请核实确认！");
				//throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm12_HT["AREA_ID"] = 3;
			tpssm12_HT["PROC_NO"] = proc_no;
			tpssm12_HT.Update("PROC_NO", "FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm14_HT["AREA_ID"] = 3;
			tpssm14_HT["PROC_NO"] = proc_no;
			tpssm14_HT.Update("PROC_NO", "FACTORY_DIV,SM_PLAN_NO,AREA_ID");
		}

		//run_status>=31(冶炼开始),pono状态为20
		if ((tpssms1["RUN_STATUS"].ToString().Compare("21") >= 0) && (tpssm11["PONO_STATUS"].ToDecimal() < 20))//预溶液开始21
		{
			tpssm11["PONO_STATUS"] = 20;
		}
		//run_status>=53(浇铸结束),pono状态为83
		if ((tpssms1["RUN_STATUS"].ToString().Compare("53") >= 0) && (tpssm11["PONO_STATUS"].ToDecimal() < 83))
		{
			tpssm11["PONO_STATUS"] = 83;
		}
		//原run_status<本次run_status 或 原当前工序<本次工序时，取本次run_status
		if ((tpssm11["RUN_STATUS"].ToString().Compare(tpssms1["RUN_STATUS"].ToString()) < 0) || (tpssm11["CURR_WP_NO"].ToDecimal() <tpssm12["CHARGE_NO"].ToDecimal()))
		{
			tpssm11["RUN_STATUS"] = tpssms1["RUN_STATUS"];
		}
		//原run_status<本次run_status 或 原当前工序<本次工序时，取本次run_status
		if ((tpssm11["CURR_WP_NO"].ToDecimal() <tpssm12["CHARGE_NO"].ToDecimal()) && (tpssms1["START_OR_END"].ToDecimal() >=2) && (tpssms1["START_OR_END"].ToDecimal() <=3))
		{
			tpssm11["CURR_WP_NO"] = tpssm12["CHARGE_NO"];
		}
		////Log::Trace("", __FUNCTION__, "run_status=[{0}]", (const char*)tpssm11["RUN_STATUS"].ToString());


		//---------------------------------------------------------------------------------
		//对于浇注完毕信号, 还需修改浇次计划的浇注顺序
		//更新tpssm10
		//
		if (tpssm11["PONO_STATUS"].ToDecimal() < 83) //83-浇注完毕
		{
			//修改该PONO的状态
			tpssm10["PONO_STATUS"] = tpssm11["PONO_STATUS"];
			tpssm10["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm10["PONO"] =tpssm11["PONO"];
			tpssm10.Update("PONO_STATUS","FACTORY_DIV,PONO");
		}
		else
		{
			//记录当前的浇注顺序

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT CC_SEQ, CC_MACH_NO FROM TPSSM10 "
						 "  WHERE FACTORY_DIV = @tpssms1.FACTORY_DIV "
						 "    AND PONO              = @tpssm11.PONO ";
				break;
			}

			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
			cmd_tpssm10_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString());
			cmd_tpssm10_inq.ExecuteReader();
			if(cmd_tpssm10_inq.Read())
			{
				tpssm10["CC_SEQ"] = cmd_tpssm10_inq.GetDecimal(1);
				tpssm10["CC_MACH_NO"] = cmd_tpssm10_inq.GetString(1);
			}
			cmd_tpssm10_inq.Close();
			//修改该PONO的状态和顺序号

			tpssm10["CC_SEQ"] = 0;
			tpssm10["PONO_STATUS"] = tpssm11["PONO_STATUS"];
			tpssm10["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm10["PONO"] =tpssm11["PONO"];

			sqlstr = "tpssm10.Update()";
			tpssm10.Update("CC_SEQ,PONO_STATUS","FACTORY_DIV,PONO");
			////Log::Info("", __FUNCTION__, "factory_div=[{0}], ccm_no=[{1}], cc_seq=[{2}], ",(const char*)tpssms1["FACTORY_DIV"].ToString(),(const char*)tpssm10["CC_MACH_NO"].ToString(),tpssm10["CC_SEQ"].ToDecimal().ToInt32());

			if (tpssm10["CC_SEQ"].ToDecimal() > 0)
			{
				//修改后续炉次的顺序号
				sqlstr = " UPDATE TPSSM10 "
						 " SET CC_SEQ            = CC_SEQ - 1 "
						 " WHERE FACTORY_DIV = TRIM(@tpssms1.FACTORY_DIV) "
						 "   AND CC_MACH_NO        = TRIM(@tpssm10.CC_MACH_NO) "
						 "   AND CC_SEQ			   > @tpssm10.CC_SEQ "
						 "   AND CC_SEQ            < 900 ";//900以后的都是封锁的计划

				cmd_tpssm10_upd.SetCommandText(sqlstr);
				cmd_tpssm10_upd.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
				cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO",tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ",tpssm10["CC_SEQ"].ToDecimal());
				cmd_tpssm10_upd.ExecuteNonQuery();
			
			}
		}


		//---------------------------------------------------------------------------------
		//1.更新TPSSM11/13表		
		sqlstr = "tpssm11.Update()";

		if (v_heat_no.Trim() != "" && tpssm11["HEAT_NO"].ToString().Trim() != v_heat_no.Trim() && ((tpssms1["RUN_STATUS"].ToString()[0] == '4') || (tpssms1["RUN_STATUS"].ToString()[0] == '5')))
		{
			Log::Trace("", __FUNCTION__, " 计划[{0}] 原炉号HEAT_NO = [{1}],信号[{2}] 炉号[{3}]", tpssm11["SM_PLAN_NO"].ToString(), tpssm11["HEAT_NO"].ToString(), tpssms1["RUN_SIGNAL"].ToString(),v_heat_no);
			tpssm11["HEAT_NO"] = v_heat_no;
			update_flag = "1";		
		}

		tpssm12_HT["HEAT_NO"] = tpssm11["HEAT_NO"];
		tpssm12_HT["PROC_NO"] = tpssm12_HT["HEAT_NO"];
		tpssm12_HT["AREA_ID"] = 3;
		tpssm12_HT.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");
		tpssm12_HT.Update("PROC_NO", "FACTORY_DIV,SM_PLAN_NO,AREA_ID");

		tpssm14_HT["HEAT_NO"] = tpssm11["HEAT_NO"];
		tpssm14_HT["PROC_NO"] = tpssm12_HT["HEAT_NO"];
		tpssm14_HT["AREA_ID"] = 3;
		tpssm14_HT.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");
		tpssm14_HT.Update("PROC_NO", "FACTORY_DIV,SM_PLAN_NO,AREA_ID");
		////Log::Trace("", __FUNCTION__, " 打印11表需要更新的字段tpssm11["HEAT_NO"] = [{0}]", tpssm11["HEAT_NO"].ToString());
		////Log::Trace("", __FUNCTION__, " 打印需要更新的字段tpssm11["PONO_STATUS"] = [{0}]", tpssm11["PONO_STATUS"].ToDecimal());
		////Log::Trace("", __FUNCTION__, " 打印需要更新的字段tpssm11["RUN_STATUS"] = [{0}]", tpssm11["RUN_STATUS"].ToString());
		////Log::Trace("", __FUNCTION__, " 打印需要更新的字段tpssm11["CURR_WP_NO"] = [{0}]", tpssm11["CURR_WP_NO"].ToDecimal());

		if (ladle_no.Trim() != "")
		{
			tpssm11["LADLE_NO"] = ladle_no;
			tpssm11.Update("HEAT_NO,PONO_STATUS,RUN_STATUS,CURR_WP_NO,LADLE_NO", "FACTORY_DIV,PONO");
		}
		else
		{
			tpssm11.Update("HEAT_NO,PONO_STATUS,RUN_STATUS,CURR_WP_NO", "FACTORY_DIV,PONO");
		}


		//wcy 复制表处理
		tpssm13["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm13["PONO"] = tpssm11["PONO"];
		tpssm13["HEAT_NO"] = tpssm11["HEAT_NO"];
		tpssm13["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		tpssm13["RUN_STATUS"] = tpssm11["RUN_STATUS"];
		tpssm13["CURR_WP_NO"] = tpssm11["CURR_WP_NO"];
		if (tpssm13.QueryCount("PONO") == 1)
		{
			tpssm13.Update("HEAT_NO,PONO_STATUS,RUN_STATUS,CURR_WP_NO", "FACTORY_DIV,PONO");
		}

		
		//2.更新TPSSM12/14表
		//需主键字段 pono, CHARGE_NO
		//tpssms1["DEV_CODE"] 是信号传入中实际工作设备; tpssm14中记录的是原计划信息
		tpssm12["PROC_NO"] = proc_no;

		switch (tpssms1["START_OR_END"].ToDecimal().ToInt32())
		{
		case 1://包到

			//tpssm12["PROC_NO"] = tpssm14.PROC_NO;
			tpssm12["ARRIVE_REAL_TIME"] = proc_time;
			tpssm12["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];			
			sqlstr = "tpssm12.Update()";
			tpssm12.Update("ARRIVE_REAL_TIME", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

			tpssm14["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm14["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm14["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm14["ARRIVE_REAL_TIME"] = tpssm12["ARRIVE_REAL_TIME"];
			tpssm14.Update("ARRIVE_REAL_TIME", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO"); 
			break;

		case 2://处理开始
			
			if ( tpssm12["DEV_CODE"].ToString()[0] != tpssms1["DEV_CODE"].ToString()[0] )
			{
				////Log::Trace("", __FUNCTION__, "计划的设备[{0}]与生产投入的设备[{1}]不同",tpssm12["DEV_CODE"].ToString(), tpssms1["DEV_CODE"].ToString());
			}

			tpssm12["DEV_CODE"] = tpssms1["DEV_CODE"];			
			tpssm12["START_TIME_REAL"] = proc_time;
			tpssm12["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			sqlstr = "tpssm12.Update(DEV_CODE,PROC_NO,START_TIME_REAL)";

			tpssm12.Update("DEV_CODE,PROC_NO,START_TIME_REAL","FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

			if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
			{
				tpssm12["END_TIME"] = (CDateTime::Parse(proc_time).AddMinutes(tpssm12["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");
				tpssm12.Update("END_TIME", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
			}
			tpssm14["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm14["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm14["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm14["START_TIME_REAL"] = tpssm12["START_TIME_REAL"];
			tpssm14["DEV_CODE"] = tpssm12["DEV_CODE"];
			tpssm14["PROC_NO"] = tpssm12["PROC_NO"];
			tpssm14.Update("DEV_CODE,START_TIME_REAL", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
			//12表没有实绩处理时间，暂不处理
			

			if (v_heat_no.Trim() != "" && update_flag == "1" && ((tpssms1["RUN_STATUS"].ToString()[0] == '4') || (tpssms1["RUN_STATUS"].ToString()[0] == '5')))
			{
				tpssm12_upd.Reset();
				tpssm12_upd["PROC_NO"] = v_heat_no;
				tpssm12_upd["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
				tpssm12_upd["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				tpssm12_upd["AREA_ID"] = 3;
				tpssm12_upd.Update("PROC_NO","SM_PLAN_NO,FACTORY_DIV,AREA_ID");
			}

			break;
		case 3://处理结束

			tpssm12["DEV_CODE"] = tpssms1["DEV_CODE"];
			tpssm12["END_TIME_REAL"] = proc_time;
			tpssm12["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12.Update("DEV_CODE,PROC_NO,END_TIME_REAL","FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

			if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
			{
				tpssm12["START_TIME"] = (CDateTime::Parse(proc_time).AddMinutes(tpssm12["PROC_TIME"].ToDouble() * -1)).ToString("yyyyMMddHHmmss");
				tpssm12.Update("START_TIME", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
			}

			tpssm14["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm14["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm14["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm14["END_TIME_REAL"] = tpssm12["END_TIME_REAL"];
			tpssm14["DEV_CODE"] = tpssm12["DEV_CODE"]; 
			tpssm14["PROC_NO"] = tpssm12["PROC_NO"];
			tpssm14.Update("DEV_CODE,END_TIME_REAL", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

			if (v_heat_no.Trim() != "" && update_flag == "1" && ((tpssms1["RUN_STATUS"].ToString()[0] == '4') || (tpssms1["RUN_STATUS"].ToString()[0] == '5')))
			{
				tpssm12_upd.Reset();
				tpssm12_upd["PROC_NO"] = v_heat_no;
				tpssm12_upd["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
				tpssm12_upd["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				tpssm12_upd["AREA_ID"] = 3;
				tpssm12_upd.Update("PROC_NO", "SM_PLAN_NO,FACTORY_DIV,AREA_ID");
			}
		
			////Log::Trace("", __FUNCTION__, "模拟信号，查询实绩是否已收simul_flag = [{0}], area_id = [{1}]", simul_flag, tpssmd1["AREA_ID"].ToDecimal());
			if (simul_flag != "0")
			{
				if (tpssmd1["AREA_ID"].ToDecimal() == 3 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "B") //BOF
				{
					tablename = "tmmsm21";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where heat_no = @heat_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and area_id = 3 and heat_no = @heat_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 3 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "A") // LF
				{
					tablename = "tmmsm22";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
					cmd_tmmsm_inq.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 4 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "F") // LF
				{
					tablename = "tmmsm24";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
					cmd_tmmsm_inq.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 4 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "R") //RH
				{
					tablename = "tmmsm23";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
					cmd_tmmsm_inq.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 4 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "V") // LF
				{
					tablename = "tmmsm25";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
					cmd_tmmsm_inq.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 4 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "S") // LF
				{
					tablename = "tmmsm26";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
					cmd_tmmsm_inq.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 5) //CC
				{
					tablename = "tmmsm31";
					mm_count = 0;

					sqlstr = "select count(*) from " + tablename + " where heat_no = @heat_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						mm_count = cmd_tmmsm_inq.GetInt32(1);
					}
					cmd_tmmsm_inq.Close();

					if (mm_count > 0)
					{
						sqlstr = "update tpssm12 set pract_rcv_flag = '1' where factory_div = @factory_div and heat_no = @heat_no and proc_no = @proc_no ";

						cmd_tpssm12_upd.SetCommandText(sqlstr);
						cmd_tpssm12_upd.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
						cmd_tpssm12_upd.Parameters.Set("heat_no", tpssm12["HEAT_NO"].ToString());
						cmd_tpssm12_upd.Parameters.Set("proc_no", tpssm12["PROC_NO"].ToString());
						cmd_tpssm12_upd.ExecuteNonQuery();

						dummy = tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
						if (dummy == 0) //全收到实绩的工序, 或收到连铸实绩 
						{
							//修改实绩接收标记
							tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
							tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
							tpssm11["PRACT_RCV_FLAG"] = "1";
							tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
						}
					}
				}
			}
			break;
		case 4://包离

			tpssm12["LEAVE_REAL_TIME"] = proc_time;
			tpssm12["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12.Update("LEAVE_REAL_TIME","FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

			tpssm14["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm14["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm14["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm14["LEAVE_REAL_TIME"] = tpssm12["LEAVE_REAL_TIME"];
			tpssm14.Update("LEAVE_REAL_TIME", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

			break;
		}
		tpssm11["STEEL_START_TIME"] = proc_time;
		tpssm11.MergeTo(in_23m.Tables[1]);
		int _ret_23 = 0;
		if (in_23m.Tables[1].Rows.get_Count() > 0)
		{
			_ret_23 = f_t8z_23m_snd(&in_23m, bcls_ret, conn);
		}

		//---------------------------------------------------------------------------------
		//修改TPSSM01表中的计划状态		

		tpssm01["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		tpssm01["FACTORY_DIV"] =tpssms1["FACTORY_DIV"];
		tpssm01["PONO"] = tpssm11["PONO"];
		tpssm01.Update("PONO_STATUS","FACTORY_DIV,PONO");
		//---------------------------------------------------------------------------------
		

	
		//---------------------------------------------------------------------------------
		//更新处理号记录表 tpssm25
		//
		tpssm25["REC_CREATOR"] = s.userid                  ;
		tpssm25["REC_CREATE_TIME"] = datetime               ;
		tpssm25["FACTORY_DIV"] = tpssms1["FACTORY_DIV"].ToString() ;
		tpssm25["STATION_ID"] = tpssmd1["STATION_ID"].ToString()        ;
		tpssm25["STATION_NO"] = tpssmd1["STATION_NO"].ToString()        ;

		if (proc_no.Trim() != "")
		{

			if (tpssm25["STATION_ID"].ToString().Trim() == "X" && tpssm25["STATION_NO"].ToString().Trim() == "1")//wcy 脱p处理号处理
			{
				proc_no = tpssm25["STATION_ID"].ToString() + tpssm25["STATION_NO"].ToString() + proc_no.Substring(2);
			}
			else if (tpssm25["STATION_ID"].ToString().Trim() == "X" && tpssm25["STATION_NO"].ToString().Trim() == "2")
			{
				proc_no = tpssm25["STATION_ID"].ToString() + tpssm25["STATION_NO"].ToString() + proc_no.Substring(2);
			}
			else if (tpssm25["STATION_ID"].ToString().Trim() == "Y" && tpssm25["STATION_NO"].ToString().Trim() == "1")
			{
				proc_no = tpssm25["STATION_ID"].ToString() + tpssm25["STATION_NO"].ToString() + proc_no.Substring(2);
			}
			else if (tpssm25["STATION_ID"].ToString().Trim() == "Y" && tpssm25["STATION_NO"].ToString().Trim() == "2")
			{
				proc_no = tpssm25["STATION_ID"].ToString() + tpssm25["STATION_NO"].ToString() + proc_no.Substring(2);
			}
			else if (tpssm25["STATION_ID"].ToString().Trim() == "Y" && tpssm25["STATION_NO"].ToString().Trim() == "3")
			{
				proc_no = tpssm25["STATION_ID"].ToString() + tpssm25["STATION_NO"].ToString() + proc_no.Substring(2);
			}
			else
			{
				proc_no = tpssm25["STATION_ID"].ToString() + tpssm25["STATION_NO"].ToString() + proc_no.Substring(2);
			}
		}
		tpssm25["CURR_PROC_NO"] = proc_no                   ;
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
		
			////Log::Trace("", __FUNCTION__, "结束更新25表处理号[{0}]", tpssm25["CURR_PROC_NO"].ToString());
		}

		//---------------------------------------------------------------------------------
		//更新浇次记录表 TPSSM26
		// 对开浇或浇注完的信号, 需记录最新的CAST_NO
		//
		//if (tpssms1["RUN_STATUS"].ToString().Compare("52") == 0 || tpssms1["RUN_STATUS"].ToString().Compare("53") == 0) //52-开浇, 53-浇完
		if (tpssms1["RUN_STATUS"].ToString().Compare("52") > 0)
		{
			//modify by gdl 20120301
			tpssm26["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
			tpssm26["DEV_CODE"] = tpssms1["DEV_CODE"];
			tpssm26.Query("FACTORY_DIV,DEV_CODE");
			if (tpssm26["CAST_NO"].ToString() <tpssm11["CAST_NO"].ToString())
			{
				tpssm26["CAST_NO"] = tpssm11["CAST_NO"];
				tpssm26["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				tpssm26.Update("CAST_NO,CAST_DIV_NO","FACTORY_DIV,DEV_CODE");
			}
			else
			{
				if (tpssm26["CAST_NO"].ToString() ==tpssm11["CAST_NO"].ToString() && tpssm26["CAST_DIV_NO"].ToDecimal() <tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					tpssm26["CAST_DIV_NO"]=tpssm11["CAST_DIV_NO"];
					tpssm26.Update("CAST_DIV_NO","FACTORY_DIV,DEV_CODE");
				}
			}
			////Log::Trace("", __FUNCTION__, "更新tpssm26["CAST_NO"] =[{0}],CAST_DIV_NO=[{1}]", tpssm26["CAST_NO"].ToString(), tpssm26["CAST_DIV_NO"].ToDecimal());
		}

		//20130401 HYF 何云飞 增加浇铸结束时向MMS发送状态电文
#ifdef _SYS_PES

		//if(tpssms1["RUN_STATUS"].ToString().Compare("53") == 0)
		//{
		//	//发送炼钢PONO状态
		//	CDataRow &row = inBlock2.Tables[0].Rows.Add();
		//	row["FACTORY_DIV"] = tpssms1["FACTORY_DIV"];
		//	row["PONO"] = tpssm11["PONO"];
		//	row["PONO_STATUS"] = 83;

		//	ret = f_cm_200009_snd(&inBlock2, bcls_ret,conn);
		//	if (ret != 0) //调用不成功
		//	{
		//		//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

#endif


		//---------------------------------------------------------------------------------
		//更新计划号表 TPSSM27
		CDataRow &row1 = inBlock.Tables["PLAN_NO"].Rows.Add();
		row1["FACTORY_DIV"] =  tpssms1["FACTORY_DIV"];
		row1["PONO"] = 			     tpssm11["PONO"];

		ret = f_pssm27_upd_plno_n(&inBlock, bcls_ret,conn);
		if (ret != 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);	
		}

		//dclian---add---2015-11-16------
		//////Log::Trace("", __FUNCTION__, "信号模拟，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
		//tpssm11["PONO"] = tpssm11["PONO"];
		//tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		//tpssm11.Query("FACTORY_DIV,PONO");
		tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = tpssm11["PONO"];

		if (simul_flag == "0")
		{
			tpssm99["EVENT_ID"] = "C3"; //信号接收
		}
		else if (simul_flag == "1")
		{
			tpssm99["EVENT_ID"] = "C1"; //信号模拟(51画面)
		}
		else if (simul_flag == "3")
		{
			tpssm99["EVENT_ID"] = "C2"; //信号模拟(甘特图)
		}
		
		tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		tpssm99["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
		tpssm99["SIMUL_FLAG"] = simul_flag;
		tpssm99["PROC_NO"] = proc_no;
		tpssm99["EVENT_DATETIME"] = proc_time;//模拟时间

		////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

		tpssm99["VALID_FLAG"] = "1";
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpssmd1["AREA_ID"].ToDecimal()<4 && (tpssms1["START_OR_END"].ToDecimal().ToInt32() == 2 || tpssms1["START_OR_END"].ToDecimal().ToInt32() == 3))
		{
			sqlstr_del = "  DELETE TPSSM12_ZPL  WHERE SM_PLAN_NO =@sm_plan_no ";
			cmd_tpssm_del.SetCommandText(sqlstr_del);
			cmd_tpssm_del.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString().Trim());
			cmd_tpssm_del.ExecuteNonQuery();
			cmd_tpssm_del.Close();

			sqlstr_ins = " INSERT INTO  TPSSM12_ZPL (SELECT *  FROM TPSSM12  WHERE AREA_ID IN(2,3) AND SM_PLAN_NO =@sm_plan_no) ";
			cmd_tpssm_ins.SetCommandText(sqlstr_ins);
			cmd_tpssm_ins.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString().Trim());
			cmd_tpssm_ins.ExecuteNonQuery();
			cmd_tpssm_ins.Close();
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

	cmd_tpssm10_inq.Close();
	return doFlag;
}
