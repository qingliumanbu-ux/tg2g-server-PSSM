/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:  3.1.0
Description: 出钢计划各工序作业同工位处理号生成
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
int f_pssm12_treatmentcount(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CString sqlstr;
	CString special_flag = "";

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");


		//----------------------------------------------------------------------------------
		//获得输入参数
		blkseq = bcls_rec->Tables.IndexOf("PLAN"); 
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm21_cast_cre_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		FACTORY_DIV = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		if (bcls_rec->Tables[blkseq].Columns.Contains("SPECIAL_FLAG"))
		{
			special_flag = bcls_rec->Tables[blkseq].Rows[0]["SPECIAL_FLAG"].ToString();
		}

		if (special_flag != "1")
		{
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
					" SELECT a.* FROM TPSSM12 a, TPSSM11 b "
					"  WHERE a.FACTORY_DIV = @FACTORY_DIV "
					"  AND b.PONO_STATUS < 83 "
					"  AND a.SM_PLAN_NO = b.SM_PLAN_NO "
					"  ORDER BY a.SM_PLAN_NO, a.CHARGE_NO ASC "
					);
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("FACTORY_DIV", FACTORY_DIV);
			cmd_tpssm12_inq.ExecuteReader();
			while (cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
				if (sm_plan_no != tpssm12["SM_PLAN_NO"].ToString())
				{
					sm_plan_no = tpssm12["SM_PLAN_NO"].ToString();
					dev_code = "";
				}
				dev_code = dev_code + tpssm12["DEV_CODE"].ToString();

				index = -1;
				dummy = 0;

				while (true)
				{
					index = dev_code.Find(tpssm12["DEV_CODE"].ToString().SubstringNE(0,1), index + 1);
					if (index >= 0)
					{
						dummy = dummy + 1;
					}
					else
					{
						tpssm12["TREATMENT_COUNTER"] = dummy;
						tpssm12.Update("TREATMENT_COUNTER", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
						break;
					}

				}
			}
			cmd_tpssm12_inq.Close();
		}
		else
		{
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
					" SELECT * FROM TPSSM16 "
					"  WHERE FACTORY_DIV = @FACTORY_DIV "
					"  ORDER BY SM_PLAN_NO, CHARGE_NO ASC "
					);
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("FACTORY_DIV", FACTORY_DIV);
			cmd_tpssm12_inq.ExecuteReader();
			while (cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm16);
				if (sm_plan_no != tpssm16["SM_PLAN_NO"].ToString())
				{
					sm_plan_no = tpssm16["SM_PLAN_NO"].ToString();
					dev_code = "";
				}
				dev_code = dev_code + tpssm16["DEV_CODE"].ToString();

				index = -1;
				dummy = 0;

				while (true)
				{
					index = dev_code.Find(tpssm16["DEV_CODE"].ToString().SubstringNE(0,1), index + 1);
					if (index >= 0)
					{
						dummy = dummy + 1;
					}
					else
					{
						tpssm16["TREATMENT_COUNTER"] = dummy;
						tpssm16.Update("TREATMENT_COUNTER", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
						break;
					}

				}
			}
			cmd_tpssm12_inq.Close();
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
	return doFlag;
}
