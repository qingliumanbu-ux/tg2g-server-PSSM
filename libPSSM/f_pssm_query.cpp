/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-16
Version:1.0
Description:
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "CUtils.h"

//程序用头文件



BM2_FUNCTION_EXPORT
int f_pssm_query(EIClass inblock_condition, EIClass inblock_source, EIClass& outblock_result, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int i = 0;
	int j = 0;
	int rows, rows2 = 0;
	CString v_condi_name[10] = { "" };
	CString v_condi[10] = { "" };

	CDecimal diff_time = 0;
	CString sqlstr = "";

	CString end_time_last = "";

	CDateTime start_timex;
	CDateTime end_timex;
	CDateTime end_time_lastx;
	CTimeSpan proc_time_dif;

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssmd3_inq(conn);
	CDbCommand cmd_tpssm12_inq2(conn);
	try
	{
		rows = inblock_condition.Tables[0].Columns.get_Count();
		rows2 = inblock_source.Tables[0].Rows.get_Count();
		if (rows == 0)
		{
			Log::Info("", __FUNCTION__, "没有查询条件，直接返回");
			return 0;
		}
		if (rows2 == 0)
		{
			Log::Info("", __FUNCTION__, "没有数据源，直接返回");
			return 0;
		}

		//PrintDataTable(inblock_condition.Tables[0]);
		//PrintDataTable(inblock_source.Tables[0]);
		for (i = 0; i < rows; i++)
		{
			v_condi_name[i] = inblock_condition.Tables[0].Columns[i].get_ColumnName();
			v_condi[i] = inblock_condition.Tables[0].Rows[0][v_condi_name[i]].ToString().Trim();
		}
			//Log::Info("", __FUNCTION__, "v_condi_name=[{0}],v_condi=[{1}]", v_condi_name, v_condi);

		//rows2 = inblock_source.Tables[0].Rows.get_Count();
		//Log::Info("", __FUNCTION__, "rows2=[{0}]", rows2);
		for (j = rows2 -1; j >= 0; j--)
		{
			//Log::Info("", __FUNCTION__, "inblock_source=[{0}],v_condi=[{1}]", inblock_source.Tables[0].Rows[j][v_condi_name].ToString().Trim(), v_condi);
			if (rows == 1)
			{
				if (v_condi[0] == inblock_source.Tables[0].Rows[j][v_condi_name[0]].ToString().Trim())
				{
					continue;
				}
				else
				{
					//Log::Info("", __FUNCTION__, "removerows=[{0}]", inblock_source.Tables[0].Rows[j][v_condi_name].ToString().Trim());
					inblock_source.Tables[0].Rows.Remove(j);
				}
			}
			else if (rows == 2)
			{
				if (v_condi[0] == inblock_source.Tables[0].Rows[j][v_condi_name[0]].ToString().Trim() 
					&& v_condi[1] == inblock_source.Tables[0].Rows[j][v_condi_name[1]].ToString().Trim())
				{
					continue;
				}
				else
				{
					//Log::Info("", __FUNCTION__, "removerows=[{0}]", inblock_source.Tables[0].Rows[j][v_condi_name].ToString().Trim());
					inblock_source.Tables[0].Rows.Remove(j);
				}
			}
			else if (rows == 3)
			{
				if (v_condi[0] == inblock_source.Tables[0].Rows[j][v_condi_name[0]].ToString().Trim()
					&& v_condi[1] == inblock_source.Tables[0].Rows[j][v_condi_name[1]].ToString().Trim()
					&& v_condi[2] == inblock_source.Tables[0].Rows[j][v_condi_name[2]].ToString().Trim())
				{
					continue;
				}
				else
				{
					//Log::Info("", __FUNCTION__, "removerows=[{0}]", inblock_source.Tables[0].Rows[j][v_condi_name].ToString().Trim());
					inblock_source.Tables[0].Rows.Remove(j);
				}
			}
			else if (rows == 4)
			{
				if (v_condi[0] == inblock_source.Tables[0].Rows[j][v_condi_name[0]].ToString().Trim()
					&& v_condi[1] == inblock_source.Tables[0].Rows[j][v_condi_name[1]].ToString().Trim()
					&& v_condi[2] == inblock_source.Tables[0].Rows[j][v_condi_name[2]].ToString().Trim()
					&& v_condi[3] == inblock_source.Tables[0].Rows[j][v_condi_name[3]].ToString().Trim())
				{
					continue;
				}
				else
				{
					//Log::Info("", __FUNCTION__, "removerows=[{0}]", inblock_source.Tables[0].Rows[j][v_condi_name].ToString().Trim());
					inblock_source.Tables[0].Rows.Remove(j);
				}
			}
			else if (rows == 5)
			{
				if (v_condi[0] == inblock_source.Tables[0].Rows[j][v_condi_name[0]].ToString().Trim()
					&& v_condi[1] == inblock_source.Tables[0].Rows[j][v_condi_name[1]].ToString().Trim()
					&& v_condi[2] == inblock_source.Tables[0].Rows[j][v_condi_name[2]].ToString().Trim()
					&& v_condi[3] == inblock_source.Tables[0].Rows[j][v_condi_name[3]].ToString().Trim()
					&& v_condi[4] == inblock_source.Tables[0].Rows[j][v_condi_name[4]].ToString().Trim())
				{
					continue;
				}
				else
				{
					//Log::Info("", __FUNCTION__, "removerows=[{0}]", inblock_source.Tables[0].Rows[j][v_condi_name].ToString().Trim());
					inblock_source.Tables[0].Rows.Remove(j);
				}
			}
		}
		//int rows5 = inblock_source.Tables[0].Rows.get_Count();
		//Log::Info("", __FUNCTION__, "rows5=[{0}]", rows5);
		outblock_result.Tables[0].Clear();
		outblock_result.Tables[0].Copy(inblock_source.Tables[0]);
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
