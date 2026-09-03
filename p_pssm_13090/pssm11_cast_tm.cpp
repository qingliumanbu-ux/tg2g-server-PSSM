/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    zhp
Version:    3.1.0
Date:     2016-11-11
Description: 出钢计划浇次时刻调整。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"






int f_pssm11_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm12_time_calc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm12_sequ_calc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 出钢计划浇次时刻调整
/// <para>根据前台传入的pono、时刻，更新各炉次计划的时刻。    </para>
/// <para>浇次计划时刻更新的范围有：1-单炉次时间修改; 2-CAST内; 3-后续所有炉次；</para>
/// <para>前工序时间调整范围有：0-不调整；1-连铸前工序都调整。</para>
/// <para>工序顺序号调整范围有：0-不调整；1-做调整；          </para>
/// <para>浇次计划更新后，根据传入参数，决定分别调用：        </para>
/// <para>1)f_pssm11_pour_time()--浇铸时刻计算。              </para>
/// <para>2)f_pssm12_time_calc()--工序时刻计算。              </para>
/// <para>数据库表：TPSSM11(炼钢出钢计划主表)                 </para>
/// <para>主调用函数：前台FormPSSM11CastDlg画面确定按钮调用。 </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <param name="pono">制造命令号                    </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_cast_tm)

int f_pssm11_cast_tm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	EIClass inBlock;
	EIClass outBlock;

	//程序用变量
	int doFlag = 0;
	CString dev_code = "";
	CString dev_name = "";
	CDecimal area_id = 0;
	CString factory_div = "";
	CString cc_mach_no = "";
	CDecimal heat_scope = 0;
	CDecimal time_upd = 0;
	CDecimal proc_no_upd = 0;

	int i = 0;
	int ret = 0;
	int rows = 0;

	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm26("TPSSM26");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm11_upd(conn);

	try
	{
		inBlock.Tables[0].set_TableName("POUR");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CC_REQ_TIME");

		inBlock.Tables.Add("PONO");
		inBlock.Tables[1].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[1].Columns.Add(DT_STRING, "PONO");

		factory_div = bcls_rec->Tables["COND"].Rows[0]["FACTORY_DIV"].ToString().Trim();
		cc_mach_no	= bcls_rec->Tables["COND"].Rows[0]["CC_MACH_NO"].ToString().Trim();
		heat_scope = bcls_rec->Tables["COND"].Rows[0]["HEAT_SCOPE"].ToDecimal();
		time_upd = bcls_rec->Tables["COND"].Rows[0]["TIME_UPD"].ToDecimal();
		proc_no_upd = bcls_rec->Tables["COND"].Rows[0]["PROC_NO_UPD"].ToDecimal();
		////Log::Info("", __FUNCTION__, "heat_scope=[{0}]", heat_scope);
		rows = bcls_rec->Tables["CAST"].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			//获取前台传入参数
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables["CAST"].Rows[i]["FACTORY_DIV"].ToString().Trim();
			tpssm11["SM_PLAN_NO"]	= bcls_rec->Tables["CAST"].Rows[i]["SM_PLAN_NO"].ToString().Trim();
			tpssm11["PONO"]		= bcls_rec->Tables["CAST"].Rows[i]["PONO"].ToString().Trim();

			if (tpssm11.Query("FACTORY_DIV, PONO") == false)
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]已不在计划中,请重新调整!", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm11["CC_REQ_TIME"] = bcls_rec->Tables["CAST"].Rows[i]["CC_REQ_TIME"].ToString().Trim();

			if (tpssm11["CC_REQ_TIME"].ToString().Trim() == "")
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]的开浇时刻为空!", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (heat_scope == 1) //1-单炉次时间修改;
			{
				tpssm11["PLAN_EDIT_FLAG"] = "U";
				tpssm11.Update("CC_REQ_TIME,PLAN_EDIT_FLAG","FACTORY_DIV,SM_PLAN_NO");
			}
			else if (heat_scope == 2) // 2-CAST内; 
			{
				tpssm11["PLAN_EDIT_FLAG"] = "U";
				tpssm11.Update("CC_REQ_TIME,PLAN_EDIT_FLAG", "FACTORY_DIV,SM_PLAN_NO");

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" UPDATE TPSSM11 "
						" SET CC_REQ_TIME = ' ', "
						"	PLAN_EDIT_FLAG = 'U' "
						" WHERE FACTORY_DIV = @factory_div "
						"	AND CAST_NO = @cast_no "
						"	AND CAST_DIV_NO	> @cast_div_no "
						);
					break;
				}

				cmd_tpssm11_upd.SetCommandText(sqlstr);
				////Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
				cmd_tpssm11_upd.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm11_upd.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
				cmd_tpssm11_upd.Parameters.Set("cast_div_no", tpssm11["CAST_DIV_NO"].ToDecimal());
				cmd_tpssm11_upd.ExecuteNonQuery();
			}
			else if (heat_scope == 3) // 3-后续所有炉次
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" UPDATE TPSSM11 "
						" SET CC_REQ_TIME = ' ', "
						"	PLAN_EDIT_FLAG = 'U' "
						" WHERE FACTORY_DIV = @factory_div "
						"	AND CC_MACH_NO = @cc_mach_no "
						"	AND ((CAST_NO = @cast_no "
						"		AND  CAST_DIV_NO >=  @cast_div_no) "
						"	OR CAST_NO  > @cast_no) "
						);
					break;
				}

				cmd_tpssm11_upd.SetCommandText(sqlstr);
				////Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
				cmd_tpssm11_upd.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm11_upd.Parameters.Set("cc_mach_no", tpssm11["CC_MACH_NO"].ToString());
				cmd_tpssm11_upd.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
				cmd_tpssm11_upd.Parameters.Set("cast_div_no", tpssm11["CAST_DIV_NO"].ToDecimal());
				cmd_tpssm11_upd.ExecuteNonQuery();

				tpssm11["PLAN_EDIT_FLAG"] = "U";
				tpssm11.Update("CC_REQ_TIME, PLAN_EDIT_FLAG", "FACTORY_DIV, SM_PLAN_NO");
			}		
		}


		//------------------------------------------------------
		// 写入连铸机
		CDataRow & row = inBlock.Tables["POUR"].Rows.Add();
		row["FACTORY_DIV"] = factory_div;
		row["CC_MACH_NO"] = cc_mach_no;
		row["CC_REQ_TIME"] = " ";

		//1.浇铸时刻计算, 输入参数:修改PONO
		ret = f_pssm11_pour_time(&inBlock, &outBlock, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//2.前工序时刻调整标记: 1-计算前工序的作业时刻
		if (time_upd == 1)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库k
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句

				sqlstr = " SELECT PONO "
					" FROM TPSSM11 "
					" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					" AND PLAN_EDIT_FLAG = 'U' "
					" AND CC_MACH_NO = @tpssm11.CC_MACH_NO ";
				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();

			while (cmd_tpssm11_inq.Read())
			{
				tpssm11["PONO"] = cmd_tpssm11_inq.GetString(1);

				CDataRow & row = inBlock.Tables["PONO"].Rows.Add();
				row["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				row["PONO"] = tpssm11["PONO"];
			}
			cmd_tpssm11_inq.Close();

			//调用函数
			ret = f_pssm12_time_calc(&inBlock, &outBlock, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		tpssm11["PLAN_EDIT_FLAG"] = " ";
		tpssm11.Update("PLAN_EDIT_FLAG", "FACTORY_DIV");
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
