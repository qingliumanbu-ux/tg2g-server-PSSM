/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-24
Description: 出钢计划炉次条件修改
**************************************************/
#include "stdafx.h"



//#include "tpssmd4.h"
//#include "tpssmd5.h"
   //修改连铸处理时间用




/*<remark>=========================================================
/// <summary>
/// 出钢计划炉次条件修改函数
/// <para>修改内容包括: 工序调整、设备调整、时间调整，开浇时刻调整</para>
/// <para>1.工序调整: 增加或删除脱磷工序, 增加、调整或删除精炼路径。</para>
/// <para>2.设备调整: 各工序设备的调整                        </para>
/// <para>1.调整预冶炼（脱磷）工序时，检查计划是否进入生产。根据前台
///要求，新增或删除该工序，并更新冶炼模式和设备信息。
/// </para>
/// <para>对非精炼工序, 调整的内容是设备工位号的更新。          </para>
/// <para>对精炼工序，存在增减精炼工序，调整的内容包括：
///钢区工艺途径、精炼路径、设备工位号的更新。                   </para>
/// <para>数据库表：TPSSMD1/11/12(炼钢出钢计划表)               </para>
/// <para>主调用函数：前台PSSM12P(炉次条件)画面F3(修改)调用。</para>
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm12_plan_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blkseq, rows, i=0;

	/* 业务变量 */
	CString v_sm_plan_no = "";	//炼钢计划号
	CString v_factory_div = "";
	CString date_time = "";
	CString item_11 = "";   //出钢计划主数据修改项
	CString sr_route = "";      //新精炼路径
	CString backlog_ea = "";    //钢区工序途径
	CDecimal sm_charge_no = 0;  //转炉的charge号
	CDecimal sr_charge_no = 0;  //精炼的charge号
	CDecimal pre_proc_time = 0; //输入的前处理时间
	CDecimal proc_time = 0;     //输入的处理时间
	CDecimal post_proc_time = 0;//输入的后处理时间

	CString plan_edit_flag = " "; //4-设备调整；5 - 路径调整；

	CString sqlstr = "";

	CDbCommand cmd(conn);
	CDbCommand cmd_upd(conn);

	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		// 定义表的实体对象
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
		//CTPSSMD4 tpssmd4(conn);
		//CTPSSMD5 tpssmd5(conn);
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm11_old("TPSSM11");//原计划信息
	CModel tpssm12("TPSSM12");
	CModel tpssm12_old("TPSSM12");//原工序计划信息


		//---------------------------------------------------
		//获得输入参数
		//循环读取浇铸信息，多记录
		blkseq = bcls_rec->Tables.IndexOf("TPSSM_CONST");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到炉次条件信息数据块[TPSSM_CONST]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [TPSSM_CONST] NOT EXIST in pssm12_upd().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"];
			////Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}]", v_sm_plan_no);

			//=========================================================
			//前台传入参数列表：（与客户端 dsPSSM11.TPSSM_CONST 或 TPSSM12A 一致）
			//  CC_REQ_TIME   //开浇时刻
			//设备号
			//  DS_DEV    //脱硫设备号
			//	DP_DEV    //脱磷转炉号
			//	SM_DEV    //转炉号
			//	SR1_DEV	  //精炼1
			//	SR2_DEV	  //精炼2
			//	SR3_DEV	  //精炼3
			//	SR4_DEV	  //精炼4
			//	CC_MACH_NO	 //连铸机号   无CC_DEV，需转换
			//作业时间
			//	DS_PROC_TIME	//铁水预处理（脱S）处理时间
			//	DP_LOAD_TIME	//转炉(脱P)装入时间
			//	DP_PROC_TIME	//转炉(脱P)处理时间
			//	DP_TAP_TIME     //转炉(脱P)出钢时间
			//	SM_LOAD_TIME    //转炉(脱C)装入时间
			//	SM_PROC_TIME	//转炉(脱C)熔炼时间    "SM_MELT_TIME"  //炼钢(脱C)冶炼炼开始
			//  SM_TAP_TIME     //炼钢(脱C)出钢时间
			//	SR1_PROC_TIME   //第一重精炼处理时间
			//	SR2_PROC_TIME
			//	SR3_PROC_TIME
			//	SR4_PROC_TIME
			//	CC_PROC_TIME    //连模铸处理时间
			//休辅时间
			//	DS_REST_TIME   //脱硫休辅时间
			//	DP_REST_TIME   //炼钢(脱P)休辅时间
			//	SM_REST_TIME   //炼钢(脱C)休辅时间
			//	SR1_REST_TIME  //精炼1休辅时间
			//	SR2_REST_TIME
			//	SR3_REST_TIME
			//	SR4_REST_TIME
			//	CC_REST_TIME   //连铸休辅时间
			//=========================================================

			tpssm11.Reset();
			tpssm11_old.Reset();
			tpssm12.Reset();
			tpssm12_old.Reset();
			sr_route = "";
			backlog_ea = "";
			item_11 = "";


			//读取原出钢计划信息
			tpssm11_old["FACTORY_DIV"] = v_factory_div;
			tpssm11_old["SM_PLAN_NO"] = v_sm_plan_no;
			sqlstr = "tpssm11_old.Query()";
			bool has11 = tpssm11_old.Query("FACTORY_DIV, SM_PLAN_NO");
			if (has11 == false)
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]不存在, 请查询更新后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//---------------------------------------------------
			//读取主计划修改项
			tpssm11["SMELT_MODE"] = bcls_rec->Tables[blkseq].Rows[i]["SMELT_MODE"].ToDecimal();
			//重开浇标记（RESTRAND_FLAG），在浇铸顺画面中操作，炉次条件不做修改
			//tpssm11.RESTRAND_FLAG = bcls_rec->Tables[blkseq].Rows[i]["RESTRAND_FLAG"].ToString().Trim();
			tpssm11["TD_CHG_FLG"] = bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"].ToDecimal();
			tpssm11["INS_FE_FLAG"] = bcls_rec->Tables[blkseq].Rows[i]["INS_FE_FLAG"];


			//主计划修改项检查
			//冶炼模式
			if (tpssm11["SMELT_MODE"].ToDecimal() != tpssm11_old["SMELT_MODE"].ToDecimal())//有不同，做检查
			{
				if (tpssm11_old["SMELT_MODE"].ToDecimal() == 2 && tpssm11_old["PONO_STATUS"].ToDecimal() > 20) //原计划是双联，脱P已开始状态
				{
					CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "炼钢计划号[{0}]脱P已开始, 不能改为双联。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (tpssm11_old["SMELT_MODE"].ToDecimal() != 2 && tpssm11_old["PONO_STATUS"].ToDecimal() > 30) //原计划是双联，脱P已开始状态
				{
					CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "炼钢计划号[{0}]转炉已开始, 不能修改转炉模式。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			//换中包
			if (tpssm11["TD_CHG_FLG"].ToDecimal() != tpssm11_old["TD_CHG_FLG"].ToDecimal())  //有不同，做修改
			{
				item_11 = item_11 + "TD_CHG_FLG,";
				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇; 83-浇完
				{
					CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "炼钢计划号[{0}]已开浇, 不能做换中包操作。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			//插铁板
			if (tpssm11["INS_FE_FLAG"].ToString().Trim() != tpssm11_old["INS_FE_FLAG"].ToString().Trim())  //有不同，做修改
			{
				item_11 = item_11 + "INS_FE_FLAG,";
				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇; 83-浇完
				{
					CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "炼钢计划号[{0}]已开浇, 不能做插铁板操作。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			

			//1. 开浇时刻调整
			//tpssm11["CC_REQ_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["CC_REQ_TIME"].ToString().Trim();
			//////Log::Info("", __FUNCTION__, "CC_REQ_TIME：new=[{0}], old=[{1}]", tpssm11["CC_REQ_TIME"].ToString(), tpssm11_old["CC_REQ_TIME"].ToString());

			//if (tpssm11["CC_REQ_TIME"].ToString().Trim() != tpssm11_old["CC_REQ_TIME"].ToString().Trim())  //有不同，做修改
			//{
			//	item_11 = item_11 + "CC_REQ_TIME,";
			//}




			//工序计划修改项处理
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12["REC_CREATOR"] = CString(s.userid);
			tpssm12["REC_CREATE_TIME"] = date_time;
			tpssm12["REC_REVISOR"] = CString(s.userid);
			tpssm12["REC_REVISE_TIME"] = date_time;

			//----------------------------------------------------------------------------
			//1、脱硫工序
			// 湛江不设脱硫工序，如下计算逻辑不会走进去
			tpssm12["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString().Trim(); //设备
			tpssm12["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["DS_DEV"].ToString().Trim(); //设备
			proc_time        = bcls_rec->Tables[blkseq].Rows[i]["DS_PROC_TIME"].ToDecimal().ToInt32(); //处理时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["DS_REST_TIME"].ToDecimal().ToInt32(); //休辅时间
			////Log::Info("", __FUNCTION__, "脱S：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());

			//读取原脱硫工序计划信息
			tpssm12_old["AREA_ID"] = 1;  //1-铁水处理
			tpssm12_old["SUB_CHARGE_NO"] = 0;
			sqlstr = "tpssm12_old.Query()-DS";
			bool has_ds = tpssm12_old.Query("FACTORY_DIV, SM_PLAN_NO, SUB_CHARGE_NO, AREA_ID");

			//判断脱硫工序设备是否为空，空-表示不走脱硫，有值-表示走脱硫
			if (tpssm12["DEV_CODE"].ToString() == "")
			{
				if (has_ds == true) //原来有脱硫，则删除
				{
					////主计划的脱硫指示置"1"
					//item_11 = item_11 + "DE_SULFUR_FLAG,";
					//tpssm11["DE_SULFUR_FLAG"] = " ";

					//删除
					sqlstr = "tpssm12_old.Delete()-DS";
					tpssm12_old.Delete();  //默认按主键删除

					//后续各工序CHARGE_NO调整(减一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO - 1 "  //各工序减一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND AREA_ID    > 1 "  //1-脱硫
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
					cmd.ExecuteNonQuery();


					plan_edit_flag = "5"; //5 - 路径调整；
				}
				else //没有，什么都不做
				{
				}

			}
			else  //修改或新增脱硫工序
			{

				backlog_ea = "K";

				if (has_ds == true) //原来有脱硫，则修改
				{
					tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
					tpssm12["SUB_CHARGE_NO"] = tpssm12_old["SUB_CHARGE_NO"];
					tpssm12["PROC_TIME"] = proc_time;

					//修改
					sqlstr = "tpssm12.Update()-DS";
					tpssm12.Update(
						"DEV_CODE,"
						"PROC_TIME,"
						"REST_TIME,"
						"REC_REVISOR,REC_REVISE_TIME",
						"FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, SUB_CHARGE_NO"  //按主键修改
						);

					if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
						plan_edit_flag = "6"; //6 - 时刻调整；
					else
						plan_edit_flag = "4"; //4-设备调整；

				}
				else //没有，则新增
				{
					////主计划的脱硫指示置"1"
					//item_11 = item_11 + "DE_SULFUR_FLAG,";
					//tpssm11["DE_SULFUR_FLAG"] = "1";

					//脱硫后续各工序CHARGE_NO调整(加一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO + 1 "  //各工序加一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND AREA_ID    > 1 "  //1-脱硫
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
					cmd.ExecuteNonQuery();

					//新增脱硫工序（此处可改进为函数）
					tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"]  = 1;  //脱S一般是第一道工序
					tpssm12["SUB_CHARGE_NO"] = 0;
					//tpssm12.PONO = tpssm11_old["PONO"];
					tpssm12["AREA_ID"] = 1;
					tpssm12["PROC_TIME"] = proc_time;
					tpssm12["PREP_TIME"] = 0;  //	准备时间

					//新增
					sqlstr = "tpssm12.Insert()-DS";
					tpssm12.Insert();

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}
			}


			//----------------------------------------------------------------------------
			//2、转炉脱P工序
			tpssm12["DEV_CODE"]	= bcls_rec->Tables[blkseq].Rows[i]["DP_DEV"].ToString().Trim(); //设备
			pre_proc_time  = bcls_rec->Tables[blkseq].Rows[i]["DP_LOAD_TIME"].ToDecimal().ToInt32(); //装入时间  目前画面不设置
			proc_time      = bcls_rec->Tables[blkseq].Rows[i]["DP_PROC_TIME"].ToDecimal().ToInt32(); //脱P 时间
			post_proc_time = bcls_rec->Tables[blkseq].Rows[i]["DP_TAP_TIME"].ToDecimal().ToInt32(); //出钢时间  目前画面不设置
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["DP_REST_TIME"].ToDecimal();     //休辅时间
			////Log::Info("", __FUNCTION__, "脱P：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());

			//读取原脱P工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["AREA_ID"] = 2;  //2-转炉脱P
			tpssm12_old["SUB_CHARGE_NO"] = 2;  //读取处理工序
			sqlstr = "tpssm12_old.Query()-DP";
			bool has_dp = tpssm12_old.Query("FACTORY_DIV, SM_PLAN_NO, SUB_CHARGE_NO, AREA_ID");

			//脱P工序调整判断
			if (tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString().Trim() )
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };

				if (tpssm12["DEV_CODE"].ToString() == "" && tpssm11_old["PONO_STATUS"].ToDecimal() > 20) //21-脱磷装入
				{
					CMessageFormat::Format(s.msg, "炼钢计划号[{0}]已转炉脱磷作业，不能删除脱磷工序计划。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (tpssm12["DEV_CODE"].ToString() != "" && tpssm11_old["PONO_STATUS"].ToDecimal() > 30) //31-脱C装入
				{
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]的转炉已作业，不能增加脱磷工序计划。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			//判断脱P工序设备是否为空，空-表示不走脱P，有值-表示走脱P
			if (tpssm12["DEV_CODE"].ToString() == "")
			{

				//主计划的冶炼模式修改
				item_11 = item_11 + "SMELT_MODE,";
				if (tpssm11["SMELT_MODE"].ToDecimal() == 2) tpssm11["SMELT_MODE"] = 1; //如果客户端指定的是双联，则强制改为“1-普通”，因为未指定设备


				if (has_dp == true) //原来有脱P，则删除
				{
					////判断计划状态，已转炉开始，不能删除脱P
					//if (tpssm11_old["PONO_STATUS"].ToDecimal() > 20)
					//{
					//	CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
					//	CMessageFormat::Format(s.msg, "炼钢计划号[{0}]已转炉脱磷作业，不能删除脱磷工序计划。", arguments, 2);
					//	throw CApplicationException(-1, s.msg, log.Location);
					//}

					//删除
					sqlstr = "tpssm12_old.Delete()-DP";
					tpssm12_old.Delete("FACTORY_DIV, SM_PLAN_NO, CHARGE_NO");

					//后续各工序CHARGE_NO调整(减一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO - 1 "  //各工序减一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND AREA_ID    > 2 "  //2-脱P
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
					cmd.ExecuteNonQuery();


					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}
				else //没有，什么都不做
				{
				}


			}
			else  //修改或新增脱P工序
			{

				backlog_ea = backlog_ea + "P";  //脱P工序

				//主计划的冶炼模式修改
				item_11 = item_11 + "SMELT_MODE,";
				tpssm11["SMELT_MODE"] = 2; //2-双联法

				if (has_dp == true) //原来有脱P，则修改
				{
					//////Log::Trace("", __FUNCTION__, "proc_time=[{0}], old_proc_time=[{1}], PONO_STATUS=[{2}]",
					//	proc_time, tpssm12_old["PROC_TIME"].ToDecimal().ToInt32(), tpssm11_old["PONO_STATUS"].ToDecimal());

					//判断计划状态，已转炉开始，不能删除脱P
					if ((proc_time != tpssm12_old["PROC_TIME"].ToDecimal().ToInt32()) && tpssm11_old["PONO_STATUS"].ToDecimal() > 20)
					{
						CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "炼钢计划号[{0}]已转炉脱磷作业，不能修改脱磷时间。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
					tpssm12["SUB_CHARGE_NO"] = 0;  //主工序

					////对于转炉脱P，主工序的处理时间 = 装入 + 处理 + 出钢
					//tpssm12["PROC_TIME"] = pre_proc_time + proc_time + post_proc_time;
					//2015-11-18 修改为 主工序的处理时间 = 处理 + 出钢
					tpssm12["PROC_TIME"] = proc_time + post_proc_time;

					//修改主工序
					sqlstr = "tpssm12.Update()-DP-0";
					tpssm12.Update(
						"DEV_CODE,"
						//"PRE_PROC_TIME,"    //装入时间
						"PROC_TIME,"
						//"POST_PROC_TIME,"   //出钢时间
						"REST_TIME,"
						"REC_REVISOR,REC_REVISE_TIME",
						"FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, SUB_CHARGE_NO"  //按主键修改
						);

					//调整整体计划状态用
					if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
						plan_edit_flag = "6"; //6 - 时刻调整；
					else
						plan_edit_flag = "4"; //4-设备调整；


				}
				else //没有，则新增
				{

					//脱P后续各工序CHARGE_NO调整(加一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO + 1 "  //各工序加一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND AREA_ID    > 2 "  //2-脱P
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString() );
					cmd.ExecuteNonQuery();

					//--------------------------------------------------------
					//读取工序标准作业时间：工序设备标识 -> PATTERN -> 处理时间				
					//1)根据 设备代码取设备标识
					tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					sqlstr = "tpssmd1.Query()-DP";
					bool hasd1 = tpssmd1.Query("FACTORY_DIV, DEV_CODE");

					//2)根据 工序设备标识+出钢记号--> 处理时间
					tpssmd3["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssmd1["DEV_CODE"];  //脱P
					tpssmd3["ST_NO"] = tpssm11["ST_NO"];
					tpssmd3["SMELT_MODE"] = tpssm11["SMELT_MODE"];  //转炉工序要区分：1-单联; 3-双渣法; 2-双联;

					sqlstr = "tpssmd3.Query()-DP";
					bool hasd4 = tpssmd3.Query("FACTORY_DIV, ST_NO, DEV_CODE, SMELT_MODE");
					if (hasd4 == false) //读取不到数据
					{
						tpssmd3["STD_PROC_TIME"] = 40;
						tpssmd3["STD_PREP_TIME"] = 5; //准备时间(炉间)
						tpssmd3["FEED_TIME"] = 5;     //转炉/电炉用：废钢、铁水装入作业; 精炼、连铸的进站时间
						tpssmd3["DRAW_TIME"] = 5;     //转炉/电炉用：出钢作业; 精炼、连铸是离站时间; 
					}
					////Log::Trace("", __FUNCTION__, "DEV_CODE=[{0}], ST_NO=[{1}], SMELT_MODE=[{2}]",
						//tpssmd3["DEV_CODE"].ToString(), tpssmd3["ST_NO"].ToString(), tpssm11["SMELT_MODE"].ToDecimal());

					//新增脱P工序（此处可改进为函数）
					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					if (has_ds == true) //原来有脱硫，则脱P为2
					{
						tpssm12["CHARGE_NO"] = 2;  //脱S一般是第一道工序
					}
					else //没有脱S，脱P为第一道工序
					{
						tpssm12["CHARGE_NO"] = 1;  //
					}
					tpssm12["SUB_CHARGE_NO"] = 0;  //0-主工序
					tpssm12["SM_PLAN_NO"] = tpssm11_old["SM_PLAN_NO"];
					tpssm12["AREA_ID"] = 2;    //2-脱P工序

					if (pre_proc_time < 1)  pre_proc_time  = tpssmd3["FEED_TIME"];  //没有输入时取标准值
					if (proc_time < 1)      proc_time = tpssmd3["STD_PROC_TIME"];  //没有输入时取标准值
					if (post_proc_time < 1) post_proc_time = tpssmd3["DRAW_TIME"];  //没有输入时取标准值

					tpssm12["PRE_PROC_TIME"] = pre_proc_time;	    /* 前处理时间=0 */
					////对于转炉脱P，主工序的处理时间 = 装入 + 处理 + 出钢
					//tpssm12["PROC_TIME"] = pre_proc_time + proc_time + post_proc_time;      /* 处理时间=装入+处理+镇静 */
					//2015-11-18 修改为 主工序的处理时间 = 处理 + 出钢
					tpssm12["PROC_TIME"] = proc_time + post_proc_time;

					tpssm12["POST_PROC_TIME"] = 0;     /* 后处理时间=0 */
					tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */

					//新增
					sqlstr = "tpssm12.Insert()-DP-0";
					tpssm12.Insert();

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}//if 是否有脱P记录

			}


			//----------------------------------------------------------------------------
			//3、转炉脱C工序
			tpssm12["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["SM_DEV"].ToString().Trim(); //设备
			pre_proc_time     = bcls_rec->Tables[blkseq].Rows[i]["SM_LOAD_TIME"].ToDecimal().ToInt32(); //装入时间
			proc_time         = bcls_rec->Tables[blkseq].Rows[i]["SM_PROC_TIME"].ToDecimal().ToInt32(); //脱C 时间
			post_proc_time    = bcls_rec->Tables[blkseq].Rows[i]["SM_TAP_TIME"].ToDecimal().ToInt32();  //出钢时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["SM_REST_TIME"].ToDecimal().ToInt32();     //休辅时间
			////Log::Info("", __FUNCTION__, "转炉脱C：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());

			//读取原炼钢工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["AREA_ID"] = 3;  //3-转炉炼钢
			tpssm12_old["SUB_CHARGE_NO"] = 0;  //读取处理工序
			sqlstr = "tpssm12_old.Query()-SM";
			bool has_sm = tpssm12_old.Query("FACTORY_DIV, SM_PLAN_NO, SUB_CHARGE_NO, AREA_ID");

			sm_charge_no = tpssm12_old["CHARGE_NO"]; //记录当前转炉charge号
			sr_charge_no = sm_charge_no;

			//判断炼钢工序设备是否为空，空-报错
			if (tpssm12["DEV_CODE"].ToString() == "")
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]的转炉/电炉设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);

			}
			else  //修改炼钢工序
			{

				if ( (proc_time != tpssm12_old["PROC_TIME"].ToDecimal().ToInt32() ||  //处理时间调整
					tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString()) &&  //设备更换
					tpssm11_old["PONO_STATUS"].ToDecimal().ToInt32() > 30) //31-装入
				{
					CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]转炉已作业，不能做时间调整。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				backlog_ea = backlog_ea + "B";  //转炉工序

				tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
				tpssm12["CHARGE_NO"]  = tpssm12_old["CHARGE_NO"];
				tpssm12["SUB_CHARGE_NO"] = 0;  //主工序

				////对于转炉，主工序的处理时间 = 装入 + 处理 + 出钢
				//tpssm12["PROC_TIME"] = pre_proc_time + proc_time + post_proc_time;      /* 处理时间=装入+处理+镇静 */
				//2015-11-18 修改为 主工序的处理时间 = 处理 + 出钢
				tpssm12["PROC_TIME"] = proc_time + post_proc_time;

				//修改主工序
				sqlstr = "tpssm12.Update()-SM-0";
				tpssm12.Update(
					"DEV_CODE,"
					//"PRE_PROC_TIME,"    //装入时间
					"PROC_TIME,"
					//"POST_PROC_TIME,"   //出钢时间
					"REST_TIME,"
					"REC_REVISOR,REC_REVISE_TIME",
					"FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, SUB_CHARGE_NO"  //按主键修改
					);


				//调整整体计划状态用
				if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
					plan_edit_flag = "6"; //6 - 时刻调整；
				else
					plan_edit_flag = "4"; //4-设备调整；


			}//if 转炉设备为空


			//----------------------------------------------------------------------------
			//4、精炼1工序
			tpssm12["DEV_CODE"]  = bcls_rec->Tables[blkseq].Rows[i]["SR1_DEV"].ToString().Trim(); //设备
			proc_time         = bcls_rec->Tables[blkseq].Rows[i]["SR1_PROC_TIME"].ToDecimal().ToInt32();     //处理时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["SR1_REST_TIME"].ToDecimal().ToInt32();     //休辅时间
			////Log::Info("", __FUNCTION__, "精炼1：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());
			tpssm12["AREA_ID"] = 4;

			pre_proc_time  = 0; //前处理时间
			post_proc_time = 0; //后处理时间

			//读取原精炼1工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["CHARGE_NO"] = sr_charge_no + 1;  //精炼charge号
			tpssm12_old["SUB_CHARGE_NO"] = 0;  //读取主工序
			tpssm12_old["AREA_ID"] = 4;    //4-精炼

			sqlstr = "tpssm12_old.Query()-SR1";
			bool has_sr1 = tpssm12_old.Query("FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, SUB_CHARGE_NO, AREA_ID");

			tpssm12_old["DEV_CODE"] = tpssm12_old["DEV_CODE"].ToString().Trim();
			////Log::Trace("", __FUNCTION__, "精炼1：SM_PLAN_NO=[{0}], CHARGE_NO=[{1}], SUB_CHARGE_NO=[{2}], has_sr2=[{3}], DEV_CODE=[{4}]",
				//tpssm12_old["SM_PLAN_NO"].ToString(), tpssm12_old["CHARGE_NO"].ToDecimal(), tpssm12_old["SUB_CHARGE_NO"].ToDecimal(), has_sr1, tpssm12_old["DEV_CODE"].ToString());

			//精炼工序调整判断
			if (tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString())
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };

				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇
				{
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]连铸已开浇，不能做精炼调整。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (has_sr1 == true)
				{
					if (tpssm12_old["ARRIVE_REAL_TIME"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]包到，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					else if (tpssm12_old["START_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["END_TIME_REAL"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}

			//判断精炼1工序设备是否为空，空-不走精炼1，非空-走精炼1
			if (tpssm12["DEV_CODE"].ToString() == "")
			{

				if (has_sr1 == true && tpssm12_old["AREA_ID"].ToDecimal().ToInt32() == 4) //原来有精炼1，则删除
				{
					////Log::Trace("", __FUNCTION__, "   删除精炼[{0}]", tpssm12_old["DEV_CODE"].ToString());

					//删除
					sqlstr = "tpssm12_old.Delete()-SR1";
					tpssm12_old.Delete("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

					//后续各工序CHARGE_NO调整(减一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO - 1 "  //各工序减一
							" WHERE FACTORY_DIV = tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND CHARGE_NO  > @tpssm12.CHARGE_NO "
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString() );
					cmd.Parameters.Set("tpssm12.CHARGE_NO",  tpssm12_old["CHARGE_NO"].ToString() );
					cmd.ExecuteNonQuery();

					//sr_charge_no = sr_charge_no - 1;  //计算当前精炼charge号

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}
				else //原来就没有精炼计划，不做任何操作
				{
				}

			}
			else  //新增或修改精炼1工序
			{
				
				//1)根据 设备代码取设备标识
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				sqlstr = "tpssmd1.Query()-SR1";
				bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

				backlog_ea = backlog_ea + tpssmd1["DEV_TECH_CODE"].ToString().Trim();      //精炼工序
				sr_route   = sr_route + tpssm12["DEV_CODE"].ToString();      //新精炼路径

				sr_charge_no = sr_charge_no + 1;  //计算当前精炼charge号

				if (has_sr1 == true && tpssm12_old["DEV_CODE"].ToString() == tpssm12["DEV_CODE"].ToString()) //原来有精炼1且设备相同，则只修改时间
				{
					////Log::Trace("", __FUNCTION__, "    修改精炼[{0}]时间", tpssm12["DEV_CODE"].ToString());

					//调整判断
					if ( proc_time != tpssm12_old["PROC_TIME"].ToDecimal().ToInt32() &&  //处理时间调整
						(tpssm12_old["END_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["START_TIME_REAL"].ToString().Trim() != "") ) //已作业
					{
						CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做时间调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
					tpssm12["SUB_CHARGE_NO"] = tpssm12_old["SUB_CHARGE_NO"];

					tpssm12["PROC_TIME"] = proc_time;

					//修改
					sqlstr = "tpssm12.Update()-SR1";
					tpssm12.Update(
						"DEV_CODE,"
						"PROC_TIME,"
						"REST_TIME,"
						"REC_REVISOR,REC_REVISE_TIME",
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
						);

					//调整整体计划状态用
					if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
						plan_edit_flag = "6"; //6 - 时刻调整；
					else
						plan_edit_flag = "4"; //4-设备调整；

				}
				else //有精炼1或设备不同：1.没有，则新增； 2.有，但设备不同，删除原有的并新增（设备修改时，用标准处理时间）
				{

					//新增或调整精炼路径时，都要采用标准处理时间  2015-5-14 xuwen 修改
					//--------------------------------------------------------
					//读取工序标准作业时间：工序设备标识 -> PATTERN -> 处理时间				
					//1)根据 设备代码取设备标识
					tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
					sqlstr = "tpssmd1.Query()-DP";
					bool hasd1 = tpssmd1.Query("FACTORY_DIV, DEV_CODE");

					//2)根据 工序设备标识+出钢记号--> 处理时间
					tpssmd3["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssmd1["DEV_CODE"];  //脱P
					tpssmd3["ST_NO"] = tpssm11["ST_NO"];
					tpssmd3["SMELT_MODE"] = tpssm11["SMELT_MODE"];  //转炉工序要区分：1-单联; 3-双渣法; 2-双联;

					sqlstr = "tpssmd3.Query()-DP";
					bool hasd4 = tpssmd3.Query("FACTORY_DIV, ST_NO, DEV_CODE, SMELT_MODE");
					if (hasd4 == false) //读取不到数据
					{
						tpssmd3["STD_PROC_TIME"] = 40;
						tpssmd3["STD_PREP_TIME"] = 5; //准备时间(炉间)
						tpssmd3["FEED_TIME"] = 5;     //转炉/电炉用：废钢、铁水装入作业; 精炼、连铸的进站时间
						tpssmd3["DRAW_TIME"] = 5;     //转炉/电炉用：出钢作业; 精炼、连铸是离站时间; 
					}
					////Log::Trace("", __FUNCTION__, "DEV_CODE=[{0}], ST_NO=[{1}], SMELT_MODE=[{2}]",
						//tpssmd3["DEV_CODE"].ToString(), tpssmd3["ST_NO"].ToString(), tpssm11["SMELT_MODE"].ToDecimal());

					pre_proc_time  = tpssmd3["FEED_TIME"];
					proc_time = tpssmd3["STD_PROC_TIME"];
					post_proc_time = tpssmd3["DRAW_TIME"];
					//--------------------------------------------------------

					if (has_sr1 == false) //原计划没有该精炼，新增精炼
					{
						////Log::Trace("", __FUNCTION__, "    新增精炼1设备[{0}]", tpssm12["DEV_CODE"].ToString());

						//SR1后续各工序CHARGE_NO调整(加一)
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = CString(
								" UPDATE TPSSM12 "
								" SET CHARGE_NO = CHARGE_NO + 1 "  //各工序加一
								" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
								"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
								"   AND CHARGE_NO  >= @tpssm12.CHARGE_NO "
								);
							break;
						}
						cmd.SetCommandText(sqlstr);
						cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
						cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
						cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
						cmd.ExecuteNonQuery();

						//新增精炼1工序（SR1，此处可改进为函数）
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */

						//新增
						sqlstr = "tpssm12.Insert()-SR1";
						tpssm12.Insert();
					}

					//修改当前精炼设备，采用标准处理时间
					if (has_sr1 == true && tpssm12_old["DEV_CODE"].ToString() != tpssm12["DEV_CODE"].ToString())
					{
						////Log::Trace("", __FUNCTION__, "    修改精炼设备[{0}]->[{1}]", tpssm12_old["DEV_CODE"].ToString(), tpssm12["DEV_CODE"].ToString());

						tpssm12["DEV_CODE"] = tpssm12["DEV_CODE"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"]  = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"]  = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"]      = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */
						tpssm12["REST_TIME"] = 0;   // 休止时间

						//修改: 只修改时间，作业时刻不改，通过模型计算调整
						sqlstr = "tpssm12.Update()-SR1";
						tpssm12.Update(
							"DEV_CODE,"
							"PREP_TIME,"        //准备时间
							"PRE_PROC_TIME,"    //前处理时间
							"PROC_TIME,"        //处理时间
							"POST_PROC_TIME,"   //后处理时间
							"REST_TIME,"
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
							);
							
					}

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}//if 是否有精炼1记录

			}//if 设备为空


			//----------------------------------------------------------------------------
			//5、精炼2工序
			tpssm12["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["SR2_DEV"].ToString().Trim(); //设备
			proc_time        = bcls_rec->Tables[blkseq].Rows[i]["SR2_PROC_TIME"].ToDecimal().ToInt32();     //处理时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["SR2_REST_TIME"].ToDecimal().ToInt32();     //休辅时间
			////Log::Info("", __FUNCTION__, "精炼2：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());
			tpssm12["AREA_ID"] = 4;

			pre_proc_time  = 0; //前处理时间
			post_proc_time = 0; //后处理时间

			//读取原精炼2工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["CHARGE_NO"] = sr_charge_no + 1;  //精炼charge号
			tpssm12_old["SUB_CHARGE_NO"] = 0;  //读取主工序
			tpssm12_old["AREA_ID"] = 4;    //4-精炼
			sqlstr = "tpssm12_old.Query()-SR2";
			bool has_sr2 = tpssm12_old.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO,AREA_ID");

			tpssm12_old["DEV_CODE"] = tpssm12_old["DEV_CODE"].ToString().Trim();
			////Log::Trace("", __FUNCTION__, "精炼2：SM_PLAN_NO=[{0}], CHARGE_NO=[{1}], SUB_CHARGE_NO=[{2}], has_sr2=[{3}], DEV_CODE=[{4}]",
				//tpssm12_old["SM_PLAN_NO"].ToString(), tpssm12_old["CHARGE_NO"].ToDecimal(), tpssm12_old["SUB_CHARGE_NO"].ToDecimal(), has_sr2, tpssm12_old["DEV_CODE"].ToString());

			//精炼工序调整判断
			if (tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString())
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };

				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇
				{
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]连铸已开浇，不能做精炼调整。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (has_sr2 == true)
				{
					if (tpssm12_old["ARRIVE_REAL_TIME"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]包到，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					else if (tpssm12_old["START_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["END_TIME_REAL"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}

			//判断精炼2工序设备是否为空，空-不走精炼2，非空-走精炼2
			if (tpssm12["DEV_CODE"].ToString() == "")
			{

				if (has_sr2 == true && tpssm12_old["AREA_ID"].ToDecimal().ToInt32() == 4) //原来有精炼2，则删除
				{
					////Log::Trace("", __FUNCTION__, "   删除精炼[{0}]", tpssm12_old["DEV_CODE"].ToString());

					//删除
					sqlstr = "tpssm12_old.Delete()-SR2";
					tpssm12_old.Delete("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

					//后续各工序CHARGE_NO调整(减一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO - 1 "  //各工序减一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND CHARGE_NO  > @tpssm12.CHARGE_NO "
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
					cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
					cmd.ExecuteNonQuery();

					//sr_charge_no = sr_charge_no - 1;  //计算当前精炼charge号
					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}
				else //原来就没有精炼计划，不做任何操作
				{
				}

			}
			else  //新增或修改精炼2工序
			{
				//1)根据 设备代码取设备标识
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				sqlstr = "tpssmd1.Query()-SR2";
				bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

				backlog_ea = backlog_ea + tpssmd1["DEV_TECH_CODE"].ToString().Trim();      //精炼工序
				sr_route = sr_route + tpssm12["DEV_CODE"].ToString();      //新精炼路径

				sr_charge_no = sr_charge_no + 1;  //计算当前精炼charge号

				if (has_sr2 == true && tpssm12_old["DEV_CODE"].ToString() == tpssm12["DEV_CODE"].ToString()) //原来有精炼2且设备相同，则只修改时间
				{
					////Log::Trace("", __FUNCTION__, "    修改精炼[{0}]时间", tpssm12["DEV_CODE"].ToString());

					//调整判断
					if (proc_time.ToInt32() != tpssm12_old["PROC_TIME"].ToDecimal().ToInt32() &&  //处理时间调整
						(tpssm12_old["END_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["START_TIME_REAL"].ToString().Trim() != "")) //已作业
					{
						CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做时间调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
					tpssm12["SUB_CHARGE_NO"] = tpssm12_old["SUB_CHARGE_NO"];

					tpssm12["PROC_TIME"] = proc_time;

					//修改
					sqlstr = "tpssm12.Update()-SR2";
					tpssm12.Update(
						"DEV_CODE,"
						"PROC_TIME,"
						"REST_TIME,"
						"REC_REVISOR,REC_REVISE_TIME",
						"FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, SUB_CHARGE_NO"  //按主键修改
						);


					//调整整体计划状态用
					if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
						plan_edit_flag = "6"; //6 - 时刻调整；
					else
						plan_edit_flag = "4"; //4-设备调整；

				}
				else //1.没有，则新增； 2.有，但设备不同，删除原有的并新增（设备修改时，用标准处理时间）
				{

					//新增或调整精炼路径时，都要采用标准处理时间  2015-5-14 xuwen 修改
					//--------------------------------------------------------
					//读取工序标准作业时间：工序设备标识 -> PATTERN -> 处理时间				
					//1)根据 设备代码取设备标识
					tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
					sqlstr = "tpssmd1.Query()-DP";
					bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

					//2)根据 工序设备标识+出钢记号--> 处理时间
					tpssmd3["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssmd1["DEV_CODE"];  //脱P
					tpssmd3["ST_NO"] = tpssm11["ST_NO"];
					tpssmd3["SMELT_MODE"] = tpssm11["SMELT_MODE"];  //转炉工序要区分：1-单联; 3-双渣法; 2-双联;

					sqlstr = "tpssmd3.Query()-DP";
					bool hasd4 = tpssmd3.Query("FACTORY_DIV, ST_NO, DEV_CODE, SMELT_MODE");
					if (hasd4 == false) //读取不到数据
					{
						tpssmd3["STD_PROC_TIME"] = 40;
						tpssmd3["STD_PREP_TIME"] = 5; //准备时间(炉间)
						tpssmd3["FEED_TIME"] = 5;     //转炉/电炉用：废钢、铁水装入作业; 精炼、连铸的进站时间
						tpssmd3["DRAW_TIME"] = 5;     //转炉/电炉用：出钢作业; 精炼、连铸是离站时间; 
					}
					////Log::Trace("", __FUNCTION__, "DEV_CODE=[{0}], ST_NO=[{1}], SMELT_MODE=[{2}]",
						//tpssmd3["DEV_CODE"].ToString(), tpssmd3["ST_NO"].ToString(), tpssm11["SMELT_MODE"].ToDecimal());

					pre_proc_time = tpssmd3["FEED_TIME"];
					proc_time = tpssmd3["STD_PROC_TIME"];
					post_proc_time = tpssmd3["DRAW_TIME"];

					//--------------------------------------------------------

					if (has_sr2 == false) //原计划没有该精炼，新增精炼
					{
						////Log::Trace("", __FUNCTION__, "    新增精炼2设备[{0}]", tpssm12["DEV_CODE"].ToString());

						//SR2后续各工序CHARGE_NO调整(加一)
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = CString(
								" UPDATE TPSSM12 "
								" SET CHARGE_NO = CHARGE_NO + 1 "  //各工序加一
								" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
								"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
								"   AND CHARGE_NO  >= @tpssm12.CHARGE_NO "
								);
							break;
						}
						cmd.SetCommandText(sqlstr);
						cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
						cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
						cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
						////Log::Trace("", __FUNCTION__, "    新增精炼2设备1sqlstr = [{0}]", sqlstr);
						cmd.ExecuteNonQuery();


						//新增精炼2工序（SR2，此处可改进为函数）
						tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	 /* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */

						//新增
						sqlstr = "tpssm12.Insert()-SR2";
						////Log::Trace("", __FUNCTION__, "    新增精炼2设备2sqlstr = [{0}]", sqlstr);
						tpssm12.Insert();
					}

					//修改当前精炼设备，采用标准处理时间
					if (has_sr2 == true && tpssm12_old["DEV_CODE"].ToString() != tpssm12["DEV_CODE"].ToString())
					{
						////Log::Trace("", __FUNCTION__, "    修改精炼设备[{0}]->[{1}]", tpssm12_old["DEV_CODE"].ToString(), tpssm12["DEV_CODE"].ToString());

						tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
						tpssm12["DEV_CODE"] = tpssm12["DEV_CODE"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"]  = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;          /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */
						tpssm12["REST_TIME"] = 0;   // 休止时间

						//修改: 只修改时间，作业时刻不改，通过模型计算调整
						sqlstr = "tpssm12.Update()-SR2";
						tpssm12.Update(
							"DEV_CODE,"
							"PREP_TIME,"        //准备时间
							"PRE_PROC_TIME,"    //前处理时间
							"PROC_TIME,"        //处理时间
							"POST_PROC_TIME,"   //后处理时间
							"REST_TIME,"
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
							);

					}

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}//if 是否有精炼2记录

			}//if 设备为空


			//----------------------------------------------------------------------------
			//6、精炼3工序
			tpssm12["DEV_CODE"]  = bcls_rec->Tables[blkseq].Rows[i]["SR3_DEV"].ToString().Trim(); //设备
			proc_time         = bcls_rec->Tables[blkseq].Rows[i]["SR3_PROC_TIME"].ToDecimal().ToInt32();     //处理时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["SR3_REST_TIME"].ToDecimal().ToInt32();     //休辅时间
			////Log::Info("", __FUNCTION__, "精炼3：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());
			tpssm12["AREA_ID"] = 4;

			pre_proc_time  = 0; //前处理时间
			post_proc_time = 0; //后处理时间

			//读取原精炼3工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["CHARGE_NO"] = sr_charge_no + 1;  //精炼charge号
			tpssm12_old["SUB_CHARGE_NO"] = 0;  //读取主工序
			tpssm12_old["AREA_ID"] = 4;    //4-精炼

			sqlstr = "tpssm12_old.Query()-SM";
			bool has_sr3 = tpssm12_old.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO,AREA_ID");

			tpssm12_old["DEV_CODE"] = tpssm12_old["DEV_CODE"].ToString().Trim();
			////Log::Trace("", __FUNCTION__, "精炼3：SM_PLAN_NO=[{0}], CHARGE_NO=[{1}], SUB_CHARGE_NO=[{2}], has_sr2=[{3}], DEV_CODE=[{4}]",
				//tpssm12_old["SM_PLAN_NO"].ToString(), tpssm12_old["CHARGE_NO"].ToDecimal(), tpssm12_old["SUB_CHARGE_NO"].ToDecimal(), has_sr3, tpssm12_old["DEV_CODE"].ToString());

			//精炼工序调整判断
			if (tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString())
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };

				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇
				{
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]连铸已开浇，不能做精炼调整。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (has_sr3 == true)
				{
					if (tpssm12_old["ARRIVE_REAL_TIME"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]包到，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					else if (tpssm12_old["START_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["END_TIME_REAL"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}

			//判断精炼1工序设备是否为空，空-不走精炼1，非空-走精炼1
			if (tpssm12["DEV_CODE"].ToString() == "")
			{

				if (has_sr3 == true && tpssm12_old["AREA_ID"].ToDecimal().ToInt32() == 4) //原来有精炼3，则删除
				{
					////Log::Trace("", __FUNCTION__, "   删除精炼[{0}]", tpssm12_old["DEV_CODE"].ToString());

					//删除
					sqlstr = "tpssm12_old.Delete()-SR3";
					tpssm12_old.Delete("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

					//后续各工序CHARGE_NO调整(减一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO - 1 "  //各工序减一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND CHARGE_NO  > @tpssm12.CHARGE_NO "
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
					cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
					cmd.ExecuteNonQuery();

					//sr_charge_no = sr_charge_no - 1;  //计算当前精炼charge号
					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}
				else //原来就没有精炼3计划，不做任何操作
				{
				}

			}
			else  //新增或修改精炼3工序
			{

				//1)根据 设备代码取设备标识
				tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				sqlstr = "tpssmd1.Query()-SR3";
				bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

				backlog_ea = backlog_ea + tpssmd1["DEV_TECH_CODE"].ToString().Trim();      //精炼工序
				sr_route = sr_route + tpssm12["DEV_CODE"].ToString();      //新精炼路径

				sr_charge_no = sr_charge_no + 1;  //计算当前精炼charge号

				if (has_sr3 == true && tpssm12_old["DEV_CODE"].ToString() == tpssm12["DEV_CODE"].ToString()) //原来有精炼1且设备相同，则只修改时间
				{
					////Log::Trace("", __FUNCTION__, "    修改精炼[{0}]时间", tpssm12["DEV_CODE"].ToString());

					//调整判断
					if (proc_time != tpssm12_old["PROC_TIME"].ToDecimal().ToInt32() &&  //处理时间调整
						(tpssm12_old["END_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["START_TIME_REAL"].ToString().Trim() != "")) //已作业
					{
						CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做时间调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
					tpssm12["SUB_CHARGE_NO"] = tpssm12_old["SUB_CHARGE_NO"];

					tpssm12["PROC_TIME"] = proc_time;

					//修改
					sqlstr = "tpssm12.Update()-SR3";
					tpssm12.Update(
						"DEV_CODE,"
						"PROC_TIME,"
						"REST_TIME,"
						"REC_REVISOR,REC_REVISE_TIME",
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
						);


					//调整整体计划状态用
					if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
						plan_edit_flag = "6"; //6 - 时刻调整；
					else
						plan_edit_flag = "4"; //4-设备调整；

				}
				else //有精炼3或设备不同：1.没有，则新增； 2.有，但设备不同，删除原有的并新增（设备修改时，用标准处理时间）
				{

					//新增或调整精炼路径时，都要采用标准处理时间  2015-5-14 xuwen 修改
					//--------------------------------------------------------
					//读取工序标准作业时间：工序设备标识 -> PATTERN -> 处理时间				
					//1)根据 设备代码取设备标识
					tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
					sqlstr = "tpssmd1.Query()-DP";
					bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

					//2)根据 工序设备标识+出钢记号--> 处理时间
					tpssmd3["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssmd1["DEV_CODE"];  //脱P
					tpssmd3["ST_NO"] = tpssm11["ST_NO"];
					tpssmd3["SMELT_MODE"] = tpssm11["SMELT_MODE"];  //转炉工序要区分：1-单联; 3-双渣法; 2-双联;

					sqlstr = "tpssmd3.Query()-DP";
					bool hasd4 = tpssmd3.Query("FACTORY_DIV, ST_NO, DEV_CODE, SMELT_MODE");
					if (hasd4 == false) //读取不到数据
					{
						tpssmd3["STD_PROC_TIME"] = 40;
						tpssmd3["STD_PREP_TIME"] = 5; //准备时间(炉间)
						tpssmd3["FEED_TIME"] = 5;     //转炉/电炉用：废钢、铁水装入作业; 精炼、连铸的进站时间
						tpssmd3["DRAW_TIME"] = 5;     //转炉/电炉用：出钢作业; 精炼、连铸是离站时间; 
					}
					////Log::Trace("", __FUNCTION__, "DEV_CODE=[{0}], ST_NO=[{1}], SMELT_MODE=[{2}]",
						//tpssmd3["DEV_CODE"].ToString(), tpssmd3["ST_NO"].ToString(), tpssm11["SMELT_MODE"].ToDecimal());
					//--------------------------------------------------------

					pre_proc_time = tpssmd3["FEED_TIME"];
					proc_time = tpssmd3["STD_PROC_TIME"];
					post_proc_time = tpssmd3["DRAW_TIME"];

					if (has_sr3 == false) //原计划没有该精炼，新增精炼
					{
						////Log::Trace("", __FUNCTION__, "    新增精炼3设备[{0}]", tpssm12["DEV_CODE"].ToString());
						//SR3后续各工序CHARGE_NO调整(加一)
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = CString(
								" UPDATE TPSSM12 "
								" SET CHARGE_NO = CHARGE_NO + 1 "  //各工序加一
								" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
								"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
								"   AND CHARGE_NO  >= @tpssm12.CHARGE_NO "
								);
							break;
						}
						cmd.SetCommandText(sqlstr);
						cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
						cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
						cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
						cmd.ExecuteNonQuery();


						if (has_sr3 == true && tpssm12_old["AREA_ID"].ToDecimal().ToInt32() == 4) //原来有精炼1，则修改
						//新增精炼3工序（SR3，此处可改进为函数）
						tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */

						//新增
						sqlstr = "tpssm12.Insert()-SR3";
						tpssm12.Insert();
					}

					//修改当前精炼设备，采用标准处理时间
					if (has_sr3 == true && tpssm12_old["DEV_CODE"].ToString() != tpssm12["DEV_CODE"].ToString())
					{
						////Log::Trace("", __FUNCTION__, "    修改精炼设备[{0}]->[{1}]", tpssm12_old["DEV_CODE"].ToString(), tpssm12["DEV_CODE"].ToString());
						tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
						tpssm12["DEV_CODE"] = tpssm12["DEV_CODE"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"]  = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */
						tpssm12["REST_TIME"] = 0;   // 休止时间

						//修改: 只修改时间，作业时刻不改，通过模型计算调整
						sqlstr = "tpssm12.Update()-SR1";
						tpssm12.Update(
							"DEV_CODE,"
							"PREP_TIME,"        //准备时间
							"PRE_PROC_TIME,"    //前处理时间
							"PROC_TIME,"        //处理时间
							"POST_PROC_TIME,"   //后处理时间
							"REST_TIME,"
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
							);

					}

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}//if 是否有精炼3记录

			}//if 设备为空


			//----------------------------------------------------------------------------
			//7、精炼4工序
			tpssm12["DEV_CODE"]  = bcls_rec->Tables[blkseq].Rows[i]["SR4_DEV"].ToString().Trim(); //设备
			proc_time         = bcls_rec->Tables[blkseq].Rows[i]["SR4_PROC_TIME"].ToDecimal().ToInt32();     //处理时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["SR4_REST_TIME"].ToDecimal().ToInt32();     //休辅时间
			////Log::Info("", __FUNCTION__, "精炼4：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());
			tpssm12["AREA_ID"] = 4;

			pre_proc_time  = 0; //前处理时间
			post_proc_time = 0; //后处理时间

			//读取原精炼4工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["FACTORY_DIV"] = v_factory_div;
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["CHARGE_NO"] = sr_charge_no + 1;  //精炼charge号
			tpssm12_old["SUB_CHARGE_NO"] = 0;  //读取主工序
			tpssm12_old["AREA_ID"] = 4;    //4-精炼

			sqlstr = "tpssm12_old.Query()-SR4";
			bool has_sr4 = tpssm12_old.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO,AREA_ID");

			tpssm12_old["DEV_CODE"] = tpssm12_old["DEV_CODE"].ToString().Trim();
			////Log::Trace("", __FUNCTION__, "精炼4：SM_PLAN_NO=[{0}], CHARGE_NO=[{1}], SUB_CHARGE_NO=[{2}], has_sr2=[{3}], DEV_CODE=[{4}]",
				//tpssm12_old["SM_PLAN_NO"].ToString(), tpssm12_old["CHARGE_NO"].ToDecimal(), tpssm12_old["SUB_CHARGE_NO"].ToDecimal(), has_sr4, tpssm12_old["DEV_CODE"].ToString());

			//精炼工序调整判断
			if (tpssm12["DEV_CODE"].ToString() != tpssm12_old["DEV_CODE"].ToString())
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };

				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇
				{
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]连铸已开浇，不能做精炼调整。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (has_sr4 == true)
				{
					if (tpssm12_old["ARRIVE_REAL_TIME"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]包到，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					else if (tpssm12_old["START_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["END_TIME_REAL"].ToString().Trim() != "")
					{
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做精炼调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}

			//判断精炼4工序设备是否为空，空-不走精炼4，非空-走精炼4
			if (tpssm12["DEV_CODE"].ToString() == "")
			{

				if (has_sr4 == true && tpssm12_old["AREA_ID"].ToDecimal().ToInt32() == 4) //原来有精炼4，则删除
				{
					////Log::Trace("", __FUNCTION__, "   删除精炼[{0}]", tpssm12_old["DEV_CODE"].ToString());

					//删除
					sqlstr = "tpssm12_old.Delete()-SR4";
					tpssm12_old.Delete("SM_PLAN_NO,CHARGE_NO");

					//后续各工序CHARGE_NO调整(减一)
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" UPDATE TPSSM12 "
							" SET CHARGE_NO = CHARGE_NO - 1 "  //各工序减一
							" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
							"   AND CHARGE_NO  > @tpssm12.CHARGE_NO "
							);
						break;
					}
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
					cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
					cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
					cmd.ExecuteNonQuery();

					//sr_charge_no = sr_charge_no - 1;  //计算当前精炼charge号
					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}
				else //原来就没有精炼计划，不做任何操作
				{
				}

			}
			else  //新增或修改精炼4工序
			{

				//1)根据 设备代码取设备标识
				tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				sqlstr = "tpssmd1.Query()-SR4";
				bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

				backlog_ea = backlog_ea + tpssmd1["DEV_TECH_CODE"].ToString().Trim();      //精炼工序
				sr_route   = sr_route + tpssm12["DEV_CODE"].ToString();      //新精炼路径

				sr_charge_no = sr_charge_no + 1;  //计算当前精炼charge号

				if (has_sr4 == true && tpssm12_old["AREA_ID"].ToDecimal().ToInt32() == 4) //原来有精炼1，则修改
				{
					////Log::Trace("", __FUNCTION__, "    修改精炼[{0}]时间", tpssm12["DEV_CODE"].ToString());

					//调整判断
					if (proc_time != tpssm12_old["PROC_TIME"].ToDecimal().ToInt32() &&  //处理时间调整
						(tpssm12_old["END_TIME_REAL"].ToString().Trim() != "" || tpssm12_old["START_TIME_REAL"].ToString().Trim() != "")) //已作业
					{
						CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };
						CMessageFormat::Format(s.msg, "炼钢计划[{0}]的精炼[{1}]已作业，不能做时间调整。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
					tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
					tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
					tpssm12["SUB_CHARGE_NO"] = tpssm12_old["SUB_CHARGE_NO"];

					tpssm12["PROC_TIME"] = proc_time;

					//修改
					sqlstr = "tpssm12.Update()-SR4";
					tpssm12.Update(
						"DEV_CODE,"
						"PROC_TIME,"
						"REST_TIME,"
						"REC_REVISOR,REC_REVISE_TIME",
						"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
						);


					//调整整体计划状态用
					if (tpssm12["DEV_CODE"].ToString() == tpssm12_old["DEV_CODE"].ToString())  //设备没变
						plan_edit_flag = "6"; //6 - 时刻调整；
					else
						plan_edit_flag = "4"; //4-设备调整；


				}
				else //有精炼4或设备不同：1.没有，则新增； 2.有，但设备不同，删除原有的并新增（设备修改时，用标准处理时间）
				{

					//新增或调整精炼路径时，都要采用标准处理时间  2015-5-14 xuwen 修改
					//--------------------------------------------------------
					//读取工序标准作业时间：工序设备标识 -> PATTERN -> 处理时间				
					//1)根据 设备代码取设备标识
					tpssmd1["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
					sqlstr = "tpssmd1.Query()-DP";
					bool hasd1 = tpssmd1.Query("FACTORY_DIV, AREA_ID, DEV_CODE");

					//2)根据 工序设备标识+出钢记号--> 处理时间
					tpssmd3["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssmd1["DEV_CODE"];  //脱P
					tpssmd3["ST_NO"] = tpssm11["ST_NO"];
					tpssmd3["SMELT_MODE"] = tpssm11["SMELT_MODE"];  //转炉工序要区分：1-单联; 3-双渣法; 2-双联;

					sqlstr = "tpssmd3.Query()-DP";
					bool hasd4 = tpssmd3.Query("FACTORY_DIV, ST_NO, DEV_CODE, SMELT_MODE");
					if (hasd4 == false) //读取不到数据
					{
						tpssmd3["STD_PROC_TIME"] = 40;
						tpssmd3["STD_PREP_TIME"] = 5; //准备时间(炉间)
						tpssmd3["FEED_TIME"] = 5;     //转炉/电炉用：废钢、铁水装入作业; 精炼、连铸的进站时间
						tpssmd3["DRAW_TIME"] = 5;     //转炉/电炉用：出钢作业; 精炼、连铸是离站时间; 
					}
					////Log::Trace("", __FUNCTION__, "DEV_CODE=[{0}], ST_NO=[{1}], SMELT_MODE=[{2}]",
						//tpssmd3["DEV_CODE"].ToString(), tpssmd3["ST_NO"].ToString(), tpssm11["SMELT_MODE"].ToDecimal());

					//新增和修改精炼设备时，取标准数据  2015-5-14 xuwen 修改
					//if (pre_proc_time < 1)  pre_proc_time = tpssmd3["FEED_TIME"];  //没有输入时取标准值
					//if (proc_time < 1)      proc_time = tpssmd5.STD_PROC_TIME;  //没有输入时取标准值
					//if (post_proc_time < 1) post_proc_time = tpssmd3["DRAW_TIME"];  //没有输入时取标准值
					pre_proc_time  = tpssmd3["FEED_TIME"];
					proc_time      = tpssmd3["STD_PROC_TIME"];
					post_proc_time = tpssmd3["DRAW_TIME"];
					//--------------------------------------------------------

					if (has_sr4 == false) //原计划没有该精炼，新增精炼
					{
						////Log::Trace("", __FUNCTION__, "    新增精炼4设备[{0}]", tpssm12["DEV_CODE"].ToString());

						//SR4后续各工序CHARGE_NO调整(加一)
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default: // 所有数据库适用，通用SQL语句
							sqlstr = CString(
								" UPDATE TPSSM12 "
								" SET CHARGE_NO = CHARGE_NO + 1 "  //各工序加一
								" WHERE FACTORY_DIV = @tpssm12.FACTORY_DIV "
								"	AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
								"   AND CHARGE_NO  >= @tpssm12.CHARGE_NO "
								);
							break;
						}
						cmd.SetCommandText(sqlstr);
						cmd.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12_old["FACTORY_DIV"].ToString());
						cmd.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12_old["SM_PLAN_NO"].ToString());
						cmd.Parameters.Set("tpssm12.CHARGE_NO", tpssm12_old["CHARGE_NO"].ToDecimal());
						cmd.ExecuteNonQuery();


						//新增精炼4工序（SR4，此处可改进为函数）
						tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */

						//新增
						sqlstr = "tpssm12.Insert()-SR4";
						tpssm12.Insert();
					}

					//修改当前精炼设备，采用标准处理时间
					if (has_sr4 == true && tpssm12_old["DEV_CODE"].ToString() != tpssm12["DEV_CODE"].ToString())
					{
						////Log::Trace("", __FUNCTION__, "    修改精炼设备[{0}]->[{1}]", tpssm12_old["DEV_CODE"].ToString(), tpssm12["DEV_CODE"].ToString());
						tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
						tpssm12["DEV_CODE"] = tpssm12["DEV_CODE"];
						tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
						tpssm12["CHARGE_NO"]  = tpssm12_old["CHARGE_NO"];
						tpssm12["SUB_CHARGE_NO"] = 0;
						tpssm12["AREA_ID"] = 4;    //4-精炼

						tpssm12["PRE_PROC_TIME"] = pre_proc_time;	/* 前处理时间=进站时间 */
						tpssm12["PROC_TIME"] = proc_time;      /* 处理时间 */
						tpssm12["POST_PROC_TIME"] = post_proc_time;     /* 后处理时间 */
						tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */
						tpssm12["REST_TIME"] = 0;   // 休止时间

						//修改: 只修改时间，作业时刻不改，通过模型计算调整
						sqlstr = "tpssm12.Update()-SR1";
						tpssm12.Update(
							"DEV_CODE,"
							"PREP_TIME,"        //准备时间
							"PRE_PROC_TIME,"    //前处理时间
							"PROC_TIME,"        //处理时间
							"POST_PROC_TIME,"   //后处理时间
							"REST_TIME,"
							"REC_REVISOR,REC_REVISE_TIME",
							"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
							);

					}

					//调整整体计划状态用
					plan_edit_flag = "5"; //5 - 路径调整；

				}//if 是否有精炼4记录

			}//if 设备为空




			//----------------------------------------------------------------------------
			//8、连铸工序
			CString cc_mach_no = bcls_rec->Tables[blkseq].Rows[i]["CC_MACH_NO"].ToString().Trim();
			if (cc_mach_no == "")
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm11_old["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]的连铸设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm12["DEV_CODE"]  = "C" + cc_mach_no;  //设备代码
			proc_time         = bcls_rec->Tables[blkseq].Rows[i]["CC_PROC_TIME"].ToDecimal().ToInt32();     //浇铸时间
			tpssm12["REST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["CC_REST_TIME"].ToDecimal().ToInt32();     //休辅时间
			////Log::Info("", __FUNCTION__, "连铸：dev=[{0}], PROC_TIME=[{1}], REST_TIME=[{2}]", tpssm12["DEV_CODE"].ToString(), proc_time, tpssm12["REST_TIME"].ToDecimal());

			//读取原连铸工序计划信息
			tpssm12_old.Reset();
			tpssm12_old["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_old["AREA_ID"] = 5;  //5-连铸
			tpssm12_old["SUB_CHARGE_NO"] = 0;  //读取主工序
			sqlstr = "tpssm12_old.Query()-CC";
			bool has_cc = tpssm12_old.Query("FACTORY_DIV,SM_PLAN_NO,SUB_CHARGE_NO,AREA_ID");

			//判断连铸工序设备是否为空，空-报错
			if (tpssm12["DEV_CODE"].ToString() == "")
			{
				CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]的浇铸设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);

			}
			else  //修改连铸工序
			{

				backlog_ea = backlog_ea + "C";      //精炼工序

				//if (tpssm12_old["DEV_CODE"].ToString().Trim() != tpssm12["DEV_CODE"].ToString().Trim()) //有换机浇铸要求
				//{
				//	CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "炼钢计划号[{0}]的浇铸设备不能更换, 请输入后操作。", arguments, 2);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				//调整判断
				if (tpssm11_old["PONO_STATUS"].ToDecimal() > 81) //82-开浇
				{
					CFormattable arguments[] = { tpssm11_old["SM_PLAN_NO"].ToString(), tpssm12_old["DEV_CODE"].ToString() };
					CMessageFormat::Format(s.msg, "炼钢计划[{0}]的连铸已作业，不能做连铸时间调整。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm12["FACTORY_DIV"] = tpssm12_old["FACTORY_DIV"];
				tpssm12["SM_PLAN_NO"] = tpssm12_old["SM_PLAN_NO"];
				tpssm12["CHARGE_NO"] = tpssm12_old["CHARGE_NO"];
				tpssm12["SUB_CHARGE_NO"] = 0;  //主工序

				tpssm12["PROC_TIME"] = proc_time;

				//修改主工序
				sqlstr = "tpssm12.Update()-CC";
				tpssm12.Update(
					//"DEV_CODE,"
					//"PRE_PROC_TIME,"
					"PROC_TIME,"
					//"POST_PROC_TIME,"
					"REST_TIME,"
					"REC_REVISOR,REC_REVISE_TIME",
					"FACTORY_DIV,SM_PLAN_NO,CHARGE_NO,SUB_CHARGE_NO"  //按主键修改
					);


				//如果有浇铸时间调整，同步修改TPSSM10表
				tpssm10["POUR_TIME"] = proc_time;
				tpssm10["PONO"]      = tpssm11["PONO"];
				sqlstr = "tpssm10.Update()";
				tpssm10.Update("POUR_TIME", "FACTORY_DIV,PONO");


				//调整整体计划状态用
				plan_edit_flag = "6"; //6 - 时刻调整；


			}//if 连铸设备为空



			//-------------------------------------------------
			//更新计划主体
			tpssm11["FACTORY_DIV"] = v_factory_div;
			tpssm11["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm11["REFINE_ROUTE_CODE"] = sr_route;
			tpssm11["BACKLOG_EA"] = backlog_ea;
			tpssm11["PONO_STATUS"] = tpssm11_old["PONO_STATUS"];

			////如果连铸状态回退包到，并有精炼调整，状态回退，2015-09-29 增加；   2015-10-29 注释
			//if (tpssm11_old["PONO_STATUS"].ToDecimal() == 81 &&  //81-包到
			//	(plan_edit_flag == "5" || plan_edit_flag == "4" || plan_edit_flag == "6") //5-路径调整(增加精炼); 4-设备调整; 6-时刻调整
			//	)
			//{
			//	tpssm11["PONO_STATUS"] = 43;  //43-精炼结束
			//	//实际包到时刻清除（对连铸回退功能）
			//	tpssm12.ARRIVE_TIME_REAL = " ";
			//	tpssm12["AREA_ID"] = 5;   //5-连铸工序
			//	sqlstr = "tpssm12.Update()-CC_ARRIVE";
			//	tpssm12.Update(
			//		"ARRIVE_TIME_REAL",
			//		"SM_PLAN_NO,AREA_ID"  //
			//		);
			//}
			
			item_11 = item_11 + "PONO_STATUS,REFINE_ROUTE_CODE,BACKLOG_EA";

			////Log::Trace("", __FUNCTION__, "主表：item=[{0}], SMELT_MODE=[{1}], REFINE_ROUTE_CODE=[{2}], BACKLOG_EA=[{3}] ",
				//item_11, tpssm11["SMELT_MODE"].ToDecimal(), tpssm11["REFINE_ROUTE_CODE"].ToString(), tpssm11["BACKLOG_EA"].ToString());

			sqlstr = "tpssm11.Update()";
			tpssm11.Update(item_11, "SM_PLAN_NO, FACTORY_DIV");

			item_11 = "";

		}//for 传入参数


		////----------------------------------------------------------
		////修改出钢计划应答表的计划编辑标记（整体计划状态）
		//sqlstr = CString(
		//	" UPDATE TPSSM23 SET "
		//	" PLAN_EDIT_FLAG = @plan_edit_flag "
		//	",PLAN_EDIT_TIME = @date_time "
		//	" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "//计划编辑时刻
		//	);
		//cmd_upd.SetCommandText(sqlstr);
		//cmd_upd.Parameters.Set("plan_edit_flag", plan_edit_flag);
		//cmd_upd.Parameters.Set("date_time", date_time);
		//cmd_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		//cmd_upd.ExecuteNonQuery();

	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = -1;
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
