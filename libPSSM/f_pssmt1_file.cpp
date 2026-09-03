/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-17
Version:1.0
Description: 归档函数
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
#include "tpssmt1.h"
#include "tpssmt2.h"
BM2_FUNCTION_EXPORT
 int f_pssmt1_file(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq;
	CString  datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	int i=0;     
	CDecimal dummy=0;

	CTPSSMT1 tpssmt1(conn);
	CTPSSMT2 tpssmt2(conn);
	CDbCommand cmd_tpssmt1_inq(conn);
	CString sqlstr;


	try
	{

		blkseq = bcls_rec->Tables.IndexOf("TPSSMT1");
		if (blkseq < 0)
		{ 
			strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		int rowCount = bcls_rec->Tables[blkseq].Rows.get_Count();
		tpssmt1.Reset();

		for(int i=0;  i < rowCount;  i++)
		{
			//将对象字段重置为默认值
			tpssmt1.Reset();
			tpssmt1.SM_PLAN_NO = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"];
			tpssmt1.FACTORY_DIV = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			if(tpssmt1.SM_PLAN_NO.Trim().GetLength() == 0)
			{
				strcpy(s.msg,"炼钢计划号为空");
				strcpy(s.sysmsg ,"炼钢计划号为空");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tpssmt1.FACTORY_DIV.Trim().GetLength() == 0)
			{
				strcpy(s.msg,"主工序代码为空");
				strcpy(s.sysmsg ,"主工序代码为空");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = "SELECT * FROM  tpssmt1 \
						 WHERE FACTORY_DIV = trim(@FACTORY_DIV) \
						 AND SM_PLAN_NO = trim(@SM_PLAN_NO)";
				break;
			}

			cmd_tpssmt1_inq.SetCommandText(sqlstr);
			cmd_tpssmt1_inq.Parameters.Set("FACTORY_DIV",tpssmt1.FACTORY_DIV);
			cmd_tpssmt1_inq.Parameters.Set("SM_PLAN_NO",tpssmt1.SM_PLAN_NO);
			cmd_tpssmt1_inq.ExecuteReader();
			while(cmd_tpssmt1_inq.Read())
			{
				cmd_tpssmt1_inq.Fetch(tpssmt1);
				tpssmt1.TrimOrBlank();

				tpssmt2.REC_CREATOR = s.userid;
				tpssmt2.REC_CREATE_TIME = datetime;
				tpssmt2.REC_REVISOR = s.userid;
				tpssmt2.REC_REVISE_TIME = datetime;
				tpssmt2.ARCHIVE_FLAG = tpssmt1.ARCHIVE_FLAG;
				tpssmt2.FACTORY_DIV = tpssmt1.FACTORY_DIV;
				tpssmt2.PLAN_NO = tpssmt1.PLAN_NO;
				tpssmt2.PONO = tpssmt1.PONO;
				tpssmt2.PLAN_EDIT_FLAG = "4";
				tpssmt2.PLAN_STATUS = tpssmt1.PLAN_STATUS;		
				tpssmt2.ST_NO = tpssmt1.ST_NO;
				tpssmt2.TPD_NO = tpssmt1.TPD_NO;
				tpssmt2.IRON_LADLE_NO = tpssmt1.IRON_LADLE_NO;     
				tpssmt2.TPD_DEV_CODE = tpssmt1.TPD_DEV_CODE;		
				tpssmt2.DES_DEV_CODE = tpssmt1.DES_DEV_CODE;		
				tpssmt2.TPD_PLAN_START_TIME = tpssmt1.TPD_PLAN_START_TIME;		
				tpssmt2.TPD_PLAN_END_TIME = tpssmt1.TPD_PLAN_END_TIME;
				tpssmt2.TPD_START_TIME =tpssmt1.TPD_START_TIME;  
				tpssmt2.TPD_END_TIME = tpssmt1.TPD_END_TIME;
				tpssmt2.DES_PLAN_START_TIME = tpssmt1.DES_PLAN_START_TIME;     
				tpssmt2.DES_PLAN_END_TIME = tpssmt1.DES_PLAN_END_TIME;		
				tpssmt2.DES_START_TIME =tpssmt1.DES_START_TIME;  		
				tpssmt2.DES_END_TIME = tpssmt1.DES_END_TIME ;		
				tpssmt2.SM_PLAN_NO = tpssmt1.SM_PLAN_NO;		
				tpssmt2.SLAG_DIV = tpssmt1.SLAG_DIV;		

				//查询 tpssmt2 表中是否有此记录
				dummy = 0;

				dummy = tpssmt2.QueryCount("FACTORY_DIV,SM_PLAN_NO");
				Log::Trace("", __FUNCTION__, "TPSSM31 INS dummy =[{0}]",dummy.ToInt32());
				if (dummy <= 0)
				{
					//插入数据				
					Log::Trace("", __FUNCTION__, "TPSSMt2 INS1 PLAN_NO =[{0}]",(const char*)tpssmt2.PLAN_NO);
					tpssmt2.Insert();
				}
			}
			cmd_tpssmt1_inq.Close();

			sqlstr = "tpssmt1.Delete()";
			tpssmt1.Delete("FACTORY_DIV,SM_PLAN_NO");

		}//for

		//设置系统返回消息，国际化信息
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  	

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
