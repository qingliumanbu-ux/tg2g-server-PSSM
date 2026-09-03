/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhengqiangqiang
Version:  1.0
Date:     2023-05-12
Description: 查询炼钢作业准点率表
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "math.h"
//程序用头文件


// service入口
BM2F_ENTERACE(pssmt_cc_zd)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssmt_cc_zd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{

	CTracer log(__FUNCTION__);

	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount;
	int i;
	int cd_count = 0;
	int zy_time_count = 0;//作业时间
	int gz_time_count = 0;//故障时间

	CDecimal zdl = 0;//准点率
	CDecimal gzl = 0;//故障率

	CString	factory_div = "A10";
	CString	station_id = "B";

	CString	date_time_begin = "";
	CString	date_time_end = "";
	CString	date_time = "";
	CString	cs_start_time = "";
	CString	cs_end_time = "";
	CString	cs_dev_code = "";
	CString prod_shift_no = "";
	CString prod_shift_group = "";


	CString sqlstr("");

	/* 实体类定义 */
	CModel tpssmb1("TPSSMB1");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	try
	{
		//测试用
		date_time = CDateTime::Now().AddDays(-1).ToString("yyyyMMddHHmmss");//默认前一天

		//获得输入参数
		if (bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim() != "")
		{
			factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		}
		else
		{
			factory_div = "A10";
		}

		if (bcls_rec->Tables[0].Rows[0]["DATE_TIME_BEGIN"].ToString().Trim() != "")
		{
			cs_start_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME_BEGIN"].ToString().Substring(0, 8);
		}
		else
		{
			cs_start_time = date_time.Substring(0, 6) + "01";
		}

		if (bcls_rec->Tables[0].Rows[0]["DATE_TIME_END"].ToString().Trim() != "")
		{
			cs_end_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME_END"].ToString().Substring(0, 8);
		}
		else
		{
			cs_end_time = date_time.Substring(0, 8);
		}

		Log::Trace("", __FUNCTION__, "...打印传入参数...");
		Log::Trace("", __FUNCTION__, "...factory_div=[{0}]", factory_div);
		Log::Trace("", __FUNCTION__, "...cs_start_time=[{0}]", cs_start_time);
		Log::Trace("", __FUNCTION__, "...cs_end_time=[{0}]", cs_end_time);


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = " select PROD_TIME DATE_TIME	"
				" , GROUP1_DATA SHIFT_GROUP_A	"
				" , GROUP2_DATA SHIFT_GROUP_B	"
				" , GROUP3_DATA SHIFT_GROUP_C	"
				" , GROUP4_DATA SHIFT_GROUP_D	"
				" , RATIO_NUM  TOTAL_PRECENT	"
				" from tpssmb1					"
				" where PROD_TIME >= @cs_start_time	 "
				" AND PROD_TIME <= @cs_end_time	 "
				" AND FACTORY_DIV = @factory_div	 "
				" AND STATION_ID='C'	 "
				" order by PROD_TIME		"
				;
			break;
		}
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("cs_start_time", cs_start_time);
		cmd_inq.Parameters.Set("cs_end_time", cs_end_time);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "==记录条数=[{0}]", bcls_ret->Tables[0].Rows.get_Count());



	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)
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
