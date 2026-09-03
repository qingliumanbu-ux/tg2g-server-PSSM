/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-05-29 17:13:56
Description: 出钢计划查询 - 在线
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
BM2F_ENTERACE(pssm131f2_inq)

int f_pssm131f2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString str = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_where = "";
	CString sqlstr2 = "";
	CString	prod_date_from = "";
	CString	prod_date_to = "";
	CString	start_time = "";    //开始时刻
	CString	end_time = "";    //结束时刻
	CString	confrm_time = "";
	CString colname_start = " ";  //开始时间列名
	CString colname_end = " ";  //开始时间列名
	CString colname_no = " ";//处理号列名
	CString colname_no1 = " ";//铸机号和转炉号
	CString colname_flag = " ";//颜色标识列名
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag = "0";
	CString cast_no_show = "";
	CString proc_no = "";    /* 生产处理号 */
	CString online_flag = "";
	int		TotalRecordCount = 0;
	int fetchRowCount = 0;
	int first_srf = 0; //精炼工序的第一个charge_no

	int ll;


	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");


	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);


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


		//为运转开始信号颜色显示用
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_PREP_FLAG");   //预处理
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRE_SMELT_FLAG");  //脱P转炉
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG"); //脱C转炉
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_FLAG");   //1重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_FLAG");   //2重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_FLAG");   //3重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_FLAG");   //4重精炼
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_FLAG");    //连铸

		//附件数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_D_COUNT");   //转炉
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_C_COUNT");   //连铸
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_A_COUNT");   //厂级
		//--------------------------------



		//获取传入查询条件参数
		tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		online_flag = "1";

		////Log::Info("", __FUNCTION__, "online_flag=[{0}]",online_flag);

		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "传入pssm11.PONO  =[{0}]",tpssm11.PONO );
		////Log::Info("", __FUNCTION__, "tpssm11.HEAT_NO  =[{0}]",tpssm11.HEAT_NO );
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]",tpssm11["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "cc_mach_no        = [{0}]",tpssm11["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status       = [{0}]",tpssm11["PONO_STATUS"].ToDecimal().ToInt32());

		////Log::Info("", __FUNCTION__, "prod_date_from       = [{0}]", prod_date_from);

		////Log::Info("", __FUNCTION__, "prod_date_to       = [{0}]", prod_date_to);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			if (online_flag.Trim() == "1")
			{
				////Log::Info("", __FUNCTION__, "查当前表online_flag1111=[{0}]",online_flag);

				sqlstr_count = " SELECT COUNT(1) "
					"   FROM TPSSM11 A, TPSSM12 B "
					"   WHERE 1  = 1 ";

				sqlstr = " SELECT STDD.ST_COUNT ST_D_COUNT, STDC.ST_COUNT ST_C_COUNT, STDA.ST_COUNT ST_A_COUNT, A.* "
					"   FROM TPSSM11 A, TPSSM12 B "
					" ,(select STA.ST_NO, count(STA.ST_NO) ST_COUNT from TQMTS10X STA,TQMTS10 STB where STA.SEQ_NO=STB.SEQ_NO AND WHOLE_BACKLOG_CODE='D' group by STA.ST_NO) STDD"
					" ,(select STA.ST_NO, count(STA.ST_NO) ST_COUNT from TQMTS10X STA,TQMTS10 STB where STA.SEQ_NO=STB.SEQ_NO AND WHOLE_BACKLOG_CODE='C' group by STA.ST_NO) STDC"
					" ,(select STA.ST_NO, count(STA.ST_NO) ST_COUNT from TQMTS10X STA,TQMTS10 STB where STA.SEQ_NO=STB.SEQ_NO AND WHOLE_BACKLOG_CODE='A' group by STA.ST_NO) STDA"
					"   WHERE 1  = 1  AND A.ST_NO = STDD.ST_NO(+) AND A.ST_NO = STDC.ST_NO(+) AND A.ST_NO = STDA.ST_NO(+) ";

			}			

			if (tpssm11["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND A.FACTORY_DIV	= '" + tpssm11["FACTORY_DIV"].ToString().Trim() + "' ";
			}
			if (tpssm11["PONO_STATUS"].ToDecimal() != 0)
			{
				sqlstr_temp += " AND A.PONO_STATUS = @tpssm11.PONO_STATUS ";
			}

			sqlstr_temp +=
				" AND A.SM_PLAN_NO = B.SM_PLAN_NO "
				" AND B.AREA_ID = 3 ";
			//" AND DECODE(B.END_TIME_REAL,' ', B.END_TIME, B.END_TIME_REAL) ";

			sqlstr_temp_where += " ORDER BY TO_NUMBER(A.SM_PLAN_NO) DESC ";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_where;
			break;
		}
		//cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO_STATUS", tpssm11["PONO_STATUS"].ToDecimal());

		cmd_tpssm11_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm11_inq.ExecuteScalar().ToInt32();
		////Log::Trace("", __FUNCTION__, " sqlstr_count = [{0}]", sqlstr_count);
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		////Log::Trace("", __FUNCTION__, " sqlstr = [{0}]", sqlstr);
		fetchRowCount = 0;
		//cmd_tpssm11_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);//分页获取
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			//////Log::Trace("", __FUNCTION__, " fetchRowCount = [{0}]", fetchRowCount);
			//tpssm11.Reset();
			cmd_tpssm11_inq.Fetch(tpssm11);

			////Log::Trace("", __FUNCTION__, " 查出tpssm11["FACTORY_DIV"] = [{0}]", tpssm11["FACTORY_DIV"].ToString());
			////Log::Trace("", __FUNCTION__, " tpssm11["PONO"] = [{0}]", tpssm11["PONO"].ToString());
			////Log::Trace("", __FUNCTION__, " tpssm11["HEAT_NO"] = [{0}]", tpssm11["HEAT_NO"].ToString());
			//返回块新增1行
			CDataRow& row1 = bcls_ret->Tables[0].Rows.Add();

			CDataRow * prow_plan = &(bcls_ret->Tables[0].Rows[fetchRowCount]);

			//////Log::Trace("", __FUNCTION__, " prow_plan.PONO = [{0}]", (*prow_plan)["PONO"].ToString());
			(*prow_plan)["ST_D_COUNT"] = cmd_tpssm11_inq.GetString(1);
			(*prow_plan)["ST_C_COUNT"] = cmd_tpssm11_inq.GetString(2);
			(*prow_plan)["ST_A_COUNT"] = cmd_tpssm11_inq.GetString(3);

			(*prow_plan)["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			(*prow_plan)["PONO"] = tpssm11["PONO"];
			(*prow_plan)["REFINE_ROUTE_CODE"] = tpssm11["REFINE_ROUTE_CODE"];
			(*prow_plan)["ST_NO"] = tpssm11["ST_NO"];
			////Log::Trace("", __FUNCTION__, " prow_plan.PONO = [{0}]", (*prow_plan)["PONO"].ToString());

			(*prow_plan)["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			(*prow_plan)["HEAT_NO"] = tpssm11["HEAT_NO"];
			(*prow_plan)["SMELT_MODE"] = tpssm11["SMELT_MODE"];
			(*prow_plan)["LADLE_NO"] = tpssm11["LADLE_NO"];

			(*prow_plan)["PONO_STATUS"] = tpssm11["PONO_STATUS"];
			(*prow_plan)["RUN_STATUS"] = tpssm11["RUN_STATUS"];
			(*prow_plan)["HEAT_CONFM_TIME"] = tpssm11["HEAT_CONFM_TIME"];

			(*prow_plan)["RESTRAND_FLG"] = tpssm11["RESTRAND_FLG"];
			(*prow_plan)["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
			(*prow_plan)["PLAN_TAP_WT"] = tpssm11["PLAN_TAP_WT"];
			(*prow_plan)["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();

			//精炼工序的第一个charge_no 必定不为0
			first_srf = 0;

			switch (conn->DatabaseKind)
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
						" AND HEAT_NO			= @tpssm11.HEAT_NO "
						" ORDER BY CHARGE_NO ASC ";

				}
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr2);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();

			while (cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
				tpssm12.TrimOrBlank();

				////Log::Trace("",__FUNCTION__," tpssm12["START_TIME_REAL"] = [{0}]",tpssm12["START_TIME_REAL"].ToString());
				////Log::Trace("",__FUNCTION__," tpssm12["START_TIME"] = [{0}]",tpssm12["START_TIME"].ToString());
				////Log::Trace("",__FUNCTION__," tpssm12["END_TIME_REAL"] = [{0}]",tpssm12["END_TIME_REAL"].ToString());

				(*prow_plan)["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();
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

					(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString();
					break;
				case 2: //脱磷
					colname_start = "P_SMELT_S_TIME";
					colname_flag = "PRE_SMELT_FLAG";

					(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString();
					(*prow_plan)["PRE_BOF_NO"] = tpssm12["DEV_CODE"].ToString();
					break;
				case 3: //转炉区
					colname_start = "SMELT_START_TIME";
					colname_end = "SMELT_END_TIME";
					colname_flag = "MAIN_SMELT_FLAG";
					colname_no1 = "BOF_NO";

					(*prow_plan)["PROD_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(0, 8);	//取冶炼开始时刻作为生产日期

					break;
				case 4: //精炼区域
					if (first_srf == 0)
					{
						first_srf = tpssm12["CHARGE_NO"].ToDecimal().ToInt32();
					}
					str = "SR" + CDecimal(tpssm12["CHARGE_NO"].ToDecimal().ToInt32() - first_srf + 1).ToString();
					colname_start = str + "_TIME";
					colname_end = str + "_END_TIME";
					colname_no = str + "_NO";
					colname_flag = colname_flag.Format("sr%.1d_flag", tpssm12["CHARGE_NO"].ToDecimal().ToInt32() - first_srf + 1);

					break;
				case 5: //浇铸
					colname_start = "CC_BEGIN_TIME";
					colname_end = "CC_END_TIME";
					colname_flag = "CC_FLAG";
					colname_no = "CC_NO";//处理号
					colname_no1 = "CCM_NO";//铸机号	

					break;
				}
				////Log::Trace("", __FUNCTION__, " 处理号colname_no= [{0}]", colname_no);
				//处理号
				if (colname_no.Trim() != "" && colname_no.GetLength() > 0)
				{
					(*prow_plan)[colname_no] = proc_no;
				}
				//转炉号，铸机号
				if (colname_no1.Trim() != "" && colname_no1.GetLength() > 0)
				{
					(*prow_plan)[colname_no1] = tpssm12["DEV_CODE"].ToString().Substring(1);

				}
				////Log::Trace("", __FUNCTION__, " 转炉号colname_no1= [{0}]", colname_no1);

				if (tpssm12["AREA_ID"].ToDecimal().ToInt32() >= 3)
				{
					//计划开始时刻和结束时刻
					if (tpssm12["START_TIME"].ToString().Trim() != "")
					{
						(*prow_plan)[colname_start] = tpssm12["START_TIME"].ToString();
					}


					if (tpssm12["END_TIME"].ToString().Trim() != "")
					{
						(*prow_plan)[colname_end] = tpssm12["END_TIME"].ToString();
					}

					////Log::Trace("", __FUNCTION__, " 计划开始时刻和结束时刻= [{0}]", colname_start, colname_end);

					//实绩开始时刻和结束时刻

					if (tpssm12["START_TIME_REAL"].ToString().Trim() != "" && tpssm12["START_TIME_REAL"].ToString().GetLength()>0)
					{
						(*prow_plan)[colname_start] = tpssm12["START_TIME_REAL"].ToString();
					}

					if (tpssm12["END_TIME_REAL"].ToString().Trim() != "" && tpssm12["END_TIME_REAL"].ToString().GetLength() > 0)
					{
						(*prow_plan)[colname_end] = tpssm12["END_TIME_REAL"].ToString();
					}
				}

				////Log::Trace("", __FUNCTION__, " 实绩开始时刻和结束时刻= [{0}]", colname_start, colname_end);

				//标志
				(*prow_plan)[colname_flag] = str_show_flag;

				////Log::Trace("", __FUNCTION__, " colname_start= [{0}]", colname_start);
				////Log::Trace("",__FUNCTION__," start_time = [{0}]",start_time);
			}
			cmd_tpssm12_inq.Close();

			fetchRowCount++;

		}
		cmd_tpssm11_inq.Close();


		/*设置系统返回参数*/
		{
			//_RES("GCRSS0000004")//查询到[{0}]条记录。
			CFormattable arguments[] = { fetchRowCount }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004"), arguments, 1); //格式化字符串 
		}

		////Log::Trace("", __FUNCTION__, " TotalRecordCount= [{0}]", TotalRecordCount);
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
