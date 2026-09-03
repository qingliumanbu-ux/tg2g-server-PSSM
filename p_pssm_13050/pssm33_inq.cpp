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
int f_pssm33f2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/****** 自定义变量 ***** */
	int doFlag = 0;
	CString sqlstr = "";
	CString cs_dev_code = "";
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
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BUNKER");	//厂别

		//依次信息为：0：计划号+分割号、1：炉号，2：出钢记号，3：计划开始时间，4：计划结束时间，5：开始，6：结束时间、7：温度、8;状态、9：0
		cs_dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"].ToString();
		tpssm33["STATION_ID"] = cs_dev_code.SubstringNE(0,1);
		tpssm33["STATION_NO"] = cs_dev_code.SubstringNE(1, 1);
		tpssm33["FACTORY_DIV"] = "A10";

		cmd_sql.Close(); //关闭游标


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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

