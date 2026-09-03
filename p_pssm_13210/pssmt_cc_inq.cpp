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
BM2F_ENTERACE(pssmt_cc_inq)

int f_pssmt_cc_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;

	//业务变量
	CString start_time = "";
	CString end_time = "";
	CString station_no = "";
	CString station_id = "";

	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{
		//获得输入参数
		station_no = bcls_rec->Tables[0].Rows[0]["STATION_NO"].ToString();
		start_time = bcls_rec->Tables[0].Rows[0]["START_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();

		Log::Trace("", __FUNCTION__, "station_no =[{0}]", station_no);


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
			sqlstr = " SELECT * FROM TMMSM31 WHERE 1=1   ";
			if (station_no != "")
			{
				sqlstr += " AND STATION_NO = @station_no ";
			}
			if (start_time.Trim() != "")
			{
				sqlstr += " AND REC_CREATE_TIME>=@start_time ";
			}
			if (end_time.Trim() != "")
			{
				sqlstr += " AND REC_CREATE_TIME<=@end_time ";
			}
			sqlstr += " ORDER  BY STATION_NO,HEAT_NO ASC ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("station_no", station_no);
		cmd_inq.Parameters.Set("start_time", start_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);

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
