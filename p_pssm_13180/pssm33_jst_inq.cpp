/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2014-3-5
Version:1.0
Description: 出钢计划甘特图查询
Update：
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "CUtils.h"
//程序用头文件
/*<remark>=========================================================
/// <summary>
/// 甘特图计划信息查询
/// <para>主要数据：主计划及工序计划，设备信息及状态，浇铸信息,传搁时间信息。</para>
/// <para>数据库表：tpssm11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：PSSM18画面查询(甘特图)调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>出钢计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm33_jst_inq)

int f_pssm33_jst_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int blkseq = 0;
	int ret = 0;
	int fetchRowCount = 0;

	CString v_factory_div = "";	//炼钢单元号
	CString base_time = "";
	CString cast_lot_no_and_div = "";
	CDecimal  pour_time1 = 0;               /* 连铸机浇注时间*/
	CDecimal  pour_time = 0;               /* 连铸机浇注时间*/
	CString dev_move_start = "";
	CString dev_move_end = "";
	CString dev_code = "";
	CString query_type = "";
	CString v_slab_dest = "";
	CDecimal prod_density = 0; //板坯密度
	int k = 0;
	int mode = 1;
	CString lslab_no = "";
	CString strand_no = "";
	CDecimal seq = 0;
	CString pono = "";
	CString show_flag = "";
	CString cs_dev_code = "";
	//CModel tep0002("TEP0002");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);
	CString sqlstr;
	CString sqlstr_cf;
	EIClass tb_tpssm11;

	CDataTable tb_tpssm03("TPSSM03");
	CDataTable tb_tpssmd3("TPSSMD3_TEST");
	CDataTable tb_tep0002("TEP0002");

	try
	{
		
		sqlstr = "   SELECT T1.STATION_ID||T1.STATION_NO AS DEV_CODE, T1.SM_PLAN_NOL2 AS PLAN_NO, NVL(T3.START_TIME,' ') START_TIME,   "
			"    NVL(T3.END_TIME,' ') END_TIME,DECODE(T1.START_TIME_REAL,' ',NVL(T3.START_TIME_REAL,' '),T1.START_TIME_REAL) START_TIME_REAL, DECODE(T1.END_TIME_REAL, ' ',NVL(T3.END_TIME_REAL,' '),T1.END_TIME_REAL) END_TIME_REAL,  "
			"   (SELECT STEEL_TEMP FROM TMMSM2B WHERE PROC_COUNT = (SELECT MAX(PROC_COUNT) FROM TMMSM2B WHERE HEAT_NO = T1.HEAT_NO   AND DEV_CODE = T1.STATION_ID || T1.STATION_NO) AND HEAT_NO = T1.HEAT_NO  AND DEV_CODE = T1.STATION_ID || T1.STATION_NO"
			" AND HEAT_NO<> ' ' ) AS TEMP,  "
			"	(SELECT CASTING_SPEED FROM TMMSM31A WHERE DEV_CODE = T1.STATION_ID || T1.STATION_NO AND  "
			"	PROD_SEQ_NO = (SELECT MAX(PROD_SEQ_NO) FROM TMMSM31A WHERE DEV_CODE = T1.STATION_ID || T1.STATION_NO)) CASTING_SPEED, "
			"	(SELECT LADLE_WEIGHT / 1000 FROM TMMSMCCM WHERE DEV_CODE = T1.STATION_ID || T1.STATION_NO AND  ID_SJ = (SELECT MAX(ID_SJ) FROM TMMSMCCM WHERE DEV_CODE = T1.STATION_ID || T1.STATION_NO))    LADLE_WEIGHT, "
			" CASE WHEN T3.END_TIME_REAL<> ' ' OR  T1.END_TIME_REAL<>' '  THEN (	case when TO_TIMESTAMP(DECODE(T1.END_TIME_REAL,' ',NVL(T3.END_TIME_REAL,' '),T1.END_TIME_REAL),'YYYY-MM-DD HH24:MI:SS') <SYSDATE-1/3 THEN 1  ELSE 0 END ) ELSE 0 END AS TC_FLAG,  "
			"		T1.*  FROM  TPSSM33 T1 LEFT JOIN TPSSM11 T2 ON T1.PONO=T2.PONO  LEFT JOIN  TPSSM12 T3 ON T3.SM_PLAN_NO=T2.SM_PLAN_NO AND  T3.DEV_CODE = T1.STATION_ID||T1.STATION_NO   ";
		
		cmd_inq.SetCommandText(sqlstr);
	
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
