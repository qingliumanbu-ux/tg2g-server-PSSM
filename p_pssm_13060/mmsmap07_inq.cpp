/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:
Version:     1.0
Date:        2021/8/31 13:40:05
Description: 一系列转炉生产日报
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
BM2F_ENTERACE(mmsmap07_inq)

int f_mmsmap07_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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


			sqlstr = "  select *  from (  select aa.lzh xh,aa.lzh||'#转炉' lzh, aa.drls, aa.drts, aa.drfg, aa.drtbfg, aa.ljls, aa.ljts, aa.ljfg, aa.ljtbfg, "
				"aa.drcgl, aa.ljcgl, aa.drtwl, aa.ljtwl, aa.drzg, aa.ljzg, aa.drzq, aa.ljzq, aa.drzyl, aa.ljzyl, aa.drtsl, aa.drgtl, aa.ljtsl, aa.ljgtl, "
				"bb.drgpzl, bb.ljgpzl, decode(bb.drgpzl, 0, 0, cast(cast(aa.drtsl as decimal(25, 5)) * 1000.000 / cast(bb.drgpzl as decimal(25, 5)) as "
				"decimal(25, 2))) drtgbl, decode(bb.drgpzl, 0, 0, cast(cast(aa.drgtl as decimal(25, 5)) * 1000.000 / cast(bb.drgpzl as decimal(25, 5)) "
				"as decimal(25, 2))) drgtlbl, decode(bb.ljgpzl, 0, 0, cast(cast(aa.ljtsl as decimal(25, 5)) * 1000.000 / cast(bb.ljgpzl as decimal(25, 5)) "
				"as decimal(25, 2))) ljtgbl, decode(bb.ljgpzl, 0, 0, cast(cast(aa.ljgtl as decimal(25, 5)) * 1000.000 / cast(bb.ljgpzl as decimal(25, 5)) "
				"as decimal(25, 2))) ljgtlbl from(SELECT LZH, SUM(ls) drls, sum(ts) drts, sum(fg) drfg, sum(tbfg) drtbfg, sum(LJLS) ljls, sum(LJTS) ljts, "
				"sum(LJFG) ljfg, sum(LJTBFG)  ljtbfg, sum(drcgl) drcgl, sum(ljcgl) ljcgl, sum(drtwl) drtwl, sum(ljtwl) ljtwl, sum(drzg) drzg, sum(ljzg) ljzg, "
				"cast(decode(sum(ls), 0, 0, cast(sum(drzzq) as decimal(20, 3)) / sum(ls) / 60)  as decimal(20, 2)) drzq, cast(decode(sum(ljls), 0, 0, "
				"cast(sum(ljzzq) as decimal(20, 3)) / sum(ljls) / 60) as decimal(20, 2)) ljzq, sum(drzzq) * 100 / 24 / 60 / 60 drzyl, "
				"sum(ljzzq) * 100 / 24 / 60 / 60 / timestampdiff(16, to_date(@datetime2, 'yyyymmddhh24miss') - to_date(@datetime1, 'yyyymmddhh24miss')) ljzyl, "
				"sum(drtsl) drtsl, sum(drgtl) drgtl, sum(ljtsl) ljtsl, sum(ljgtl) ljgtl	from(SELECT SUBSTR(a.HEAT_NO, 3, 1)  LZH, COUNT(a.HEAT_NO) LS, "
				"SUM(a.MOLTIRON_WT) TS, SUM(a.SCRAP_WEIGHT) FG, SUM(a.SCRAP_WT) TBFG, sum(a.OUT_STEEL_WT) drcgl, 0 LJLS, 0 LJTS, 0 LJFG, 0 LJTBFG, 0 ljcgl, "
				"sum(a.SMELT_CYCLE) drzzq, 0 ljzzq, sum(b.drtwl) drtwl, 0 ljtwl, sum(a.MOLTIRON_WT) drtsl, nvl(sum(MOLTIRON_WT), 0) + nvl(sum(SCRAP_WEIGHT), 0) + nvl(sum(SCRAP_WT), 0) "
				"+ nvl(sum(b.drtwl), 0) - nvl(sum(drzg), 0)  drgtl, 0 ljtsl, 0 ljgtl, sum(drzg) drzg, 0 ljzg	 FROM TMMSM21  a  left join(select heat_no, "
				"decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) drtwl from tmmsm2a where mat_code = 'FWLJ' group by heat_no) b   on  a.heat_no = b.HEAT_NO "
				"left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) drzg  from tmmsm2a where station_id = 'B'  and mat_code in "
				"('FJGLT', 'FJGZG', 'FN009') group by heat_no) c   on  a.heat_no = c.HEAT_NO  WHERE a.BLOW_START_TIME1 <> ' '	 and "
				"TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date 		 GROUP BY SUBSTR(a.HEAT_NO, 3, 1) "
				"UNION all    SELECT SUBSTR(a.HEAT_NO, 3, 1)  LZH, 0 LS, 0 TS, 0 FG, 0 TBFG, 0 drcgl, COUNT(a.HEAT_NO) LJLS, SUM(a.MOLTIRON_WT) LJTS, "
				"SUM(a.SCRAP_WEIGHT) LJFG, SUM(a.SCRAP_WT) LJTBFG, sum(a.OUT_STEEL_WT) ljcgl, 0 drzzq, sum(a.SMELT_CYCLE) ljzzq, 0 drtwl, sum(ljtwl) ljtwl, "
				"0 drtsl, 0 drgtl, sum(a.MOLTIRON_WT)  ljtsl, nvl(sum(MOLTIRON_WT), 0) + nvl(sum(SCRAP_WEIGHT), 0) + nvl(sum(SCRAP_WT), 0) + nvl(sum(b.ljtwl), 0) - nvl(sum(ljzg), 0) ljgtl, "
				"0 drzg, sum(ljzg) ljzg FROM TMMSM21 a left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) ljtwl from tmmsm2a where mat_code = 'FWLJ' "
				"group by heat_no) b on a.heat_no = b.HEAT_NO left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) ljzg  from tmmsm2a where station_id = 'B' "
				"and mat_code in('FJGLT', 'FJGZG', 'FN009') group by heat_no) c on  a.heat_no = c.HEAT_NO	WHERE a.BLOW_START_TIME1 <> ' '  and  a.BLOW_START_TIME1 "
				">= @datetime1   and  a.BLOW_START_TIME1 < @datetime2   GROUP BY SUBSTR(a.HEAT_NO, 3, 1)) GROUP BY LZH  ORDER BY LZH) aa, (select lzh, "
				"sum(drgpzl) drgpzl, sum(ljgpzl) ljgpzl from(select substr(heat_no, 3, 1) lzh, sum(slab_wt) drgpzl, 0 ljgpzl from tmmsm33 a where heat_no in "
				"(select heat_no from tmmsm21   where BLOW_START_TIME1 <> ' '  and TO_CHAR((to_date(BLOW_START_TIME1, 'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date) "
				"group by substr(heat_no, 3, 1)  	union all		select substr(heat_no, 3, 1) lzh, 0 drgpzl, sum(slab_wt) ljgpzl from tmmsm33 a  where heat_no in(select heat_no "
				"from tmmsm21 where BLOW_START_TIME1 <> ' ' and BLOW_START_TIME1 >= @datetime1	and  BLOW_START_TIME1 < @datetime2)group by substr(heat_no, 3, 1))group by lzh) bb "
				"where aa.lzh = bb.lzh	 union all 	select aa.lzh xh, decode(aa.lzh, '4', '一系列合计') lzh, aa.drls, aa.drts, aa.drfg, aa.drtbfg, aa.ljls, aa.ljts, aa.ljfg, "
				"aa.ljtbfg, aa.drcgl, aa.ljcgl, aa.drtwl, aa.ljtwl, aa.drzg, aa.ljzg, aa.drzq, aa.ljzq, aa.drzyl, aa.ljzyl, aa.drtsl, aa.drgtl, aa.ljtsl, aa.ljgtl, "
				"bb.drgpzl, bb.ljgpzl, decode(bb.drgpzl, 0, 0, cast(cast(aa.drtsl as decimal(25, 5)) * 1000.000 / cast(bb.drgpzl as decimal(25, 5)) as decimal(25, 2))) drtgbl, "
				"decode(bb.drgpzl, 0, 0, cast(cast(aa.drgtl as decimal(25, 5)) * 1000.000 / cast(bb.drgpzl as decimal(25, 5)) as decimal(25, 2))) drgtlbl, "
				"decode(bb.ljgpzl, 0, 0, cast(cast(aa.ljtsl as decimal(25, 5)) * 1000.000 / cast(bb.ljgpzl as decimal(25, 5)) as decimal(25, 2))) ljtgbl, decode(bb.ljgpzl, 0, 0, cast(cast(aa.ljgtl as "
				"decimal(25, 5)) * 1000.000 / cast(bb.ljgpzl as decimal(25, 5)) as decimal(25, 2))) ljgtlbl  from(SELECT lzh, SUM(ls) drls, sum(ts) drts, sum(fg) drfg, sum(tbfg) drtbfg, "
				"sum(LJLS) ljls, sum(LJTS) ljts, sum(LJFG) ljfg, sum(LJTBFG)  ljtbfg, sum(drcgl) drcgl, sum(ljcgl) ljcgl, sum(drtwl) drtwl, sum(ljtwl) ljtwl, sum(drzg) drzg, sum(ljzg) ljzg, "
				"cast(decode(sum(ls), 0, 0, cast(sum(drzzq) as decimal(20, 3)) / sum(ls) / 60)  as decimal(20, 2)) drzq, cast(decode(sum(ljls), 0, 0, cast(sum(ljzzq) as decimal(20, 3)) / sum(ljls) / 60) "
				"as decimal(20, 2)) ljzq, sum(drzzq) * 100 / 24 / 60 / 60 / 3 drzyl, sum(ljzzq) * 100 / 24 / 60 / 60 / 3 / timestampdiff(16, to_date(@datetime2, 'yyyymmddhh24miss') - to_date(@datetime1, "
				"'yyyymmddhh24miss'))  ljzyl, sum(drtsl) drtsl, sum(drgtl) drgtl, sum(ljtsl) ljtsl, sum(ljgtl) ljgtl	from(SELECT decode(SUBSTR(a.HEAT_NO, 3, 1), '1', '4', '2', '4', '3', '4', '8')  LZH, "
				"COUNT(a.HEAT_NO) LS, SUM(a.MOLTIRON_WT) TS, SUM(a.SCRAP_WEIGHT) FG, SUM(a.SCRAP_WT) TBFG, sum(a.OUT_STEEL_WT) drcgl, 0 LJLS, 0 LJTS, 0 LJFG, 0 LJTBFG, 0 ljcgl, "
				"sum(a.SMELT_CYCLE) drzzq, 0 ljzzq, sum(b.drtwl) drtwl, 0 ljtwl, sum(a.MOLTIRON_WT) drtsl, sum(MOLTIRON_WT) + sum(SCRAP_WEIGHT) + sum(SCRAP_WT) + nvl(sum(b.drtwl), 0) - sum(drzg)  drgtl, "
				"0 ljtsl, 0 ljgtl, sum(drzg) drzg, 0 ljzg FROM TMMSM21  a  left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) drtwl from tmmsm2a where mat_code = 'FWLJ' group by "
				"heat_no) b   on  a.heat_no = b.HEAT_NO  left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) drzg  from tmmsm2a where station_id = 'B' 	and mat_code in "
				"('FJGLT', 'FJGZG', 'FN009') group by heat_no) c   on  a.heat_no = c.HEAT_NO   WHERE a.BLOW_START_TIME1 <> ' '	and TO_CHAR((to_date(a.BLOW_START_TIME1, 'YYYYMMDDHH24MISS') "
				"+ 3 HOUR), 'YYYYMMDD') = @v_date GROUP BY decode(SUBSTR(a.HEAT_NO, 3, 1), '1', '4', '2', '4', '3', '4', '8') UNION all   SELECT decode(SUBSTR(a.HEAT_NO, 3, 1), '1', '4', '2', '4', '3', '4', '8') "
				"LZH, 0 LS, 0 TS, 0 FG, 0 TBFG, 0 drcgl, COUNT(a.HEAT_NO) LJLS, SUM(a.MOLTIRON_WT) LJTS, SUM(a.SCRAP_WEIGHT) LJFG, SUM(a.SCRAP_WT) LJTBFG, sum(a.OUT_STEEL_WT) ljcgl, "
				"0 drzzq, sum(a.SMELT_CYCLE) ljzzq, 0 drtwl, sum(ljtwl) ljtwl, 0 drtsl, 0 drgtl, sum(a.MOLTIRON_WT)  ljtsl, sum(MOLTIRON_WT) + sum(SCRAP_WEIGHT) + sum(SCRAP_WT) + nvl(sum(b.ljtwl), 0) - sum(ljzg) ljgtl, "
				"0 drzg, sum(ljzg) ljzg	FROM TMMSM21  a  left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) ljtwl from tmmsm2a where mat_code = 'FWLJ' group by heat_no) b   on  a.heat_no = b.HEAT_NO "
				"left join(select heat_no, decimal(nvl(sum(devo_wt / 1000), 0), 20, 3) ljzg  from tmmsm2a where station_id = 'B' and mat_code in('FJGLT', 'FJGZG', 'FN009') group by heat_no) c   on  a.heat_no = c.HEAT_NO "
				"WHERE a.BLOW_START_TIME1 <> ' '  and  a.BLOW_START_TIME1 >= @datetime1   and  a.BLOW_START_TIME1 < @datetime2  GROUP BY decode(SUBSTR(a.HEAT_NO, 3, 1), '1', '4', '2', '4', '3', '4', '8') "
				")GROUP BY lzh  ORDER BY lzh) aa, (select decode(LZH, '1', '4', '2', '4', '3', '4', '8') lzh, sum(drgpzl) drgpzl, sum(ljgpzl) ljgpzl from(select substr(heat_no, 3, 1) lzh, "
				"sum(slab_wt) drgpzl, 0 ljgpzl from tmmsm33 a  where heat_no in(select heat_no from tmmsm21   where BLOW_START_TIME1 <> ' '  and TO_CHAR((to_date(BLOW_START_TIME1, "
				"'YYYYMMDDHH24MISS') + 3 HOUR), 'YYYYMMDD') = @v_date)	group by substr(heat_no, 3, 1) union all	select substr(heat_no, 3, 1) lzh, 0 drgpzl, sum(slab_wt) ljgpzl from "
				"tmmsm33 a where heat_no in(select heat_no from tmmsm21 	where BLOW_START_TIME1 <> ' ' and  BLOW_START_TIME1 >= @datetime1 and  BLOW_START_TIME1 < @datetime2) "
				"group by substr(heat_no, 3, 1)) group by decode(LZH, '1', '4', '2', '4', '3', '4', '8')) bb where aa.lzh = bb.lzh	)  order by xh  ";

		


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