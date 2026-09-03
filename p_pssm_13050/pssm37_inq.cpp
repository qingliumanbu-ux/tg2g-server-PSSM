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
BM2F_ENTERACE(pssm37_inq)

int f_pssm37_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	CString cs_oder_flag = " ";


	CModel tpssm10("TPSSM10");

	CDbCommand cmd_inq(conn);

	try
	{

		//--------------------------------
		//获取传入参数 判断排序条件的传入参数
		

		cs_oder_flag = bcls_rec->Tables[0].Rows[0]["ODER_FLAG"].ToString();
		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "cs_oder_flag  =[{0}]", cs_oder_flag);
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]",tpssm10["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "cc_mach_no        = [{0}]",tpssm10["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "cast_lot_no       = [{0}]",tpssm10["CAST_LOT_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status       = [{0}]",tpssm10["PONO_STATUS"].ToDecimal().ToInt32());

		
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT T.PONO_STATUS,T.SM_PLAN_NOL2,T.REFINE_ROUTE_CODE,T.ST_NO,T.HEAT_NO,T.CURR_WP_NO,T.CAST_DIV_NO AS ST_D_COUNT,    "
					" (SELECT COUNT(1) FROM (SELECT CAST_NO FROM TPSSM11 WHERE CAST_NO = T.CAST_NO  UNION ALL SELECT CAST_NO FROM TPSSM41 WHERE CAST_NO = T.CAST_NO)) ST_C_COUNT,  "
					" DECODE(T1.START_TIME_REAL, ' ', T1.START_TIME, T1.START_TIME_REAL) AS  SMELT_START_TIME, DECODE(T1.END_TIME_REAL, ' ', T1.END_TIME, T1.END_TIME_REAL) AS SMELT_END_TIME, T1.DEV_CODE AS SR8_NO,   "
					" CASE WHEN T1.START_TIME_REAL<>' ' AND T1.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T1.START_TIME_REAL <> ' ' AND T1.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END MAIN_SMELT_FLAG,   "
					" DECODE(T2.START_TIME_REAL, ' ', T2.START_TIME, T2.START_TIME_REAL) AS  SR2_TIME, DECODE(T2.END_TIME_REAL, ' ', T2.END_TIME, T2.END_TIME_REAL) AS SR2_END_TIME, T2.DEV_CODE AS SR2_NO,   "
					" CASE WHEN T2.START_TIME_REAL<>' ' AND T2.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T2.START_TIME_REAL <> ' ' AND T2.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END SR2_FLAG,   "
					" DECODE(T3.START_TIME_REAL, ' ', T3.START_TIME, T3.START_TIME_REAL) AS  SR1_TIME, DECODE(T3.END_TIME_REAL, ' ', T3.END_TIME, T3.END_TIME_REAL) AS SR1_END_TIME, T3.DEV_CODE AS SR1_NO,   "
					" CASE WHEN T3.START_TIME_REAL<>' ' AND T3.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T3.START_TIME_REAL <> ' ' AND T3.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END SR1_FLAG,   "
					" DECODE(T4.START_TIME_REAL, ' ', T4.START_TIME, T4.START_TIME_REAL) AS  CC_BEGIN_TIME, DECODE(T4.END_TIME_REAL, ' ', T4.END_TIME, T4.END_TIME_REAL) AS CC_END_TIME, T4.DEV_CODE AS CC_MACH_NO,   "
					" CASE WHEN T4.START_TIME_REAL<>' ' AND T4.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T4.START_TIME_REAL <> ' ' AND T4.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END CC_FLAG,   "
					" DECODE(T5.START_TIME_REAL, ' ', T5.START_TIME, T5.START_TIME_REAL) AS  SR3_TIME, DECODE(T5.END_TIME_REAL, ' ', T5.END_TIME, T5.END_TIME_REAL) AS SR3_END_TIME, T5.DEV_CODE AS SR3_NO,   "
					" CASE WHEN T5.START_TIME_REAL<>' ' AND T5.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T5.START_TIME_REAL <> ' ' AND T5.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END SR3_FLAG,   "
					" DECODE(T6.START_TIME_REAL, ' ', T6.START_TIME, T6.START_TIME_REAL) AS  SR4_TIME, DECODE(T6.END_TIME_REAL, ' ', T6.END_TIME, T6.END_TIME_REAL) AS SR4_END_TIME, T6.DEV_CODE AS SR4_NO,   "
					" CASE WHEN T6.START_TIME_REAL<>' ' AND T6.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T6.START_TIME_REAL <> ' ' AND T6.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END SR4_FLAG,   "
					" DECODE(T7.START_TIME_REAL, ' ', T7.START_TIME, T7.START_TIME_REAL) AS  SY1_TIME, DECODE(T7.END_TIME_REAL, ' ', T7.END_TIME, T7.END_TIME_REAL) AS SY1_END_TIME, T7.DEV_CODE AS SY1_NO,   "
					" CASE WHEN T7.START_TIME_REAL<>' ' AND T7.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T7.START_TIME_REAL <> ' ' AND T7.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END SY1_FLAG,   "
					" DECODE(T8.START_TIME_REAL, ' ', T8.START_TIME, T8.START_TIME_REAL) AS  SY2_TIME, DECODE(T8.END_TIME_REAL, ' ', T8.END_TIME, T8.END_TIME_REAL) AS SY2_END_TIME, T8.DEV_CODE AS SY2_NO,   "
					" CASE WHEN T8.START_TIME_REAL<>' ' AND T8.END_TIME_REAL<>' ' THEN '2' ELSE CASE WHEN T8.START_TIME_REAL <> ' ' AND T8.END_TIME_REAL = ' ' THEN  '1' ELSE '0' END END SY2_FLAG   "
					" FROM  TPSSM11 T   "
					" LEFT JOIN TPSSM12 T1 ON T.SM_PLAN_NO = T1.SM_PLAN_NO  AND T1.DEV_CODE LIKE 'B%'   "
					" LEFT JOIN TPSSM12 T2 ON T.SM_PLAN_NO = T2.SM_PLAN_NO  AND T2.DEV_CODE LIKE 'A%'   "
					" LEFT JOIN TPSSM12 T3 ON T.SM_PLAN_NO = T3.SM_PLAN_NO AND T3.DEV_CODE LIKE 'E%' AND  T3.CHARGE_NO = (CASE WHEN T.PONO_STATUS<24 THEN(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE SM_PLAN_NO = T.SM_PLAN_NO)   "
					" ELSE(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE SM_PLAN_NO = T.SM_PLAN_NO) END)   "
					" LEFT JOIN TPSSM12 T4 ON T.SM_PLAN_NO = T4.SM_PLAN_NO AND T4.DEV_CODE LIKE 'C%'   "
					" LEFT JOIN TPSSM12 T5 ON T.SM_PLAN_NO = T5.SM_PLAN_NO AND T5.DEV_CODE LIKE 'S%' AND  T5.CHARGE_NO = (CASE WHEN T.CURR_WP_NO + 1<(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'S%' AND SM_PLAN_NO = T.SM_PLAN_NO)   "
					" THEN(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'S%' AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(CASE WHEN T.CURR_WP_NO + 1 >(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'S%' AND SM_PLAN_NO = T.SM_PLAN_NO )   "
					" THEN(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'S%' AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'S%' AND  CHARGE_NO >= T.CURR_WP_NO + 1 AND SM_PLAN_NO = T.SM_PLAN_NO) END)  END)   "
					" LEFT JOIN TPSSM12 T6 ON T.SM_PLAN_NO = T6.SM_PLAN_NO AND T6.DEV_CODE LIKE 'V%' AND  T6.CHARGE_NO = (CASE WHEN T.CURR_WP_NO + 1<(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'V%' AND SM_PLAN_NO = T.SM_PLAN_NO)   "
					" THEN(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'V%' AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(CASE WHEN T.CURR_WP_NO + 1 >(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'V%' AND SM_PLAN_NO = T.SM_PLAN_NO)    "
					" THEN(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'V%' AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'V%' AND  CHARGE_NO >= T.CURR_WP_NO + 1 AND SM_PLAN_NO = T.SM_PLAN_NO) END)  END)   "
					" LEFT JOIN TPSSM12 T7 ON T.SM_PLAN_NO = T7.SM_PLAN_NO AND T7.DEV_CODE LIKE 'F%' AND  T7.CHARGE_NO = (CASE WHEN T.CURR_WP_NO + 1<(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'F%' AND SM_PLAN_NO = T.SM_PLAN_NO)   "
					" THEN(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'F%'AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(CASE WHEN T.CURR_WP_NO + 1 >(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'F%' AND SM_PLAN_NO = T.SM_PLAN_NO )   "
					" THEN(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'F%'AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'F%' AND  CHARGE_NO >= T.CURR_WP_NO + 1 AND SM_PLAN_NO = T.SM_PLAN_NO) END)  END)   "
					" LEFT JOIN TPSSM12 T8 ON T.SM_PLAN_NO = T8.SM_PLAN_NO AND T8.DEV_CODE LIKE 'R%' AND  T8.CHARGE_NO = (CASE WHEN T.CURR_WP_NO + 1<(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'R%' AND SM_PLAN_NO = T.SM_PLAN_NO)   "
					" THEN(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'R%' AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(CASE WHEN T.CURR_WP_NO + 1 >(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'R%' AND SM_PLAN_NO = T.SM_PLAN_NO)   "
					" THEN(SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'R%' AND SM_PLAN_NO = T.SM_PLAN_NO) ELSE(SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE LIKE 'R%' AND  CHARGE_NO >= T.CURR_WP_NO + 1 AND SM_PLAN_NO = T.SM_PLAN_NO) END)  END)   ";
				if (cs_oder_flag =="BOF")
				{

					sqlstr_temp_order = "  ORDER BY T1.DEV_CODE ASC ,SMELT_START_TIME ASC  ";

				}
				else if(cs_oder_flag == "AOD")
				{
					sqlstr_temp_order = "  ORDER BY T2.DEV_CODE ASC ,SR2_TIME ASC  ";

				}
				else if (cs_oder_flag == "EAF")
				{
					sqlstr_temp_order = "  ORDER BY T3.DEV_CODE ASC ,SR1_TIME ASC  ";

				}
				else if (cs_oder_flag == "LTS")
				{
					sqlstr_temp_order = "  ORDER BY T5.DEV_CODE ASC ,SR3_TIME ASC  ";

				}
				else if (cs_oder_flag == "VOD")
				{
					sqlstr_temp_order = "  ORDER BY T6.DEV_CODE ASC ,SR4_TIME ASC  ";

				}
				else if (cs_oder_flag == "LF")
				{
					sqlstr_temp_order = "  ORDER BY T7.DEV_CODE ASC ,SY1_TIME ASC  ";

				}
				else if (cs_oder_flag == "RH")
				{
					sqlstr_temp_order = "  ORDER BY T8.DEV_CODE ASC ,SY2_TIME ASC  ";

				}
				else if (cs_oder_flag == "CCM")
				{
					sqlstr_temp_order = "  ORDER BY T4.DEV_CODE ASC ,CC_BEGIN_TIME ASC  ";

				}
				
				
				sqlstr = sqlstr  + sqlstr_temp_order;
				break;
			}

			//分页获取
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
			for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
			{
				for (int j = 0; j < bcls_ret->Tables[0].Columns.get_Count(); j++)
				{
					if (bcls_ret->Tables[0].Rows[i][j].ToString().GetLength() == 14)
					{
						bcls_ret->Tables[0].Rows[i][j] = bcls_ret->Tables[0].Rows[i][j].ToString().SubstringNE(8, 2) + ":" + bcls_ret->Tables[0].Rows[i][j].ToString().SubstringNE(10, 2);
					}
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
