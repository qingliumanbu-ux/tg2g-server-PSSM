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
int f_pssm_deal_pre_plan(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn)
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
		tpssm11.Query("SM_PLAN_NO");

		/*sqlstr = CString(
			" UPDATE																											"
			" TPSSM12																											"
			" SET																												"
			" START_TIME_REAL = (																								"
			" SELECT																											"
			" START_TIME																										"
			" FROM																												"
			" (																													"
			" SELECT																											"
			" START_TIME, CHARGE_NO																								"
			" FROM																												"
			" TPSSM12																											"
			" WHERE																												"
			" AREA_ID < 3																										"
			" AND SM_PLAN_NO = @sm_plan_no																						"
			" ORDER BY																											"
			" CHARGE_NO ASC) A																									"
			" WHERE																												"
			" TPSSM12.CHARGE_NO = A.CHARGE_NO),																					"
			" END_TIME_REAL = (																									"
			" SELECT																											"
			" START_TIME_R																										"
			" FROM																												"
			" (																													"
			" SELECT																											"
			" to_char(TO_DATE(START_TIME, 'yyyymmddhh24miss') + 1 / 24 / 60, 'yyyymmddhh24miss') AS START_TIME_R, CHARGE_NO		"
			" FROM																												"
			" TPSSM12																											"
			" WHERE																												"
			" AREA_ID < 3																										"
			" AND SM_PLAN_NO = @sm_plan_no																						"
			" ORDER BY																											"
			" CHARGE_NO ASC) B																									"
			" WHERE																												"
			" TPSSM12.CHARGE_NO = B.CHARGE_NO)																					"
			" WHERE																												"
			" AREA_ID < 3																										"
			" AND SM_PLAN_NO = @sm_plan_no																						"
			" AND START_TIME_REAL = ' '																							"
			" AND END_TIME_REAL = ' '																							"
			);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sm_plan_no", tpssm12["SM_PLAN_NO"].ToString().Trim());
		cmd_inq.ExecuteNonQuery();*/

		sqlstr = " SELECT CHARGE_NO FROM TPSSM12 WHERE	AREA_ID < 3	AND SM_PLAN_NO = @sm_plan_no AND START_TIME_REAL = ' ' AND END_TIME_REAL = ' ' ORDER BY CHARGE_NO desc ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sm_plan_no", tpssm12["SM_PLAN_NO"].ToString().Trim());
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			charge_no = cmd_inq.GetDecimal(1);
			if (tpssm11["CURR_WP_NO"].ToDecimal() > charge_no)
			{
				//tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["CHARGE_NO"] = charge_no;
				tpssm12.Query("SM_PLAN_NO,CHARGE_NO");

				if (tpssm12["AREA_ID"].ToDecimal() == 4 || tpssm12["AREA_ID"].ToDecimal() == 2)
				{
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "" && tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{
						tpssm12.Delete("SM_PLAN_NO,CHARGE_NO");

						sqlstr = "UPDATE TPSSM12 "
							"	SET	CHARGE_NO = CHARGE_NO - 1 "
							"	WHERE SM_PLAN_NO = @sm_plan_no "
							"	AND CHARGE_NO > @charge_no ";

						cmd_upd.Parameters.Set("sm_plan_no", tpssm12["SM_PLAN_NO"].ToString());
						cmd_upd.Parameters.Set("charge_no", charge_no);
						cmd_upd.SetCommandText(sqlstr);
						cmd_upd.ExecuteNonQuery();

						sqlstr = "UPDATE TPSSM11 "
							"	SET	CURR_WP_NO = CURR_WP_NO - 1 "
							"	WHERE SM_PLAN_NO = @sm_plan_no ";

						cmd_upd.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
						cmd_upd.SetCommandText(sqlstr);
						cmd_upd.ExecuteNonQuery();
						cmd_upd.Close();
					}
					else
					{
						CFormattable arguments[] = { tpssm12["START_TIME_REAL"].ToString(), tpssm12["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "存在实绩时间，实绩开始时间[{0}],实绩结束时间[{1}]", arguments, 2); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else
				{
					CFormattable arguments[] = { tpssm12["AREA_ID"].ToDecimal() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "不允许删除脱碳或者连铸设备", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (tpssm11["CURR_WP_NO"].ToDecimal() < charge_no)
			{
				//tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["CHARGE_NO"] = charge_no;
				tpssm12.Query("SM_PLAN_NO,CHARGE_NO");
				if (tpssm12["AREA_ID"].ToDecimal() == 4 || tpssm12["AREA_ID"].ToDecimal() == 2)
				{
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "" && tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{
						tpssm12.Delete("SM_PLAN_NO,CHARGE_NO");

						sqlstr = "UPDATE TPSSM12 "
							"	SET	CHARGE_NO = CHARGE_NO - 1 "
							"	WHERE SM_PLAN_NO = @sm_plan_no "
							"	AND CHARGE_NO > @charge_no ";

						cmd_upd.Parameters.Set("sm_plan_no", tpssm12["SM_PLAN_NO"].ToString());
						cmd_upd.Parameters.Set("charge_no", charge_no);
						cmd_upd.SetCommandText(sqlstr);
						cmd_upd.ExecuteNonQuery();
						cmd_upd.Close();
					}
					else
					{
						CFormattable arguments[] = { tpssm12["START_TIME_REAL"].ToString(), tpssm12["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "存在实绩时间，实绩开始时间[{0}],实绩结束时间[{1}]", arguments, 2); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else
				{
					CFormattable arguments[] = { tpssm12["AREA_ID"].ToDecimal() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "不允许删除脱碳或者连铸设备", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}
		cmd_inq.Close();

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
