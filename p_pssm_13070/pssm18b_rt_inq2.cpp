/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2012-01-16
Version:1.0
Description: 返送画面熔炼号和返送位置查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/***** C++ 的业务头文件部分 *****/
//外部函数声明
int f_edsetcustominfo(EIClass * bcls_rec, EIClass * bcls_ret);

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 返送画面熔炼号和返送位置查询
/// <para>查询熔炼号和返送位置。                            </para>
/// <para>数据库表：tpssm13/D1                    </para>
/// <para>主调用函数：PSSM18R画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>熔炼号和返送位置</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18b_rt_inq2)
//-EP_SYSTEM_HEAD_END
int f_pssm18b_rt_inq2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;

	CString v_function_id = "";
	CString v_heat_no = "";

	CModel tpssm35("TPSSM35");
	CString sqlstr = "";
	CDbCommand cmd_tpssm35_inq(conn);

	try
	{
		//获得输入参数
		v_heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString();
		////Log::Trace("", __FUNCTION__, "===v_heat_no =  [{0}]", v_heat_no);

		v_function_id = "PSSM18R_INQ";

		//前台传了FUNCTION_ID，后台直接调用就可以
		if (bcls_rec->Tables[0].Columns.IndexOf("function_id") < 0)
		{
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "FUNCTION_ID");
			bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"] = v_function_id;
		}
		else
		{
			v_function_id = bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"];
		}
		////Log::Trace("", __FUNCTION__, "===v_function_id =  [{0}]", v_function_id);
		if (bcls_ret->Tables.get_Count() < 1)
		{
			//在bcls_ret 中增加一个块，放查询结果
			bcls_ret->Tables.Add();
		}
		f_edsetcustominfo(bcls_rec, bcls_ret);

		//---------------查询熔炼号---------------------------
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT A.* FROM TPSSM35 A, TPSSM11 B  \
						WHERE A.FACTORY_DIV = B.FACTORY_DIV \
							AND A.PONO = B.PONO \
							AND B.HEAT_NO = @HEAT_NO ";
			break;
		}
		cmd_tpssm35_inq.SetCommandText(sqlstr);
		cmd_tpssm35_inq.Parameters.Set("HEAT_NO", v_heat_no);
		cmd_tpssm35_inq.ExecuteReader();
		while (cmd_tpssm35_inq.Read())
		{
			cmd_tpssm35_inq.Fetch(tpssm35);

			CDataRow & row = bcls_ret->Tables[0].Rows.Add();
			row.Merge(tpssm35);
		}
		cmd_tpssm35_inq.Close();
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
