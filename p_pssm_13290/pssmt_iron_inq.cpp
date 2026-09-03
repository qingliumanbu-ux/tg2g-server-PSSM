/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhengqq
Version:    1.0
Date:     2023-5-1
Description:转炉工序-时间流信息
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

// service入口
BM2F_ENTERACE(pssmt_iron_inq)

int f_pssmt_iron_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;

	//业务变量
	CString start_time="";
	CString end_time = "";
	CString station_no = "";
	CString station_id = "";
	CString date_time = "";

	CDecimal iron_temp1 = 0; 
	CDecimal iron_count1 = 0;
	CDecimal iron_temp2 = 0;
	CDecimal iron_count2 = 0;
	CDecimal iron_temp_total = 0;
	CDecimal iron_num_total = 0;

	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{

		//设置返回块
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DATE_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "FACTORY_DIV_1"); 
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "FACTORY_DIV_2"); 
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_PRECENT"); 



		//获得输入参数
		start_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME_BEGIN"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME_END"].ToString();

		Log::Trace("", __FUNCTION__, "==start_time =[{0}]", start_time);
		Log::Trace("", __FUNCTION__, "==end_time =[{0}]", end_time);


		switch (conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库															 
				sqlstr = " SELECT SUBSTRING(START_TIME, 1, 8)	DATE_TIME					 "
					" , SUM(CASE WHEN FACTORY_DIV = 'A10' THEN IRON_TEMP ELSE 0 END) IRON_TEMP1  "
					" , SUM(CASE WHEN FACTORY_DIV = 'A10' THEN 1 ELSE 0 END) IRON_COUNT1		 "
					" , SUM(CASE WHEN FACTORY_DIV = 'A20' THEN IRON_TEMP ELSE 0 END) IRON_TEMP2	 "
					" , SUM(CASE WHEN FACTORY_DIV = 'A20' THEN 1 ELSE 0 END) IRON_COUNT2			 "
					" , SUM(IRON_TEMP) IRON_TEMP_TOTAL											 "
					" , count(*) IRON_NUM_TOTAL													 "
					" FROM TMMSM21																 "
					" WHERE 1 = 1														   ";
				if (start_time.Trim()!= "")
				{
					sqlstr += " AND START_TIME>=@start_time ";
				}
				if (end_time.Trim()!= "")
				{
					sqlstr += " AND START_TIME<=@end_time ";
				}
				sqlstr += " GROUP BY SUBSTRING(START_TIME,1,8) ";
				break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("start_time", start_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			date_time = cmd_inq.GetString(1).Trim();
			iron_temp1 = cmd_inq.GetDecimal(2).ToDouble();
			iron_count1 = cmd_inq.GetDecimal(3).ToDouble();
			iron_temp2 = cmd_inq.GetDecimal(4).ToDouble();
			iron_count2 = cmd_inq.GetDecimal(5).ToDouble();
			iron_temp_total = cmd_inq.GetDecimal(6).ToDouble();
			iron_num_total = cmd_inq.GetDecimal(7).ToDouble();

			CDataRow & row= bcls_ret->Tables[0].Rows.Add();
			row["DATE_TIME"] = date_time;
			row["FACTORY_DIV_1"] = iron_count1 > 0 ? (iron_temp1 / iron_count1) * 10000 : 0;
			row["FACTORY_DIV_2"] = iron_count2 > 0 ? (iron_temp2 / iron_count2) * 10000 : 0;
			row["TOTAL_PRECENT"] = iron_num_total > 0 ? (iron_temp_total / iron_num_total) * 10000 : 0;

		}
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "==记录条数=[{0}]", bcls_ret->Tables[0].Rows.get_Count());


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = {  ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg)-1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
