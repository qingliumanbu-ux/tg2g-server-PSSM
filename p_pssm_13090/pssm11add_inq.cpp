/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-16
Version:  3.1.0
Description: 出钢计划编制条件查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件





/*<remark>=========================================================
/// <summary>
/// 出钢计划编制条件查询
/// <para>查询连铸炉次条件和连铸浇铸信息。                 </para>
/// <para>数据库表：TPSSM26/10                             </para>
/// <para>主调用函数：PSSM11P编制对话框画面调用。          </para>
/// </summary>
/// <param name="sm_unit_no"> 炼钢厂别代码      </param>
/// <returns>连铸炉次条件和连铸浇铸信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11add_inq)

int f_pssm11add_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量

	int doFlag = 0;

	/* 业务变量 */
	CString factory_div = "";
	CString cc_mach_no = "";
	int count = 0;
	int count1 = 0;

	CString sqlstr = "";

	CDbCommand cmd_tpssm26_inq(conn);

	// 定义表的实体对象
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm26("TPSSM26");


	try
	{

		//---------------------------------------------------
		//设定返回数据表
		//1)连铸公共条件(出钢计划编制条件)
		bcls_ret->Tables[0].set_TableName("TPSSM26");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");    //连铸机设备代码
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TOTAL_NUM");  //可编计划数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PONO_NUM"); //编入计划数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_SHOW");   //当前CAST号，显示用

		CDataRow *prow = 0;
		
		//---------------------------------------------------
		//获得输入参数
		//CString flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString().Trim();
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().Trim();

		if (cc_mach_no.Trim() == "")
		{
			cc_mach_no = "0";
		}
		//////Log::Trace("", __FUNCTION__, "flag1[{0}]", flag);
		//---------------------------------------------------
		//查询连铸公共条件
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT * FROM TPSSM26 "
				"  WHERE FACTORY_DIV = @factory_div "
				"    AND CC_MACH_NO = DECODE(@cc_mach_no, '0', CC_MACH_NO, @cc_mach_no) "
				" ORDER BY DEV_CODE ASC "
				);
			break;
		}
		cmd_tpssm26_inq.SetCommandText(sqlstr);
		cmd_tpssm26_inq.Parameters.Set("factory_div", factory_div);
		cmd_tpssm26_inq.Parameters.Set("cc_mach_no", cc_mach_no);
		cmd_tpssm26_inq.ExecuteReader();
		while (cmd_tpssm26_inq.Read())
		{
			cmd_tpssm26_inq.Fetch(tpssm26);

			////Log::Trace("", __FUNCTION__, "DEV_CODE=[{0}], CAST_NO=[{1}], CAST_DIV_NO=[{2}]", tpssm26["DEV_CODE"].ToString(), tpssm26["CAST_NO"].ToString(), tpssm26["CAST_DIV_NO"].ToDecimal());

			CDataRow & row = bcls_ret->Tables["TPSSM26"].Rows.Add();

			if (tpssm26["CAST_NO"].ToString().Trim() == "")
			{
				row["CAST_SHOW"] = "";
			}
			else
			{
				row["CAST_SHOW"] = tpssm26["CAST_NO"].ToString().Trim() + "-" + tpssm26["CAST_DIV_NO"].ToDecimal().ToString();
			}

			//统计该铸机下可编计划数
			tpssm10["CC_MACH_NO"] = tpssm26["CC_MACH_NO"];
			tpssm10["FACTORY_DIV"] = tpssm26["FACTORY_DIV"];
			tpssm10["PONO_STATUS"] = 16;  //16-未编入计划
			count = tpssm10.QueryCount("CC_MACH_NO, PONO_STATUS");

			//统计出钢计划下编入计划数
			tpssm11["CC_MACH_NO"] = tpssm26["CC_MACH_NO"];
			count1 = tpssm11.QueryCount("CC_MACH_NO");

			row["CC_MACH_NO"] = tpssm26["CC_MACH_NO"];
			row["TOTAL_NUM"] = CDecimal(count);
			row["PONO_NUM"] = CDecimal(count1);
		}
		cmd_tpssm26_inq.Close();
		
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

	cmd_tpssm26_inq.Close();
	
	return doFlag;

}
