/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-24
Description: 出钢计划炉次调整
**************************************************/
#include "stdafx.h"





int f_pssm12_plan_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划炉次条件修改
int f_pssm12_time_calc(EIClass *bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划各工序作业时刻计算
int f_pssm_get_cc_prep_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //获取连铸炉次间准备时间（单记录）

//调用TPS模型
//int f_pssm_call_tps(CString main_backlog_code, int mode, int bof_cc_matching, CDbConnection * conn);//

/*<remark>=========================================================
/// <summary>
/// 出钢计划炉次条件修改
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
// service入口
BM2F_ENTERACE(pssm12a_upd)


int f_pssm12a_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i=0;
	CString date_time = "";

	/* 业务变量 */
	CDecimal v_sm_plan_no = 0;	//炼钢计划号
	CString  v_pono = "";
	CString  v_smelt_mode = "";	//冶炼模式
	CString  v_sr_route = "";	//精炼路径
	CString  v_td_chg = "";	    //换中包

	int     bof_cc_matching = 0;     /*炉机匹配标记: 0-不指定,模型推荐; 1-指定,模型按要求计算*/
	int		mode = 0;      //调用模型功能:  0-模型;   1-匹配;

	EIClass inBlock;
	EIClass outBlock;  //CC准备时间计算返回块（单记录）

	CString sqlstr = "";

	CDbCommand cmd(conn);

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm10("TPSSM10");
	CModel tpssmd1("TPSSMD1");


	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------------------
		//定义函数调用信息块
		blkseq = 0;  //第一块 "PONO"块，f_pssm12_time_calc()函数用
		inBlock.Tables[blkseq].set_TableName("PONO");
		//inBlock.Tables[blkseq].Columns.Add(tpssm11);   //从实体对象创建架构
		inBlock.Tables[blkseq].Columns.Add(DT_STRING, "PONO");//制造命令号
		inBlock.Tables[blkseq].Columns.Add(DT_STRING, "FACTORY_DIV");//制造命令号

		//CC准备时间计算
		CDataTable &table = inBlock.Tables.Add("PONO_CAST");
		table.Columns.Add(DT_STRING, "PONO");//制造命令号
		table.Columns.Add(DT_STRING, "CC_MACH_NO"); //连铸机号
		table.Columns.Add(DT_STRING, "RESTRAND_FLG"); //重开浇标志 T-重开浇
		table.Columns.Add(DT_STRING, "TD_CHG_FLG"); //更换中包：D-换中包
		table.Rows.Add();  //单记录

		//---------------------------------------------------
		//获得输入参数，并进行相应解析（调用f_pssm12_plan_upd函数用）
		blkseq = bcls_rec->Tables.IndexOf("TPSSM12A");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有传入炉次条件调整信息数据块[TPSSM12A]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [TPSSM12A] NOT EXIST in pssm12_upd().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//修改输入块名，调用f_pssm12_plan_upd() 用
		bcls_rec->Tables[blkseq].set_TableName("TPSSM_CONST");
		if (bcls_rec->Tables[blkseq].Columns.Contains("DS_DEV") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DS_DEV");//脱硫设备号
		if (bcls_rec->Tables[blkseq].Columns.Contains("DS_PROC_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DS_PROC_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("DS_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DS_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("DP_LOAD_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DP_LOAD_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("DP_PROC_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DP_PROC_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("DP_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DP_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("DP_TAP_TIME") == false)  bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "DP_TAP_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SM_LOAD_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SM_LOAD_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SM_PROC_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SM_PROC_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SM_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SM_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SM_TAP_TIME") == false)  bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SM_TAP_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SR1_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SR1_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SR2_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SR2_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SR3_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SR3_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("SR4_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "SR4_REST_TIME");
		if (bcls_rec->Tables[blkseq].Columns.Contains("CC_REST_TIME") == false) bcls_rec->Tables[blkseq].Columns.Add(DT_STRING, "CC_REST_TIME");

		//循环读取信息，多记录
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"].ToDecimal();
			v_pono       = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString();
			////Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}], PONO=[{1}]", v_sm_plan_no, v_pono);


			//主要修改项，进行分解
			//1.冶炼模式
			v_smelt_mode = bcls_rec->Tables[blkseq].Rows[i]["SMELT_MODE"].ToString().Trim(); //冶炼模式

			//1.转炉设备
			CString dp_dev = bcls_rec->Tables[blkseq].Rows[i]["DP_DEV"].ToString().Trim();
			CString sm_dev = bcls_rec->Tables[blkseq].Rows[i]["SM_DEV"].ToString().Trim();

			//脱P设备
			if (dp_dev != "")
			{
				tpssmd1["DEV_CODE"] = "P" + dp_dev;
				tpssmd1["AREA_ID"] = 2;

				if (tpssmd1.QueryCount("DEV_CODE,AREA_ID") < 0)
				{
					CFormattable arguments[] = { v_sm_plan_no.ToString(), v_pono, dp_dev }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划号[{0}]设定的脱P炉号[{2}]不正确，请重新输入。", arguments, 3); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				bcls_rec->Tables[blkseq].Rows[i]["DP_DEV"] = tpssmd1["DEV_CODE"];
			}

			//脱C设备
			if (sm_dev != "")
			{
				tpssmd1["DEV_CODE"] = "B" + sm_dev;
				tpssmd1["AREA_ID"] = 3;

				if (tpssmd1.QueryCount("DEV_CODE,AREA_ID") < 0)
				{
					CFormattable arguments[] = { v_sm_plan_no.ToString(), v_pono, sm_dev }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划号[{0}]设定的脱C炉号[{2}]不正确，请重新输入。", arguments, 3); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				bcls_rec->Tables[blkseq].Rows[i]["SM_DEV"] = tpssmd1["DEV_CODE"];
			}

			//2.精炼路径(分解)
			v_sr_route = bcls_rec->Tables[blkseq].Rows[i]["SR_ROUTE"].ToString().Trim();  //精炼路径

			////Log::Info("", __FUNCTION__, "DP_DEV=[{0}], SM_DEV=[{1}], SR_ROUTE=[{2}]", dp_dev, sm_dev, v_sr_route);

			//3.解析精炼路径（每2位表示一精炼设备代码）
			int sr_len = v_sr_route.GetLength();
			if (sr_len > 8)
			{
				CFormattable arguments[] = { v_sm_plan_no.ToString(), v_pono, v_sr_route }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划号[{0}]更新的精炼路径[{2}]超长（8位字符以内），请重新输入。", arguments, 3); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			for (int j = 0; j < sr_len; j = j + 2)
			{
				CString sr_dev = v_sr_route.SubstringNE(j, 2);

				tpssmd1["DEV_CODE"] = sr_dev;
				tpssmd1["AREA_ID"] = 4;

				if (tpssmd1.QueryCount("DEV_CODE,AREA_ID") < 0)
				{
					CFormattable arguments[] = { v_sm_plan_no.ToString(), v_pono, v_sr_route }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划号[{0}]更新的精炼路径[{2}]不正确，请重新输入。", arguments, 3); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//解析的新设备代码赋值
				switch (j / 2)
				{
				case 0: bcls_rec->Tables[blkseq].Rows[i]["SR1_DEV"] = sr_dev;  break;
				case 1: bcls_rec->Tables[blkseq].Rows[i]["SR2_DEV"] = sr_dev;  break;
				case 2: bcls_rec->Tables[blkseq].Rows[i]["SR3_DEV"] = sr_dev;  break;
				case 3: bcls_rec->Tables[blkseq].Rows[i]["SR4_DEV"] = sr_dev;  break;
				}

			}//for 精炼
			//后几道精炼置空（共4重精炼）
			for (int j = sr_len; j < 8; j = j + 2)
			{
				switch (j / 2)
				{
				case 0: bcls_rec->Tables[blkseq].Rows[i]["SR1_DEV"] = "";  break;
				case 1: bcls_rec->Tables[blkseq].Rows[i]["SR2_DEV"] = "";  break;
				case 2: bcls_rec->Tables[blkseq].Rows[i]["SR3_DEV"] = "";  break;
				case 3: bcls_rec->Tables[blkseq].Rows[i]["SR4_DEV"] = "";  break;
				}
			}

			////换中包
			//v_td_chg = bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"].ToString().Trim();
			//if (v_td_chg == "D")  //换中包
			//{
			//	bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"] = "D";
			//	bcls_rec->Tables[blkseq].Rows[i]["INS_FE_FLG"] = "";
			//}
			//else if (v_td_chg == "X") //插铁板
			//{
			//	bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"] = "";
			//	bcls_rec->Tables[blkseq].Rows[i]["INS_FE_FLG"] = "X";
			//}
			//else if (v_td_chg == "DX") //换中包 + 插铁板
			//{
			//	bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"] = "D";
			//	bcls_rec->Tables[blkseq].Rows[i]["INS_FE_FLG"] = "X";
			//}
			//else
			//{
			//	bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"] = "";
			//	bcls_rec->Tables[blkseq].Rows[i]["INS_FE_FLG"] = "";
			//}

			////---------------------------------------
			////连铸炉次间准备时间计算
			//tpssm10["PONO"] = v_pono;
			//tpssm10["RESTRAND_FLG"] = bcls_rec->Tables[blkseq].Rows[i]["RESTRAND_FLG"];
			//tpssm10["TD_CHG_FLG"] = bcls_rec->Tables[blkseq].Rows[i]["TD_CHG_FLG"];
			//tpssm11["INS_FE_FLAG"] = bcls_rec->Tables[blkseq].Rows[i]["INS_FE_FLAG"];
			//inBlock.Tables["PONO_CAST"].Rows[0]["PONO"] = v_pono;
			//inBlock.Tables["PONO_CAST"].Rows[0]["CC_MACH_NO"] = bcls_rec->Tables[blkseq].Rows[i]["CC_MACH_NO"];
			//inBlock.Tables["PONO_CAST"].Rows[0]["RESTRAND_FLG"] = tpssm10["RESTRAND_FLG"];
			//inBlock.Tables["PONO_CAST"].Rows[0]["TD_CHG_FLG"] = tpssm10["TD_CHG_FLG"];

			//ret = f_pssm_get_cc_prep_time(&inBlock, &outBlock, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			//CDecimal cc_prep_time = outBlock.Tables["PREP_TIME"].Rows[0]["CC_PREP_TIME"];

			//////Log::Trace("", __FUNCTION__, "get CC_PREP_TIME=[{0}]", cc_prep_time);

			////同步修改浇铸计划中的炉次间准备时间(是否可以放到函数里??)
			//tpssm10["CC_PREP_TIME"] = cc_prep_time.ToInt32();
			//sqlstr = "tpssm10.Update()";
			//tpssm10.Update("RESTRAND_FLG,TD_CHG_FLG,INS_FE_FLAG,CC_PREP_TIME", "PONO");


			//2.处理时间（以下处理时间画面不传入，初始赋-1）
			bcls_rec->Tables[blkseq].Rows[i]["DP_LOAD_TIME"] = "-1";  //脱P装入时间
			bcls_rec->Tables[blkseq].Rows[i]["DP_TAP_TIME"]  = "-1";  //脱P出钢时间
			bcls_rec->Tables[blkseq].Rows[i]["SM_LOAD_TIME"] = "-1";  //脱C装入时间
			bcls_rec->Tables[blkseq].Rows[i]["SM_TAP_TIME"]  = "-1";  //脱C出钢时间


			//工序修改后，时刻推算用数据
			CDataRow &row = inBlock.Tables["PONO"].Rows.Add();
			row["PONO"] = v_pono;
			row["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString();

			//TPSSM11字段修改
			tpssm11["PONO"] = v_pono;
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString();
			if (tpssm11.Query("FACTORY_DIV,PONO"))
			{
				//吹氩区分、转炉辅助作业
				if (83 > tpssm11["PONO_STATUS"].ToDecimal())
				{
					tpssm11["AR_DIFF"] = bcls_rec->Tables[blkseq].Rows[i]["AR_DIFF"].ToString();
					tpssm11["BOF_ASSIST_OPT"] = bcls_rec->Tables[blkseq].Rows[i]["BOF_ASSIST_OPT"].ToString();
					tpssm11["BOF_ASSIST_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["BOF_ASSIST_TIME"].ToString();
				}
				//更新表
				tpssm11.Update("AR_DIFF,BOF_ASSIST_OPT,BOF_ASSIST_TIME", "FACTORY_DIV,PONO");
			}


		}//for 传入参数


		//----------------------------------------------------------
		//1.出钢计划炉次条件修改
		ret = f_pssm12_plan_upd(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------------
		//2.出钢计划各工序作业时刻计算 （使用模型计算时，本函数可以不调用）
		ret = f_pssm12_time_calc(&inBlock, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------------
		//模型计算
		//mode=0 正常编制   mode=1 时间优化 mode=2 局部编制  mode=3 转炉编制  mode=4 非转炉编制 mode=5 模铸编制
		mode = 1;
		bof_cc_matching = 1;
		//ret = f_pssm_call_tps(v_sm_unit_no, mode, bof_cc_matching, conn);
		if (ret < 0)
		{
			//strcpy(s.msg, "模型调用出错.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

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
