/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-27
Description:	 发送计划状态信息至l2。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

#include "epex.h"


int f_plan_slab_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
/*<remark>=========================================================
/// <summary>
/// 发送计划状态信息至MMS
/// <para>1.读取传入的厂别区分、制造命令号、制造命令状态</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：由计划编制，计划删除调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="pono_status">制造命令状态          </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_plan_insert_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;

	CString lpsz_tc_no = " ";
	CString lpsz_tc_no2 = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	EPEX epex(&s, conn);

	CString sqlstr;
	CString tablename1 = "TPSSM11";//主表
	CString tablename2 = "TPSSM12";//子表
	CString tablename3 = "TPSSM13";//复制主表
	CString tablename4 = "TPSSM14";//复制子表
	CString action_flag = "I";
	CString c_div = " ";
	CString sm_plan_no = "";
	CString sm_plan_no_in = "";
	CString sm_plan_no_aa = "";
	CDecimal spilt_sum = 0;
	CDecimal treatment_count = 0;
	CDecimal complete_flag = 0;
	CDecimal seq_no = 0;
	//CString dev_code = "";
	/*实体类定义*/
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm13("TPSSM13");
	CModel tpssm14("TPSSM14");

	CDataTable tb_tpssm12("TPSSM12");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);

	EIClass inblock_slab;
	inblock_slab.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblock_slab.Tables[0].Columns.Add(DT_STRING, "PONO");
	inblock_slab.Tables[0].Columns.Add(DT_STRING, "ACTION");
	inblock_slab.Tables[0].Rows.Add();

	try
	{

		/*初始化表结构变量*/

		/* ***** 获取输入参数 ***** */

		if (bcls_rec->Tables[0].Columns.Contains("SM_PLAN_NO"))
		{
			sm_plan_no = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"].ToString();
		}
		if (bcls_rec->Tables[0].Columns.Contains("SM_PLAN_NO_IN"))
		{
			sm_plan_no_in = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO_IN"].ToString();
		}
		if (bcls_rec->Tables[0].Columns.Contains("SM_PLAN_NO_AA"))
		{
			sm_plan_no_aa = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO_AA"].ToString();
		}

		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "sm_plan_no ins =[{0}]", sm_plan_no);
		Log::Info("", __FUNCTION__, "sm_plan_no_in =[{0}]", sm_plan_no_in);
		Log::Info("", __FUNCTION__, "sm_plan_no_aa =[{0}]", sm_plan_no_aa);
		////Log::Info("", __FUNCTION__, "factory_div=[{0}]", tpssm11["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "pono=[{0}]", tpssm11["PONO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status=[{0}]", tpssm11["PONO_STATUS"].ToDecimal());
		////Log::Info("", __FUNCTION__, "heat_no=[{0}]", tpssm11["HEAT_NO"].ToString());

		/* ***** 检查输入参数合法性 ***** */
		
		/* ***** 程序处理 ***** */
		if (sm_plan_no.Trim() == "")
		{
			sqlstr = " SELECT SM_PLAN_NO FROM " + tablename1 + " MINUS SELECT SM_PLAN_NO FROM " + tablename3;

			cmd_tpssm11_inq.SetCommandText(sqlstr);
			//cmd_tpssm11_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				if (sm_plan_no_aa.Find("A" + cmd_tpssm11_inq.GetString(1) + "A") >= 0)
				{
					if (sm_plan_no.Trim() == "") sm_plan_no = cmd_tpssm11_inq.GetString(1);
					else sm_plan_no = sm_plan_no + "," + cmd_tpssm11_inq.GetString(1);
				}
			}
			cmd_tpssm11_inq.Close();
		}
		if (sm_plan_no.Trim() != "")
		{
			sqlstr = " SELECT * FROM " + tablename2 + " WHERE SM_PLAN_NO IN (" + sm_plan_no + ")  ORDER BY SM_PLAN_NO, CHARGE_NO ";//AND ARRIVE_REAL_TIME = ' ' AND START_TIME_REAL = ' ' AND END_TIME_REAL = ' '
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.ExecuteQuery(tb_tpssm12);
			cmd_tpssm12_inq.Close();
		}
		Log::Info("", __FUNCTION__, "sm_plan_no ins =[{0}] COUNT [{1}]", sm_plan_no, tb_tpssm12.Rows.get_Count());

		if (tb_tpssm12.Rows.get_Count() > 0)
		{
			sm_plan_no = " ";
			for (int index = 0; index < tb_tpssm12.Rows.get_Count(); index++)
			{
				tpssm12.MergeFrom(tb_tpssm12.Rows[index]);
				c_div = " ";
				if (tpssm12["SM_PLAN_NO"].ToString() != sm_plan_no)
				{
					tpssm11.Reset();
					tpssm11["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
					tpssm11.Query("SM_PLAN_NO");

					tpssm10.Reset();
					tpssm10["PONO"] = tpssm11["PONO"];
					tpssm10.Query("PONO");

					//11表比较字段
					tpssm13["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm13["ST_NO"] = tpssm11["ST_NO"];
					tpssm13["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
					tpssm13["CAST_PONO_SUM"] = tpssm11["CAST_PONO_SUM"];
				}

				//过滤重复数据
				//12表比较字段
				tpssm14["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
				tpssm14["CHARGE_NO"] = tpssm12["CHARGE_NO"];
				tpssm14["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssm14["START_TIME"] = tpssm12["START_TIME"];
				tpssm14["END_TIME"] = tpssm12["END_TIME"];
				tpssm14["TREATMENT_COUNTER"] = tpssm12["TREATMENT_COUNTER"];

				if (tpssm14.QueryCount("SM_PLAN_NO,DEV_CODE,TREATMENT_COUNTER") == 1 || tpssm12["DEV_CODE"].ToString() == "M5" || tpssm12["DEV_CODE"].ToString() == "M6")//tpssm13.QueryCount("SM_PLAN_NO,ST_NO,CAST_DIV_NO,CAST_PONO_SUM") == 1 && 
				{
					continue; 
				}

				/* *****	发送电文开始 ***** */
				lpsz_tc_no = "T8E2S1";/*赋电文号*/
				lpsz_tc_no2 = "T8F001";

				//分包总数
				sqlstr = " SELECT MAX(SPLIT_INDICATION)  FROM " + tablename3 + " WHERE SM_PLAN_NO LIKE @SM_PLAN_NO ";
				cmd_tpssm11_inq.SetCommandText(sqlstr);
				cmd_tpssm11_inq.Parameters.Set("SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString().Substring(0, tpssm12["SM_PLAN_NO"].ToString().GetLength() - 1) + "%");
				spilt_sum = cmd_tpssm11_inq.ExecuteScalar();
				cmd_tpssm11_inq.Close();
				//精炼重数
				if (tpssm12["AREA_ID"].ToDecimal() != 4)
				{
					treatment_count = 1;
				}
				else
				{
					sqlstr = " SELECT CHARGE_NO  FROM " + tablename4 + " WHERE SM_PLAN_NO = @SM_PLAN_NO AND AREA_ID = 3 "; 
					cmd_tpssm12_inq.SetCommandText(sqlstr);
					cmd_tpssm12_inq.Parameters.Set("SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
					treatment_count = tpssm12["CHARGE_NO"].ToDecimal() - cmd_tpssm12_inq.ExecuteScalar();
					cmd_tpssm12_inq.Close();
				}
				//完成标记
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
				{
					complete_flag = 0;
				}
				else complete_flag = 1;

				if (action_flag == "D") complete_flag = 1;//2级未知逻辑
				else
				{
					if (tpssm12["AREA_ID"].ToDecimal() == 5)
					{
						complete_flag = 1;
					}
					else
					{
						complete_flag = 0;
					}
				}

				if (tpssm10["C_DIV"].ToString() == "1")
				{
					c_div = "S";
				}
				else if (tpssm10["C_DIV"].ToString() == "2")
				{
					c_div = "C";
				}

				/*初始化*/
				if (epex.Initialize(lpsz_tc_no) < 0)
				{
					strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
					sprintf(s.sysmsg, "电文T8E2S1初始化出错！");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				/*拼电文数据*/
				if (epex.SetValue("INT_COM_ORDER", "ACTION", 0, action_flag) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "AGGREGATE_NAME", 0, tpssm12["DEV_CODE"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "PLAN_NUMBER", 0, tpssm12["SM_PLAN_NOL2"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "SPLIT_INDICATION", 0, tpssm12["SPLIT_INDICATION"].ToDecimal()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "NUMBER_OF_SPLITS", 0, spilt_sum) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "TREATMENT_COUNTER", 0, tpssm12["TREATMENT_COUNTER"].ToDecimal()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "PLANNED_START_TIME", 0, tpssm12["START_TIME"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "PLANNED_END_TIME", 0, tpssm12["END_TIME"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (tpssm12["DEV_CODE"].ToString().Trim().Substring(0, 1) == "B" && tpssm12["AREA_ID"].ToDecimal() == 2)
				{
					if (epex.SetValue("INT_COM_ORDER", "GRADE", 0, "DeP") < 0)
					{
						sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
						sprintf(s.sysmsg, "电文T8E2S1拼接失败");
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else
				{
					if (epex.SetValue("INT_COM_ORDER", "GRADE", 0, tpssm10["ST_NO"].ToString()) < 0)
					{
						sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
						sprintf(s.sysmsg, "电文T8E2S1拼接失败");
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}

				if (epex.SetValue("INT_COM_ORDER", "GRADE_CLASS", 0, c_div) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "HEAT_IN_CAST", 0, tpssm11["CAST_DIV_NO"].ToDecimal()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "TOTAL_HEAT_IN_CAST", 0, tpssm11["CAST_PONO_SUM"].ToDecimal()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "SCHEDULE_COMPLETE", 0, complete_flag) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "AIM_TEMP", 0, 0) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "AIM_WEIGHT", 0, tpssm10["PLAN_TAP_WT"].ToDecimal() * 1000) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "AIM_LIQUID_INPUT", 0, 0) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (epex.SetValue("INT_COM_ORDER", "TIME_STAMP", 0, dateNow) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8E2S1拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				/*发送电文*/
				if (epex.SendTele() < 0)
				{
					sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//释放电文
				epex.Uninitialize();

				sm_plan_no = tb_tpssm12.Rows[index]["SM_PLAN_NO"].ToString();

				//发送板坯电文
				if (tpssm12["AREA_ID"].ToDecimal() == 5)
				{
					if (tpssm14.QueryCount("SM_PLAN_NO,DEV_CODE") != 1)
					{
						inblock_slab.Tables[0].Rows[0]["SM_PLAN_NO"] = tb_tpssm12.Rows[index]["SM_PLAN_NO"].ToString();
						inblock_slab.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"].ToString();
						inblock_slab.Tables[0].Rows[0]["ACTION"] = action_flag;
						ret = f_plan_slab_snd(&inblock_slab, bcls_ret, conn);
						if (ret < 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
				}

				//发送能源
				/*初始化*/
				if (epex.Initialize(lpsz_tc_no2) < 0)
				{
					strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
					sprintf(s.sysmsg, "电文T8F001初始化出错！");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				/*拼电文数据*/
				if (epex.SetValue("PROC_DIV", 0, action_flag) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("PLANID", 0, tpssm12["SM_PLAN_NOL2"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("FACTORY_DIV", 0, "LG1") < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("PONO", 0, tpssm10["PONO"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("HEAT_ID", 0, tpssm11["HEAT_NO"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("ST_NO", 0, tpssm10["ST_NO"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("FACILITY_ID", 0, tpssm12["DEV_CODE"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("STAR_OF_HEAT", 0, tpssm12["START_TIME"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("END_OF_HEAT", 0, tpssm12["END_TIME"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				sqlstr = "  SELECT PSSM_PLAN_SND_ID.nextval FROM dual  ";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					seq_no = cmd_inq.GetDecimal(1);
				}
				if (epex.SetValue("SQ", 0, seq_no) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (epex.SetValue("SEND_TIME", 0, dateNow) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文T8F001拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				/*发送电文*/
				if (epex.SendTele() < 0)
				{
					sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//释放电文
				epex.Uninitialize();
			}
		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
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
