/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-11-12
Description: 制造命令号对应的板坯查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/

//#include "tom01.h"

// service入口
BM2F_ENTERACE(pssm02l_slab_inq)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssm02l_slab_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__); 	//系统日志类定义

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;

	/* 业务变量 */
	CString lslab_no_pre = "";
	CString fix_len = "";

	/* 数据库SQL操作字符串 */
	CString sqlstr = "";
	

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm03_inq(conn);

	CModel tpssm03("TPSSM03");
	//CTOM01 tom01(conn);

	try
	{
		//获取传入参数
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
		{
			tpssm03["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();
		}
		//打印传入参数
		////Log::Info("", __FUNCTION__, "pssm02l_slab_inq>tpssm03["PONO"] = [{0}]", tpssm03["PONO"].ToString());

		/* 逻辑处理 */
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:

			sqlstr = " SELECT * FROM TPSSM03 "
				" WHERE PONO = @PONO "
				" ORDER BY LSLAB_NO ";

			break;
		}
		cmd_tpssm03_inq.SetCommandText(sqlstr);
		cmd_tpssm03_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString());
		ret = cmd_tpssm03_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_tpssm03_inq.Close();
		
		if (!bcls_ret->Tables[0].Columns.Contains("FIX_LEN"))
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "FIX_LEN"); //定尺长度
		}

		if (!bcls_ret->Tables[0].Columns.Contains("DELIVY_DATE"))
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "DELIVY_DATE"); //交货日期
		}

		for (int i = 0; i < ret; i++)
		{
			
			if (bcls_ret->Tables[0].Rows[i]["LSLAB_NO"].ToString() != lslab_no_pre)
			{
				if (bcls_ret->Tables[0].Rows[i]["BILLET_TYPE"].ToString() == "1") //板坯
				{
					sqlstr = "SELECT LSLAB_NO,SUM(SLAB_LEN) FIX_LEN FROM TPSSM03 WHERE LSLAB_NO = @LSLAB_NO GROUP BY LSLAB_NO ";
				}
				else //方坯
				{
					sqlstr = "SELECT distinct LSLAB_NO,SLAB_LEN AS FIX_LEN FROM TPSSM03 WHERE LSLAB_NO = @LSLAB_NO ";
				}
				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("LSLAB_NO", bcls_ret->Tables[0].Rows[i]["LSLAB_NO"]);
				cmd_tpssm03_inq.ExecuteReader();
				if (cmd_tpssm03_inq.Read())
				{
					fix_len = cmd_tpssm03_inq.GetString(2);					
				}
				cmd_tpssm03_inq.Close();

				////Log::Info("", __FUNCTION__, "fix_len = [{0}]", fix_len);

				bcls_ret->Tables[0].Rows[i]["FIX_LEN"] = fix_len;

				lslab_no_pre = bcls_ret->Tables[0].Rows[i]["LSLAB_NO"];
			}

			//tom01.ORDER_NO = bcls_ret->Tables[0].Rows[i]["ORDER_NO"];
			//sqlstr = "tom01.query()";
			//ret = tom01.QueryCount("ORDER_NO");
			//if (ret == 1)
			//{
			//	tom01.Query("ORDER_NO");

			//	bcls_ret->Tables[0].Rows[i]["DELIVY_DATE"] = tom01.DELIVY_DATE;
			//}
			
		}
		

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
	cmd_tpssm03_inq.Close();
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}




