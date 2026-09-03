/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-07-18
功能: 计划流向信息查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark>=========================================================
/// <summary>
/// 计划流量信息查询
/// <para>
/// 1.根据传入的条件获取计划中的制造命令后续流向信息。
/// 2.查询条件：在线/离线标记、生产日期、炼钢区分、铸机号；
/// 3.排序方式：制造命令号升序；
/// </para>
/// <para>数据库表：TPSSM01(炼钢连铸制造命令表)</para>
/// <para>主调用函数：前台PSSM09画面F2(查询)调用。   </para>
/// </summary>
/// <param name="CHECK_FLAG">在线/离线标记  </param>
/// <param name="START_DATE">起始日期  </param>
/// <param name="END_DATE">结束日期  </param>
/// <param name="FACTORY_DIV">炼钢产线区分  </param>
/// <param name="CC_MACH_NO">铸机号  </param>
/// <returns>计划流向信息</returns>
/// <returns>成功：0</returns>
/// <returns>失败：-1</returns>
===========================================================</remark>*/
/******service入口******/
BM2F_ENTERACE(pssm09_inq);

int f_pssm09_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */	
	int doFlag = 0;		//返回值
	int logFlag = 1;
	int i=0;
	int code_row=0;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString query_condition="";


	/*定义业务用变量*/
	CString check_flag = "";
	CString start_date = "";
	CString end_date = "";
	CString status_to = "";
	CString status_from = "";
	CString code_table_name = "";
	CString cur_band_ord_sort = "";
	CString pre_band_ord_sort = "0";
	CDateTime date_start_date;
	CDateTime date_end_date;

	//CString backlog = "";
	CString pre_backlog = "";
	CString pre_backlog_wk = "";
	CString cur_backlog = "";
	CString cur_backlog_wk = "";

	CString pono_union = "";

	CTimeSpan m_timespan;

	/*实体类定义*/
	CModel tpssm01("TPSSM01");

	/****** 业务处理开始 ******/

	try 
	{
		/*获取前台输入数据*/
		check_flag = bcls_rec->Tables[0].Rows[0]["CHECK_FLAG"].ToString();
		start_date = bcls_rec->Tables[0].Rows[0]["START_DATE"].ToString();
		end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString();
		status_from = bcls_rec->Tables[0].Rows[0]["STATUS_FROM"].ToString();
		status_to = bcls_rec->Tables[0].Rows[0]["STATUS_TO"].ToString();
		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm01["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();

		if (bcls_rec->Tables[0].Columns.Contains("PONO_UNION"))
		{
			pono_union = bcls_rec->Tables[0].Rows[0]["PONO_UNION"].ToString();
		}
		////Log::Debug("", __FUNCTION__ ,"IN:CHECK_FLAG = [{0}]",check_flag);
		////Log::Debug("", __FUNCTION__ ,"IN:START_DATE = [{0}]",start_date);
		////Log::Debug("", __FUNCTION__ ,"IN:END_DATE = [{0}]",end_date);
		////Log::Debug("", __FUNCTION__ ,"IN:FACTORY_DIV = [{0}]",tpssm01["FACTORY_DIV"].ToString());
		////Log::Debug("", __FUNCTION__ ,"IN:CC_MACH_NO = [{0}]",tpssm01["CC_MACH_NO"].ToString());
		////Log::Debug("", __FUNCTION__ ,"IN:pono_union = [{0}]",pono_union);

		date_start_date=CDateTime::Parse(start_date);
		date_end_date = CDateTime::Parse(end_date);
		CDateTime date_end_date_cpy = CDateTime::Parse(end_date);



		if (start_date > end_date )
		{
			sprintf(s.msg,_RES("PSSMS0000188")/*起始日期晚于结束日期。*/);
			throw CApplicationException(-1,s.msg,log.Location);
		}
		//m_timespan = date_end_date.Subtract(date_start_date);
		m_timespan = date_end_date_cpy.Subtract(date_start_date);
		////Log::Trace("", __FUNCTION__, "m_timespan = [{0}]", m_timespan.Days());
		if (m_timespan.Days()>100)
		{
			CFormattable arguments[] = { date_start_date,date_end_date };
			CMessageFormat::Format(s.msg,  "起始日期[{0}]与结束日期[{1}]之间大于100天，请减小时间范围", arguments, 2);
			throw CApplicationException(-1,s.msg,log.Location);
		}

		/*建立查询数据SQL语句*/

		//构建查询第一层 查询得出TPSSM01表中符合条件的制造命令号
		if (check_flag=="1")//当前档案
		{
			sqlstr = " SELECT A.FACTORY_DIV, A.PLAN_DATE, B.SLAB_DEST,SUM(B.SLAB_WT) AS SLAB_WT FROM TPSSM01 A,TPSSM03 B "
				" WHERE A.FACTORY_DIV = B.FACTORY_DIV "
				" AND A.PONO = B.PONO ";
		}
		else
		{
			sqlstr = " SELECT A.FACTORY_DIV, A.PLAN_DATE, B.SLAB_DEST,SUM(B.SLAB_WT) AS SLAB_WT FROM HPSSM01 A,HPSSM03 B "
				" WHERE A.FACTORY_DIV = B.FACTORY_DIV "
				" AND A.PONO = B.PONO ";
		}

		/*拼接条件语句*/
		if(start_date.Trim()!= "")
		{
			sqlstr  += " AND  A.PLAN_DATE >= @start_date ";  
		}
		if(start_date.Trim()!= "")
		{
			sqlstr  += " AND  A.PLAN_DATE <= @end_date ";  
		}
		if(tpssm01["FACTORY_DIV"].ToString().Trim()!= "")
		{
			sqlstr  += " AND  A.FACTORY_DIV = @factory_div ";  
		}
		if(tpssm01["CC_MACH_NO"].ToString().Trim()!= "")
		{
			sqlstr  += " AND  A.CC_MACH_NO = @cc_mach_no ";  
		}
		if(status_from.Trim()!= "")
		{
			sqlstr  += " AND  A.PONO_STATUS >= @status_from ";  
		}
		if(status_to.Trim()!= "")
		{
			sqlstr  += " AND  A.PONO_STATUS <= @status_to ";  
		}
		if (pono_union.Trim()!="")
		{
			sqlstr += " AND A.PONO IN ("+ pono_union +  ") ";
		}

		sqlstr += " GROUP BY A.FACTORY_DIV, A.PLAN_DATE, B.SLAB_DEST ";

		/*给查询SQL赋条件值*/
		CDbCommand cmd_sql(sqlstr,conn);
		cmd_sql.Parameters.Set("start_date", start_date);
		cmd_sql.Parameters.Set("end_date", end_date);
		cmd_sql.Parameters.Set("factory_div", tpssm01["FACTORY_DIV"].ToString());
		cmd_sql.Parameters.Set("cc_mach_no",tpssm01["CC_MACH_NO"].ToString());
		cmd_sql.Parameters.Set("status_from", status_from);
		cmd_sql.Parameters.Set("status_to", status_to);

		//分页获取
		cmd_sql.SetCommandText(sqlstr);
		cmd_sql.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_sql.Close();


		/*返回处理信息*/
		if(bcls_ret->Tables[0].Rows.get_Count() == 0)
		{
			strcpy(s.msg,_RES("GCRSS0000013")/*没有满足条件的记录。*/);
		}
		else
		{
			CFormattable arguments[] = {bcls_ret->Tables[0].Rows.get_Count()}; 
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004")/*查询到[{0}]条记录。*/,	arguments, 1); 			
		}
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

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}


