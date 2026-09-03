/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2015-07-13
Version:  3.1.0
Description: 返送炉次对话框信息查询
**************************************************/
//框架头文件
#include "stdafx.h"

//业务头文件
#include "tpssm11.h"

/*<remark>=========================================================
///<summary>
///返送炉次对话框信息查询
///<para>查询连铸设备</para>
///</summary>
/// <param name="sm_plan_no">2个炼钢计划号  </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11retdlg_inq);

int f_pssm11retdlg_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;

	CString pono = " ";
	CString factory_div = " ";

	/* 实体类定义 */
	CTPSSM11 tpssm11(conn);

	/* 数据库操作类定义 */
	CString sqlstr("");
	CDbCommand cmd_inq(conn);


	try
	{
		//---------------------------------------------------
		//设定返回参数表
		bcls_ret->Tables[0].set_TableName("TPSSM_PLAN"); //与Client端DS_TPSSM35.TPSSM_PLAN一致
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SM_PLAN_NO");  //计划号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO");  //钢包号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_SHOW");

		//根据模板定义各连铸机返回块
		CDataTable &table = bcls_ret->Tables.Add("TPSSM11"); //与Client端DS_TPSSM35.TPSSM11一致
		table.Columns.Add(tpssm11);  //从实体对象创建架构
		table.Columns.Add(DT_STRING, "CAST_SHOW");
		
		//--------------------------------------------------------------
		//获得输入参数
		pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToDecimal().ToInt32();
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToDecimal().ToInt32();

		//查询炉次信息
		tpssm11.PONO = pono;
		sqlstr = "";
		bool has31 = tpssm11.Query("FACTORY_DIV, PONO");
		if (has31 == false)
		{
			CFormattable arguments[] = { tpssm11.PONO }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令[{0}]在计划中不存在，请确认。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		CDataRow &row31 = bcls_ret->Tables["TPSSM_PLAN"].Rows.Add();
		row31.Merge(tpssm11);

		if (tpssm11.CAST_NO.Trim() != "")
			row31["CAST_SHOW"] = tpssm11.CAST_NO.Trim() + "-" + tpssm11.CAST_DIV_NO.ToString();


		//查询未开始出钢计划
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT * FROM TPSSM11 "  //
				"  WHERE PONO_STATUS < 20 "
				"    AND FACTORY_DIV = @factory_div "
				" ORDER BY STEEL_START_TIME ASC "
				);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm11);

			CDataRow &row = bcls_ret->Tables["TPSSM11"].Rows.Add();
			row.Merge(tpssm11);

			if (tpssm11.CAST_NO.Trim() != "")
				row["CAST_SHOW"] = tpssm11.CAST_NO.Trim() + "-" + tpssm11.CAST_DIV_NO.ToString();

		}
		cmd_inq.Close();

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