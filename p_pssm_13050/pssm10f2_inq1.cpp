/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2014-06-25
Description: 制造命令号对应的板坯查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/ 
// New Include
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/ 



// service入口
BM2F_ENTERACE(pssm10f2_inq1)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssm10f2_inq1(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{

	
CTracer log(__FUNCTION__); 	//系统日志类定义

	/* 程序内部变量 */
	int doFlag	= 0;
	int blkNum	= 0;

	/* 业务变量 */
	CString	datetime("");  
	CString v_table_type  = "";

	
	/* 数据库SQL操作字符串 */
	CString sqlstr;

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	CModel tpssm03("TPSSM03");

	try
	{	

	
		if(bcls_rec->Tables[0].Columns.Contains("PONO"))
		tpssm03["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();
	
		Log::Info("", __FUNCTION__, "PONO=[{0}]",tpssm03["PONO"].ToString());
	
			

		/* 逻辑处理 */
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

		
				sqlstr = " SELECT T.SLAB_LEN||'*'||T.SLAB_WIDTH||'*'||t.SLAB_THICK AS SLAB_SIZE,"
					"  (SELECT ATTRI_NUM_ATFLV||'-'||ATTRI_NUM_ATFLB  FROM TQMOM02 WHERE ATTRI_ITEM ='A000' AND ORDER_NO =t.ORDER_NO)  AS A000_XS,  "
					"  (SELECT ATTRI_NUM_ATFLV||'-'||ATTRI_NUM_ATFLB  FROM TQMOM02 WHERE ATTRI_ITEM ='A001' AND ORDER_NO =t.ORDER_NO)  AS A001_XS,  "
					"  (SELECT ATTRI_NUM_ATFLV||'-'||ATTRI_NUM_ATFLB  FROM TQMOM02 WHERE ATTRI_ITEM ='A002' AND ORDER_NO =t.ORDER_NO)  AS A002_XS,  "
					" T.* FROM tpssm03  t   "
					" WHERE PONO =@pono   ORDER BY  SLAB_SEQ_2 ASC" ;
						 				 
				
				////Log::Info("", __FUNCTION__, "sqlstr      =[{0}]", sqlstr);

     		break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("pono", tpssm03["PONO"].ToString());
		bcls_ret->Tables[0].set_TableName("TPSSM03");
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	




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
	cmd_inq.Close();
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
} 




