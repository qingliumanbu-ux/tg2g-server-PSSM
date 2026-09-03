/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM09画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12v_inq)

int f_pssm12v_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	CString v_CC_PLAN_MODE = "";

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int		TotalRecordCount = 0;


	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm10("TPSSM10");

	CDbCommand cmd_inq(conn);

	try
	{
		sqlstr = " SELECT * FROM (  "
			"  SELECT SM_PLAN_NO, SPLIT_INDICATION, 0 TREATMENT_COUNTER, ' ' DEV_CODE, ' ' START_TIME, ' ' END_TIME, ST_NO, (SELECT C_DIV FROM TPSSM10 WHERE PONO = B.PONO) C_DIV,  "
			" ' ' START_TIME_REAL, ' ' END_TIME_REAL, ' 'STATUS, 0  AREA_ID FROM TPSSM11 B   WHERE  B.RUN_STATUS < 53  "
		"	UNION ALL "
		"  SELECT T.SM_PLAN_NO SM_PLAN_NO, T.SPLIT_INDICATION  SPLIT_INDICATION, T.TREATMENT_COUNTER TREATMENT_COUNTER, T.DEV_CODE DEV_CODE, T.START_TIME START_TIME, T.END_TIME END_TIME "
		"  , A.ST_NO ST_NO, (SELECT C_DIV FROM TPSSM10 WHERE PONO = A.PONO) C_DIV,"
		"   T.START_TIME_REAL START_TIME_REAL, T.END_TIME_REAL END_TIME_REAL, DECODE(T.START_TIME_REAL, ' ', '未生产', '已生产') STATUS, T.AREA_ID AREA_ID  FROM TPSSM12 T LEFT JOIN  "
		"   TPSSM11 A ON  A.SM_PLAN_NO = T.SM_PLAN_NO WHERE  A.RUN_STATUS < 53   "
		"	) AAA ORDER BY AAA.SM_PLAN_NO, AAA.AREA_ID   "
				;

		//分页获取
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

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
