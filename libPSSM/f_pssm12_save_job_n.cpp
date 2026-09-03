/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:   3.1.0
Date:     2014-11-26
Description: 出钢计划之工序计划保存（甘特图用）。
**************************************************************************************************************/
#include "stdafx.h"


//程序用头文件





int f_pssm_query(EIClass inblock_condition, EIClass inblock_source, EIClass& outblock_result, CDbConnection * conn);

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
int f_pssm12_save_job_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blkseq, blkseq2,rows, i;

	/* 业务变量 */
	int charge_no;
	int srf_count = 0;		 //精炼路径长度计数
	CString v_factory_div = "";	//炼钢单元号
	CString v_sm_plan_no = "";	//炼钢计划号
	CString v_backlog_ea = "";  	//钢区工序途径: 一位一工序
	CString v_refine_route = "";	//精炼路径
	CTimeSpan proc_time;
	CDateTime start_time;
	CDateTime end_time;
	CString v_steel_start_time = "", v_steel_end_time = "";
	CString cs_c_div = "";
	CString v_billet_type = "";
	CString dateNow14 = "";
	CDecimal v_slab_thick_max = 0;
	CDecimal v_slab_thick_min = 0;
	CDecimal v_slab_width_max = 0;
	CDecimal v_slab_width_min = 0;
	CDecimal check_num;

	CString special_flag = "";
	CString insertsql = ""; 
	CString updatesql = "";
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm12_check("TPSSM12");
	CModel tpssm12_old("TPSSM12");//原工序计划信息
	
	CModel tpssmd1("TPSSMD1");
	//CModel tpssmd9("TPSSMD9");

	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD1_CONDI;
	EIClass TPSSMD1_RESULT;
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "AREA_ID");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	CDbCommand cmd(conn);
	CDbCommand cmd_sql(conn);
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
		if (bcls_rec->Tables[blkseq].Columns.Contains("SPECIAL_FLAG"))
		{
			special_flag = bcls_rec->Tables[blkseq].Rows[0]["SPECIAL_FLAG"].ToString();
		}

		blkseq2 = bcls_rec->Tables.IndexOf("TPSSMD1");
		if (blkseq >= 0)
		{
			TPSSMD1_SOURCE.Tables[0].Copy(bcls_rec->Tables[blkseq2]);
		}

		if (special_flag != "1")
		{

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

			//1)删除前保存当前炉次的所有工序计划信息
			//tb_tpssm12.Clear();
			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:	        // MS SQL Server数据库
			//case DB_KIND_ORACLE:        // Oracle 数据库
			//default: // 所有数据库适用，通用SQL语句
			//	sqlstr = CString(
			//		" SELECT * FROM TPSSM12 "
			//		"  WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
			//		"    AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
			//		"    AND SUB_CHARGE_NO = 0 "  //0-主工序，甘特图只算这个
			//		"  ORDER BY CHARGE_NO ASC "
			//		);
			//	break;
			//}
			//cmd.SetCommandText(sqlstr);
			//cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString().Trim());
			//cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString().Trim());
			//cmd.ExecuteQuery(tb_tpssm12);

			//2）删除当前炉次的未开始工序计划
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" DELETE TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
					"   AND CHARGE_NO  > @tpssm11.CURR_WP_NO "  //1-脱硫
					"   AND ARRIVE_REAL_TIME  = ' ' "     //wcy  重要，会导致保存就把包到时间全刷了 在一查询 预处理号都更新了
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString().Trim());
			cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString().Trim());
			cmd.Parameters.Set("tpssm11.CURR_WP_NO", tpssm11["CURR_WP_NO"].ToDecimal());
			cmd.ExecuteNonQuery();


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
			for (i = 0; i < rows; i++)
			{
				//tpssm12.MergeFrom(bcls_rec->Tables[0].Rows[i]);
				tpssm12["AREA_ID"] = bcls_rec->Tables[blkseq].Rows[i]["AREA_ID"].ToDecimal(); //区域
				tpssm12["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["DEV_CODE"].ToString().Trim(); //设备
				tpssm12["START_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["START_TIME"].ToString().Trim(); //开始时刻
				tpssm12["END_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["END_TIME"].ToString().Trim();    //结束时刻

				start_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
				end_time = CDateTime::Parse(tpssm12["END_TIME"].ToString());
				proc_time = end_time - start_time;
				tpssm12["PROC_TIME"] = proc_time.TotalMinutes();

				//Log::Info("", __FUNCTION__, " AREA_ID=[{0}] DEV_CODE=[{1}] START_TIME=[{2}] END_TIME=[{3}]",
				//tpssm12["AREA_ID"].ToDecimal(), tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString());


				//charge_no赋值, 必须从1开始
				charge_no = charge_no + 1;

				//1)查询设备配置信息
				tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				//sqlstr = "tpssmd1.Query()";
				//bool hasd1 = tpssmd1.Query("FACTORY_DIV,AREA_ID,DEV_CODE");
				//if (hasd1 == false) //没查询到记录
				//{
				//	CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "区域[{0}]下的设备[{1}]配置信息读取不到，请联系系统维护人员。", arguments, 2); //格式化字符串
				//	CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 2);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				TPSSMD1_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm12["DEV_CODE"];
				TPSSMD1_CONDI.Tables[0].Rows[0]["AREA_ID"] = tpssm12["AREA_ID"];
				TPSSMD1_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);

				if (TPSSMD1_RESULT.Tables[0].Rows.get_Count() != 1)
				{
					if (tpssmd1["AREA_ID"].ToDecimal() == 2 && tpssmd1["DEV_CODE"].ToString().Substring(0, 1) == "A")
					{
						CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString(), tpssm12["SM_PLAN_NOL2"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "计划号[{2}]不能将AOD设备做预溶液处理，当前保存失败，请检查后刷新。", arguments, 3); //格式化字符串
						CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 3);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					else
					{
						CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "区域[{0}]下的设备[{1}]配置信息读取不到，请联系系统维护人员。", arguments, 2); //格式化字符串
						CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);


				//拼接钢区工艺途径
				v_backlog_ea = v_backlog_ea + tpssmd1["STATION_ID"].ToString().Trim();

				//有脱硫，主计划的脱硫指示置"1"
				if (tpssmd1["AREA_ID"].ToDecimal() == 1)	tpssm11["DE_SULFUR_FLAG"] = "1";
				else tpssm11["DE_SULFUR_FLAG"] = " ";

				//有预溶液，子计划预溶液指示标记置"1" 20231107 wcy
				if (tpssmd1["AREA_ID"].ToDecimal() == 2 && (tpssmd1["STATION_ID"].ToString().Trim() == "X" || tpssmd1["STATION_ID"].ToString().Trim() == "Y" || tpssmd1["STATION_ID"].ToString().Trim() == "Z" || tpssmd1["STATION_ID"].ToString().Trim() == "L"))	tpssm12["PRE_SOLUTION_FLAG"] = "1";
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
				if (charge_no < tpssm11["CURR_WP_NO"].ToDecimal())  continue;


				//读取原工序计划信息
				tpssm12_old["FACTORY_DIV"] = v_factory_div;
				tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
				tpssm12_old["CHARGE_NO"] = charge_no;
				tpssm12_old["SUB_CHARGE_NO"] = 0;
				sqlstr = "tpssm12_old.Query()";
				bool has_12 = tpssm12_old.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"); //用主键查询

				//Log::Info("", __FUNCTION__, " AREA_ID=[{0}] DEV_CODE=[{1}] START_TIME=[{2}] END_TIME=[{3}]",
				//tpssm12_old["AREA_ID"].ToDecimal(), tpssm12_old["DEV_CODE"].ToString(), tpssm12_old["START_TIME"].ToString(), tpssm12_old["END_TIME"].ToString());

				if (has_12 == false) //不存在，则新增
				{
					/////崩溃校验/////
					tpssm12_check.Reset();
					tpssm12_check["AREA_ID"] = tpssm12["AREA_ID"];
					tpssm12_check["SM_PLAN_NO"] = v_sm_plan_no;
					if (tpssm12_check["AREA_ID"].ToDecimal() == 3 || tpssm12_check["AREA_ID"].ToDecimal() == 5)
					{
						check_num = tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO");
						if (check_num >= 1)
						{
							CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString() };
							CMessageFormat::Format(s.msg, "计划[{0}]已经存在区域[{1}]上的设备,跳过", arguments, 2);

							charge_no = charge_no - 1;
							continue;
						}
					}
					/////////////////
					tpssm12["CHARGE_NO"] = charge_no;
					tpssm12["SUB_CHARGE_NO"] = 0;

					tpssm12["REC_CREATE_TIME"] = dateNow14;
					tpssm12["REC_CREATOR"] = CString(s.userid);

					//判断是否换机浇铸 wcy 不需判定
					if (tpssm12["AREA_ID"].ToDecimal().ToInt32() == 5 && tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString().Trim())
					{
						//罪过啊，换机浇铸太恐怖，一堆数据要更新
						tpssm11["CC_MACH_NO"] = tpssmd1["STATION_NO"];

						//if (tpssmd1["STATION_ID"].ToString() == "C")
						//{
						//	switch (conn->DatabaseKind)
						//	{
						//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
						//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
						//	case DB_KIND_ORACLE:	    // Oracle 数据库
						//	default:
						//		sqlstr = "SELECT MAX(SLAB_THICK), MIN(SLAB_THICK), "
						//			"  MAX(SLAB_WIDTH), MIN(SLAB_WIDTH), MAX(BILLET_TYPE) "
						//			"  FROM TPSSM03 "
						//			" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
						//			"   AND PONO = @tpssm11.PONO ";
						//		break;
						//	}
						//	cmd_sql.SetCommandText(sqlstr);
						//	cmd_sql.Parameters.Clear();
						//	cmd_sql.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						//	cmd_sql.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
						//	cmd_sql.ExecuteReader();
						//	if (cmd_sql.Read())
						//	{
						//		v_slab_thick_max = cmd_sql.GetDecimal(1);
						//		v_slab_thick_min = cmd_sql.GetDecimal(2);
						//		v_slab_width_max = cmd_sql.GetDecimal(3);
						//		v_slab_width_min = cmd_sql.GetDecimal(4);
						//		v_billet_type = cmd_sql.GetString(5);
						//	}
						//	cmd_sql.Close();

						//	tpssmd9["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
						//	tpssmd9["BILLET_TYPE"] = v_billet_type;
						//	tpssmd9["CAST_THICK"] = v_slab_thick_max;
						//	tpssmd9["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
						//	if (tpssmd9.Query("FACTORY_DIV,CC_MACH_NO, BILLET_TYPE, CAST_THICK") == false)
						//	{
						//		CFormattable arguments[] = { tpssmd9["CC_MACH_NO"].ToString(), tpssmd9["BILLET_TYPE"].ToString(), tpssmd9["CAST_THICK"].ToDecimal(), tpssm11["PONO"].ToString() }; // 定义参数列表的数组
						//		CMessageFormat::Format(s.msg, "连铸机[{0}]不能浇注钢坯类型[{1}]且断面为[{2}]的PONO[{3}]。", arguments, 4);
						//		CMessageFormat::Format(s.sysmsg, "连铸机[{0}]不能浇注钢坯类型[{1}]且断面为[{2}]的PONO[{3}]。", arguments, 4);
						//		throw CApplicationException(-1, s.msg, log.Location);
						//	}
						//}

						//if (v_slab_thick_max != tpssmd9["CAST_THICK"].ToDecimal())
						//{
						//	CFormattable arguments[] = { tpssm11["PONO"].ToString(), v_slab_thick_max, tpssmd9["CAST_THICK"].ToDecimal() }; // 定义参数列表的数组
						//	CMessageFormat::Format(s.msg, "炉次[{0}]下的浇注厚度[{1}]与铸机断面不符[{2}]。", arguments, 3);
						//	CMessageFormat::Format(s.sysmsg, "炉次[{0}]下的浇注厚度[{1}]与铸机断面不符[{2}]。", arguments, 3);
						//	throw CApplicationException(-1, s.msg, log.Location);
						//}

						//if ((v_slab_width_max > tpssmd9["CAST_WIDTH_MAX"].ToDecimal())
						//	|| (v_slab_width_min < tpssmd9["CAST_WIDTH_MIN"].ToDecimal()))
						//{
						//	CFormattable arguments[] = { tpssm11["PONO"].ToString(), v_slab_width_max, v_slab_width_min, tpssmd9["CAST_WIDTH_MAX"].ToDecimal(), tpssmd9["CAST_WIDTH_MIN"].ToDecimal()}; // 定义参数列表的数组
						//	CMessageFormat::Format(s.msg, "炉次[{0}]下的浇注宽度[{1}]-[{2}]与铸机断面不符[{3}]-[{4}]。", arguments, 5);
						//	CMessageFormat::Format(s.sysmsg, "炉次[{0}]下的浇注宽度[{1}]-[{2}]与铸机断面不符[{3}]-[{4}]。", arguments, 5);
						//	throw CApplicationException(-1, s.msg, log.Location);
						//}
					}

					//if (tpssm12["AREA_ID"].ToDecimal().ToInt32() == 5)
					//{

					//}

					//新增
					tpssm12.TrimOrBlank();
					sqlstr = "tpssm12.Insert()";
					tpssm12.Insert();

				}
				else //当前工序存在，则修改(逻辑上就是charge_no == tpssm11["CURR_WP_NO"].ToDecimal())
				{

					/*----------------------------------------------
					//当前工序修改，有2种情形：
					//1.当前工序只是开始，可以修改结束时间
					//2.当前工序在客户端没开始，而后台已接收运转信号，则不能调整（由于前台未刷新问题）
					//----------------------------------------------*/

					//判断设备是否改变，尤其单联变双联法时，设备没变
					if (tpssm12_old["DEV_CODE"].ToString() != tpssm12["DEV_CODE"].ToString() || tpssm12_old["AREA_ID"].ToDecimal() != tpssm12["AREA_ID"].ToDecimal())
					{
						CFormattable arguments[] = { tpssm11["PONO"].ToString(), tpssm12_old["DEV_CODE"].ToString(), tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "炉次[{0}]下的设备[{1}]的作业已开始或者已经接管，不能调整到[{2}]。", arguments, 3);
						CMessageFormat::Format(s.sysmsg, "刷新下数据，重新编制.", arguments, 3);
						//throw CApplicationException(-1, s.msg, log.Location);
						continue;

					}



					//实际工序开始生产时刻早于计划而且计划未刷新时候,需要按照前台传送的工序处理时间长度来更新结束时刻
					if (tpssm12_old["START_TIME_REAL"].ToString().Trim() != "" && tpssm12_old["END_TIME_REAL"].ToString().Trim() == "")
					{
						//proc_time = tpssm12["PROC_TIME"].ToDecimal().ToDouble();

						start_time = CDateTime::Parse(tpssm12_old["START_TIME_REAL"].ToString()); //用实际
						//tpssm12["END_TIME"] = start_time.Add(proc_time).ToString("yyyyMMddHHmmss");
						end_time = start_time.Add(proc_time);
						tpssm12["END_TIME"] = end_time.ToString("yyyyMMddHHmmss");
						tpssm12["CHARGE_NO"] = charge_no;
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["REC_REVISE_TIME"] = dateNow14;
						tpssm12["REC_REVISOR"] = CString(s.userid);

						sqlstr = "tpssm12.Update(END_TIME)";
						tpssm12.Update(
							"END_TIME,"//PROC_TIME 开始以后不更新处理时间 20231107 wcy
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO");

					}

					if (tpssm12_old["START_TIME_REAL"].ToString().Trim() == "" && tpssm12_old["END_TIME_REAL"].ToString().Trim() == "")//只有计划只有包到时间 20231107 wcy
					{
						//proc_time = tpssm12["PROC_TIME"].ToDecimal().ToDouble();

						//start_time = CDateTime::Parse(tpssm12_old["START_TIME_REAL"].ToString()); //用实际
						//tpssm12["END_TIME"] = start_time.Add(proc_time).ToString("yyyyMMddHHmmss");
						//end_time = start_time.Add(proc_time);
						//tpssm12["END_TIME"] = end_time.ToString("yyyyMMddHHmmss");
						tpssm12["CHARGE_NO"] = charge_no;
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["REC_REVISE_TIME"] = dateNow14;
						tpssm12["REC_REVISOR"] = CString(s.userid);

						sqlstr = "tpssm12.Update(END_TIME)";
						tpssm12.Update(
							"START_TIME,END_TIME,PROC_TIME,"
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO");
					}

				}//if 原工序计划是否存在

				/*if (tpssm12["AREA_ID"].ToDecimal().ToInt32() == 5)
				{
					v_steel_end_time = tpssm12["END_TIME_REAL"].ToString() == " " ? tpssm12["END_TIME"].ToString() : tpssm12["END_TIME_REAL"].ToString();
				}
				else if (tpssm12["AREA_ID"].ToDecimal().ToInt32() == 3)
				{
					v_steel_start_time = tpssm12["START_TIME_REAL"].ToString() == " " ? tpssm12["START_TIME"].ToString() : tpssm12["START_TIME_REAL"].ToString();
				}*/


			}//for 循环读取输入

			/////崩溃校验/////
			tpssm12_check.Reset();
			tpssm12_check["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_check["AREA_ID"] = 3;
			if (tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") != 1)
			{
				CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") };
				CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在脱碳区域[{1}]上的设备数量有误[{2}]", arguments, 3);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				// 碳钢保留bof 不锈钢保留AOD 20240509 14:30，lxx//一定要有A或者B,钢种不论 20240516 初亮
				tpssm12_check.Query("SM_PLAN_NO,AREA_ID");
				/*sqlstr = "  SELECT C_DIV FROM TPSSM10  WHERE PONO = @pono  ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("pono", tpssm11["PONO"].ToString());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
				cs_c_div = cmd.GetString(1);
				}
				cmd.Close();*/
				if (tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "B" && tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "A")
				{
					CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check["DEV_CODE"].ToString() };
					CMessageFormat::Format(s.msg, "计划[{0}]已经存在脱碳区域[{1}]上的设备有误[{2}]，应为BOF或者AOD", arguments, 3);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			tpssm12_check.Reset();
			tpssm12_check["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_check["AREA_ID"] = 5;
			if (tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") != 1)
			{
				CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check.QueryCount("AREA_ID,SM_PLAN_NO") };
				CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在连铸区域[{1}]上的设备数量有误[{2}]", arguments, 3);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				tpssm12_check.Query("SM_PLAN_NO,AREA_ID");
				if (tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "C" && tpssm12_check["DEV_CODE"].ToString().Substring(0, 1) != "M")
				{
					CFormattable arguments[] = { v_sm_plan_no, tpssm12_check["AREA_ID"].ToString(), tpssm12_check["DEV_CODE"].ToString() };
					CMessageFormat::Format(s.msg, "实绩信号已经接收，请刷新计划再操作  备注：计划[{0}]已经存在连铸区域[{1}]上的设备有误[{2}]", arguments, 3);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			/////////////////

			//----------------------------------
			//主计划相关信息修改

			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:         // MS SQL Server数据库
			//case DB_KIND_ORACLE:        // Oracle 数据库
			//default:  // 所有数据库适用，通用SQL语句
			//	sqlstr = CString(
			//		"SELECT DECODE(TRIM(START_TIME_REAL), NULL, START_TIME, START_TIME_REAL) START_TIME FROM TPSSM12"
			//		" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
			//		"   AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO"
			//		"   AND area_id = 3 "
			//		);
			//	break;
			//}
			//cmd.SetCommandText(sqlstr);
			//cmd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			//cmd.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			//cmd.ExecuteReader();
			//if (cmd.Read())
			//{
			//	tpssm11["STEEL_START_TIME"] = cmd.GetString(1);
			//}
			//cmd.Close();

			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:         // MS SQL Server数据库
			//case DB_KIND_ORACLE:        // Oracle 数据库
			//default:  // 所有数据库适用，通用SQL语句
			//	sqlstr = CString(
			//		"SELECT DECODE(TRIM(END_TIME_REAL), NULL, END_TIME, END_TIME_REAL) END_TIME FROM TPSSM12 "
			//		" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
			//		"   AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO"
			//		"   AND area_id = 5 "
			//		);
			//	break;
			//}
			//cmd.SetCommandText(sqlstr);
			//cmd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			//cmd.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			//cmd.ExecuteReader();
			//if (cmd.Read())
			//{
			//	tpssm11["STEEL_END_TIME"] = cmd.GetString(1);
			//}
			//cmd.Close();

			tpssm11["BACKLOG_EA"] = v_backlog_ea.TrimOrBlank();
			tpssm11["REFINE_ROUTE_CODE"] = v_refine_route.TrimOrBlank();
			/*tpssm11["STEEL_START_TIME"] = v_steel_start_time;
			tpssm11["STEEL_END_TIME"] = v_steel_end_time;

			Log::Info("", __FUNCTION__, " STEEL_START_TIME=[{0}]", v_steel_start_time);
			Log::Info("", __FUNCTION__, " STEEL_END_TIME=[{0}]", v_steel_end_time);*/

			sqlstr = "tpssm11.Update()";
			tpssm11.Update(
				"DE_SULFUR_FLAG,"        //脱S指示			
				"BACKLOG_EA,"
				"REFINE_ROUTE_CODE,"
				"CC_MACH_NO",  //连铸机
				"FACTORY_DIV,SM_PLAN_NO");//"STEEL_START_TIME,""STEEL_END_TIME",

		}
		else//wcy 日平衡逻辑
		{
			CModel tpssmd9("TPSSMD9");
			CModel tpssm15("TPSSM15");
			CModel tpssm16("TPSSM16");
			CModel tpssm16_old("TPSSM16");//原工序计划信息
			//---------------------------------------------------
			//获得输入参数
			//1、主计划信息, 单记录
			blkseq = bcls_rec->Tables.IndexOf("TPSSM15");
			if (blkseq < 0)
			{
				//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
				sprintf(s.msg, "没有找到主计划数据块[TPSSM15]，请联系系统维护人员。");
				sprintf(s.sysmsg, "TABLE [TPSSM15] NOT EXIST in f_pssm12_save_job_n().");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"].ToString().Trim();
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[0]["SM_PLAN_NO"].ToString().Trim();
			Log::Info("", __FUNCTION__, " factory_div=[{0}]", v_factory_div);
			Log::Info("", __FUNCTION__, " sm_plan_no=[{0}]", v_sm_plan_no);
			//读取主计划信息
			tpssm15["FACTORY_DIV"] = v_factory_div;
			tpssm15["SM_PLAN_NO"] = v_sm_plan_no;
			sqlstr = "tpssm15.Query()";
			bool has15 = tpssm15.Query("FACTORY_DIV,SM_PLAN_NO");
			if (has15 == false) //计划表中没有该炉次，新增
			{
				CFormattable arguments[] = { tpssm15["SM_PLAN_NO"].ToString(), tpssm15["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "编制的炉次[{0}],计划号[{1}]在出钢计划中不存在。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Info("", __FUNCTION__, " CURR_WP_NO=[{0}]", tpssm15["CURR_WP_NO"].ToDecimal());


			//----------------------------------
			//甘特图编辑后的子工序计划，只要未开始的，就无条件覆盖
			tpssm16["FACTORY_DIV"] = v_factory_div;
			tpssm16["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm16["HEAT_NO"] = tpssm15["HEAT_NO"];
			tpssm16["SM_PLAN_NOL2"] = tpssm15["SM_PLAN_NOL2"];

			//1)删除前保存当前炉次的所有工序计划信息
			//tb_tpssm16.Clear();
			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:	        // MS SQL Server数据库
			//case DB_KIND_ORACLE:        // Oracle 数据库
			//default: // 所有数据库适用，通用SQL语句
			//	sqlstr = CString(
			//		" SELECT * FROM TPSSM16 "
			//		"  WHERE FACTORY_DIV = @tpssm16.FACTORY_DIV "
			//		"    AND SM_PLAN_NO = @tpssm16.SM_PLAN_NO "
			//		"    AND SUB_CHARGE_NO = 0 "  //0-主工序，甘特图只算这个
			//		"  ORDER BY CHARGE_NO ASC "
			//		);
			//	break;
			//}
			//cmd.SetCommandText(sqlstr);
			//cmd.Parameters.Set("tpssm16.FACTORY_DIV", tpssm16["FACTORY_DIV"].ToString().Trim());
			//cmd.Parameters.Set("tpssm16.SM_PLAN_NO", tpssm16["SM_PLAN_NO"].ToString().Trim());
			//cmd.ExecuteQuery(tb_tpssm16);

			//2）删除当前炉次的未开始工序计划
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" DELETE TPSSM16 "
					" WHERE FACTORY_DIV = @tpssm16.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm16.SM_PLAN_NO "
					"   AND CHARGE_NO  > @tpssm15.CURR_WP_NO "  //1-脱硫
					"   AND ARRIVE_REAL_TIME  = ' ' "     //wcy  重要，会导致保存就把包到时间全刷了 在一查询 预处理号都更新了
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("tpssm16.FACTORY_DIV", tpssm16["FACTORY_DIV"].ToString().Trim());
			cmd.Parameters.Set("tpssm16.SM_PLAN_NO", tpssm16["SM_PLAN_NO"].ToString().Trim());
			cmd.Parameters.Set("tpssm15.CURR_WP_NO", tpssm15["CURR_WP_NO"].ToDecimal());
			cmd.ExecuteNonQuery();


			//---------------------------------------------------
			//2、循环读取浇铸信息，多记录
			blkseq = bcls_rec->Tables.IndexOf("TPSSM16");
			if (blkseq < 0)
			{
				//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
				sprintf(s.msg, "没有找到工序计划数据块[TPSSM16]，请联系系统维护人员。");
				sprintf(s.sysmsg, "TABLE [TPSSM16] NOT EXIST in f_pssm12_save_job_n().");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			rows = bcls_rec->Tables[blkseq].Rows.get_Count();
			for (i = 0; i < rows; i++)
			{
				//tpssm16.MergeFrom(bcls_rec->Tables[0].Rows[i]);
				tpssm16["AREA_ID"] = bcls_rec->Tables[blkseq].Rows[i]["AREA_ID"].ToDecimal(); //区域
				tpssm16["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["DEV_CODE"].ToString().Trim(); //设备
				tpssm16["START_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["START_TIME"].ToString().Trim(); //开始时刻
				tpssm16["END_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["END_TIME"].ToString().Trim();    //结束时刻

				start_time = CDateTime::Parse(tpssm16["START_TIME"].ToString());
				end_time = CDateTime::Parse(tpssm16["END_TIME"].ToString());
				proc_time = end_time - start_time;
				tpssm16["PROC_TIME"] = proc_time.TotalMinutes();

				////Log::Info("", __FUNCTION__, " AREA_ID=[{0}] DEV_CODE=[{1}] START_TIME=[{2}] END_TIME=[{3}]",
				//tpssm16["AREA_ID"].ToDecimal(), tpssm16["DEV_CODE"].ToString(), tpssm16["START_TIME"].ToString(), tpssm16["END_TIME"].ToString());


				//charge_no赋值, 必须从1开始
				charge_no = i + 1;

				//1)查询设备配置信息
				tpssmd1["FACTORY_DIV"] = tpssm16["FACTORY_DIV"];
				tpssmd1["DEV_CODE"] = tpssm16["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm16["AREA_ID"];
				//sqlstr = "tpssmd1.Query()";
				//bool hasd1 = tpssmd1.Query("FACTORY_DIV,AREA_ID,DEV_CODE");
				//if (hasd1 == false) //没查询到记录
				//{
				//	CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "区域[{0}]下的设备[{1}]配置信息读取不到，请联系系统维护人员。", arguments, 2); //格式化字符串
				//	CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 2);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm16["FACTORY_DIV"];
				TPSSMD1_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm16["DEV_CODE"];
				TPSSMD1_CONDI.Tables[0].Rows[0]["AREA_ID"] = tpssm16["AREA_ID"];
				TPSSMD1_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);

				if (TPSSMD1_RESULT.Tables[0].Rows.get_Count() != 1)
				{
					CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "区域[{0}]下的设备[{1}]配置信息读取不到，请联系系统维护人员。", arguments, 2); //格式化字符串
					CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

				//拼接钢区工艺途径
				v_backlog_ea = v_backlog_ea + tpssmd1["STATION_ID"].ToString().Trim();

				//有脱硫，主计划的脱硫指示置"1"
				if (tpssmd1["AREA_ID"].ToDecimal() == 1)	tpssm15["DE_SULFUR_FLAG"] = "1";
				else tpssm15["DE_SULFUR_FLAG"] = " ";

				//有预溶液，子计划预溶液指示标记置"1" 20231107 wcy
				if (tpssmd1["AREA_ID"].ToDecimal() == 2 && (tpssmd1["STATION_ID"].ToString().Trim() == "X" || tpssmd1["STATION_ID"].ToString().Trim() == "Y" || tpssmd1["STATION_ID"].ToString().Trim() == "Z"))	tpssm16["PRE_SOLUTION_FLAG"] = "1";
				else tpssm16["PRE_SOLUTION_FLAG"] = " ";

				//有脱P，必然是双联法，主计划的冶炼模式置"2"
				//if (tpssmd1["AREA_ID"].ToDecimal() == 2)	tpssm15["SMELT_MODE"] = 2;  //2-双联
				//else tpssm15["SMELT_MODE"] = 1;


				//确定出钢计划的精炼路径
				if (tpssm16["AREA_ID"].ToDecimal() == 4)
				{
					v_refine_route = v_refine_route + tpssm16["DEV_CODE"].ToString();
				}



				//跳过比当前工序小的charge
				if (charge_no < tpssm15["CURR_WP_NO"].ToDecimal())  continue;


				//读取原工序计划信息
				tpssm16_old["FACTORY_DIV"] = v_factory_div;
				tpssm16_old["SM_PLAN_NO"] = v_sm_plan_no;
				tpssm16_old["CHARGE_NO"] = charge_no;
				tpssm16_old["SUB_CHARGE_NO"] = 0;
				sqlstr = "tpssm16_old.Query()";
				bool has_16 = tpssm16_old.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"); //用主键查询

				if (has_16 == false) //不存在，则新增
				{
					tpssm16["CHARGE_NO"] = charge_no;
					tpssm16["SUB_CHARGE_NO"] = 0;

					tpssm16["REC_CREATE_TIME"] = dateNow14;
					tpssm16["REC_CREATOR"] = CString(s.userid);

					//判断是否换机浇铸
					if (tpssm16["AREA_ID"].ToDecimal().ToInt32() == 5 && tpssm16["DEV_CODE"].ToString() != tpssm16_old["DEV_CODE"].ToString().Trim())
					{
						//罪过啊，换机浇铸太恐怖，一堆数据要更新
						tpssm15["CC_MACH_NO"] = tpssmd1["STATION_NO"];

						//if (tpssmd1["STATION_ID"].ToString() == "C")
						//{
						//	switch (conn->DatabaseKind)
						//	{
						//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
						//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
						//	case DB_KIND_ORACLE:	    // Oracle 数据库
						//	default:
						//		sqlstr = "SELECT MAX(SLAB_THICK), MIN(SLAB_THICK), "
						//			"  MAX(SLAB_WIDTH), MIN(SLAB_WIDTH), MAX(BILLET_TYPE) "
						//			"  FROM TPSSM03 "
						//			" WHERE FACTORY_DIV = @tpssm15.FACTORY_DIV "
						//			"   AND PONO = @tpssm15.PONO ";
						//		break;
						//	}
						//	cmd_sql.SetCommandText(sqlstr);
						//	cmd_sql.Parameters.Clear();
						//	cmd_sql.Parameters.Set("tpssm15.FACTORY_DIV", tpssm15["FACTORY_DIV"].ToString());
						//	cmd_sql.Parameters.Set("tpssm15.PONO", tpssm15["PONO"].ToString());
						//	cmd_sql.ExecuteReader();
						//	if (cmd_sql.Read())
						//	{
						//		v_slab_thick_max = cmd_sql.GetDecimal(1);
						//		v_slab_thick_min = cmd_sql.GetDecimal(2);
						//		v_slab_width_max = cmd_sql.GetDecimal(3);
						//		v_slab_width_min = cmd_sql.GetDecimal(4);
						//		v_billet_type = cmd_sql.GetString(5);
						//	}
						//	cmd_sql.Close();

						//	tpssmd9["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
						//	tpssmd9["BILLET_TYPE"] = v_billet_type;
						//	tpssmd9["CAST_THICK"] = v_slab_thick_max;
						//	tpssmd9["FACTORY_DIV"] = tpssm15["FACTORY_DIV"];
						//	if (tpssmd9.Query("FACTORY_DIV,CC_MACH_NO, BILLET_TYPE, CAST_THICK") == false)
						//	{
						//		CFormattable arguments[] = { tpssmd9["CC_MACH_NO"].ToString(), tpssmd9["BILLET_TYPE"].ToString(), tpssmd9["CAST_THICK"].ToDecimal(), tpssm15["PONO"].ToString() }; // 定义参数列表的数组
						//		CMessageFormat::Format(s.msg, "连铸机[{0}]不能浇注钢坯类型[{1}]且断面为[{2}]的PONO[{3}]。", arguments, 4);
						//		CMessageFormat::Format(s.sysmsg, "连铸机[{0}]不能浇注钢坯类型[{1}]且断面为[{2}]的PONO[{3}]。", arguments, 4);
						//		throw CApplicationException(-1, s.msg, log.Location);
						//	}
						//}

						//if (v_slab_thick_max != tpssmd9["CAST_THICK"].ToDecimal())
						//{
						//	CFormattable arguments[] = { tpssm15["PONO"].ToString(), v_slab_thick_max, tpssmd9["CAST_THICK"].ToDecimal() }; // 定义参数列表的数组
						//	CMessageFormat::Format(s.msg, "炉次[{0}]下的浇注厚度[{1}]与铸机断面不符[{2}]。", arguments, 3);
						//	CMessageFormat::Format(s.sysmsg, "炉次[{0}]下的浇注厚度[{1}]与铸机断面不符[{2}]。", arguments, 3);
						//	throw CApplicationException(-1, s.msg, log.Location);
						//}

						//if ((v_slab_width_max > tpssmd9["CAST_WIDTH_MAX"].ToDecimal())
						//	|| (v_slab_width_min < tpssmd9["CAST_WIDTH_MIN"].ToDecimal()))
						//{
						//	CFormattable arguments[] = { tpssm15["PONO"].ToString(), v_slab_width_max, v_slab_width_min, tpssmd9["CAST_WIDTH_MAX"].ToDecimal(), tpssmd9["CAST_WIDTH_MIN"].ToDecimal()}; // 定义参数列表的数组
						//	CMessageFormat::Format(s.msg, "炉次[{0}]下的浇注宽度[{1}]-[{2}]与铸机断面不符[{3}]-[{4}]。", arguments, 5);
						//	CMessageFormat::Format(s.sysmsg, "炉次[{0}]下的浇注宽度[{1}]-[{2}]与铸机断面不符[{3}]-[{4}]。", arguments, 5);
						//	throw CApplicationException(-1, s.msg, log.Location);
						//}
					}

					//if (tpssm16["AREA_ID"].ToDecimal().ToInt32() == 5)
					//{

					//}

					//新增
					tpssm16.TrimOrBlank();
					sqlstr = "tpssm16.Insert()";
					tpssm16.Insert();

				}
				else //当前工序存在，则修改(逻辑上就是charge_no == tpssm15["CURR_WP_NO"].ToDecimal())
				{

					/*----------------------------------------------
					//当前工序修改，有2种情形：
					//1.当前工序只是开始，可以修改结束时间
					//2.当前工序在客户端没开始，而后台已接收运转信号，则不能调整（由于前台未刷新问题）
					//----------------------------------------------*/

					//判断设备是否改变，尤其单联变双联法时，设备没变
					if (tpssm16_old["DEV_CODE"].ToString() != tpssm16["DEV_CODE"].ToString() || tpssm16_old["AREA_ID"].ToDecimal() != tpssm16["AREA_ID"].ToDecimal())
					{
						CFormattable arguments[] = { tpssm15["PONO"].ToString(), tpssm16_old["DEV_CODE"].ToString(), tpssm16["DEV_CODE"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "炉次[{0}]下的设备[{1}]的作业已开始或者已经接管，不能调整到[{2}]。", arguments, 3);
						CMessageFormat::Format(s.sysmsg, "刷新下数据，重新编制.", arguments, 3);
						throw CApplicationException(-1, s.msg, log.Location);
					}


					//实际工序开始生产时刻早于计划而且计划未刷新时候,需要按照前台传送的工序处理时间长度来更新结束时刻
					if (tpssm16_old["START_TIME_REAL"].ToString().Trim() != "" && tpssm16_old["END_TIME_REAL"].ToString().Trim() == "")
					{
						//proc_time = tpssm16["PROC_TIME"].ToDecimal().ToDouble();

						start_time = CDateTime::Parse(tpssm16_old["START_TIME_REAL"].ToString()); //用实际
						//tpssm16["END_TIME"] = start_time.Add(proc_time).ToString("yyyyMMddHHmmss");
						end_time = start_time.Add(proc_time);
						tpssm16["END_TIME"] = end_time.ToString("yyyyMMddHHmmss");
						tpssm16["CHARGE_NO"] = charge_no;
						tpssm16["SUB_CHARGE_NO"] = 0;
						tpssm16["REC_REVISE_TIME"] = dateNow14;
						tpssm16["REC_REVISOR"] = CString(s.userid);

						sqlstr = "tpssm16.Update(END_TIME)";
						tpssm16.Update(
							"END_TIME,"//PROC_TIME 开始以后不更新处理时间 20231107 wcy
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO");

					}

					if (tpssm16_old["START_TIME_REAL"].ToString().Trim() == "" && tpssm16_old["END_TIME_REAL"].ToString().Trim() == "")//只有计划只有包到时间 20231107 wcy
					{
						//proc_time = tpssm16["PROC_TIME"].ToDecimal().ToDouble();

						//start_time = CDateTime::Parse(tpssm16_old["START_TIME_REAL"].ToString()); //用实际
						//tpssm16["END_TIME"] = start_time.Add(proc_time).ToString("yyyyMMddHHmmss");
						//end_time = start_time.Add(proc_time);
						//tpssm16["END_TIME"] = end_time.ToString("yyyyMMddHHmmss");
						tpssm16["CHARGE_NO"] = charge_no;
						tpssm16["SUB_CHARGE_NO"] = 0;
						tpssm16["REC_REVISE_TIME"] = dateNow14;
						tpssm16["REC_REVISOR"] = CString(s.userid);

						sqlstr = "tpssm16.Update(END_TIME)";
						tpssm16.Update(
							"START_TIME,END_TIME,PROC_TIME,"
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO");

					}

				}//if 原工序计划是否存在


			}//for 循环读取输入


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
					"SELECT DECODE(TRIM(START_TIME_REAL), NULL, START_TIME, START_TIME_REAL) START_TIME FROM TPSSM16"
					" WHERE FACTORY_DIV = @tpssm15.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm15.SM_PLAN_NO"
					"   AND area_id = 3 "
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("tpssm15.FACTORY_DIV", tpssm15["FACTORY_DIV"].ToString());
			cmd.Parameters.Set("tpssm15.SM_PLAN_NO", tpssm15["SM_PLAN_NO"].ToString());
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				tpssm15["STEEL_START_TIME"] = cmd.GetString(1);
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
					"SELECT DECODE(TRIM(END_TIME_REAL), NULL, END_TIME, END_TIME_REAL) END_TIME FROM TPSSM16 "
					" WHERE FACTORY_DIV = @tpssm15.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm15.SM_PLAN_NO"
					"   AND area_id = 5 "
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("tpssm15.FACTORY_DIV", tpssm15["FACTORY_DIV"].ToString());
			cmd.Parameters.Set("tpssm15.SM_PLAN_NO", tpssm15["SM_PLAN_NO"].ToString());
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				tpssm15["STEEL_END_TIME"] = cmd.GetString(1);
			}
			cmd.Close();

			tpssm15["BACKLOG_EA"] = v_backlog_ea.TrimOrBlank();
			tpssm15["REFINE_ROUTE_CODE"] = v_refine_route.TrimOrBlank();

			sqlstr = "tpssm15.Update()";
			tpssm15.Update(
				"DE_SULFUR_FLAG,"        //脱S指示			
				"BACKLOG_EA,"
				"REFINE_ROUTE_CODE,"
				"CC_MACH_NO,"        //连铸机
				"STEEL_START_TIME,"
				"STEEL_END_TIME",
				"FACTORY_DIV,SM_PLAN_NO");
		}

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
