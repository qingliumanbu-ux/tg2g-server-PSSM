/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    1.0
Date:      2015-04-09
Description:	出钢计划获取开浇时刻。
**************************************************************************************************************/
#include "stdafx.h"

//程序用头文件






/*<remark>=========================================================
/// <summary>
/// 出钢计划获取开浇时刻
/// <para>根据连铸计划预测时刻，获取开浇时刻，并重新计算结束时刻。</para>
/// <para>1.依据当前出钢计划各炉次，获取TPSSM10表开浇时刻；   </para>
/// <para>2.计算各时刻：</para>
/// <para> 1)由开浇时刻 -> 浇铸结束时刻；</para>
/// <para> 2)由浇完时刻 -> 包离开时刻；</para>
/// <para> 3)由开浇时刻 -> 包到达时刻；</para>
/// <para>数据库表：TPSSM11(炼钢出钢计划主表)</para>
/// <para>主调用函数：f_pssm11_cast()</para>
/// </summary>
/// <param name="PONO">制造命令号</param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm11t_get_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	//int i, rows, blkseq;
	int k = 0;

	CDecimal diff_time = 0;
	//CString ccm_no("");          //当前连铸机号
	//CString prev_end_time = "";    //上一炉浇铸完时刻，作为下炉开浇时刻的计算基本时刻
	//CString prev_is_new = "";      //上一炉是否是新增炉：N-新增; O-已有
	//char dest_time[15] = "";
	CDateTime tmp_time;
	CString v_factory_div = "";
	CString v_cc_req_time = "";
	CString end_time_pre = "";
	int fetchRowCount = 0;

	CString sqlstr = "";
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd9("TPSSMD9");

	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);	

	try
	{
		//-----------------------------------------------------------------------
		//获得输入参数
		//全出钢计划，无参数
		v_factory_div = bcls_rec->Tables["POUR"].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_cc_req_time = bcls_rec->Tables["POUR"].Rows[0]["CC_REQ_TIME"];
		//----------------------------------------------------------
		//查询炼钢作业计划表(TPSSM11)中所有炉次，都计算一下。
		sqlstr = CString(
			" SELECT b.CC_REQ_TIME, b.CC_REQ_TIME_FLAG, b.CC_PREP_TIME, b.POUR_TIME, a.* "
			"   FROM TPSSM11 a, TPSSM10 b "
			"  WHERE a.FACTORY_DIV = b.FACTORY_DIV "
			"	 AND a.PLAN_EDIT_FLAG IN ( 'N', 'U') "
			"	 AND a.CC_REQ_TIME = ' '  "
			"	 AND a.PONO = b.PONO "
			"	 AND a.FACTORY_DIV = @v_factory_div"
			"	 AND a.RUN_STATUS < '53' "
			"  ORDER BY a.CAST_NO, a.CAST_DIV_NO ASC "
			);
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm11_inq.ExecuteReader();

		while ( cmd_tpssm11_inq.Read() )
		{
			k = 1;

			tpssm10["CC_REQ_TIME"]  = cmd_tpssm11_inq.GetString(k++).Trim();
			tpssm10["CC_REQ_TIME_FLAG"] = cmd_tpssm11_inq.GetString(k++).Trim();
			tpssm10["CC_PREP_TIME"] = cmd_tpssm11_inq.GetDecimal(k++).ToInt32();
			tpssm10["POUR_TIME"]    = cmd_tpssm11_inq.GetDecimal(k++).ToInt32();

			cmd_tpssm11_inq.Fetch(tpssm11, k);

			//tpssm11.TrimOrBlank();

			//指定开浇时刻的计算（跟随PONO写入表中 ）
			////Log::Trace("", __FUNCTION__, "---- pono=[{0}], pono_status=[{1}], cc_req_time=[{2}], PREP_TIME=[{3}], POUR_TIME=[{4}] ----",
			//	tpssm11["PONO"].ToString(), tpssm11["PONO_STATUS"].ToDecimal(), tpssm10["CC_REQ_TIME"].ToString(), tpssm10["CC_PREP_TIME"].ToDecimal(), tpssm10["POUR_TIME"].ToDecimal());

				
			//查询当前炉次的浇铸工序计划信息
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["SUB_CHARGE_NO"] = 0;
			tpssm12["AREA_ID"] = 5;  //浇铸
			sqlstr = "tpssm12.Query()";
			bool has12 = tpssm12.Query("FACTORY_DIV, SM_PLAN_NO,SUB_CHARGE_NO, AREA_ID");
			if (has12 == false)
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]的连铸工序信息不存在。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}


			//浇完的炉次跳过
			if (tpssm11["PONO_STATUS"].ToDecimal().ToInt32() == 83) //83-浇完
			{
				continue;
			}

			//读取连铸设备参数表
			if (tpssmd9["CC_MACH_NO"].ToString() != tpssm11["CC_MACH_NO"].ToString().Trim()) //没有读取过,第一炉。
			{
				tpssmd9["FACTORY_DIV"] = tpssm11["FACTORY_DIV"].ToString().Trim();
				tpssmd9["CC_MACH_NO"] = tpssm11["CC_MACH_NO"].ToString().Trim();
				sqlstr = "tpssmd9.Query()";
				bool hasc1 = tpssmd9.Query();
				if (hasc1 == false)
				{
					tpssmd9["STRAND_NUM"] = 2;  //都是2流连铸
					tpssmd9["TT_PREP_2CH"] = 4; //默认CAST内时2炉间隔4分钟
					tpssmd9["TT_PREP_LAST_2CH"] = 10;  //默认10分钟
					tpssmd9["TT_PREP_W0_CAST"] = 80; //默认CAST间时2炉间隔80分钟
					//或报错
					//CFormattable arguments[] = { tpssm10["CC_MACH_NO"].ToString() }; // 定义参数列表的数组
					//CMessageFormat::Format(s.msg, "浇铸时间计算中，没有找到铸机[{0}]的设备参数配置。", arguments, 1);
					//throw CApplicationException(-1, s.msg, log.Location);
				}

				////Log::Trace("", __FUNCTION__, "cc_mach_no=[{0}], strand_num=[{1}], PREP_W0==[{2}]",
					//tpssm11["CC_MACH_NO"].ToString(), tpssmd9["STRAND_NUM"].ToDecimal(), tpssmd9["TT_PREP_W0_CAST"].ToDecimal());

				tpssm12["START_TIME"] = v_cc_req_time;

			}
			else
			{
				////Log::Info("", __FUNCTION__, "计算第[{0}]炉", fetchRowCount);

				diff_time = tpssm12["PREP_TIME"];

				////Log::Info("", __FUNCTION__, "准备时间 = [{0}]", tpssm12["PREP_TIME"].ToDecimal());
				////Log::Info("", __FUNCTION__, "end_time_pre = [{0}]", end_time_pre);

				//根据上一炉的结束时刻和准备时间, 计算本炉的开浇时刻
				tmp_time = CDateTime::Parse(end_time_pre);
				tpssm12["START_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");
			}


			//---------------------------------------------------------------
			//获取各处理时间
			tpssm12["PREP_TIME"] = tpssm10["CC_PREP_TIME"].ToDecimal().ToInt32();   //炉间准备时间

			//2015-6-11 xuwen 修改    //第2炉准备时间 + 第2炉直前准备
			tpssm12["PRE_PROC_TIME"] = tpssmd9["TT_PREP_2CH"].ToDecimal() + tpssmd9["TT_PREP_LAST_2CH"].ToDecimal(); //连铸前处理时间（推算包到时刻用）


			//推算时刻
			//如果炉次已开浇，计算结束时刻作为基准，推算后续炉次开浇时刻
			if (tpssm11["PONO_STATUS"].ToDecimal().ToInt32() == 82) //82-开浇; 83-浇完
			{
				if (tpssm12["START_TIME_REAL"].ToString().Trim() == "") tmp_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
				else tmp_time = CDateTime::Parse(tpssm12["START_TIME_REAL"].ToString());

				//根据开始时刻推算结束时刻
				diff_time = tpssm12["PROC_TIME"];
				tpssm12["END_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");

				end_time_pre = tpssm12["END_TIME"];

				//更新当前炉次主表的结束时刻
				tpssm11["STEEL_END_TIME"] = tpssm12["END_TIME"];
				sqlstr = "tpssm11.Update(STEEL_END_TIME)";
				tpssm11.Update("STEEL_END_TIME", "FACTORY_DIV, SM_PLAN_NO");

				////Log::Trace("", __FUNCTION__, "开浇pono=[{0}]: END_TIME=[{1}]", tpssm11["PONO"].ToString(), tpssm12["END_TIME"].ToString());

				sqlstr = "tpssm12.Update()-82";
				tpssm12.Update(
					" PREP_TIME"       //准备时间
					",PRE_PROC_TIME"   //前处理时间
					",PROC_TIME"       //处理时间
					",LADLE_ARRIVE_TIME"  //包到
					",START_TIME"         //开浇
					",END_TIME"           //浇完
					",LADLE_LEAVE_TIME",  //包离
					"FACTORY_DIV, SM_PLAN_NO, AREA_ID");

				continue;
			}


			//------------------------------------------
			//推算（未开浇炉次时刻刷新及计算）
			//1.开浇时刻
			//tpssm12["START_TIME"] = tpssm10["CC_REQ_TIME"];

			//2.根据本炉次开浇时刻, 计算浇完时刻
			diff_time = tpssm12["PROC_TIME"];
			tmp_time  = CDateTime::Parse(tpssm12["START_TIME"].ToString());
			tpssm12["END_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");

			end_time_pre = tpssm12["END_TIME"];
			//////Log::Trace("", __FUNCTION__, "tpssm12["END_TIME"] =[{0}]", tpssm12["END_TIME"].ToString());
			

			//3.根据开浇时刻, 计算包到达时刻
			diff_time = tpssm12["PRE_PROC_TIME"];   //前处理时间
			tmp_time = CDateTime::Parse(tpssm12["START_TIME"].ToString());
			tpssm12["LADLE_ARRIVE_TIME"] = tmp_time.AddMinutes(-diff_time.ToDouble()).ToString("yyyyMMddHHmmss");


			//4.根据浇完时刻, 计算包离开时刻
			diff_time = tpssm12["POST_PROC_TIME"];  //后处理时间
			tmp_time = CDateTime::Parse(tpssm12["END_TIME"].ToString());
			tpssm12["LADLE_LEAVE_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");

			////Log::Trace("", __FUNCTION__,  "CC: Arrive=[{0}], Start=[{1}], End=[{2}], Leave=[{3}]", 
				//tpssm12["LADLE_ARRIVE_TIME"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["LADLE_LEAVE_TIME"].ToString());


			//将计算的结果写入计划子表中
			//tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			//tpssm12["AREA_ID"] = 5;
			sqlstr = "tpssm12.Update()";
			tpssm12.Update(
				" PREP_TIME"       //准备时间
				",PRE_PROC_TIME"   //前处理时间
				",PROC_TIME"       //处理时间
				",LADLE_ARRIVE_TIME"  //包到
				",START_TIME"         //开浇
				",END_TIME"           //浇完
				",LADLE_LEAVE_TIME",  //包离
				"FACTORY_DIV, SM_PLAN_NO, AREA_ID");


			//修改计划主表的cc_req_time，炼钢完成时刻
			if (tpssm10["CC_REQ_TIME_FLAG"].ToString() == "1") //指定开浇时刻
			{
				tpssm11["CC_REQ_TIME"] = tpssm10["CC_REQ_TIME"];    //CC要求时刻是用户指定(有合理判断)，千万不要写入计算的值
			}
			else
			{
				tpssm11["CC_REQ_TIME"] = tpssm12["START_TIME"];
			}

			tpssm11["STEEL_END_TIME"] = tpssm12["END_TIME"];
			sqlstr = "tpssm11.Update(CC_REQ_TIME)";
			tpssm11.Update(
				"CC_REQ_TIME,"
				"STEEL_END_TIME",
				"FACTORY_DIV, SM_PLAN_NO");


		}//while
		cmd_tpssm11_inq.Close();

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
