/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-11-12
Description: 板坯对应的合同信息查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/


// service入口
BM2F_ENTERACE(pssm02l_order_inq)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssm02l_order_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__); 	//系统日志类定义

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;

	/* 业务变量 */
	CString slab_no = "";
	CString factory_div = "";

	/* 数据库SQL操作字符串 */
	CString sqlstr = "";	

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	CModel tpssm03("TPSSM03");

	try
	{
		//获取传入参数
		if (bcls_rec->Tables[0].Columns.Contains("SLAB_NO"))
		{
			slab_no = bcls_rec->Tables[0].Rows[0]["SLAB_NO"].ToString().Trim();
		}
		if (bcls_rec->Tables[0].Columns.Contains("FACTORY_DIV"))
		{
			factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		}

		//打印传入参数
		////Log::Info("", __FUNCTION__, "pssm02l_order_inq>tpssm03["SLAB_NO"] = [{0}]", slab_no);

		tpssm03["FACTORY_DIV"] = factory_div;
		tpssm03["SLAB_NO"] = slab_no;

		if (tpssm03.Query("FACTORY_DIV, SLAB_NO") == false)
		{
			CFormattable arguments[] = { slab_no }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "命令板坯信息[{0}]已删除，请重新查询数据。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpssm03["SLAB_DEST"].ToString() == "10")
		{
			/* 逻辑处理 */
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

				sqlstr = " SELECT A.ORDER_NO, "
					" A.ORDER_THICK, "
					" A.ORDER_WIDTH, "
					" A.ORDER_LEN, "
					" A.SMALL_PLATE_NUM, "
					" A.PSC, "
					" B.PSC_DESC, "
					" A.DELIVY_DATE "
					" FROM TPMOUHP32 A, TQMTP01 B "
					" WHERE A.PSC = B.PSC "
					" AND A.PONO_SLAB = @SLAB_NO ";

				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("SLAB_NO", slab_no);
			cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
			ret = cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		}
		else
		{
			/* 逻辑处理 */
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

				sqlstr = " SELECT A.ORDER_NO, "
					" B.ORDER_THICK, "
					" B.ORDER_WIDTH, "
					" B.ORDER_LEN, "
					" B.PSC, "
					" B.DELIVY_DATE "
					" FROM TPSSM03 A, TOM01 B "
					" WHERE A.ORDER_NO = B.ORDER_NO "
					" AND A.SLAB_NO = @SLAB_NO "
					" AND A.FACTORY_DIV = @FACTORY_DIV ";

				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("SLAB_NO", slab_no);
			cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
			ret = cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
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
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}
