/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    xuwen
Version:   3.1.0
Date:      2015-4-30
Description: 出钢计划PONO状态代码查询
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
//#include "tpssm10.h"

/*<remark>=========================================================
/// <summary>
/// 出钢计划PONO状态代码查询
/// <para>处理内容：更新 TPSSM10表内容，切割结束标记      </para>
/// <para>数据库表：TPSSM10(浇铸顺序表)                  </para>
/// <para>主调用函数：pssm11_add 调用。               </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_pono_status_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;


	CString sqlstr;
	CDbCommand cmd_inq(conn);


	try
	{
		//--------------------------------------------------------------
		//定义返回数据块
		CDataTable &table = bcls_ret->Tables.Add("PONO_STATUS");
		//bcls_ret->Tables[blkseq].Columns.Add(tpssm11);   //从实体对象创建架构
		table.Columns.Add(DT_STRING, "CODE");    //PONO状态代码
		table.Columns.Add(DT_STRING, "CODE_DESC");   //状态说明

		//--------------------------------------------------------------
		//获得输入参数
		//1. 读取指定的默认设备，存储在dev_code数组中
		//blkseq = bcls_rec->Tables.IndexOf("DEV");
		//if (blkseq < 0)
		//{
		//	strcpy(s.msg, "传入默认设备数据块[DEV]不存在，请联系系统维护人员。");
		//	sprintf(s.sysmsg, "TABLE [DEV] NOT EXIST.");
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
		//rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		//for (i = 0; i < rows; i++)
		//{
		//	tpssmd1.DEV_CODE = bcls_rec->Tables[blkseq].Rows[i]["DEV_CODE"];
		//	////Log::Info("", __FUNCTION__, "dev_code=[{2}]", tpssmd1.DEV_CODE);
		//}//for

		//--------------------------------------------------------------
		//查询代码
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				"SELECT CODE, CODE_DESC_1_CONTENT FROM TEP0002 "
				" WHERE CODE_CLASS = 'PSA1' "  //PS06-炼钢出钢计划（PONO）状态
				"ORDER BY CODE ASC "
			);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("cc_mach_no", tpssmd1.DEV_CODE.SubstringNE(1));
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			//cmd_inq.Fetch(tpssm10);

			CDataRow &row = bcls_ret->Tables["PONO_STATUS"].Rows.Add();
			row["CODE"]      = cmd_inq.GetString(1).Trim();
			row["CODE_DESC"] = cmd_inq.GetString(2).Trim();
		}
		cmd_inq.Close();


		//如果返回为空时
		if (bcls_ret->Tables["PONO_STATUS"].Rows.get_Count() == 0)
		{
		}

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

	return doFlag;
}
