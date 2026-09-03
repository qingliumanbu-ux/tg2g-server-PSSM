/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:   3.1.0
Date:     2014-11-26
Description: 出钢计划之工序计划保存（甘特图用）。
**************************************************************************************************************/
#include "stdafx.h"


//程序用头文件







/*<remark>=========================================================
/// <summary>
///  出钢计划之工序计划保存（单记录处理）
/// <para>处理内容：根据传入的工序计划内容（甘特图），做保存处理。</para>
/// <para>数据库表：TPSSM12(炼钢工序计划表) </para>
/// <para>主调用函数：pssm18_save。           </para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="pono">制造命令号          </param>
/// <param name="qv_elem_div">         </param>
/// <param name="on_elem_div">    </param>
/// <param name="h_elem_div">   </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm12_save_job_n_2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blkseq, rows, i;

	/* 业务变量 */
	CDecimal charge_no = 0;
	CDecimal charge_no_in;
	int srf_count = 0;		 //精炼路径长度计数
	int add_count = 0;
	CString v_factory_div = "";	//炼钢单元号
	CString v_sm_plan_no = "";	//炼钢计划号
	CString v_backlog_ea = "";  	//钢区工序途径: 一位一工序
	CString v_refine_route = "";	//精炼路径
	CTimeSpan proc_time;
	CDateTime start_time;
	CDateTime end_time;
	CString v_steel_start_time = "", v_steel_end_time = "";

	CString v_billet_type = "";
	CString dateNow14 = "";
	CDecimal v_slab_thick_max = 0;
	CDecimal v_slab_thick_min = 0;
	CDecimal v_slab_width_max = 0;
	CDecimal v_slab_width_min = 0;
	CDecimal check_num;

	CString special_flag = "";

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm12_check("TPSSM12");
	CModel tpssm12_old("TPSSM12");//原工序计划信息
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssm16_old("TPSSM16");//原工序计划信息
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd9("TPSSMD9");


	CDbCommand cmd(conn);
	CDbCommand cmd_sql(conn);
	CDbCommand cmd_upd12(conn);
	CString sqlstr;

	CDataTable tb_tpssm12("TPSSM12");  //保存当前炉次的所有工序计划信息

	try
	{
		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//---------------------------------------------------
		//获得输入参数
		//读取炼钢单元号
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm11_ins_heat_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];

			//---------------------------------------------------
			//获得输入参数
			//1、主计划信息, 单记录
			blkseq = bcls_rec->Tables.IndexOf("TPSSM11");
			if (blkseq < 0)
			{
				//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
				sprintf(s.msg, "没有找到主计划数据块[TPSSM11]，请联系系统维护人员。");
				sprintf(s.sysmsg, "TABLE [TPSSM11] NOT EXIST in f_pssm12_save_job_n().");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"].ToString().Trim();
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[0]["SM_PLAN_NO"].ToString().Trim();
			////Log::Info("", __FUNCTION__, " factory_div=[{0}]", v_factory_div);
			////Log::Info("", __FUNCTION__, " sm_plan_no=[{0}]", v_sm_plan_no);

			//读取主计划信息
			tpssm11["FACTORY_DIV"] = v_factory_div;
			tpssm11["SM_PLAN_NO"] = v_sm_plan_no;
			sqlstr = "tpssm11.Query()";
			bool has11 = tpssm11.Query("FACTORY_DIV,SM_PLAN_NO");
			if (has11 == false) //计划表中没有该炉次，新增
			{
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString(), tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "编制的炉次[{0}],计划号[{1}]在出钢计划中不存在。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Info("", __FUNCTION__, " CURR_WP_NO=[{0}]", tpssm11["CURR_WP_NO"].ToDecimal());


			//----------------------------------
			//甘特图编辑后的子工序计划，只要未开始的，就无条件覆盖
			tpssm12["FACTORY_DIV"] = v_factory_div;
			tpssm12["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12["HEAT_NO"] = tpssm11["HEAT_NO"];
			tpssm12["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];//wcy 2级计划号携带

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = "UPDATE TPSSM12 "
					"	SET	CHARGE_NO = CHARGE_NO*10 "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no ";
				break;
			}
			cmd_upd12.SetCommandText(sqlstr);
			cmd_upd12.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd12.Parameters.Set("sm_plan_no1", tpssm11["SM_PLAN_NO"].ToString());
			cmd_upd12.ExecuteNonQuery();
			cmd_upd12.Close();


			//---------------------------------------------------
			//2、循环读取浇铸信息，多记录
			blkseq = bcls_rec->Tables.IndexOf("TPSSM12");
			if (blkseq < 0)
			{
				//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
				sprintf(s.msg, "没有找到工序计划数据块[TPSSM12]，请联系系统维护人员。");
				sprintf(s.sysmsg, "TABLE [TPSSM12] NOT EXIST in f_pssm12_save_job_n().");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			rows = bcls_rec->Tables[blkseq].Rows.get_Count();
			charge_no = 0;
			add_count = 1;
			for (i = 0; i < rows; i++)
			{
				//tpssm12.MergeFrom(bcls_rec->Tables[0].Rows[i]);
				tpssm12["AREA_ID"] = bcls_rec->Tables[blkseq].Rows[i]["AREA_ID"].ToDecimal(); //区域
				tpssm12["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["DEV_CODE"].ToString().Trim(); //设备
				tpssm12["START_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["START_TIME"].ToString().Trim(); //开始时刻
				tpssm12["END_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["END_TIME"].ToString().Trim();    //结束时刻
				charge_no_in = bcls_rec->Tables[blkseq].Rows[i]["CHARGE_NO"].ToDecimal();

				if (bcls_rec->Tables[blkseq].Rows[i]["CHARGE_NO"].ToDecimal() != 0)
				{
					charge_no = bcls_rec->Tables[blkseq].Rows[i]["CHARGE_NO"].ToDecimal();
					add_count = 1;
				}

				if (charge_no_in != 0) continue;
				else
				{
					Log::Info("", __FUNCTION__, " sm_plan_no=[{0}] 新增区域[{1}]工序[{2}] 在原顺序[{3}]之后", v_sm_plan_no, tpssm12["AREA_ID"].ToDecimal(), tpssm12["DEV_CODE"].ToString(), charge_no);

					start_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
					end_time = CDateTime::Parse(tpssm12["END_TIME"].ToString());
					proc_time = end_time - start_time;
					tpssm12["PROC_TIME"] = proc_time.TotalMinutes();
					tpssm12["CHARGE_NO"] = charge_no * 10 + add_count;
					add_count = add_count + 1;
					//Log::Info("", __FUNCTION__, " AREA_ID=[{0}] DEV_CODE=[{1}] START_TIME=[{2}] END_TIME=[{3}]",
					//tpssm12["AREA_ID"].ToDecimal(), tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString());

					//1)查询设备配置信息
					tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
					sqlstr = "tpssmd1.Query()";
					bool hasd1 = tpssmd1.Query("FACTORY_DIV,AREA_ID,DEV_CODE");
					if (hasd1 == false) //没查询到记录
					{
						CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "区域[{0}]下的设备[{1}]配置信息读取不到，请联系系统维护人员。", arguments, 2); //格式化字符串
						CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//拼接钢区工艺途径
					v_backlog_ea = v_backlog_ea + tpssmd1["STATION_ID"].ToString().Trim();

					//有脱硫，主计划的脱硫指示置"1"
					if (tpssmd1["AREA_ID"].ToDecimal() == 1)	tpssm11["DE_SULFUR_FLAG"] = "1";
					else tpssm11["DE_SULFUR_FLAG"] = " ";

					//有预溶液，子计划预溶液指示标记置"1" 20231107 wcy
					if (tpssmd1["AREA_ID"].ToDecimal() == 2 && (tpssmd1["STATION_ID"].ToString().Trim() == "X" || tpssmd1["STATION_ID"].ToString().Trim() == "Y" || tpssmd1["STATION_ID"].ToString().Trim() == "Z"))	tpssm12["PRE_SOLUTION_FLAG"] = "1";
					else tpssm12["PRE_SOLUTION_FLAG"] = " ";

					//有脱P，必然是双联法，主计划的冶炼模式置"2"
					//if (tpssmd1["AREA_ID"].ToDecimal() == 2)	tpssm11["SMELT_MODE"] = 2;  //2-双联
					//else tpssm11["SMELT_MODE"] = 1;

					//确定出钢计划的精炼路径
					if (tpssm12["AREA_ID"].ToDecimal() == 4)
					{
						v_refine_route = v_refine_route + tpssm12["DEV_CODE"].ToString();
					}
					//跳过比当前工序小的charge
					//if (charge_no < tpssm11["CURR_WP_NO"].ToDecimal())  continue;

					tpssm12["SUB_CHARGE_NO"] = 0;

					tpssm12["REC_CREATE_TIME"] = dateNow14;
					tpssm12["REC_CREATOR"] = CString(s.userid);

					//新增
					tpssm12.TrimOrBlank();
					sqlstr = "tpssm12.Insert()";
					tpssm12.Insert();
				}

			}//for 循环读取输入





			/////崩溃校验/////
			tpssm12_check.Reset();
			tpssm12_check["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_check["AREA_ID"] = 3;
			if (tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") != 1)
			{
				CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") };
				CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在区域[{1}]上的设备数量有误[{2}]", arguments, 3);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				tpssm12_check.Query("SM_PLAN_NO,AREA_ID");
				if (tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "B" && tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "A" && tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "E")
				{
					CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check["DEV_CODE"].ToString() };
					CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在区域[{1}]上的设备有误[{2}]", arguments, 3);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			tpssm12_check.Reset();
			tpssm12_check["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_check["AREA_ID"] = 5;
			if (tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") != 1)
			{
				CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") };
				CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在区域[{1}]上的设备数量有误[{2}]", arguments, 3);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				tpssm12_check.Query("SM_PLAN_NO,AREA_ID");
				if (tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "C" && tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "M")
				{
					CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check["DEV_CODE"].ToString() };
					CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在区域[{1}]上的设备有误[{2}]", arguments, 3);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			/////////////////

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " UPDATE TPSSM12 "
					" SET CHARGE_NO = (SELECT RN FROM(SELECT CHARGE_NO, "
					" ROW_NUMBER() OVER(ORDER BY CHARGE_NO ASC) AS RN "
					" FROM TPSSM12 "
					" WHERE FACTORY_DIV = @factory_div AND SM_PLAN_NO = @sm_plan_no ) A WHERE TPSSM12.CHARGE_NO = A.CHARGE_NO) "
					" WHERE FACTORY_DIV = @factory_div AND SM_PLAN_NO = @sm_plan_no ";
				break;
			}
			cmd_upd12.SetCommandText(sqlstr);
			cmd_upd12.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd12.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_upd12.ExecuteNonQuery();
			cmd_upd12.Close();


			//----------------------------------
			//主计划相关信息修改

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"SELECT DECODE(TRIM(START_TIME_REAL), NULL, START_TIME, START_TIME_REAL) START_TIME FROM TPSSM12"
					" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO"
					"   AND area_id = 3 "
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				tpssm11["STEEL_START_TIME"] = cmd.GetString(1);
			}
			cmd.Close();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"SELECT DECODE(TRIM(END_TIME_REAL), NULL, END_TIME, END_TIME_REAL) END_TIME FROM TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO"
					"   AND area_id = 5 "
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				tpssm11["STEEL_END_TIME"] = cmd.GetString(1);
			}
			cmd.Close();

			tpssm11["BACKLOG_EA"] = v_backlog_ea.TrimOrBlank();
			tpssm11["REFINE_ROUTE_CODE"] = v_refine_route.TrimOrBlank();

			sqlstr = "tpssm11.Update()";
			tpssm11.Update(
				"DE_SULFUR_FLAG,"        //脱S指示			
				"BACKLOG_EA,"
				"REFINE_ROUTE_CODE,"
				"CC_MACH_NO,"        //连铸机
				"STEEL_START_TIME,"
				"STEEL_END_TIME",
				"FACTORY_DIV,SM_PLAN_NO");

		

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
