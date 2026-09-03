/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-04-26
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/

//#include "tpssmsg.h"

/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM10(炼钢制造命令表)              </para>
/// <para>主调用函数：前台PSSM101CC画面F2(查询)按钮      </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                      </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssmzlf3_pro)

int f_pssmzlf3_pro(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString cs_pono = "";
	CString sqlstr = "";
	CString sqlstr1 = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int		TotalRecordCount = 0;

	CModel tpssm03("TPSSM03");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	try
	{
		
		cs_pono=bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();
		sqlstr = " SELECT *  FROM  TPSSM03   WHERE   PONO = @pono ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("pono", cs_pono);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssm03.Reset();
			cmd_inq.Fetch(tpssm03);
			if (tpssm03["STRAND_NO"].ToString().Trim()=="2")
			{
				tpssm03["STRAND_NO"] = "1";
				tpssm03.Update("STRAND_NO", "SLAB_NO");
			}
			else if (tpssm03["STRAND_NO"].ToString().Trim() == "1")
			{
				tpssm03["STRAND_NO"] = "2";
				tpssm03.Update("STRAND_NO", "SLAB_NO");
			}
		}
		cmd_inq.Close();
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
