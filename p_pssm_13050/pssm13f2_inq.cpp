/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-04-17 17:13:56  
Description: 出钢计划查询
**************************************************/

/***** C++ 的标准头文件部分 *****/ 
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/ 





/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 出钢计划查询
/// <para>
/// 1.根据pono,heat_no等条件进行出钢计划查询。
/// 
/// </para>
/// <para>数据库表：TPSSM11、TPSSM12(出钢计划主子表)     </para>
/// <para>主调用函数：前台PSSM13画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm13f2_inq)

int f_pssm13f2_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr					= "";
	CString str = "";
	CString sqlstr_count			= "";
	CString sqlstr_temp				= "";
	CString sqlstr_temp_where		= "";
	CString sqlstr2 = "";
	CString sqlstr_zpl = "";
	CString	prod_date_from="";
	CString	prod_date_to="";
	CString	start_time="";    //开始时刻
	CString	end_time = "";    //结束时刻
	CString	confrm_time="";       
	CString colname_start = " ";  //开始时间列名
	CString colname_end = " ";  //开始时间列名
	CString colname_no = " ";//处理号列名
	CString colname_no1 = " ";//铸机号和转炉号
	CString colname_flag = " ";//颜色标识列名
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag  = "0";
	CString cast_no_show = "";
	CString proc_no="";    /* 生产处理号 */    
	CString online_flag = "";
	int		TotalRecordCount		= 0  ;
	int fetchRowCount=0;
	int first_srf=0; //精炼工序的第一个charge_no
	int Z12_flag = 0;
	int ll;
	CDecimal count_z =0;
	CString end_time_real_z12 = "";
	CString colname_qz = "";
	CString start_time_real_z12 = "";
	//系统的分页类信息。
	CPageInfo pageInfo; 
	CString cs_dz_name = "";
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm12z("TPSSM12Z");
	
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm12Z_inq(conn);

	try
	{	
		
		//获取分页信息
		if (bcls_rec->Tables.Contains("PageInfo"))
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		else
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;  //每页记录数量
		}
		//设置返回块参数：主计划+时间+标志
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NOL2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "REFINE_ROUTE_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SMELT_MODE");		//为显示颜色用
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LADLE_NO");	//预处理时刻
		
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");	//转炉冶炼开始时刻
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "RUN_STATUS");	//转炉冶炼结束时刻
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_CONFM_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "RESTRAND_FLG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PLAN_TAP_WT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROD_TIME");//生产日期


		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRE_BOF_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BOF_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CCM_NO");		//为显示颜色用
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRE_DEAL_TIME");	//预处理时刻
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "P_SMELT_S_TIME");	//转炉脱P
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SMELT_START_TIME");	//转炉冶炼开始时刻
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SMELT_END_TIME");	//转炉冶炼结束时刻
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_BEGIN_TIME");	    //连铸开浇
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_END_TIME");	    //连铸开浇
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_NO"); //连模铸处理号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_NO_SHOW");

		// 取中频炉时间和炉号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY1_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY1_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY1_END_TIME"); 
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY1_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY2_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY2_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY2_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY2_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY3_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY3_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY3_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY3_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY4_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY4_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY4_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SY4_FLAG");
		// 取中频炉标记
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ1_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ1_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ1_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ1_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ2_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ2_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ2_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ2_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ3_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ3_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ3_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ3_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ4_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ4_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ4_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SZ4_FLAG");
		//为运转开始信号颜色显示用
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_PREP_FLAG");   //预处理
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRE_SMELT_FLAG");  //脱P转炉
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG"); //脱C转炉
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_FLAG");   //1重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_FLAG");   //2重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_FLAG");   //3重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_FLAG");   //4重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_FLAG");    //连铸
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "AAAA_FLAG");    //区分实际和计划
		//--------------------------------
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR5_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR5_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR5_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR5_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR6_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR6_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR6_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR6_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR7_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR7_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR7_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR7_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR8_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR8_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR8_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR8_FLAG");
		
		
		//获取传入查询条件参数
		tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		if(bcls_rec->Tables[0].Columns.Contains("ONLINE_FLAG"))
		online_flag = bcls_rec->Tables[0].Rows[0]["ONLINE_FLAG"].ToString().Trim();
	
		////Log::Info("", __FUNCTION__, "online_flag=[{0}]",online_flag);
		
		if (bcls_rec->Tables[0].Columns.Contains("START_TIME_F"))
		prod_date_from = bcls_rec->Tables[0].Rows[0]["START_TIME_F"].ToString().Trim();

		if (bcls_rec->Tables[0].Columns.Contains("START_TIME_T"))
		prod_date_to = bcls_rec->Tables[0].Rows[0]["START_TIME_T"].ToString().Trim();
			
		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "传入pssm11.PONO  =[{0}]",tpssm11.PONO );
		////Log::Info("", __FUNCTION__, "tpssm11.HEAT_NO  =[{0}]",tpssm11.HEAT_NO );
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]",tpssm11["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "cc_mach_no        = [{0}]",tpssm11["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status       = [{0}]",tpssm11["PONO_STATUS"].ToDecimal().ToInt32());
	
		////Log::Info("", __FUNCTION__, "prod_date_from       = [{0}]", prod_date_from);
		////Log::Info("", __FUNCTION__, "prod_date_to       = [{0}]", prod_date_to);

		Log::Info("", __FUNCTION__, "online_flag       = [{0}]", online_flag);

		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				if (online_flag.Trim() == "1" || online_flag.Trim() == "true")
				{
					////Log::Info("", __FUNCTION__, "查当前表online_flag1111=[{0}]",online_flag);
	
						sqlstr_count = " SELECT COUNT(1) "
							"   FROM TPSSM11 A, TPSSM12 B "
							"   WHERE 1  = 1 ";
					
						sqlstr = " SELECT A.* "
							"   FROM TPSSM11 A, TPSSM12 B "
							"   WHERE 1  = 1  ";

				}
				else
				{
				////Log::Info("", __FUNCTION__, "online_flag222=[{0}]",online_flag);
	
						sqlstr_count = " SELECT COUNT(1) "
							"   FROM TPSSM41 A, TPSSM42 B "
							"   WHERE 1  = 1 ";
					
						sqlstr = " SELECT A.* "
							"   FROM TPSSM41 A, TPSSM42 B "
							"   WHERE 1  = 1  ";
					}

				if(tpssm11["FACTORY_DIV"].ToString().Trim() != "")
				{
					sqlstr_temp += " AND A.FACTORY_DIV	= '" + tpssm11["FACTORY_DIV"].ToString().Trim() + "' ";
				}

				if(tpssm11["PONO"].ToString().Trim() != "")
				{
					sqlstr_temp	+= " AND A.PONO LIKE @tpssm11.PONO ";
				}

				if (tpssm11["HEAT_NO"].ToString().Trim() != "")
				{
					sqlstr_temp += " AND A.HEAT_NO LIKE @tpssm11.HEAT_NO ";
				}
			
				if(tpssm11["PONO_STATUS"].ToDecimal() != 0 )
				{
					sqlstr_temp	+= " AND A.PONO_STATUS = @tpssm11.PONO_STATUS "; 
				}				

				sqlstr_temp +=
					" AND A.SM_PLAN_NO = B.SM_PLAN_NO "
					" AND B.AREA_ID = 3 ";
				//" AND DECODE(B.END_TIME_REAL,' ', B.END_TIME, B.END_TIME_REAL) ";

				if (prod_date_from.Trim() != "")
				{
					sqlstr_temp	+= "AND DECODE(B.END_TIME_REAL,' ', B.END_TIME, B.END_TIME_REAL) >= @prod_date_from||'000000' ";
				}
				else
				{
					//sqlstr_temp	+= " >= DECODE(B.END_TIME_REAL,' ', B.END_TIME, B.END_TIME_REAL) ";
				}

				if (prod_date_to.Trim() != "")
				{
					sqlstr_temp	+= " AND DECODE(B.END_TIME_REAL,' ', B.END_TIME, B.END_TIME_REAL) <= @prod_date_to||'240000' ";
				}
				else
				{
					//sqlstr_temp	+=  " <= DECODE(B.END_TIME_REAL,' ', B.END_TIME, B.END_TIME_REAL) ";
				}

  			sqlstr_temp_where += " ORDER BY TO_NUMBER(A.SM_PLAN_NO) DESC ";
					
				sqlstr_count = sqlstr_count + sqlstr_temp;
				sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_where;
				break;
		}
		//cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO_STATUS",tpssm11["PONO_STATUS"].ToDecimal());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString()+"%");
		cmd_tpssm11_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString() + "%");
		cmd_tpssm11_inq.Parameters.Set("prod_date_from",prod_date_from);
		cmd_tpssm11_inq.Parameters.Set("prod_date_to",prod_date_to);

		cmd_tpssm11_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm11_inq.ExecuteScalar().ToInt32(); 
		Log::Trace("", __FUNCTION__, " sqlstr_count = [{0}]", sqlstr_count);
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, " sqlstr = [{0}]", sqlstr);
		fetchRowCount = 0;
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);//分页获取
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			//////Log::Trace("", __FUNCTION__, " fetchRowCount = [{0}]", fetchRowCount);
			tpssm11.Reset();
			cmd_tpssm11_inq.Fetch(tpssm11);

			////Log::Trace("", __FUNCTION__, " 查出tpssm11["FACTORY_DIV"] = [{0}]", tpssm11["FACTORY_DIV"].ToString());
			////Log::Trace("", __FUNCTION__, " tpssm11["PONO"] = [{0}]", tpssm11["PONO"].ToString());
			////Log::Trace("", __FUNCTION__, " tpssm11["HEAT_NO"] = [{0}]", tpssm11["HEAT_NO"].ToString());
			//返回块新增2行
			CDataRow& row1 = bcls_ret->Tables[0].Rows.Add();
			CDataRow& row2 = bcls_ret->Tables[0].Rows.Add();

			CDataRow * prow_plan = &(bcls_ret->Tables[0].Rows[fetchRowCount * 2]);
			CDataRow * prow_fact = &(bcls_ret->Tables[0].Rows[fetchRowCount * 2 + 1]);
			
			//prow_plan->Merge(tpssm11);  //计划行
			//prow_fact->Merge(tpssm11);  //实际行

			//////Log::Trace("", __FUNCTION__, " prow_plan.PONO = [{0}]", (*prow_plan)["PONO"].ToString());
			
			(*prow_plan)["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			(*prow_plan)["PONO"] = tpssm11["PONO"];
			(*prow_plan)["REFINE_ROUTE_CODE"] = tpssm11["REFINE_ROUTE_CODE"];
			(*prow_plan)["ST_NO"] = tpssm11["ST_NO"];
			////Log::Trace("", __FUNCTION__, " prow_plan.PONO = [{0}]", (*prow_plan)["PONO"].ToString());

			(*prow_plan)["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			(*prow_plan)["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
			(*prow_plan)["HEAT_NO"] = tpssm11["HEAT_NO"];
			(*prow_plan)["SMELT_MODE"] = tpssm11["SMELT_MODE"];
			(*prow_plan)["LADLE_NO"] = tpssm11["LADLE_NO"];
			
			(*prow_plan)["PONO_STATUS"] = tpssm11["PONO_STATUS"];
			(*prow_plan)["RUN_STATUS"] = tpssm11["RUN_STATUS"];			
			(*prow_plan)["HEAT_CONFM_TIME"] = tpssm11["HEAT_CONFM_TIME"];
			
			(*prow_plan)["RESTRAND_FLG"] = tpssm11["RESTRAND_FLG"];
			(*prow_plan)["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
			(*prow_plan)["PLAN_TAP_WT"] = tpssm11["PLAN_TAP_WT"];
			
			(*prow_fact)["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			(*prow_fact)["PONO"] = tpssm11["PONO"];
			(*prow_fact)["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			(*prow_fact)["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
			(*prow_fact)["HEAT_NO"] = tpssm11["HEAT_NO"];
			(*prow_fact)["SMELT_MODE"] = tpssm11["SMELT_MODE"];
			(*prow_fact)["LADLE_NO"] = tpssm11["LADLE_NO"];
			(*prow_fact)["REFINE_ROUTE_CODE"] = tpssm11["REFINE_ROUTE_CODE"];
			(*prow_fact)["ST_NO"] = tpssm11["ST_NO"];
			(*prow_fact)["PONO_STATUS"] = tpssm11["PONO_STATUS"];
			(*prow_fact)["RUN_STATUS"] = tpssm11["RUN_STATUS"];
			(*prow_fact)["HEAT_CONFM_TIME"] = tpssm11["HEAT_CONFM_TIME"];
			//(*prow_fact)["TD_CHG_FLAG"] = tpssm11.TD_CHG_FLAG;
			(*prow_fact)["RESTRAND_FLG"] = tpssm11["RESTRAND_FLG"];
			(*prow_fact)["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
			(*prow_fact)["PLAN_TAP_WT"] = tpssm11["PLAN_TAP_WT"];
			
			(*prow_plan)["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();

			(*prow_fact)["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();
			
			(*prow_plan)["AAAA_FLAG"] = "计划";
			(*prow_fact)["AAAA_FLAG"] = "实际";

			//精炼工序的第一个charge_no 必定不为0
			first_srf = 0;
			tpssm12.Reset();
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				if (online_flag.Trim() == "1")
				{
					////Log::Info("", __FUNCTION__, "online_flag333=[{0}]",online_flag);
	
					sqlstr2 = " SELECT * FROM TPSSM12 "
							 " WHERE FACTORY_DIV	= @tpssm11.FACTORY_DIV "
							 " AND SM_PLAN_NO			= @tpssm11.HEAT_NO "
							 " ORDER BY CHARGE_NO ASC ";

				}
				else
				{
					////Log::Info("", __FUNCTION__, "online_flag444=[{0}]",online_flag);
	
					sqlstr2 = " SELECT * FROM TPSSM42 "
							 " WHERE FACTORY_DIV	= @tpssm11.FACTORY_DIV "
							 " AND SM_PLAN_NO			= @tpssm11.HEAT_NO "
							 " ORDER BY CHARGE_NO ASC ";
				}
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr2);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.HEAT_NO",tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();

			while(cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
				tpssm12.TrimOrBlank();

				////Log::Trace("",__FUNCTION__," tpssm12["START_TIME_REAL"] = [{0}]",tpssm12["START_TIME_REAL"].ToString());
				////Log::Trace("",__FUNCTION__," tpssm12["START_TIME"] = [{0}]",tpssm12["START_TIME"].ToString());
				////Log::Trace("",__FUNCTION__," tpssm12["END_TIME_REAL"] = [{0}]",tpssm12["END_TIME_REAL"].ToString());
				
				(*prow_plan)["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();

				(*prow_fact)["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();
				//判断实绩是否接收过,以置实绩标志和取实绩时刻
				if (tpssm12["END_TIME_REAL"].ToString().Compare(" ") == 0)//实绩结束时刻没送来
				{
					if (tpssm12["START_TIME_REAL"].ToString().Compare(" ") == 0)//实绩开始时刻没送来
					{
						start_time = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
						proc_no = tpssm12["PRE_PROC_NO"];
						show_flag = "0";  //实绩无
					}
					else
					{
						start_time = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
						proc_no = tpssm12["PROC_NO"];
						show_flag = "1";  //开始
					}
					end_time = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
				}
				else  //实绩结束时刻有
				{
					if (tpssm12["START_TIME_REAL"].ToString().Compare(" ") == 0)//实绩开始时刻没送来
					{
						start_time = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
					}
					else
					{
						start_time = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
					}
					end_time = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
					show_flag = "2";  //实绩有
					proc_no = tpssm12["PROC_NO"];
				}

				////Log::Trace("", __FUNCTION__, " show_flag= [{0}]", show_flag);
				str_show_flag = show_flag;

				////Log::Trace("", __FUNCTION__, "str_show_flag=[{0}]", str_show_flag);
				////Log::Trace("", __FUNCTION__, "tpssm12["AREA_ID"] =[{0}]", tpssm12["AREA_ID"].ToDecimal());

				colname_start = "";
				colname_end = "";
				colname_flag = "";
				colname_no = "";
				colname_no1 = "";
				//根据计划子表的内容, 确定记录写入BLOCK的列名
				switch (tpssm12["AREA_ID"].ToDecimal().ToInt32())
				{
					case 1: //预处理
						colname_start = "PRE_DEAL_TIME";
						colname_flag = "MAT_PREP_FLAG"; 

						(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
						(*prow_fact)[colname_start] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
						break;
					case 2: //电炉
						cs_dz_name="SY"+tpssm12["CHARGE_NO"].ToDecimal().ToString();
						colname_start = cs_dz_name+"_TIME";
						colname_end = cs_dz_name + "_END_TIME";
						colname_flag = cs_dz_name + "_FLAG";
						colname_no = cs_dz_name + "_NO";
						
						(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString().SubstringNE(8, 2) + ":" + tpssm12["START_TIME"].ToString().SubstringNE(10, 2);
						(*prow_plan)[colname_end] = tpssm12["END_TIME"].ToString().SubstringNE(8, 2) + ":" + tpssm12["END_TIME"].ToString().SubstringNE(10, 2);
						if (tpssm12["START_TIME_REAL"].ToString().Trim() != "" && tpssm12["START_TIME_REAL"].ToString().GetLength()>0)
						{
							(*prow_fact)[colname_start] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 2) + ":" + tpssm12["START_TIME_REAL"].ToString().SubstringNE(10, 2);
						}
						else
						{
							(*prow_fact)[colname_start] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
						}

						if (tpssm12["END_TIME_REAL"].ToString().Trim() != "" && tpssm12["END_TIME_REAL"].ToString().GetLength() > 0)
						{
							(*prow_fact)[colname_end] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 2) + ":" + tpssm12["END_TIME_REAL"].ToString().SubstringNE(10, 2);
						}
						else
						{
							(*prow_fact)[colname_end] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
						(*prow_plan)[cs_dz_name + "_NO"] = tpssm12["DEV_CODE"].ToString();
						(*prow_fact)[cs_dz_name + "_NO"] = tpssm12["DEV_CODE"].ToString();
						(*prow_plan)[cs_dz_name + "_FLAG"] = show_flag;
						(*prow_fact)[cs_dz_name + "_FLAG"] = show_flag;
						break;
					case 3: //转炉区
						colname_start = "SMELT_START_TIME";
						colname_end = "SMELT_END_TIME";
						colname_flag = "MAIN_SMELT_FLAG";
						colname_no1 = "BOF_NO";
						
						(*prow_fact)["PROD_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(0, 8);	//取冶炼开始时刻作为生产日期
						(*prow_plan)["PROD_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(0, 8);

						break;
					case 4: //精炼区域
						if (first_srf == 0)
						{
							first_srf = tpssm12["CHARGE_NO"].ToDecimal().ToInt32();
						}				
						str = "SR" + CDecimal(tpssm12["CHARGE_NO"].ToDecimal().ToInt32() - first_srf + 1).ToString() ;
						colname_start = str + "_TIME";
						colname_end = str + "_END_TIME";
						colname_no = str + "_NO";
						colname_flag = colname_flag.Format("sr%.1d_flag", tpssm12["CHARGE_NO"].ToDecimal().ToInt32() - first_srf + 1);						
						
						break;
					case 5: //浇铸
						colname_start = "CC_BEGIN_TIME";
						colname_end = "CC_END_TIME";
						colname_flag = "CC_FLAG";
						colname_no =  "CC_NO";//处理号
						colname_no1 = "CCM_NO";//铸机号	
					/*	(*prow_plan)["CC_NO"] = tpssm12["PRE_PROC_NO"];						
						(*prow_fact)["CC_NO"] = tpssm12["PRE_PROC_NO"];*/

						break;
				}
				////Log::Trace("", __FUNCTION__, " 处理号colname_no= [{0}]", colname_no);
				//处理号
				if (colname_no.Trim()!="" && colname_no.GetLength() > 0)
				{
					(*prow_plan)[colname_no] = proc_no;
					(*prow_fact)[colname_no] = proc_no;
				}
				//转炉号，铸机号
				if (colname_no1.Trim() != "" && colname_no1.GetLength() > 0)
				{
					(*prow_plan)[colname_no1] = tpssm12["DEV_CODE"].ToString().Substring(1);
					(*prow_fact)[colname_no1] = tpssm12["DEV_CODE"].ToString().Substring(1);

				}
				////Log::Trace("", __FUNCTION__, " 转炉号colname_no1= [{0}]", colname_no1);

				if (tpssm12["AREA_ID"].ToDecimal().ToInt32() >= 3)
				{			
					//计划开始时刻和结束时刻
					if (tpssm12["START_TIME"].ToString().Trim() != "" )
					{
						(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString().SubstringNE(8, 2) + ":" + tpssm12["START_TIME"].ToString().SubstringNE(10, 2);
					}
					else
					{
						(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
					}
				

					if (tpssm12["END_TIME"].ToString().Trim() != "" )
					{
						(*prow_plan)[colname_end] = tpssm12["END_TIME"].ToString().SubstringNE(8, 2) + ":" + tpssm12["END_TIME"].ToString().SubstringNE(10, 2);
					}
					else
					{
						(*prow_plan)[colname_end] = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
					}
				
					//Log::Trace("", __FUNCTION__, " 计划开始时刻和结束时刻= [{0}]", colname_start, colname_end);
				
					//实绩开始时刻和结束时刻
				
					if (tpssm12["START_TIME_REAL"].ToString().Trim() != "" && tpssm12["START_TIME_REAL"].ToString().GetLength()>0)
					{
						(*prow_fact)[colname_start] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 2) + ":" + tpssm12["START_TIME_REAL"].ToString().SubstringNE(10, 2);
					}
					else
					{
						(*prow_fact)[colname_start] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
					}
				
					if (tpssm12["END_TIME_REAL"].ToString().Trim() != "" && tpssm12["END_TIME_REAL"].ToString().GetLength() > 0)
					{
						(*prow_fact)[colname_end] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 2) + ":" + tpssm12["END_TIME_REAL"].ToString().SubstringNE(10, 2);
					}
					else
					{
						(*prow_fact)[colname_end] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
					}
				}
				
				////Log::Trace("", __FUNCTION__, " 实绩开始时刻和结束时刻= [{0}]", colname_start, colname_end);

				//标志
				(*prow_plan)[colname_flag] = str_show_flag;
				(*prow_fact)[colname_flag] = str_show_flag;
								
				////Log::Trace("", __FUNCTION__, " colname_start= [{0}]", colname_start);
				////Log::Trace("",__FUNCTION__," start_time = [{0}]",start_time);

				////Log::Trace("", __FUNCTION__, " colname_end= [{0}]", colname_end);
				////Log::Trace("", __FUNCTION__, " end_time = [{0}]", end_time);
			}
			cmd_tpssm12_inq.Close();
			// 查询中频炉信息，并且获取关键数据
			if (tpssm11["HEAT_NO"].ToString().Trim() != "")
			{
				sqlstr_zpl = " SELECT START_TIME,END_TIME,START_TIME_REAL,END_TIME_REAL,PROC_NO  FROM TPSSM12Z WHERE HEAT_NO			= @tpssm11.HEAT_NO ORDER BY ID_SJ ASC ";
				cmd_tpssm12Z_inq.SetCommandText(sqlstr_zpl);
				cmd_tpssm12Z_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
				Log::Trace("", __FUNCTION__, " LXX1 = [{0}]", tpssm11["HEAT_NO"].ToString());
				cmd_tpssm12Z_inq.ExecuteReader();
				count_z = 0;
				while (cmd_tpssm12Z_inq.Read())
				{
					cmd_tpssm12Z_inq.Fetch(tpssm12z);
					start_time_real_z12 = " ";
					end_time_real_z12 = " ";
					if (tpssm12z["START_TIME_REAL"].ToString().Trim() != "" && tpssm12z["START_TIME_REAL"].ToString().GetLength() > 0)
					{
						start_time_real_z12 = tpssm12z["START_TIME_REAL"].ToString();
						Z12_flag = 1;
					}
					if (tpssm12z["END_TIME_REAL"].ToString().Trim() != "" && tpssm12z["END_TIME_REAL"].ToString().GetLength() > 0)
					{
						end_time_real_z12 = tpssm12z["START_TIME_REAL"].ToString();
						Z12_flag = 2;
					}
					colname_flag = "SZ" + (count_z + 1).ToString() + "_FLAG";
					// 此为前缀名称，根据时间匹配到对应的字段
					colname_qz = "SZ" + (count_z + 1).ToString();
					Log::Trace("", __FUNCTION__, " LXX1 = [{0}]", count_z + 2);
					(*prow_plan)[colname_flag] = Z12_flag;
					(*prow_fact)[colname_flag] = Z12_flag;
					(*prow_plan)[colname_qz + "_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 2) + ":" + tpssm12["START_TIME"].ToString().SubstringNE(10, 2);
					if (start_time_real_z12.Trim()=="")
					{
						(*prow_fact)[colname_qz + "_TIME"] = start_time_real_z12;
					}
					else
					{
						(*prow_fact)[colname_qz + "_TIME"] = start_time_real_z12.SubstringNE(8, 2) + ":" + start_time_real_z12.SubstringNE(10, 2);
					}
					if (end_time_real_z12.Trim() == "")
					{
						(*prow_fact)[colname_qz + "_END_TIME"] = end_time_real_z12;
					}
					else
					{
						(*prow_fact)[colname_qz + "_END_TIME"] = end_time_real_z12.SubstringNE(8, 2) + ":" + end_time_real_z12.SubstringNE(10, 2);
					}
					(*prow_plan)[colname_qz + "_END_TIME"] = tpssm12["END_TIME"].ToString().SubstringNE(8, 2) + ":" + tpssm12["END_TIME"].ToString().SubstringNE(10, 2);
					
					(*prow_plan)[colname_qz + "_NO"] = tpssm12z["PROC_NO"].ToString();
					(*prow_fact)[colname_qz + "_NO"] = tpssm12z["PROC_NO"].ToString();
					count_z = count_z + 1;
				}
			}
			cmd_tpssm12Z_inq.Close();
			fetchRowCount++;
			
		}
		cmd_tpssm11_inq.Close();
		 Log::Trace("", __FUNCTION__, " 计划开始时刻和结束时刻= [{0}]", bcls_ret->Tables[0].Rows[0]["SMELT_END_TIME"].ToString());
			
		/*设置系统返回参数*/
		{
			//_RES("GCRSS0000004")//查询到[{0}]条记录。
			CFormattable arguments[] = {fetchRowCount}; // 定义参数列表的数组
			CMessageFormat::Format(s.msg,  _RES("GCRSS0000004"),	arguments, 1); //格式化字符串 
		}

		////Log::Trace("", __FUNCTION__, " TotalRecordCount= [{0}]", TotalRecordCount);
		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL,"TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;	

	 }
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
