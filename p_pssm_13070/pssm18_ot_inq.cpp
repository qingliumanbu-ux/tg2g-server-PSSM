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


int f_pssm21_ot_inq(EIClass *bcls_rec,EIClass *bcls_ret,CDbConnection * conn);
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
BM2F_ENTERACE(pssm18_ot_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm18_ot_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
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
	CDbCommand cmd_inq(conn);
	try
	{
		//获得输入参数
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"];
		v_dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"];

		dev_code = v_dev_code.SubstringNE(0, 2);
		charge_no = atoi(v_dev_code.SubstringNE(v_dev_code.GetLength() - 1, 1));

		//Log::Trace("", __FUNCTION__, "dev_code =[{0}]", dev_code);
		//Log::Trace("", __FUNCTION__, "charge_no =[{0}]", charge_no);
		//Log::Trace("", __FUNCTION__, "FACTORY_DIV =[{0}]", tpssm11["FACTORY_DIV"].ToString());
		//Log::Trace("", __FUNCTION__, "PONO =[{0}]", tpssm11["PONO"].ToString());
		//设定返回参数表

		//运转状态
		bcls_ret->Tables[0].set_TableName("STATE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"RUN_STATUS");

		//设备代码
		bcls_ret->Tables.Add();
		bcls_ret->Tables[1].set_TableName("DEV_CODE");
		bcls_ret->Tables[1].Columns.Add(DT_STRING,"DEV_CODE");
		bcls_ret->Tables[1].Columns.Add(DT_DECIMAL, "CHARGE_NO");

		//计划时间
		bcls_ret->Tables.Add();
		bcls_ret->Tables[2].set_TableName("PROC_TIME");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "START_TIME");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "END_TIME");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "PRE_PROC_NO");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "AREA_ID");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "PROC_NO");

		//设备代码-电炉
		bcls_ret->Tables.Add();
		bcls_ret->Tables[3].set_TableName("DEV_CODE_E");
		bcls_ret->Tables[3].Columns.Add(DT_STRING, "DEV_CODE_E");
		//bcls_ret->Tables[3].Columns.Add(DT_DECIMAL, "CHARGE_NO");

		//运转状态
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT CODE_DESC_1_CONTENT FROM TEP0002 T WHERE CODE_CLASS = 'PSA22N' \
                          AND CODE  = (SELECT RUN_STATUS FROM TPSSM11 WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV AND PONO =@tpssm11.PONO) \
					 ";
			break; 
		}//wcy 二钢小代码
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		if(cmd_tpssm11_inq.Read())
		{			
			CDataRow & row = bcls_ret->Tables[0].Rows.Add();
			row["RUN_STATUS"] = cmd_tpssm11_inq.GetString(1);
		}
		cmd_tpssm11_inq.Close();

		//设备代码
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT * from "
					 " ( select b.dev_code || '-' || a.charge_no AS code, b.dev_code AS DEV_CODE, a.CHARGE_NO AS charge_no from tpssm12 a, tpssmd1 b "
				     " where a.sm_plan_no =(select sm_plan_no from tpssm11 where pono = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) AND SUBSTR(a.DEV_CODE,1,1) = b.DEV_TECH_CODE AND a.AREA_ID <> 5 "
					 " UNION SELECT dev_code||'-'|| charge_no AS code, dev_code AS DEV_CODE, CHARGE_NO AS charge_no from tpssm12 where sm_plan_no =(select sm_plan_no from tpssm11 where pono = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) AND AREA_ID = 5)"
					 "   ORDER BY charge_no,dev_code  ";
					
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		while(cmd_tpssm11_inq.Read())
		{			
			CDataRow & row = bcls_ret->Tables[1].Rows.Add();
			row["DEV_CODE"] = cmd_tpssm11_inq.GetString(1);
			row["CHARGE_NO"] = cmd_tpssm11_inq.GetDecimal(3);
		}
		cmd_tpssm11_inq.Close();

		//计划时间
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT START_TIME,END_TIME,START_TIME_REAL,END_TIME_REAL,PRE_PROC_NO,AREA_ID,PROC_NO FROM TPSSM12"
				" WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) "
				" AND CHARGE_NO = @CHARGE_NO "//AND DEV_CODE = @DEV_CODE 
				" AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";

			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
		//cmd_tpssm11_inq.Parameters.Set("DEV_CODE", dev_code);
		cmd_tpssm11_inq.Parameters.Set("CHARGE_NO", charge_no);
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			CDataRow & row = bcls_ret->Tables[2].Rows.Add();

			if (cmd_tpssm11_inq.GetString(3).Trim() != "")
			{
				row["START_TIME"] = cmd_tpssm11_inq.GetString(3);
			}
			else
			{
				row["START_TIME"] = cmd_tpssm11_inq.GetString(1);
			}

			if (cmd_tpssm11_inq.GetString(4).Trim() != "")
			{
				row["END_TIME"] = cmd_tpssm11_inq.GetString(4);
			}
			else
			{
				row["END_TIME"] = cmd_tpssm11_inq.GetString(2);
			}		
			
			row["PRE_PROC_NO"] = cmd_tpssm11_inq.GetString(5);
			row["AREA_ID"] = cmd_tpssm11_inq.GetString(6);
			row["PROC_NO"] = cmd_tpssm11_inq.GetString(7);
		}
		cmd_tpssm11_inq.Close();
		

		//电炉设备
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT DEV_CODE FROM TPSSMD1 WHERE DEV_TECH_CODE = 'E' ORDER BY DEV_CODE ";

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			CDataRow & row = bcls_ret->Tables[3].Rows.Add();
			row["DEV_CODE_E"] = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
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
	cmd_tpssm11_inq.Close();	
	return doFlag;

}
