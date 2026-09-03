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
//调用TPS模型
//int f_pssm_call_tps(CString main_backlog_code, int mode, int bof_cc_matching, CDbConnection * conn);//
//根据作业时刻，赋流水号
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);

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
BM2F_ENTERACE(pssm12_upd)


int f_pssm12_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i=0;
	int blkNum = 0;

	/* 业务变量 */
	CDecimal v_sm_plan_no = 0;	//炼钢计划号
	CString  v_factory_div = "";
	CString  v_pono = "";
	CString date_time = "";
	CString v_sm_unit_no = "";	//炼钢单元号
	int     bof_cc_matching = 0;     /*炉机匹配标记: 0-不指定,模型推荐; 1-指定,模型按要求计算*/
	int		mode = 0;      //调用模型功能:  0-模型;   1-匹配;

	EIClass inBlock;

	CString sqlstr = "";

	CDbCommand cmd(conn);

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");


	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------------------
		//定义函数调用信息块
		blkseq = 0;  //第一块 "PONO"块，f_pssm12_time_calc()函数用
		inBlock.Tables[blkseq].set_TableName("PONO");
		//inBlock.Tables[blkseq].Columns.Add(tpssm11);   //从实体对象创建架构
		inBlock.Tables[blkseq].Columns.Add(DT_STRING, "FACTORY_DIV");//制造命令号
		inBlock.Tables[blkseq].Columns.Add(DT_STRING, "PONO");//制造命令号

		blkNum = bcls_rec->Tables.IndexOf("PLAN");
		if (blkNum <= 0)
		{
			bcls_rec->Tables.Add("PLAN");
			blkNum = bcls_rec->Tables.IndexOf("PLAN");
		}
		else
		{
			bcls_rec->Tables["PLAN"].Clear();
		}
		bcls_rec->Tables["PLAN"].Columns.Add(DT_STRING, "FACTORY_DIV");


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
			v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString();
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"].ToDecimal();
			v_pono       = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString();
			////Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}], PONO=[{1}]", v_sm_plan_no, v_pono);

			//工序修改后，时刻推算用数据
			CDataRow &row = inBlock.Tables["PONO"].Rows.Add();
			row["FACTORY_DIV"] = v_factory_div;
			row["PONO"] = v_pono;

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

		bcls_rec->Tables["PLAN"].Rows.Add();

		bcls_rec->Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;

		//----------------------------------------------------------
		//4.根据作业时刻，赋流水号（预定炉号）
		ret = f_pssm12_sequ_calc_n(bcls_rec, bcls_ret, conn);
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
