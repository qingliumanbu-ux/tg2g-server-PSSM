/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:向萍
Date:2014-7-16
Version:1.0
Description: 炼钢计划运行监控查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




/* -EP_CODE_VERSION 1
-EP_SYSTEM_HEAD_BEGIN
-此节代码请勿更改 */
/*<remark>=========================================================
/// <summary>
/// 炼钢计划运行监控查询
/// <para>查询炼钢计划运行监控。                             </para>
/// <para>数据库表：tpssm33(炼钢计划运行监控表)              </para>
/// <para>主调用函数：前台PSSM33画面F2(查询)调用。           </para>
/// </summary>
/// <param name="main_backlog_code">炼钢主工序代码    </param>
/// <returns>计划运行信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm33f2_inq)
/* -EP_SYSTEM_HEAD_END */
int f_pssm33f2_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	/****** 自定义变量 ***** */
	int doFlag = 0;
	CString sqlstr = "";
	CString v_factory_div = "";	//炼钢单元号
	int fetchRowCount;
	int dummy = 0;
	CString cast_div_no;
	CString cast_pono_sum;
	CString refine_route_code = "";            /* 精炼路径 */


	CDbCommand cmd_sql(conn); //与DB 建立连接。
	CDbCommand cmd_inq(conn); //与DB 建立连接。
  
	CModel tpssm33("TPSSM33");
	CModel tpssm11("TPSSM11");

	try
	{
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}]", v_factory_div);
	
		/* 逻辑处理 */
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

				sqlstr = " SELECT * FROM  TPSSM33  t  WHERE  FACTORY_DIV = @v_factory_div   ORDER BY AREA_ID asc,STATION_NO ASC,VIEW_POS ASC";
				
				////Log::Info("", __FUNCTION__, "sqlstr      =[{0}]", sqlstr);

     		break;
		}
	
		cmd_sql.SetCommandText(sqlstr);
		cmd_sql.Parameters.Set("v_factory_div", v_factory_div);
		cmd_sql.ExecuteReader();
	

		if(!bcls_ret->Tables[0].Columns.Contains("CAST_NO_SHOW"))
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING,"CAST_NO_SHOW"); 
		}

		if(!bcls_ret->Tables[0].Columns.Contains("REFINE_ROUTE_CODE"))
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING,"REFINE_ROUTE_CODE"); 
		}
		
				//相关信息初始化。
		//===============
		fetchRowCount = 0;
		while(cmd_sql.Read()) //只读取单记录，可用IF 语句。
		{		
			
			cmd_sql.Fetch(tpssm33);		 //整个表结构的获取。
					
			////Log::Info("", __FUNCTION__, " fetchRowCount =[{0}]",  fetchRowCount);

			////Log::Info("", __FUNCTION__, " tpssm33.PONO  =[{0}]",  tpssm33["PONO"].ToString());

			//bcls_ret->Tables[0].Rows.Add();


			tpssm33.MergeTo(bcls_ret->Tables[0], false);

			if(tpssm33["CAST_NO"].ToString().Trim() != "" )
			{
				cast_pono_sum = CString::Format("%d", tpssm33["CAST_PONO_SUM"].ToDecimal().ToInt32());

				cast_div_no = CString::Format("%d", tpssm33["CAST_DIV_NO"].ToDecimal().ToInt32());

				bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_NO_SHOW"] = tpssm33["CAST_NO"].ToString() + "-"  + cast_div_no;//+ cast_pono_sum + "-"

			}

			if(tpssm33["START_TIME"].ToString().Trim() != "" )
			{
				 bcls_ret->Tables[0].Rows[fetchRowCount]["START_TIME"] = tpssm33["START_TIME"].ToString().Substring(8, 6);
			}
			if(tpssm33["TAP_END_TIME"].ToString().Trim() != "" )
			{
				 bcls_ret->Tables[0].Rows[fetchRowCount]["TAP_END_TIME"] = tpssm33["TAP_END_TIME"].ToString().Substring(8, 6);
			}
			if(tpssm33["POUR_START_TIME"].ToString().Trim() != "" )
			{
				 bcls_ret->Tables[0].Rows[fetchRowCount]["POUR_START_TIME"] = tpssm33["POUR_START_TIME"].ToString().Substring(8, 6);
			}

			//查询精炼路径
			if (tpssm33["PONO"].ToString().Trim() != "" )
			{
				dummy = 0;
				
				tpssm11["PONO"] = tpssm33["PONO"];
				dummy = tpssm11.QueryCount("PONO");
				if (dummy <= 0)
				{					
					switch(conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " SELECT REFINE_ROUTE_CODE FROM TPSSM11  \
								    WHERE PONO = @tpssm33.PONO ";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO",tpssm33["PONO"].ToString());
					cmd_inq.ExecuteReader();
					if(cmd_inq.Read())
					{
						refine_route_code = cmd_inq.GetString(1);
					}
					cmd_inq.Close();

				}
				else
				{
					//取跟踪表的精炼路径;
					
					switch(conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " SELECT REFINE_ROUTE_CODE FROM TPSSM11 \
								    WHERE PONO = @tpssm33.PONO ";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("tpssm33.PONO",tpssm33["PONO"].ToString());
					cmd_inq.ExecuteReader();
					if(cmd_inq.Read())
					{
						refine_route_code = cmd_inq.GetString(1);
					}
					cmd_inq.Close();
				}

				 bcls_ret->Tables[0].Rows[fetchRowCount]["REFINE_ROUTE_CODE"] = refine_route_code;
			}

		
			fetchRowCount++;
			

		}
		cmd_sql.Close(); //关闭游标


	 }
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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

