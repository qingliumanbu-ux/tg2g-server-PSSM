/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      
Version:     1.0
Date:        2021/9/6 10:08:19
Description: 炉次成分信息查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** 头文件部分 *****/
//#include "tpssm10.h"

//#include "AppFunc.h"

/*<remark>=========================================================
/// <summary>
///  一系列生产计划查询
/// <para>
/// </para>
/// <para>数据库表：</para>
/// </summary>
/// <param name="">  </param>
/// <returns>返回参数：材料数据</returns>
===========================================================</remark>*/
// Service 入口
BM2F_ENTERACE(pssmap02_inq)

int f_pssmap02_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString v_heat_no("");
	//CString v_proc_no("");
	CDecimal  o_value_aim = 0;
	CDecimal o_value_act = 0;
	CString o_judge_result("");

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString o_elm_code("");
	
	/* 实体类定义 */
	//CTPSSM10 tpssm10(conn);


	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	//AppFunc XYZ(bcls_rec, bcls_ret, conn);

	try
	{
		//---------------------------------------------------
		//获得输入参数
		//v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		//v_date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().Trim();
		v_heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString().Trim();
		//v_proc_no = bcls_rec->Tables[0].Rows[0]["PROC_NO"].ToString().Trim();


		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "====== 接收块开始v_heat_no[{0}] =======  ", v_heat_no);


		//---------------------------------------------------
		//设置返回行参数
		//bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPEC_C4");

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "C_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "C_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "C_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SI_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SI_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SI_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MN_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MN_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MN_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "P_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "P_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "P_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "S_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "S_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "S_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "V_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "V_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "V_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AL_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AL_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "AL_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "ALS_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "ALS_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ALS_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CEQ_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CEQ_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CEQ_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TI_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "TI_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "TI_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MO_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MO_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MO_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CU_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CU_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CU_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NI_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NI_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NI_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CR_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CR_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CR_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CA_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "CA_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CA_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "B_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "B_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "B_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NB_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NB_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NB_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AS_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AS_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "AS_JUDGE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N_VALUE_AIM");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "N_VALUE_ACT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "N_JUDGE");

		//开始查询
		//CDataRow & bcls_ret->Tables[0].Rows[0] = bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows.Add();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:						// 所有数据库适用，通用SQL语句
			sqlstr = "SELECT A.ST_NO,A.ELM_CODE,NVL(B.MAIN_AIM,0),NVL(A.ELM_VALUE,0),NVL(C.CODE_DESC_1_CONTENT,'未判') "
				" FROM TQMTS29 A "
				" LEFT JOIN TQMTS02 B ON A.ST_NO = B.ST_NO AND A.ELM_CODE = B.ELM_CODE AND B.WHOLE_BACKLOG_CODE = 'G' "
				" LEFT JOIN TEP0002 C ON C.CODE_CLASS = 'QM1I' AND A.ELM_OK = C.CODE "
				" WHERE A.HEAT_NO = @v_heat_no "
				;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_heat_no", v_heat_no);
		Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		Log::Trace("", __FUNCTION__, "v_heat_no[{0}]", v_heat_no);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			bcls_ret->Tables[0].Rows[0]["ST_NO"] = cmd_inq.GetString(1);
			o_elm_code = cmd_inq.GetString(2);
			o_value_aim = cmd_inq.GetDecimal(3);
			o_value_act = cmd_inq.GetDecimal(4);
			o_judge_result = cmd_inq.GetString(5);
			Log::Trace("", __FUNCTION__, "SQL查询结果o_elm_code = [{0}],o_value_aim = [{1}],o_value_act=[{2}],o_judge_result=[{3}]", o_elm_code, o_value_aim, o_value_act, o_judge_result);
			if (o_elm_code == "012")
			{
				bcls_ret->Tables[0].Rows[0]["C_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["C_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["C_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__, "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["C_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["C_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["C_JUDGE"].ToString());
			}
			else if (o_elm_code == "028")
			{
				bcls_ret->Tables[0].Rows[0]["SI_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["SI_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["SI_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__, "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["SI_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["SI_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["SI_JUDGE"].ToString());
			}
			else if (o_elm_code == "055")
			{
				bcls_ret->Tables[0].Rows[0]["MN_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["MN_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["MN_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__, "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["MN_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MN_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MN_JUDGE"].ToString());
			}
			else if (o_elm_code == "030")
			{
				bcls_ret->Tables[0].Rows[0]["P_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["P_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["P_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["P_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["P_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["P_JUDGE"].ToString());
			}
			else if (o_elm_code == "032")
			{
				bcls_ret->Tables[0].Rows[0]["S_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["S_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["S_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["S_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["S_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["S_JUDGE"].ToString());
			}
			else if (o_elm_code == "051")
			{
				bcls_ret->Tables[0].Rows[0]["V_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["V_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["V_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["V_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["V_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["V_JUDGE"].ToString());
			}
			else if (o_elm_code == "027")
			{
				bcls_ret->Tables[0].Rows[0]["AL_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["AL_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["AL_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["AL_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AL_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AL_JUDGE"].ToString());
			}
			else if (o_elm_code == "211")
			{
				bcls_ret->Tables[0].Rows[0]["ALS_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["ALS_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["ALS_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["ALS_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["ALS_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["ALS_JUDGE"].ToString());
			}
			else if (o_elm_code == "C01")
			{
				bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["CEQ_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__, "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CEQ_JUDGE"].ToString());
			}
			else if (o_elm_code == "C02")
			{
				bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["CEQ_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CEQ_JUDGE"].ToString());
			}
			else if (o_elm_code == "048")
			{
				bcls_ret->Tables[0].Rows[0]["TI_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["TI_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["TI_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["TI_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["TI_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["TI_JUDGE"].ToString());
			}
			else if (o_elm_code == "096")
			{
				bcls_ret->Tables[0].Rows[0]["MO_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["MO_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["MO_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["MO_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MO_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MO_JUDGE"].ToString());
			}
			else if (o_elm_code == "064")
			{
				bcls_ret->Tables[0].Rows[0]["CU_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["CU_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["CU_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["CU_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CU_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CU_JUDGE"].ToString());
			}
			else if (o_elm_code == "058")
			{
				bcls_ret->Tables[0].Rows[0]["NI_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["NI_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["NI_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["NI_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NI_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NI_JUDGE"].ToString());
			}
			else if (o_elm_code == "052")
			{
				bcls_ret->Tables[0].Rows[0]["CR_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["CR_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["CR_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["CR_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CR_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CR_JUDGE"].ToString());
			}
			else if (o_elm_code == "042")
			{
				bcls_ret->Tables[0].Rows[0]["CA_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["CA_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["CA_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["CA_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CA_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CA_JUDGE"].ToString());
			}
			else if (o_elm_code == "011")
			{
				bcls_ret->Tables[0].Rows[0]["B_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["B_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["B_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["B_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["B_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["B_JUDGE"].ToString());
			}
			else if (o_elm_code == "093")
			{
				bcls_ret->Tables[0].Rows[0]["NB_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["NB_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["NB_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["NB_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NB_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NB_JUDGE"].ToString());
			}
			else if (o_elm_code == "075")
			{
				bcls_ret->Tables[0].Rows[0]["AS_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["AS_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["AS_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["AS_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AS_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AS_JUDGE"].ToString());
			}
			else if (o_elm_code == "014")
			{
				bcls_ret->Tables[0].Rows[0]["N_VALUE_AIM"] = o_value_aim;
				bcls_ret->Tables[0].Rows[0]["N_VALUE_ACT"] = o_value_act;
				bcls_ret->Tables[0].Rows[0]["N_JUDGE"] = o_judge_result;
				Log::Trace("", __FUNCTION__,  "返回行aim = [{0}],act=[{1}],result=[{2}]", bcls_ret->Tables[0].Rows[0]["N_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["N_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["N_JUDGE"].ToString());
			}
		}
		cmd_inq.Close();
		Log::Trace("", __FUNCTION__, "返回块C_VALUE_AIM = [{0}],C_VALUE_ACT=[{1}],C_VALUE_ACT=[{2}]", bcls_ret->Tables[0].Rows[0]["C_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["C_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["C_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块SI_VALUE_AIM = [{0}],SI_VALUE_ACT=[{1}],SI_VALUE_ACT=[{2}]", bcls_ret->Tables[0].Rows[0]["SI_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["SI_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["SI_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块MN_VALUE_AIM = [{0}],MN_VALUE_ACT=[{1}],MN_VALUE_ACT=[{2}]", bcls_ret->Tables[0].Rows[0]["MN_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MN_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MN_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块P_VALUE_AIM = [{0}],P_VALUE_ACT=[{1}],P_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["P_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["P_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["P_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块S_VALUE_AIM = [{0}],S_VALUE_ACT=[{1}],S_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["S_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["S_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["S_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块V_VALUE_AIM = [{0}],V_VALUE_ACT=[{1}],V_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["V_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["V_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["V_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块AL_VALUE_AIM = [{0}],AL_VALUE_ACT=[{1}],AL_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["AL_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AL_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AL_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块ALS_VALUE_AIM = [{0}],ALS_VALUE_ACT=[{1}],ALS_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["ALS_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["ALS_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["ALS_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块CEQ_VALUE_AIM = [{0}],CEQ_VALUE_ACT=[{1}],CEQ_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CEQ_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CEQ_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块TI_VALUE_AIM = [{0}],TI_VALUE_ACT=[{1}],TI_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["TI_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["TI_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["TI_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块MO_VALUE_AIM = [{0}],MO_VALUE_ACT=[{1}],MO_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["MO_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MO_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["MO_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块CU_VALUE_AIM = [{0}],CU_VALUE_ACT=[{1}],CU_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["CU_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CU_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CU_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块NI_VALUE_AIM = [{0}],NI_VALUE_ACT=[{1}],NI_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["NI_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NI_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NI_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块CR_VALUE_AIM = [{0}],CR_VALUE_ACT=[{1}],CR_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["CR_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CR_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CR_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块CA_VALUE_AIM = [{0}],CA_VALUE_ACT=[{1}],CA_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["CA_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CA_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["CA_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块B_VALUE_AIM = [{0}],B_VALUE_ACT=[{1}],B_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["B_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["B_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["B_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块NB_VALUE_AIM = [{0}],NB_VALUE_ACT=[{1}],NB_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["NB_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NB_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["NB_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块AS_VALUE_AIM = [{0}],AS_VALUE_ACT=[{1}],AS_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["AS_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AS_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["AS_JUDGE"].ToString());
		Log::Trace("", __FUNCTION__, "返回块N_VALUE_AIM = [{0}],N_VALUE_ACT=[{1}],N_JUDGE=[{2}]", bcls_ret->Tables[0].Rows[0]["N_VALUE_AIM"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["N_VALUE_ACT"].ToDecimal(), bcls_ret->Tables[0].Rows[0]["N_JUDGE"].ToString());

	}
	catch (CDbException& ex)					//捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)	//捕获应用错误
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

	return(doFlag);
}