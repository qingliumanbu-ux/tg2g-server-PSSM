/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Version:    1.0
Date:     2015-09-10
Description: 炉次强制确定
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

int f_plan_delete_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/***** C++ 的业务头文件部分 *****/

/* ***** 静态函数申明 ***** */

/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 炉次强制确定
/// <para>
/// 1.炉次强制确定。
///
/// </para>
/// <para>数据库表：TPSSM11(出钢计划表)					</para>
/// <para>主调用函数：前台PSSM91画面F7按钮				</para>
/// </summary>
/// <param name=" ">     </param>
/// <param name=" ">                </param>
/// <returns>  </returns>
===========================================================</remark>*/
// service入口


BM2F_ENTERACE(pssm91f8_proc)

int f_pssm91f8_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{

	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr;
	EIClass inBlock;

	inBlock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inBlock.Tables[0].Rows.Add();  //只生成一行

	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			inBlock.Tables[0].Rows[0]["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[i]["SM_PLAN_NO"].ToString();
			//inBlock.Tables[0].Rows[0]["SM_PLAN_NO_IN"] = tpssm11["SM_PLAN_NO"];
			ret = f_plan_delete_snd2(&inBlock, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
