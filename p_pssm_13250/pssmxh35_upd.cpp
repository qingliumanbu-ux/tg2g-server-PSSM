/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2023-06-8 10:13:56
Description: 钢水返送信息新增
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/
#include "tpssm35.h"

/* ***** 静态函数申明 ***** */
int f_mmsm_gyhl(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsm_gyupd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
// service入口
BM2F_ENTERACE(pssmxh35_upd)
int f_pssmxh35_upd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	int fetchRowCount;
	int i;
	CDecimal affectRow = 0;
	CString	datetimeNow; /* 记录创建时间 */

	CString v_factory_div = "LG1";

	CString sqlstr = "";
	CString str = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	EIClass bcls_rec_xh;
	bcls_rec_xh.Tables[0].set_TableName("XH");
	bcls_rec_xh.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	bcls_rec_xh.Tables[0].Rows.Add();

	//系统的分页类信息。
	//CPageInfo pageInfo;
	CString ret_heat_1 = "", ret_heat_2 = "", ret_heat_3 = "", ret_heat_4 = "";
	CModel tpssm35("TPSSM35");
	CModel tpssm35f("TPSSM35");
	CModel tpssm35ff("TPSSM35");
	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");
	CDbCommand cmd(conn);
	CDbCommand cmd_inq(conn);
	EIClass inblkmmsm;
	
	
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "PROC_FLAG");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "MB_HEAT_NO");
	inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "WTS");
	inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "FLAG");
	inblkmmsm.Tables[0].Rows.Add();  //只生成一行

	EIClass inipssm35;
	inipssm35.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO1");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO2");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO3");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO4");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO5");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO6");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO7");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO8");
	inipssm35.Tables[0].Columns.Add(DT_STRING, "RET_HEAT_NO9");

	try
	{
		datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm35.Reset();
			tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tpssm35.TrimOrBlank();
			if (bcls_rec->Tables[0].Rows[i]["RET_TIME"].ToString().Trim() != "")
			{
				tpssm35["RET_TIME"] = CDateTime::Parse(bcls_rec->Tables[0].Rows[i]["RET_TIME"].ToString().SubstringNE(0, 14)).ToString("yyyyMMddHHmmss");
			}
			else
			{
				tpssm35["RET_TIME"] = datetimeNow;
			}
			//校验主键(查重)
			if (tpssm35["HEAT_NO"].ToString().Trim() == "")
			{
				strcpy(s.msg, "请输入需要返送的炉号！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//20260202
			inipssm35.Tables[0].Rows.Clear();
			CDecimal i_count = 0;
			CString cs_column1 = "";
			sqlstr = "  SELECT *  FROM TPSSM35 WHERE HEAT_NO = @cs_heat_no  order by RET_HEAT_NO";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("cs_heat_no", tpssm35["HEAT_NO"].ToString().Trim());
			cmd_inq.ExecuteReader();
			inipssm35.Tables[0].Rows.Add();
			inipssm35.Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"].ToString().Trim();
			while (cmd_inq.Read())
			{
				tpssm35f.Reset();
				cmd_inq.Fetch(tpssm35f);
				if (i_count == 0)
				{
					inipssm35.Tables[0].Rows[0]["RET_HEAT_NO"] = tpssm35f["RET_HEAT_NO"].ToString().Trim();
					
				}
				else
				{
					cs_column1 = "RET_HEAT_NO" + (i_count).ToString();
					inipssm35.Tables[0].Rows[0][cs_column1] = tpssm35f["RET_HEAT_NO"].ToString().Trim();
				}

				i_count = i_count + 1;

				tpssm35ff.Reset();
				// 按联合主键精准删除历史返送记录
				tpssm35ff["FACTORY_DIV"] = v_factory_div;
				tpssm35ff["HEAT_NO"] = inipssm35.Tables[0].Rows[0]["HEAT_NO"].ToString().Trim();
				tpssm35ff["RET_HEAT_NO"] = inipssm35.Tables[0].Rows[0]["RET_HEAT_NO"].ToString().Trim();
				tpssm35ff.Delete("FACTORY_DIV,HEAT_NO,RET_HEAT_NO");
			}
			cmd_inq.Close();

			
			//
			if (bcls_rec->Tables[0].Rows[i]["RATE9"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE8"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE7"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE6"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE5"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE4"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE3"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE2"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE1"].ToDecimal() / 100 + bcls_rec->Tables[0].Rows[i]["RATE"].ToDecimal() / 100 > 1)
			{
				strcpy(s.msg, "比例超过百分之百！");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			CString mainHeatNo = tpssm35["HEAT_NO"].ToString().Trim();

			CString returnHeatFields[10] = {
				"RET_HEAT_NO", "RET_HEAT_NO1", "RET_HEAT_NO2", "RET_HEAT_NO3",
				"RET_HEAT_NO4", "RET_HEAT_NO5", "RET_HEAT_NO6", "RET_HEAT_NO7",
				"RET_HEAT_NO8", "RET_HEAT_NO9"
			};

			for (int k = 0; k < 10; k++)
			{
				CString retHeat = bcls_rec->Tables[0].Rows[i][returnHeatFields[k]].ToString().Trim();
				if (!retHeat.IsEmpty() && retHeat == mainHeatNo)
				{
					strcpy(s.msg, "返送炉号不能与主炉号相同！");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE"].ToDecimal() / 100;
			tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"].ToString();
			tpssm41["HEAT_NO"] = tpssm35["HEAT_NO"].ToString();
			if (!tpssm11.Query("HEAT_NO"))
			{
				tpssm41.Query("HEAT_NO");
				tpssm35["PONO"] = tpssm41["PONO"];
				tpssm35["SM_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
				tpssm35["ST_NO"] = tpssm41["ST_NO"];
			}
			else
			{
				tpssm35["PONO"] = tpssm11["PONO"];
				tpssm35["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm35["ST_NO"] = tpssm11["ST_NO"];
			}
			tpssm11.Reset(); tpssm41.Reset();
			tpssm35["FACTORY_DIV"] = v_factory_div;

			tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
			tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
			if (!tpssm11.Query("HEAT_NO"))
			{
				tpssm41.Query("HEAT_NO");
				tpssm35["RET_PONO"] = tpssm41["PONO"];
				tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
				tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
			}
			else
			{
				tpssm35["RET_PONO"] = tpssm11["PONO"];
				tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
			}

			tpssm35["REC_CREATOR"] = s.userid;
			tpssm35["REC_CREATE_TIME"] = datetimeNow;
			tpssm35.TrimOrBlank();
			tpssm35["REC_REVISOR"] = s.userid;
			tpssm35["REC_REVISE_TIME"] = datetimeNow;
			if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
			{
				tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
			}
			else
			{
				if (tpssm35.QueryCount("HEAT_NO") > 0)
				{
					tpssm35.Delete("HEAT_NO");
				}
				tpssm35.Insert();
			}
			
			// 1
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO1"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE1"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();

				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}

			}
		//2
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO2"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE2"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}
			}
			//3
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO3"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE3"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}
			
			}
			//4
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO4"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE4"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}

			}
			//5
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO5"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE5"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}

			}
			//6
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO6"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE6"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}

			}
			//7
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO7"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE7"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}

			}
			//8
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO8"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE8"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}

			}
			//9
			tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["RET_HEAT_NO9"].ToString();
			tpssm35["RATE"] = bcls_rec->Tables[0].Rows[i]["RATE9"].ToDecimal() / 100;
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				tpssm11["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				tpssm41["HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString();
				if (!tpssm11.Query("HEAT_NO"))
				{
					tpssm41.Query("HEAT_NO");
					tpssm35["RET_PONO"] = tpssm41["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm41["ST_NO"];
				}
				else
				{
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["RETURN_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["IN_ST_NO"] = tpssm11["ST_NO"];
				}
				tpssm11.Reset(); tpssm41.Reset();
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (tpssm35.QueryCount("HEAT_NO,RET_HEAT_NO") == 1)
				{
					tpssm35.Update("SM_PLAN_NO,PONO,RET_TIME,RET_PONO,RETURN_PLAN_NO,REMARK,TOTAL_WT,RATE", "HEAT_NO,RET_HEAT_NO");
				}
				else
				{
					tpssm35.Insert();
				}
			}
			
			if (tpssm35["HEAT_NO"].ToString().Trim() != "")
			{
				bcls_rec_xh.Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"].ToString();
				ret = f_mmsm_gyhl(&bcls_rec_xh, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			if (inipssm35.Tables[0].Rows.get_Count() > 0)
			{
				CString heatNoFields[10] = { "RET_HEAT_NO", "RET_HEAT_NO1", "RET_HEAT_NO2", "RET_HEAT_NO3", "RET_HEAT_NO4", "RET_HEAT_NO5", "RET_HEAT_NO6", "RET_HEAT_NO7", "RET_HEAT_NO8", "RET_HEAT_NO9" };
				for (int j = 0; j <10; j++)
				{
					CString csHeatNo = inipssm35.Tables[0].Rows[0][heatNoFields[j]].ToString().Trim();
					if (!csHeatNo.IsEmpty())
					{
						bcls_rec_xh.Tables[0].Rows[0]["HEAT_NO"] = csHeatNo;
						ret = f_mmsm_gyupd(&bcls_rec_xh, bcls_ret, conn);
						if (ret < 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
				}
			}
		}

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