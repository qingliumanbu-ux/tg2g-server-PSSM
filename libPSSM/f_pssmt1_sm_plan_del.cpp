/*****************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2012
Author:    JHZHAO
Version:   1.0
Date:      2012-08-02
Description: del tpssmt1 info
******************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
using namespace BM2;
using namespace BM2::Data;
using namespace BM2::Data::DbClient;

//程序用头文件
#include "tpssmt1.h"
#include "tpssm13.h"
#include "tpssm11.h"

/*<remark>=========================================================
/// <summary>
///根据tpssm13(炼钢计划主表）中炼钢计划新增删除操作tpssmt1(铁水预处理表)
/// 删除tpssmt1(铁水预处理表)中已经被删除的出钢计划中对应的铁水预计划
///tpssm13(炼钢计划主表）中如果已经下达的没有铁水预计划的炼钢计划进行新增铁水预计划
///主调用函数PSSM11画面F6下达
===========================================================</remark>*/


int f_pssmt1_sm_plan_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	int		doFlag=0;
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	/* ***** 业务变量 ***** */

	CTPSSMT1 tpssmt1(conn);
	CTPSSM13 tpssm13(conn);
	CTPSSM11 tpssm11(conn);

	CString sqlstr;

	CDbCommand cmd_tpssmt1_inq(conn);
	CDbCommand cmd_tpssmt1_upt(conn);
	CDbCommand cmd_tpssm13_inq(conn);
	CDbCommand cmd_tpssm13_ins(conn);

	CTracer log(__FUNCTION__);

	try
	{

		//清除TSSMT1表中在TPSSM13表中已经删除的出钢计划字段begin
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	            // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	            // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "SELECT distinct A.FACTORY_DIV,A.PLAN_NO  FROM TPSSMT1 A , TPSSM11 B"
				" WHERE A.SM_PLAN_NO not in(select B.SM_PLAN_NO from TPSSM11 B ) "
				" and A.FACTORY_DIV = B.FACTORY_DIV  "
				" AND A.PLAN_STATUS <'12' ";
			break;
		}
		cmd_tpssmt1_inq.SetCommandText(sqlstr);
		cmd_tpssmt1_inq.ExecuteReader();

		while(cmd_tpssmt1_inq.Read())
		{
			tpssmt1.Reset();
			tpssmt1.FACTORY_DIV =	cmd_tpssmt1_inq.GetString(1);
			tpssmt1.PLAN_NO =	cmd_tpssmt1_inq.GetString(2);
			tpssmt1.PLAN_EDIT_FLAG = "2";
			tpssmt1.REC_REVISOR = CString(s.userid);
			tpssmt1.REC_REVISE_TIME = dateNow;

			Log::Trace("", __FUNCTION__,  "factory_div=[{0}]", (const char*)tpssmt1.FACTORY_DIV);
			Log::Trace("", __FUNCTION__,  "SM_PLAN_NO=[{0}]"       , (const char*)tpssmt1.SM_PLAN_NO);
			Log::Trace("", __FUNCTION__,  "PLAN_NO=[{0}]"            , (const char*)tpssmt1.PLAN_NO);

			sqlstr = "tpssmt1.Update(SM_PLAN_NO,ST_NO,DES_DEV_CODE,TPD_NO,FACTORY_DIV, PLAN_NO);";
			tpssmt1.Update("SM_PLAN_NO,ST_NO,DES_DEV_CODE,TPD_NO", "FACTORY_DIV, PLAN_NO");

			//清除TSSMT1表中在TPSSM13表中已经删除的出钢计划字段end
		}
		cmd_tpssmt1_inq.Close();

		//将为新下达的出钢计划编制铁水预计划begin

		// switch(conn->DatabaseKind)
		// {
		// case DB_KIND_DB2:	            // DB2 数据库（未开Oracle兼容）
		// case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		// case DB_KIND_MSSQL:	            // MS SQL Server数据库
		// case DB_KIND_ORACLE:	        // Oracle 数据库
		// default: // 所有数据库适用，通用SQL语句
		// sqlstr = "SELECT distinct A.FACTORY_DIV ,A.SM_PLAN_NO,A.ST_NO FROM TPSSM13 A , TPSSMT1 B"
		// " WHERE A.SM_PLAN_NO not in(select B.SM_PLAN_NO from TPSSMT1 B ) "
		// " and A.FACTORY_DIV = B.FACTORY_DIV  "
		// " AND A.RUN_STATUS <'12' ";
		// break;
		// }
		// cmd_tpssm13_inq.SetCommandText(sqlstr);
		// cmd_tpssm13_inq.ExecuteReader();
		// while(cmd_tpssm13_inq.Read())
		// {
		////cmd_tpssm13_inq.Fetch(tpssm13);
		// tpssm13.FACTORY_DIV =	cmd_tpssm13_inq.GetString(1);
		// tpssm13.SM_PLAN_NO =	cmd_tpssm13_inq.GetString(2);
		// tpssm13.ST_NO =	cmd_tpssm13_inq.GetString(3);

		// tpssmt1.Reset();
		// switch(conn->DatabaseKind)
		// {
		// case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		// case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		// case DB_KIND_MSSQL:	        // MS SQL Server数据库
		// case DB_KIND_ORACLE:	       // Oracle 数据库
		// default: // 所有数据库适用，通用SQL语句

		// sqlstr = " SELECT TO_CHAR(MAX(TO_NUMBER(PLAN_NO) +1))  FROM TPSSMT1 "
		// " WHERE FACTORY_DIV = @factory_div  ";
		// break;
		// }
		// cmd_tpssm13_ins.SetCommandText(sqlstr);
		// cmd_tpssm13_ins.Parameters.Set("factory_div", tpssm13.FACTORY_DIV);
		// cmd_tpssm13_ins.ExecuteReader();

		// if(cmd_tpssm13_ins.Read())
		// {
		// tpssmt1.PLAN_NO = cmd_tpssm13_ins.GetString(1);
		// Log::Trace("", __FUNCTION__,  "PLAN_NO=[{0}]",(const char*)tpssmt1.PLAN_NO);
		// }
		// else
		// {
		// tpssmt1.PLAN_NO = "1";
		// }
		// cmd_tpssm13_ins.Close();
		// tpssmt1.FACTORY_DIV = tpssm13.FACTORY_DIV ;
		// tpssmt1.REC_CREATOR = s.userid;
		// tpssmt1.REC_CREATE_TIME = dateNow;
		// tpssmt1.ST_NO = tpssm13.ST_NO;
		// tpssmt1.SM_PLAN_NO = tpssm13.SM_PLAN_NO;
		// tpssmt1.PLAN_STATUS = "00";
		// tpssmt1.TPD_DEV_CODE = "T";
		////执行新增,失败抛出异常
		// sqlstr = "tpssmt1.Insert()";
		// tpssmt1.Insert();      //封装的新增方法
		// } 
		// cmd_tpssm13_inq.Close();

		//将为新下达的出钢计划编制铁水预计划end


		/* ***** 正常结束处理 ***** */
		strcpy(s.msg, _RES("GCRSS0000002"));/*处理成功。*/
		strcpy(s.sysmsg, "处理成功。");//sysmsg
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

