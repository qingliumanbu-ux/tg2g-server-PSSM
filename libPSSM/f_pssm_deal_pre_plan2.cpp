/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-16
Version:1.0
Description:
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件


BM2_FUNCTION_EXPORT
int f_pssm_deal_pre_plan2(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	CString datetime = "";
	CString sqlstr = "";
	CDecimal charge_no = 0;      //工序charge号

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得输入参数
		tpssm12["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"];
		tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"];
		//tpssm11.Query("SM_PLAN_NO");

		//sqlstr = CString(
		//" UPDATE																											"
		//" TPSSM12																											"
		//" SET																												"
		//" START_TIME_REAL = (																								"
		//" SELECT																											"
		//" START_TIME																										"
		//" FROM																												"
		//" (																													"
		//" SELECT																											"
		//" START_TIME, CHARGE_NO																								"
		//" FROM																												"
		//" TPSSM12																											"
		//" WHERE																												"
		//" AREA_ID = 3																										"
		//" AND SM_PLAN_NO = @sm_plan_no																						"
		//" ORDER BY																											"
		//" CHARGE_NO ASC) A																									"
		//" WHERE																												"
		//" TPSSM12.CHARGE_NO = A.CHARGE_NO),																					"
		//" END_TIME_REAL = (																									"
		//" SELECT																											"
		//" START_TIME_R																										"
		//" FROM																												"
		//" (																													"
		//" SELECT																											"
		//" START_TIME AS START_TIME_R, CHARGE_NO		"
		////" to_char(TO_DATE(START_TIME, 'yyyymmddhh24miss') + 1 / 24 / 60, 'yyyymmddhh24miss') AS START_TIME_R, CHARGE_NO		"
		//" FROM																												"
		//" TPSSM12																											"
		//" WHERE																												"
		//" AREA_ID = 3																										"
		//" AND SM_PLAN_NO = @sm_plan_no																						"
		//" ORDER BY																											"
		//" CHARGE_NO ASC) B																									"
		//" WHERE																												"
		//" TPSSM12.CHARGE_NO = B.CHARGE_NO)																					"
		//" WHERE																												"
		//" AREA_ID = 3																										"
		//" AND SM_PLAN_NO = @sm_plan_no																						"
		//" AND START_TIME_REAL = ' '																							"
		//" AND END_TIME_REAL = ' '																							"
		//);

		sqlstr = " UPDATE TPSSM12 SET "
			" START_TIME_REAL = TO_CHAR(SYSDATE - 1 / 24 / 60, 'YYYYMMDDHH24MISS'), "
			" END_TIME_REAL = TO_CHAR(SYSDATE - 1 / 24 / 60, 'YYYYMMDDHH24MISS') "
			" WHERE AREA_ID = 3 AND SM_PLAN_NO = @sm_plan_no AND START_TIME_REAL = ' ' AND END_TIME_REAL = ' '";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sm_plan_no", tpssm12["SM_PLAN_NO"].ToString().Trim());
		cmd_inq.ExecuteNonQuery();

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
