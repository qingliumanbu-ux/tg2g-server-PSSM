/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-27
Description:	 发送计划状态信息至MMS。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

#include "epex.h"



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
int f_plan_slab_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;
	int index = 0;
	int rowcount = 0;

	CString lpsz_tc_no = " ";
	CString action = " ";
	CString lslab_no = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal seq = 0;
	CString strand_no = " ";

	EPEX epex(&s, conn);

	CString sqlstr;

	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm03("TPSSM03");
	CModel tpssm11("TPSSM11");
	CModel tpssm13("TPSSM13");

	CDataTable tb_tpssm03("TPSSM03");

	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_inq(conn);
	try
	{

		/*初始化表结构变量*/
		rowcount = bcls_rec->Tables[0].Rows.get_Count();
		for (int rownum = 0; rownum < rowcount; rownum++)
		{
			/* ***** 获取输入参数 ***** */
			//tpssm03["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
			tpssm03["PONO"] = bcls_rec->Tables[0].Rows[rownum]["PONO"].ToString();
			tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[rownum]["SM_PLAN_NO"].ToString();
			tpssm13["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[rownum]["SM_PLAN_NO"].ToString();
			action = bcls_rec->Tables[0].Rows[rownum]["ACTION"].ToString();

			/* ***** 打印输入参数 ***** */
			////Log::Info("", __FUNCTION__, "factory_div=[{0}]", tpssm11["FACTORY_DIV"].ToString());
			Log::Info("", __FUNCTION__, "action=[{0}]", action);
			Log::Info("", __FUNCTION__, "PONO=[{0}]", tpssm03["PONO"].ToString());
			Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}]", tpssm11["SM_PLAN_NO"].ToString());
			////Log::Info("", __FUNCTION__, "pono_status=[{0}]", tpssm11["PONO_STATUS"].ToDecimal());
			////Log::Info("", __FUNCTION__, "heat_no=[{0}]", tpssm11["HEAT_NO"].ToString());

			/* ***** 检查输入参数合法性 ***** */

			/* ***** 程序处理 ***** */
			if (action == "I")
			{
				action = "U";
				tpssm11.Query("SM_PLAN_NO");
				if (tpssm11["CC_MACH_NO"].ToString() == "5" || tpssm11["CC_MACH_NO"].ToString() == "6")
				{

				}
				else
				{
					if (tpssm11["RUN_STATUS"].ToDecimal() < 53) //浇注终了
					{
						//* *****	发送电文开始 ***** */
						lpsz_tc_no = "T8E2S3";/*赋电文号*/

						/*初始化*/
						if (epex.Initialize(lpsz_tc_no) < 0)
						{
							strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
							sprintf(s.sysmsg, "电文H4H309初始化出错！");
							throw CApplicationException(-1, s.msg, log.Location);
						}
						seq = 0;
						index = 0;
						/*拼电文数据*/
						if (epex.SetValue("INT_CCM_PROD_PLAN", "ACTION", 0, action) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "PLAN_NUMBER", 0, tpssm11["SM_PLAN_NOL2"].ToString()) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "SPLIT_INDICATION", 0, tpssm11["SPLIT_INDICATION"].ToDecimal()) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "MWC_MODE", 0, "O") < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "MWC_SPEED_MAX", 0, 0) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "TIME_STAMP", 0, dateNow) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						sqlstr = " SELECT LSLAB_NO, SUM(SLAB_SEQ_2), STRAND_NO FROM tpssm03 WHERE PONO = @PONO GROUP BY LSLAB_NO, STRAND_NO ORDER BY SUM(SLAB_SEQ_2) ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString().Trim());
						cmd_inq.ExecuteReader();
						while (cmd_inq.Read())
						{
							lslab_no = cmd_inq.GetString(1);
							sqlstr = " SELECT * FROM TPSSM03 WHERE PONO = @PONO AND LSLAB_NO = @LSLAB_NO ORDER BY LSLAB_NO,SLAB_NO ";
							cmd_tpssm03_inq.SetCommandText(sqlstr);
							cmd_tpssm03_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString().Trim());
							cmd_tpssm03_inq.Parameters.Set("LSLAB_NO", lslab_no);
							cmd_tpssm03_inq.ExecuteReader();

							if (cmd_tpssm03_inq.Read())
							{
								cmd_tpssm03_inq.Fetch(tpssm03);
								seq = seq + 1;
								if (tpssm11["CC_MACH_NO"].ToString() == "0")
								{
									strand_no = "Z";
								}
								else if (tpssm11["CC_MACH_NO"].ToString() == "1")
								{
									strand_no = "A";
								}
								else if (tpssm11["CC_MACH_NO"].ToString() == "2")
								{
									strand_no = "B";
								}
								else if (tpssm11["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "1")
								{
									strand_no = "C";
								}
								else if (tpssm11["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "2")
								{
									strand_no = "D";
								}
								else if (tpssm11["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "1")
								{
									strand_no = "E";
								}
								else if (tpssm11["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "2")
								{
									strand_no = "F";
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "VIRTUAL_SLAB_ID", index, tpssm03["LSLAB_NO"].ToString()) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "STRAND_NUMBER", index, strand_no) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "SLAB_SEQ_NUMBER", index, seq) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_WIDTH", index, tpssm03["SLAB_WIDTH"].ToDecimal() / 1000) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_THICKNESS", index, tpssm03["SLAB_THICK"].ToDecimal() / 1000) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								if (tpssm03["LSLAB_NO_LENGTH"].ToDecimal() != 0)
								{
									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_LENGTH", index, tpssm03["LSLAB_NO_LENGTH"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MIN_LENGTH", index, tpssm03["LSLAB_NO_LENGTH_MIN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MAX_LENGTH", index, tpssm03["LSLAB_NO_LENGTH_MAX"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}
								}
								else
								{
									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_LENGTH", index, tpssm03["SLAB_LEN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MIN_LENGTH", index, tpssm03["SLAB_MIN_LEN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MAX_LENGTH", index, tpssm03["SLAB_MAX_LEN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "SAMPLE_CUT", index, 0) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_DESTINATION", index, tpssm03["FACTORY_NEXT"].ToString()) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "TIME_STAMP", index, dateNow) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								index++;
							}

							cmd_tpssm03_inq.Close();
						}
						/*发送电文*/
						if (epex.SendTele() < 0)
						{
							sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
							throw CApplicationException(-1, s.msg, log.Location);
						}

						//释放电文
						epex.Uninitialize();
						cmd_inq.Close();
					}
				}
			}
			else if (action == "D")
			{
				tpssm13.Query("SM_PLAN_NO");
				if (tpssm13["CC_MACH_NO"].ToString() == "5" || tpssm13["CC_MACH_NO"].ToString() == "6")
				{

				}
				else
				{
					if (tpssm13["RUN_STATUS"].ToDecimal() < 51) //浇注终了
					{
						//* *****	发送电文开始 ***** */
						lpsz_tc_no = "T8E2S3";/*赋电文号*/

						/*初始化*/
						if (epex.Initialize(lpsz_tc_no) < 0)
						{
							strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
							sprintf(s.sysmsg, "电文H4H309初始化出错！");
							throw CApplicationException(-1, s.msg, log.Location);
						}
						seq = 0;
						index = 0;
						/*拼电文数据*/
						if (epex.SetValue("INT_CCM_PROD_PLAN", "ACTION", 0, action) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "PLAN_NUMBER", 0, tpssm13["SM_PLAN_NOL2"].ToString()) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "SPLIT_INDICATION", 0, tpssm13["SPLIT_INDICATION"].ToDecimal()) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "MWC_MODE", 0, "O") < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "MWC_SPEED_MAX", 0, 0) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						if (epex.SetValue("INT_CCM_PROD_PLAN", "TIME_STAMP", 0, dateNow) < 0)
						{
							sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
							sprintf(s.sysmsg, "电文T8E2S3拼接失败");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						sqlstr = " SELECT LSLAB_NO, SUM(SLAB_SEQ_2), STRAND_NO FROM tpssm03 WHERE PONO = @PONO GROUP BY LSLAB_NO, STRAND_NO ORDER BY SUM(SLAB_SEQ_2) ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString().Trim());
						cmd_inq.ExecuteReader();
						while (cmd_inq.Read())
						{
							lslab_no = cmd_inq.GetString(1);
							sqlstr = " SELECT * FROM TPSSM03 WHERE PONO = @PONO AND LSLAB_NO = @LSLAB_NO ORDER BY LSLAB_NO,SLAB_NO ";
							cmd_tpssm03_inq.SetCommandText(sqlstr);
							cmd_tpssm03_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString().Trim());
							cmd_tpssm03_inq.Parameters.Set("LSLAB_NO", lslab_no);
							cmd_tpssm03_inq.ExecuteReader();

							if (cmd_tpssm03_inq.Read())
							{
								cmd_tpssm03_inq.Fetch(tpssm03);
								seq = seq + 1;
								if (tpssm13["CC_MACH_NO"].ToString() == "0")
								{
									strand_no = "Z";
								}
								else if (tpssm13["CC_MACH_NO"].ToString() == "1")
								{
									strand_no = "A";
								}
								else if (tpssm13["CC_MACH_NO"].ToString() == "2")
								{
									strand_no = "B";
								}
								else if (tpssm13["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "1")
								{
									strand_no = "C";
								}
								else if (tpssm13["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "2")
								{
									strand_no = "D";
								}
								else if (tpssm13["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "1")
								{
									strand_no = "E";
								}
								else if (tpssm13["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "2")
								{
									strand_no = "F";
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "VIRTUAL_SLAB_ID", index, tpssm03["LSLAB_NO"].ToString()) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "STRAND_NUMBER", index, strand_no) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "SLAB_SEQ_NUMBER", index, seq) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_WIDTH", index, tpssm03["SLAB_WIDTH"].ToDecimal() / 1000) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_THICKNESS", index, tpssm03["SLAB_THICK"].ToDecimal() / 1000) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								if (tpssm03["LSLAB_NO_LENGTH"].ToDecimal() != 0)
								{
									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_LENGTH", index, tpssm03["LSLAB_NO_LENGTH"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MIN_LENGTH", index, tpssm03["LSLAB_NO_LENGTH_MIN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MAX_LENGTH", index, tpssm03["LSLAB_NO_LENGTH_MAX"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}
								}
								else
								{
									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_LENGTH", index, tpssm03["SLAB_LEN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MIN_LENGTH", index, tpssm03["SLAB_MIN_LEN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}

									if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "MAX_LENGTH", index, tpssm03["SLAB_MAX_LEN"].ToDecimal() / 1000) < 0)
									{
										sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
										sprintf(s.sysmsg, "电文T8E2S3拼接失败");
										throw CApplicationException(-1, s.msg, log.Location);
									}
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "SAMPLE_CUT", index, 0) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "AIM_DESTINATION", index, tpssm03["FACTORY_NEXT"].ToString()) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								if (epex.SetValue("INT_CCM_PROD_PLAN_DET", "TIME_STAMP", index, dateNow) < 0)
								{
									sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
									sprintf(s.sysmsg, "电文T8E2S3拼接失败");
									throw CApplicationException(-1, s.msg, log.Location);
								}
								//Log::Info("", __FUNCTION__, "LSLAB_NO=[{0}]", tpssm03["LSLAB_NO"].ToString());
								index++;
							}

							cmd_tpssm03_inq.Close();
						}
						/*发送电文*/
						if (epex.SendTele() < 0)
						{
							sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
							throw CApplicationException(-1, s.msg, log.Location);
						}

						//释放电文
						epex.Uninitialize();
						cmd_inq.Close();
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
