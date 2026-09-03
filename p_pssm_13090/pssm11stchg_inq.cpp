/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-12-26 17:13:56
Version:  3.1.0
Description: 钢种变更-可变更炉次命令查询
**************************************************/
//框架头文件
#include "stdafx.h"

//业务头文件


/*<remark>=========================================================
///<summary>
///可变更炉次命令查询
///<para>数据库表：TPSSM01(炼钢炉次命令信息表)</para>
///</summary>
/// <param name="sm_plan_no">2个炼钢计划号  </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11stchg_inq);

int f_pssm11stchg_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;

	CString v_dev_code = "";
	CString v_cc_mach_no = "";
	CString v_factory_div = " ";

	EIClass inBlock;

	/* 实体类定义 */
	CModel tpssm10("TPSSM10");

	/* 数据库SQL操作字符串 */
	CString sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);


	try
	{
		//---------------------------------------------------
		//设定返回参数表
		//连铸浇铸信息
		bcls_ret->Tables[0].set_TableName("TPSSM10");
		bcls_ret->Tables["TPSSM10"].Columns.Add(tpssm10);
		bcls_ret->Tables["TPSSM10"].Columns.Add(DT_STRING, "CAST_SHOW");  //CAST号，显示用
		bcls_ret->Tables["TPSSM10"].Columns.Add(DT_STRING, "LOT_SHOW");   //浇次号，显示用
		bcls_ret->Tables["TPSSM10"].Columns.Add(DT_STRING, "SLAB_SPEC");  //铸坯规格(厚宽长)


		//---------------------------------------------------
		//获得输入参数(单记录)
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		v_cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().Trim();

		//////Log::Info("", __FUNCTION__, "dev_code=[{0}]", v_dev_code);

		////传入的参数是设备代码，取第2位为连铸机号
		//if (v_dev_code != "")
		//{
		//	v_cc_mach_no = v_dev_code.SubstringNE(1);
		//}
		//else
		//{
		//	v_cc_mach_no = "";
		//}
		////Log::Info("", __FUNCTION__, "cc_mach_no=[{0}]", v_cc_mach_no);

		//---------------------------------------------------
		//查询连铸浇铸信息，查询未编入计划的炉次
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(" SELECT * FROM TPSSM10 WHERE PONO_STATUS = 16 ");//16-命令接收
			if (v_factory_div != "") sqlstr = sqlstr + " AND FACTORY_DIV = @v_factory_div ";
			if (v_cc_mach_no != "") sqlstr = sqlstr + " AND CC_MACH_NO = @v_cc_mach_no ";
			sqlstr = sqlstr + " ORDER BY CC_MACH_NO ASC, CC_SEQ ASC ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.Parameters.Set("v_cc_mach_no", v_cc_mach_no);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm10);

			CDataRow & row = bcls_ret->Tables["TPSSM10"].Rows.Add();
			row.Merge(tpssm10);

			if (tpssm10["CC_REQ_TIME_FLAG"].ToString().Trim() == "1") //有指定要求
			{
				row["CC_REQ_TIME"] = tpssm10["CC_REQ_TIME"];
			}
			else
			{
				row["CC_REQ_TIME"] = "";
			}

			if (tpssm10["CAST_LOT_NO"].ToString().Trim() == "")
			{
				row["CAST_SHOW"] = "";
			}
			else
			{
				row["CAST_SHOW"] = tpssm10["CAST_LOT_NO"].ToString().Trim() + "-" + tpssm10["CAST_LOT_DIV_NO"].ToDecimal().ToString();
			}

			row["LOT_SHOW"] = tpssm10["CAST_LOT_NO"].ToString().Trim() + "-" + tpssm10["CAST_LOT_DIV_NO"].ToDecimal().ToString();
			row["SLAB_SPEC"] = tpssm10["SLAB_THICK"].ToDecimal().ToString() + "*" + tpssm10["SLAB_WIDTH"].ToDecimal().ToString() + "*" + tpssm10["SLAB_LEN"].ToDecimal().ToString();
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
