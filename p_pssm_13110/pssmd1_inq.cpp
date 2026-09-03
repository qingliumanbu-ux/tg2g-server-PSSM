/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   yl
Version:    1.0
Date:     2023-5-20
Description:炼钢计划单表通用查询
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include <cstring>

// service入口
BM2F_ENTERACE(pssmd1_inq)

int f_pssmd1_inq(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;

	//业务变量
	CString table_name = "";
	CString factory_div = "";
	CString dev_tech_code = "";

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CString sqlstr;
	CString sqlstr_count;
	CString field_name = "";	//字段名
	CString sqlstr_temp;
	int TotalRecordCount = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;

	try
	{
		//获得输入参数
		table_name = bcls_rec->Tables[0].Rows[0]["TABLE_NAME"].ToString();
		CModel table_model(table_name);
		table_model.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		sqlstr = " SELECT * FROM  " + table_name +    "  WHERE 1 = 1";

		sqlstr_count =
			" SELECT COUNT(1)  FROM  " + table_name + "  WHERE 1 = 1   ";
				
		for (int i = 0; i < bcls_rec->Tables[0].Columns.get_Count(); i++)
		{
			
			field_name = bcls_rec->Tables[0].Columns[i].get_ColumnName();
			Log::Info("", __FUNCTION__, "field_name = [{0}]", field_name);
			if (table_model.GetFields().Contains(field_name))
			{
				if (table_model.GetFields()[field_name].ColumnType == DT_STRING &&table_model[field_name].ToString().Trim() != "")
				{
					sqlstr_temp += " AND ";
					sqlstr_temp += field_name + " LIKE @" + field_name + " ||'%' ";
					cmd_inq.Parameters.Set(field_name, table_model[field_name].ToString().Trim());
					cmd_inq_1.Parameters.Set(field_name, table_model[field_name].ToString().Trim());
				}
				if (table_model.GetFields()[field_name].ColumnType == DT_DECIMAL &&table_model[field_name].ToDecimal()!=0)
				{
					sqlstr_temp += " AND ";
					sqlstr_temp += field_name + " LIKE @" + field_name + " ||'%' ";
					cmd_inq.Parameters.Set(field_name, table_model[field_name].ToDecimal());
					cmd_inq_1.Parameters.Set(field_name, table_model[field_name].ToDecimal());
				}
			}
		}
			
		sqlstr_count = sqlstr_count + sqlstr_temp;
		sqlstr = sqlstr + sqlstr_temp;

		Log::Info("", __FUNCTION__, "sqlstr_count = [{0}]", sqlstr_count);
		Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);

		
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		cmd_inq_1.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_inq_1.ExecuteScalar().ToInt32();
		cmd_inq_1.Close();
		Log::Info("", __FUNCTION__, "TotalRecordCount[{0}]", TotalRecordCount);
		

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;


		Log::Trace("", __FUNCTION__, "==记录条数=[{0}]", bcls_ret->Tables[0].Rows.get_Count());


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
