/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhengqiangqiang
Version:    1.0
Date:     2022-09-01 17:13:56
Description: 炼钢计划预配料-制造命令查询
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
/// <para>数据库表：TPSSM01(炼钢制造命令表)    </para>
/// <para>主调用函数：前台PSSM01画面F2(查询)按钮   </para>
/// </summary>
/// <param name=""> </param>
/// <param name="">  </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(pssm01p_pono_inq)

int f_pssm01p_pono_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_ord = "";
	int TotalRecordCount = 0;

	int v_slab_thick = 0;
	int v_slab_width = 0;
	int v_slab_len = 0;
	CDecimal v_mat_tube = 0;
	CString v_billet_type;
	CString slab_thick = "";
	CString slab_width = "";
	CString slab_len = "";
	CString v_factory_div = "";
	CString v_mat_specs = "";
	CString v_cc_div = "";
	CString v_last_plan_date = "";
	int fetchRowCount = 0;
	CString v_prec_roll_plan_no = " ";
	CDecimal v_prec_roll_seq_no = 0;
	CString function_id = "PSSM01P_PONO_INQ";

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm01("TPSSM01");
	CModel tqmts0x("TQMTS0X");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。

	try
	{
		try
		{
			//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;
		}

		//--------------------------------
		//获取传入参数
		tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		tpssm01["PONO_STATUS"] = 13;
		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "factory_div = [{0}]", tpssm01["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "SLAB_DEST = [{0}]", tpssm01["SLAB_DEST"].ToString());
		Log::Info("", __FUNCTION__, "cast_lot_no = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
		Log::Info("", __FUNCTION__, "pono_status = [{0}]", tpssm01["PONO_STATUS"].ToDecimal().ToInt32());
		Log::Info("", __FUNCTION__, "plan_date = [{0}]", tpssm01["PLAN_DATE"].ToString());

		//如果前台传了FUNCTION_ID，后台直接调用就可以
		/*if (bcls_rec->Tables[0].Columns.IndexOf("function_id") < 0)
		{
		bcls_rec->Tables[0].Columns.Add(DT_STRING, "FUNCTION_ID");
		bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"] = function_id;
		}
		else
		{
		function_id = bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"];
		}*/
		Log::Trace("", __FUNCTION__, "===function_id =  [{0}]", function_id);
		if (bcls_ret->Tables.get_Count() < 1)
		{
			//在bcls_ret 中增加一个块，放查询结果
			bcls_ret->Tables.Add();
		}
		f_edsetcustominfo(bcls_rec, bcls_ret);

		strcpy(s.msg, " ");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	  // Oracle 数据库
		default:
			sqlstr_count = " SELECT COUNT(1) "
				"   FROM TPSSM01 t "
				"  WHERE 1=1  "
				;
			sqlstr =" SELECT distinct t.*, "
				" case "
				"  when t.cast_lot_sum <> 0 "
				"  then t.cast_lot_sum || '-' || t.cast_lot_div_no "
				" else ' '"
				" end as cc_div, "
				" a.SLAB_THICK,  "
				" a.SLAB_WIDTH,  "
				" a.SLAB_LEN,    "
				"  CAST(a.slab_thick AS int)||'*'||CAST(a.slab_width AS int)||'*'||CAST(a.slab_len AS int) AS MAT_SPECS, "
				" a.PREC_ROLL_PLAN_NO,   "
				" a.PREC_ROLL_SEQ_NO,  "
				" b.billet_type  "
				//" NVL(c.old_st_no,t.ST_NO) as  old_st_no "
				" FROM TPSSM01 t   "
				" left join tpssm03 a "
				"   on t.pono = a.pono "
				" AND a.SLAB_SEQ_1 = 1 "
				" and t.factory_div = a.factory_div "
				" left join tpssm02 b "
				"   on t.factory_div = b.factory_div "
				" and t.cast_lot_no = b.cast_lot_no "
				" left join tqmts0x c "
				"   on t.st_no = c.st_no "
				" WHERE 1 = 1  "
				;

			if (tpssm01["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND t.FACTORY_DIV = @tpssm01.FACTORY_DIV ";
			}

			if (tpssm01["SLAB_DEST"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND t.SLAB_DEST = @tpssm01.SLAB_DEST " ;
			}
			if (tpssm01["PONO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND t.PONO like '%'|| @tpssm01.PONO||'%' ";
			}
			if (tpssm01["CAST_LOT_NO"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND t.CAST_LOT_NO = @tpssm01.CAST_LOT_NO	";
			}
			if (tpssm01["PONO_STATUS"].ToDecimal() != 0)
			{
				sqlstr_temp += " AND t.PONO_STATUS <= @tpssm01.PONO_STATUS  and   t.PONO_STATUS>=11 ";
			}
			else
			{
				sqlstr_temp += " AND t.PONO_STATUS >=11 AND t.PONO_STATUS < 83 ";
			}
			if (tpssm01["PLAN_DATE"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND t.PLAN_DATE = @tpssm01.PLAN_DATE  ";
			}
			sqlstr_temp += "   and t.HOT_CHARGE_FLAG in('2','5')    ";

			sqlstr_temp_ord = "  ORDER BY t.PLAN_DATE,t.CC_MACH_NO,t.CC_SEQ,t.CAST_LOT_NO ASC, t.CAST_LOT_DIV_NO ASC  ";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_ord;
			break;
		}

		Log::Trace("", "", "==1.=sqlstr_count={0}", sqlstr_count);
		Log::Trace("", "", "==2.=sqlstr={0}", sqlstr);

		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.SLAB_DEST", tpssm01["SLAB_DEST"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO_STATUS", tpssm01["PONO_STATUS"].ToDecimal());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PLAN_DATE", tpssm01["PLAN_DATE"].ToString());

		cmd_tpssm01_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm01_inq.ExecuteScalar().ToInt32();

		Log::Trace("", "", "TotalRecordCount11={0}", TotalRecordCount);

		//分页获取
		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_tpssm01_inq.Close();


		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

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
