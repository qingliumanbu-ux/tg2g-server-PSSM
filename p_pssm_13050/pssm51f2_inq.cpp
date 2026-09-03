/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-5
Version:1.0
Description: 模拟运转画面的计划查询 
Update: 2015-04-07 lijie 更新数据表
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

int f_pssm51f2_inq(EIClass *bcls_rec,EIClass *bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 模拟运转画面的计划查询
/// <para>查询当前下达的计划内容，即炼钢计划运行跟踪表。     </para>
/// <para>数据库表：tpssm11(炼钢计划表)              </para>
/// <para>主调用函数：前台PSSM51画面F2(查询)调用。           </para>
/// </summary>
/// <param name="factory_div">炼钢主工序代码    </param>
/// <returns>设备当前作业信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm51f2_inq)

//-EP_SYSTEM_HEAD_END
int f_pssm51f2_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	/* 程序用变量 */
	int doFlag = 0;
	CString sqlstr;
			

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm11_inq(conn);

	CModel tpssm11("TPSSM11");
	

	try
	{

		
		if(bcls_rec->Tables[0].Columns.Contains("FACTORY_DIV"))
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
	
		////Log::Info("", __FUNCTION__, "tpssm11["FACTORY_DIV"] =[{0}]",tpssm11["FACTORY_DIV"].ToString());
	
			

		/* 逻辑处理 */
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

		
				sqlstr = " SELECT * FROM TPSSM11 ";
				if (tpssm11["FACTORY_DIV"].ToString().Trim()!="")
				{
					sqlstr = sqlstr + " WHERE FACTORY_DIV = @factory_div ";

				}
									
				sqlstr = sqlstr +	" ORDER BY  CAST_NO ASC, CAST_DIV_NO ASC";
				
				//" AND   PONO_STATUS < 83" 20150825 lijie 
						 				 
				
				////Log::Info("", __FUNCTION__, "sqlstr      =[{0}]", sqlstr);

     		break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//bcls_ret->Tables[0].set_TableName("TPSSM11");
		cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_tpssm11_inq.Close();


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
