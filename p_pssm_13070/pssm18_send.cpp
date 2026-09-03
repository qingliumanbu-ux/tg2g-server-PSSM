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
BM2F_ENTERACE(pssm18_send)

int f_pssm18_send(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "LG1";	//炼钢单元号
	CString v_pono = "";
	CString v_restrand_flg = "";     //连浇标记
	CString v_cc_req_time = "";  //开浇时刻
	CString v_tpd_start_time = "";  //倒罐开始时刻
	CDecimal v_td_chg_flg = 0, v_smelt_mode = 0;
	CDecimal charge_no = 0;      //工序charge号
	CString datetime = "";
	CString datetime_send = "";
	CString routebagkey = "";
	CString routelist = "";
	CString sm_plan_no = " ";
	CString Asm_plan_noA = " ";

	double time = 0;

	EIClass inblk;        //调用函数用
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm18_del(conn);
	CDbCommand cmd_tpssm11_inq(conn);

	CDataTable tb_tpssm11("TPSSM11");

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		time = bcls_rec->Tables["TIME"].Rows[0]["TIME"].ToDouble();
		//time = 240;
		Log::Trace("", __FUNCTION__, "时间范围time={0}", time);

		if (time > 0)
		{
			datetime_send = (CDateTime::Parse(datetime).AddHours(time).ToString("yyyyMMddHHmmss"));
			sqlstr = "SELECT DISTINCT A.SM_PLAN_NO FROM TPSSM12 A, TPSSM11 B WHERE A.FACTORY_DIV=@v_factory_div AND A.CHARGE_NO = 1 AND DECODE( A.START_TIME_REAL,' ',A.START_TIME,A.START_TIME_REAL ) < @datetime_send AND A.SM_PLAN_NO = B.SM_PLAN_NO AND B.RUN_STATUS < '52' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.Parameters.Set("datetime_send", datetime_send);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				if (sm_plan_no.Trim() == "")
				{
					sm_plan_no = cmd_inq.GetString(1);
					Asm_plan_noA = "A" + cmd_inq.GetString(1) + "A"; 
				}
				else
				{
					sm_plan_no = sm_plan_no + "," + cmd_inq.GetString(1);
					Asm_plan_noA = Asm_plan_noA + "," + "A" + cmd_inq.GetString(1) + "A";
				}
			}
			cmd_inq.Close();

			sqlstr = "SELECT DISTINCT A.SM_PLAN_NO FROM TPSSM14 A, TPSSM13 B WHERE A.FACTORY_DIV=@v_factory_div AND A.CHARGE_NO = 1 AND DECODE( A.START_TIME_REAL,' ',A.START_TIME,A.START_TIME_REAL ) < @datetime_send AND A.SM_PLAN_NO = B.SM_PLAN_NO AND B.RUN_STATUS < '52' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.Parameters.Set("datetime_send", datetime_send);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				if (sm_plan_no.Trim() == "")
				{
					sm_plan_no = cmd_inq.GetString(1);
					Asm_plan_noA = "A" + cmd_inq.GetString(1) + "A";
				}
				else
				{
					if (Asm_plan_noA.Find("A" + cmd_inq.GetString(1) + "A") < 0)
					{
						sm_plan_no = sm_plan_no + "," + cmd_inq.GetString(1);
						Asm_plan_noA = Asm_plan_noA + "," + "A" + cmd_inq.GetString(1) + "A";
					}
				}
			}
			cmd_inq.Close();
		}
		Log::Trace("", __FUNCTION__, "计划范围sm_plan_no={0}", sm_plan_no);
		Log::Trace("", __FUNCTION__, "计划范围sm_plan_no={0}", Asm_plan_noA);

		if (sm_plan_no.Trim() != "")
		{
			//--------------------------------
			//定义函数调用信息结构
			//1、总体计划块
			inblk.Tables[0].set_TableName("PLAN");  //
			inblk.Tables["PLAN"].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
			inblk.Tables["PLAN"].Columns.Add(DT_STRING, "OPER_FLAG");
			inblk.Tables["PLAN"].Columns.Add(DT_STRING, "SM_PLAN_NO");
			inblk.Tables["PLAN"].Columns.Add(DT_STRING, "SM_PLAN_NO_IN");
			inblk.Tables["PLAN"].Columns.Add(DT_STRING, "SM_PLAN_NO_AA");
			inblk.Tables["PLAN"].Rows.Add();

			inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;
			inblk.Tables["PLAN"].Rows[0]["OPER_FLAG"] = "I";//新增
			inblk.Tables["PLAN"].Rows[0]["SM_PLAN_NO"] = " ";
			inblk.Tables["PLAN"].Rows[0]["SM_PLAN_NO_IN"] = sm_plan_no;
			inblk.Tables["PLAN"].Rows[0]["SM_PLAN_NO_AA"] = Asm_plan_noA;

			ret = f_t8e2s1_snd(&inblk, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		//wcy 比较表
		if (sm_plan_no.Trim() == "")
		{
			sqlstr = " DELETE FROM TPSSM13 ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " INSERT INTO TPSSM13 SELECT * FROM TPSSM11 ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " DELETE FROM TPSSM14 ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " INSERT INTO TPSSM14 SELECT * FROM TPSSM12 ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}
		else
		{
			sqlstr = " DELETE FROM TPSSM13 WHERE SM_PLAN_NO IN (" + sm_plan_no + ")";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " INSERT INTO TPSSM13 SELECT * FROM TPSSM11 WHERE SM_PLAN_NO IN (" + sm_plan_no + ")";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " DELETE FROM TPSSM14 WHERE SM_PLAN_NO IN (" + sm_plan_no + ")";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " INSERT INTO TPSSM14 SELECT * FROM TPSSM12 WHERE SM_PLAN_NO IN (" + sm_plan_no + ")";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
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
