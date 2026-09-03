
#include "stdafx.h"
//using namespace BM2;
//using namespace BM2::Data;
//using namespace BM2::Data::DbClient;


// service入口
BM2F_ENTERACE(pssmdhv_F2_inq)

int f_pssmdhv_F2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 程序内部变量 ***** */
	int doFlag = 0;
	int fetchRowCount = 0;


	/* ***** 业务变量 ***** */
	int    RowCount = 0;
	CString  v_steelgrade = "";

	//实体类定义


	//数据库操作类定义
	CDbCommand cmd_inq(conn);


	// 数据库SQL操作字符串 
	CString  sqlstr("");
	CString  sqlstr_temp("");
	try
	{
		/////////////////////////////////////////////////////////// 
		//获取前台传入参数

		if (bcls_rec->Tables[0].Columns.Contains("STEEL_GRADE"))
		{
			v_steelgrade = bcls_rec->Tables[0].Rows[0]["STEEL_GRADE"];//内部钢种
		}
		Log::Trace("", __FUNCTION__, "传入参数v_steelgrade[{0}]", v_steelgrade);
		/* 查询钢种和工艺路线包关系 */
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT a.* "
				"   FROM TPSSMDH a "
				"  WHERE 1=1 ";

			if (v_steelgrade.Trim() != "")
			{
				sqlstr_temp += " AND a.STEEL_GRADE LIKE '%' || @v_steelgrade || '%' ";
			}

			sqlstr_temp += " ORDER BY a.REC_REVISE_TIME DESC";
			sqlstr = sqlstr + sqlstr_temp;
			break;
		}
		Log::Trace("", __FUNCTION__, "sqlstr				= [{0}]", (const char*)sqlstr);
		cmd_inq.Parameters.Clear();


		cmd_inq.SetCommandText(sqlstr);
		if (v_steelgrade.Trim() != "")
		{
			cmd_inq.Parameters.Set("v_steelgrade", v_steelgrade);
		}

		Log::Trace("", __FUNCTION__, "SQL = {0}", sqlstr);
		cmd_inq.ExecuteReader();
		//返回记录给table
		bcls_ret->Tables[0].Clear();
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
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
	cmd_inq.Close();
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
