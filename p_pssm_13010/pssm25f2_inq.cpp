/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM09画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm25f2_inq)

int f_pssm25f2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_where = "";
	int		TotalRecordCount = 0;



	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm05("TPSSM05");

	CDbCommand cmd_inq(conn);

	try
	{
		try
		{//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;
		}
		////Log::Info("", __FUNCTION__, "pageInfo	= [{0}][{1}]", pageInfo.RecordFrom, pageInfo.PageSize);

		//--------------------------------
		//获取传入参数
		tpssm05.MergeFrom(bcls_rec->Tables[0].Rows[0]);


		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", tpssm05["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "plan_backlog_code	= [{0}]", tpssm05["PLAN_BACKLOG_CODE"].ToString());
		////Log::Info("", __FUNCTION__, "plan_no	= [{0}]", tpssm05["PLAN_NO"].ToString());
		////Log::Info("", __FUNCTION__, "plan_status	= [{0}]", tpssm05["PLAN_STATUS"].ToString());

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr_count = " SELECT COUNT(1) "
				"   FROM TPSSM05 "
				"  WHERE 1=1 "
				;
			sqlstr = " SELECT * "
				"   FROM TPSSM05 "
				"  WHERE 1=1 "
				;

			if (tpssm05["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND FACTORY_DIV		= @tpssm05.FACTORY_DIV";
			}

			if (tpssm05["PLAN_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND PLAN_BACKLOG_CODE	= @tpssm05.PLAN_BACKLOG_CODE";
			}
			if (tpssm05["PLAN_NO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND PLAN_NO	= @tpssm05.PLAN_NO";
			}
			if (tpssm05["PLAN_STATUS"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND PLAN_STATUS	= @tpssm05.PLAN_STATUS";
			}

			sqlstr_temp_where = " ORDER BY PLAN_NO ASC, MAT_SEQ_NO ASC ";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_where;
			break;
		}
		////Log::Info("", __FUNCTION__, "sqlstr_count = [{0}]", sqlstr_count);

		cmd_inq.SetCommandText(sqlstr_count);
		cmd_inq.Parameters.Set("tpssm05.FACTORY_DIV", tpssm05["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm05.PLAN_BACKLOG_CODE", tpssm05["PLAN_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("tpssm05.PLAN_NO", tpssm05["PLAN_NO"].ToString());
		cmd_inq.Parameters.Set("tpssm05.PLAN_STATUS", tpssm05["PLAN_STATUS"].ToString());
		TotalRecordCount = cmd_inq.ExecuteScalar().ToInt32();
		////Log::Info("", __FUNCTION__, "TotalRecordCount = [{0}]", TotalRecordCount);

		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		//分页获取
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm05.FACTORY_DIV", tpssm05["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm05.PLAN_BACKLOG_CODE", tpssm05["PLAN_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("tpssm05.PLAN_NO", tpssm05["PLAN_NO"].ToString());
		cmd_inq.Parameters.Set("tpssm05.PLAN_STATUS", tpssm05["PLAN_STATUS"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_inq.Close();

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
