/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-10-12
Version:1.0
Description: 炉次状态查询
Update:
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 炉次状态查询
/// <para>查询run_status。                            </para>
/// <para>数据库表：tpssm11                    </para>
/// <para>主调用函数：PSSM21O画面调用。                </para>
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别区分     </param>
/// <returns>run_status</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_del_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm18_del_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CString v_dev_code = ""; //DEV_CODE-CHARGE_NO
	CString dev_code = ""; //DEV_CODE
	CDecimal charge_no = 0; //CHARGE编号

	CModel tpssm11("TPSSM11");

	CDbCommand cmd_tpssm11_inq(conn);

	try
	{
		//获得输入参数
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"];
		tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"];
		tpssm11.Query("SM_PLAN_NO");
		v_dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"];

		dev_code = v_dev_code.SubstringNE(0, 2);
		charge_no = atoi(v_dev_code.SubstringNE(v_dev_code.GetLength() - 1, 1));

		Log::Trace("", __FUNCTION__, "dev_code =[{0}]", dev_code);
		Log::Trace("", __FUNCTION__, "charge_no =[{0}]", charge_no);
		Log::Trace("", __FUNCTION__, "FACTORY_DIV =[{0}]", tpssm11["FACTORY_DIV"].ToString());
		Log::Trace("", __FUNCTION__, "PONO =[{0}]", tpssm11["PONO"].ToString());
		Log::Trace("", __FUNCTION__, "SM_PLAN_NO =[{0}]", tpssm11["SM_PLAN_NO"].ToString());
		//设定返回参数表

		//运转状态
		bcls_ret->Tables[0].set_TableName("STATE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "RUN_STATUS");

		//设备代码
		bcls_ret->Tables.Add();
		bcls_ret->Tables[1].set_TableName("DEV_CODE");
		bcls_ret->Tables[1].Columns.Add(DT_STRING, "DEV_CODE");

		//2级计划号
		bcls_ret->Tables.Add();
		bcls_ret->Tables[2].set_TableName("SM_PLAN_NOL2");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "SM_PLAN_NOL2");

		CDataRow & rowp = bcls_ret->Tables[2].Rows.Add();
		rowp["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"].ToString().Trim();
		//运转状态
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT CODE_DESC_1_CONTENT FROM TEP0002 T WHERE CODE_CLASS = 'PSA22N' \
					                           AND CODE  = (SELECT RUN_STATUS FROM TPSSM11 WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV AND SM_PLAN_NO =@tpssm11.SM_PLAN_NO) \
											   					 ";
			break;
		}//wcy 二钢小代码
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		if (cmd_tpssm11_inq.Read())
		{
			CDataRow & row = bcls_ret->Tables[0].Rows.Add();
			row["RUN_STATUS"] = cmd_tpssm11_inq.GetString(1);
		}
		cmd_tpssm11_inq.Close();

		//设备代码
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT * FROM ( select dev_code || '-' || charge_no,charge_no from tpssm12 where sm_plan_no = @sm_plan_no UNION ALL  select dev_code || '-' || charge_no,charge_no from tpssm42 where sm_plan_no = @sm_plan_no ) ORDER BY charge_no ";

			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			CDataRow & row = bcls_ret->Tables[1].Rows.Add();
			row["DEV_CODE"] = cmd_tpssm11_inq.GetString(1);
		}
		cmd_tpssm11_inq.Close();

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
	cmd_tpssm11_inq.Close();
	return doFlag;

}
