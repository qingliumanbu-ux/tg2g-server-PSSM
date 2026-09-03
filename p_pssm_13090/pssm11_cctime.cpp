/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-19
Description: 连铸浇铸计划开浇时刻设置
**************************************************/
#include "stdafx.h"

#include "tpssm10.h"
#include "tpssmd9.h"


/*<remark>=========================================================
/// <summary>
/// 连铸浇铸计划开浇时刻设置
/// <para>根据传入的某连铸机下的PONO，修改当前开浇标志。</para>
/// <para>
/// 1.约束：连浇的第一炉设置
/// 2. 开浇标志可重复设置：
///   1）原记录没有标志，置“T”；
///   2）原记录有“T”标志，置空。
/// <para>数据库表：TPSSM10(炼钢浇铸计划表)                    </para>
/// <para>主调用函数：前台PSSM10P画面F4(重开浇)调用。              </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <param name="pono">制造命令号             </param>
/// <returns></returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_cctime)


int f_pssm11_cctime(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_pono = "";
	CString v_pour_time = "";  //指定开浇时刻
	CString v_cc_req_time_flag = "";     //开浇时刻指定标记，前台指定
	CString v_restrd = "";     //开浇"T"
	CDecimal v_td_chg = 0;     //换中包
	CString v_ins_fe = "";     //插铁板

	CString date_time = "";

	// 定义表的实体对象
	CTPSSM10 tpssm10(conn);
	CTPSSMD9 tpssmd9(conn);

	CString sqlstr = "";
	//CDbCommand cmd_inq(conn);

	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");


		//---------------------------------------------------
		//获得输入参数
		//循环读取浇铸信息，单记录
		blkseq = bcls_rec->Tables.IndexOf("TPSSM10_DLG");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到浇铸时刻的信息数据块[TPSSM10_DLG]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [TPSSM10_DLG] NOT EXIST in pssm10p_seq().");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		if (rows == 0)  //没有记录
		{
			sprintf(s.msg, "浇铸时刻信息[TPSSM10_DLG]中没有数据，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [TPSSM10_DLG] haven't data.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		v_pono = bcls_rec->Tables[blkseq].Rows[0]["PONO"].ToString().Trim();
		v_pour_time = bcls_rec->Tables[blkseq].Rows[0]["CC_REQ_TIME"].ToString().Trim();
		v_cc_req_time_flag = bcls_rec->Tables[blkseq].Rows[0]["CC_REQ_TIME_FLAG"].ToString().Trim();//"1"-指定， ""-取消指定
		v_restrd = bcls_rec->Tables[blkseq].Rows[0]["RESTRAND_FLG"].ToString().Trim();
		v_td_chg = bcls_rec->Tables[blkseq].Rows[0]["TD_CHG_FLG"].ToDecimal();  //换中包

		Log::Info("", __FUNCTION__, "PONO=[{0}], v_restrd=[{1}]", v_pono, v_restrd);
		Log::Info("", __FUNCTION__, "pour_time=[{0}], cc_req_time_flag=[{1}]", v_pour_time, v_cc_req_time_flag);

		tpssm10.PONO = v_pono;

		//读取浇铸信息
		sqlstr = "tpssm10.Query()";
		bool has10 = tpssm10.Query("PONO");
		if (has10 == false)
		{
			CFormattable arguments[] = { tpssm10.PONO }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令号[{0}]不存在，请查询后操作。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//判断状态，如果已浇铸，顺序号改为0
		if (tpssm10.PONO_STATUS.ToInt32() >= 52) //52-开浇
		{
			CFormattable arguments[] = { tpssm10.PONO }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令号[{0}]已开浇，不能设置开浇时刻，请重新选择操作。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//判断是否设置开浇标志
		if (v_cc_req_time_flag == "1" && v_restrd == "" && tpssm10.RESTRAND_FLG.Trim() == "")
		{
			CFormattable arguments[] = { tpssm10.PONO }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令号[{0}]不是开浇炉（“T”标记炉次），不能设置开浇时刻，请重新选择操作。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//------------------------------
		//数据设置
		tpssm10.TD_CHG_FLG = v_td_chg;   //换中包

		//1. 开浇标志设置
		if (v_restrd == "T")
		{
			tpssm10.RESTRAND_FLG = "T";  //设置开浇炉

			//开浇时刻指定标志
			if (v_cc_req_time_flag == "")
			{
				tpssm10.CC_REQ_TIME_FLAG = " ";
			}
			else
			{
				tpssm10.CC_REQ_TIME_FLAG = "1";

				//指定开浇时刻校验
				CDateTime dt1 = CDateTime::Now(); //本炉开浇时刻
				CDateTime dt2 = CDateTime::Parse(v_pour_time);    //设定开浇时刻
				CTimeSpan  interval = dt2 - dt1;
				CDecimal interval_time = CDecimal(interval.TotalMinutes());

				//如果输入的时刻比当前时刻晚，认为是跨天设置
				if (interval_time < -10) //比-10小，时刻设定不正确
				{
					CFormattable arguments[] = { v_pour_time, date_time }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "设定时刻[{0}]比当前时刻[{1}]早，请重新设置。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else if (interval_time > 60 * 24) //不允许超过24小时
				{
					CFormattable arguments[] = { v_pour_time, date_time }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "设定时刻[{0}]超过当前时刻[{1}]1天，请重新设置。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tpssm10.CC_REQ_TIME = v_pour_time;
			}

			tpssm10.TD_CHG_FLG = 0;    //换中包
		}
		else  //开浇标志取消
		{
			tpssm10.RESTRAND_FLG == " ";
			tpssm10.CC_REQ_TIME_FLAG = " "; //有指定开浇时刻的标记取消
		}


		//----------------------------------------------------
		//读取连铸设备参数表（tpssmd9），对准备时间做修正
		bool hasc1 = false;
		if (tpssmd9.CC_MACH_NO != tpssm10.CC_MACH_NO.Trim()) //没有读取过
		{
			tpssmd9.CC_MACH_NO = tpssm10.CC_MACH_NO.Trim();

			sqlstr = "tpssmd9.Query()";
			hasc1 = tpssmd9.Query("CC_MACH_NO");
		}


		//设置炉间准备时间，必须与函数 f_pssm10_pour_time, pssm10_strd 中的计算一致
		if (hasc1 == true)
		{
			//重开浇炉次
			if (tpssm10.RESTRAND_FLG == "T")//CAST间, CAST的第一炉
			{
				tpssm10.CC_PREP_TIME = tpssmd9.TT_TD_DELAY_CAST + tpssmd9.TT_PREP_W0_CAST;    //CCCAST间TD延长时间 + CAST间准备时间(调幅无)
			}
			else  //CAST内
			{

				if (tpssm10.TD_CHG_FLG == 1) //更换中包
				{
					//CC第二炉次直前准备时间 + max(CC第二炉次TD交换时间, 第二炉次直前准备时间) 
					tpssm10.CC_PREP_TIME =
						((tpssmd9.TT_TDEX_2CH > tpssmd9.TT_PREP_2CH) ? tpssmd9.TT_TDEX_2CH : tpssmd9.TT_PREP_2CH);
				}
				else
				{
					//第二炉次准备时间 + CC第二炉次直前准备时间
					tpssm10.CC_PREP_TIME = tpssmd9.TT_PREP_2CH; // + tpssmd9.TT_PREP_LAST_2CH
					//Log::Trace("", __FUNCTION__, "CAST内不换中包, pono=[{0}] prep_time=[{1}]", tpssm10.PONO, tpssm10.CC_PREP_TIME);
				}

			}//if

		}
		else //若没找到对应连铸机配置，取默认值
		{
			//重开浇炉次
			if (tpssm10.RESTRAND_FLG == "T")//CAST间, CAST的第一炉
			{
				tpssm10.CC_PREP_TIME = 60;
			}
			else  //CAST内
			{
				tpssm10.CC_PREP_TIME = 4;
			}
		}


		tpssm10.REC_REVISOR = CString(s.userid);
		tpssm10.REC_REVISE_TIME = date_time;

		sqlstr = "tpssm10.Update()";
		tpssm10.Update(
			"RESTRAND_FLG,"
			"CC_REQ_TIME,"
			"CC_REQ_TIME_FLAG,"
			"CC_PREP_TIME,"
			"TD_CHG_FLG,"
			"INS_FE_FLG,"
			"REC_REVISOR,REC_REVISE_TIME",
			"PONO");


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