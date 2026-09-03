/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:  3.1.0
Description: 复制计划给中频炉信息
Update：  2014-11-13  xuwen  炉次条件合并
**************************************************/
#include "stdafx.h"

//程序用头文件





/*<remark>=========================================================
/// <summary>
/// 出钢计划各工序作业顺序计算（包括HEAT_NO。不需要各工序时刻的作法）
/// 根据配置的设备为单位，搜索计划信息，并排序赋号
/// <para>数据库表：TPSSM11/12/16/25/d1            </para>
/// <para>主调用函数：pssm11_add(出钢计划编入调用)              </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm12_zpl_copy(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int fetchRowCount = 0;
	int index = 0;
	int  blkseq;//i, rows,

	CString  datetime;

	CDecimal    dummy = 0;
	CString   dev_code = "";
	CString   sm_plan_no = "";
	CString  FACTORY_DIV = "LG1";
	CString   cs_flag = "";
	CModel tpssm12_zpl("TPSSM12_ZPL");
	CModel tpssm12("TPSSM12");
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CString sqlstr;
	CString sqlstr_1;
	CString special_flag = "";

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		sqlstr_1 = " DELETE TPSSM12_ZPL  ";
		cmd_tpssm11_inq.SetCommandText(sqlstr_1);
		cmd_tpssm11_inq.ExecuteNonQuery();
		cmd_tpssm11_inq.Close();
		//---------------------------------------------------------
		//循环读取各工序设备的信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				"  INSERT INTO TPSSM12_ZPL ( SELECT   a.* FROM TPSSM12 A  WHERE   a.AREA_ID IN(2, 3)  "
				"   UNION ALL    "
				"   SELECT   a.*FROM TPSSM42 A  WHERE   a.AREA_ID IN(2, 3) AND  A.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS')) ");
			break;
		}
		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.ExecuteNonQuery();
		cmd_tpssm12_inq.Close();
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
