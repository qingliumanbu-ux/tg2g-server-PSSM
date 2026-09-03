/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2016-01-14 19:13:56
Description: 钢水返送信息一览查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/




/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 钢水返送信息一览查询
/// <para>
/// 1.根据factory_div,时间等条件进行出钢计划查询。
///
/// </para>
/// <para>数据库表：TPSSM35钢水返送信息记录表)     </para>
/// <para>主调用函数：前台PSSM35画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm82_inq)

int f_pssm82_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString str = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_where = "";
	CString	prod_date_from = "";
	CString	prod_date_to = "";
	CString	start_time = "";
	CString	confrm_time = "";
	CString colname = " ";
	CString colname2 = " ";
	CString cs_dev_code = "";
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag = "0";
	CString cast_no_show = "";
	CString proc_no = "";    /* 生产处理号 */
	CString online_flag = "";
	int		TotalRecordCount = 0;
	int fetchRowCount = 0;
	int first_srf = 0; //精炼工序的第一个charge_no

	int ll;



	CDbCommand cmd_tpssm35_inq(conn);


	try
	{
	

		//--------------------------------
		//获取传入参数
		cs_dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"].ToString().Trim();
		prod_date_from = bcls_rec->Tables[0].Rows[0]["DATI_MSG_SENT_FROM"].ToString().Trim();
		prod_date_to = bcls_rec->Tables[0].Rows[0]["DATI_MSG_SENT_TO"].ToString().Trim();
		/* ***** 打印输入参数 ***** */

		////Log::Info("", __FUNCTION__, "打印输入参数factory_div = [{0}]", tpssm35["FACTORY_DIV"].ToString());
		////Log::Trace("", __FUNCTION__, "prod_date_from = [{0}]", prod_date_from);
		////Log::Trace("", __FUNCTION__, "FACTORY_DIV = [{0}]", tpssm35["FACTORY_DIV"].ToString());
		////Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm35["PONO"].ToString());
		////Log::Trace("", __FUNCTION__, "打印输入参数HEAT_NO = [{0}]", tpssm35["HEAT_NO"].ToString());

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr_count = " SELECT COUNT(1) "
				"   FROM  tpssm82  A"
				"   WHERE 1  = 1 ";

			sqlstr = " SELECT * "
				"   FROM tpssm82  A"
				"   WHERE 1  = 1  ";

			
			if (cs_dev_code.Trim() != "")
			{
				sqlstr_temp += "AND A.DEV_CODE = @cs_dev_code  ";
				
			}

			if (prod_date_from.Trim() != "")
			{
				sqlstr_temp += "AND A.DATI_MSG_SENT >= @prod_date_from  ";
				if (prod_date_from.GetLength() == 8)
					prod_date_from += "000000";
			}
			if (prod_date_to.Trim() != "")
			{
				sqlstr_temp += " AND A.DATI_MSG_SENT <= @prod_date_to ";
				if (prod_date_to.GetLength() == 8)
					prod_date_to += "235959";
			}
			sqlstr_temp_where += " ORDER BY  DATI_MSG_SENT  desc";//RET_TIME倒序排列

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_where;
			break;
		}
		cmd_tpssm35_inq.Parameters.Set("cs_dev_code", cs_dev_code);
		cmd_tpssm35_inq.Parameters.Set("prod_date_from", prod_date_from);
		cmd_tpssm35_inq.Parameters.Set("prod_date_to", prod_date_to);

		cmd_tpssm35_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm35_inq.ExecuteScalar().ToInt32();
		////Log::Trace("", __FUNCTION__, "TotalRecordCount = [{0}]", TotalRecordCount);

		//分页获取
		cmd_tpssm35_inq.SetCommandText(sqlstr);
		cmd_tpssm35_inq.ExecuteQuery(bcls_ret->Tables[0]);

		cmd_tpssm35_inq.Close();
		////Log::Trace("", __FUNCTION__, "sqlstr_count = [{0}]", sqlstr_count);

	

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
