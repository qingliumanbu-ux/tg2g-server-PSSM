/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2015-08-18
Description: 出钢计划脱碳转炉炉号修改
**************************************************/
#include "stdafx.h"

#include "tpssm11.h"
#include "tpssm12.h"
#include "tpssmd1.h"

/*<remark>=========================================================
/// <summary>
/// 出钢计划脱碳转炉炉号修改
/// <para>
/// 1.校验输入计划状态
/// 2.校验输入炉号是否存在
/// 3.修改转炉工序的炉号及整体计划状态
/// </para>
/// <para>数据库表：TPSSMD1(炼钢设备配置表)                    </para>
/// <para>主调用函数：前台PSSM11画面炉号列数据项修改。         </para>
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_devno)


int f_pssm11_devno(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq = 0, rownum = 0, i = 0; //读取传入参数用
	CString date_time = "";

	/* 业务变量 */
	CString  dc_dev_no = "";
	CDecimal sm_plan_no = 0;

	// 定义表的实体对象
	CTPSSM11 tpssm11(conn);
	CTPSSM12 tpssm12(conn);
	CTPSSMD1 tpssmd1(conn);


	CString sqlstr = "";
	CDbCommand cmd_upd(conn);

	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//---------------------------------------------------
		//获得输入参数（单记录操作）
		blkseq = 0;
		i = 0;
		dc_dev_no  = bcls_rec->Tables[blkseq].Rows[i]["DC_DEV_NO"].ToString().Trim();
		sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"].ToDecimal().ToInt32();

		Log::Info("", __FUNCTION__, "dc_dev_no=[{0}], sm_plan_no=[{1}]", dc_dev_no, sm_plan_no);


		//1.校验输入转炉号是否正确
		tpssmd1.DEV_CODE = "B" + dc_dev_no;
		bool hasd1 = tpssmd1.Query("DEV_CODE");
		if (hasd1 == false)
		{
			CFormattable arguments[] = { dc_dev_no }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "输入的转炉炉号[{0}]不正确。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//2.校验输入计划状态
		tpssm11.SM_PLAN_NO = sm_plan_no;
		sqlstr = "tpssm11.Query()";
		bool has11 = tpssm11.Query("SM_PLAN_NO");
		if (has11 == false)
		{
			CFormattable arguments[] = { sm_plan_no.ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划号[{0}]不存在，转炉炉号不能修改。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpssm11.PONO_STATUS > 30)
		{
			CFormattable arguments[] = { sm_plan_no.ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划号[{0}]转炉已作业，不能修改炉号。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//------------------------------------------
		//修改炉号
		tpssm12.SM_PLAN_NO = sm_plan_no;
		tpssm12.AREA_ID = 3;  //3-转炉工序
		tpssm12.DEV_CODE = tpssmd1.DEV_CODE;

		sqlstr = "tpssm12.Query()";
		tpssm12.Update("DEV_CODE", "SM_PLAN_NO,AREA_ID");

		
		//----------------------------------------------------------
		//修改出钢计划应答表的计划编辑标记（整体计划状态）
		sqlstr = CString(
			" UPDATE TPSSM23 SET "
			" PLAN_EDIT_FLAG = '4' " //4-设备调整；
			",PLAN_EDIT_TIME = @date_time " //计划编辑时刻
			);
		cmd_upd.SetCommandText(sqlstr);
		cmd_upd.Parameters.Set("date_time", date_time);
		cmd_upd.ExecuteNonQuery();



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
