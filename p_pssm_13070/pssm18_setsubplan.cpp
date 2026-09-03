/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-26
Version:  3.1.0
Description: 甘特图出钢计划编制保存
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

int f_t8e2s1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划编制保存
/// <para>根据Client甘特图输入的数据，修改出钢计划。  </para>
/// <para>
///   1.出钢计划信息写入:主计划与工序计划
///   2.删除排除的炉次
///   3.CAST号计算
///   4.计划号计算
///   5.处理号计算
/// <para>数据库表：TPSSM11/12                   </para>
/// <para>主调用函数：PSSM18画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>制造命令号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_setsubplan)

int f_pssm18_setsubplan(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "LG1";	//炼钢单元号
	CString v_pono = "";
	CString datetime = "";
	CDecimal v_count_prod = 0;
	CDecimal v_count_prod2 = 0;

	EIClass inblk;        //调用函数用
	CString sqlstr = "";

	CModel tpssm11("TPSSM11");
	CModel tpssm11_1("TPSSM11");
	CModel tpssm11_2("TPSSM11");
	CModel tpssm12_1("TPSSM12");
	CModel tpssm12_2("TPSSM12");

	CDbCommand cmd_tpssm12_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		rows = bcls_rec->Tables[0].Rows.get_Count();
		if (rows != 2)
		{
			strcpy(s.msg, "分包必须有二炉计划")/*钢水交换必须有二炉计划*/;
			//strcpy(s.sysmsg,"钢水交换必须有二炉计划");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//读取传入的 2炉，做校验
		for (i = 1; i <= 2; i++)
		{
			tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[i - 1]["SM_PLAN_NO"].ToString();
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i - 1]["FACTORY_DIV"].ToString();
			sqlstr = "tpssm11.Query()";
			bool has11 = tpssm11.Query("FACTORY_DIV,SM_PLAN_NO");
			if (has11 == false)
			{
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString() };
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]不存在，请重新选择后操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm11.TrimOrBlank();

			sqlstr = " SELECT COUNT(1) FROM TPSSM12 WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO AND DEV_CODE LIKE 'E%' ";
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString().Trim());

			if (i == 1)
			{
				v_count_prod = cmd_tpssm12_inq.ExecuteScalar().ToInt32();
			}
			else
			{
				v_count_prod2 = cmd_tpssm12_inq.ExecuteScalar().ToInt32();
			}
			cmd_tpssm12_inq.Close();

			if (i == 1)
				tpssm11_1.CopyFrom(tpssm11);
			else
				tpssm11_2.CopyFrom(tpssm11);
		}

		if (v_count_prod == 0 && v_count_prod2 == 0)
		{
			strcpy(s.msg, "两个计划都并不存在电炉,不能分包。")/*钢水交换必须有二炉计划*/;
			//strcpy(s.sysmsg,"钢水交换必须有二炉计划");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		else if (v_count_prod > 0 && v_count_prod2 > 0)
		{
			strcpy(s.msg, "两个计划都存在电炉,不能分包。")/*钢水交换必须有二炉计划*/;
			//strcpy(s.sysmsg,"钢水交换必须有二炉计划");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		else if (v_count_prod == 0 && v_count_prod2 > 0)
		{
			tpssm12_1["SM_PLAN_NO"] = tpssm11_1["SM_PLAN_NO"];
			tpssm12_2["SM_PLAN_NO"] = tpssm11_2["SM_PLAN_NO"];


			sqlstr = " SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO AND DEV_CODE LIKE 'E%' ";
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm12_2["SM_PLAN_NO"].ToString().Trim());
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				tpssm12_2["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(1);
			}
			cmd_tpssm12_inq.Close();

			tpssm12_2.Query("SM_PLAN_NO,CHARGE_NO");

			if (tpssm12_2["PROC_SUB"].ToString().Trim() == "" && tpssm12_2["PROC_SUB2"].ToString().Trim() == "")
			{
				tpssm12_2["PROC_SUB"] = tpssm12_1["SM_PLAN_NO"];
				tpssm12_2.Update("PROC_SUB", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_2["PROC_SUB"].ToString().Trim() != "" && tpssm12_2["PROC_SUB2"].ToString().Trim() == "" && tpssm12_2["PROC_SUB"].ToString().Trim() != tpssm12_1["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_2["PROC_SUB2"] = tpssm12_1["SM_PLAN_NO"];
				tpssm12_2.Update("PROC_SUB2", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_2["PROC_SUB"].ToString().Trim() == "" && tpssm12_2["PROC_SUB2"].ToString().Trim() != "" && tpssm12_2["PROC_SUB2"].ToString().Trim() != tpssm12_1["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_2["PROC_SUB"] = tpssm12_1["SM_PLAN_NO"];
				tpssm12_2.Update("PROC_SUB", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_2["PROC_SUB"].ToString().Trim() != "" && tpssm12_2["PROC_SUB2"].ToString().Trim() != "" && tpssm12_2["PROC_SUB2"].ToString().Trim() != tpssm12_1["SM_PLAN_NO"].ToString().Trim() && tpssm12_2["PROC_SUB"].ToString().Trim() != tpssm12_1["SM_PLAN_NO"].ToString().Trim())
			{
				strcpy(s.msg, "电炉最多分3包。")/*钢水交换必须有二炉计划*/;
				//strcpy(s.sysmsg,"钢水交换必须有二炉计划");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else if (tpssm12_2["PROC_SUB"].ToString().Trim() == tpssm12_1["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_2["PROC_SUB"] = " ";
				tpssm12_2.Update("PROC_SUB", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_2["PROC_SUB2"].ToString().Trim() == tpssm12_1["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_2["PROC_SUB2"] = " ";
				tpssm12_2.Update("PROC_SUB2", "SM_PLAN_NO,CHARGE_NO");
			}
		}
		else if (v_count_prod > 0 && v_count_prod2 == 0)
		{
			tpssm12_1["SM_PLAN_NO"] = tpssm11_1["SM_PLAN_NO"];
			tpssm12_2["SM_PLAN_NO"] = tpssm11_2["SM_PLAN_NO"];

			sqlstr = " SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO AND DEV_CODE LIKE 'E%' ";
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm12_1["SM_PLAN_NO"].ToString().Trim());
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				tpssm12_1["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(1);
			}
			cmd_tpssm12_inq.Close();

			tpssm12_1.Query("SM_PLAN_NO,CHARGE_NO");

			if (tpssm12_1["PROC_SUB"].ToString().Trim() == "" && tpssm12_1["PROC_SUB2"].ToString().Trim() == "")
			{
				tpssm12_1["PROC_SUB"] = tpssm12_2["SM_PLAN_NO"];
				tpssm12_1.Update("PROC_SUB", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_1["PROC_SUB"].ToString().Trim() != "" && tpssm12_1["PROC_SUB2"].ToString().Trim() == "" && tpssm12_1["PROC_SUB"].ToString().Trim() != tpssm12_2["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_1["PROC_SUB2"] = tpssm12_2["SM_PLAN_NO"];
				tpssm12_1.Update("PROC_SUB2", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_1["PROC_SUB"].ToString().Trim() == "" && tpssm12_1["PROC_SUB2"].ToString().Trim() != "" && tpssm12_1["PROC_SUB2"].ToString().Trim() != tpssm12_2["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_1["PROC_SUB"] = tpssm12_2["SM_PLAN_NO"];
				tpssm12_1.Update("PROC_SUB", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_1["PROC_SUB"].ToString().Trim() != "" && tpssm12_1["PROC_SUB2"].ToString().Trim() != "" && tpssm12_1["PROC_SUB2"].ToString().Trim() != tpssm12_2["SM_PLAN_NO"].ToString().Trim() && tpssm12_1["PROC_SUB"].ToString().Trim() != tpssm12_2["SM_PLAN_NO"].ToString().Trim())
			{
				strcpy(s.msg, "电炉最多分3包。")/*钢水交换必须有二炉计划*/;
				//strcpy(s.sysmsg,"钢水交换必须有二炉计划");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else if (tpssm12_1["PROC_SUB"].ToString().Trim() == tpssm12_2["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_1["PROC_SUB"] = " ";
				tpssm12_1.Update("PROC_SUB", "SM_PLAN_NO,CHARGE_NO");
			}
			else if (tpssm12_1["PROC_SUB2"].ToString().Trim() == tpssm12_2["SM_PLAN_NO"].ToString().Trim())
			{
				tpssm12_1["PROC_SUB2"] = " ";
				tpssm12_1.Update("PROC_SUB2", "SM_PLAN_NO,CHARGE_NO");
			}
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
