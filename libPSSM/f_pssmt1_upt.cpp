/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:JHZHAO
Date:2011-12-17
Version:1.0
Description: 删除出钢计划调用函数
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
#include "tpssmt1.h"
BM2_FUNCTION_EXPORT
 int f_pssmt1_upt(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq;
	CDecimal v_totalCount_1 = 0;
	CString  v_factory_div;
	CString  v_sm_plan_no;
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CTPSSMT1 tpssmt1(conn);
	CDbCommand cmd_inq_c(conn);
	CDbCommand cmd_inq(conn);
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
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"];
			v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			if(v_sm_plan_no.Trim().GetLength() == 0)
			{
				strcpy(s.msg,"炼钢计划号为空");
				strcpy(s.sysmsg ,"炼钢计划号为空");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(v_factory_div.Trim().GetLength() == 0)
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
			case DB_KIND_ORACLE:	       // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句

				sqlstr = " SELECT COUNT(*) FROM TPSSMT1 "
					" WHERE SM_PLAN_NO = @sm_plan_no  "
					" AND FACTORY_DIV = @factory_div  ";
				break;
			}
			cmd_inq_c.SetCommandText(sqlstr);
			cmd_inq_c.Parameters.Set("factory_div", v_factory_div);
			cmd_inq_c.Parameters.Set("sm_plan_no", v_sm_plan_no);
			v_totalCount_1 = cmd_inq_c.ExecuteScalar();
			Log::Trace("", __FUNCTION__, "v_totalCount_1 = [{0}]",v_totalCount_1.ToInt32());

			if( v_totalCount_1.ToInt32()>0)
			{
				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	       // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句

					sqlstr = " SELECT PLAN_NO,DES_DEV_CODE FROM TPSSMT1 "
						" WHERE SM_PLAN_NO = @sm_plan_no  "
						" AND FACTORY_DIV = @factory_div  ";
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("factory_div", v_factory_div);
				cmd_inq.Parameters.Set("sm_plan_no", v_sm_plan_no);
				cmd_inq.ExecuteReader();
				if(cmd_inq.Read())
				{
					tpssmt1.PLAN_NO = cmd_inq.GetString(1);
					tpssmt1.DES_DEV_CODE = cmd_inq.GetString(2);
					Log::Trace("", __FUNCTION__, "PLAN_NO = [{0}]", (const char*)tpssmt1.PLAN_NO );
					Log::Trace("", __FUNCTION__, "DES_DEV_CODE = [{0}]", (const char*)tpssmt1.DES_DEV_CODE );
				}
				if(tpssmt1.DES_DEV_CODE.Trim()!="")
				{
					tpssmt1.PLAN_EDIT_FLAG ="2";
				}
				else
				{
					tpssmt1.PLAN_EDIT_FLAG ="1";
				}

				//tpssmt1.PLAN_EDIT_FLAG = "1";
				tpssmt1.PLAN_STATUS = "00";
				tpssmt1.SM_PLAN_NO = " ";
				tpssmt1.PONO = " ";
				tpssmt1.REC_CREATOR = s.userid;
				tpssmt1.REC_CREATE_TIME = dateNow;
				tpssmt1.FACTORY_DIV = v_factory_div ;
			}
			cmd_inq.Close();
			tpssmt1.Update("PLAN_STATUS, SM_PLAN_NO,PONO, PLAN_EDIT_FLAG, REC_CREATOR, REC_CREATE_TIME ", "FACTORY_DIV, PLAN_NO ");

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
