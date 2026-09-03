/*************************************************
Copyright:		Baosight Software LTD.co Copyright (c) 2013
Author:         JHZHAO
Version:		1.0
Date:			2014-10-14
Description:	炼钢生产计划查询
**************************************************/

//框架头文件
#include "stdafx.h"
//程序用头文件
#include "tpssm63.h"

using namespace BM2;
using namespace BM2::Data;
using namespace BM2::Data::DbClient;

//业务头文件


//函数申明

/*<remark>=========================================================
///<summary>
///炼钢生产计划查询
///<para>
///炼钢生产计划查询
///</para>
///<para>数据库表：TPSSM63炼钢生产计划表
///<returns>返回符合查询条件的炼钢生产计划信息</returns>
===========================================================</remark>*/

BM2F_ENTERACE(pssm38_inq);

int f_pssm38_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 实体类定义 */
	/* 数据库SQL操作字符串 */
	CString sqlstr = "";
	CString sqlwhere = "";
	CString date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CTPSSM63 tpssm63(conn);
	try
	{
		bcls_ret->Tables[0].set_TableName("TPSSM63");
		bcls_ret->Tables[0].Columns.Add(tpssm63);

		tpssm63.PROD_DATE = bcls_rec->Tables[0].Rows[0]["PROD_DATE"].ToString().Trim();

		Log::Trace("", __FUNCTION__, "tpssm63.PROD_DATE: {0}", (const char*)tpssm63.PROD_DATE);

		if (tpssm63.PROD_DATE.Trim() != "")
		{
			sqlwhere += "AND PROD_DATE = @prod_date ";
		}

		sqlstr = "SELECT COUNT(*) FROM TPSSM63 WHERE 1=1 ";
		sqlstr = sqlstr + sqlwhere;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date", tpssm63.PROD_DATE);
		CDecimal count = cmd_inq.ExecuteScalar();
		if (count <= 0)
		{
			sqlstr = "SELECT DISTINCT PROD_DATE FROM TPSSM63 WHERE PROD_DATE >= @date_time "
				     "ORDER BY PROD_DATE DESC";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("date_time", date_time.SubstringNE(0,6));
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tpssm63.PROD_DATE = cmd_inq.GetString(1);
			}
			cmd_inq.Close();
		}

		sqlstr = "SELECT * FROM TPSSM63 WHERE 1=1 ";
		sqlstr = sqlstr + sqlwhere;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date", tpssm63.PROD_DATE);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm63);
			tpssm63.TrimOrBlank();
			tpssm63.MergeTo(bcls_ret->Tables["TPSSM63"], false);
		}
		/* 关闭数据库操作类 */
		cmd_inq.Close();

	}
	catch (const CApplicationException& ex)
	{
		doFlag = ex.GetCode();
		strcpy(s.msg, (const char*)ex.GetMsg());
	}
	catch (const CException& ex)
	{
		doFlag = -1;
		strcpy(s.msg, (const char*)ex.GetMsg());
	}

	s.flag = doFlag;



	return(doFlag);
}