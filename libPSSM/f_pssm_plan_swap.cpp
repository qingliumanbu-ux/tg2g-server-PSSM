/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    xuwen
Version:   3.1.0
Date:      2014-12-12
Description: 计划号交换
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 出钢计划交换-计划号交换
/// <para>当选择二炉计划都未进入生产时，进行的计划号交换(包括钢号交换)。</para>
/// <para>数据库表：TPSSM11/12(出钢计划主子表)                  </para>
/// <para>主调用函数：pssm11_stno_tra 调用。                    </para>
/// </summary>
/// <param name="plan_no">计划号       </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_plan_swap(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;
	
	CDecimal sm_plan_no_tmp = 99999999;  //8位临时计划号
	CString v_factory_div = "";

	CModel tpssm11_1("TPSSM11");//计划1
	CModel tpssm11_2("TPSSM11");//计划2

	CString sqlstr;

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);


	try
	{

		//--------------------------------------------------------------
		//获得输入参数
		//1. 读取指定的默认设备，存储在dev_code数组中
		blkseq = bcls_rec->Tables.IndexOf("PLAN_SWAP");
		if (blkseq < 0)
		{
			strcpy(s.msg, "计划号交换时传入数据块[PLAN_SWAP]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN_SWAP] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToDecimal().ToInt32();
			tpssm11_1["SM_PLAN_NO"] = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO1"].ToDecimal().ToInt32();
			tpssm11_2["SM_PLAN_NO"] = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO2"].ToDecimal().ToInt32();

			////Log::Info("", __FUNCTION__, "SM_PLAN_NO1=[{0}], SM_PLAN_NO2=[{1}]", tpssm11_1["SM_PLAN_NO"].ToString(), tpssm11_2["SM_PLAN_NO"].ToString());


			//------------------------
			// TPSSM11表 修改PLAN1的全部工序，计划号改为临时号
			sqlstr = CString(
				"UPDATE TPSSM11 "
				"   SET SM_PLAN_NO = @sm_plan_no_tmp "
				" WHERE SM_PLAN_NO = @sm_plan_no1 "
				"   AND FACTORY_DIV = @v_factory_div "
				);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("sm_plan_no_tmp", sm_plan_no_tmp);
			cmd_upd.Parameters.Set("sm_plan_no1", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
			cmd_upd.ExecuteNonQuery();

			// TPSSM11表 修改PLAN2的全部工序，计划号改为PLAN1
			sqlstr = CString(
				"UPDATE TPSSM11 "
				"   SET SM_PLAN_NO = @sm_plan_no1 "
				" WHERE SM_PLAN_NO = @sm_plan_no2 "
				"   AND FACTORY_DIV = @v_factory_div "
				);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("sm_plan_no1", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("sm_plan_no2", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
			cmd_upd.ExecuteNonQuery();

			// TPSSM11表 修改临时计划号,改为PLAN2号
			sqlstr = CString(
				"UPDATE TPSSM11 "
				"   SET SM_PLAN_NO = @sm_plan_no2 "
				" WHERE SM_PLAN_NO = @sm_plan_no_tmp "
				"   AND FACTORY_DIV = @v_factory_div "
				);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("sm_plan_no2", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("sm_plan_no_tmp", sm_plan_no_tmp);
			cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
			cmd_upd.ExecuteNonQuery();


			//------------------------
			// TPSSM12表 修改PLAN1的全部工序，计划号改为临时号
			sqlstr = CString(
				"UPDATE TPSSM12 "
				"   SET SM_PLAN_NO = @sm_plan_no_tmp "
				" WHERE SM_PLAN_NO = @sm_plan_no1 "
				"   AND FACTORY_DIV = @v_factory_div "
				);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("sm_plan_no_tmp", sm_plan_no_tmp);
			cmd_upd.Parameters.Set("sm_plan_no1", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
			cmd_upd.ExecuteNonQuery();

			// TPSSM12表 修改PLAN2的全部工序，计划号改为PLAN1
			sqlstr = CString(
				"UPDATE TPSSM12 "
				"   SET SM_PLAN_NO = @sm_plan_no1 "
				" WHERE SM_PLAN_NO = @sm_plan_no2 "
				"   AND FACTORY_DIV = @v_factory_div "
				);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("sm_plan_no1", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("sm_plan_no2", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
			cmd_upd.ExecuteNonQuery();

			// TPSSM12表 修改临时计划号,改为PLAN2号
			sqlstr = CString(
				"UPDATE TPSSM12 "
				"   SET SM_PLAN_NO = @sm_plan_no2 "
				" WHERE SM_PLAN_NO = @sm_plan_no_tmp "
				"   AND FACTORY_DIV = @v_factory_div "
				);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("sm_plan_no2", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd.Parameters.Set("sm_plan_no_tmp", sm_plan_no_tmp);
			cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
			cmd_upd.ExecuteNonQuery();

		}//for



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
