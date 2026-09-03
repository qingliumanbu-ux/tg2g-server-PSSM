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


int f_pssm21_ot_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
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
BM2F_ENTERACE(pssm11signal_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm11signal_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CString v_dev_code = ""; //DEV_CODE-CHARGE_NO
	CString dev_code = ""; //DEV_CODE
	CDecimal charge_no = 0; //CHARGE编号
	int v_code = 0;

	CModel tpssm11("TPSSM11");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);

	try
	{
		//获得输入参数
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"];
		//v_dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"];

		dev_code = v_dev_code.SubstringNE(0, 2);
		charge_no = atoi(v_dev_code.SubstringNE(v_dev_code.GetLength() - 1, 1));

		//设定返回参数表

		//运转状态
		bcls_ret->Tables[0].set_TableName("STATE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "RUN_STATUS");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_CODE");

		//设备代码
		bcls_ret->Tables.Add();
		bcls_ret->Tables[1].set_TableName("DEV_CODE");
		bcls_ret->Tables[1].Columns.Add(DT_STRING, "DEV_CODE");

		//计划时间
		bcls_ret->Tables.Add();
		bcls_ret->Tables[2].set_TableName("PROC_TIME");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "START_TIME");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "END_TIME");
		bcls_ret->Tables[2].Columns.Add(DT_STRING, "PRE_PROC_NO");

		CDataRow & row_0 = bcls_ret->Tables[0].Rows.Add();
		//运转状态
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT TO_NUMBER(CODE), CODE_DESC_1_CONTENT FROM TEP0002 T WHERE CODE_CLASS = 'PSA2' \
					 					                           AND CODE  = (SELECT RUN_STATUS FROM TPSSM11 WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV AND PONO =@tpssm11.PONO) \
																   											   					 ";
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		if (cmd_tpssm11_inq.Read())
		{
			v_code = cmd_tpssm11_inq.GetInt32(1);
			row_0["RUN_STATUS"] = cmd_tpssm11_inq.GetString(2);
		}
		cmd_tpssm11_inq.Close();

		if (v_code >= 83)
		{
			//当前设备代码
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				sqlstr = "SELECT DEV_CODE||'-'|| CHARGE_NO, DEV_CODE, CHARGE_NO, HEAT_NO FROM TPSSM12 T	 \
						 							WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
																				AND T.SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO) \
																											ORDER BY CHARGE_NO DESC FETCH FIRST 1 ROWS ONLY \
																																		";
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				sqlstr = "SELECT DEV_CODE||'-'|| CHARGE_NO, DEV_CODE, CHARGE_NO, HEAT_NO FROM TPSSM12 T	 \
						 							WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
																				AND T.SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO) \
																											ORDER BY CHARGE_NO DESC FETCH FIRST 1 ROWS ONLY \
																																		";

			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = "  SELECT * FROM ( \
						 					        SELECT DEV_CODE||'-'|| CHARGE_NO, DEV_CODE, CHARGE_NO, HEAT_NO FROM TPSSM12 T	 \
																				WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
																											AND T.SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO) \
																																		ORDER BY CHARGE_NO DESC) ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				row_0["DEV_CODE"] = cmd_tpssm12_inq.GetString(1);
				dev_code = cmd_tpssm12_inq.GetString(2);
				charge_no = cmd_tpssm12_inq.GetDecimal(3);
				row_0["HEAT_NO"] = cmd_tpssm12_inq.GetString(4);
			}
			cmd_tpssm12_inq.Close();

			//设备代码
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = " SELECT DEV_CODE||'-'|| CHARGE_NO FROM TPSSM12 "
					" WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) "
					"   AND FACTORY_DIV = @tpssm11.FACTORY_DIV  ";

				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				CDataRow & row_1 = bcls_ret->Tables[1].Rows.Add();
				row_1["DEV_CODE"] = cmd_tpssm11_inq.GetString(1);
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

				sqlstr = " SELECT START_TIME,END_TIME,START_TIME_REAL,END_TIME_REAL,PRE_PROC_NO FROM TPSSM12"
					" WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) "
					" AND DEV_CODE = @DEV_CODE AND CHARGE_NO = @CHARGE_NO "
					" AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";

				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm11_inq.Parameters.Set("DEV_CODE", dev_code);
			cmd_tpssm11_inq.Parameters.Set("CHARGE_NO", charge_no);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				CDataRow & row_2 = bcls_ret->Tables[2].Rows.Add();

				if (cmd_tpssm11_inq.GetString(3).Trim() != "")
				{
					row_2["START_TIME"] = cmd_tpssm11_inq.GetString(3);
				}
				else
				{
					row_2["START_TIME"] = cmd_tpssm11_inq.GetString(1);
				}

				if (cmd_tpssm11_inq.GetString(4).Trim() != "")
				{
					row_2["END_TIME"] = cmd_tpssm11_inq.GetString(4);
				}
				else
				{
					row_2["END_TIME"] = cmd_tpssm11_inq.GetString(2);
				}

				row_2["PRE_PROC_NO"] = cmd_tpssm11_inq.GetString(5);
			}
			cmd_tpssm11_inq.Close();
		}
		else
		{
			//当前设备代码
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				sqlstr = "SELECT DEV_CODE||'-'|| CHARGE_NO, DEV_CODE, CHARGE_NO, HEAT_NO FROM TPSSM12 T	 \
							WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
							AND T.SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO)  \
							AND(T.START_TIME_REAL = ' ' OR T.END_TIME_REAL = ' ')  \
							ORDER BY CHARGE_NO FETCH FIRST 1 ROWS ONLY \
							";
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				sqlstr = "SELECT DEV_CODE||'-'|| CHARGE_NO, DEV_CODE, CHARGE_NO, HEAT_NO FROM TPSSM12 T	 \
							WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
							AND T.SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO)  \
							AND(T.START_TIME_REAL = ' ' OR T.END_TIME_REAL = ' ')  \
							ORDER BY CHARGE_NO FETCH FIRST 1 ROWS ONLY \
							";
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库

			default:

				sqlstr = "  SELECT * FROM ( \
						 					        SELECT DEV_CODE||'-'|| CHARGE_NO, DEV_CODE, CHARGE_NO, HEAT_NO FROM TPSSM12 T	 \
																				WHERE T.FACTORY_DIV = @tpssm11.FACTORY_DIV  \
																											AND T.SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO)  \
																																		AND(T.START_TIME_REAL = ' ' OR T.END_TIME_REAL = ' ')  \
																																									ORDER BY CHARGE_NO ) ";

				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				row_0["DEV_CODE"] = cmd_tpssm12_inq.GetString(1);
				dev_code = cmd_tpssm12_inq.GetString(2);
				charge_no = cmd_tpssm12_inq.GetDecimal(3);
				row_0["HEAT_NO"] = cmd_tpssm12_inq.GetString(4);
			}
			cmd_tpssm12_inq.Close();

			//设备代码
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = " SELECT DEV_CODE||'-'|| CHARGE_NO FROM TPSSM12 "
					" WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) "
					"   AND FACTORY_DIV = @tpssm11.FACTORY_DIV  ";

				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				CDataRow & row_1 = bcls_ret->Tables[1].Rows.Add();
				row_1["DEV_CODE"] = cmd_tpssm11_inq.GetString(1);
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

				sqlstr = " SELECT START_TIME,END_TIME,START_TIME_REAL,END_TIME_REAL,PRE_PROC_NO FROM TPSSM12"
					" WHERE SM_PLAN_NO = (SELECT SM_PLAN_NO FROM TPSSM11 WHERE PONO = @tpssm11.PONO AND FACTORY_DIV = @tpssm11.FACTORY_DIV) "
					" AND DEV_CODE = @DEV_CODE AND CHARGE_NO = @CHARGE_NO "
					" AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";

				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_tpssm11_inq.Parameters.Set("DEV_CODE", dev_code);
			cmd_tpssm11_inq.Parameters.Set("CHARGE_NO", charge_no);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				CDataRow & row_2 = bcls_ret->Tables[2].Rows.Add();

				if (cmd_tpssm11_inq.GetString(3).Trim() != "")
				{
					row_2["START_TIME"] = cmd_tpssm11_inq.GetString(3);
				}
				else
				{
					row_2["START_TIME"] = cmd_tpssm11_inq.GetString(1);
				}

				if (cmd_tpssm11_inq.GetString(4).Trim() != "")
				{
					row_2["END_TIME"] = cmd_tpssm11_inq.GetString(4);
				}
				else
				{
					row_2["END_TIME"] = cmd_tpssm11_inq.GetString(2);
				}

				row_2["PRE_PROC_NO"] = cmd_tpssm11_inq.GetString(5);
			}
			cmd_tpssm11_inq.Close();
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
	cmd_tpssm11_inq.Close();
	return doFlag;

}
