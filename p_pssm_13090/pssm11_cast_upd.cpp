/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    zhp
Version:    3.1.0
Date:     2016-11-11
Description: 出钢计划浇次调整。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"






/*<remark>=========================================================
/// <summary>
/// 出钢计划浇次调整
/// <para>根据前台连铸计划顺序和浇铸标记，更新浇次信息。      </para>
/// <para>1.确定当前连铸机及该连铸机的CAST号。                </para>
/// <para>2.根据前台传入的PONO顺序，为每一炉计划生成CAST号。  </para>
/// <para>2.将cast_no, cast_div_no更新入计划中。              </para>
/// <para>数据库表：TPSSM11(炼钢出钢计划主表)                 </para>
/// <para>主调用函数：前台FormPSSM11CastDlg画面确定按钮调用。 </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <param name="pono">制造命令号                    </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_cast_upd)

int f_pssm11_cast_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	CString datetime = "";
	CString factory_div = "";
	CString cc_mach_no = "";
	CString cast_no = "";
	CDecimal cast_stream_no = 0;
	CDecimal cast_div_no = 0;
	CDecimal cast_pono_sum = 0;
	int i = 0;
	int rows = 0;
	int dummy = 0;
	CString v_restrand_flg = "";

	CModel tpssm03("TPSSM03");
	CModel tpssm26("TPSSM26");
	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm11_inq2(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName("CAST");  //与Client端dS_PSSM11ProcNo.TPSSM25表保持一致
		bcls_ret->Tables[0].Columns.Add(tpssm11);
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_NO_SHOW");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_THICK");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH_1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH_2");

		factory_div = bcls_rec->Tables["DEV"].Rows[0]["FACTORY_DIV"].ToString().Trim();
		cc_mach_no = bcls_rec->Tables["DEV"].Rows[0]["CC_MACH_NO"].ToString().Trim();

		sqlstr = CString(
			"SELECT CAST_NO, CAST_DIV_NO "
			"  FROM TPSSM26 "
			"  WHERE FACTORY_DIV = @FACTORY_DIV "
			"	 AND CC_MACH_NO = @CC_MACH_NO  "
			);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_inq.Parameters.Set("CC_MACH_NO", cc_mach_no);

		cmd_inq.ExecuteReader();

		if (cmd_inq.Read())
		{
			tpssm26["CAST_NO"] = cmd_inq.GetString(1);
			tpssm26["CAST_DIV_NO"] = cmd_inq.GetDecimal(2);
		}
		cmd_inq.Close();

		cast_no = tpssm26["CAST_NO"];
		cast_div_no = tpssm26["CAST_DIV_NO"];

		rows = bcls_rec->Tables["CAST"].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm11["FACTORY_DIV"]		= bcls_rec->Tables["CAST"].Rows[i]["FACTORY_DIV"].ToString().Trim();
			tpssm11["SM_PLAN_NO"]		= bcls_rec->Tables["CAST"].Rows[i]["SM_PLAN_NO"].ToString().Trim();
			tpssm11["PONO"]			= bcls_rec->Tables["CAST"].Rows[i]["PONO"].ToString().Trim();
			v_restrand_flg			= bcls_rec->Tables["CAST"].Rows[i]["RESTRAND_FLG"].ToString().Trim();

			////Log::Info("", __FUNCTION__, "tpssm11["PONO"] =[{0}], tpssm11["RESTRAND_FLG"] =[{1}]", tpssm11["PONO"].ToString(), v_restrand_flg);

			if (tpssm11.Query("FACTORY_DIV, PONO") == false)
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]已不在计划中,请重新调整!", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			////Log::Info("", __FUNCTION__, "tpssm11["RUN_STATUS"] =[{0}]", tpssm11["RUN_STATUS"].ToString());
			if (tpssm11["RUN_STATUS"].ToString() >= "52")
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]已经开浇，请重新查询确认!", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm11["RESTRAND_FLG"] = v_restrand_flg;
			////Log::Info("", __FUNCTION__, "tpssm11["RESTRAND_FLG"] =[{0}]", tpssm11["RESTRAND_FLG"].ToString());
			if (tpssm11["RESTRAND_FLG"].ToString() == "T") //炉次重开浇标识
			{
				cast_stream_no = cast_stream_no.Parse(cast_no.Substring(2));//流水号--在前面扩1位后从第三位开始取，原本cast_no.Substring(1)
				cast_stream_no = cast_stream_no + 1;
				cast_no = cast_no.Format("%s%.4d", (const char*)cast_no.SubstringNE(0, 2), cast_stream_no.ToInt32());
				cast_div_no = 1;
			}
			else //不重引锭--连浇, cast_no不变
			{
				tpssm11["RESTRAND_FLG"] = " ";
				cast_div_no = cast_div_no + 1;
			}

			tpssm11["CAST_NO"] = cast_no;
			tpssm11["CAST_DIV_NO"] = cast_div_no;

			////Log::Info("", __FUNCTION__, "tpssm11["CAST_NO"] =[{0}], tpssm11["CAST_DIV_NO"] =[{1}]", tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal());
			tpssm11.Update("CAST_NO, CAST_DIV_NO, RESTRAND_FLG","FACTORY_DIV, PONO");
		}

		//读取TPSSM11表记录，按cast_no汇总
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString("SELECT CAST_NO,MAX(CAST_DIV_NO) FROM TPSSM11 WHERE FACTORY_DIV = @FACTORY_DIV AND CC_MACH_NO = @CC_MACH_NO AND RUN_STATUS < '52' GROUP BY CAST_NO ");
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_tpssm11_inq.Parameters.Set("CC_MACH_NO", cc_mach_no);
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			tpssm11["CAST_NO"] = cmd_tpssm11_inq.GetString(1);
			cast_pono_sum = cmd_tpssm11_inq.GetDecimal(2);

			/*dummy = tpssm11.QueryCount("FACTORY_DIV,CAST_NO");
			////Log::Trace("", __FUNCTION__, "tpssm11.QueryCount(CAST_NO)=[{0}]", dummy);*/

			//更新CAST内炉数
			tpssm11["CAST_PONO_SUM"] = cast_pono_sum;
			tpssm11["FACTORY_DIV"] = factory_div;
			sqlstr = "tpssm11.Update(CAST_PONO_SUM)";
			tpssm11.Update("CAST_PONO_SUM", "FACTORY_DIV,CAST_NO");

			//tpssm41["CAST_PONO_SUM"] = cast_pono_sum;
			//tpssm41["FACTORY_DIV"] = factory_div;
			//tpssm41["CAST_NO"] = tpssm11["CAST_NO"];
			//tpssm41.Update("CAST_PONO_SUM", "FACTORY_DIV,CAST_NO");
		}
		cmd_tpssm11_inq.Close();
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
