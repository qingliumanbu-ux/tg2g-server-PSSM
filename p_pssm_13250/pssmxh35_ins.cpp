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
// service入口
BM2F_ENTERACE(pssmxh35_ins)
int f_pssmxh35_ins(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
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
	CString ret_heat_no_1 = "", ret_heat_no_2 = "", ret_heat_no_3 = "", ret_heat_no_4 = "", ret_heat_no_5 = "", ret_heat_no_6 = "", ret_heat_no_7 = "", ret_heat_no_8 = "", ret_heat_no_9 = "", ret_heat_no_10 = "";
	//系统的分页类信息。
	//CPageInfo pageInfo;

	CModel tpssm35("TPSSM35");
	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");
	CDbCommand cmd(conn);

	EIClass inblkmmsm;

	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "PROC_FLAG");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "MB_HEAT_NO");
	inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "WTS");
	inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "FLAG");
	inblkmmsm.Tables[0].Rows.Add();  //只生成一行


	try
	{
		datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm35.Reset();
			tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			if (bcls_rec->Tables[0].Rows[i]["RET_TIME"].ToString().Trim() != "")
			{
				tpssm35["RET_TIME"] = CDateTime::Parse(bcls_rec->Tables[0].Rows[i]["RET_TIME"].ToString()).ToString("yyyyMMddHHmmss");
			}
			else
			{
				tpssm35["RET_TIME"] = datetimeNow;
			}

			Log::Trace("", "", "FACTORY_DIV[{0}]", v_factory_div);
			Log::Trace("", "", "PONO[{0}]", (const char*)tpssm35["PONO"].ToString());
			Log::Trace("", "", "RET_PONO[{0}]", (const char*)tpssm35["RET_PONO"].ToString());
			Log::Trace("", "", "HEAT_NO[{0}]", (const char*)tpssm35["HEAT_NO"].ToString());
			Log::Trace("", "", "RET_HEAT_NO[{0}]", (const char*)tpssm35["RET_HEAT_NO"].ToString());
			//校验主键(查重)
			if (tpssm35["HEAT_NO"].ToString().Trim() == "")
			{
				strcpy(s.msg, "请输入需要返送的炉号！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

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
			//0
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
			sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
				" AND HEAT_NO = @HEAT_NO "
				" AND RET_HEAT_NO = @RET_HEAT_NO";

			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("factory_div", v_factory_div);
			cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
			cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				affectRow = cmd.GetDecimal(1);
			}
			cmd.Close();
			Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

			if (affectRow >= 1)
			{
				strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
				Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
				Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm35["FACTORY_DIV"] = v_factory_div;
			/*tpssm35["PONO"] = tpssm35["PONO"].ToString().Trim();
			tpssm35["RET_PONO"] = tpssm35["RET_PONO"].ToString().Trim();

			tpssm35["HEAT_NO"] = tpssm35["HEAT_NO"].ToString().Trim();
			tpssm35["RET_HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString().Trim();
			tpssm35["RETURN_MLSL"] = tpssm35["RETURN_MLSL"].ToString();
			tpssm35["RET_TIME"] = tpssm35["RET_TIME"].ToString().Trim();
			tpssm35["REMARK"] = tpssm35["REMARK"].ToString().Trim();*/
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
			if (!tpssm35.Insert())
			{
				strcpy(s.msg, "新增信息失败");
				Log::Trace("", "", "新增信息失败");
				s.flag = -1;
				return -1;
			}
			ret_heat_no_1 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_2 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_3 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_4 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_5 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_6 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_7 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_8 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_9 = tpssm35["RET_HEAT_NO"];
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
				sqlstr = "SELECT COUNT (*)  FROM TPSSM35 WHERE FACTORY_DIV = @factory_div "
					" AND HEAT_NO = @HEAT_NO "
					" AND RET_HEAT_NO = @RET_HEAT_NO";

				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("factory_div", v_factory_div);
				cmd.Parameters.Set("HEAT_NO", tpssm35["HEAT_NO"].ToString().Trim());
				cmd.Parameters.Set("RET_HEAT_NO", tpssm35["RET_HEAT_NO"].ToString().Trim());
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					affectRow = cmd.GetDecimal(1);
				}
				cmd.Close();
				Log::Trace("", "", "--------  2 ---------- ExecuteReader END ");

				if (affectRow >= 1)
				{
					strcpy(s.msg, "该条数据主键冲突，新增失败！请修改后重试！");
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!FACTORY_DIV[{0}]", tpssm35["FACTORY_DIV"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!PONO[{0}]", tpssm35["PONO"].ToString().Trim());
					Log::Trace("", "", "该条数据主键冲突，新增失败！请修改后重试！!RET_PONO[{0}]", tpssm35["RET_PONO"].ToString().Trim());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_CREATE_TIME"] = datetimeNow;
				tpssm35.TrimOrBlank();
				if (!tpssm35.Insert())
				{
					strcpy(s.msg, "新增信息失败");
					Log::Trace("", "", "新增信息失败");
					s.flag = -1;
					return -1;
				}
				ret_heat_no_10 = tpssm35["RET_HEAT_NO"];
			}
		}
		bcls_rec_xh.Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"].ToString();
		ret = f_mmsm_gyhl(&bcls_rec_xh, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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