/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-24
Description: 钢坯精整命令自动生成
**************************************************/
#include "stdafx.h"




int f_mmsm99(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 钢坯精整命令自动生成函数
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm81_plan_auto(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount, samprow;
	int blkNum = 0;
	int i = 0;
	int fetchRowCount1 = 0;
	int fetchRowCount2 = 0;
	int rows = 0;

	int v_rownum_pm99 = 0;
	int v_rownum_mm99 = 0;

	CString userid = " ";                  /* 登陆用户 */
	int v_cnt = 0;
	CDecimal v_cnt1 = 0;
	int v_cnt2 = 0;
	CDecimal v_cnt3 = 0;
	int v_mat_num = 0;
	CDecimal v_mat_wt = 0;
	CString v_errmsg = " ";         /* 错误信息 */
	CDecimal v_plan_exec_seq_no_max = 0;      /* 新顺序号 */
	int v_seq = 0;
	CString v_product_code = "";
	CString v_backlog_code = "";
	CString v_plan_no = " "; 				/* 计划号*/
	CString v_factory_div = " ";
	CString v_plan_backlog_code = " ";
	CString v_reser_start_time = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString v_year = datetime.Substring(3, 1);	 /* 年 */
	CString v_month = datetime.Substring(4, 2); /* 月 */
	CString v_day = datetime.Substring(6, 2); /* 日 */
	CDecimal v_mat_seq_no = 0;


	EIClass inBlock;
	EIClass outBlock;

	CModel tpssm81("TPSSM81");
	CModel tmmsm01("TMMSM01");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm81_inq(conn);
	CDbCommand cmd_tqmtjf1_upd(conn);
	CDbCommand cmd_tep0002_inq(conn);

	CString sqlstr;

	CTracer log(__FUNCTION__);
	try
	{
		userid = s.userid;

		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		v_plan_backlog_code = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
		v_reser_start_time = bcls_rec->Tables[0].Rows[0]["PLAN_PROD_TIME"];

		////Log::Trace("", __FUNCTION__, "tpssm81.FACTORY_DIV[{0}],[{1}],[{2}]", v_factory_div, v_plan_backlog_code, v_reser_start_time);

		//机组号4＋年 1 ＋月1 ＋日2 +流水号2（01）
		v_plan_no = tpssm81["PLAN_BACKLOG_CODE"].ToString() + v_year;
		if (v_month == "10")
			v_plan_no = v_plan_no + "A";
		else if (v_month == "11")
			v_plan_no = v_plan_no + "B";
		else if (v_month == "12")
			v_plan_no = v_plan_no + "C";
		else
			v_plan_no = v_plan_no + v_month.Substring(1, 1) + v_day + "01";

		sqlstr = "SELECT CODE_DESC_1_CONTENT FROM TEP0002 WHERE CODE_CLASS = 'PSS0' AND CODE = @v_plan_backlog_code ";

		cmd_tep0002_inq.SetCommandText(sqlstr);
		cmd_tep0002_inq.Parameters.Set("v_plan_backlog_code", v_plan_backlog_code);
		cmd_tep0002_inq.ExecuteReader();

		if (cmd_tep0002_inq.Read())
		{
			v_backlog_code = cmd_tep0002_inq.GetString(1);
		}
		cmd_tep0002_inq.Close();

		if (v_backlog_code.Trim() == "")
		{
			strcpy(s.msg, "计划机组代码对应工序不允许为空！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		tpssm81["FACTORY_DIV"] = v_factory_div;
		tpssm81["PLAN_BACKLOG_CODE"] = v_plan_backlog_code;
		tpssm81["PLAN_MAKE_TIME"] = datetime;
		tpssm81["RESER_START_TIME"] = v_reser_start_time;

		//获取当前工序已释放的计划的最大计划执行顺序号,
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库			
		default: // 所有数据库适用，通用SQL语句
			sqlstr = " SELECT NVL(MAX(PLAN_EXEC_SEQ_NO),0)  FROM TPSSM81 "
				" WHERE FACTORY_DIV = @tpssm81.FACTORY_DIV "
				"   AND PLAN_BACKLOG_CODE = @tpssm81.PLAN_BACKLOG_CODE "
				"   AND PLAN_NO = @v_plan_no ";
			break;
		}
		cmd_tpssm81_inq.SetCommandText(sqlstr);
		cmd_tpssm81_inq.Parameters.Set("tpssm81.FACTORY_DIV", tpssm81["FACTORY_DIV"].ToString());
		cmd_tpssm81_inq.Parameters.Set("tpssm81.PLAN_BACKLOG_CODE", tpssm81["PLAN_BACKLOG_CODE"].ToString());
		cmd_tpssm81_inq.Parameters.Set("v_plan_no", v_plan_no);
		cmd_tpssm81_inq.ExecuteReader();

		if (cmd_tpssm81_inq.Read())
		{
			v_mat_seq_no = cmd_tpssm81_inq.GetDecimal(1);
		}
		cmd_tpssm81_inq.Close();

		//2.  获取第一块输入参数；
		rows = bcls_rec->Tables[0].Rows.get_Count();

		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm81["MAT_NO"] = bcls_rec->Tables[0].Rows[i]["MAT_NO"];
			tpssm81["REPAIR_FLAG"] = bcls_rec->Tables[0].Rows[i]["REPAIR_FLAG"];

			tmmsm01["MAT_NO"] = tpssm81["MAT_NO"];
			if (tmmsm01.Query("MAT_NO") == false)
			{
				CFormattable arguments[] = { tmmsm01["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在主档不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm81.CopyFrom(tmmsm01);

			if (tmmsm01["REPAIR_FLAG"].ToString() == "1")
			{
				if (tmmsm01["MAT_STATUS"].ToString() != "22")
				{
					CFormattable arguments[] = { tmmsm01["MAT_NO"].ToString() };// 定义参数列表的数组
					CMessageFormat::Format(s.msg, "返修材料[{0}]状态必须在22状态。", arguments, 1);
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
			else
			{
				if (tmmsm01["MAT_STATUS"].ToString() != "23")
				{
					CFormattable arguments[] = { tmmsm01["MAT_NO"].ToString() };// 定义参数列表的数组
					CMessageFormat::Format(s.msg, "非返修材料[{0}]状态必须在23状态。", arguments, 1);
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}

			tpssm81["REC_CREATOR"] = s.userid;
			tpssm81["REC_CREATE_TIME"] = datetime;
			tpssm81["REC_REVISOR"] = s.userid;
			tpssm81["REC_REVISE_TIME"] = datetime;

			v_mat_seq_no = v_mat_seq_no + 1;
			tpssm81["PLAN_NO"] = v_plan_no;
			tpssm81["MAT_SEQ_NO"] = v_mat_seq_no;
			tpssm81["PLAN_STATUS"] = "04";

			if (tpssm81["ST_NO"].ToString() != " ")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库

				default: // 所有数据库适用，通用SQL语句
					sqlstr = " SELECT MACH_CLEAR_DIV,MACH_CLEAR_DIF,MACH_CLEAR_LEV,SLAB_FINISH_CODE FROM TQMTS0X "
						" WHERE FACTORY_DIV = @tpssm81.FACTORY_DIV "
						"	AND ST_NO = @tpssm81.ST_NO "
						"	AND VALID_FLAG = '1' ";
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm81.FACTORY_DIV", tpssm81["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm81.ST_NO", tpssm81["ST_NO"].ToString());
				cmd_inq.ExecuteReader();

				if (cmd_inq.Read())
				{
					tpssm81["MACH_CLEAR_DIV"] = cmd_inq.GetString(1);
					tpssm81["MACH_CLEAR_DIF"] = cmd_inq.GetString(2);
					tpssm81["MACH_CLEAR_LEV"] = cmd_inq.GetString(3);
					tpssm81["SLAB_FINISH_CODE"] = cmd_inq.GetString(4);
				}
				cmd_inq.Close();
			}
			else
			{
				CFormattable arguments[] = { tmmsm01["ST_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "最终出钢记号[{0}]不允许为空！。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			//tpssm81["MAT_NUM"] = tmmsm01["MAT_NUM"];
			//tpssm81["MAT_ACT_WT"] = tmmsm01["MAT_ACT_WT"];

			//// 更新材料主档表当前材料的材料状态、计划号
			//tpssm81["NEXT_WHOLE_BACKLOG_CODE"] = tmmsm01["NEXT_WHOLE_BACKLOG_CODE"];
			//tpssm81["NEXT_WHOLE_BACKLOG_SEQ"] = tmmsm01["NEXT_WHOLE_BACKLOG_SEQ"];
			//tpssm81["WHOLE_BACKLOG_CODE"] = tmmsm01["WHOLE_BACKLOG_CODE"];
			//tpssm81["WHOLE_BACKLOG_SEQ"] = tmmsm01["WHOLE_BACKLOG_SEQ"];
			//tpssm81["WHOLE_BACKLOG"] = tmmsm01["WHOLE_BACKLOG"];
			//tpssm81["WHOLE_BACKLOG_NO"] = tmmsm01["WHOLE_BACKLOG_NO"];

			if (tpssm81["REPAIR_FLAG"].ToString() == "1")
			{
				//修改返修处置表的记录状态：1,计划中
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:				// MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " UPDATE TQMTJF1 SET STATUS_FLAG = '1' "
						"  WHERE MAT_NO = @tpssm81.MAT_NO  "
						"	 AND SEQ_NO = "
						"				(SELECT  MIN(SEQ_NO) SEQ_NO "
						"						FROM TQMTJF1 A "
						"					WHERE STATUS_FLAG = '0' "
						"						AND IN_MAT_NO = @tpssm81.MAT_NO "
						"						AND REPAIR_BACKLOG_CODE = v_backlog_code ";
					break;
				}

				cmd_tqmtjf1_upd.SetCommandText(sqlstr);
				cmd_tqmtjf1_upd.Parameters.Set("tpssm81.MAT_NO", tpssm81["MAT_NO"].ToString());
				cmd_tqmtjf1_upd.Parameters.Set("v_backlog_code", v_backlog_code);
				cmd_tqmtjf1_upd.ExecuteNonQuery();
			}

			// 更新材料主档表当前材料的材料状态、计划号；
			//调用物料跟踪函数,修改厚板主档表，材料状态＝23，
			////Log::Trace("", __FUNCTION__, "更新材料主档表当前材料的材料状态、计划号");
			inBlock.Tables["MM0099"].Rows.Add();

			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_ID"] = "PS01";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_LINE_TYPE"] = "00";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["SYSTEM_ID"] = "PSSM";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["FUNC_ID"] = s.svc_name;
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["MAT_NO"] = tpssm81["MAT_NO"]; //材料号
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["PLAN_NO"] = tpssm81["PLAN_NO"];

			v_rownum_mm99++;

			tpssm81.TrimOrBlank();
			tpssm81.Insert();
		}

		////Log::Trace("", __FUNCTION__, "v_rownum_mm99[{0}]", v_rownum_mm99);
		if (v_rownum_mm99 > 0)
		{
			doFlag = f_mmsm99(&inBlock, &outBlock, conn);

			if (doFlag != 0)
			{
				CFormattable arguments[] = { tpssm81["MAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PS00S0000148")/*材料[{0}]修改物料跟踪信息出错。*/, arguments, 1); //格式化字符串 
				throw CApplicationException(doFlag, s.msg, log.Location);
			}
		}
	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = -1;
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
