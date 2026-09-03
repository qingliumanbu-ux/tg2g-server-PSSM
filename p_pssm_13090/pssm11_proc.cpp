/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2014-10-29
Description:查询tpssm33表中的当前处理号信息。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 当前处理号查询
/// <para>数据库表：tpssm33(炼钢作业计划处理号维护表) TPSSMD1(炼钢作业计划设备代码表)          </para>
/// <para>主调用函数：前台 PSSM11画面F2调用。   </para>
/// </summary>
/// <returns>当前处理号信息</returns>
===========================================================</remark>*/
BM2F_ENTERACE(pssm11_proc)

int f_pssm11_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	CString v_factory_div = "";
	CString v_col_name = "";

	CModel tpssm33("TPSSM33");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	try
	{

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName(CString("TPSSM_PROCNO"));
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NAME");

		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();

		sqlstr = "SELECT * FROM TPSSM33 WHERE FACTORY_DIV = @factory_div ORDER BY AREA_ID,STATION_ID,STATION_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", v_factory_div);

		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm33);
			tpssm33.TrimOrBlank();

			v_col_name = "";
			if (tpssm33["STATION_ID"].ToString() == "B")
			{
				v_col_name = "BOF";
			}
			else if (tpssm33["STATION_ID"].ToString() == "E")
			{
				v_col_name = "EAF";
			}
			else if (tpssm33["STATION_ID"].ToString() == "A")
			{
				v_col_name = "LATS";
			}
			else if (tpssm33["STATION_ID"].ToString() == "L")
			{
				v_col_name = "LF";
			}
			else if (tpssm33["STATION_ID"].ToString() == "R")
			{
				v_col_name = "RH";
			}
			else if (tpssm33["STATION_ID"].ToString() == "V")
			{
				v_col_name = "VD";
			}
			else if (tpssm33["STATION_ID"].ToString() == "C")
			{
				v_col_name = "CC";
			}
			else if (tpssm33["STATION_ID"].ToString() == "I")
			{
				v_col_name = "IC";
			}

			if (v_col_name.Trim() != "")
			{
				v_col_name = v_col_name + tpssm33["STATION_NO"].ToString() + "_NO";

				////Log::Trace("", __FUNCTION__, "新增列：tpssm33["STATION_ID"] =[{0}], v_col_name=[{1}]", tpssm33["STATION_ID"].ToString(), v_col_name);

				if (!bcls_ret->Tables[0].Columns.Contains(v_col_name))
				{
					bcls_ret->Tables[0].Columns.Add(DT_STRING, v_col_name);
				}
			}
		}//while 结束
		cmd_inq.Close();
		
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[0]["NAME"] = "熔炼号";
		bcls_ret->Tables[0].Rows[1]["NAME"] = "处理号";

		sqlstr = "SELECT * FROM TPSSM33 WHERE FACTORY_DIV = @factory_div ORDER BY AREA_ID,STATION_ID,STATION_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", v_factory_div);

		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm33);
			tpssm33.TrimOrBlank();

			v_col_name = "";
			if (tpssm33["STATION_ID"].ToString() == "B")
			{
				v_col_name = "BOF";
			}
			else if (tpssm33["STATION_ID"].ToString() == "E")
			{
				v_col_name = "EAF";
			}
			else if (tpssm33["STATION_ID"].ToString() == "A")
			{
				v_col_name = "LATS";
			}
			else if (tpssm33["STATION_ID"].ToString() == "L")
			{
				v_col_name = "LF";
			}
			else if (tpssm33["STATION_ID"].ToString() == "R")
			{
				v_col_name = "RH";
			}
			else if (tpssm33["STATION_ID"].ToString() == "V")
			{
				v_col_name = "VD";
			}
			else if (tpssm33["STATION_ID"].ToString() == "C")
			{
				v_col_name = "CC";
			}
			else if (tpssm33["STATION_ID"].ToString() == "I")
			{
				v_col_name = "IC";
			}

			if (v_col_name.Trim() != "")
			{
				v_col_name = v_col_name + tpssm33["STATION_NO"].ToString() + "_NO";
				////Log::Trace("", __FUNCTION__, "新增行：v_col_name = [{0}], HEAT_NO =[{1}], CURR_PROC_NO =[{2}]", tpssm33["STATION_ID"].ToString(), v_col_name, tpssm33["HEAT_NO"].ToString(), tpssm33["CURR_PROC_NO"].ToString());
				bcls_ret->Tables[0].Rows[0][v_col_name] = tpssm33["HEAT_NO"];
				bcls_ret->Tables[0].Rows[1][v_col_name] = tpssm33["CURR_PROC_NO"];
			}
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
