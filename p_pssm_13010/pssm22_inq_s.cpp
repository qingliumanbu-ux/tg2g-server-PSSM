/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-9-12
Description:	查询系统的工序设备
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/*<remark>=========================================================
/// <summary>
/// 工序设备查询
/// <para>
/// 1.根据传入的厂别区分，查询工序设备。
/// 2.查询条件：厂别区分
///   从tpssmd1表搜索对应的工序设备信息；
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别代码    </param>
/// <returns>工序设备信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm22_inq_s)
//-EP_SYSTEM_HEAD_END
int f_pssm22_inq_s(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;
	int i = 0;

	CDbCommand cmd_tpssmd1_inq(conn);
	CString sqlstr;

	CModel tpssmd1("TPSSMD1");

	try
	{
		bcls_ret->Tables[0].set_TableName(CString("STATION_NAME"));
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CODE_DESC_1_CONTENT");
		

		//获得输入参数
		tpssmd1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];

		//打印输入参数
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", tpssmd1["FACTORY_DIV"].ToString());

		//查询转炉，精炼工序设备
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT DISTINCT STATION_ID FROM TPSSMD1 WHERE AREA_ID IN('3','4') ";
			if (tpssmd1["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr += " AND FACTORY_DIV = @FACTORY_DIV ";
			}
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while (cmd_tpssmd1_inq.Read())
		{
			tpssmd1["STATION_ID"] = cmd_tpssmd1_inq.GetString(1);
			////Log::Info("", __FUNCTION__, "STATION_ID = [{0}]", tpssmd1["STATION_ID"].ToString());

			//1)根据station_id查询工序名称
			if (tpssmd1["STATION_ID"].ToString() == "B")
			{
				tpssmd1["DEV_CODE"] = "B0";
				tpssmd1["STATION_NAME"] = "BOF";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "R")
			{
				tpssmd1["DEV_CODE"] = "R0";
				tpssmd1["STATION_NAME"] = "RH";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "L")
			{
				tpssmd1["DEV_CODE"] = "L0";
				tpssmd1["STATION_NAME"] = "LF";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "V")
			{
				tpssmd1["DEV_CODE"] = "V0";
				tpssmd1["STATION_NAME"] = "VD";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "A")
			{
				tpssmd1["DEV_CODE"] = "A0";
				tpssmd1["STATION_NAME"] = "AR";
			}
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["CODE"] = tpssmd1["DEV_CODE"]; //设备代码
			bcls_ret->Tables[0].Rows[i]["CODE_DESC_1_CONTENT"] = tpssmd1["STATION_NAME"]; //工序名称

			i++;
		}
		cmd_tpssmd1_inq.Close();

		//查询连铸工序设备
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT DEV_CODE,STATION_NAME FROM TPSSMD1 WHERE AREA_ID = 5 ";
			if (tpssmd1["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr += " AND FACTORY_DIV = @FACTORY_DIV ";
			}
			sqlstr += " ORDER  BY STATION_NO ASC ";
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while (cmd_tpssmd1_inq.Read())
		{
			tpssmd1["DEV_CODE"] = cmd_tpssmd1_inq.GetString(1);
			tpssmd1["STATION_NAME"] = cmd_tpssmd1_inq.GetString(2);

			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["CODE"] = tpssmd1["DEV_CODE"]; //设备代码
			bcls_ret->Tables[0].Rows[i]["CODE_DESC_1_CONTENT"] = tpssmd1["STATION_NAME"]; //工序名称

			i++;
		}
		cmd_tpssmd1_inq.Close();

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
