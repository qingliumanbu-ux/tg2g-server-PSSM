/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2014-10-29
Description:查询TPSSM23出钢计划应答表中信息。
**************************************************************************************************************/
#include "stdafx.h"

//程序用头文件
#include "tpssm33.h"
/*<remark>=========================================================
/// <summary>
/// 当前处理号查询
/// <para>数据库表：TPSSM23出钢计划应答表</para>
/// <para>主调用函数：前台 PSSM11画面F2调用。   </para>
/// </summary>
/// <returns>当前处理号信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_respond)

int f_pssm11_respond(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString v_factory_div = "";
	CString v_col_name = "";

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	CString sqlstr;
	CTPSSM33 tpssm33(conn);
	try
	{

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName(CString("TPSSM_ECHO"));
		bcls_ret->Tables["TPSSM_ECHO"].Rows.Add();

		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		sqlstr = "SELECT * FROM TPSSM33 WHERE FACTORY_DIV = @factory_div ORDER BY AREA_ID,STATION_ID,STATION_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", v_factory_div);

		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm33);
			tpssm33.TrimOrBlank();

			if (tpssm33.STATION_ID = "B")
			{
				v_col_name = "BOF";
			}
			else if (tpssm33.STATION_ID = "E")
			{
				v_col_name = "EAF";
			}
			else if (tpssm33.STATION_ID = "L")
			{
				v_col_name = "LF";
			}
			else if (tpssm33.STATION_ID = "R")
			{
				v_col_name = "RH";
			}
			else if (tpssm33.STATION_ID = "V")
			{
				v_col_name = "VD";
			}
			else if (tpssm33.STATION_ID = "C")
			{
				v_col_name = "CC";
			}
			else if (tpssm33.STATION_ID = "I")
			{
				v_col_name = "IC";
			}

			v_col_name = v_col_name + tpssm33.STATION_NO + "_NO";

			if (!bcls_ret->Tables[0].Columns.Contains(v_col_name))
			{
				bcls_ret->Tables[0].Columns.Add(DT_STRING, v_col_name);
			}

			bcls_rec->Tables[0].Rows[0][v_col_name] = tpssm33.CURR_PROC_NO;

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
