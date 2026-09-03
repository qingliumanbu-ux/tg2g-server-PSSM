/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:  3.1.0
Description: 出钢计划各工序作业处理号生成（包括HEAT_NO）
Update：  2014-11-13  xuwen  炉次条件合并
**************************************************/
#include "stdafx.h"

//程序用头文件
#include "tpssm11.h"
#include "tpssm12.h"
#include "tpssm25.h"
#include "tpssmd1.h"

/*<remark>=========================================================
/// <summary>
/// 出钢计划各工序作业处理号生成计算（包括HEAT_NO。不需要各工序时刻的作法）
/// 根据配置的设备为单位，搜索计划信息，并排序赋号
///   湛江的处理号代码定义：2位设备 + 6位流水。 
/// <para>数据库表：TPSSM11/12/16/25/d1            </para>
/// <para>主调用函数：pssm11_add(出钢计划编入调用)              </para>
/// </summary>
/// <returns>处理结果</returns>
===========================================================</remark>*/
int f_pssm12_sequ_calc(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int fetchRowCount = 0;
	int  blkseq;//i, rows,

	CString  datetime;

	CDecimal    dummy=0;
	//CDecimal    proc_seq_no=0;                    /* 处理顺序号 */
	CString   pre_proc_no="";                /* 预定处理号 */
	CDecimal    iproc_no=0;

	CTPSSM11 tpssm11(conn);
	CTPSSM12 tpssm12(conn);
	CTPSSM25 tpssm25(conn);
	CTPSSMD1 tpssmd1(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm16_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CString sqlstr;

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//----------------------------------------------------------------------------------
		//获得输入参数
		//读取编入计划的炼钢单元号，单记录方式
		blkseq = bcls_rec->Tables.IndexOf("PONO");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PONO] NOT EXIST in f_pssm12_sequ_calc_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		tpssm11.FACTORY_DIV = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		tpssm25.FACTORY_DIV = tpssm11.FACTORY_DIV;
		tpssm25.REC_CREATE_TIME = datetime;
		tpssm25.REC_CREATOR = s.userid;

		//---------------------------------------------------------
		//循环读取各工序设备的信息
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * FROM TPSSMD1 "
				"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
				"    AND AREA_ID > 1 "  //脱硫不算
				"  ORDER BY DEV_CODE ASC "
				);
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11.FACTORY_DIV);
		cmd_tpssmd1_inq.ExecuteReader();
		fetchRowCount = 0;
		while(cmd_tpssmd1_inq.Read())
		{
			cmd_tpssmd1_inq.Fetch(tpssmd1);
			fetchRowCount ++;
			tpssmd1.TrimOrBlank();

			//查看新增出钢计划中是否有该设备的作业
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT COUNT(1) FROM TPSSM12"
					"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
					"    AND DEV_CODE   = @tpssmd1.DEV_CODE "
					"    AND AREA_ID    = @tpssmd1.AREA_ID "
					"    AND ( START_TIME_REAL = ' ' and END_TIME_REAL = ' ' and  ARRIVE_REAL_TIME=' ' and  LEAVE_REAL_TIME = ' ') "
					"    AND SM_PLAN_NO IN "
					"        ( "
					"           SELECT SM_PLAN_NO FROM TPSSM11 "
					"            WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
					"              AND PONO_STATUS < 83 "
					"        ) "
					);
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1.FACTORY_DIV);
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1.DEV_CODE);
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1.AREA_ID);
			dummy = cmd_tpssm12_inq.ExecuteScalar();
			if (dummy == 0) //没有则不再做计算
			{
				continue;
			}

			//读取该设备下的最大顺序号
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT MAX(PROC_NO) "
					"   FROM TPSSM12 "
					"  WHERE DEV_CODE   = @tpssmd1.DEV_CODE "
					"    AND AREA_ID    = @tpssmd1.AREA_ID "
					);
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE",tpssmd1.DEV_CODE);
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.AREA_ID",tpssmd1.AREA_ID);
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				//proc_seq_no = cmd_tpssm12_inq.GetDecimal(1);
				pre_proc_no = cmd_tpssm12_inq.GetString(1).TrimOrBlank();
			}
			else
			{
				//proc_seq_no = 0;
				pre_proc_no = " ";
			}
			cmd_tpssm12_inq.Close();

			Log::Trace("", __FUNCTION__, "dev_code[{0}]: proc_no=[{1}]", tpssmd1.DEV_CODE, pre_proc_no);
			

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT PROC_NO FROM TPSSM25 "
					"  WHERE DEV_CODE   = @tpssmd1.DEV_CODE "
					);
				break;
			}
			cmd_tpssm25_inq.SetCommandText(sqlstr);
			cmd_tpssm25_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1.DEV_CODE);
			cmd_tpssm25_inq.ExecuteReader();
			if(cmd_tpssm25_inq.Read())
			{
				tpssm25.PROC_NO = cmd_tpssm25_inq.GetString(1);
			}
			else
			{
				tpssm25.PROC_NO = pre_proc_no;
			}
			cmd_tpssm25_inq.Close();

			Log::Trace("", __FUNCTION__, "PROC_NO=[{0}]", tpssm25.PROC_NO.Trim() );

			//获取流水部分（2位设备 + 6位流水）
			//iproc_no = atol(&tpssm25.CURR_PROC_NO[3]); //从第3位开始流水, 6位流水
			if (tpssm25.PROC_NO.Trim().GetLength() > 3)
			{
				iproc_no = iproc_no.Parse(tpssm25.PROC_NO.Trim().SubstringNE(2));
			}				
			else
			{
				iproc_no = 0;
			}

			Log::Trace("", __FUNCTION__, "iproc_no=[{0}], tpssmd1.DEV_CODE=[{1}]", iproc_no, tpssmd1.DEV_CODE);

			//----------------------------------------------------------
			//按开始时刻排序，循环读取各设备计划，并赋顺序号
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = CString(
					" SELECT * FROM TPSSM12 "
					"  WHERE DEV_CODE   = @tpssmd1.DEV_CODE "
					"    AND ( START_TIME_REAL=' ' and END_TIME_REAL=' ' and  ARRIVE_TIME_REAL=' ' and  LEAVE_TIME_REAL=' ') "
					"    AND SM_PLAN_NO IN "
					"        ( "
					"           SELECT SM_PLAN_NO FROM TPSSM11 "
					"            WHERE PONO_STATUS < 83 "  //83-浇铸完毕
					"        ) "
					"    AND SUB_CHARGE_NO = 0 "
					"  ORDER BY START_TIME ASC "
					);
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1.DEV_CODE);
			cmd_tpssm12_inq.ExecuteReader();
			while(cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
				tpssm12.TrimOrBlank();

				Log::Trace("", __FUNCTION__, "pono=[{0}], charge_no=[{1}], dev_code=[{2}]", tpssm12.PONO, tpssm12.CHARGE_NO, tpssm12.DEV_CODE);

				iproc_no = (iproc_no.ToInt32() + 1)%1000000; // 6位流水
				//HYF 20130427 SubstringNE

				//(const char*)datetime.Trim().SubstringNE(3, 1),  获取年末位
				tpssm12.PROC_NO = tpssm12.PROC_NO.Format("%s%.6d", (const char*)tpssmd1.DEV_CODE.Trim(), iproc_no.ToInt32());
				Log::Trace("", __FUNCTION__, "pre_proc_no=[{0}]", tpssm12.PROC_NO);

				//赋值
				sqlstr = "tpssm12.Update()";
				tpssm12.Update(
					"PROC_NO",   //预定处理号
					"SM_PLAN_NO,CHARGE_NO");

				//湛江的钢号由L2产生，注释 xuwen 
				//if(tpssmd1.AREA_ID == 3) //转炉、电炉时，写入主表
				//{					
				//	tpssm11.HEAT_NO = tpssm12.PROC_NO;
				//	tpssm11.PONO = tpssm12.PONO;

				//	sqlstr = "tpssm11.Update()";
				//	tpssm11.Update("HEAT_NO", "PONO");
				//}

			}
			cmd_tpssm12_inq.Close();

		}
		cmd_tpssmd1_inq.Close();

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

	cmd_tpssm12_inq.Close();
	cmd_tpssm25_inq.Close();
	cmd_tpssmd1_inq.Close();

	return doFlag;
}
