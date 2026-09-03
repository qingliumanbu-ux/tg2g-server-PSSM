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
int f_pssm12z_combine(CString ladle_no, CString heat_no, CString sm_plan_no, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int i = 0;
	int j = 0;
	int rows, rows2 = 0;
	CString sqlstr = "";
	CString proc_no = "";
	CString proc_no2 = "";
	CString proc_no3 = "";
	CString proc_no4 = "";

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm12z("TPSSM12Z");
	CModel tpssm12zt("TPSSM12ZT");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm12z_inq(conn);
	try
	{
		sqlstr = " SELECT PROC_NO,PROC_NO2,PROC_NO3,PROC_NO4 FROM TPSSM12ZT WHERE LADLE_NO = @ladle_no ";
		cmd_tpssm12z_inq.SetCommandText(sqlstr);
		cmd_tpssm12z_inq.Parameters.Set("ladle_no", ladle_no);
		cmd_tpssm12z_inq.ExecuteReader();
		if (cmd_tpssm12z_inq.Read())
		{
			proc_no = cmd_tpssm12z_inq.GetString(1);
			proc_no2 = cmd_tpssm12z_inq.GetString(2);
			proc_no3 = cmd_tpssm12z_inq.GetString(3);
			proc_no4 = cmd_tpssm12z_inq.GetString(4);
		}
		cmd_tpssm12z_inq.Close();

		sqlstr = " update TPSSM12Z set SM_PLAN_NO=' ',PONO=' ',ST_NO=' ',HEAT_NO=' ',SM_PLAN_NOL2=' ' where HEAT_NO='" + heat_no + "'  ";
		cmd_tpssm12z_inq.SetCommandText(sqlstr);
		cmd_tpssm12z_inq.ExecuteNonQuery();

		if (proc_no.Trim() != "")
		{
			tpssm12z.Reset();
			tpssm12z["PROC_NO"] = proc_no;
			sqlstr = " SELECT * FROM TPSSM12Z WHERE PROC_NO = @PROC_NO ";
			cmd_tpssm12z_inq.SetCommandText(sqlstr);
			cmd_tpssm12z_inq.Parameters.Set("PROC_NO", tpssm12z["PROC_NO"].ToString().Trim());
			cmd_tpssm12z_inq.ExecuteReader();
			if (cmd_tpssm12z_inq.Read())
			{
				cmd_tpssm12z_inq.Fetch(tpssm12z);
				tpssm11.Reset();
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11.Query("SM_PLAN_NO");

				if (tpssm12z["HEAT_NO"].ToString().Trim() == "")
				{
					tpssm12z["SM_PLAN_NO"] = sm_plan_no;
					tpssm12z["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
					tpssm12z["PONO"] = tpssm11["PONO"];
					tpssm12z["ST_NO"] = tpssm11["ST_NO"];
					tpssm12z["HEAT_NO"] = heat_no;
					tpssm12z["DESTION_BACKLOG"] = heat_no.SubstringNE(0, 2);

					if (tpssm12z["GROUP_NO"].ToDecimal() == 0)//无分组
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "PROC_NO");
					}
					else
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "GROUP_NO");
					}
				}
			}
			cmd_tpssm12z_inq.Close();
		}
		
		if (proc_no2.Trim() != "")
		{
			tpssm12z.Reset();
			tpssm12z["PROC_NO"] = proc_no2;
			sqlstr = " SELECT * FROM TPSSM12Z WHERE PROC_NO = @PROC_NO ";
			cmd_tpssm12z_inq.SetCommandText(sqlstr);
			cmd_tpssm12z_inq.Parameters.Set("PROC_NO", tpssm12z["PROC_NO"].ToString().Trim());
			cmd_tpssm12z_inq.ExecuteReader();
			if (cmd_tpssm12z_inq.Read())
			{
				cmd_tpssm12z_inq.Fetch(tpssm12z);
				tpssm11.Reset();
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11.Query("SM_PLAN_NO");

				if (tpssm12z["HEAT_NO"].ToString().Trim() == "")
				{
					tpssm12z["SM_PLAN_NO"] = sm_plan_no;
					tpssm12z["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
					tpssm12z["PONO"] = tpssm11["PONO"];
					tpssm12z["ST_NO"] = tpssm11["ST_NO"];
					tpssm12z["HEAT_NO"] = heat_no;
					tpssm12z["DESTION_BACKLOG"] = heat_no.SubstringNE(0, 2);

					if (tpssm12z["GROUP_NO"].ToDecimal() == 0)//无分组
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "PROC_NO");
					}
					else
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "GROUP_NO");
					}
				}
			}
			cmd_tpssm12z_inq.Close();
		}

		if (proc_no3.Trim() != "")
		{
			tpssm12z.Reset();
			tpssm12z["PROC_NO"] = proc_no3;
			sqlstr = " SELECT * FROM TPSSM12Z WHERE PROC_NO = @PROC_NO ";
			cmd_tpssm12z_inq.SetCommandText(sqlstr);
			cmd_tpssm12z_inq.Parameters.Set("PROC_NO", tpssm12z["PROC_NO"].ToString().Trim());
			cmd_tpssm12z_inq.ExecuteReader();
			if (cmd_tpssm12z_inq.Read())
			{
				cmd_tpssm12z_inq.Fetch(tpssm12z);
				tpssm11.Reset();
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11.Query("SM_PLAN_NO");

				if (tpssm12z["HEAT_NO"].ToString().Trim() == "")
				{
					tpssm12z["SM_PLAN_NO"] = sm_plan_no;
					tpssm12z["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
					tpssm12z["PONO"] = tpssm11["PONO"];
					tpssm12z["ST_NO"] = tpssm11["ST_NO"];
					tpssm12z["HEAT_NO"] = heat_no;
					tpssm12z["DESTION_BACKLOG"] = heat_no.SubstringNE(0, 2);

					if (tpssm12z["GROUP_NO"].ToDecimal() == 0)//无分组
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "PROC_NO");
					}
					else
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "GROUP_NO");
					}
				}
			}
			cmd_tpssm12z_inq.Close();
		}

		if (proc_no4.Trim() != "")
		{
			tpssm12z.Reset();
			tpssm12z["PROC_NO"] = proc_no4;
			sqlstr = " SELECT * FROM TPSSM12Z WHERE PROC_NO = @PROC_NO ";
			cmd_tpssm12z_inq.SetCommandText(sqlstr);
			cmd_tpssm12z_inq.Parameters.Set("PROC_NO", tpssm12z["PROC_NO"].ToString().Trim());
			cmd_tpssm12z_inq.ExecuteReader();
			if (cmd_tpssm12z_inq.Read())
			{
				cmd_tpssm12z_inq.Fetch(tpssm12z);
				tpssm11.Reset();
				tpssm11["SM_PLAN_NO"] = sm_plan_no;
				tpssm11.Query("SM_PLAN_NO");

				if (tpssm12z["HEAT_NO"].ToString().Trim() == "")
				{
					tpssm12z["SM_PLAN_NO"] = sm_plan_no;
					tpssm12z["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
					tpssm12z["PONO"] = tpssm11["PONO"];
					tpssm12z["ST_NO"] = tpssm11["ST_NO"];
					tpssm12z["HEAT_NO"] = heat_no;
					tpssm12z["DESTION_BACKLOG"] = heat_no.SubstringNE(0, 2);

					if (tpssm12z["GROUP_NO"].ToDecimal() == 0)//无分组
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "PROC_NO");
					}
					else
					{
						tpssm12z.Update("SM_PLAN_NO,SM_PLAN_NOL2,PONO,ST_NO,HEAT_NO,DESTION_BACKLOG", "GROUP_NO");
					}
				}
			}
			cmd_tpssm12z_inq.Close();
		}
	
		tpssm12zt["LADLE_NO"] = ladle_no;
		tpssm12zt["PROC_NO"] = " ";
		tpssm12zt["PROC_NO2"] = " ";
		tpssm12zt["PROC_NO3"] = " ";
		tpssm12zt["PROC_NO4"] = " ";
		tpssm12zt["SMELT_MODE2"] = " ";
		tpssm12zt["REMARK"] = " ";
		tpssm12zt.Update("PROC_NO,PROC_NO2,PROC_NO3,PROC_NO4,SMELT_MODE2,REMARK", "LADLE_NO");
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
