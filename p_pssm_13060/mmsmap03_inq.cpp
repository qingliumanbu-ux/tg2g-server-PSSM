/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一炼钢板坯日库存信息查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** 头文件部分 *****/

//#include "AppFunc.h"

/*<remark>=========================================================
/// <summary>
///  一系列生产计划查询
/// <para>
/// </para>
/// <para>数据库表：</para>
/// </summary>
/// <param name="">  </param>
/// <returns>返回参数：材料数据</returns>
===========================================================</remark>*/
// Service 入口
BM2F_ENTERACE(mmsmap03_inq)

int f_mmsmap03_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_factory_div("A1");
	CString v_date_time("");
	CString v_datetime_now = "";
	//CDecimal  cmd_flag = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sql("");
	CString sqlwhere("");
	CString code_class("");
	CString v_mat_destion = "";
	CString v_mat_shape_flag = "";
	CDecimal v_mat_act_num = 0;
	CDecimal v_mat_act_wt = 0;
	CDecimal v_mat_act_num_all = 0;
	CDecimal v_mat_act_wt_all = 0;

	CDecimal v_mat_act_num_j1 = 0;
	CDecimal v_mat_act_wt_j1 = 0;
	CDecimal v_mat_act_num_j2 = 0;
	CDecimal v_mat_act_wt_j2 = 0;
	CDecimal v_mat_act_num_h1 = 0;
	CDecimal v_mat_act_wt_h1 = 0;
	CDecimal v_mat_act_num_wx = 0;
	CDecimal v_mat_act_wt_wx = 0;

	/* 实体类定义 */


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		//v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();

		v_datetime_now = CDateTime::Now().AddDays(-1).ToString("yyyyMMdd");


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_date_time[{1}] =======  ", v_factory_div, v_date_time);


		//---------------------------------------------------
		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_4100_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_2700_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_1780_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_TAKEOUT_NUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_SUM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_4100_WT");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_2700_WT");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_1780_WT");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_TAKEOUT_WT");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_WT");

		if (v_date_time = v_datetime_now)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				sqlstr =
					" SELECT MAT_DESTION, MAT_SHAPE_FLAG, COUNT(1), SUM(MAT_ACT_WT) "
					"  FROM (SELECT MAT_NO, "
					"               DECODE(NEXT_WHOLE_BACKLOG_CODE, "
					"                      '9A', "
					"                      '20', "
					"                      '9B', "
					"                      '20', "
					"                      MAT_DESTION) AS MAT_DESTION, "
					"               MAT_SHAPE_FLAG, "
					"               MAT_ACT_WT "
					"          FROM TMMSM01 T "
					"         WHERE T.MAT_LINE_TYPE = 'SM' "
					"           AND T.MAT_SHAPE_FLAG = '1') "
					" GROUP BY MAT_DESTION, MAT_SHAPE_FLAG ";

				break;
			case DB_KIND_MSSQL:			// MS SQL Server数据库
				break;
			case DB_KIND_ORACLE:		// Oracle 数据库
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
			cmd_inq.ExecuteReader();

			while (cmd_inq.Read())
			{
				v_mat_destion = cmd_inq.GetString(1);
				v_mat_shape_flag = cmd_inq.GetString(2);
				v_mat_act_num = cmd_inq.GetDecimal(3);
				v_mat_act_wt = cmd_inq.GetDecimal(4);

				if (v_mat_destion == "15")
				{
					v_mat_act_num_j1 = v_mat_act_num;
					v_mat_act_wt_j1 = v_mat_act_wt;
				}

				if (v_mat_destion == "16")
				{
					v_mat_act_num_j2 = v_mat_act_num;
					v_mat_act_wt_j2 = v_mat_act_wt;
				}

				if (v_mat_destion == "14")
				{
					v_mat_act_num_h1 = v_mat_act_num;
					v_mat_act_wt_h1 = v_mat_act_wt;
				}

				if (v_mat_destion == "20")
				{
					v_mat_act_num_wx = v_mat_act_num;
					v_mat_act_wt_wx = v_mat_act_wt;
				}

				v_mat_act_num_all =+ v_mat_act_num;
				v_mat_act_wt_all =+ v_mat_act_wt;
			}
			cmd_inq.Close();
		}
		else
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				sqlstr =
					" SELECT MAT_DESTION, MAT_SHAPE_FLAG, COUNT(1), SUM(MAT_ACT_WT) "
					"  FROM (SELECT MAT_NO, "
					"               DECODE(NEXT_WHOLE_BACKLOG_CODE, "
					"                      '9A', "
					"                      '20', "
					"                      '9B', "
					"                      '20', "
					"                      MAT_DESTION) AS MAT_DESTION, "
					"               MAT_SHAPE_FLAG, "
					"               MAT_ACT_WT "
					"          FROM TMMSM01_KC T "
					"         WHERE T.MAT_LINE_TYPE = 'SM' "
					"           AND T.MAT_SHAPE_FLAG = '1' "
					"           AND T.REPORT_DATE = @v_date_time "
					"           AND T.PRODUCT_CODE IN "
					"               (SELECT CODE "
					"                  FROM TEP0002 "
					"                 WHERE CODE_CLASS = 'MMZC' "
					"                   AND CODE_DESC_3_CONTENT IN ('0', '1'))) "
					" GROUP BY MAT_DESTION, MAT_SHAPE_FLAG ";

				break;
			case DB_KIND_MSSQL:			// MS SQL Server数据库
				break;
			case DB_KIND_ORACLE:		// Oracle 数据库
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);

			cmd_inq.Parameters.Set("v_date_time", v_date_time);
			cmd_inq.ExecuteReader();

			while (cmd_inq.Read())
			{
				v_mat_destion = cmd_inq.GetString(1);
				v_mat_shape_flag = cmd_inq.GetString(2);
				v_mat_act_num = cmd_inq.GetDecimal(3);
				v_mat_act_wt = cmd_inq.GetDecimal(4);

				if (v_mat_destion == "15")
				{
					v_mat_act_num_j1 = v_mat_act_num;
					v_mat_act_wt_j1 = v_mat_act_wt;
				}

				if (v_mat_destion == "16")
				{
					v_mat_act_num_j2 = v_mat_act_num;
					v_mat_act_wt_j2 = v_mat_act_wt;
				}

				if (v_mat_destion == "14")
				{
					v_mat_act_num_h1 = v_mat_act_num;
					v_mat_act_wt_h1 = v_mat_act_wt;
				}

				if (v_mat_destion == "20")
				{
					v_mat_act_num_wx = v_mat_act_num;
					v_mat_act_wt_wx = v_mat_act_wt;
				}

				v_mat_act_num_all =+ v_mat_act_num;
				v_mat_act_wt_all =+ v_mat_act_wt;
			}
			cmd_inq.Close();
		}

		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["SLAB_4100_NUM"] = v_mat_act_wt_j1;
		row["SLAB_2700_NUM"] = v_mat_act_wt_j2;
		row["SLAB_1780_NUM"] = v_mat_act_wt_h1;
		row["SLAB_TAKEOUT_NUM"] = v_mat_act_wt_wx;
		row["SLAB_SUM"] = v_mat_act_wt_all;
	}
	catch (CDbException& ex)					//捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)	//捕获应用错误
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

	return(doFlag);
}