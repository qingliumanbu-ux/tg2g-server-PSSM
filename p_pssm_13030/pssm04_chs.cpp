/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-25
Description: 编制炼钢计划
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件






/* ***** 静态函数申明 ***** */
int f_pmom_status_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm21_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //日出钢能力统计
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪
int f_pssm01_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 编制炼钢计划
/// <para>
/// 1.校验选择的连铸机是否合理
/// 2.cc_seq 赋值
/// 3.调用PM函数
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)         </para>
/// <para>主调用函数：前台PSSM01画面F3 计划编制调用 </para>
/// </summary>
/// <param name="cc_mach_no">连铸机    </param>
/// <param name="plan_date">计划日期   </param>
/// <param name="shift_group">班组     </param>
/// <param name="shift_no">班次        </param>
/// <returns>编入计划的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm04_chs)
//-EP_SYSTEM_HEAD_END

int f_pssm04_chs(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret;
	int dummy = 0;
	int cc_seq = 0;
	CString datetime = CDateTime::Now().ToString("yyyyMMddhhmmss");

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息
	CString v_cast_lot_no = "";
	CString sqlstr;
	CString v_billet_type = "";
	CString v_ic = "";
	CString v_cc_mach_no = "";
	int cast_lot_div_no = 0;
	int v_lack_per = 0;
	int v_count = 0;

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssmd8("TPSSMD8");
	CModel tpssmd9("TPSSMD9");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm01_inq_1(conn);
	CDbCommand cmd_tep0002_inq(conn);
	CDbCommand cmd_tpssmd8_inq(conn);
	CDbCommand cmd_tpssmd9_inq(conn);

	EIClass inBlock;  //调用连铸机校验函数接口
	EIClass inBlock1;
	EIClass inBlock2; //调用日出钢统计
	EIClass inBlock3; //调用炼钢履历跟踪
	EIClass outBlock;


	try
	{
		//----------------------------------------------
		//声明调用连铸机校验的接口参数
		inBlock.Tables[0].set_TableName("PSSM01");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");

		//声明调用生产的接口参数
		inBlock.Tables.Add("PSSMCHS");
		inBlock.Tables["PSSMCHS"].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables["PSSMCHS"].Columns.Add(DT_STRING, "PONO");

		//计算浇注时间
		inBlock1.Tables[0].set_TableName("POURTIME");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		inBlock1.Tables[0].Rows.Add();

		//调用日出钢统计
		inBlock2.Tables[0].set_TableName("PSSM21");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PLAN_DATE");
		inBlock2.Tables[0].Rows.Add();

		//调用炼钢履历跟踪
		inBlock3.Tables[0].set_TableName("TRACE");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码


		//------------------------------------------------------------------------------------------
		//获得输入参数
		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		tpssm01["CC_MACH_NO"] = bcls_rec->Tables[1].Rows[0]["CC_MACH_NO"].ToString().TrimOrBlank();
		tpssm01["PLAN_DATE"] = bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString().TrimOrBlank();

		//------------------------------------------------------------------------------------------
		//打印输入参数
		////Log::Trace("", __FUNCTION__, "FACTORY_DIV	= [{0}]", tpssm01["FACTORY_DIV"].ToString());
		////Log::Trace("", __FUNCTION__, "CC_MACH_NO		= [{0}]", tpssm01["CC_MACH_NO"].ToString());
		////Log::Trace("", __FUNCTION__, "PLAN_DATE		= [{0}]", tpssm01["PLAN_DATE"].ToString());

		//校验连铸机号是否为空
		if (tpssm01["CC_MACH_NO"].ToString().Trim() == "")
		{
			strcpy(s.msg, "请选择浇铸设备.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//校验连铸机号是否为空
		if (tpssm01["PLAN_DATE"].ToString().Trim() == "")
		{
			strcpy(s.msg, "请选择生产日期.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------
		//找到指定连铸、日期内编入计划的CC_SEQ最大值
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = CString(" SELECT MAX(CC_SEQ) FROM TPSSM01 "
				" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
				" AND    CC_MACH_NO  = @tpssm01.CC_MACH_NO "
				" AND 	  PLAN_DATE   = @tpssm01.PLAN_DATE ");
			break;
		}

		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PLAN_DATE", tpssm01["PLAN_DATE"].ToString());
		cmd_tpssm01_inq.ExecuteReader();

		if (cmd_tpssm01_inq.Read())
		{
			cc_seq = cmd_tpssm01_inq.GetInt32(1);
		}
		else
		{
			cc_seq = 0;
		}
		cmd_tpssm01_inq.Close();
		////Log::Trace("", __FUNCTION__, "MAX CC_SEQ  = [{0}]", cc_seq);

		v_cc_mach_no = tpssm01["CC_MACH_NO"];
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString().TrimOrBlank();
			tpssm01["CAST_LOT_NO"] = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO"].ToString().TrimOrBlank();

			////Log::Trace("", __FUNCTION__, "CAST_LOT_NO		= [{0}]", tpssm01["CAST_LOT_NO"].ToString());

			//如果LOT相同，该LOT下的PONO都编入计划，跳过处理
			if (tpssm01["CAST_LOT_NO"].ToString() == v_cast_lot_no)
			{
				continue;
			}

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT MIN(CAST_LOT_SUM), NVL(MIN(CAST_LOT_DIV_NO), 0) "
					"FROM TPSSM01 "
					"WHERE FACTORY_DIV = @FACTORY_DIV "
					"AND CAST_LOT_NO = @CAST_LOT_NO "
					"AND PONO_STATUS = 11 ";
				break;
			}
			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm01_inq.ExecuteReader();

			if (cmd_tpssm01_inq.Read())
			{
				tpssm01["CAST_LOT_SUM"] = cmd_tpssm01_inq.GetInt32(1);
				cast_lot_div_no = cmd_tpssm01_inq.GetInt32(2);
			}
			cmd_tpssm01_inq.Close();

			//查询LOT下的PONO
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT * FROM TPSSM01 "
					" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
					"   AND  CAST_LOT_NO = @tpssm01.CAST_LOT_NO "
					"	AND PONO_STATUS = 11 "
					" ORDER BY CAST_LOT_DIV_NO ASC "
					);
				break;
			}

			cmd_tpssm01_inq_1.SetCommandText(sqlstr);
			cmd_tpssm01_inq_1.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq_1.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm01_inq_1.ExecuteReader();

			while (cmd_tpssm01_inq_1.Read())
			{
				cmd_tpssm01_inq_1.Fetch(tpssm01);
				tpssm01.TrimOrBlank();

				////连铸机校验
				////铸坯类型，断面规格校验			
				//sqlstr = "SELECT COUNT(1) FROM TPSSMD8 WHERE FACTORY_DIV = @FACTORY_DIV "
				//	" AND CC_MACH_NO = @CC_MACH_NO "
				//	" AND CC_TYPE = @CC_TYPE ";

				//cmd_tpssmd8_inq.SetCommandText(sqlstr);
				//cmd_tpssmd8_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				//cmd_tpssmd8_inq.Parameters.Set("CC_MACH_NO", v_cc_mach_no);
				//cmd_tpssmd8_inq.Parameters.Set("CC_TYPE", tpssm01["CC_TYPE"].ToString());
				//cmd_tpssmd8_inq.ExecuteReader();

				//if (cmd_tpssmd8_inq.Read())
				//{
				//	v_count = cmd_tpssmd8_inq.GetInt32(1);
				//}
				//cmd_tpssmd8_inq.Close();

				//if (v_count == 0)
				//{
				//	CFormattable arguments[] = { v_cc_mach_no, tpssm01["CC_TYPE"].ToString() };
				//	CMessageFormat::Format(s.msg, "该铸机[{0}]下无法浇注连铸机类型[{1}]的钢坯！", arguments, 2);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}
				
				//如果生产用了连铸机类型代码，此段可以不校验
				sqlstr = "SELECT DISTINCT SLAB_THICK,SLAB_WIDTH,BILLET_TYPE FROM TPSSM03 WHERE FACTORY_DIV = @FACTORY_DIV "
					" AND PONO = @PONO ";

				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm03_inq.Parameters.Set("PONO", tpssm01["PONO"].ToString());
				cmd_tpssm03_inq.ExecuteReader();
				while (cmd_tpssm03_inq.Read())
				{
					tpssm03["SLAB_THICK"] = cmd_tpssm03_inq.GetDecimal(1);
					tpssm03["SLAB_WIDTH"] = cmd_tpssm03_inq.GetDecimal(2);
					v_billet_type = cmd_tpssm03_inq.GetString(3);

					//20160919 周平，暂时注释
					//连铸机校验
					//铸坯类型，断面规格校验			
					sqlstr = "SELECT * FROM TPSSMD9 WHERE FACTORY_DIV = @FACTORY_DIV "
						" AND CC_MACH_NO = @CC_MACH_NO "
						" AND BILLET_TYPE = @BILLET_TYPE "
						" AND CAST_THICK = @CAST_THICK ";

					cmd_tpssmd9_inq.SetCommandText(sqlstr);
					cmd_tpssmd9_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
					cmd_tpssmd9_inq.Parameters.Set("CC_MACH_NO", v_cc_mach_no);
					cmd_tpssmd9_inq.Parameters.Set("BILLET_TYPE", v_billet_type);
					cmd_tpssmd9_inq.Parameters.Set("CAST_THICK", tpssm03["SLAB_THICK"].ToDecimal());
					cmd_tpssmd9_inq.ExecuteReader();
					if (cmd_tpssmd9_inq.Read())
					{
						tpssmd9.Reset();
						cmd_tpssmd9_inq.Fetch(tpssmd9);
						tpssmd9.TrimOrBlank();

						if (tpssmd9["BILLET_TYPE"].ToString() != v_billet_type)
						{
							CFormattable arguments[] = { tpssmd9["BILLET_TYPE"].ToString(), v_billet_type };
							CMessageFormat::Format(s.msg, "该铸机的铸坯类型为[{0}],与计划铸坯类型[{1}]不符", arguments, 2);
							throw CApplicationException(-1, s.msg, log.Location);
						}
						if (tpssm03["SLAB_WIDTH"].ToDecimal() < tpssmd9["CAST_WIDTH_MIN"].ToDecimal() || tpssm03["SLAB_WIDTH"].ToDecimal() > tpssmd9["CAST_WIDTH_MAX"].ToDecimal())
						{
							CFormattable arguments[] = { tpssm03["SLAB_WIDTH"].ToDecimal(), tpssmd9["CAST_WIDTH_MIN"].ToDecimal(), tpssmd9["CAST_WIDTH_MAX"].ToDecimal() };
							CMessageFormat::Format(s.msg, "计划宽度[{0}],不在铸机宽度极限[{1}]-[{2}]范围内。", arguments, 3);
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					else
					{
						strcpy(s.msg, "制造命令信息与连铸机参数不符！请确认！");
						throw CApplicationException(-1, s.msg, log.Location);
					}
					cmd_tpssmd9_inq.Close();
				}
				cmd_tpssm03_inq.Close();

				if (tpssm01["CAST_LOT_DIV_NO"].ToDecimal() == cast_lot_div_no)
				{
					tpssm01["RESTRAND_FLG"] = "T";
				}
				else
				{
					tpssm01["RESTRAND_FLG"] = " ";
				}

				//连铸机号使用表1的，HYF修改 0215
				tpssm01["CC_MACH_NO"] = bcls_rec->Tables[1].Rows[0]["CC_MACH_NO"].ToString().TrimOrBlank();
				tpssm01["PLAN_DATE"] = bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString().TrimOrBlank();

				//更新修改者，修改时间
				tpssm01["REC_REVISOR"] = CString(s.userid);
				tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

				tpssm01["PONO_STATUS"] = 13; //计划编制
				tpssm01["CC_SEQ"] = ++cc_seq;

				//-----------------------------------------------
				//计算浇注时间

				inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"]; //厂别区分
				inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"]; //制造命令号
				inBlock1.Tables[0].Rows[0]["CC_MACH_NO"] = tpssm01["CC_MACH_NO"]; //连铸机

				ret = f_pssm01_pour_time(&inBlock1, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tpssm01["POUR_TIME"] = bcls_ret->Tables[0].Rows[i]["POUR_TIME"];

				////Log::Debug("", __FUNCTION__, "编入计划：PONO=[{0}], CC_SEQ  = [{1}]", tpssm01["PONO"].ToString(), tpssm01["CC_SEQ"].ToDecimal());

				v_update = "REC_REVISOR,REC_REVISE_TIME,CC_MACH_NO,PONO_STATUS,PLAN_DATE,POUR_TIME,CC_SEQ,RESTRAND_FLG";//修改字段信息。
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				//更新计划状态
				sqlstr = "tpssm01.Update()";
				if (tpssm01.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "tpssm01 Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//调用生产函数的数据准备
				CDataRow &row = inBlock.Tables["PSSMCHS"].Rows.Add();
				row["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row["PONO"] = tpssm01["PONO"];

				//炼钢履历跟踪
				CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
				row3["EVENT_ID"] = "A1"; //编入计划
				row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row3["PONO"] = tpssm01["PONO"];
				row3["PONO_STATUS"] = tpssm01["PONO_STATUS"];
			}
			cmd_tpssm01_inq_1.Close();

			v_lack_per++;
			////Log::Info("", __FUNCTION__, "v_lack_per++ = [{0}]", v_lack_per);

			tpssm02["LOT_STATUS"] = 2;
			tpssm02["LACK_PER"] = v_lack_per;
			tpssm02["REC_REVISE_TIME"] = datetime;
			tpssm02["REC_REVISOR"] = s.userid;

			tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

			v_update = "REC_REVISOR,REC_REVISE_TIME,LACK_PER, LOT_STATUS";//修改字段信息。
			v_condi = "CAST_LOT_NO"; //查询条件

			if (tpssm02.Update(v_update, v_condi) != true)
			{
				strcpy(s.msg, " tpssm02 Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			v_cast_lot_no = tpssm01["CAST_LOT_NO"];
		}//for

		//-----------------------------------------------
		//调用生产模块函数(材料申请状态置炼钢计划编入)
		ret = f_pmom_status_upd(&inBlock, &outBlock, conn);
		ret = 0;
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-----------------------------------------------
		//调用日出钢能力统计

		inBlock2.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"]; //厂别区分
		inBlock2.Tables[0].Rows[0]["PLAN_DATE"] = tpssm01["PLAN_DATE"]; //计划日期

		//20160728 周平单体测试，临时注释
		//ret = f_pssm21_upd(&inBlock2, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock3, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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

	cmd_tpssm01_inq.Close();

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
