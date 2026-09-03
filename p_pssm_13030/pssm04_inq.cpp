/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 炼钢计划制造命令查询
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
/// <para>主调用函数：前台PSSM01画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm04_inq)

int f_pssm04_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int TotalRecordCount = 0;

	int max_slab_thick_1 = 0;
	int max_slab_width_1 = 0;
	int min_slab_width_1 = 0;
	int max_slab_thick_2 = 0;
	int max_slab_width_2 = 0;
	int min_slab_width_2 = 0;
	int strand_num = 0;
	int cc_div = 0;
	CDecimal slab_len_dif = 0;
	CString slab_thick_1 = "";
	CString slab_width_1 = "";
	CString slab_thick_2 = "";
	CString slab_width_2 = "";
	CString v_cc_div = "";
	CString v_factory_div = "";
	int fetchRowCount = 0;
	int fetchRowCount2 = 0;
	CString v_prec_roll_plan_no = " ";
	CString function_id = "";
	CDecimal v_prec_roll_seq_no = 0;
	CString strN_ccc;   //add by liuyali 连连铸
	CString cast_lot_sum = "";
	CString cast_lot_div_no = "";
	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm01("TPSSM01");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq2(conn);  //与DB 建立连接。
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
			pageInfo.RecordFrom = 1;
			pageInfo.PageSize = 1000;
		}

		//--------------------------------
		//获取传入参数
		tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", tpssm01["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "cc_mach_no = [{0}]", tpssm01["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "cast_lot_no = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status = [{0}]", tpssm01["PONO_STATUS"].ToDecimal().ToInt32());
		////Log::Info("", __FUNCTION__, "plan_date = [{0}]", tpssm01["PLAN_DATE"].ToString());

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr_count = " SELECT COUNT(1) "
				"   FROM TPSSM01 "
				"  WHERE PONO_STATUS IN ('11','12') "
				"    AND FACTORY_DIV	= @tpssm01.FACTORY_DIV "
				;
			sqlstr = " SELECT * "
				"   FROM TPSSM01 "
				"  WHERE PONO_STATUS IN ('11','12') "
				"    AND FACTORY_DIV	= @tpssm01.FACTORY_DIV "
				;
				
			//if ((tpssm01["FACTORY_DIV"].ToString().Trim() != "") && (tpssm01["CC_MACH_NO"].ToString().Trim() != ""))
			//{
			//	sqlstr_temp += " AND CC_TYPE IN (SELECT CC_TYPE FROM TPSSMD8 ";
			//	sqlstr_temp += " WHERE FACTORY_DIV	= @tpssm01.FACTORY_DIV ";
			//	sqlstr_temp += " AND CC_MACH_NO		= @tpssm01["CC_MACH_NO"].ToString()) ";
			//}

			sqlstr_temp_order += " ORDER BY PLAN_DATE,CC_MACH_NO,CC_SEQ,CAST_LOT_NO,CAST_LOT_DIV_NO ASC ";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
			break;
		}
		////Log::Trace("", __FUNCTION__, "sqlstr_temp = [{0}]", sqlstr_temp);

		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());


		cmd_tpssm01_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm01_inq.ExecuteScalar().ToInt32();
		//分页获取
		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_tpssm01_inq.Close();

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_THICK");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_THICK2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LEN_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PREC_ROLL_PLAN_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_DIV");          //add by liuyali

		for (fetchRowCount = 0; fetchRowCount < bcls_ret->Tables[0].Rows.get_Count(); fetchRowCount++)
		{
			tpssm01["PONO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["PONO"];
			tpssm01["CAST_LOT_NO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_NO"];
			tpssm01["CAST_LOT_SUM"] = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_SUM"];
			tpssm01["CAST_LOT_DIV_NO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_DIV_NO"];
			tpssm01["FACTORY_DIV"] = bcls_ret->Tables[0].Rows[fetchRowCount]["FACTORY_DIV"];
			tpssm01["ST_NO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["ST_NO"];

			////Log::Info("", __FUNCTION__, "pono  =[{0}]", tpssm01["PONO"].ToString());
			////Log::Info("", __FUNCTION__, "cast_lot_no  =[{0}]", tpssm01["CAST_LOT_NO"].ToString());
			////Log::Info("", __FUNCTION__, "FACTORY_DIV  =[{0}]", tpssm01["FACTORY_DIV"].ToString());
			////Log::Info("", __FUNCTION__, "CAST_LOT_SUM  =[{0}]", tpssm01["CAST_LOT_SUM"].ToDecimal());
			////Log::Info("", __FUNCTION__, "CAST_LOT_DIV_NO  =[{0}]", tpssm01["CAST_LOT_DIV_NO"].ToDecimal());

			if (tpssm01["CAST_LOT_SUM"].ToDecimal() != 0)
			{
				v_cc_div = tpssm01["CAST_LOT_SUM"].ToDecimal().ToString() + "-" + tpssm01["CAST_LOT_DIV_NO"].ToDecimal().ToString();
			}
			else
			{
				v_cc_div = " ";
			}
			////Log::Info("", __FUNCTION__, "v_cc_div  =[{0}]", v_cc_div);
			/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
			cast_lot_sum = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
			cast_lot_div_no = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/
			strN_ccc = cast_lot_sum + " - " + cast_lot_div_no;
			bcls_ret->Tables[0].Rows[fetchRowCount]["CC_DIV"] = strN_ccc;
			//bcls_ret->Tables[0].Rows[fetchRowCount]["CC_DIV"] = v_cc_div;
			//板坯规格
			//1流板坯规格
			sqlstr = " SELECT "
				" MAX(SLAB_THICK),"
				" MAX(SLAB_WIDTH), "
				" MIN(SLAB_WIDTH), "
				" MAX(STRAND_NUM) "
				" FROM TPSSM03 "
				" WHERE FACTORY_DIV = @factory_div"
				"   AND PONO = @pono";
			sqlstr += "  AND  STRAND_NO = '1' ";
			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();
			////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);

			if (cmd_tpssm03_inq.Read())
			{
				max_slab_thick_1 = cmd_tpssm03_inq.GetDecimal(1).ToInt32();
				max_slab_width_1 = cmd_tpssm03_inq.GetDecimal(2).ToInt32();
				min_slab_width_1 = cmd_tpssm03_inq.GetDecimal(3).ToInt32();
				strand_num = cmd_tpssm03_inq.GetDecimal(4).ToInt32();

				slab_thick_1 = CConvert::ToString(max_slab_thick_1);
				////Log::Info("", __FUNCTION__, "max_slab_width_1 = [{0}]", max_slab_width_1);
				////Log::Info("", __FUNCTION__, "min_slab_width_1 = [{0}]", min_slab_width_1);
				slab_width_1 = CConvert::ToString(max_slab_width_1) + " - " + CConvert::ToString(min_slab_width_1);
			}
			else
			{
				slab_thick_1 = "";
				slab_width_1 = "";
			}
			cmd_tpssm03_inq.Close();

			if (strand_num == 2)
			{
				//2流板坯规格
				sqlstr = " SELECT "
					" MAX(SLAB_THICK),"
					" MAX(SLAB_WIDTH), "
					" MIN(SLAB_WIDTH) "
					" FROM TPSSM03 "
					" WHERE FACTORY_DIV = @factory_div"
					"   AND PONO = @pono";
				sqlstr += "  AND  STRAND_NO = '2' ";
				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("factory_div", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
				cmd_tpssm03_inq.ExecuteReader();

				if (cmd_tpssm03_inq.Read())
				{
					max_slab_thick_2 = cmd_tpssm03_inq.GetDecimal(1).ToInt32();
					max_slab_width_2 = cmd_tpssm03_inq.GetDecimal(2).ToInt32();
					min_slab_width_2 = cmd_tpssm03_inq.GetDecimal(3).ToInt32();

					slab_thick_2 = CConvert::ToString(max_slab_thick_2);
					slab_width_2 = CConvert::ToString(max_slab_width_2) + " - " + CConvert::ToString(min_slab_width_2);
				}
				cmd_tpssm03_inq.Close();
			}
			else
			{
				slab_thick_2 = "";
				slab_width_2 = "";
			}

			//等长标记
			CString sql_len = " SELECT MIN(SLAB_MAX_LEN - SLAB_MIN_LEN) "
				" FROM TPSSM03 "
				" WHERE FACTORY_DIV = @factory_div"
				"   AND PONO = @pono";
			sqlstr = sql_len;
			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();

			if (cmd_tpssm03_inq.Read())
			{
				slab_len_dif = cmd_tpssm03_inq.GetDecimal(1);
				if (slab_len_dif >  0 && slab_len_dif <  0.3)//长度之差小于0.3显示"1"
				{;
				bcls_ret->Tables[0].Rows[fetchRowCount]["LEN_FLAG"] = "1";
				}
				else
				{
					bcls_ret->Tables[0].Rows[fetchRowCount]["LEN_FLAG"] = "";
				}
			}
			else
			{
				bcls_ret->Tables[0].Rows[fetchRowCount]["LEN_FLAG"] = "";
			}
			cmd_tpssm03_inq.Close();

			//////Log::Info("", __FUNCTION__, "v_mat_tube      = [{0}]",v_mat_tube);

			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_WIDTH"] = slab_width_1;
			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_THICK"] = slab_thick_1;
			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_WIDTH2"] = slab_width_2;
			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_THICK2"] = slab_thick_2;

			fetchRowCount2 = 1;
			v_prec_roll_plan_no = "";
			////Log::Info("", __FUNCTION__, "1:---fetchRowCount2  =[{0}], v_prec_roll_plan_no  =[{1}]---", fetchRowCount2, v_prec_roll_plan_no);

			CString sql_roll_plan = " SELECT DISTINCT PREC_ROLL_PLAN_NO "
				" FROM TPSSM03 "
				" WHERE FACTORY_DIV = @factory_div "
				"   AND PREC_ROLL_PLAN_NO <> ' ' "
				"   AND PONO = @pono";
			sqlstr = sql_roll_plan;
			cmd_tpssm03_inq2.SetCommandText(sqlstr);
			cmd_tpssm03_inq2.Parameters.Set("factory_div", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq2.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq2.ExecuteReader();

			while (cmd_tpssm03_inq2.Read())
			{
				if (fetchRowCount2 == 1)
				{
					v_prec_roll_plan_no = cmd_tpssm03_inq2.GetString(1);
				}
				else
				{
					v_prec_roll_plan_no = "/" + v_prec_roll_plan_no.Trim() + cmd_tpssm03_inq2.GetString(1);
				}

				////Log::Info("", __FUNCTION__, "2:---fetchRowCount2  =[{0}], v_prec_roll_plan_no  =[{1}]---", fetchRowCount2, v_prec_roll_plan_no);

				fetchRowCount2++;
			}
			cmd_tpssm03_inq2.Close();

			bcls_ret->Tables[0].Rows[fetchRowCount]["PREC_ROLL_PLAN_NO"] = v_prec_roll_plan_no;
		}

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
