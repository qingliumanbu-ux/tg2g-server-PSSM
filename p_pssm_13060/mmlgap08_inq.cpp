/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一二系列精炼生产日报
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
BM2F_ENTERACE(mmlgap08_inq)

int f_mmlgap08_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString datetime1("");
	CString datetime2("");
	CString v_date("");
	/* 数据库SQL操作字符串 */
	CString sqlstr("");

	/* 实体类定义 */


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		datetime1 = bcls_rec->Tables[0].Rows[0]["DATE_TIME_F"].ToString().Trim() + "210000";
		datetime2 = bcls_rec->Tables[0].Rows[0]["DATE_TIME_T"].ToString().Trim() + "210000";
		v_date = bcls_rec->Tables[0].Rows[0]["DATE_TIME_T"].ToString().Substring(0, 8);

		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始datetime1[{0}],datetime2[{1}] =======  ", datetime1, datetime2);


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = "SELECT tjlls.cb,tjlls.drlf,tjlls.ljlf,tjlls.drrh,tjlls.ljrh,tjlls.drlr,tjlls.ljlr,tjlls.drrl,tjlls.ljrl,tjlls.drzls, "
					"tjlls.ljzls, tjlls.drlfbl, tjlls.drrhbl, tjlls.drlrbl, tjlls.drrlbl, tjlls.ljlfbl, tjlls.ljrhbl, tjlls.ljlrbl, tjlls.ljrlbl, "
					"tjlzl.drlf drlfzl, tjlzl.ljlf ljlfzl, tjlzl.drrh drrhzl, tjlzl.ljrh ljrhzl, tjlzl.drlr drlrzl, tjlzl.ljlr ljlrzl, tjlzl.drrl drrlzl, "
					"tjlzl.ljrl ljrlzl from( "
					"select '二系列' as cb, sum(drlf) as drlf, sum(ljlf) as ljlf, sum(drvd) as drvd, sum(ljvd) as ljvd, sum(drlv) as drlv, sum(ljLV) as ljLV, sum(drvl) as drvl, sum(ljVL) as ljLV, sum(drzls) as drzls, sum(ljzls) as ljzls, "
					"decode(sum(drzls), 0, 0, cast(round(cast(sum(drlf) * 100 as decimal(31, 3)) / sum(drzls), 2) as decimal(31, 2))) as drlfbl, "
					"decode(sum(drzls), 0, 0, cast(round(cast(sum(drvd) * 100 as decimal(31, 3)) / sum(drzls), 2) as decimal(31, 2))) as drvdbl, "
					"decode(sum(drzls), 0, 0, cast(round(cast(sum(drlv) * 100 as decimal(31, 3)) / sum(drzls), 2) as decimal(31, 2))) as drlvbl, "
					"decode(sum(drzls), 0, 0, cast(round(cast(sum(drvl) * 100 as decimal(31, 3)) / sum(drzls), 2) as decimal(31, 2))) as drvlbl, "
					"decode(sum(ljzls), 0, 0, cast(round(cast(sum(ljlf) * 100 as decimal(31, 3)) / sum(ljzls), 2) as decimal(31, 2))) as ljlfbl, "
					"decode(sum(ljzls), 0, 0, cast(round(cast(sum(ljvd) * 100 as decimal(31, 3)) / sum(ljzls), 2) as decimal(31, 2))) as ljvdbl, "
					"decode(sum(ljzls), 0, 0, cast(round(cast(sum(ljLV) * 100 as decimal(31, 3)) / sum(ljzls), 2) as decimal(31, 2))) as ljLVbl, "
					"decode(sum(ljzls), 0, 0, cast(round(cast(sum(ljVL) * 100 as decimal(31, 3)) / sum(ljzls), 2) as decimal(31, 2))) as ljVLbl from("
					"select count(a.heat_no) as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BLC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select count(a.heat_no) as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BLC')and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, count(a.heat_no) drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BVC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, count(a.heat_no) drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BVC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, count(a.heat_no) as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BLVC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, count(a.heat_no) as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BLVC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, count(a.heat_no) as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BVLC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, count(a.heat_no) as drvl, 0 as ljVL, 0 as drzls, 0 ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BVLC')and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, count(a.heat_no) as drzls, 0 ljzls  from tpssm41b a, tmmsm21 c where c.BLOW_START_TIME1 <>' ' and a.heat_no = c.heat_no and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, count(a.heat_no) as drzls, 0 ljzls  from tpssm11b a, tmmsm21 c where c.BLOW_START_TIME1 <>' ' and a.heat_no = c.heat_no and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, count(a.heat_no) as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 as ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BLC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, count(a.heat_no) as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 as ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BLC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, count(a.heat_no) as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 as ljzls from tpssm41b a, tmmsm21 c where a.backlog_ea in('BRC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, count(a.heat_no) as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 as ljzls from tpssm11b a, tmmsm21 c where a.backlog_ea in('BRC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, count(a.heat_no) as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 as ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BLRC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, count(a.heat_no) as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, 0 as ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BLRC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, count(a.heat_no) as ljVL, 0 as drzls, 0 as ljzls  from tpssm41b a, tmmsm21 c where a.backlog_ea in('BRLC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, count(a.heat_no) as ljVL, 0 as drzls, 0 as ljzls  from tpssm11b a, tmmsm21 c where a.backlog_ea in('BRLC') and a.heat_no = c.heat_no and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, count(a.heat_no) as ljzls  from tpssm41b a, tmmsm21 c where c.BLOW_START_TIME1 <>' ' and a.heat_no = c.heat_no and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljLF, 0 drvd, 0 as ljvd, 0 as drlv, 0 as ljLV, 0 as drvl, 0 as ljVL, 0 as drzls, count(a.heat_no) as ljzls  from tpssm11b a, tmmsm21 c where c.BLOW_START_TIME1 <>' ' and a.heat_no = c.heat_no and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1  < @datetime2)) tjlls,"
					"(select '二系列' as cb, sum(drlf) as drlf, sum(ljlf) as ljlf, sum(drvd) as drvd, sum(ljvd) as ljvd, sum(drlv) as drlv, sum(ljlv) as ljlv, sum(drvl) as drvl, sum(ljvl) as ljvl from"
					"(select cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl "
					"from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLC') and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLC') and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLC') and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLC') and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljlf, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVC') and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljlf, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVC') and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVC')and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVC') and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLVC')and c.BLOW_START_TIME1 <>' '  and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drlv, 0 as ljlv, 0 as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLVC') and c.BLOW_START_TIME1<>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljlv, 0 as drvl, 0 as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLVC') and c.BLOW_START_TIME1<>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljlv, 0 as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BLVC') and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as drvl, 0 as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVLC') and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, cast(nvl(SUM(b.sla b_wt), 0) as decimal(20, 3)) as drvl, 0 as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVLC')and c.BLOW_START_TIME1 <>' ' and TO_CHAR((to_date(c.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljvl from tpssm41b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVLC') and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2 "
					"union all select 0 as drlf, 0 as ljlf, 0 as drvd, 0 as ljvd, 0 as drlv, 0 as ljlv, 0 as drvl, cast(nvl(SUM(b.slab_wt), 0) as decimal(20, 3)) as ljvl from tpssm11b a, tmmsm33 b, tmmsm21 c where a.HEAT_NO = b.HEAT_NO and a.heat_no = c.heat_no and a.backlog_ea in('BVLC') and c.BLOW_START_TIME1 <>' ' and c.BLOW_START_TIME1 >= @datetime1 and c.BLOW_START_TIME1 < @datetime2) ))  tjlzl "
					"where  tjlls.cb = tjlzl.cb ";
			break;
		}
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr);

		cmd_inq.Parameters.Clear();
		cmd_inq.Parameters.Set("datetime1", datetime1);
		cmd_inq.Parameters.Set("datetime2", datetime2);
		cmd_inq.Parameters.Set("v_date", v_date);
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