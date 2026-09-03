/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-04-013
功能: 制造命令下信息查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"

/******service入口******/
BM2F_ENTERACE(pssm08_inq1);

int f_pssm08_inq1(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/
	int doFlag = 0;		//返回值
	int logFlag = 1;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString sql_where = "";
	CString sqlstr_temp_order = "";
	CString sqlstr_count = "";
	CString sql_count = "";
	/*定义业务用变量*/
	CString strSt_no = "";
	CString temp_cast_lot = "";
	CString factory_div;
	CString cc_mach_no = "";
	CString cc_mach_no1 ="";
	CString cc_mach_no2 = "";
	CString cc_mach_no3 = ""; 
	int blkseq = 0;

	CString plan_date1;
	CString plan_date2;
	CString steel_app_date1;
	CString steel_app_date2;
	CString strN_ccc;
	CString strN_ccc_1;
	CString strN_ccc_2;
	CString slab_width_1;
	CString slab_width_2;
	CString strBsqf;
	CString strBsqf_1;
	CString strBsqf_2;
	CString strBsqf_3;
	CString strCast_lot_vol;
	CString cc_div;
	CString cast_sum;
	CString cast_div_no;
	CString strFlame_clean_1;
	CString strFactory_div;
	CString v_hot_charge_method = "";
	CDecimal min_slab_width;
	CDecimal max_slab_width;
	CDecimal slab_thick_1;
	CDecimal slab_thick_2;
	CDecimal strand_num = 0;
	int TotalRecordCount = 0;
	int TotalRecordCount1 = 0;
	int TotalRecordCount2 = 0;

	//系统的分页类信息。
	CPageInfo pageInfo;

	/*实体类定义*/

	CDbCommand cmd_sql(sqlstr, conn);

	/******业务处理开始******/
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

		/*获取前台输入数据*/
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();/*炼钢区分*/
		plan_date1 = bcls_rec->Tables[0].Rows[0]["PLAN_DATE1"].ToString();
		plan_date2 = bcls_rec->Tables[0].Rows[0]["PLAN_DATE2"].ToString();
		cc_mach_no1 = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO1"].ToString().Trim();
		cc_mach_no2 = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO2"].ToString().Trim();
		cc_mach_no3 = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO3"].ToString().Trim();


		////Log::Trace("", __FUNCTION__, "IN:factory_div = [{0}]", factory_div);
		////Log::Trace("", __FUNCTION__, "IN:plan_date1 = [{0}]", plan_date1);
		////Log::Trace("", __FUNCTION__, "IN:plan_date2 = [{0}]", plan_date2);
		////Log::Trace("", __FUNCTION__, "IN:cc_mach_no1 = [{0}]", cc_mach_no1);
		////Log::Trace("", __FUNCTION__, "IN:cc_mach_no2 = [{0}]", cc_mach_no2);
		////Log::Trace("", __FUNCTION__, "IN:cc_mach_no3 = [{0}]", cc_mach_no3);
		bcls_ret->Tables.Add("tab1");
		bcls_ret->Tables.Add("tab2");
		if (cc_mach_no1.Trim().GetLength() > 0 && factory_div.Trim().GetLength() > 0)
		{
			sqlstr_count = "select count(1) from "

				"(select b.*, a.SLAB_THICK, a.SLAB_WIDTH from "
				"(select * from TPSSM01 WHERE PONO_STATUS > 13 AND(TPSSM01.PLAN_DATE BETWEEN @plan_date1 AND @plan_date2) AND TPSSM01.CC_MACH_NO = @cc_mach_no1 AND TPSSM01.FACTORY_DIV = @factory_div)b "
				"left join  TPSSM03 a "
				"on b.PONO = a.PONO and b.factory_div = a.FACTORY_DIV)c "
			;
			sqlstr = "select b.*, a.SLAB_THICK, a.SLAB_WIDTH from "
				"(select * from TPSSM01 WHERE PONO_STATUS > 13 AND(TPSSM01.PLAN_DATE BETWEEN @plan_date1 AND @plan_date2) AND TPSSM01.CC_MACH_NO = @cc_mach_no1 AND TPSSM01.FACTORY_DIV = @factory_div)b "
				"left join  TPSSM03 a "
				"on b.PONO = a.PONO and b.factory_div = a.FACTORY_DIV ";
				//"ORDER BY CC_MACH_NO, PLAN_DATE, CC_SEQ ";

			cmd_sql.Parameters.Set("factory_div", factory_div);
			cmd_sql.Parameters.Set("plan_date1", plan_date1);
			cmd_sql.Parameters.Set("plan_date2", plan_date2);
			cmd_sql.Parameters.Set("cc_mach_no1", cc_mach_no1);
			cmd_sql.SetCommandText(sqlstr_count);
			TotalRecordCount = cmd_sql.ExecuteScalar().ToInt32();
			//分页获取
			cmd_sql.SetCommandText(sqlstr);
			cmd_sql.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
			cmd_sql.Close();

			bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_DIV");

			for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
			{
				
				strN_ccc_1 = bcls_ret->Tables[0].Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
				strN_ccc_2 = bcls_ret->Tables[0].Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/

				strCast_lot_vol = bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/

				////Log::Trace("", __FUNCTION__, "strCast_lot_vol = [{0}]", strCast_lot_vol);
				/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
				strN_ccc = strN_ccc_1 + " - " + strN_ccc_2;
				bcls_ret->Tables[0].Rows[i]["CC_DIV"] = strN_ccc;

				/*CAST - LOT号*/
					bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"] = strCast_lot_vol;
			}
			bcls_ret->Tables[0].set_TableName("tab0");

			//返回分页总数量信息 
			bcls_ret->Tables.Add("PageInfo_0");
			bcls_ret->Tables["PageInfo_0"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
			bcls_ret->Tables["PageInfo_0"].Rows.Add();
			bcls_ret->Tables["PageInfo_0"].Rows[0]["TotalRecordCount"] = TotalRecordCount;
		}

		if (cc_mach_no2.Trim().GetLength() > 0 && factory_div.Trim().GetLength() > 0)
		{
			sqlstr_count =  "select count(1) from "
				"(select b.*, a.SLAB_THICK, a.SLAB_WIDTH from "
				"(select * from TPSSM01 WHERE PONO_STATUS > 13 AND(TPSSM01.PLAN_DATE BETWEEN @plan_date1 AND @plan_date2) AND TPSSM01.CC_MACH_NO = @cc_mach_no2 AND TPSSM01.FACTORY_DIV = @factory_div)b "
				"left join  TPSSM03 a "
				"on b.PONO = a.PONO and b.factory_div = a.FACTORY_DIV)c "
				;
			sqlstr = "select b.*, a.SLAB_THICK, a.SLAB_WIDTH from "
				"(select * from TPSSM01 WHERE PONO_STATUS > 13 AND(TPSSM01.PLAN_DATE BETWEEN @plan_date1 AND @plan_date2) AND TPSSM01.CC_MACH_NO = @cc_mach_no2 AND TPSSM01.FACTORY_DIV = @factory_div)b "
				"left join  TPSSM03 a "
				"on b.PONO = a.PONO and b.factory_div = a.FACTORY_DIV ";
			//"ORDER BY CC_MACH_NO, PLAN_DATE, CC_SEQ ";

			cmd_sql.Parameters.Set("factory_div", factory_div);
			cmd_sql.Parameters.Set("plan_date1", plan_date1);
			cmd_sql.Parameters.Set("plan_date2", plan_date2);
			cmd_sql.Parameters.Set("cc_mach_no2", cc_mach_no2);
			cmd_sql.SetCommandText(sqlstr_count);
			TotalRecordCount1 = cmd_sql.ExecuteScalar().ToInt32();
			//分页获取
			cmd_sql.SetCommandText(sqlstr);
			cmd_sql.ExecuteQuery(bcls_ret->Tables["tab1"], pageInfo.RecordFrom, pageInfo.PageSize);
			cmd_sql.Close();

			bcls_ret->Tables["tab1"].Columns.Add(DT_STRING, "CC_DIV");

			for (int i = 0; i < bcls_ret->Tables["tab1"].Rows.get_Count(); i++)
			{

				strN_ccc_1 = bcls_ret->Tables["tab1"].Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
				strN_ccc_2 = bcls_ret->Tables["tab1"].Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/

				strCast_lot_vol = bcls_ret->Tables["tab1"].Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/

				////Log::Trace("", __FUNCTION__, "strCast_lot_vol = [{0}]", strCast_lot_vol);
				/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
				strN_ccc = strN_ccc_1 + " - " + strN_ccc_2;
				bcls_ret->Tables["tab1"].Rows[i]["CC_DIV"] = strN_ccc;

				/*CAST - LOT号*/
				bcls_ret->Tables["tab1"].Rows[i]["CAST_LOT_NO"] = strCast_lot_vol;
			}
			//bcls_ret->Tables["tab1"].set_TableName("TPSSM01");

			//返回分页总数量信息 
			bcls_ret->Tables.Add("PageInfo_1");
			bcls_ret->Tables["PageInfo_1"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
			bcls_ret->Tables["PageInfo_1"].Rows.Add();
			bcls_ret->Tables["PageInfo_1"].Rows[0]["TotalRecordCount"] = TotalRecordCount1;
		}

		if (cc_mach_no3.Trim().GetLength() > 0 && factory_div.Trim().GetLength() > 0)
		{
			////Log::Trace("", __FUNCTION__, "cc_mach_no3 = [{0}]", cc_mach_no3);
			sqlstr_count = "select count(1) from "
				"(select b.*, a.SLAB_THICK, a.SLAB_WIDTH from "
				"(select * from TPSSM01 WHERE PONO_STATUS > 13 AND(TPSSM01.PLAN_DATE BETWEEN @plan_date1 AND @plan_date2) AND TPSSM01.CC_MACH_NO = @cc_mach_no3 AND TPSSM01.FACTORY_DIV = @factory_div)b "
				"left join  TPSSM03 a "
				"on b.PONO = a.PONO and b.factory_div = a.FACTORY_DIV)c ";
				;
			sqlstr = "select b.*, a.SLAB_THICK, a.SLAB_WIDTH from "
				"(select * from TPSSM01 WHERE PONO_STATUS > 13 AND(TPSSM01.PLAN_DATE BETWEEN @plan_date1 AND @plan_date2) AND TPSSM01.CC_MACH_NO = @cc_mach_no3 AND TPSSM01.FACTORY_DIV = @factory_div)b "
				"left join  TPSSM03 a "
				"on b.PONO = a.PONO and b.factory_div = a.FACTORY_DIV ";
			//"ORDER BY CC_MACH_NO, PLAN_DATE, CC_SEQ ";

			cmd_sql.Parameters.Set("factory_div", factory_div);
			cmd_sql.Parameters.Set("plan_date1", plan_date1);
			cmd_sql.Parameters.Set("plan_date2", plan_date2);
			cmd_sql.Parameters.Set("cc_mach_no3", cc_mach_no3);
			cmd_sql.SetCommandText(sqlstr_count);
			TotalRecordCount2 = cmd_sql.ExecuteScalar().ToInt32();
			//分页获取
			cmd_sql.SetCommandText(sqlstr);
			cmd_sql.ExecuteQuery(bcls_ret->Tables["tab2"], pageInfo.RecordFrom, pageInfo.PageSize);
			cmd_sql.Close();

			bcls_ret->Tables["tab2"].Columns.Add(DT_STRING, "CC_DIV");

			for (int i = 0; i < bcls_ret->Tables["tab2"].Rows.get_Count(); i++)
			{

				strN_ccc_1 = bcls_ret->Tables["tab2"].Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
				strN_ccc_2 = bcls_ret->Tables["tab2"].Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/

				strCast_lot_vol = bcls_ret->Tables["tab2"].Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/

				////Log::Trace("", __FUNCTION__, "strCast_lot_vol = [{0}]", strCast_lot_vol);
				/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
				strN_ccc = strN_ccc_1 + " - " + strN_ccc_2;
				bcls_ret->Tables["tab2"].Rows[i]["CC_DIV"] = strN_ccc;

				/*CAST - LOT号*/
				bcls_ret->Tables["tab2"].Rows[i]["CAST_LOT_NO"] = strCast_lot_vol;
			}

			//返回分页总数量信息 
			bcls_ret->Tables.Add("PageInfo_2");
			bcls_ret->Tables["PageInfo_2"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
			bcls_ret->Tables["PageInfo_2"].Rows.Add();
			bcls_ret->Tables["PageInfo_2"].Rows[0]["TotalRecordCount"] = TotalRecordCount2;
		}

	//	sql_where = "  WHERE PONO_STATUS > 13  "
	//		"    AND (PLAN_DATE BETWEEN @plan_date1 AND @plan_date2)  ";

	//	if (factory_div.Trim().GetLength() > 0)
	//	{
	//		sql_where += " AND FACTORY_DIV = @factory_div ";
	//	}
	//	if (cc_mach_no.Trim().GetLength() > 0)
	//	{
	//		sql_where += " AND CC_MACH_NO = @cc_mach_no ";
	//	}

	//	sqlstr = "select distinct CC_MACH_NO "
	//		"from TPSSM01 " + sql_where;
	//	cmd_sql.Parameters.Set("factory_div", factory_div);
	//	cmd_sql.Parameters.Set("plan_date1", plan_date1);
	//	cmd_sql.Parameters.Set("plan_date2", plan_date2);
	//	cmd_sql.Parameters.Set("cc_mach_no", cc_mach_no);
	//	cmd_sql.SetCommandText(sqlstr);
	//	////Log::Trace("", __FUNCTION__, sqlstr);
	//	CDataTable temp;
	//	cmd_sql.ExecuteQuery(temp);
	//	sql_where += " AND CC_MACH_NO = @cc_mach_no ";
	//	for (int j = 0; j < temp.Rows.get_Count(); j++)
	//	{
	//		sqlstr_temp_order = " ORDER BY CC_MACH_NO, PLAN_DATE, CC_SEQ ";
	//		sqlstr_count = " SELECT COUNT(1) "
	//			"   FROM TPSSM01 " + sql_where;
	//		sqlstr = " SELECT * "
	//			"   FROM TPSSM01 " + sql_where + sqlstr_temp_order;
	//		cc_mach_no = temp.Rows[j][0].ToString();
	//		cmd_sql.Parameters.Set("cc_mach_no", cc_mach_no);
	//		cmd_sql.SetCommandText(sqlstr_count);
	//		////Log::Trace("", __FUNCTION__, sqlstr_count);
	//		TotalRecordCount = cmd_sql.ExecuteScalar().ToInt32();
	//		cmd_sql.SetCommandText(sqlstr);
	//		////Log::Trace("", __FUNCTION__, sqlstr);
	//		CDataTable &dt = bcls_ret->Tables.Add(cc_mach_no);
	//		cmd_sql.ExecuteQuery(dt);
	//		dt.Columns.Add(DT_STRING, "HOT_SENDCHARGE_FLAG");
	//		dt.Columns.Add(DT_STRING, "CC_DIV");
	//		dt.Columns.Add(DT_STRING, "SLAB_WIDTH");

	//		temp_cast_lot = "";
	//		for (int i = 0; i < dt.Rows.get_Count(); i++)
	//		{
	//			strFactory_div = dt.Rows[i]["FACTORY_DIV"];
	//			strN_ccc_1 = dt.Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
	//			strN_ccc_2 = dt.Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/

	//			strCast_lot_vol = dt.Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/
	//			strSt_no = dt.Rows[i]["ST_NO"].ToString(); /*出钢记号*/

	//			////Log::Trace("", __FUNCTION__, "strBsqf_1 = [{0}]", strBsqf_1);
	//			////Log::Trace("", __FUNCTION__, "strCast_lot_vol = [{0}]", strCast_lot_vol);
	//			/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
	//			strN_ccc = strN_ccc_1 + " - " + strN_ccc_2;
	//			dt.Rows[i]["CC_DIV"] = strN_ccc;

	//			/*CAST - LOT号*/  
	//			dt.Rows[i]["CAST_LOT_NO"] = strCast_lot_vol;

	//			/*根据查询到的浇铸批号查询铸机流中的厚度和宽度*/ //从01 03表读取数据。采用连接的方式，03表查询出板坯的厚度和宽度。流数不要
	//			if (strCast_lot_vol != temp_cast_lot)
	//			{
	//				sqlstr = " SELECT MAX(SLAB_THICK), "
	//					" MAX(SLAB_WIDTH), "
	//					" MIN(SLAB_WIDTH), "
	//					" MAX(STRAND_NUM) "
	//					" FROM TPSSM03 "
	//					" WHERE CAST_LOT_NO = @cast_lot_vol "
	//					"   AND FACTORY_DIV = @factory_div ";

	//				sqlstr += "  AND  STRAND_NO = '1' ";

	//				CDbCommand cmd_inq(sqlstr, conn);

	//				cmd_inq.Parameters.Set("cast_lot_vol", strCast_lot_vol);
	//				cmd_inq.Parameters.Set("factory_div", strFactory_div);
	//				cmd_inq.ExecuteReader();

	//				if (cmd_inq.Read())
	//				{
	//					slab_thick_1 = cmd_inq.GetInt32(1);
	//					////Log::Trace("", __FUNCTION__, "slab_thick_1 = [{0}]", slab_thick_1.ToString());
	//					max_slab_width = cmd_inq.GetInt32(2);
	//					////Log::Trace("", __FUNCTION__, "max_slab_width = [{0}]", max_slab_width.ToString());
	//					min_slab_width = cmd_inq.GetInt32(3);
	//					strand_num = cmd_inq.GetInt32(4);
	//				}

	//				slab_width_1 = max_slab_width.ToString() + "-" + min_slab_width.ToString();
	//				cmd_inq.Close();

	//				if (strand_num == 2)
	//				{
	//					sqlstr = " SELECT  MAX(SLAB_THICK),"
	//						" MAX(SLAB_WIDTH),"
	//						" MIN(SLAB_WIDTH) "
	//						" FROM TPSSM03 "
	//						" WHERE CAST_LOT_NO = @cast_lot_vol"
	//						"   AND FACTORY_DIV = @factory_div ";

	//					sqlstr += "  AND STRAND_NO = '2'";

	//					CDbCommand cmd_inq1(sqlstr, conn);
	//					cmd_inq1.Parameters.Set("cast_lot_vol", strCast_lot_vol);
	//					cmd_inq1.Parameters.Set("factory_div", strFactory_div);

	//					cmd_inq1.ExecuteReader();

	//					if (cmd_inq1.Read())
	//					{
	//						slab_thick_2 = cmd_inq1.GetInt32(1);
	//						max_slab_width = cmd_inq1.GetInt32(2);
	//						min_slab_width = cmd_inq1.GetInt32(3);
	//					}

	//					slab_width_2 = max_slab_width.ToString() + " - " + min_slab_width.ToString();

	//					dt.Rows[i]["SLAB_WIDTH2"] = slab_width_2;
	//					dt.Rows[i]["SLAB_THICK2"] = slab_thick_2.ToString();

	//					cmd_inq1.Close();
	//				}

	//				temp_cast_lot = strCast_lot_vol;
	//			}
	//			else
	//			{

	//				/*流宽、流厚、CAST - LOT号置空*/
	//				dt.Rows[i]["SLAB_WIDTH"] = "";
	//				dt.Rows[i]["CAST_LOT_NO"] = "";
	//			}
	//	/*	}*/
	//		//返回分页总数量信息 
	//		CDataTable &pageTable=bcls_ret->Tables.Add("PageInfo_"+ cc_mach_no);
	//		/*CDataTable &pageTable = bcls_ret->Tables.Add("PageInfo");*/
	//		pageTable.Columns.Add(DT_DECIMAL, "TotalRecordCount");
	//		pageTable.Rows.Add();
	//		pageTable.Rows[0]["TotalRecordCount"] = TotalRecordCount;
	//	/*}*/
}

	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
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
		strncpy(s.msg, (const char *)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}



