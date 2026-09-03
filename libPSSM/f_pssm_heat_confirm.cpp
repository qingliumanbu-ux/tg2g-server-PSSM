/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:向萍
Date:2015-7-30
Version:1.0
Description: 炉次确定
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件







#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)

#endif



//炼钢PES发往热轧PES DHCR板坯未产出信息
int f_mm2030m2_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
//调用DHCR计划删除
int f_mm2030m2_pro(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
//调用计划处理函数
int f_pssm11_file(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
//调用强制确定计划处理函数
int f_pssm11_force_file(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);

int f_t8z_23m_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);//发送专家系统数据

#if defined _SYS_MES
int f_pmom_pono_confm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pmom_lot_confm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsm_heat_confirm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsm_heat_confirm_lot(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
int f_qmtqhp_dele_resv_chg_new(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_qmtqhp_bujt(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

BM2_FUNCTION_EXPORT
int f_pssm_heat_confirm(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	CString sqlstr="";
	CString v_judge_code = "";
	CString v_opt_flag = "1";//操作标记1-正常确定，2-强制确定
	CString v_code_desc_5_content = "";
	CString c_datetime = CDateTime::Now().AddHours(-8).ToString("yyyyMMddHHmmss");
	CString c_datetime2 = CDateTime::Now().AddHours(-60).ToString("yyyyMMddHHmmss");
	CString code = "";
	int row_num = 0;
	int row_num_p = 0;
	int row_num_l = 0;
	int v_count = 0;

	CModel tep0002("TEP0002");
	CModel tpssm12("TPSSM12");
	CModel tpssm11("TPSSM11");
	CModel tpssm11_2("TPSSM11");
	CModel tpssm01("TPSSM01");
	CModel tpssm01_1("TPSSM01");
	CModel tpssm03("TPSSM03");
	CModel tmmsm01("TMMSM01");

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
	CModel tpmouhp31("TPMOUHP31");
#endif



	CDbCommand cmd_tep0002_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm03_upd(conn); 
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tmmsm01_inq(conn);
	CDbCommand cmd_sql(conn);

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
	CDbCommand cmd_tpmouhp31_inq(conn);
#endif


	EIClass inBlock1;//DHCR未产出板坯接口
	EIClass inBlock2;//
	EIClass inBlock3;//DHCR未产出板坯接口
	EIClass inBlock_pmconfm;
	EIClass inBlock_mmconfm;

	EIClass bcls_rec_dele_resv;
	EIClass bcls_ret_dele_resv;

	EIClass bcls_rec_bujt;
	EIClass bcls_ret_bujt;

	EIClass in_23m;
	in_23m.Tables[0].Columns.Add(DT_STRING, "TC_NO");
	in_23m.Tables[0].Rows.Add();
	in_23m.Tables[0].Rows[0]["TC_NO"] = "T82324";
	in_23m.Tables.Add();
	in_23m.Tables[1].Columns.Add(tpssm01);
	in_23m.Tables.Add();
	in_23m.Tables[2].Columns.Add(tpssm03);
	try
	{
		inBlock_pmconfm.Clear();
		if (!inBlock_pmconfm.Tables.Contains("PONOCONFM"))
		{
			inBlock_pmconfm.Tables.Add("PONOCONFM");
		}
		if (!inBlock_pmconfm.Tables["PONOCONFM"].Columns.Contains("PONO"))
		{
			inBlock_pmconfm.Tables["PONOCONFM"].Columns.Add(DT_STRING, "PONO");
		}

		if (!inBlock_pmconfm.Tables.Contains("LOTCONFM"))
		{
			inBlock_pmconfm.Tables.Add("LOTCONFM");
		}
		if (!inBlock_pmconfm.Tables["LOTCONFM"].Columns.Contains("CAST_LOT_NO"))
		{
			inBlock_pmconfm.Tables["LOTCONFM"].Columns.Add(DT_STRING, "CAST_LOT_NO");
		}

		inBlock_mmconfm.Clear();
		//modify by xp 2016/11/29 应张颖要求炉次确定增加调用合同跟踪处理
		if (!inBlock_mmconfm.Tables.Contains("MMSMCONFM"))
		{
			inBlock_mmconfm.Tables.Add("MMSMCONFM");
		}
		if (!inBlock_mmconfm.Tables["MMSMCONFM"].Columns.Contains("PONO"))
		{
			inBlock_mmconfm.Tables["MMSMCONFM"].Columns.Add(DT_STRING, "PONO");
		}

		if (!inBlock_mmconfm.Tables.Contains("MMSMCONFMLOT"))
		{
			inBlock_mmconfm.Tables.Add("MMSMCONFMLOT");
		}
		if (!inBlock_mmconfm.Tables["MMSMCONFMLOT"].Columns.Contains("CAST_LOT_NO"))
		{
			inBlock_mmconfm.Tables["MMSMCONFMLOT"].Columns.Add(DT_STRING, "CAST_LOT_NO");
		}

		if (!bcls_rec_bujt.Tables.Contains("QMZSBlock"))
		{
			bcls_rec_bujt.Tables.Add("QMZSBlock");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "OUHP_MAT_TYPE");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "PONO");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "TMP_SLAB_NO");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "SAMPLE_LOT_STATUS");
			bcls_rec_bujt.Tables["QMZSBlock"].Rows.Add();
		}

		if (bcls_rec->Tables.IndexOf("OP_FLAG") > 0)
		{
			v_opt_flag = bcls_rec->Tables["OP_FLAG"].Rows[0]["FLAG"].ToString();
		}
		////Log::Trace("", __FUNCTION__, "v_opt_flag=[{0}]", v_opt_flag);

		inBlock1.Tables[0].set_TableName("XX2030M2");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "MAT_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PREC_SLAB_NO");

		inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		
		inBlock2.Tables[0].Rows.Add();  //只生成一行

		if (!bcls_rec_dele_resv.Tables.Contains("QMZSBlock"))
		{
			bcls_rec_dele_resv.Tables.Add("QMZSBlock");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "PONO_SLAB");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "PONO");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "EVENT_ID");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "EVENT_DESC");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "SYSTEM_ID");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "FUNC_ID");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "FORM_CODE");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "MAT_KIND");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "MAT_NO");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Rows.Add();
		}
		
		if (v_opt_flag == "3")

		{
			sqlstr = CString(" SELECT a.PONO AS PONO2, a.STEEL_RETURN_CODE AS CODE, a.* FROM TPSSM11 a left join TPSSM12 b on a.sm_plan_no = b.sm_plan_no WHERE a.PONO_STATUS = 83 AND b.area_id = 5 AND (( b.end_time_real <= @c_datetime AND a.CUT_FIN_FLAG = '1'  AND a.REP_ELM_SEL_FLAG = '1') OR a.STEEL_RETURN_CODE = '1')  ORDER BY b.end_time_real "); //AND ROWNUM <= 50 AND a.PRACT_RCV_FLAG = '1'
			//sqlstr = CString(" SELECT a.PONO AS PONO2, a.STEEL_RETURN_CODE AS CODE, a.* FROM TPSSM11 a left join TPSSM12 b on a.sm_plan_no = b.sm_plan_no WHERE a.PONO_STATUS = 83 AND b.area_id = 5 AND (( b.end_time_real <= @c_datetime2) OR a.STEEL_RETURN_CODE = '1')  ORDER BY b.end_time_real ");
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("c_datetime", c_datetime);
			cmd_tpssm11_inq.Parameters.Set("c_datetime2", c_datetime2);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				//CDataRow dr;
				//dr = cmd_tpssm11_inq.
				tpssm11_2.Reset();
				cmd_tpssm11_inq.Fetch(tpssm11_2);
				//tpssm11_2.MergeTo(bcls_rec->Tables[0]);
				tmmsm01["PONO"] = cmd_tpssm11_inq.GetString(1); 
				code = cmd_tpssm11_inq.GetString(2);

				sqlstr = " select count(1) from(select DISTINCT (LSLAB_NO) from tpssm03 where pono = @pono AND SLAB_PROD_FLAG = '1') ";
				cmd_tmmsm01_inq.SetCommandText(sqlstr);
				cmd_tmmsm01_inq.Parameters.Set("pono", tmmsm01["PONO"].ToString());
				CDecimal count0 = cmd_tmmsm01_inq.ExecuteScalar();

				sqlstr = " select count(1) from (select * from tmmsm01 where LSLAB_NO IN (select DISTINCT (LSLAB_NO) from tpssm03 where pono = @pono AND SLAB_PROD_FLAG = '1') union all select * from hmmsm01 where LSLAB_NO IN (select DISTINCT (LSLAB_NO) from tpssm03 where pono = @pono AND SLAB_PROD_FLAG = '1')) ";
				cmd_tmmsm01_inq.SetCommandText(sqlstr);
				cmd_tmmsm01_inq.Parameters.Set("pono", tmmsm01["PONO"].ToString());
				CDecimal count1 = cmd_tmmsm01_inq.ExecuteScalar();

				sqlstr = " select count(1) from (select * from tmmsm01 where LSLAB_NO IN (select DISTINCT (LSLAB_NO) from tpssm03 where pono = @pono AND SLAB_PROD_FLAG = '1') AND RCV_MAT_FLAG = 'S' union all select * from hmmsm01 where LSLAB_NO IN (select DISTINCT (LSLAB_NO) from tpssm03 where pono = @pono AND SLAB_PROD_FLAG = '1') AND RCV_MAT_FLAG = 'S') ";
				cmd_tmmsm01_inq.SetCommandText(sqlstr);
				cmd_tmmsm01_inq.Parameters.Set("pono", tmmsm01["PONO"].ToString());
				CDecimal count2 = cmd_tmmsm01_inq.ExecuteScalar();
				
				Log::Trace("", __FUNCTION__, "Count0 = [{0}], Count1=[{1}], Count2=[{2}], PONO=[{3}],code = [{4}]", count0, count1, count2, tmmsm01["PONO"].ToString(), code);
				if ((count0 == count2 && count1 == count2 && count2 != 0) || code.Trim() == "1" || tmmsm01["PONO"].ToString().Substring(0,1) == "9")
				{
					//continue;
					tpssm11_2.MergeTo(bcls_rec->Tables[0]);
				}
			}
			cmd_tpssm11_inq.Close();
		}
		Log::Trace("", __FUNCTION__, "Count=[{0}]", bcls_rec->Tables[0].Rows.get_Count());

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//获取传入参数
			tpssm11.Reset();
			tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			////Log::Trace("", __FUNCTION__, "i=[{0}]", i);
			////Log::Trace("", __FUNCTION__, "tpssm11["PONO"] =[{0}]", tpssm11["PONO"].ToString());
			////Log::Trace("", __FUNCTION__, "tpssm11["HEAT_NO"] =[{0}]", tpssm11["HEAT_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "tpssm11["REP_ELM_SEL_FLAG"] =[{0}]", tpssm11["REP_ELM_SEL_FLAG"].ToString());
			////Log::Trace("", __FUNCTION__, "tpssm11["CUT_FIN_FLAG"] =[{0}]", tpssm11["CUT_FIN_FLAG"].ToString());
			////Log::Trace("", __FUNCTION__, "tpssm11["PRACT_RCV_FLAG"] =[{0}]", tpssm11["PRACT_RCV_FLAG"].ToString());

			if (tpssm11["PONO_STATUS"].ToDecimal() == 91)
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "PONO=[{0}]已经做过炉次确定", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (v_opt_flag == "1" || v_opt_flag == "3")//正常确定时判断是否满足确定条件
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:
					sqlstr = " SELECT CODE FROM "
						" ( SELECT CODE FROM TEP0002 "
						"   WHERE CODE_CLASS = 'PSA92N' "
						"     AND CODE_DESC_2_CONTENT = '1' " 
						"   ORDER BY CODE) "
						;
					break;
				}
				//wcy 二钢小代码
				cmd_tep0002_inq.SetCommandText(sqlstr);
				////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
				cmd_tep0002_inq.ExecuteReader();
				while (cmd_tep0002_inq.Read())
				{
					tep0002["CODE"] = cmd_tep0002_inq.GetString(1);
					if (tpssm11["RUN_STATUS"].ToString() != "83"&&tpssm11["RUN_STATUS"].ToString() != "84" && tpssm11["STEEL_RETURN_CODE"].ToString().Trim() == "")
					{
						if (tep0002["CODE"].ToString() == "1")  //代表成分
						{
							if (tpssm11["REP_ELM_SEL_FLAG"].ToString().TrimOrBlank().Compare(" ") == 0 || tpssm11["REP_ELM_SEL_FLAG"].ToString().TrimOrBlank().Compare("0") == 0)
							{
								CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
								CMessageFormat::Format(s.msg, _RES("PSSMS0000141")/*制造命令号[{0}]的代表成分没有选定，不能做炉次确定操作。*/, arguments, 1); //格式化字符串
								throw CApplicationException(-1, s.msg, log.Location);
							}

						}
						else if (tep0002["CODE"].ToString() == "2") //切断标记
						{
							if (tpssm11["CUT_FIN_FLAG"].ToString().TrimOrBlank().Compare(" ") == 0 || tpssm11["CUT_FIN_FLAG"].ToString().TrimOrBlank().Compare("0") == 0)
							{
								CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
								CMessageFormat::Format(s.msg, "制造命令号[{0}]的铸坯未全部生产/切断完毕，不能做炉次确定操作。", arguments, 1); //格式化字符串
								throw CApplicationException(-1, s.msg, log.Location);
							}
						}
						else if (tep0002["CODE"].ToString() == "3") // 工序收集
						{
							if (tpssm11["PRACT_RCV_FLAG"].ToString().TrimOrBlank().Compare(" ") == 0 || tpssm11["PRACT_RCV_FLAG"].ToString().TrimOrBlank().Compare("0") == 0)
							{
								CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
								CMessageFormat::Format(s.msg, _RES("PSSMS0000143")/*制造命令号[{0}]的工序实绩没有全部收到，不能做炉次确定操作。*/, arguments, 1); //格式化字符串
								throw CApplicationException(-1, s.msg, log.Location);
							}
						}
					}

				}
				cmd_tep0002_inq.Close();
			}//判断是否满足确定条件-正常确定时

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = "SELECT HOT_SEND_FLAG, HOT_CHARGE_FLAG, CAST_LOT_NO, CC_MACH_NO "
					"  FROM TPSSM01 "
					" WHERE PONO	= @tpssm11.PONO";
				break;
			}
			cmd_sql.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
			cmd_sql.Parameters.Clear();
			cmd_sql.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_sql.ExecuteReader();
			if (cmd_sql.Read())
			{
				tpssm01["HOT_SEND_FLAG"] = cmd_sql.GetString(1);
				tpssm01["HOT_CHARGE_FLAG"] = cmd_sql.GetString(2);
				tpssm01["CAST_LOT_NO"] = cmd_sql.GetString(3);
				tpssm01["CC_MACH_NO"] = cmd_sql.GetString(4);
			}
			cmd_sql.Close();

			//DHCR未产出
			if (((tpssm01["HOT_SEND_FLAG"].ToString() == "1") || (tpssm01["HOT_SEND_FLAG"].ToString() == "2")) && (tpssm01["HOT_CHARGE_FLAG"].ToString() == "2"))
			{
				inBlock1.Tables[0].Rows.Add();
				inBlock1.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];



#ifdef _LINE_NEXT_HR      ////下产线与热轧PES连接 _PES_SM_HR

				//炼钢PES发往热轧PES DHCR板坯未产出信息
				//ret = f_mm2030m2_snd(&inBlock1, bcls_ret,conn); 
				if (ret != 0)
				{
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}

#elif _LINE_HR       //包含热轧产线，并与炼钢在同一系统

				//调用DHCR计划删除
				//ret = f_mm2030m2_pro(&inBlock1, bcls_ret, conn);
				if (ret != 0)
				{
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
#endif

			}//DHCR未产出

			//2.调用计划归档函数			

			inBlock2.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			inBlock2.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
			inBlock2.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];

			//CDataRow& row_pono = inBlock2.Tables[0].Rows.Add();   //新增空行
			//row_pono["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			//row_pono["PONO"] = tpssm11["PONO"];
			//row_pono["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];

			////Log::Trace("", __FUNCTION__, "tpssm11["SM_PLAN_NO"] =[{0}]", tpssm11["SM_PLAN_NO"].ToString());

			if (v_opt_flag == "1" || v_opt_flag == "3")
			{
				ret = f_pssm11_file(&inBlock2, bcls_ret, conn);
				if (ret != 0)
				{
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (v_opt_flag == "2")
			{
				ret = f_pssm11_force_file(&inBlock2, bcls_ret, conn);
				if (ret != 0)
				{
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			tpssm01_1.Reset();
			tpssm01_1["PONO"] = tpssm11["PONO"].ToString();
			tpssm01_1["PONO_STATUS"] = 91;
			tpssm01_1.MergeTo(in_23m.Tables[1]);

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT CODE_DESC_5_CONTENT FROM TEP0002  "
					"	WHERE CODE_CLASS = 'PSA62N' "
					"	  AND CODE = "
					"			(SELECT BILLET_TYPE "
					"			 FROM TPSSM02 "
					"			 WHERE CAST_LOT_NO	= @tpssm01.CAST_LOT_NO) ";
				break;
			}
			cmd_sql.SetCommandText(sqlstr);
			cmd_sql.Parameters.Clear();
			cmd_sql.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_sql.ExecuteReader();
			if (cmd_sql.Read())
			{
				v_code_desc_5_content = cmd_sql.GetString(1);
			}
			cmd_sql.Close();

			////Log::Trace("", __FUNCTION__, "v_code_desc_5_content =[{0}]", v_code_desc_5_content);

			if (v_code_desc_5_content == "P")
			{

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
				sqlstr = " SELECT PONO,SLAB_NO FROM TPSSM03 "
					" WHERE SLAB_PROD_FLAG = '0' "
					"   AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"   AND PONO = @tpssm11.PONO ";

				cmd_tpssm03_inq.SetCommandText(sqlstr);
				////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
				cmd_tpssm03_inq.Parameters.Clear();
				cmd_tpssm03_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm03_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
				cmd_tpssm03_inq.ExecuteReader();

				while (cmd_tpssm03_inq.Read())
				{
					tpssm03["PONO"] = cmd_tpssm03_inq.GetString(1);
					tpssm03["SLAB_NO"] = cmd_tpssm03_inq.GetString(2);

					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["PONO_SLAB"] = tpssm03["SLAB_NO"];
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["PONO"] = tpssm03["PONO"];

					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["EVENT_ID"] = "PSA2";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["EVENT_DESC"] = "炉次确定未产出";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["SYSTEM_ID"] = "PSSM";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["FUNC_ID"] = "f_pssm_heat_confirm";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["FORM_CODE"] = s.formname;
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["MAT_KIND"] = "SM";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["MAT_NO"] = tpssm03["SLAB_NO"];

					////Log::Trace("", __FUNCTION__, "★★★★★f_qmtqhp_dele_resv_chg_new start★★★★★");
					ret = f_qmtqhp_dele_resv_chg_new(&bcls_rec_dele_resv, &bcls_ret_dele_resv, conn);
				
					if (ret != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					sqlstr = " SELECT MAT_DESIGN_KIND,TMP_SLAB_NO "
						" FROM TPMOUHP31 t "
						" WHERE PONO_SLAB = @tpssm03.SLAB_NO ";

					cmd_tpmouhp31_inq.SetCommandText(sqlstr);
					////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
					cmd_tpmouhp31_inq.Parameters.Clear();
					cmd_tpmouhp31_inq.Parameters.Set("tpssm03.SLAB_NO", tpssm03["SLAB_NO"].ToString());
					cmd_tpmouhp31_inq.ExecuteReader();

					if (cmd_tpmouhp31_inq.Read())
					{
						tpmouhp31["MAT_DESIGN_KIND"] = cmd_tpmouhp31_inq.GetString(1);
						tpmouhp31["TMP_SLAB_NO"] = cmd_tpmouhp31_inq.GetString(2);

						bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["OUHP_MAT_TYPE"] = tpmouhp31["MAT_DESIGN_KIND"];
						bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["PONO"] = tpssm03["PONO"];
						bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["TMP_SLAB_NO"] = tpmouhp31["TMP_SLAB_NO"];
						//bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["SAMPLE_LOT_STATUS"] = "14";//12 试材回退 14命令回退
						bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["SAMPLE_LOT_STATUS"] = "12";//12 试材回退 14命令回退 与孙羽田确认，传12 2017-06-29

						doFlag = f_qmtqhp_bujt(&bcls_rec_bujt, &bcls_ret_bujt, conn);

						////Log::Trace("", __FUNCTION__, "doFlag = [{0}]", doFlag);

						if (doFlag < 0)
						{
							strcpy(s.msg, CString::Format("调用质量的试材回退函数失败。[%d][%s]", doFlag, (const char*)s.msg));
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					cmd_tpmouhp31_inq.Close();
				}
				cmd_tpssm03_inq.Close();
#endif

				sqlstr = " UPDATE TPSSM03 "
					"  SET  SLAB_PROD_FLAG = '9' "
					" WHERE SLAB_PROD_FLAG = '0' "
					"   AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"   AND PONO = @tpssm11.PONO ";

				cmd_tpssm03_upd.SetCommandText(sqlstr);
				////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
				cmd_tpssm03_upd.Parameters.Clear();
				cmd_tpssm03_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
				cmd_tpssm03_upd.ExecuteNonQuery();

				inBlock_pmconfm.Tables["PONOCONFM"].Rows.Add();
				inBlock_pmconfm.Tables["PONOCONFM"].Rows[row_num_p]["PONO"] = tpssm11["PONO"];

				inBlock_mmconfm.Tables["MMSMCONFM"].Rows.Add();
				inBlock_mmconfm.Tables["MMSMCONFM"].Rows[row_num_p]["PONO"] = tpssm11["PONO"];
				row_num_p++;
			}
			else
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = " SELECT COUNT(1) "
						" FROM TPSSM01 "
						" WHERE CAST_LOT_NO	= @tpssm01.CAST_LOT_NO "
						"   AND PONO_STATUS	< '91' ";
					break;
				}
				cmd_sql.SetCommandText(sqlstr);
				////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
				cmd_sql.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
				cmd_sql.ExecuteReader();
				if (cmd_sql.Read())
				{
					v_count = cmd_sql.GetInt32(1);
				}
				cmd_sql.Close();

				if (v_count == 0)
				{
					sqlstr = " UPDATE TPSSM03 "
						"  SET  SLAB_PROD_FLAG = '9' "
						" WHERE SLAB_PROD_FLAG = '0' "
						"   AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
						"   AND CAST_LOT_NO = @tpssm01.CAST_LOT_NO ";

					cmd_tpssm03_upd.SetCommandText(sqlstr);
					////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
					cmd_tpssm03_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm03_upd.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
					cmd_tpssm03_upd.ExecuteNonQuery();

					inBlock_mmconfm.Tables["MMSMCONFMLOT"].Rows.Add();
					inBlock_mmconfm.Tables["MMSMCONFMLOT"].Rows[row_num_l]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

					inBlock_pmconfm.Tables["LOTCONFM"].Rows.Add();
					inBlock_pmconfm.Tables["LOTCONFM"].Rows[row_num_l]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

					row_num_l++;
				}
			}

			////赋值板坯出钢记号

			//sqlstr = "SELECT JUDGE_CODE "
			//		"  FROM TQMTS23 "
			//		" WHERE HEAT_NO	= @tpssm11.HEAT_NO";

			//cmd_sql.SetCommandText(sqlstr);
			//cmd_sql.Parameters.Clear();
			//cmd_sql.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
			//cmd_sql.ExecuteReader();
			//if (cmd_sql.Read())
			//{
			//	v_judge_code = cmd_sql.GetString(1);
			//}

			//cmd_sql.Close();


			//if (tpssm11["REP_ELM_SEL_FLAG"].ToString() == "1") //代表成分选择并且预定通过
			//{
			//	if (v_judge_code.Trim() == "1")
			//	{

			//	}

			//}



			////3.向MMS发送炉次确定电文
			//inBlock3.Tables[0].set_TableName("X200000");
			//inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
			//inBlock3.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
			//inBlock3.Tables[0].Rows.Add();  //只生成一行

			//inBlock3.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
			//inBlock3.Tables[0].Rows[0]["HEAT_NO"] = tpssm11["HEAT_NO"];

			//ret = f_cm_200000_snd(&inBlock3, bcls_ret, conn);
			//if (ret != 0)
			//{
			//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
		}

#pragma region 调用函数，发送智慧质量电文
		if (in_23m.Tables[1].Rows.get_Count() > 0)
		{
			doFlag = f_t8z_23m_snd(&in_23m, bcls_ret, conn);
		}
#pragma endregion
#if defined _SYS_MES

		row_num = inBlock_pmconfm.Tables["PONOCONFM"].Rows.get_Count();

		if (row_num > 0)
		{
			doFlag = f_pmom_pono_confm(&inBlock_pmconfm, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		row_num = 0;
		row_num = inBlock_pmconfm.Tables["LOTCONFM"].Rows.get_Count();
	
		if (row_num > 0)
		{
			doFlag = f_pmom_lot_confm(&inBlock_pmconfm, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		row_num = 0;
		row_num = inBlock_mmconfm.Tables["MMSMCONFM"].Rows.get_Count();

		if (row_num > 0)
		{
			doFlag = f_mmsm_heat_confirm(&inBlock_mmconfm, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		row_num = 0;
		row_num = inBlock_mmconfm.Tables["MMSMCONFMLOT"].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, "row_num = [{0}]", row_num);
		if (row_num > 0)
		{
			doFlag = f_mmsm_heat_confirm_lot(&inBlock_mmconfm, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

#endif
		
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}
