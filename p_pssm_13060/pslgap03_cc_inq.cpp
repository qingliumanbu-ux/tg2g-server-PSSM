/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/9/6 10:08:19
Description: 炼钢二系列冶炼指标信息查询
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
BM2F_ENTERACE(pslgap03_cc_inq)

int f_pslgap03_cc_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_start_time("");
	CString v_end_time("");

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sql("");
	/* 实体类定义 */


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		v_start_time = bcls_rec->Tables[0].Rows[0]["START_TIME"].ToString().Trim();
		v_end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().Trim();


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_start_time[{0}],v_end_time[{1}] =======  ", v_start_time, v_end_time);


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = "SELECT PROD_WT_CC,HEAT_NUM_CC,STEEL_WT_CC,PROD_WT_C1,PROD_WT_C2,PROD_WT_C3 FROM(select cast(sum(b.slab_wt) "
				"as decimal(20, 3)) AS PROD_WT_CC, count(distinct(a.heat_no)) AS HEAT_NUM_CC  from tmmsm21 a, tmmsm33 b where "
				"a.heat_no = b.heat_no and a.BLOW_START_TIME1 <> ' ' and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') "
				"+ 3 HOUR), 'YYYYMMDD') >= @START_TIME and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), "
				"'YYYYMMDD') <  @END_TIME and a.FACTORY_DIV = 'A2'), (select cast(sum(b.CAST_STEEL_WT) as decimal(20, 3)) AS "
				"STEEL_WT_CC from tmmsm21 a, tmmsm31 b where a.heat_no = b.heat_no and a.BLOW_START_TIME1 <> ' ' and TO_CHAR(( "
				"to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') >= @START_TIME and TO_CHAR((to_date( "
				"a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') < @END_TIME and a.FACTORY_DIV = 'A2'), (select "
				"cast(sum(b.slab_wt) as decimal(20, 3)) AS PROD_WT_C1 from tmmsm21 a, tmmsm33 b where a.heat_no = b.heat_no and "
				"a.BLOW_START_TIME1 <> ' ' and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') "
				">= @START_TIME and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') <  @END_TIME "
				"and a.FACTORY_DIV = 'A2' and substr(b.cast_no, 2, 1) = '5'), (select cast(sum(b.slab_wt) as decimal(20, 3)) AS "
				"PROD_WT_C2 from tmmsm21 a, tmmsm33 b where a.heat_no = b.heat_no and a.BLOW_START_TIME1 <> ' ' and TO_CHAR(( "
				"to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') >= @START_TIME and TO_CHAR((to_date( "
				"a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') <  @END_TIME and a.FACTORY_DIV = 'A2' and "
				"substr(b.cast_no, 2, 1) = '6'), (select cast(sum(b.slab_wt) as decimal(20, 3)) AS PROD_WT_C3 from tmmsm21 a, tmmsm33 b "
				"where a.heat_no = b.heat_no and a.BLOW_START_TIME1 <> ' ' and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') "
				"+ 3 HOUR), 'YYYYMMDD') >= @START_TIME and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') "
				"< @END_TIME and a.FACTORY_DIV = 'A2' and substr(b.cast_no, 2, 1) = '7')";

			break;
		}
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr);
		cmd_inq.Parameters.Clear();
		cmd_inq.Parameters.Set("START_TIME", v_start_time);
		cmd_inq.Parameters.Set("END_TIME", v_end_time);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

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