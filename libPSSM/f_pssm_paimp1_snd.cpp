/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dclian
Version:    1.0
Date:     2016-1-05
Description:	 发送连铸预计化至铁前
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

#include "tpssmd1.h"
#include "tpssm10.h"
#include "tpssm01.h"
#include "tpssmdb.h"
#include "tpssm12.h"
#include "tpssm99.h"


int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
/*<remark>=========================================================
/// <summary>
/// 发送出钢计划至L2
///<para>1.读取传入的厂别区分、制造命令号、操作区分</para>
/// <para>2.拼接电文后发送铁前。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：保存下发调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="handle_div">计划号          </param>
/// <param name="pono">制造命令号          </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_paimp1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;
	int fetchRowCount1 = 0;

	CString sqlstr = "";
	int send_flag = 0;

	CString lpsz_tc_no = " ";
	int tele_num = 1;

	CDecimal d_total_tel_num = 0;
	CDecimal v_count = 0;
	CDecimal plan_charge_num = 0;
	CString v_bof_no = " ";
	CString v_sg_sign = " ";
	CString v_ladle_level = " ";
	CString factory_div = " ";//分区标志
	CString oper_flag = " ";//操作区分标志
	CDecimal max_num = 15; //电文循环数

	EPEX epex(&s, conn, 1);

	CTPSSMD1 tpssmd1(conn);
	CTPSSM10 tpssm10(conn);
	CTPSSM01 tpssm01(conn);
	CTPSSMDB tpssmdb(conn);
	CTPSSM12 tpssm12(conn);
	CTPSSM99 tpssm99(conn);

	CDbCommand cmd_tpssmdb_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_inq(conn);
	EIClass inblk;        //调用函数用
	EIClass in_pssm99trace;//调用履历函数

	try
	{
		factory_div = bcls_rec->Tables["TQ"].Rows[0]["FACTORY_DIV"].ToString();
		oper_flag = bcls_rec->Tables["TQ"].Rows[0]["OPER_FLAG"].ToString();
		tpssm01.PONO = bcls_rec->Tables["TQ"].Rows[0]["PONO"].ToString();
		tpssm01.PLAN_TAP_WT = bcls_rec->Tables["TQ"].Rows[0]["PLAN_TAP_WT"];
		tpssm01.PLAN_DATE = bcls_rec->Tables["TQ"].Rows[0]["PLAN_DATE"].ToString();
		
		Log::Trace("", __FUNCTION__, "传入factory_div=[{0}]", factory_div);
		Log::Trace("", __FUNCTION__, "传入OPER_FLAG=[{0}]", oper_flag);

		lpsz_tc_no = "PAIMP1";/*一区赋电文号*/


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT COUNT(1) FROM TPSSMDB \
					 					 					 WHERE  FACTORY_DIV = @factory_div \
															 										 					  ";
			break;
		}

		cmd_tpssmdb_inq.SetCommandText(sqlstr);
		cmd_tpssmdb_inq.Parameters.Set("factory_div", factory_div);
		v_count = cmd_tpssmdb_inq.ExecuteScalar();
		cmd_tpssmdb_inq.Close();
		Log::Trace("", __FUNCTION__, "转炉区sqlstr[{0}]", sqlstr);
		Log::Trace("", __FUNCTION__, "转炉区当前计划数v_count[{0}]", v_count.ToInt32());
		if (v_count != 0)
		{
			switch (conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
				sqlstr = " SELECT  IRON_WT  FROM TPSSMDB \
					 				 WHERE  FACTORY_DIV = @factory_div \
									 ";
			
			  break;
			}

			cmd_tpssmdb_inq.SetCommandText(sqlstr);
			cmd_tpssmdb_inq.Parameters.Set("factory_div", factory_div);
			cmd_tpssmdb_inq.ExecuteReader();
			while (cmd_tpssmdb_inq.Read())
			{
				tpssmdb.IRON_WT = cmd_tpssmdb_inq.GetDecimal(1);
				Log::Trace("", __FUNCTION__, "取值tpssmdb.IRON_WT=[{0}]", tpssmdb.IRON_WT);
			}
			cmd_tpssmdb_inq.Close();


			//初始化
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				Log::Trace("", __FUNCTION__, "初始化出错.");
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//操作区分
			if (epex.SetValue("handle_div", 0, oper_flag.Trim()) < 0)
			{
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				Log::Trace("", __FUNCTION__, "初始化oper_flag出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//厂别区分
			if (epex.SetValue("factory_div", 0, factory_div.Trim()) < 0)
			{
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				Log::Trace("", __FUNCTION__, "初始化factory_div出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//PONO
			if (epex.SetValue("pono", 0, tpssm01.PONO.Trim()) < 0)
			{
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				Log::Trace("", __FUNCTION__, "初始化PONO出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//计划炉数
			if (epex.SetValue("heat_num", 0, 1) < 0)
			{
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				Log::Trace("", __FUNCTION__, "初始化heat_num出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//计划日期
			if (epex.SetValue("plan_date", 0, tpssm01.PLAN_DATE) < 0)
			{
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				Log::Trace("", __FUNCTION__, "初始化PLAN_DATE出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//计划钢水量
			if (epex.SetValue("plan_tap_wt", 0, tpssm01.PLAN_TAP_WT) < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//计划钢水量
			if (epex.SetValue("plan_iron_wt", 0, tpssmdb.IRON_WT) < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm01.FACTORY_DIV = factory_div.Trim();
			tpssm01.Query("FACTORY_DIV,PONO");
			//班次--铁前说需要才加的，信息来源于MMS编制预计划的时候人工选择，若果人工不选，则不会有这个值，这个值目前不是必选项
			if (epex.SetValue("prod_shift_no", 0, tpssm01.SHIFT_NO) < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}


			//发送电文
			if (epex.SendTele() < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			// 释放
			epex.Uninitialize();
			Log::Trace("", __FUNCTION__, "计划成功= [{0}]", tele_num);

		

		}
		else
		{
			CFormattable arguments[] = { factory_div }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "请先在PSSMDB画面维护本区域每炉所需铁水量。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
			Log::Trace("", __FUNCTION__, "CCCCCCC 000000 send!");

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

