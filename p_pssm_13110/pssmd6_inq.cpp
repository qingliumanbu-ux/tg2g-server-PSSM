/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-11-29
Description:	对炼钢作业计划设备传搁时间表进行查询。
**************************************************************************************************************/

#include "stdafx.h"



/*<remark>=========================================================
/// <summary>
/// 	炼钢作业计划设备传搁时间表查询
/// <para>根据前台传入的MAIN_BACKLOG_CDOE,	对炼钢作业计划设备传搁时间表进行查询。    </para>
/// <para>数据库表：tpssmd6(炼钢作业计划设备传搁时间表)                 </para>
/// <para>主调用函数：前台FormPSSMD6画面查询按钮调用。 </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <returns>设备传搁时间信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssmd6_inq)

int f_pssmd6_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag=0;
	CString sqlstr ="";
	// 定义表的实体对象
	CModel tpssmd6("TPSSMD6");
	CDbCommand cmd_inq(conn);

	try
	{
		// 获取前台传入参数
		tpssmd6["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		////Log::Trace("", __FUNCTION__, "factory_div=[{0}]", tpssmd6["FACTORY_DIV"].ToString());

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr= "SELECT * FROM TPSSMD6 WHERE FACTORY_DIV = @factory_div ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", tpssmd6["FACTORY_DIV"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		////Log::Trace("", __FUNCTION__, "query records. [{0}]", bcls_ret->Tables[0].Rows.get_Count() );

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}


