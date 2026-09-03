/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 制造命令查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM09画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm81add_inq)

int f_pssm81add_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int newbk = 0;
	int k = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int		TotalRecordCount = 0;

	CString v_factory_div = "";
	CString v_plan_backlog_code = "";
	CString v_backlog_code = "";
	CString v_heat_no = "";
	CString v_st_no = "";
	CString v_order_no = "";
	CString v_stock_place_no = "";
	CString v_prod_time_f = "";
	CString v_prod_time_t = "";
	CString	function_id = "PSSM81ADD_INQD";/* 功能号 */
	CString v_mach_clear_div = "";
	CString v_slab_cool_ind = "";
	CString v_stock_no = "";

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tmmsm01("TMMSM01");

	CDbCommand cmd_inq(conn); 
	CDbCommand cmd_tep0002_inq(conn);
	CDbCommand cmd_tqmtjf1_inq(conn);
	CDbCommand cmd_tmmsm01_inq(conn);

	try
	{
		//设置返回块
		//设置查询结果的功能号
		bcls_rec->Tables[0].Columns.Add(DT_STRING, "function_id");
		bcls_rec->Tables[0].Rows[0]["function_id"] = function_id;
		////Log::Trace("", __FUNCTION__, "===function_id = [{0}]  ", bcls_rec->Tables[0].Rows[0]["function_id"].ToString());

		if (bcls_ret->GetBlkNum() < 1)
		{
			//在bcls_rec 中增加一个块，放查询结果
			newbk = bcls_ret->AddBlock();
		}
		bcls_ret->blk_now = 1; //返回到第一块中

		f_edsetcustominfo(bcls_rec, bcls_ret);

		//--------------------------------
		//获取传入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		v_plan_backlog_code = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
		v_heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"];
		v_st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"];
		v_order_no = bcls_rec->Tables[0].Rows[0]["ORDER_NO"];
		v_stock_place_no = bcls_rec->Tables[0].Rows[0]["STOCK_PLACE_NO"];
		v_prod_time_f = bcls_rec->Tables[0].Rows[0]["PROD_TIME_F"];
		v_prod_time_t = bcls_rec->Tables[0].Rows[0]["PROD_TIME_T"];


		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", v_factory_div);
		////Log::Info("", __FUNCTION__, "v_plan_backlog_code	= [{0}]", v_plan_backlog_code);
		////Log::Info("", __FUNCTION__, "v_heat_no	= [{0}]", v_heat_no);
		////Log::Info("", __FUNCTION__, "v_st_no	= [{0}]", v_st_no);
		////Log::Info("", __FUNCTION__, "v_order_no	= [{0}]", v_order_no);
		////Log::Info("", __FUNCTION__, "v_stock_place_no	= [{0}]", v_stock_place_no);
		////Log::Info("", __FUNCTION__, "v_prod_time	= [{0}]-[{1}]", v_prod_time_f, v_prod_time_t);

		if (v_factory_div.Trim() == "")
		{
			strcpy(s.msg, "厂别代码不允许为空！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (v_plan_backlog_code.Trim() == "")
		{
			strcpy(s.msg, "计划工序代码不允许为空！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sqlstr = "SELECT CODE_DESC_2_CONTENT,CODE_DESC_3_CONTENT FROM TEP0002 WHERE CODE_CLASS = 'PSS0' AND CODE = @v_plan_backlog_code ";

		cmd_tep0002_inq.SetCommandText(sqlstr);
		cmd_tep0002_inq.Parameters.Set("v_plan_backlog_code", v_plan_backlog_code);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);

		cmd_tep0002_inq.ExecuteReader();

		if (cmd_tep0002_inq.Read())
		{
			v_backlog_code = cmd_tep0002_inq.GetString(1);
			v_stock_no = cmd_tep0002_inq.GetString(2);
		}
		cmd_tep0002_inq.Close();

		if (v_backlog_code.Trim() == "")
		{
			strcpy(s.msg, "计划机组代码对应工序不允许为空！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////Log::Info("", __FUNCTION__, "v_stock_no = [{0}]", v_stock_no);

		//查询返修处置的材料信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT A.* "
				"   FROM TMMSM01 A, TQMTJF1 B, "
				"	(SELECT  MAT_NO, MIN(SEQ_NO) SEQ_NO "
				"			FROM  TQMTJF1 M  "
				"			WHERE  STATUS_FLAG = '0'  "
				"			  AND MAT_NO NOT IN (SELECT MAT_NO FROM TQMTJF1 WHERE STATUS_FLAG = '1')  "
				"    GROUP  BY MAT_NO) C  "
				"  WHERE A.FACTORY_DIV = @v_factory_div "
				"    AND A.MAT_NO = B.MAT_NO "
				"	 AND B.MAT_NO = C.MAT_NO "
				"    AND B.SEQ_NO = C.SEQ_NO "
				"    AND A.MAT_STATUS = '22'  "
				"	 AND A.REPAIR_FLAG = '1'  "
				"	 AND A.IN_FLAG = '1'  "
				"	 AND A.PLAN_NO = ' '  "
				"	 AND B.REPAIR_BACKLOG_CODE != ' '  ";
			if (v_backlog_code.Trim() == "SMJ")
			{
				sqlstr_temp += " AND B.REPAIR_BACKLOG_CODE = 'A6' ";
			}

			if (v_stock_no != "ALL")
			{
				sqlstr_temp += " AND A.STOCK_NO = @v_stock_no ";
			}

			if (v_heat_no.Trim() != "")
			{
				sqlstr_temp += " AND A.HEAT_NO	= @v_heat_no";
			}
			if (v_st_no != "")
			{
				sqlstr_temp += " AND A.ST_NO	= @v_st_no";
			}
			if (v_order_no != "")
			{
				sqlstr_temp += " AND A.ORDER_NO	= @v_order_no";
			}
			if (v_stock_place_no != "")
			{
				sqlstr_temp += " AND A.STOCK_PLACE_NO = @v_stock_place_no";
			}
			if (v_prod_time_t != "")
			{
				sqlstr_temp += " AND A.PROD_TIME BETWEEN @v_prod_time_f||'000000' AND @v_prod_time_t||'240000' ";
			}

			sqlstr_temp_order += " ORDER BY A.STOCK_PLACE_NO  ASC, A.MAT_NO ASC ";
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
			break;
		}

		cmd_tqmtjf1_inq.SetCommandText(sqlstr);
		cmd_tqmtjf1_inq.Parameters.Set("v_factory_div", v_factory_div);

		if (v_stock_no.Trim() != "ALL")
		{
			cmd_tqmtjf1_inq.Parameters.Set("v_stock_no", v_stock_no);
		}
		if (v_heat_no.Trim() != "")
		{
			cmd_tqmtjf1_inq.Parameters.Set("v_heat_no", v_heat_no);
		}
		if (v_st_no.Trim() != "")
		{
			cmd_tqmtjf1_inq.Parameters.Set("v_st_no", v_st_no);
		}
		if (v_order_no.Trim() != "")
		{
			cmd_tqmtjf1_inq.Parameters.Set("v_order_no", v_order_no);
		}
		if (v_stock_place_no.Trim() != "")
		{
			cmd_tqmtjf1_inq.Parameters.Set("v_stock_place_no", v_stock_place_no);
		}
		if (v_prod_time_t.Trim() != "")
		{
			cmd_tqmtjf1_inq.Parameters.Set("v_prod_time_f", v_prod_time_f);
			cmd_tqmtjf1_inq.Parameters.Set("v_prod_time_t", v_prod_time_t);
		}

		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_tqmtjf1_inq.ExecuteReader();

		while (cmd_tqmtjf1_inq.Read())
		{
			k = cmd_tqmtjf1_inq.Fetch(tmmsm01);

			////Log::Info("", __FUNCTION__, "tmmsm01["MAT_NO"] = [{0}]", tmmsm01["MAT_NO"].ToString());

			v_mach_clear_div = "";
			v_slab_cool_ind = "";

			// 将结果放入返回块
			CDataRow& row = bcls_ret->Tables[0].Rows.Add(); //新增一行
			row.Merge(tmmsm01);
			row["REPAIR_FLAG"] = "1";
			row["MACH_CLEAR_DIV"] = v_mach_clear_div;
			row["SLAB_COOL_IND"] = v_slab_cool_ind;
			row["BACKLOG_CODE"] = v_backlog_code;
		}
		cmd_tqmtjf1_inq.Close();

		sqlstr_temp = "";
		sqlstr_temp_order = "";
		//查询材料信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT A.*, B.MACH_CLEAR_DIV, B.SLAB_COOL_IND  "
				"   FROM TMMSM01 A,TQMTS0X B "
				"  WHERE A.FACTORY_DIV = @v_factory_div "
				"    AND SUBSTR(A.FACTORY_DIV, 1, 1) = B.FACTORY_DIV  "
				"	 AND A.ST_NO = B.ST_NO  "
				"    AND A.MAT_STATUS = '20'  "
				"	 AND A.IN_FLAG = '1'  "
				"	 AND A.PLAN_NO = ' '  ";
			if (v_backlog_code.Trim() == "SMJ")
			{
				sqlstr_temp += " AND A.FINISH_FLAG = '1' "; //1-待做,9-已做
			}
			else if (v_backlog_code.Trim() == "SMH")
			{
				sqlstr_temp += " AND A.COOL_FLAG = '1' "; //1-待做,9-已做
			}
			else
			{
				strcpy(s.msg, "计划工序代码有误，请确认！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (v_stock_no != "ALL")
			{
				sqlstr_temp += " AND A.STOCK_NO = @v_stock_no ";
			}

			if (v_heat_no.Trim() != "")
			{
				sqlstr_temp += " AND A.HEAT_NO	= @v_heat_no";
			}
			if (v_st_no != "")
			{
				sqlstr_temp += " AND A.ST_NO	= @v_st_no";
			}
			if (v_order_no != "")
			{
				sqlstr_temp += " AND A.ORDER_NO	= @v_order_no";
			}
			if (v_stock_place_no != "")
			{
				sqlstr_temp += " AND A.STOCK_PLACE_NO	= @v_stock_place_no";
			}
			if (v_prod_time_t != "")
			{
				sqlstr_temp += " AND A.PROD_TIME BETWEEN @v_prod_time_f||'000000' AND @v_prod_time_t||'240000' ";
			}

			sqlstr_temp_order += " ORDER BY A.STOCK_PLACE_NO  ASC, A.MAT_NO ASC ";

			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
			break;
		}

		cmd_tmmsm01_inq.SetCommandText(sqlstr);
		cmd_tmmsm01_inq.Parameters.Set("v_factory_div", v_factory_div);

		if (v_stock_no.Trim() != "ALL")
		{
			cmd_tmmsm01_inq.Parameters.Set("v_stock_no", v_stock_no);
		}
		if (v_heat_no.Trim() != "")
		{
			cmd_tmmsm01_inq.Parameters.Set("v_heat_no", v_heat_no);
		}
		if (v_st_no.Trim() != "")
		{
			cmd_tmmsm01_inq.Parameters.Set("v_st_no", v_st_no);
		}
		if (v_order_no.Trim() != "")
		{
			cmd_tmmsm01_inq.Parameters.Set("v_order_no", v_order_no);
		}
		if (v_stock_place_no.Trim() != "")
		{
			cmd_tmmsm01_inq.Parameters.Set("v_stock_place_no", v_stock_place_no);
		}
		if (v_prod_time_t.Trim() != "")
		{
			cmd_tmmsm01_inq.Parameters.Set("v_prod_time_f", v_prod_time_f);
			cmd_tmmsm01_inq.Parameters.Set("v_prod_time_t", v_prod_time_t);
		}

		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_tmmsm01_inq.ExecuteReader();

		while (cmd_tmmsm01_inq.Read())
		{
			k = cmd_tmmsm01_inq.Fetch(tmmsm01);
			v_mach_clear_div = cmd_tmmsm01_inq.GetString(k + 1);
			v_slab_cool_ind = cmd_tmmsm01_inq.GetString(k + 2);

			////Log::Info("", __FUNCTION__, "tmmsm01["MAT_NO"] = [{0}]", tmmsm01["MAT_NO"].ToString());
			////Log::Info("", __FUNCTION__, "v_mach_clear_div = [{0}]", v_mach_clear_div);
			////Log::Info("", __FUNCTION__, "v_slab_cool_ind = [{0}]", v_slab_cool_ind);

			// 将结果放入返回块
			CDataRow& row = bcls_ret->Tables[0].Rows.Add(); //新增一行
			row.Merge(tmmsm01);
			row["REPAIR_FLAG"] = "0";
			row["MACH_CLEAR_DIV"] = v_mach_clear_div;
			row["SLAB_COOL_IND"] = v_slab_cool_ind;
			row["BACKLOG_CODE"] = v_backlog_code;
		}
		cmd_tmmsm01_inq.Close();

		
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
