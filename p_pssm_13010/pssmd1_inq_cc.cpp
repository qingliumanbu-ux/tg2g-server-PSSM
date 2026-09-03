/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2011-11-30
Description:	查询系统的连铸机号, 即连铸机的工位号 前台的 query_ccm_info()函数调用
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/*<remark>=========================================================
/// <summary>
/// 连铸机号查询
/// <para>
/// 1.根据传入的厂别区分，查询连铸机号。
/// 2.查询条件：厂别区分
///   从tpssmd1表搜索对应的连铸机信息；
/// 3.排序方式：cc_mach_no ASC；
/// 4.查询该铸机的 铸机号 铸机类型 流数；
/// </para>
/// <para>数据库表：TPSSMC1(炼钢连铸设备参数表) TPSSMD1(炼钢作业计划设备代码表)          </para>
/// <para>主调用函数：前台 query_ccm_info()函数调用。   </para>
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别代码    </param>
/// <returns>连铸机信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssmd1_inq_cc)
//-EP_SYSTEM_HEAD_END
int f_pssmd1_inq_cc(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;

	CDbCommand cmd_ccm_info_inq(conn);
	CString sqlstr;

	CModel tpssmd1("TPSSMD1");

	try
	{
		bcls_ret->Tables[0].set_TableName(CString("CCM_INFO"));
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NAME");

		//获得输入参数
		tpssmd1["FACTORY_DIV"]=bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
			sqlstr = " SELECT STATION_NO CC_MACH_NO, STATION_NAME  FROM   TPSSMD1 WHERE AREA_ID  = 5 ";
			if (tpssmd1["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr += " AND FACTORY_DIV		= @tpssmd1.FACTORY_DIV ";
			}

			sqlstr += " ORDER  BY STATION_NO ASC ";
			break;
		}


		cmd_ccm_info_inq.SetCommandText(sqlstr);
		cmd_ccm_info_inq.Parameters.Set("tpssmd1.FACTORY_DIV",tpssmd1["FACTORY_DIV"].ToString());
		cmd_ccm_info_inq.ExecuteQuery(bcls_ret->Tables[0]);

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
