/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author: 
Version:   
Date:    
Description: 炼钢各工序处理号修改
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 炼钢各工序处理号修改
/// <para>数据库表：tpssm25(炼钢各工序处理号)                 </para>
/// <para>主调用函数：前台PSSM11ProcNo 确定。 </para>
/// </summary>
/// <param name="DEV_CODE">设备代码    </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11proc_upd)


int f_pssm11proc_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;

	CString datetime = "";

	// 定义表的实体对象
	CModel tpssm25("TPSSM25");

	CString sqlstr = "";
	CDbCommand cmd_upd(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm25.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			
			////Log::Trace("", __FUNCTION__, "CURR_PROC_NO=[{0}]", tpssm25["CURR_PROC_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}]", tpssm25["FACTORY_DIV"].ToString());
			////Log::Trace("", __FUNCTION__, "STATION_NO=[{0}]", tpssm25["STATION_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "STATION_ID=[{0}]", tpssm25["STATION_ID"].ToString());
			
			//判断是否有原记录
			sqlstr = "tpssm25.QueryCount()";
			int count = tpssm25.QueryCount("FACTORY_DIV, STATION_NO, STATION_ID");

			if (count == 0) //没有原记录
			{
				//插入新记录;
				sqlstr = "tpssm25.Insert()";
				tpssm25.Insert();
			}
			else
			{
				//修改记录
				sqlstr = "tpssm25.Insert()";
				tpssm25.Update(
					"CURR_PROC_NO, REC_REVISOR, REC_REVISE_TIME",
					"FACTORY_DIV, STATION_NO, STATION_ID");
			}

		}


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}

