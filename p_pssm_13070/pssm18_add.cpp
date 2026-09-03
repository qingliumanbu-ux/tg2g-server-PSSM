/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-26
Version:  3.1.0
Description: 甘特图出钢计划编制保存
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

int f_pssm_call_tps_n3(CString factory_div, int mode, EIClass inblockadd, EIClass & outblockadd, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划编制保存
/// <para>根据Client甘特图输入的数据，修改出钢计划。  </para>
/// <para>
///   1.出钢计划信息写入:主计划与工序计划
///   2.删除排除的炉次
///   3.CAST号计算
///   4.计划号计算
///   5.处理号计算
/// <para>数据库表：TPSSM11/12                   </para>
/// <para>主调用函数：PSSM18画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>制造命令号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_add)

int f_pssm18_add(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	CString v_pono = "";
	CString v_restrand_flg = "";     //连浇标记
	CString v_cc_req_time = "";  //开浇时刻
	CString v_tpd_start_time = "";  //倒罐开始时刻
	CDecimal v_td_chg_flg = 0, v_smelt_mode = 0;
	CDecimal charge_no = 0;      //工序charge号
	CDecimal area_id = 0;
	CString datetime = "";
	CString routebagkey = "";
	CString routelist = "";
	CString backlog_ea = "";
	CString ref_route = "";
	CString dev_start = "";
	CString dev_end = "";
	CString station_id = "";
	CString station_no = "";
	CString cast_no = "";
	CDecimal cast_no_div = 0;

	CDateTime start_timex;
	CDateTime end_timex;
	CDecimal diff_time = 0;
	CTimeSpan proc_time_dif;

	EIClass inblock;
	EIClass outblock;
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm18_del(conn);
	CDbCommand cmd_tpssm11_inq(conn);

	CDataTable tb_tpssm11("TPSSM11");

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		CModel tpssm10("TPSSM10");
		CModel tpssm11("TPSSM11");
		CModel tpssm12("TPSSM12");
		CModel tpssm12_ds("TPSSM12");//脱硫
		CModel tpssm12_sd("TPSSM12");//预溶液
		CModel tpssm12_dp("TPSSM12");//转炉脱磷
		CModel tpssm12_bof("TPSSM12");//转炉/电炉工序
		CModel tpssm12_sr("TPSSM12");//精炼
		CModel tpssm12_cc("TPSSM12");//连铸
		CModel tpssm18("TPSSM18");//设备状态
		CModel tpssm99("TPSSM99");
		CModel tpssmd1("TPSSMD1");
		CModel tpssmd6("TPSSMD6");
		//--------------------------------
		//定义函数调用信息结构
		//1、总体计划块
		
		inblock.Tables[0].set_TableName("PLAN");  //tpssm11 tpssm12
		inblock.Tables[0].Columns.Add(DT_STRING, "PONO");
		inblock.Tables[0].Columns.Add(DT_STRING, "BACKLOG_EA");//路径设备区分
		//inblock.Tables[0].Columns.Add(DT_STRING, "ROUTE_CONTACT");//路径关联关系？？传入为00000
		inblock.Tables[0].Columns.Add(DT_STRING, "ROUTE_DEV_TECH_CODE");//设备类型区分
		inblock.Tables[0].Columns.Add(DT_STRING, "CC_REQ_TIME");//CC要求时刻
		inblock.Tables[0].Columns.Add(DT_STRING, "CAST_NO");//浇次号
		inblock.Tables[0].Columns.Add(DT_STRING, "CAST_DIV_NO");//浇次分割号
		//inblock.Tables[0].Columns.Add(DT_STRING, "ROUTEBAGKEY");//工艺路径包
		inblock.Tables[0].Columns.Add(DT_STRING, "ROUTELIST");//工艺路径
		//inblock.Tables[0].Columns.Add(DT_STRING, "WORK_DEV");//正在处理的工位 模型暂时不读这2个字段？？
		//inblock.Tables[0].Columns.Add(DT_STRING, "RUN_STATUS");//炉次状态 模型暂时不读这2个字段？？
		inblock.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");//工序设备
		inblock.Tables[0].Columns.Add(DT_DECIMAL, "PREP_TIME");//工序准备时间
		inblock.Tables[0].Columns.Add(DT_DECIMAL, "MOVE_TIME");//上工序至本工序传搁时间
		inblock.Tables[0].Columns.Add(DT_DECIMAL, "PROC_TIME");//工序处理时间
		inblock.Tables[0].Columns.Add(DT_STRING, "START_TIME");//计划开始时间
		inblock.Tables[0].Columns.Add(DT_STRING, "END_TIME");//计划结束时间
		inblock.Tables[0].Columns.Add(DT_STRING, "START_TIME_REAL");//实绩开始时间
		inblock.Tables[0].Columns.Add(DT_STRING, "END_TIME_REAL");//实绩结束时间

		/*	in_pssm18.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");*/
		/* -----------输入参数 说明 ---------------------------------
		甘特图是以一条记录将一炉次的所有计划内容传入
		pono
		heat_no
		st_no
		cc_mark
		plan_style
		td_chg_flg
		refine_route_code  精炼路径
		pono_status        PONO状态
		cc_req_time        ??????? 甘特图有指定？
		tpd_id             倒罐
		tpd_start_time     倒罐开始
		tpd_end_time       倒罐结束
		kr_id              脱S 工位
		kr_start_time
		kr_end_time
		sd_1_id              脱硫工位（新增）
		sd_1_wait_start_time
		sd_1_end_time
		sd_2_id              预溶液工位（新增）
		sd_2_wait_start_time
		sd_2_end_time
		ld_1_id            转炉脱P 工位
		ld_1_wait_start_time
		ld_1_end_time
		ld_2_id            转炉脱C/电炉 工位
		ld_2_wait_start_time
		ld_2_end_time
		finery_1_id        精炼1 工位
		finery_1_start_time
		finery_1_end_time");
		finery_2_id        精炼2 工位
		finery_2_start_time
		finery_2_end_time
		finery_3_id        精炼3 工位
		finery_3_start_time
		finery_3_end_time
		finery_4_id        精炼4 工位
		finery_4_start_time
		finery_4_end_time
		cast_1_wait_id     连铸等待工位
		cast_1_wait_start_time
		cast_1_wait_end_time
		cast_1_id          连铸工位
		cast_1_start_time
		cast_1_end_time
		steel_return_code  返送代码
		sg_sign
		routebagkey 工艺路线包
		routelist 工艺路线
		*--------------------------------------------------------*/


		//-----------------------------------------------------
		// 读取输入信息，单记录
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in pssm21_save().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];

		//在做出钢计划保存前，当前出钢计划置删除标记"D"
		tpssm10["FACTORY_DIV"] = v_factory_div;

		/* ***** 获取输入参数 ***** */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rows; i++)
		{
			backlog_ea = "00";
			ref_route = "0";
			cast_no = "";
			cast_no_div = 0;
			v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();
			tpssm10["PONO"] = v_pono;
			tpssm10.Query("PONO,FACTORY_DIV");
			v_cc_req_time = bcls_rec->Tables[0].Rows[i]["CC_REQ_TIME"].ToString().Trim();
			if (v_cc_req_time.Trim() == "") v_cc_req_time = "00000000000000";
			if (bcls_rec->Tables[0].Columns.Contains("ROUTEBAGKEY"))
			{
				routebagkey = bcls_rec->Tables[0].Rows[i]["ROUTEBAGKEY"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("ROUTELIST"))
			{
				routelist = bcls_rec->Tables[0].Rows[i]["ROUTELIST"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("SD_1_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["SD_1_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_1_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 1;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("SD_2_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["SD_2_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_2_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 2;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("LD_1_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["LD_1_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_1_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 2;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("LD_2_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["LD_2_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_2_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 3;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("FINERY_1_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["FINERY_1_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 4;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("FINERY_2_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["FINERY_2_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 4;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("FINERY_3_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["FINERY_3_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 4;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("FINERY_4_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["FINERY_4_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 4;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("CAST_1_ID"))
			{
				backlog_ea = backlog_ea + bcls_rec->Tables[0].Rows[i]["CAST_1_ID"].ToString().Trim();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["CAST_1_ID"].ToString().Trim();
				tpssmd1["AREA_ID"] = 5;
				if (tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID"))
				{
					ref_route = ref_route + tpssmd1["DEV_TECH_CODE"].ToString().Trim();
				}
			}
			/* ***** 检查输入参数合法性 ***** */
			if (v_pono.GetLength() <= 0)
			{
				doFlag = -11;
				//sprintf(s.msg, "收到的PONO号[%s]长度有误！",(const char*)tpssm11["PONO"].ToString());
				CFormattable arguments[] = { v_pono }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//-------------------------------------------------------------------
			//工序计划处理: 整理输入工序计划数据项
			charge_no = 0;
			dev_start = "";
			dev_end = "";
			diff_time = 0;


			CDataRow& row = inblock.Tables["PLAN"].Rows.Add();
			row["PONO"] = v_pono;
			row["BACKLOG_EA"] = backlog_ea;
			row["ROUTE_DEV_TECH_CODE"] = ref_route;
			row["CC_REQ_TIME"] = "00000000000000";
			row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();;
			row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
			row["ROUTELIST"] = routelist;

			row["DEV_CODE"] = "00";
			row["PREP_TIME"] = 0;
			row["MOVE_TIME"] = 0;
			row["PROC_TIME"] = 0;
			row["START_TIME"] = "00000000000000";
			row["END_TIME"] = "00000000000000";
			row["START_TIME_REAL"] = "00000000000000";
			row["END_TIME_REAL"] = "00000000000000";
			//------------------------
			//1、脱硫工序（甘特图单独工序管理）
			if (bcls_rec->Tables[0].Columns.Contains("SD_1_ID"))
			{
				tpssm12_ds["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_1_ID"].ToString().Trim();
				tpssm12_ds["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_1_WAIT_START_TIME"].ToString().Trim();
				tpssm12_ds["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_1_END_TIME"].ToString().Trim();
				if (tpssm12_ds["DEV_CODE"].ToString().Trim() != "")  //有脱硫设备，则走该工序
				{
					charge_no = charge_no + 1;  //指定charge号
					area_id = 1;
					start_timex = CDateTime::Parse(tpssm12_ds["START_TIME"].ToString());
					end_timex = CDateTime::Parse(tpssm12_ds["END_TIME"].ToString());
					proc_time_dif = end_timex - start_timex;
					diff_time = proc_time_dif.TotalMinutes();
					dev_end = tpssm12_ds["DEV_CODE"].ToString();

					if (dev_start.Trim() != "" && dev_end.Trim() != "")
					{
						tpssmd1["FACTORY_DIV"] = v_factory_div;
						tpssmd1["DEV_CODE"] = dev_end;
						tpssmd1["AREA_ID"] = area_id;
						tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

						tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
						tpssmd6["DEV_MOVE_START"] = station_id + station_no;
						tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

						if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
						{
							tpssmd6["MOVE_TIME"] = 0;
						}
						else
						{
							tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
						}
					}
					
					CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
					row["PONO"] = v_pono;
					row["BACKLOG_EA"] = backlog_ea;
					row["ROUTE_DEV_TECH_CODE"] = ref_route;
					row["CC_REQ_TIME"] = v_cc_req_time;
					row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
					row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
					row["ROUTELIST"] = routelist;
					row["DEV_CODE"] = tpssm12_ds["DEV_CODE"].ToString();   //1-脱硫
					row["PREP_TIME"] = 0;
					row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
					row["PROC_TIME"] = diff_time;
					row["START_TIME"] = tpssm12_ds["START_TIME"].ToString();
					row["END_TIME"] = tpssm12_ds["END_TIME"].ToString();
					row["START_TIME_REAL"] = "00000000000000";
					row["END_TIME_REAL"] = "00000000000000";

					dev_start = tpssm12_ds["DEV_CODE"].ToString();
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_start;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					station_id = tpssmd1["STATION_ID"];
					station_no = tpssmd1["STATION_NO"];
				}
			}
			Log::Trace("", __FUNCTION__, "脱硫({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_ds["DEV_CODE"].ToString(), tpssm12_ds["START_TIME"].ToString(), tpssm12_ds["END_TIME"].ToString(), charge_no);

			//------------------------
			//1.5、预溶液工序
			if (bcls_rec->Tables[0].Columns.Contains("SD_2_ID")){
				tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_2_ID"].ToString().Trim();
				tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_2_WAIT_START_TIME"].ToString().Trim();
				tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_2_END_TIME"].ToString().Trim();
				//v_smelt_mode = 1;
				if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预溶液，则走该工序
				{
					charge_no = charge_no + 1;
					area_id = 2;
					start_timex = CDateTime::Parse(tpssm12_sd["START_TIME"].ToString());
					end_timex = CDateTime::Parse(tpssm12_sd["END_TIME"].ToString());
					proc_time_dif = end_timex - start_timex;
					diff_time = proc_time_dif.TotalMinutes();
					dev_end = tpssm12_sd["DEV_CODE"].ToString();

					if (dev_start.Trim() != "" && dev_end.Trim() != "")
					{
						tpssmd1["FACTORY_DIV"] = v_factory_div;
						tpssmd1["DEV_CODE"] = dev_end;
						tpssmd1["AREA_ID"] = area_id;
						tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

						tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
						tpssmd6["DEV_MOVE_START"] = station_id + station_no;
						tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

						if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
						{
							tpssmd6["MOVE_TIME"] = 0;
						}
						else
						{
							tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
						}
					}

					CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
					row["PONO"] = v_pono;
					row["BACKLOG_EA"] = backlog_ea;
					row["ROUTE_DEV_TECH_CODE"] = ref_route;
					row["CC_REQ_TIME"] = v_cc_req_time;
					row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
					row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
					row["ROUTELIST"] = routelist;
					row["DEV_CODE"] = tpssm12_sd["DEV_CODE"].ToString();  
					row["PREP_TIME"] = 0;
					row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
					row["PROC_TIME"] = diff_time;
					row["START_TIME"] = tpssm12_sd["START_TIME"].ToString();
					row["END_TIME"] = tpssm12_sd["END_TIME"].ToString();
					row["START_TIME_REAL"] = "00000000000000";
					row["END_TIME_REAL"] = "00000000000000";

					dev_start = tpssm12_sd["DEV_CODE"].ToString();
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_start;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					station_id = tpssmd1["STATION_ID"];
					station_no = tpssmd1["STATION_NO"];
				}
			}
			Log::Trace("", __FUNCTION__, "预溶液({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sd["DEV_CODE"].ToString(), tpssm12_sd["START_TIME"].ToString(), tpssm12_sd["END_TIME"].ToString(), charge_no);

			//------------------------
			//2、转炉脱P工序（甘特图没有）
			tpssm12_dp["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_1_ID"].ToString().Trim();
			tpssm12_dp["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_WAIT_START_TIME"].ToString().Trim();
			tpssm12_dp["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_END_TIME"].ToString().Trim();
			v_smelt_mode = 1;
			if (tpssm12_dp["DEV_CODE"].ToString().Trim() != "")  //有脱P设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 2;
				start_timex = CDateTime::Parse(tpssm12_dp["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_dp["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_dp["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_dp["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_dp["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_dp["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_dp["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			Log::Trace("", __FUNCTION__, "脱P ({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_dp["DEV_CODE"].ToString(), tpssm12_dp["START_TIME"].ToString(), tpssm12_dp["END_TIME"].ToString(), charge_no);


			//------------------------
			//3、转炉脱C、电炉工序
			tpssm12_bof["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_2_ID"].ToString().Trim();
			tpssm12_bof["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_2_WAIT_START_TIME"].ToString().Trim();
			tpssm12_bof["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_2_END_TIME"].ToString().Trim();
			if (tpssm12_bof["DEV_CODE"].ToString().Trim() != "")  //有转炉设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 3;
				start_timex = CDateTime::Parse(tpssm12_bof["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_bof["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_bof["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_bof["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_bof["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_bof["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_bof["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			else
			{
				CFormattable arguments[] = { v_pono, tpssm12_bof["DEV_CODE"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划炉次[{0}]的转炉/电炉设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			Log::Trace("", __FUNCTION__, "转炉({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_bof["DEV_CODE"].ToString(), tpssm12_bof["START_TIME"].ToString(), tpssm12_bof["END_TIME"].ToString(), charge_no);


			//------------------------
			//4、精炼1
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 4;
				start_timex = CDateTime::Parse(tpssm12_sr["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_sr["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_sr["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_sr["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_sr["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_sr["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			Log::Trace("", __FUNCTION__, "精炼1({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


			//------------------------
			//5、精炼2
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼2设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 4;
				start_timex = CDateTime::Parse(tpssm12_sr["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_sr["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_sr["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_sr["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_sr["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_sr["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			Log::Trace("", __FUNCTION__, "精炼2({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


			//------------------------
			//6、精炼3
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 4;
				start_timex = CDateTime::Parse(tpssm12_sr["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_sr["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_sr["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_sr["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_sr["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_sr["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			Log::Trace("", __FUNCTION__, "精炼3({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


			//------------------------
			//7、精炼4
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 4;
				start_timex = CDateTime::Parse(tpssm12_sr["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_sr["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_sr["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_sr["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_sr["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_sr["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			Log::Trace("", __FUNCTION__, "精炼4({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);

			//如果多于4重精炼，在此处扩展。


			//------------------------
			//8、连铸、模铸工序
			tpssm12_cc["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["CAST_1_ID"].ToString().Trim();
			tpssm12_cc["START_TIME"] = bcls_rec->Tables[0].Rows[i]["CAST_1_START_TIME"].ToString().Trim();
			tpssm12_cc["END_TIME"] = bcls_rec->Tables[0].Rows[i]["CAST_1_END_TIME"].ToString().Trim();
			if (tpssm12_cc["DEV_CODE"].ToString().Trim() != "")  //有浇铸设备，则走该工序
			{
				charge_no = charge_no + 1;
				area_id = 5;
				start_timex = CDateTime::Parse(tpssm12_cc["START_TIME"].ToString());
				end_timex = CDateTime::Parse(tpssm12_cc["END_TIME"].ToString());
				proc_time_dif = end_timex - start_timex;
				diff_time = proc_time_dif.TotalMinutes();
				dev_end = tpssm12_cc["DEV_CODE"].ToString();

				if (dev_start.Trim() != "" && dev_end.Trim() != "")
				{
					tpssmd1["FACTORY_DIV"] = v_factory_div;
					tpssmd1["DEV_CODE"] = dev_end;
					tpssmd1["AREA_ID"] = area_id;
					tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

					tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd6["DEV_MOVE_START"] = station_id + station_no;
					tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

					if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
					{
						tpssmd6["MOVE_TIME"] = 0;
					}
					else
					{
						tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
					}
				}

				CDataRow &row = inblock.Tables["PLAN"].Rows.Add();
				row["PONO"] = v_pono;
				row["BACKLOG_EA"] = backlog_ea;
				row["ROUTE_DEV_TECH_CODE"] = ref_route;
				row["CC_REQ_TIME"] = v_cc_req_time;
				row["CAST_NO"] = tpssm10["CAST_LOT_NO"].ToString().Trim();
				row["CAST_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
				row["ROUTELIST"] = routelist;
				row["DEV_CODE"] = tpssm12_cc["DEV_CODE"].ToString();
				row["PREP_TIME"] = 0;
				row["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
				row["PROC_TIME"] = diff_time;
				row["START_TIME"] = tpssm12_cc["START_TIME"].ToString();
				row["END_TIME"] = tpssm12_cc["END_TIME"].ToString();
				row["START_TIME_REAL"] = "00000000000000";
				row["END_TIME_REAL"] = "00000000000000";

				dev_start = tpssm12_cc["DEV_CODE"].ToString();
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = dev_start;
				tpssmd1["AREA_ID"] = area_id;
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
			}
			else
			{
				CFormattable arguments[] = { v_pono, tpssm12_cc["DEV_CODE"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划炉次[{0}]的浇铸设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			Log::Trace("", __FUNCTION__, "连铸({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_cc["DEV_CODE"].ToString(), tpssm12_cc["START_TIME"].ToString(), tpssm12_cc["END_TIME"].ToString(), charge_no);

		}		

		int mode = 2;
		ret = f_pssm_call_tps_n3(v_factory_div, mode, inblock,  outblock, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		bcls_ret->Tables[0].Copy(outblock.Tables[0]);

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
