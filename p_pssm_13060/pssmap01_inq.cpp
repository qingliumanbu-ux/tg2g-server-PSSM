/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      zhengqiangqiang
Version:     1.0
Date:        2020/7/6 16:08:19
Description: 一系列生产计划查询
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
BM2F_ENTERACE(pssmap01_inq)

int f_pssmap01_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CDecimal  cmd_flag = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sql("");
	CString sqlwhere("");
	CString code_class("");
	CString v_cc_mach_no = "";
	CString v_station_no = "";
	CString v_factory_div = "A1";
	CDecimal v_slab_wt = 0;
	CDecimal v_plan_wt_c1 = 0;
	CDecimal v_plan_wt_c2 = 0;
	CDecimal v_plan_wt_c3 = 0;
	CDecimal v_plan_wt_c4 = 0;
	CDecimal v_prod_wt_c1 = 0;
	CDecimal v_prod_wt_c2 = 0;
	CDecimal v_prod_wt_c3 = 0;
	CDecimal v_prod_wt_c4 = 0;
	CString v_plan_desc = "";
	CString v_mat_spec_c1 = "";
	CString v_mat_spec_c2 = "";
	CString v_mat_spec_c3 = "";
	CString v_mat_spec_c4 = "";
	CString v_date_time = "";
	CString v_date_time_1 = "";
	CDecimal v_heat_num_b1 = 0;
	CDecimal v_heat_num_b2 = 0;
	CDecimal v_heat_num_b3 = 0;
	CDecimal v_prod_wt_b1 = 0;
	CDecimal v_prod_wt_b2 = 0;
	CDecimal v_prod_wt_b3 = 0;
	CDecimal v_tpc_num_a1 = 0;
	CDecimal v_tpc_wt_a1 = 0;

	CDecimal v_out_steel_sum = 0;
	CDecimal v_out_steel_wt = 0;

	CString v_cc_mach_no_pre = "";
	CString v_plan_desc_pre = "";
	/* 实体类定义 */
	CModel tpssm10("TPSSM10");

	
	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		//v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();
		v_date_time_1 = CDateTime::Parse(v_date_time + "000000").AddMinutes(-1).ToString("yyyyMMdd");

		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_factory_div[{0}],v_date_time[{1}] =======  ", v_factory_div, v_date_time);
		
		//---------------------------------------------------
		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_WT_C1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_C1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C1");

		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_WT_C2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_C2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C2");

		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_WT_C3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_C3");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C3");

		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_WT_C4");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_C4");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C4");

		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TPC_NUM_A1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TPC_WT_A1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "HEAT_NUM_B1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_B1");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "HEAT_NUM_B2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_B2");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "HEAT_NUM_B3");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT_B3");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr =
				" SELECT A.CC_MACH_NO, SUM(B.SLAB_WT) "
				"  FROM TPSSM01 A, TPSSM03 B "
				" WHERE A.PONO = B.PONO "
				"   AND A.FACTORY_DIV = @v_factory_div "
				"   AND A.PLAN_DATE = @v_date_time "
				" GROUP BY A.CC_MACH_NO ";
			
			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}
		
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())
		{
			v_cc_mach_no = cmd_inq.GetString(1);
			v_slab_wt = cmd_inq.GetDecimal(2);

			if (v_cc_mach_no == "1")
			{
				v_plan_wt_c1 = v_slab_wt;
			}

			if (v_cc_mach_no == "2")
			{
				v_plan_wt_c2 = v_slab_wt;
			}

			if (v_cc_mach_no == "3")
			{
				v_plan_wt_c3 = v_slab_wt;
			}

			if (v_cc_mach_no == "4")
			{
				v_plan_wt_c4 = v_slab_wt;
			}
		}
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr = " SELECT CC_MACH_NO, "
				"        ST_NO || '*' || TO_CHAR(CEIL(SLAB_THICK)) || '*' || TO_CHAR(CEIL(SLAB_WIDTH)) || '(' ||COUNT(1) || ')'"
				"   FROM (SELECT DISTINCT A.CC_MACH_NO, "
				"                         A.PONO, "
				"                         A.ST_NO, "
				"                         B.SLAB_THICK, "
				"                         B.SLAB_WIDTH "
				"           FROM TPSSM01 A, TPSSM03 B "
				"          WHERE A.PONO = B.PONO "
				"			 AND A.FACTORY_DIV = @v_factory_div "
				"			 AND A.PLAN_DATE = @v_date_time) "
				"  GROUP BY CC_MACH_NO, ST_NO, SLAB_THICK, SLAB_WIDTH ";

			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())
		{
			v_cc_mach_no = cmd_inq.GetString(1);
			v_plan_desc = cmd_inq.GetString(2);

			if (v_cc_mach_no_pre != v_cc_mach_no)
			{
				v_plan_desc_pre = "";
			}

			if (v_cc_mach_no == "1")
			{
				if (v_plan_desc_pre.Trim() == "")
				{
					v_mat_spec_c1 = v_plan_desc;
					
				}
				else
				{
					v_mat_spec_c1 = v_plan_desc_pre.Trim() + "/" + v_plan_desc;
				}
			}

			if (v_cc_mach_no == "2")
			{
				if (v_plan_desc_pre.Trim() == "")
				{
					v_mat_spec_c2 = v_plan_desc;

				}
				else
				{
					v_mat_spec_c2 = v_plan_desc_pre.Trim() + "/" + v_plan_desc;
				}
			}

			if (v_cc_mach_no == "3")
			{
				if (v_plan_desc_pre.Trim() == "")
				{
					v_mat_spec_c3 = v_plan_desc;

				}
				else
				{
					v_mat_spec_c3 = v_plan_desc_pre.Trim() + "/" + v_plan_desc;
				}
			}

			if (v_cc_mach_no == "4")
			{
				if (v_plan_desc_pre.Trim() == "")
				{
					v_mat_spec_c4 = v_plan_desc;

				}
				else
				{
					v_mat_spec_c4 = v_plan_desc_pre.Trim() + "/" + v_plan_desc;
				}
			}

			v_cc_mach_no_pre = v_cc_mach_no;
			v_plan_desc_pre = v_plan_desc;
		}
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr =
				" SELECT A.FACTORY_DIV,COUNT(1),SUM(A.MOLTIRON_WT1) "
				"  FROM TMMSM18 A "
				" WHERE A.FACTORY_DIV = @v_factory_div "
				"  AND  A.PRODUCTION_TIME = @v_date_time "
				" GROUP BY A.FACTORY_DIV ";

			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())
		{
			v_tpc_num_a1 = cmd_inq.GetDecimal(2);
			v_tpc_wt_a1 = cmd_inq.GetDecimal(3);
		}
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr =
				" SELECT A.STATION_NO, SUM(A.SLAB_WT) "
				"  FROM TMMSM33 A "
				" WHERE A.FACTORY_DIV = @v_factory_div "
				"  AND  A.START_TIME >= @v_date_time_1||'210000' "
				"  AND  A.START_TIME < @v_date_time||'210000' "
				" GROUP BY A.STATION_NO ";

			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.Parameters.Set("v_date_time_1", v_date_time_1);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())
		{
			v_cc_mach_no = cmd_inq.GetString(1);
			v_slab_wt = cmd_inq.GetDecimal(2);

			if (v_cc_mach_no == "1")
			{
				v_prod_wt_c1 = v_slab_wt;
			}

			if (v_cc_mach_no == "2")
			{
				v_prod_wt_c2 = v_slab_wt;
			}

			if (v_cc_mach_no == "3")
			{
				v_prod_wt_c3 = v_slab_wt;
			}

			if (v_cc_mach_no == "4")
			{
				v_prod_wt_c4 = v_slab_wt;
			}
		}
		cmd_inq.Close();
		

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			sqlstr =
				" SELECT A.STATION_NO, COUNT(1), SUM(A.OUT_STEEL_WT) "
				"  FROM TMMSM21 A "
				" WHERE A.FACTORY_DIV = @v_factory_div "
				"  AND  A.START_TIME >= @v_date_time_1||'210000' "
				"  AND  A.START_TIME < @v_date_time||'210000' "
				" GROUP BY A.STATION_NO ";

			break;
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "====== sqlstr[{0}] =======  ", sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.Parameters.Set("v_date_time_1", v_date_time_1);
		cmd_inq.Parameters.Set("v_date_time", v_date_time);
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())
		{
			v_station_no = cmd_inq.GetString(1);
			v_out_steel_sum = cmd_inq.GetDecimal(2);
			v_out_steel_wt = cmd_inq.GetDecimal(3);

			if (v_station_no == "1")
			{
				v_heat_num_b1 = v_out_steel_sum;
				v_prod_wt_b1 = v_out_steel_wt;
			}

			if (v_station_no == "2")
			{
				v_heat_num_b2 = v_out_steel_sum;
				v_prod_wt_b2 = v_out_steel_wt;
			}

			if (v_station_no == "3")
			{
				v_heat_num_b3 = v_out_steel_sum;
				v_prod_wt_b3 = v_out_steel_wt;
			}
		}
		cmd_inq.Close();

		//先取默认值，后续sql取数据源
		CDataRow & row = bcls_ret->Tables[0].Rows.Add();
		row["PLAN_WT_C1"] = v_plan_wt_c1;
		row["PROD_WT_C1"] = v_prod_wt_c1;
		row["MAT_SPEC_C1"] = v_mat_spec_c1;

		row["PLAN_WT_C2"] = v_plan_wt_c2;
		row["PROD_WT_C2"] = v_prod_wt_c2;
		row["MAT_SPEC_C2"] = v_mat_spec_c2;

		row["PLAN_WT_C3"] = v_plan_wt_c3;
		row["PROD_WT_C3"] = v_prod_wt_c3;
		row["MAT_SPEC_C3"] = v_mat_spec_c3;

		row["PLAN_WT_C4"] = v_plan_wt_c4;
		row["PROD_WT_C4"] = v_prod_wt_c4;
		row["MAT_SPEC_C4"] = v_mat_spec_c4;

		row["TPC_NUM_A1"] = v_tpc_num_a1;
		row["TPC_WT_A1"] = v_tpc_wt_a1;

		row["HEAT_NUM_B1"] = v_heat_num_b1;
		row["PROD_WT_B1"] = v_prod_wt_b1;
		row["HEAT_NUM_B2"] = v_heat_num_b2;
		row["PROD_WT_B2"] = v_prod_wt_b2;
		row["HEAT_NUM_B3"] = v_heat_num_b3;
		row["PROD_WT_B3"] = v_prod_wt_b3;

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
