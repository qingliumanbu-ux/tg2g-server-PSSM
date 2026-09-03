/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:zhengqq
Date:2022-11-02
Version:1.0
Description: 钢水返送
Update: 2022-11-02 zhengqq  做成可配置（是否生成事故命令号 9xx）
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件









int f_pssm_get_no(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//获取PONO和CAST_LOT
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_plan_delete_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsmgglwt_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

//int f_pssm_rt_sendl2(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);


//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 钢水返送
/// <para>钢水返送                            </para>
/// <para>数据库表：tpssm35                    </para>
/// <para>主调用函数：PSSM18R画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns></returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18r_rt_upd)
//-EP_SYSTEM_HEAD_END
int f_pssm18r_rt_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int dummy = 0;
	CString	date_time;            /* 记录创建时刻 */
	int curr_wp_no = 0;
	CDecimal v_cast_no = 0;
	CDecimal resume_seq_no = 0;
	CString cast_no = " ";
	CString proc_flag = "";
	int cast_div_no = 0;
	int v_count = 0;
	int ret = 0;
	CString v_sm_plan_no = " ";
	CString sqlstr = "";
	CString v_pono = "";
	CString ret_st_no = "";
	CString source_dev = "";

	EIClass inblk;        //调用函数用
	EIClass inSendL2;
	EIClass inBlock99; //调用炼钢计划履历用	
	EIClass inblkmmsm;

	EIClass inBlock1; //生成流水号	
	EIClass outBlock;

	EIClass inBlock;
	EIClass inblock2;

	CModel tpssm35("TPSSM35");
	CModel tpssm35_new("TPSSM35");
	CModel tpssm13("TPSSM13");
	CModel tpssm12("TPSSM12");
	CModel tpssm11("TPSSM11");
	CModel tpssm10("TPSSM10");
	CModel tpssm01("TPSSM01");
	CModel tep0002("TEP0002");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);
	CDbCommand cmd_tpssm26_upd(conn);
	CModel tpssm11_ret("TPSSM11_RET");
	try
	{
		//1.增加向MMS发送炉次状态电文,单记录
		inBlock.Tables[0].set_TableName("X200009");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock.Tables[0].Rows.Add();  //只生成一行

		//定义输入Block的参数
		// 获取PONO, LOT号
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "GETSEQ_DIF"); //2-生成PONO,4-CAST_LOT_NO
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CS_PLAN_MAKER");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");

		inblk.Tables[0].set_TableName("PLAN");  //
		inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
		inblk.Tables[0].Rows.Add();

		inSendL2.Tables[0].set_TableName("SEND_LG");
		inSendL2.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		inSendL2.Tables[0].Columns.Add(DT_STRING, "PONO");
		inSendL2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		inSendL2.Tables[0].Rows.Add();

		inblock2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		inblock2.Tables[0].Rows.Add();

		//设置调用炼钢计划履历传入参数
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "RETURN_MLSL");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID");

		inblkmmsm.Tables[0].Columns.Add(DT_STRING, "PROC_FLAG");
		inblkmmsm.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
		inblkmmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO"); 
		inblkmmsm.Tables[0].Columns.Add(DT_STRING, "MB_HEAT_NO");
		inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "WTS");
		inblkmmsm.Tables[0].Columns.Add(DT_STRING, "FLAG");
		inblkmmsm.Tables[0].Rows.Add();  //只生成一行

		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------------------------------------------------------
		//获得输入参数
		tpssm35["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm35["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HTNO"].ToString();

		tpssm35["RETURN_MLSL"] = bcls_rec->Tables[0].Rows[0]["RETURNWT"].ToDecimal();
		tpssm35["RET_DEST"] = bcls_rec->Tables[0].Rows[0]["RETURNPOS"].ToString().TrimOrBlank();
		tpssm35["RET_TIME"] = bcls_rec->Tables[0].Rows[0]["DATE"].ToString();
		tpssm35["RET_HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["RETHTNO"].ToString().TrimOrBlank();

		//--------------------------------------------------------------------------------
		//打印输入参数
		//Log::Trace("", __FUNCTION__, "pssm21_rt_upd>tpssm35["FACTORY_DIV"] = [{0}]", tpssm35["FACTORY_DIV"].ToString());
		//Log::Trace("", __FUNCTION__, "pssm21_rt_upd>tpssm35["HEAT_NO"] = [{0}]", tpssm35["HEAT_NO"].ToString());
		//Log::Trace("", __FUNCTION__, "pssm21_rt_upd>tpssm35["RETURN_MLSL"] = [{0}]", tpssm35["RETURN_MLSL"].ToDecimal());
		//Log::Trace("", __FUNCTION__, "pssm21_rt_upd>tpssm35["DEV_CODE"] = [{0}]", tpssm35["DEV_CODE"].ToString());
		//Log::Trace("", __FUNCTION__, "pssm21_rt_upd>tpssm35["RET_TIME"] = [{0}]", tpssm35["RET_TIME"].ToString());

		tpssm11["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
		tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
		{
			tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().TrimOrBlank();
			Log::Trace("", __FUNCTION__, "pssm21_rt_upd>PONO = [{0}]", tpssm11["PONO"].ToString());

			if (tpssm11.Query("FACTORY_DIV,HEAT_NO,PONO") == false)
			{
				CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, "熔炼号[{0}]不存在。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		else
		{
			//--------------------------------------------------------------------------------
			//逻辑校验
			if (tpssm11.Query("FACTORY_DIV,HEAT_NO") == false)
			{
				CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, "熔炼号[{0}]不存在。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		
		tpssm35["PONO"] = tpssm11["PONO"];
		tpssm11_ret.CopyFrom(tpssm11);
		tpssm11_ret.Insert();
		sqlstr = "SELECT DEV_CODE FROM TPSSM12 "
			"	WHERE SM_PLAN_NO = @sm_plan_no "
			"	AND START_TIME_REAL <> ' ' ORDER BY CHARGE_NO DESC ";

		cmd_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString().Trim());
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			source_dev = cmd_inq.GetString(1);
		}
		cmd_inq.Close();

		if (tpssm11["STEEL_RETURN_CODE"].ToString().Trim() == "1")
		{
			ret = tpssm35.QueryCount("HEAT_NO,FACTORY_DIV,RET_HEAT_NO");
			if (ret == 1)
			{
				tpssm35["REC_REVISE_TIME"] = date_time;
				tpssm35["REC_REVISOR"] = s.userid;
				
				//插入一条新记录到TPSSM35
				sqlstr = "tpssm35.Update()";
				if (tpssm35.Update("REC_REVISE_TIME,REC_REVISOR,RETURN_MLSL,RET_TIME","FACTORY_DIV,HEAT_NO,RET_HEAT_NO") == false)
				{
					strcpy(s.msg, "tpssm35 Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				proc_flag = "U";
			}
			//CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
			//CMessageFormat::Format(s.msg, _RES("PSSMS0000199")/*熔炼号={0}的计划已经回炉。*/, arguments, 1);
			//throw CApplicationException(-1, s.msg, log.Location);
			else if (ret == 0)
			{
				tpssm35["PONO"] = tpssm11["PONO"];
				//tpssm35["RET_PONO"] = tpssm11["PONO"];
				tpssm35["DEV_CODE"] = source_dev;
				tpssm35["REC_CREATE_TIME"] = date_time;
				tpssm35["REC_REVISE_TIME"] = date_time;
				tpssm35["REC_CREATOR"] = s.userid;
				tpssm35["REC_REVISOR"] = s.userid;
				tpssm35["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm35["STEEL_RETURN_CODE"] = "1";
				tpssm35["RET_DEST"] = bcls_rec->Tables[0].Rows[0]["RETURNPOS"].ToString();

				if (tpssm35["HEAT_NO"].ToString().Trim() != "" && tpssm35["RET_HEAT_NO"].ToString().Trim() != "" )//&& tpssm35["RETURN_MLSL"].ToDecimal() != 0
				{
					//插入一条新记录到TPSSM35
					sqlstr = "tpssm35.Insert()";
					if (tpssm35.Insert() == false)
					{
						strcpy(s.msg, "tpssm35 insert failed.");
						throw CApplicationException(-1, s.msg, log.Location);
					}
					proc_flag = "I";
				}
			}
		}
		
		else if (tpssm11["STEEL_RETURN_CODE"].ToString().Trim() == "")
		{
			if (tpssm11["RUN_STATUS"].ToString().Trim() < "30")
			{
				CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, _RES("PSSMS0000171")/*熔炼号=[{0}]的计划未开始生产，不能回炉。*/, arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (tpssm11["RUN_STATUS"].ToString().Trim() >= "51")
			{
				CFormattable arguments[] = { tpssm35["HEAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, "熔炼号=[{0}]已经到连铸，不能回炉。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1) < "3")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, "熔炼号=[{0}]没有到达转炉工序不能回炉。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}


			inblock2.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"]; //wcy 回炉删除计划
			ret = f_plan_delete_snd2(&inblock2, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////判断后续炉次是否已经开始生产
			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:	        // MS SQL Server数据库
			//case DB_KIND_ORACLE:	        // Oracle 数据库
			//default:

			//	sqlstr = "SELECT COUNT(1) \
									//			  FROM TPSSM12 \
									//			  WHERE DEV_CODE = (SELECT DEV_CODE FROM TPSSM12 WHERE AREA_ID = 3 AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO) \
									//				AND START_TIME_REAL > (SELECT START_TIME_REAL FROM TPSSM12 WHERE AREA_ID = 3 AND SM_PLAN_NO = @tpssm11["SM_PLAN_NO"].ToString()) ";
			//	break;
			//}
			//cmd_inq.SetCommandText(sqlstr);
			//cmd_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			//v_count = cmd_inq.ExecuteScalar().ToInt32();

			//if (v_count > 0)
			//{
			//	CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() };
			//	CMessageFormat::Format(s.msg, "熔炼号=[{0}]后有炉次已经开始生产,不能回炉。", arguments, 1);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			sqlstr = "tep0002.Query()";
			tep0002["CODE"] = "1";
			tep0002["CODE_CLASS"] = "PSAG2N";//wcy 二钢小代码
			bool pz = tep0002.Query("CODE, CODE_CLASS");

			if (pz == true)
			{
				//--------------------------------------------------------------------------------
				//**生成后备炉次命令,原有制造命令可继续使用**
				// 获取 PONO 号和 CAST_LOT 号
				inBlock1.Tables[0].Rows.Clear();
				inBlock1.Tables[0].Rows.Add();
				inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
				inBlock1.Tables[0].Rows[0]["CS_PLAN_MAKER"] = s.userid;
				inBlock1.Tables[0].Rows[0]["GETSEQ_DIF"] = "2";//2-生成PONO号
				inBlock1.Tables[0].Rows[0]["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];

				EDLog(1, 1, "----- 生成PONO开始 -----");
				ret = f_pssm_get_no(&inBlock1, &outBlock, conn);
				if (ret == -1)
				{
					EDLog(1, 1, "调用f_pssm_get_no()生成PONO出错:[%s]", s.msg);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//获取生成的PONO
				v_pono = outBlock.Tables[0].Rows[0]["PONO"];
				EDLog(1, 1, "v_pono = [%s]", (const char*)v_pono);

				if (v_pono.Trim() == "")
				{
					strcpy(s.msg, "生成的制造命令号为空。");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//更新tpssm10 11 12 35表
				tpssm10["PONO"] = tpssm11["PONO"];
				tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				sqlstr = "tpssm10.Query()";
				tpssm10.Query();

				tpssm10["PONO_STATUS"] = 83;
				tpssm10["PONO"] = v_pono;
				tpssm10["HOT_CHARGE_FLAG"] = "0";
				tpssm10["HOT_SEND_FLAG"] = "0";
				tpssm10["REC_CREATE_TIME"] = date_time;
				tpssm10["REC_CREATOR"] = s.userid;
				tpssm10["REC_REVISE_TIME"] = date_time;
				tpssm10["REC_REVISOR"] = s.userid;
				sqlstr = "tpssm10.Insert()";
				tpssm10.Insert();

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " UPDATE TPSSM11 \
					SET PONO_STATUS = '83', \
					RUN_STATUS = '53',   \
					STEEL_RETURN_CODE = '1',\
					PONO = @v_pono \
					WHERE FACTORY_DIV = @tpssm35.FACTORY_DIV \
					AND HEAT_NO    = @tpssm35.HEAT_NO ";
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("v_pono", v_pono);
				cmd_inq.Parameters.Set("tpssm35.FACTORY_DIV", tpssm35["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm35.HEAT_NO", tpssm35["HEAT_NO"].ToString());
				cmd_inq.ExecuteNonQuery();
				cmd_inq.Close();

				//tpssm13["HEAT_NO"] = tpssm11["HEAT_NO"];
				//tpssm13.Delete("FACTORY_DIV, HEAT_NO");

				tpssm01["PONO_STATUS"] = 15;
				tpssm01["PONO"] = tpssm11["PONO"];
				tpssm01.Update("PONO_STATUS", "PONO");

				tpssm10["PONO_STATUS"] = 15;
				tpssm10["PONO"] = tpssm11["PONO"];
				tpssm10.Update("PONO_STATUS", "PONO");

				//当前工序结束
				tpssm12["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["CHARGE_NO"] = tpssm11["CURR_WP_NO"];
				sqlstr = "tpssm12.Query()";
				tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

				/*Log::Info("", __FUNCTION__, "tpssm12["END_TIME_REAL"] = [{0}]", tpssm12["END_TIME_REAL"].ToString());*/
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
				{
					tpssm12["END_TIME_REAL"] = tpssm12["END_TIME"];

					sqlstr = "tpssm12.Update()";
					tpssm12.Update("END_TIME_REAL", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
				}

				////删除tpssm12表当前工序以后的路径
				//sqlstr = "delete from tpssm12 "
				//	" where factory_div = @factory_div "
				//	" and sm_plan_no = @sm_plan_no "
				//	" and charge_no > @charge_no ";
				//cmd_tpssm12_inq.SetCommandText(sqlstr);
				//cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm35["FACTORY_DIV"].ToString());
				//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
				//cmd_tpssm12_inq.Parameters.Set("charge_no", tpssm11["CURR_WP_NO"].ToDecimal());
				//ret = cmd_tpssm12_inq.ExecuteNonQuery();
				//if (ret <= 0)
				//{
				//	strcpy(s.msg, "delete tpssm12 failed.");
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}


				ret = tpssm35.QueryCount("PONO,FACTORY_DIV");
				if (ret == 0)
				{
					tpssm35["PONO"] = v_pono;
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssm35["REC_CREATE_TIME"] = date_time;
					tpssm35["REC_REVISE_TIME"] = date_time;
					tpssm35["REC_CREATOR"] = s.userid;
					tpssm35["REC_REVISOR"] = s.userid;
					tpssm35["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["STEEL_RETURN_CODE"] = "1";
					tpssm35["RET_DEST"] = bcls_rec->Tables[0].Rows[0]["RETURNPOS"].ToString();

					//if (tpssm35["HEAT_NO"].ToString().Trim() != "" && tpssm35["RET_HEAT_NO"].ToString().Trim() != "" )//&& tpssm35["RETURN_MLSL"].ToDecimal() != 0
					//{
						//插入一条新记录到TPSSM35
						sqlstr = "tpssm35.Insert()";
						if (tpssm35.Insert() == false)
						{
							strcpy(s.msg, "tpssm35 insert failed.");
							throw CApplicationException(-1, s.msg, log.Location);
						}
						proc_flag = "I";
					//}
				}

				//-----------------------------------------------
				//发送炼钢PONO状态
				//inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				//inBlock.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
				//inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 15;

				//ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);
				//if (ret != 0) //调用不成功
				//{
				//	//Log::Trace("", __FUNCTION__,  "f_cm_gegm09_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
				//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

			}
			else
			{
				//--------------------------------------------------------------------------------
				//**不生成后备炉次命令,原有制造命令不可继续使用**

				//当前工序结束
				tpssm12["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["CHARGE_NO"] = tpssm11["CURR_WP_NO"];
				sqlstr = "tpssm12.Query()";
				tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");

				/*Log::Info("", __FUNCTION__, "tpssm12["END_TIME_REAL"] = [{0}]", tpssm12["END_TIME_REAL"].ToString());*/
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
				{
					tpssm12["END_TIME_REAL"] = tpssm12["END_TIME"];

					sqlstr = "tpssm12.Update()";
					tpssm12.Update("END_TIME_REAL", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
				}

				//更新TPSSM11
				tpssm11["STEEL_RETURN_CODE"] = "1";
				tpssm11["PONO_STATUS"] = 83;
				tpssm11["RUN_STATUS"] = "83";
				tpssm11["CURR_WP_NO"] = 9;
				tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
				tpssm11["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
				tpssm11.Update("STEEL_RETURN_CODE,PONO_STATUS,RUN_STATUS,CURR_WP_NO", "HEAT_NO,FACTORY_DIV");

				//更新TPSSM10
				tpssm10["PONO_STATUS"] = 83;
				tpssm10["PONO"] = tpssm35["PONO"];
				tpssm10["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
				tpssm10.Update("PONO_STATUS", "PONO,FACTORY_DIV");

				//插入TPSSM35表
				ret = tpssm35.QueryCount("PONO,FACTORY_DIV");
				if (ret == 0)
				{
					//tpssm35["PONO"] = v_pono;
					tpssm35["RET_PONO"] = tpssm11["PONO"];
					tpssm35["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssm35["REC_CREATE_TIME"] = date_time;
					tpssm35["REC_REVISE_TIME"] = date_time;
					tpssm35["REC_CREATOR"] = s.userid;
					tpssm35["REC_REVISOR"] = s.userid;
					tpssm35["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
					tpssm35["STEEL_RETURN_CODE"] = "1";
					tpssm35["RET_DEST"] = bcls_rec->Tables[0].Rows[0]["RETURNPOS"].ToString();

					//if (tpssm35["HEAT_NO"].ToString().Trim() != "" && tpssm35["RET_HEAT_NO"].ToString().Trim() != "")// && tpssm35["RETURN_MLSL"].ToDecimal() != 0
					//{
						//插入一条新记录到TPSSM35
						sqlstr = "tpssm35.Insert()";
						if (tpssm35.Insert() == false)
						{
							strcpy(s.msg, "tpssm35 insert failed.");
							throw CApplicationException(-1, s.msg, log.Location);
						}
						proc_flag = "I";
					//}
				}

				//-----------------------------------------------
				//发送炼钢PONO状态
				inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				inBlock.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
				inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 83;

				//ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);
				//if (ret != 0) //调用不成功
				//{
				//	//Log::Trace("", __FUNCTION__,  "f_cm_gegm09_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
				//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

			}



			//-----------------------------------------------
			//调用炼钢计划履历函数
			Log::Trace("", __FUNCTION__, "调用炼钢计划履历函数");
			CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
			row99["PONO"] = v_pono;
			row99["HEAT_NO"] = tpssm11["HEAT_NO"];
			row99["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			row99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
			row99["DEV_CODE"] = tpssm35["DEV_CODE"];
			row99["RETURN_MLSL"] = tpssm35["RETURN_MLSL"];
			row99["EVENT_ID"] = "G1"; //G1	钢水返送

			ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
			if (ret != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			////回炉信息下达给炼钢L2
			//Log::Trace("", __FUNCTION__, "计划下达给L2");
			//inSendL2.Tables[0].Rows[0]["HEAT_NO"] = tpssm11["HEAT_NO"];
			//inSendL2.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
			//inSendL2.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];

			//ret = f_pssm_rt_sendl2(&inSendL2, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
		}

		if (tpssm35["HEAT_NO"].ToString().Trim() != "" && tpssm35["RET_HEAT_NO"].ToString().Trim() != "" )//&& tpssm35["RETURN_MLSL"].ToDecimal() != 0
		{
			

			/*inblkmmsm.Tables[0].Rows[0]["PROC_FLAG"] = proc_flag;
			inblkmmsm.Tables[0].Rows[0]["DEV_CODE"] = source_dev;
			inblkmmsm.Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"];
			inblkmmsm.Tables[0].Rows[0]["MB_HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString().TrimOrBlank();
			inblkmmsm.Tables[0].Rows[0]["WTS"] = tpssm35["RETURN_MLSL"];
			inblkmmsm.Tables[0].Rows[0]["FLAG"] = "1";
			ret = f_mmsmgglwt_proc(&inblkmmsm, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}*/


			sqlstr = "  SELECT DA_HEAT_BACK_SEQ.nextval FROM dual  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				resume_seq_no = cmd_inq.GetDecimal(1);
			}

			sqlstr = "  SELECT ST_NO FROM TPSSM11 WHERE HEAT_NO = @heat_no  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("heat_no", tpssm35["RET_HEAT_NO"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				ret_st_no = cmd_inq.GetString(1);
			}

			sqlstr = " INSERT INTO DA_HEAT_BACK (ID,HEAT_NUMBER,HEAT_BACK_RATIO,HEAT_BACK,CONSUME_ID,TIME_STAMP,IS_CANCEL,IS_UP,IS_UP_TIME,SCRAP_WEIGHT,STEEL_BACK) VALUES(@resume_seq_no, @heat_no, @heat_back_ratio, @heat_back, @consume_id, SYSDATE, @is_cancel, @is_up, @is_up_time, @scrap_weight, @st_no) ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("resume_seq_no", resume_seq_no);
			cmd_inq.Parameters.Set("heat_no", tpssm35["HEAT_NO"].ToString());
			cmd_inq.Parameters.Set("heat_back_ratio", 0);//比例?
			cmd_inq.Parameters.Set("heat_back", tpssm35["RET_HEAT_NO"].ToString());
			cmd_inq.Parameters.Set("consume_id", 0);
			cmd_inq.Parameters.Set("is_cancel", "S");
			cmd_inq.Parameters.Set("is_up", "S");
			cmd_inq.Parameters.Set("is_up_time", "");
			cmd_inq.Parameters.Set("scrap_weight", 0);
			cmd_inq.Parameters.Set("st_no", ret_st_no);
			cmd_inq.ExecuteNonQuery();
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
	cmd_tpssm11_inq.Close();
	return doFlag;

}

