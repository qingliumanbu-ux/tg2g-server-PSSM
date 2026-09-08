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
BM2F_ENTERACE(pssm10cllcf2_inq)

int f_pssm10cllcf2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	int v_slab_thick = 0;
	int v_slab_width = 0;
	int v_slab_len = 0;
	int count = 0;
	CString sg_sign = "";

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
	CString v_ingot_code = "";
	CString v_hp_sg_remark = ""; //厚板钢坯钢种
	CString v_apply_sg_sign_1 = ""; //炼钢外部钢种
	CString v_cast_no = "";
	CString v_cc_remark = "";
	CString v_cc_remark_idx = "";
	CString v_strand_no_1 = "";//一流宽度
	CString v_strand_no_2 = "";//二流宽度
	CString label1 = "";//协议牌号
	CString v_cc_mach_no = "";//连铸机号

	CString cast_no_plan = "";
	CString cast_plan_seq = "";
	CString	prod_date_from = "";
	CString	prod_date_to = "";
	CDecimal v_pono_status = 0; //制造命令状态

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm01("TPSSM01");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tqmbms1_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm26_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tqmbms2_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03a_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03b_inq(conn);  //与DB 建立连接。

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
		/*prod_date_from = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString().Trim();
		prod_date_to = bcls_rec->Tables[0].Rows[0]["PLAN_DATE2"].ToString().Trim();*/
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().Trim();
		//v_pono_status = bcls_rec->Tables[0].Rows[0]["PONO_STATUS"].ToDecimal();


		//固定制造命令状态为13（编制计划状态）
		//tpssm01.PONO_STATUS = 13;

		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "factory_div = [{0}]", tpssm01["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "cc_mach_no = [{0}]", v_cc_mach_no);
		Log::Info("", __FUNCTION__, "cast_lot_no = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
		//	Log::Info("", __FUNCTION__, "pono_status = [{0}]", tpssm01.PONO_STATUS.ToInt32());
		Log::Info("", __FUNCTION__, "plan_date = [{0}]", tpssm01["PLAN_DATE"].ToString());
		//Log::Info("", __FUNCTION__, "v_pono_status = [{0}]", v_pono_status);



		if (tpssm01["FACTORY_DIV"].ToString().Trim() != "")
		{
			sqlstr_temp += " AND A.FACTORY_DIV		= @tpssm01.FACTORY_DIV";
		}

		if (prod_date_from.Trim() != "")
		{
			sqlstr_temp += " AND A.PLAN_DATE >= @prod_date_from  ";
		}
		if (prod_date_to.Trim() != "")
		{
			sqlstr_temp += " AND A.PLAN_DATE <= @prod_date_to ";
		}
		if ("" != v_cc_mach_no.Trim())
		{
			sqlstr_temp += " AND A.CC_MACH_NO = @v_cc_mach_no ";
		}
		sqlstr_count =
			" SELECT COUNT(1) "
			" FROM TPSSM10 A "
			" WHERE 1=1 " + sqlstr_temp  //15:命令挂起; 16:命令接收
			;
// DM8 适配 CHANGE-125:查询。TPSSM10。空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
		// sqlstr =
			// " SELECT * FROM ( "
			// " SELECT  A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			// " A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, DECODE(D1.MAT_RCV, NULL, 0 , D1.MAT_RCV) || '/' || D.MAT_TUBE MAT_TUBE,"
			// " A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA    "
			// " FROM TPSSM10 A"
			// " LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			// " LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			// " WHERE 1=1"
			// + sqlstr_temp +
			// " AND A.PONO_STATUS < 83 AND A.PONO_STATUS > 16"
			// " ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO) "
			// " UNION ALL "
			// " SELECT * FROM ( "
			// " SELECT A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			// " A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, DECODE(D1.MAT_RCV, NULL, 0 , D1.MAT_RCV) || '/' || D.MAT_TUBE MAT_TUBE,"
			// " A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA   "
			// " FROM TPSSM10 A"
			// " LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			// " LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			// " WHERE 1=1"
			// + sqlstr_temp +
			// " AND A.PONO_STATUS IN (15,16)"
			// " AND A.CC_SEQ <> 999  "
			// " ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO  )   "
// DM8 SQL：
// DM8 适配 CHANGE-126:查询。TPSSM10。空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
		// sqlstr =
			// " SELECT * FROM ( "
			// " SELECT  A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			// " A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, CASE WHEN D1.MAT_RCV IS NULL THEN 0 ELSE D1.MAT_RCV END || '/' || D.MAT_TUBE MAT_TUBE,"
			// " A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA    "
			// " FROM TPSSM10 A"
			// " LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			// " LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			// " WHERE 1=1"
			// + sqlstr_temp +
			// " AND A.PONO_STATUS < 83 AND A.PONO_STATUS > 16"
			// " ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO) "
			// " UNION ALL "
			// " SELECT * FROM ( "
			// " SELECT A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			// " A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, CASE WHEN D1.MAT_RCV IS NULL THEN 0 ELSE D1.MAT_RCV END || '/' || D.MAT_TUBE MAT_TUBE,"
			// " A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA   "
			// " FROM TPSSM10 A"
			// " LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			// " LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			// " WHERE 1=1"
			// + sqlstr_temp +
			// " AND A.PONO_STATUS IN (15,16)"
			// " AND A.CC_SEQ <> 999  "
			// " ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO  )   "
// 
			// " UNION ALL "
			// " SELECT * FROM ( "
			// " SELECT A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			// " A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, DECODE(D1.MAT_RCV, NULL, 0 , D1.MAT_RCV) || '/' || D.MAT_TUBE MAT_TUBE,"
			// " A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA     "
			// " FROM TPSSM10 A"
			// " LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			// " LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			// " LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			// " WHERE 1=1"
			// + sqlstr_temp +
			// " AND A.PONO_STATUS IN (15,16)"
			// " AND A.CC_SEQ = 999  "
			// " ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO ) "
// 
// 
			// ;
// DM8 SQL：
		sqlstr =
			" SELECT * FROM ( "
			" SELECT  A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			" A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, CASE WHEN D1.MAT_RCV IS NULL THEN 0 ELSE D1.MAT_RCV END || '/' || D.MAT_TUBE MAT_TUBE,"
			" A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA    "
			" FROM TPSSM10 A"
			" LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			" LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			" LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			" LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			" WHERE 1=1"
			+ sqlstr_temp +
			" AND A.PONO_STATUS < 83 AND A.PONO_STATUS > 16"
			" ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO) "
			" UNION ALL "
			" SELECT * FROM ( "
			" SELECT A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			" A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, CASE WHEN D1.MAT_RCV IS NULL THEN 0 ELSE D1.MAT_RCV END || '/' || D.MAT_TUBE MAT_TUBE,"
			" A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA   "
			" FROM TPSSM10 A"
			" LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			" LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			" LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			" LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			" WHERE 1=1"
			+ sqlstr_temp +
			" AND A.PONO_STATUS IN (15,16)"
			" AND A.CC_SEQ <> 999  "
			" ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO  )   "

			" UNION ALL "
			" SELECT * FROM ( "
			" SELECT A.SLAB_DEST, A.CAST_LOT_NO || '-' || A.CAST_LOT_DIV_NO CAST_LOT_NO,  B.CAST_NO,"
			" A.SLAB_THICK || '*' || A.SLAB_WIDTH  MAT_SPECS, C.CAST_SUM, CASE WHEN D1.MAT_RCV IS NULL THEN 0 ELSE D1.MAT_RCV END || '/' || D.MAT_TUBE MAT_TUBE,"
			" A.CC_SEQ,A.RESTRAND_FLG,A.PONO,A.PONO_STATUS,A.ST_NO,A.SG_SIGN,A.PLAN_TAP_WT,A.HOT_SEND_FLAG,A.HOT_CHARGE_FLAG,A.PLAN_DATE,A.FACTORY_DIV,A.BACKLOG_EA     "
			" FROM TPSSM10 A"
			" LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
			" LEFT JOIN (SELECT CAST_NO, COUNT(PONO) CAST_SUM FROM TPSSM11 WHERE CAST_NO <> ' ' GROUP BY CAST_NO) C ON B.CAST_NO = C.CAST_NO"
			" LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_TUBE FROM TPSSM03 GROUP BY PONO) D ON A.PONO = D.PONO"
			" LEFT JOIN (SELECT PONO, COUNT(PONO) MAT_RCV FROM TPSSM03 WHERE SLAB_PROD_FLAG = '1' GROUP BY PONO) D1 ON A.PONO = D1.PONO"
			" WHERE 1=1"
			+ sqlstr_temp +
			" AND A.PONO_STATUS IN (15,16)"
			" AND A.CC_SEQ = 999  "
			" ORDER BY A.CC_SEQ,A.PLAN_DATE,A.CC_MACH_NO,B.CAST_NO,A.CAST_LOT_NO,A.CAST_LOT_DIV_NO ) "


			;


		Log::Info("", __FUNCTION__, "sqlstr_count = [{0}]", sqlstr_count);
		Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);


		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("prod_date_from", prod_date_from);
		cmd_tpssm01_inq.Parameters.Set("prod_date_to", prod_date_to);
		cmd_tpssm01_inq.Parameters.Set("v_cc_mach_no", tpssm01["CC_MACH_NO"].ToString());
		//cmd_tpssm01_inq.Parameters.Set("v_pono_status", v_pono_status);


		cmd_tpssm01_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm01_inq.ExecuteScalar().ToInt32();
		Log::Info("", __FUNCTION__, "TotalRecordCount[{0}]", TotalRecordCount);
		//分页获取
		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_tpssm01_inq.Close();

		

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;
		bcls_ret->Tables[0].set_TableName("PSSM101_INQ");
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
