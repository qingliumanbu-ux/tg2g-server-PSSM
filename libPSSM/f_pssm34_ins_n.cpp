/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-12-26
Description:	 写炼钢事项信号履历表。
Update: 2015-04-07 lijie 更新数据表
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 写炼钢事项信号履历表
/// <para>处理内容：写炼钢事项信号履历表  </para>
/// <para>数据库表：TPSSM34(炼钢事项信号履历表) </para>
/// <para>主调用函数：被f_pssm_run_proc()调用。           </para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="pono">制造命令          </param>
/// <param name="heat_no">熔炼号          </param>
/// <param name="run_signal">运转信号         </param>
/// <param name="proc_no">处理号     </param>
/// <param name="proc_time">处理时刻     </param>
/// <param name="dev_code">设备代码     </param>
/// <param name="simul_flag">模拟标志    </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm34_ins_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	
	CModel tpssm34("TPSSM34");
	CModel tpssm11("TPSSM11");
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	
	try
	{
		create_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		/* 对输入信息循环处理 */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{
			/* 取得单行传入信息 */
			tpssm34.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tpssm34["REC_CREATOR"] = s.userid;
			tpssm34["REC_CREATE_TIME"] = create_time;
			tpssm34["REC_REVISOR"] = s.userid;
			tpssm34["START_TIME"] = bcls_rec->Tables[0].Rows[i]["PROC_TIME"].ToString();//存对应处理时刻

			////Log::Trace("", __FUNCTION__,  "f_pssm34_ins_n>dev_code=[{0}]",(const char*)tpssm34["DEV_CODE"].ToString());
			////Log::Trace("", __FUNCTION__, "tpssm34["PONO"] =[{0}] FACTORY_DIV =[{1}]", tpssm34["PONO"].ToString(), tpssm34["FACTORY_DIV"].ToString());

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT MAX(RUN_SEQ) FROM TPSSM34 "
						 " WHERE FACTORY_DIV = @tpssm34.FACTORY_DIV "
						 " AND PONO                = @tpssm34.PONO ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tpssm34.FACTORY_DIV", tpssm34["FACTORY_DIV"].ToString());
			cmd_inq.Parameters.Set("tpssm34.PONO"             , tpssm34["PONO"].ToString());
            cmd_inq.ExecuteReader();
			if(cmd_inq.Read())
			{
				tpssm34["RUN_SEQ"] = cmd_inq.GetDecimal(1);
			}
			else
			{
				tpssm34["RUN_SEQ"] = 0;
			}

			
			////Log::Trace("", __FUNCTION__, "tpssm34["RUN_SEQ"] =[{0}]",tpssm34["RUN_SEQ"].ToDecimal());
			tpssm34["RUN_SEQ"] =tpssm34["RUN_SEQ"].ToDecimal() + 1;
			tpssm34["PRACT_PONO"] = tpssm34["PONO"];

			//取HEAT_NO
			tpssm11["FACTORY_DIV"]=tpssm34["FACTORY_DIV"];
			tpssm11["PONO"]=tpssm34["PONO"];
			sqlstr = "tpssm11.Query(PONO)";
			tpssm11.Query("FACTORY_DIV,PONO");

			tpssm34["HEAT_NO"]=tpssm11["HEAT_NO"].ToString().TrimOrBlank();
			tpssm34["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"].ToString().TrimOrBlank();
			tpssm34["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"].ToString().TrimOrBlank();
			tpssm34["ST_NO"]=tpssm11["ST_NO"].ToString().TrimOrBlank();
			sqlstr = "tpssm34.Insert()";
			tpssm34.Insert();

		}


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

