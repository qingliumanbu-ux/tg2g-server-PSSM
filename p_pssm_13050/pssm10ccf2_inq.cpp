/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-04-26
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/

//#include "tpssmsg.h"

/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM10(炼钢制造命令表)              </para>
/// <para>主调用函数：前台PSSM101CC画面F2(查询)按钮      </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                      </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm10ccf2_inq)

int f_pssm10ccf2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int		TotalRecordCount = 0;


	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm10("TPSSM10");

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

		//测试用
		//Log::Trace("", __FUNCTION__, " test TpssmSg = [{0}]", TpssmSg::getInstance().REC_ID);
		//if (TpssmSg::getInstance().REC_ID == "123")
		//	TpssmSg::getInstance().REC_ID = "456";
		//else
		//	TpssmSg::getInstance().REC_ID = "123";

		//--------------------------------
		//获取传入参数
		tpssm10.MergeFrom(bcls_rec->Tables[0].Rows[0]);


		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "tpssm01.PONO  =[{0}]",tpssm10.PONO );
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]",tpssm10["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "cc_mach_no        = [{0}]",tpssm10["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "cast_lot_no       = [{0}]",tpssm10["CAST_LOT_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status       = [{0}]",tpssm10["PONO_STATUS"].ToDecimal().ToInt32());


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr_count = " SELECT COUNT(1) "
				"   FROM TPSSM10 "
				"  WHERE 1=1 "
				;
			sqlstr = " SELECT * "
				"   FROM TPSSM10 "
				"  WHERE 1=1 "
				;

			if (tpssm10["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND FACTORY_DIV		= @tpssm10.FACTORY_DIV";
			}

			if (tpssm10["CC_MACH_NO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND CC_MACH_NO		= @tpssm10.CC_MACH_NO";
			}
			if (tpssm10["PONO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND PONO			like '%'|| @tpssm10.PONO||'%'";
			}
			if (tpssm10["CAST_LOT_NO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND CAST_LOT_NO		= @tpssm10.CAST_LOT_NO";
			}
			if (tpssm10["PONO_STATUS"].ToDecimal() != 0)
			{
				sqlstr_temp += " AND PONO_STATUS			= @tpssm10.PONO_STATUS";
			}

			sqlstr_temp_order += " ORDER BY CC_MACH_NO ASC, CC_SEQ ASC";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
			break;
		}
		cmd_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
		cmd_inq.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
		cmd_inq.Parameters.Set("tpssm10.CAST_LOT_NO", tpssm10["CAST_LOT_NO"].ToString());
		cmd_inq.Parameters.Set("tpssm10.PONO_STATUS", tpssm10["PONO_STATUS"].ToDecimal());


		cmd_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_inq.ExecuteScalar().ToInt32();
		//分页获取
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_inq.Close();

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

		bcls_ret->Tables[0].set_TableName("PSSM101_INQ");

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
