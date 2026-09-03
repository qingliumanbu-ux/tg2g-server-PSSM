/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-18
Description: 出钢计划画面初始化查询
**************************************************/
#include "stdafx.h"

//#include "tpssmd1.h"

int f_pssm_pono_status_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //出钢计划PONO状态代码查询

/*<remark>=========================================================
/// <summary>
/// 出钢计划画面初始化查询
/// <para>1.根据传入的炼钢单元号，查询当前铸机信息。</para>
/// <para>数据库表：TPSSMD1(炼钢设备配置表)                    </para>
/// <para>主调用函数：前台PSSM10P画面加载时调用。              </para>
/// </summary>
/// <returns>连铸机号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_init)


int f_pssm11_init(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blknum;

	/* 业务变量 */
	CString  carry_div = "";

	// 定义表的实体对象
	//CTPSSMD1 tpssmd1(conn);

	CString sqlstr = "";

	CDbCommand cmd_inq(conn);

	try
	{

		//--------------------------------
		//设定返回查询记录信息结构
		//blknum = 0; //第1块
		//bcls_ret->Tables[blknum].set_TableName("CAST_DEV");  //与Client端dsPSSM10P.CAST_DEV表名一致
		//bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_MACH_NO");  //连铸机号
		//bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DEV_CODE");
		//bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DEV_DESC");



		//---------------------------------------------------
		//获得输入参数


		//------------------------------------------
		//获取制造命令状态代码定义
		ret = f_pssm_pono_status_inq(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			//不报错
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
