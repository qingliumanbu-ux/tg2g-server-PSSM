/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1.0
Date:     2015-5-4
Description: 查询炼钢各工序处理号。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"




/*<remark>=========================================================
/// <summary>
/// 查询炼钢各工序处理号
/// <para>数据库表：TPSSM25(炼钢作业计划处理号维护表) TPSSMD1(炼钢作业计划设备代码表)          </para>
/// <para>主调用函数：前台 PSSM11画面F2调用。   </para>
/// </summary>
/// <returns>当前处理号信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11proc_inq)

int f_pssm11proc_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	CString dev_code = "";
	CString dev_name = "";
	CString factory_div = "";
	CDecimal area_id = 0;

	CModel tpssm25("TPSSM25");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);


	try
	{

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName("TPSSM25");  //与Client端dS_PSSM11ProcNo.TPSSM25表保持一致
		//bcls_ret->Tables[0].Columns.Add(tpssm25);
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NAME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "AREA_ID");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CURR_PROC_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_ID");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NO");

		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		sqlstr = CString(
			"SELECT a.DEV_CODE, a.STATION_NAME, a.AREA_ID, b.* "
			"  FROM TPSSMD1 a left join TPSSM25 b on (a.STATION_ID = b.STATION_ID AND a.STATION_NO = B.STATION_NO) "
			"  WHERE a.FACTORY_DIV = @FACTORY_DIV "
			" ORDER BY a.AREA_ID ASC, a.DEV_CODE ASC "
			);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
		////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);

		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssm25.Reset();

			dev_code = cmd_inq.GetString(1);
			dev_name = cmd_inq.GetString(2);
			area_id = cmd_inq.GetDecimal(3);
			cmd_inq.Fetch(tpssm25, 4);

			tpssm25.TrimOrBlank();

			CDataRow& row = bcls_ret->Tables["TPSSM25"].Rows.Add();
			row.Merge(tpssm25);
			
			row["DEV_CODE"] = dev_code;
			row["STATION_NAME"] = dev_name;
			row["AREA_ID"] = area_id;

		}//while 结束
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
