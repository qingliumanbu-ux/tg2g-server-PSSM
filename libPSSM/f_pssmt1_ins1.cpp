/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:JHZHAO
Date:2011-12-17
Version:1.0
Description: 新增铁水预计划函数
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
#include "tpssmt1.h"


BM2_FUNCTION_EXPORT
 int f_pssmt1_ins1(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq;
	CString  v_tpd_no;
	CString  v_tpd_start_time;
	CString  v_tpd_end_time;
	CString  v_tpd_dev_code;
	CString  v_iron_ladle_no;
	CDecimal v_totalCount_1 = 0;
	CString  v_factory_div;
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
			v_tpd_no = bcls_rec->Tables[blkseq].Rows[i]["TPD_NO"];
			v_tpd_start_time = bcls_rec->Tables[blkseq].Rows[i]["TPD_START_TIME"];
			v_tpd_end_time = v_tpd_start_time = bcls_rec->Tables[blkseq].Rows[i]["TPD_END_TIME"];
			v_iron_ladle_no = bcls_rec->Tables[blkseq].Rows[i]["IRON_LADLE_NO"];
			v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			if(v_tpd_no.Trim().GetLength() == 0)
			{
			   strcpy(s.msg,"倒灌处理号为空");
			   strcpy(s.sysmsg ,"倒灌处理号为空");
			   throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(v_iron_ladle_no.Trim().GetLength() == 0)
			{
			   strcpy(s.msg,"铁水包号为空");
			   strcpy(s.sysmsg ,"铁水包号为空");
			   throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(v_factory_div.Trim().GetLength() == 0)
			{
			   strcpy(s.msg,"主工序代码为空");
			   strcpy(s.sysmsg ,"主工序代码为空");
			   throw CApplicationException(-1, s.msg, s.svc_name);
			}
			//HYF 20130427 SubstringNE
			v_tpd_dev_code = v_tpd_no.Trim().SubstringNE(0,1);
			
			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	       // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
				
				sqlstr = " SELECT COUNT(*)  FROM TPSSMT1 "
					     " WHERE FACTORY_DIV = @factory_div  ";
				break;
			}
			cmd_inq_c.SetCommandText(sqlstr);
			cmd_inq_c.Parameters.Set("factory_div", v_factory_div);
			v_totalCount_1 = cmd_inq_c.ExecuteScalar();
			Log::Trace("", __FUNCTION__, "v_totalCount_1 = [%d]",v_totalCount_1.ToInt32());
			
			if( v_totalCount_1.ToInt32()<=0)
			{
			    tpssmt1.PLAN_NO = v_tpd_no; //倒罐号做为计划号，原"1"， xuwen修改
			}
			else
			{
				////判断该出钢计划是否已经分配过铁水预计划end
				//switch(conn->DatabaseKind)
				//{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//	case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	        // Oracle 数据库
				//	default: // 所有数据库适用，通用SQL语句
				//	sqlstr = " SELECT TO_CHAR(MAX(TO_NUMBER(PLAN_NO) +1))  FROM TPSSMT1 "
				//		 " WHERE FACTORY_DIV = @factory_div  ";
				//	break;
				//}
				////CDbCommand cmd_inq(sqlstr, conn);
				//cmd_inq.SetCommandText(sqlstr);
				//cmd_inq.Parameters.Set("factory_div", v_factory_div);
				//cmd_inq.ExecuteReader();
				//if ( cmd_inq.Read() )
				//{
				//	tpssmt1.PLAN_NO = cmd_inq.GetString(1);
				//}
				//cmd_inq.Close();
				tpssmt1.PLAN_NO = v_tpd_no;
			}	

			// 获取前台传入参数
			//tpssmt1.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tpssmt1.FACTORY_DIV = v_factory_div ;
			tpssmt1.REC_CREATOR = s.userid;
			tpssmt1.REC_CREATE_TIME = dateNow;

			tpssmt1.PLAN_STATUS = "00";
			tpssmt1.TPD_DEV_CODE = v_tpd_dev_code;
			tpssmt1.PLAN_EDIT_FLAG = "1";
			tpssmt1.TPD_NO = v_tpd_no;
			tpssmt1.IRON_LADLE_NO = v_iron_ladle_no;
			tpssmt1.TPD_START_TIME = v_tpd_start_time;
			tpssmt1.TPD_END_TIME = v_tpd_end_time;

			// 执行新增,失败抛出异常
			sqlstr = "tpssmt1.Insert()";
			tpssmt1.Insert();      //封装的新增方法
	
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

