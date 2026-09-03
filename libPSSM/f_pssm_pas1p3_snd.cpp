/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   戎翔
Version:    1.0
Date:     2014-06-09
Description:	 钢种变更下发L2电文。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

/*<remark>=========================================================
/// <summary>
/// 钢种变更下发L2电文
/// <para>1.读取传入的计划号、制造命令号、熔炼号</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：由钢种变更调用。                             </para>
/// </summary>
/// <param name="sm_plan_no">计划号          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="heat_no">熔炼号          </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_pas1p3_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	CString lpsz_tc_no = " ";

	CString sqlstr="";

	CString factory_div = " ";//分区标志

	CString sm_plan_no="";
	CString heat_no="";
	CString pono="";
	CString st_no="";

	CString sm_plan_no_new = "";
	CString heat_no_new = "";
	CString pono_new="";
	CString st_no_new="";

	CDbCommand cmd_inq(conn);

	EPEX epex(&s,conn);

	try
	{

 		//获得输入参数
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();
		pono_new = bcls_rec->Tables[0].Rows[0]["PONO_NEW"].ToString().Trim();

		Log::Trace("", __FUNCTION__, "传入factory_div=[{0}]", factory_div);
		if (factory_div.Trim() == "1")
		{
			lpsz_tc_no = "PAS1P3";/*一区赋电文号*/
		}
		if (factory_div.Trim() == "2")
		{
			lpsz_tc_no = "PAS2P3";/*二区赋电文号*/
		}
		if (factory_div.Trim() == "3")
		{
			lpsz_tc_no = "PAS3P3";/*三区赋电文号*/
		}


		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = CString(" SELECT SM_PLAN_NO,HEAT_NO,ST_NO FROM TPSSM11 \
							    WHERE FACTORY_DIV = @FACTORY_DIV \
							      AND PONO              = @PONO ");
			break;
		}		
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_inq.Parameters.Set("PONO",pono);
		cmd_inq.ExecuteReader();
		if(cmd_inq.Read())
		{
			sm_plan_no = cmd_inq.GetString(1).Trim();
			heat_no = cmd_inq.GetString(2).Trim();
			st_no = cmd_inq.GetString(3).Trim();
		}
		cmd_inq.Close();

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = CString(" SELECT ST_NO,SM_PLAN_NO,HEAT_NO FROM TPSSM10 \
							    WHERE FACTORY_DIV = @FACTORY_DIV \
							      AND PONO              = @PONO_NEW ");
			break;
		}		
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_inq.Parameters.Set("PONO_NEW", pono_new);
		cmd_inq.ExecuteReader();
		if(cmd_inq.Read())
		{
			st_no_new = cmd_inq.GetString(1).Trim();
			sm_plan_no_new = cmd_inq.GetString(2).Trim();
			heat_no_new = cmd_inq.GetString(3).Trim();
		}
		cmd_inq.Close();


		/* ***** 打印输入参数 ***** */
		EDLog(1,1,"SM_PLAN_NO=[%s]",(const char *)sm_plan_no);
		EDLog(1,1,"HEAT_NO=[%s]",(const char *)heat_no);
		EDLog(1,1,"pono_new=[%s]",(const char *)pono_new);	
		EDLog(1,1,"PONO_IN=[%s]",(const char *)pono);	

		/* *****	发送电文开始 ***** */
		//lpsz_tc_no = "HMJL03";/*赋电文号*/

		/*初始化*/
		if(epex.Initialize(lpsz_tc_no) < 0)
		{
			strcpy(s.msg,_RES("PS00S0000686")/*电文初始化失败。*/);
			sprintf(s.sysmsg, "电文初始化出错！");
			throw CApplicationException(-1, s.msg, log.Location); 
		}

		if(epex.SetValue("sm_plan_no", 0,sm_plan_no) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg,"初始化sm_plan_no[%s]出错.",(const char*)sm_plan_no);
			throw CApplicationException(-1, s.msg, log.Location);				
		}
		if(epex.SetValue("pono", 0,pono) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化pono_out[%s]出错.", (const char*)pono);
			throw CApplicationException(-1, s.msg, log.Location);				
		}
		if(epex.SetValue("st_no", 0,st_no) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化st_no_out[%s]出错.", (const char*)st_no);
			throw CApplicationException(-1, s.msg, log.Location);				
		}
		if (epex.SetValue("heat_no", 0, heat_no) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化st_no_out[%s]出错.", (const char*)heat_no);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (epex.SetValue("sm_plan_no_new", 0, sm_plan_no_new) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化st_no_in[%s]出错.", (const char*)sm_plan_no_new);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (epex.SetValue("st_no_new", 0, st_no_new) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化st_no_in[%s]出错.", (const char*)st_no_new);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (epex.SetValue("pono_new", 0, pono_new) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化pono_in[%s]出错.", (const char*)pono_new);
			throw CApplicationException(-1, s.msg, log.Location);				
		}
		if (epex.SetValue("st_no_new", 0, st_no_new) < 0)
		{
			sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "初始化st_no_in[%s]出错.", (const char*)st_no_new);
			throw CApplicationException(-1, s.msg, log.Location);				
		}

		/*发送电文*/
		if(epex.SendTele() < 0)
		{
			EDLog(1, 1, "钢种变更下发精炼L2电文 = [%s]",(const char*) epex.GetMsg());
			sprintf(s.msg, _RES("GCRSS0000032")/*电文发送失败。*/);
			throw CApplicationException(-1, s.msg, log.Location); 
		}

		epex.Uninitialize();   

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
