/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: yss
日期: 2018-7-12 10:00:00
功能: 炼钢计划单表查询。
修改历史：
日期:________；修改人：________; 需求提出人________
变更内容:
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h"

/// <summary>
/// <para>
///		炼钢计划单表查询。
/// </para>
/// </summary>
/// <param name="bcls_rec">传入块  </param>
/// <param name="bcls_ret">返回块  </param>
/// <returns>返回：是否成功</returns>

// service入口
BM2F_ENTERACE(pssm20_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm20_inq(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";		//SQL 查询语句
	CString sqlstr_count = "";	//SQL 查询总记录数语句
	CString sqlstr_temp = "";	//SQL 临时变量
	CString sqlstr_order = "";	//SQL 排序
	CString table_name = "";	//表名
	CString func_id = "";		//功能号
	CString field_name = "";	//字段名
	CString msgstr = "";		//提示信息。
	CString v_factory_div = "";	//厂别
	int	TotalRecordCount = 0;	//总记录数

	//系统的分页类信息。
	//CPageInfo pageInfo;			//分页信息

	try
	{
		//try
		//{//获取前台DEV控件传入的分页信息
		//	pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		//}
		//catch (CException& cex)
		//{
		//	pageInfo.RecordFrom = 0;
		//	pageInfo.PageSize = 1000;
		//}

		//获取传入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "FACTORY_DIV =[{0}]", v_factory_div);
		////Log::Trace("", __FUNCTION__, "func_id =[{0}]", func_id);

		sqlstr = " SELECT substr(e.dev_code,2,1) furnace_no, a.sm_plan_no, a.pono, a.heat_no, a.st_no, a.SMELT_MODE "
			" ,a.REFINE_ROUTE_CODE, case when a.RESTRAND_FLG = 'T' then 'T' else case when a.TD_CHG_FLG=1 then 'D' else ' ' end end CAST_STATUS "
			" ,a.cast_no||'-'||a.cast_div_no cast_info, a.TPD_END_TIME, s.END_TIME DE_S_TIME, a.LADLE_NO, a.LD_STATE "
			" ,to_char(a.pono_status) pono_status, a.run_status, a.FACTORY_DIV, "
			"	E.START_TIME_REAL B_SR, E.START_TIME B_S, E.END_TIME_REAL B_ER, E.END_TIME B_E,"
			"	G1.START_TIME_REAL J1_SR, G1.START_TIME J1_S, G1.END_TIME_REAL J1_ER, G1.END_TIME J1_E,"
			"	G2.START_TIME_REAL J2_SR, G2.START_TIME J2_S, G2.END_TIME_REAL J2_ER, G2.END_TIME J2_E,"
			"	G3.START_TIME_REAL J3_SR, G3.START_TIME J3_S, G3.END_TIME_REAL J3_ER, G3.END_TIME J3_E,"
			"	G4.START_TIME_REAL J4_SR, G4.START_TIME J4_S, G4.END_TIME_REAL J4_ER, G4.END_TIME J4_E,"
			"	H.START_TIME_REAL C_SR, H.START_TIME C_S, H.END_TIME_REAL C_ER, H.END_TIME C_E "
			"   ,a.PONO_STATUS PLAN_STATUS,A.REC_CREATE_TIME CREATE_TIME "
			"	FROM TPSSM11 A"
			"	LEFT JOIN TPSSM10 C ON A.PONO = C.PONO"
			"	LEFT JOIN TPSSM01 D ON A.PONO = D.PONO"
			"	LEFT JOIN TPSSM12 E ON A.SM_PLAN_NO = E.SM_PLAN_NO AND E.AREA_ID = 3"
			"	LEFT JOIN TMMSM21 F ON A.HEAT_NO = F.HEAT_NO"
			"	LEFT JOIN TPSSM12 G1 ON A.SM_PLAN_NO = G1.SM_PLAN_NO AND G1.AREA_ID = 4 AND G1.CHARGE_NO = 2"
			"	LEFT JOIN TPSSM12 G2 ON A.SM_PLAN_NO = G2.SM_PLAN_NO AND G2.AREA_ID = 4 AND G2.CHARGE_NO = 3"
			"	LEFT JOIN TPSSM12 G3 ON A.SM_PLAN_NO = G3.SM_PLAN_NO AND G3.AREA_ID = 4 AND G3.CHARGE_NO = 4"
			"	LEFT JOIN TPSSM12 G4 ON A.SM_PLAN_NO = G4.SM_PLAN_NO AND G4.AREA_ID = 4 AND G4.CHARGE_NO = 5"
			"	LEFT JOIN TPSSM12 H ON A.SM_PLAN_NO = H.SM_PLAN_NO AND H.AREA_ID = 5"
			" left join tmmsm14 s on s.heat_no = a.heat_no and s.heat_no <> ' ' "
			" WHERE a.FACTORY_DIV = '" + v_factory_div + "' ORDER BY a.heat_no ";
		Db::QueryTable(sqlstr, bcls_ret->Tables[0]);

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BOF_BLOW_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_START_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_END_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_PREP_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR1_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR2_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR3_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SR4_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_FLAG");

		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			//转炉脱碳时间
			if (bcls_ret->Tables[0].Rows[i]["B_ER"].ToString().Trim() != "")//脱碳实绩结束
			{
				bcls_ret->Tables[0].Rows[i]["BOF_BLOW_TIME"] = bcls_ret->Tables[0].Rows[i]["B_ER"];
				bcls_ret->Tables[0].Rows[i]["MAT_PREP_FLAG"] = "1";
				bcls_ret->Tables[0].Rows[i]["MAIN_SMELT_FLAG"] = "1";
			}
			else if(bcls_ret->Tables[0].Rows[i]["B_SR"].ToString().Trim() != "")//脱碳实绩已开始未结束
			{
				bcls_ret->Tables[0].Rows[i]["BOF_BLOW_TIME"] = bcls_ret->Tables[0].Rows[i]["B_SR"];
				bcls_ret->Tables[0].Rows[i]["MAT_PREP_FLAG"] = "2";//转炉开始
				bcls_ret->Tables[0].Rows[i]["MAIN_SMELT_FLAG"] = "0";
			}
			else//脱碳实绩未开始
			{
				bcls_ret->Tables[0].Rows[i]["BOF_BLOW_TIME"] = bcls_ret->Tables[0].Rows[i]["B_S"];
			}

			string j_er, j_sr, j_e, j_s, sr_no, sr_end_time, sr_time, sr_flag;
			for (int j = 1; j <= 4; j++)
			{
				j_er = "J" + to_string(j) + "_ER";
				j_sr = "J" + to_string(j) + "_SR";
				j_e = "J" + to_string(j) + "_E";
				j_s = "J" + to_string(j) + "_S";
				sr_no = "SR" + to_string(j) + "_NO";
				sr_end_time = "SR" + to_string(j) + "_END_TIME";
				sr_time = "SR" + to_string(j) + "_TIME";
				sr_flag = "SR" + to_string(j) + "_FLAG";

				if (bcls_ret->Tables[0].Rows[i][j_s].ToString().Trim() == "")
				{
					continue;
				}
				else if (bcls_ret->Tables[0].Rows[i][j_er].ToString().Trim() != "")
				{
					bcls_ret->Tables[0].Rows[i][sr_no] = bcls_ret->Tables[0].Rows[i][j_er];
					bcls_ret->Tables[0].Rows[i][sr_end_time] = bcls_ret->Tables[0].Rows[i][j_er];
					bcls_ret->Tables[0].Rows[i][sr_time] = bcls_ret->Tables[0].Rows[i][j_sr];
					bcls_ret->Tables[0].Rows[i][sr_flag] = "1";//精炼结束
				}
				else if (bcls_ret->Tables[0].Rows[i][j_sr].ToString().Trim() != "")
				{
					bcls_ret->Tables[0].Rows[i][sr_no] = bcls_ret->Tables[0].Rows[i][j_sr];
					bcls_ret->Tables[0].Rows[i][sr_end_time] = bcls_ret->Tables[0].Rows[i][j_e];
					bcls_ret->Tables[0].Rows[i][sr_time] = bcls_ret->Tables[0].Rows[i][j_sr];
					bcls_ret->Tables[0].Rows[i][sr_flag] = "2";//精炼开始
				}
				else
				{
					bcls_ret->Tables[0].Rows[i][sr_no] = bcls_ret->Tables[0].Rows[i][j_s];
					bcls_ret->Tables[0].Rows[i][sr_end_time] = bcls_ret->Tables[0].Rows[i][j_e];
					bcls_ret->Tables[0].Rows[i][sr_time] = bcls_ret->Tables[0].Rows[i][j_s];
					bcls_ret->Tables[0].Rows[i][sr_flag] = "0";//精炼未开始
				}
				/*if (bcls_ret->Tables[0].Rows[i][sr_no].ToString().Trim().GetLength() == 14)
					bcls_ret->Tables[0].Rows[i][sr_no] = bcls_ret->Tables[0].Rows[i][sr_no].ToString().Substring(8, 4);
				if (bcls_ret->Tables[0].Rows[i][sr_end_time].ToString().Trim().GetLength() == 14)
					bcls_ret->Tables[0].Rows[i][sr_end_time] = bcls_ret->Tables[0].Rows[i][sr_end_time].ToString().Substring(8, 4);
				if (bcls_ret->Tables[0].Rows[i][sr_time].ToString().Trim().GetLength() == 14)
					bcls_ret->Tables[0].Rows[i][sr_time] = bcls_ret->Tables[0].Rows[i][sr_time].ToString().Substring(8, 4);*/
			}

			//CC时间
			if (bcls_ret->Tables[0].Rows[i]["C_ER"].ToString().Trim() != "")//CC实绩结束
			{
				bcls_ret->Tables[0].Rows[i]["CC_START_TIME"] = bcls_ret->Tables[0].Rows[i]["C_SR"];
				bcls_ret->Tables[0].Rows[i]["CC_END_TIME"] = bcls_ret->Tables[0].Rows[i]["C_ER"];
				bcls_ret->Tables[0].Rows[i]["CC_FLAG"] = "1";//
			}
			else if (bcls_ret->Tables[0].Rows[i]["C_SR"].ToString().Trim() != "")//CC实绩开始未结束
			{
				bcls_ret->Tables[0].Rows[i]["CC_START_TIME"] = bcls_ret->Tables[0].Rows[i]["C_SR"];
				bcls_ret->Tables[0].Rows[i]["CC_END_TIME"] = bcls_ret->Tables[0].Rows[i]["C_E"];
				bcls_ret->Tables[0].Rows[i]["CC_FLAG"] = "2";//
			}
			else//CC实绩未开始
			{
				bcls_ret->Tables[0].Rows[i]["CC_START_TIME"] = bcls_ret->Tables[0].Rows[i]["C_S"];
				bcls_ret->Tables[0].Rows[i]["CC_END_TIME"] = bcls_ret->Tables[0].Rows[i]["C_E"];
				bcls_ret->Tables[0].Rows[i]["CC_FLAG"] = "0";//
			}
			/*if (bcls_ret->Tables[0].Rows[i]["CC_NO"].ToString().Trim().GetLength() == 14)
			{
				bcls_ret->Tables[0].Rows[i]["CC_NO"] = bcls_ret->Tables[0].Rows[i]["CC_NO"].ToString().Substring(8, 4);
			}*/
		}

		//工序处理号查询
		if (bcls_ret->Tables.get_Count() <= 1)
			bcls_ret->Tables.Add();
		sqlstr ="select STATION_ID ,STATION_NO ,CURR_PROC_NO from tpssm25 a "
			" WHERE a.FACTORY_DIV = '" + v_factory_div + "' ";
		RecordList recList = Db::QueryList(sqlstr);
		for (int i = 0; i < recList.size(); i++)
		{
			CString col_name = recList[i]["STATION_ID"].ToString().Trim()
				+ recList[i]["STATION_NO"].ToString().Trim()
				+ "_PROC_NO";
			if (!bcls_ret->Tables[1].Columns.Contains(col_name))
				bcls_ret->Tables[1].Columns.Add(DT_STRING, col_name);
			if (bcls_ret->Tables[1].Rows.get_Count() <= 0)
				bcls_ret->Tables[1].Rows.Add();
			bcls_ret->Tables[1].Rows[0][col_name] = recList[i]["CURR_PROC_NO"].ToString().Trim();
		}
		//浇次号查询
		sqlstr=" SELECT DEV_CODE,CAST_NO||'-'||CAST_DIV_NO CAST_INFO FROM TPSSM26 a "
			" WHERE a.FACTORY_DIV = '" + v_factory_div + "' ";
		recList = Db::QueryList(sqlstr);
		for (int i = 0; i < recList.size(); i++)
		{
			CString col_name = recList[i]["DEV_CODE"].ToString().Trim() + "_CAST_NO";
			if (!bcls_ret->Tables[1].Columns.Contains(col_name))
				bcls_ret->Tables[1].Columns.Add(DT_STRING, col_name);
			if (bcls_ret->Tables[1].Rows.get_Count() <= 0)
				bcls_ret->Tables[1].Rows.Add();
			bcls_ret->Tables[1].Rows[0][col_name] = recList[i]["CAST_INFO"].ToString().Trim();
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
		////Log::Warn("", __FUNCTION__, "CDbException: {0}", s.msg);
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
		////Log::Error("", __FUNCTION__, "CApplicationException: {0}", ex.GetMsg());
	}
	catch (CException& ex)
	{
		strncpy(s.sysmsg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
		////Log::Fatal("", __FUNCTION__, "CException: {0}", ex.GetMsg());
	}
	return doFlag;
}