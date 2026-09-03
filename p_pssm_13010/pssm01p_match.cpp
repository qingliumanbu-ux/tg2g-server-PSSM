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
int f_pssm99_trace_l4(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 编制炼钢计划
/// <para>
/// 1.校验选择的连铸机是否合理
/// 2.cc_seq 赋值
/// 3.调用PM函数
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm01p_match)
//-EP_SYSTEM_HEAD_END

int f_pssm01p_match(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssmd9("TPSSMD9");
	CModel tpsbwa1("TPSBWA1");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm03_upd(conn);
	CDbCommand cmd_tep0002_inq(conn);
	CDbCommand cmd_tpsbwa4_inq(conn);

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
		inBlock3.Tables[0].Columns.Add(DT_STRING, "REMARK");

		//------------------------------------------------------------------------------------------
		//获得输入参数
		tpsbwa1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"].ToString().TrimOrBlank();
		tpsbwa1["ROLL_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["ROLL_PLAN_NO"].ToString().TrimOrBlank();

		//------------------------------------------------------------------------------------------
		//打印输入参数
		Log::Trace("", __FUNCTION__, "PLAN_BACKLOG_CODE	= [{0}]", tpsbwa1["PLAN_BACKLOG_CODE"].ToString());
		Log::Trace("", __FUNCTION__, "ROLL_PLAN_NO		= [{0}]", tpsbwa1["ROLL_PLAN_NO"].ToString());

		//校验计划工序代码
		if (tpsbwa1["PLAN_BACKLOG_CODE"].ToString().Trim() == "")
		{
			strcpy(s.msg, "请选择机组！！.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//校验棒线预计划
		if (tpsbwa1["ROLL_PLAN_NO"].ToString().Trim() == "")
		{
			strcpy(s.msg, "请选择预计划！！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpsbwa1.Query("ROLL_PLAN_NO") == false)
		{
			CFormattable arguments[] = { tpsbwa1["ROLL_PLAN_NO"].ToString() };// 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PS00S0000025")/*计划[{0}]不存在，不能执行当前操作。*/, arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		rows = bcls_rec->Tables[1].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			tpssm01["FACTORY_DIV"] = bcls_rec->Tables[1].Rows[i]["FACTORY_DIV"].ToString().TrimOrBlank();
			tpssm01["CC_MACH_NO"]	= bcls_rec->Tables[1].Rows[i]["CC_MACH_NO"].ToString().TrimOrBlank();
			tpssm01["PONO"]		= bcls_rec->Tables[1].Rows[i]["PONO"].ToString().TrimOrBlank();

			Log::Trace("", __FUNCTION__, "FACTORY_DIV		= [{0}]", tpssm01["FACTORY_DIV"].ToString());
			Log::Trace("", __FUNCTION__, "PONO				= [{0}]", tpssm01["PONO"].ToString());
			Log::Trace("", __FUNCTION__, "CC_MACH_NO		= [{0}]", tpssm01["CC_MACH_NO"].ToString());
			
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT * FROM TPSSM01 "
					" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
					"   AND  PONO = @tpssm01.PONO "
					);
				break;
			}

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm01_inq.ExecuteReader();

			if (cmd_tpssm01_inq.Read())
			{
				cmd_tpssm01_inq.Fetch(tpssm01);
				tpssm01.TrimOrBlank();

//				if (tpssm01["SLAB_DEST"].ToString() == "11")
//				{
//					/*if (tpssm01["CC_MACH_NO"].ToString() != "4")
//					{
//						CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["SLAB_DEST"].ToString() };
//						CMessageFormat::Format(s.msg, "制造命令[{0}]去向[{1}]，只能选择4#连铸机。", arguments, 2);
//						throw CApplicationException(-1, s.msg, log.Location);
//					}
//*/
//					if (tpsbwa1["PLAN_BACKLOG_CODE"].ToString() != "L101")
//					{
//						CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["SLAB_DEST"].ToString() };
//						CMessageFormat::Format(s.msg, "制造命令[{0}]去向为[{1}]，只能选择双高棒预计划。", arguments, 2);
//						throw CApplicationException(-1, s.msg, log.Location);
//					}
//				}
//				else if (tpssm01["SLAB_DEST"].ToString() == "12")
//				{
//					/*if (tpssm01["CC_MACH_NO"].ToString() != "1" && tpssm01["CC_MACH_NO"].ToString() != "2")
//					{
//						CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["SLAB_DEST"].ToString() };
//						CMessageFormat::Format(s.msg, "制造命令[{0}]去向[{1}]，只能选择1#、2#连铸机。", arguments, 2);
//						throw CApplicationException(-1, s.msg, log.Location);
//					}*/
//
//					if (tpsbwa1["PLAN_BACKLOG_CODE"].ToString() != "L201")
//					{
//						CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["SLAB_DEST"].ToString() };
//						CMessageFormat::Format(s.msg, "制造命令[{0}]去向为[{1}]，只能选择普棒预计划。", arguments, 2);
//						throw CApplicationException(-1, s.msg, log.Location);
//					}
//				}
//				else if (tpssm01["SLAB_DEST"].ToString() == "13")
//				{
//					//if (tpssm01["CC_MACH_NO"].ToString() != "4")
//					//{
//					//	CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["SLAB_DEST"].ToString() };
//					//	CMessageFormat::Format(s.msg, "制造命令[{0}]去向[{1}]，只能选择4#连铸机。", arguments, 2);
//					//	throw CApplicationException(-1, s.msg, log.Location);
//					//}
//
//					if (tpsbwa1["PLAN_BACKLOG_CODE"].ToString() != "W101")
//					{
//						CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["SLAB_DEST"].ToString() };
//						CMessageFormat::Format(s.msg, "制造命令[{0}]去向为[{1}]，只能选择线材预计划。", arguments, 2);
//						throw CApplicationException(-1, s.msg, log.Location);
//					}
//				}

				//----------------------------------------
				//条件校验1-炉次钢种需和预计划钢种匹配
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr =
						//" select count(*) from TPSBWA4 where roll_plan_no=@tpsbwa1.ROLL_PLAN_NO and st_no=@tpssm01.ST_NO	"
						" select count(*) from TPSBWA1 where roll_plan_no=@tpsbwa1.ROLL_PLAN_NO and st_no=@tpssm01.ST_NO	"
						;
					break;
				}
				cmd_tpsbwa4_inq.SetCommandText(sqlstr);
				cmd_tpsbwa4_inq.Parameters.Set("tpsbwa1.ROLL_PLAN_NO", tpsbwa1["ROLL_PLAN_NO"].ToString());
				cmd_tpsbwa4_inq.Parameters.Set("tpssm01.ST_NO", tpssm01["ST_NO"].ToString());
				int v_count = cmd_tpsbwa4_inq.ExecuteScalar().ToInt32();
				if (v_count <= 0)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString(), tpssm01["ST_NO"].ToString(), tpsbwa1["ROLL_PLAN_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "PONO[{0}]的钢种[{1}]和预计划可配钢种不匹配，不允许进行预配料！！！", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//------------------------
				tpssm03["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm03["PONO"] = tpssm01["PONO"];

				//更新修改者，修改时间
				tpssm03["REC_REVISOR"] = CString(s.userid);
				tpssm03["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tpssm03["PREC_ROLL_PLAN_NO"] = tpsbwa1["ROLL_PLAN_NO"];
				tpssm03["PREC_ROLL_SEQ_NO"] = tpsbwa1["PLAN_EXEC_SEQ_NO"];

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = " UPDATE TPSSM03  "
						"  SET REC_REVISE_TIME = @tpssm03.REC_REVISE_TIME, "
						"  REC_REVISOR = @tpssm03.REC_REVISOR, "
						"  PREC_ROLL_PLAN_NO = @tpssm03.PREC_ROLL_PLAN_NO, "
						"  PREC_ROLL_SEQ_NO = @tpssm03.PREC_ROLL_SEQ_NO "
						"  WHERE FACTORY_DIV =  @tpssm03.FACTORY_DIV "
						"    AND PONO = @tpssm03.PONO "
						;
					break;
				}
				Log::Trace("", __FUNCTION__, "-- sqlstr=[{0}]--- ", sqlstr);
				cmd_tpssm03_upd.SetCommandText(sqlstr);
				cmd_tpssm03_upd.Parameters.Set("tpssm03.REC_REVISE_TIME", tpssm03["REC_REVISE_TIME"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm03.REC_REVISOR", tpssm03["REC_REVISOR"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm03.PREC_ROLL_PLAN_NO", tpssm03["PREC_ROLL_PLAN_NO"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm03.PREC_ROLL_SEQ_NO", tpssm03["PREC_ROLL_SEQ_NO"].ToDecimal());
				cmd_tpssm03_upd.Parameters.Set("tpssm03.FACTORY_DIV", tpssm03["FACTORY_DIV"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm03.PONO", tpssm03["PONO"].ToString());
				cmd_tpssm03_upd.ExecuteNonQuery();

				//炼钢履历跟踪
				CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
				row3["EVENT_ID"] = "P1"; //预配料
				row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row3["PONO"] = tpssm01["PONO"];
				row3["PONO_STATUS"] = tpssm01["PONO_STATUS"];
				row3["REMARK"] = "预配料计划号：" + tpsbwa1["ROLL_PLAN_NO"].ToString();
			}
			cmd_tpssm01_inq.Close();
		}//for

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace_l4(&inBlock3, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		/*设置系统返回参数*/
		strcpy(s.msg, _RES("GCRSS0000002")/*处理成功。*/);
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
