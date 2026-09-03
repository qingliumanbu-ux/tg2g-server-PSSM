/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一炼钢板坯产量信息查询
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
BM2F_ENTERACE(mmsmap05_inq)

int f_mmsmap05_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_date_time("");
	CString time_month("");
	CString time_year("");
	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sqlstr1("");
	CString sqlstr2("");
	CString sqlstr3("");
	/* 实体类定义 */

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();
		time_month = v_date_time.Substring(0, 6);
		time_year = v_date_time.Substring(0,4);
		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "v_date_time[{0}]", v_date_time);
		Log::Trace("", __FUNCTION__, "time_month[{0}]", time_month);
		Log::Trace("", __FUNCTION__, "time_year[{0}]", time_year);
		bcls_ret->Tables.Add("A");
		bcls_ret->Tables.Add("B");
		bcls_ret->Tables.Add("C");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = "SELECT T1.STATION_NO,WT_YEAR,WT_MONTH,WT_WEEK,WT_DAY FROM(select STATION_NO,sum(OUT_STEEL_WT) AS WT_YEAR FROM (select case  "
				"BLOW_START_TIME1 WHEN to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), "
				"'210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(BLOW_START_TIME1, 1, 8) when to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') > "
				"to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(BLOW_START_TIME1, 1, 8), "
				"'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS BLOW_START_TIME1, OUT_STEEL_WT, case STATION_NO WHEN STATION_NO = 1 THEN '1#转炉钢水' when "
				"STATION_NO = 2 THEN '2#转炉钢水' when STATION_NO = 3 THEN '3#转炉钢水' end as STATION_NO from TMMSM21 where  FACTORY_DIV = 'A1' and BLOW_START_TIME1 <> '') "
				"where BLOW_START_TIME1 like @TIME_YEAR || '%' GROUP BY STATION_NO) T1 LEFT JOIN(select STATION_NO, sum(OUT_STEEL_WT) AS WT_MONTH FROM "
				"(select case BLOW_START_TIME1 WHEN to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(BLOW_START_TIME1, 1, 8)when to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING( "
				"BLOW_START_TIME1, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(BLOW_START_TIME1, 1, 8), 'YYYY-MM-DD') + 1 day, "
				"'YYYYMMDD')END AS BLOW_START_TIME1, OUT_STEEL_WT, case STATION_NO WHEN STATION_NO = 1 THEN '1#转炉钢水' when STATION_NO = 2 THEN '2#转炉钢水' "
				"when STATION_NO = 3 THEN '3#转炉钢水' end as STATION_NO from TMMSM21 where  FACTORY_DIV = 'A1' and BLOW_START_TIME1 <> '') where BLOW_START_TIME1 like @TIME_MONTH || '%' "
				"GROUP BY STATION_NO)T2 ON T1.STATION_NO = T2.STATION_NO LEFT JOIN(select STATION_NO, sum(OUT_STEEL_WT) AS WT_WEEK FROM(select case "
				"BLOW_START_TIME1 WHEN to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(BLOW_START_TIME1, 1, 8)when to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING( "
				"BLOW_START_TIME1, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')THEN to_char(to_date(SUBSTRING(BLOW_START_TIME1, 1, 8), 'YYYY-MM-DD') + 1 day, "
				"'YYYYMMDD')END AS BLOW_START_TIME1, OUT_STEEL_WT, case STATION_NO WHEN STATION_NO = 1 THEN '1#转炉钢水' when STATION_NO = 2 THEN '2#转炉钢水' "
				"when STATION_NO = 3 THEN '3#转炉钢水' end as STATION_NO from TMMSM21 where  FACTORY_DIV = 'A1' and BLOW_START_TIME1 <> '') where WEEK_ISO(to_date(@TIME_DAY, "
				"'YYYY-MM-DD')) = WEEK_ISO(to_date(BLOW_START_TIME1, 'YYYY-MM-DD')) GROUP BY STATION_NO)T3 ON T1.STATION_NO = T3.STATION_NO LEFT JOIN "
				"(select STATION_NO, sum(OUT_STEEL_WT) AS WT_DAY FROM(select case BLOW_START_TIME1 WHEN to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') "
				">= to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') "
				"< to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(BLOW_START_TIME1, 1, 8) "
				"when to_date(BLOW_START_TIME1, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(BLOW_START_TIME1, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') "
				"THEN to_char(to_date(SUBSTRING(BLOW_START_TIME1, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS BLOW_START_TIME1, OUT_STEEL_WT, case STATION_NO "
				"WHEN STATION_NO = 1 THEN '1#转炉钢水' when STATION_NO = 2 THEN '2#转炉钢水' when STATION_NO = 3 THEN '3#转炉钢水' end as STATION_NO "
				"from TMMSM21 where  FACTORY_DIV = 'A1' and BLOW_START_TIME1 <> '') where BLOW_START_TIME1 = @TIME_DAY GROUP BY STATION_NO) T4 ON T1.STATION_NO = T4.STATION_NO";

			sqlstr1 = "SELECT T1.STATION_NO,WT_YEAR,WT_MONTH,WT_WEEK,WT_DAY FROM(select STATION_NO,sum(FIN_LADLE_W_L) AS WT_YEAR FROM (select case "
				"START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), "
				"'210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8) when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > "
				"to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), "
				"'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO WHEN STATION_NO = 1 THEN '1#LF产量' when "
				"STATION_NO = 2 THEN '2#LF产量'  end as STATION_NO from TMMSM24 where  FACTORY_DIV = 'A1' and START_TIME not in('')) "
				"where START_TIME like @TIME_YEAR || '%' GROUP BY STATION_NO) T1 LEFT JOIN(select STATION_NO, sum(FIN_LADLE_W_L) AS WT_MONTH FROM "
				"(select case START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8)when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING( "
				"START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, "
				"'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO WHEN STATION_NO = 1 THEN '1#LF产量' when STATION_NO = 2 THEN '2#LF产量' "
				"end as STATION_NO from TMMSM24 where  FACTORY_DIV = 'A1' and START_TIME not in('')) where START_TIME like @TIME_MONTH || '%' "
				"GROUP BY STATION_NO)T2 ON T1.STATION_NO = T2.STATION_NO LEFT JOIN(select STATION_NO, sum(FIN_LADLE_W_L) AS WT_WEEK FROM(select case "
				"START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8)when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING( "
				"START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, "
				"'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO WHEN STATION_NO = 1 THEN '1#LF产量' when STATION_NO = 2 THEN '2#LF产量' "
				"end as STATION_NO from TMMSM24 where  FACTORY_DIV = 'A1' and START_TIME not in('')) where WEEK_ISO(to_date(@TIME_DAY, "
				"'YYYY-MM-DD')) = WEEK_ISO(to_date(START_TIME, 'YYYY-MM-DD')) GROUP BY STATION_NO)T3 ON T1.STATION_NO = T3.STATION_NO LEFT JOIN "
				"(select STATION_NO, sum(FIN_LADLE_W_L) AS WT_DAY FROM(select case START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') "
				">= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') "
				"< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8) "
				"when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') "
				"THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO "
				"WHEN  STATION_NO = 1 THEN '1#LF产量' when STATION_NO = 2 THEN '2#LF产量' end as STATION_NO from TMMSM24 where  FACTORY_DIV = 'A1' and "
				"START_TIME not in('')) where START_TIME = @TIME_DAY GROUP BY STATION_NO) T4 ON T1.STATION_NO = T4.STATION_NO";

			sqlstr2 = "SELECT T1.STATION_NO,WT_YEAR,WT_MONTH,WT_WEEK,WT_DAY FROM(select STATION_NO,sum(FIN_LADLE_W_L) AS WT_YEAR FROM (select case "
				"START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), "
				"'210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8) when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > "
				"to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), "
				"'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO WHEN STATION_NO = 1 THEN '单RH产量' "
				"end as STATION_NO from TMMSM23 where  FACTORY_DIV = 'A1' and START_TIME not in('')) "
				"where START_TIME like @TIME_YEAR || '%' GROUP BY STATION_NO) T1 LEFT JOIN(select STATION_NO, sum(FIN_LADLE_W_L) AS WT_MONTH FROM "
				"(select case START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8)when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING( "
				"START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, "
				"'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO WHEN STATION_NO = 1 THEN '单RH产量' "
				"end as STATION_NO from TMMSM23 where  FACTORY_DIV = 'A1' and START_TIME not in('')) where START_TIME like @TIME_MONTH || '%' "
				"GROUP BY STATION_NO)T2 ON T1.STATION_NO = T2.STATION_NO LEFT JOIN(select STATION_NO, sum(FIN_LADLE_W_L) AS WT_WEEK FROM(select case " 
				"START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8)when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING( "
				"START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, "
				"'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO WHEN STATION_NO = 1 THEN '单RH产量' "
				"end as STATION_NO from TMMSM23 where  FACTORY_DIV = 'A1' and START_TIME not in('')) where WEEK_ISO(to_date(@TIME_DAY, "
				"'YYYY-MM-DD')) = WEEK_ISO(to_date(START_TIME, 'YYYY-MM-DD')) GROUP BY STATION_NO)T3 ON T1.STATION_NO = T3.STATION_NO LEFT JOIN "
				"(select STATION_NO, sum(FIN_LADLE_W_L) AS WT_DAY FROM(select case START_TIME WHEN to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') "
				">= to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') "
				"< to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(START_TIME, 1, 8) "
				"when to_date(START_TIME, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(START_TIME, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') "
				"THEN to_char(to_date(SUBSTRING(START_TIME, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS START_TIME, FIN_LADLE_W_L, case STATION_NO "
				"WHEN  STATION_NO = 1 THEN '单RH产量' end as STATION_NO from TMMSM23 where  FACTORY_DIV = 'A1' and "
				"START_TIME not in('')) where START_TIME = @TIME_DAY GROUP BY STATION_NO) T4 ON T1.STATION_NO = T4.STATION_NO";

			sqlstr3 = "SELECT T1.STATION_NO,WT_YEAR,WT_MONTH,WT_WEEK,WT_DAY FROM(select STATION_NO,sum(SLAB_WT) AS WT_YEAR FROM (select   "
				"case slab_cut_time WHEN to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), "
				"'210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(slab_cut_time, 1, 8) when to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS') > "
				"to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(slab_cut_time, 1, 8), "
				"'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS slab_cut_time, SLAB_WT, case STATION_NO WHEN STATION_NO = 1 THEN '1#连铸机板坯' WHEN STATION_NO = 2 "
				"THEN '2#连铸机板坯' WHEN STATION_NO = 3 THEN '3#连铸机板坯' WHEN STATION_NO = 4 THEN '4#连铸机板坯' end as STATION_NO from "
				"(select a.SLAB_WT, a.STATION_NO, a.slab_cut_time from TMMSM33 a where  a.FACTORY_DIV = 'A1')) "
				"where slab_cut_time like @TIME_YEAR || '%' GROUP BY STATION_NO) T1 LEFT JOIN(select STATION_NO, sum(SLAB_WT) AS WT_MONTH FROM "
				"(select case slab_cut_time WHEN to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), "
				"'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), "
				"'210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(slab_cut_time, 1, 8) when to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS') > "
				"to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date(SUBSTRING(slab_cut_time, 1, 8), "
				"'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS slab_cut_time, SLAB_WT, case STATION_NO WHEN STATION_NO = 1 THEN '1#连铸机板坯' WHEN STATION_NO = 2 "
				"THEN '2#连铸机板坯' WHEN STATION_NO = 3 THEN '3#连铸机板坯' WHEN STATION_NO = 4 THEN '4#连铸机板坯' end as STATION_NO from "
				"(select a.SLAB_WT, a.STATION_NO, a.slab_cut_time from TMMSM33 a where  a.FACTORY_DIV = 'A1')) "
				"where slab_cut_time like @TIME_MONTH || '%' GROUP BY STATION_NO)T2 ON T1.STATION_NO = T2.STATION_NO LEFT JOIN(select STATION_NO, "
				"sum(SLAB_WT) AS WT_WEEK FROM(select case slab_cut_time WHEN to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING "
				"(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT( "
				"SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(slab_cut_time, 1, 8) when to_date(slab_cut_time, "
				"'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN to_char(to_date( "
				"SUBSTRING(slab_cut_time, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS slab_cut_time, SLAB_WT, case STATION_NO WHEN STATION_NO = 1 THEN "
				"'1#连铸机板坯' WHEN STATION_NO = 2 THEN '2#连铸机板坯' WHEN STATION_NO = 3 THEN '3#连铸机板坯' WHEN STATION_NO = 4 THEN '4#连铸机板坯' "
				"end as STATION_NO from(select a.SLAB_WT, a.STATION_NO, a.slab_cut_time from TMMSM33 a where  a.FACTORY_DIV "
				"= 'A1')) where WEEK_ISO(to_date(@TIME_DAY, 'YYYY-MM-DD')) = WEEK_ISO(to_date(slab_cut_time, 'YYYY-MM-DD')) GROUP BY STATION_NO)T3 ON "
				"T1.STATION_NO = T3.STATION_NO LEFT JOIN(select STATION_NO, sum(SLAB_WT) AS WT_DAY FROM(select case slab_cut_time WHEN to_date(slab_cut_time, "
				"'YYYY-MM-DD HH24:MI:SS') >= to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') - 1 day AND to_date(slab_cut_time, "
				"'YYYY-MM-DD HH24:MI:SS')< to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS')  THEN SUBSTRING(slab_cut_time, 1, 8) "
				"when to_date(slab_cut_time, 'YYYY-MM-DD HH24:MI:SS') > to_date(CONCAT(SUBSTRING(slab_cut_time, 1, 8), '210000'), 'YYYY-MM-DD HH24:MI:SS') THEN "
				"to_char(to_date(SUBSTRING(slab_cut_time, 1, 8), 'YYYY-MM-DD') + 1 day, 'YYYYMMDD')END AS slab_cut_time, SLAB_WT, case STATION_NO WHEN STATION_NO = 1 "
				"THEN '1#连铸机板坯' WHEN STATION_NO = 2 THEN '2#连铸机板坯' WHEN STATION_NO = 3 THEN '3#连铸机板坯' WHEN STATION_NO = 4 THEN '4#连铸机板坯' "
				"end as STATION_NO from(select a.SLAB_WT, a.STATION_NO, a.slab_cut_time from TMMSM33 a where  a.FACTORY_DIV = 'A1')) "
				"where slab_cut_time = @TIME_DAY GROUP BY STATION_NO) T4 ON T1.STATION_NO = T4.STATION_NO";


			break;
		}
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr);
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr1);
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr2);
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr3);
		cmd_inq.Parameters.Clear();
		cmd_inq.Parameters.Set("TIME_DAY", v_date_time);
		cmd_inq.Parameters.Set("TIME_MONTH", time_month);
		cmd_inq.Parameters.Set("TIME_YEAR", time_year);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
		cmd_inq.SetCommandText(sqlstr1);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["A"]);
		cmd_inq.Close();
		cmd_inq.SetCommandText(sqlstr2);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["B"]);
		cmd_inq.Close();
		cmd_inq.SetCommandText(sqlstr3);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["C"]);
		cmd_inq.Close();
		CDecimal WT_YEAR;
		CDecimal WT_MONTH;
		CDecimal WT_WEEK;
		CDecimal WT_DAY;
		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++){
			WT_YEAR = WT_YEAR + bcls_ret->Tables[0].Rows[i]["WT_YEAR"];
			WT_MONTH = WT_MONTH + bcls_ret->Tables[0].Rows[i]["WT_MONTH"];
			WT_WEEK = WT_WEEK + bcls_ret->Tables[0].Rows[i]["WT_WEEK"];
			WT_DAY = WT_DAY + bcls_ret->Tables[0].Rows[i]["WT_DAY"];
		}
		int j = bcls_ret->Tables[0].Rows.get_Count();
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[j]["STATION_NO"] = "钢水产量汇总";
		bcls_ret->Tables[0].Rows[j]["WT_YEAR"] = WT_YEAR;
		bcls_ret->Tables[0].Rows[j]["WT_MONTH"] = WT_MONTH;
		bcls_ret->Tables[0].Rows[j]["WT_WEEK"] = WT_WEEK;
		bcls_ret->Tables[0].Rows[j]["WT_DAY"] = WT_DAY;
		for (int i = 0; i < bcls_ret->Tables["A"].Rows.get_Count(); i++){
			int n = bcls_ret->Tables[0].Rows.get_Count();
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[n]["STATION_NO"] = bcls_ret->Tables["A"].Rows[i]["STATION_NO"];
			bcls_ret->Tables[0].Rows[n]["WT_YEAR"] = bcls_ret->Tables["A"].Rows[i]["WT_YEAR"];
			bcls_ret->Tables[0].Rows[n]["WT_MONTH"] = bcls_ret->Tables["A"].Rows[i]["WT_MONTH"];
			bcls_ret->Tables[0].Rows[n]["WT_WEEK"] = bcls_ret->Tables["A"].Rows[i]["WT_WEEK"];
			bcls_ret->Tables[0].Rows[n]["WT_DAY"] = bcls_ret->Tables["A"].Rows[i]["WT_DAY"];
		}
		for (int i = 0; i < bcls_ret->Tables["B"].Rows.get_Count(); i++){
			int n = bcls_ret->Tables[0].Rows.get_Count();
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[n]["STATION_NO"] = bcls_ret->Tables["B"].Rows[i]["STATION_NO"];
			bcls_ret->Tables[0].Rows[n]["WT_YEAR"] = bcls_ret->Tables["B"].Rows[i]["WT_YEAR"];
			bcls_ret->Tables[0].Rows[n]["WT_MONTH"] = bcls_ret->Tables["B"].Rows[i]["WT_MONTH"];
			bcls_ret->Tables[0].Rows[n]["WT_WEEK"] = bcls_ret->Tables["B"].Rows[i]["WT_WEEK"];
			bcls_ret->Tables[0].Rows[n]["WT_DAY"] = bcls_ret->Tables["B"].Rows[i]["WT_DAY"];
		}
		for (int i = 0; i < bcls_ret->Tables["C"].Rows.get_Count(); i++){
			int n = bcls_ret->Tables[0].Rows.get_Count();
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[n]["STATION_NO"] = bcls_ret->Tables["C"].Rows[i]["STATION_NO"];
			bcls_ret->Tables[0].Rows[n]["WT_YEAR"] = bcls_ret->Tables["C"].Rows[i]["WT_YEAR"];
			bcls_ret->Tables[0].Rows[n]["WT_MONTH"] = bcls_ret->Tables["C"].Rows[i]["WT_MONTH"];
			bcls_ret->Tables[0].Rows[n]["WT_WEEK"] = bcls_ret->Tables["C"].Rows[i]["WT_WEEK"];
			bcls_ret->Tables[0].Rows[n]["WT_DAY"] = bcls_ret->Tables["C"].Rows[i]["WT_DAY"];
		}
		WT_YEAR = 0;
		WT_MONTH = 0;
		WT_WEEK = 0;
		WT_DAY = 0;
		for (int i = 0; i < bcls_ret->Tables["C"].Rows.get_Count(); i++){
			WT_YEAR = WT_YEAR + bcls_ret->Tables["C"].Rows[i]["WT_YEAR"];
			WT_MONTH = WT_MONTH + bcls_ret->Tables["C"].Rows[i]["WT_MONTH"];
			WT_WEEK = WT_WEEK + bcls_ret->Tables["C"].Rows[i]["WT_WEEK"];
			WT_DAY = WT_DAY + bcls_ret->Tables["C"].Rows[i]["WT_DAY"];
		}
		int n = bcls_ret->Tables[0].Rows.get_Count();
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[n]["STATION_NO"] = "钢坯产量汇总";
		bcls_ret->Tables[0].Rows[n]["WT_YEAR"] = WT_YEAR;
		bcls_ret->Tables[0].Rows[n]["WT_MONTH"] = WT_MONTH;
		bcls_ret->Tables[0].Rows[n]["WT_WEEK"] = WT_WEEK;
		bcls_ret->Tables[0].Rows[n]["WT_DAY"] = WT_DAY;
		bcls_ret->Tables["A"].Delete();
		bcls_ret->Tables["B"].Delete();
		bcls_ret->Tables["C"].Delete();


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