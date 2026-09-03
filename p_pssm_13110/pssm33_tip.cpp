/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   yl
Version:    1.0
Date:     2023-5-20
Description:工序状态跟踪查询（提示信息）
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include <cstring>

// service入口
BM2F_ENTERACE(pssm33_tip)

int f_pssm33_tip(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;
	CString sqlstr;

	//业务变量
	CString factory_div = "";
	CString station_id = "";
	CString station_no = "";
	CString heat_no = "";
	CString proc_no = "";
	CString pono = "";

	CDbCommand cmd_inq(conn);

	//系统的分页类信息。

	try
	{
		//获得输入参数
		if (bcls_rec->Tables[0].Columns.Contains("FACTORY_DIV"))
			factory_div = bcls_rec->Tables[0].Rows[0]["factory_div"].ToString().Trim();
		if (bcls_rec->Tables[0].Columns.Contains("STATION_ID"))
			station_id = bcls_rec->Tables[0].Rows[0]["station_id"].ToString().Trim();
		if (bcls_rec->Tables[0].Columns.Contains("STATION_NO"))
			station_no = bcls_rec->Tables[0].Rows[0]["station_no"].ToString().Trim();
		if (bcls_rec->Tables[0].Columns.Contains("HEAT_NO"))
			heat_no = bcls_rec->Tables[0].Rows[0]["heat_no"].ToString().Trim();
		if (bcls_rec->Tables[0].Columns.Contains("PROC_NO"))
			proc_no = bcls_rec->Tables[0].Rows[0]["proc_no"].ToString().Trim();
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
			pono = bcls_rec->Tables[0].Rows[0]["pono"].ToString().Trim();
		sqlstr = " SELECT * FROM TMMSMT1 WHERE ENABLE_FLAG = '1' ";
		if (factory_div != "")
		{
			sqlstr += " AND FACTORY_DIV = '" + factory_div + "'";
		}
		if (station_id != "")
		{
			sqlstr += " AND STATION_ID = '" + station_id + "'";
		}
		if (station_no != "")
		{
			sqlstr += " AND STATION_NO = '" + station_no + "'";
		}
		if (proc_no != "")
		{
			sqlstr += " AND PROC_NO LIKE '%" + proc_no + "%'";
		}
		if (heat_no != "")
		{
			sqlstr += " AND HEAT_NO LIKE '%" + heat_no + "%'";
		}
		if (pono != "")
		{
			sqlstr += " AND PONO LIKE '%" + pono + "%'";
		}
		Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		//cmd_inq.Close();



		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_ID");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "POS_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "TIP_MSG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FORM_NAME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ENABLE_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "TIP_LEVEL");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "TIP_COLOR");

		CDataRow & dr = bcls_ret->Tables[0].Rows.Add();
		dr["STATION_ID"] = "A";
		dr["STATION_NO"] = "1";
		dr["POS_NO"] = "1";
		dr["HEAT_NO"] = "B1300001";
		dr["PONO"] = "300001";
		dr["TIP_MSG"] = "吹氩实绩开始时刻缺失。";
		dr["FORM_NAME"] = "MMSM22SI";
		dr["ENABLE_FLAG"] = "0";
		dr["TIP_LEVEL"] = "1";
		dr["TIP_COLOR"] = "Red";

		CDataRow & dr1 = bcls_ret->Tables[0].Rows.Add();
		dr1["STATION_ID"] = "C";
		dr1["STATION_NO"] = "1";
		dr1["POS_NO"] = "1";
		dr1["HEAT_NO"] = "B1300002";
		dr1["PONO"] = "300002";
		dr1["TIP_MSG"] = "连铸实绩开始时刻缺失。";
		dr1["FORM_NAME"] = "MMSM31SI";
		dr1["ENABLE_FLAG"] = "0";
		dr1["TIP_LEVEL"] = "1";
		dr1["TIP_COLOR"] = "Black";
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
