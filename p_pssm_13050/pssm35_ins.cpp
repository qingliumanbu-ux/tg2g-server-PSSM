/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2015-07-14
Version:  3.1.0
Description: 炉次返送计划新增  返送代码1：回炉；3：返送精炼
**************************************************/
//框架头文件
#include "stdafx.h"

//业务头文件






/*<remark>=========================================================
///<summary>
///炉次返送计划新增
///<para>炉次返送计划新增</para>
///</summary>
/// <param name="sm_plan_no">2个炼钢计划号  </param>
/// <returns>无</returns>
===========================================================</remark>*/
//int f_pssm_nvnkg3_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
// service入口
BM2F_ENTERACE(pssm35_ins)

int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
int f_pssm35_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	int ret = 0;
	int blkseq = 0, rownum = 0, i = 0; //读取传入参数用

	CString datetime = "";
	CDecimal sm_plan_no = 0;
	CString  return_mode = "";
	CString event_id = "";

	/* 实体类定义 */
	CModel tpssm35("TPSSM35");
	CModel tpssm11("TPSSM11");
	CModel tpssm10("TPSSM10");
	CModel tpssm01("TPSSM01");
	CModel tpssm11_d("TPSSM11");
	CModel tpssm26("TPSSM26");


	/* 数据库操作类定义 */
	CString sqlstr("");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);

	CModel tpssm99("TPSSM99");
	EIClass pssm99trace;//调用履历函数
	pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位

	EIClass inBlock1;
	//定义函数调用信息块		
	inBlock1.Tables[0].Columns.Add(tpssm11);
	inBlock1.Tables[0].Rows.Add();


	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");


		//--------------------------------------------------------------
		//获得输入参数
		blkseq = bcls_rec->Tables.IndexOf("HTNO_RET");
		if (blkseq < 0)
		{
			strcpy(s.msg, "炉次返送数据块[HTNO_RET]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [HTNO_RET] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		rownum = bcls_rec->Tables[blkseq].Rows.get_Count();

		for (i = 0; i < rownum; i++)
		{
			tpssm35["SM_PLAN_NO"] = bcls_rec->Tables[blkseq].Rows[0]["SM_PLAN_NO"]; //返送的计划号
			tpssm35["RETURN_PLAN_NO"] = bcls_rec->Tables[blkseq].Rows[i]["DEST_PLAN_NO"];//返送目的计划号
			return_mode = bcls_rec->Tables[blkseq].Rows[0]["RETURN_MODE"].ToString().Trim(); //返送方式：1-回炉；2-炉后
			tpssm35["RETURN_STEEL_WT"] = bcls_rec->Tables[blkseq].Rows[0]["RETURN_WT"].ToDecimal().ToInt32(); //返送钢水量
			tpssm35["REMARK"] = bcls_rec->Tables[blkseq].Rows[0]["REMARK"].ToString().Trim(); //备注
			tpssm35["LADLE_NO"] = bcls_rec->Tables[blkseq].Rows[0]["LADLE_NO"].ToString().Trim();
			tpssm35["LADLE_TYPE"] = bcls_rec->Tables[blkseq].Rows[0]["LADLE_TYPE"].ToString().Trim();
			//Log::Info("", __FUNCTION__, "返送计划号=[{0}], 目的计划号=[{1}]", tpssm35.SM_PLAN_NO, tpssm35.RETURN_PLAN_NO);
			//Log::Info("", __FUNCTION__, "return_mode=[{0}], RETURN_STEEL_WT=[{1}]", return_mode, tpssm35.RETURN_STEEL_WT);
			//Log::Info("", __FUNCTION__, "tpssm35["REMARK"] =[{0}]", tpssm35["REMARK"].ToString());


			//基本校验
			//1.返送钢水的计划号不能为空
			if (tpssm35["SM_PLAN_NO"].ToString().Trim() == "")
			{
				CFormattable arguments[] = { tpssm35["SM_PLAN_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "返送钢水的计划号[{0}]不能为空。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//2.返送钢水量不能为零
			if (tpssm35["RETURN_STEEL_WT"].ToDecimal() <= 0)
			{
				CFormattable arguments[] = { tpssm35["RETURN_STEEL_WT"].ToDecimal() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "返送钢水的重量[{0}]不能为零。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//3.返送计划是否存在			
			//sqlstr = "tpssm35.Insert()";
			//int count = tpssm35.QueryCount("SM_PLAN_NO");
			//if (count > 0)
			//{
			//	CFormattable arguments[] = { tpssm35.SM_PLAN_NO }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "计划号[{0}]已做过返送，不能再做。", arguments, 1); //格式化字符串
			//	Log::Trace("", __FUNCTION__, "{0}", s.msg);
			//	//throw CApplicationException(-1, s.msg, log.Location);
			//}

			//查询炉次信息
			tpssm11["SM_PLAN_NO"] = tpssm35["SM_PLAN_NO"].ToString();
			sqlstr = "tpssm11.Query";
			bool has11 = tpssm11.Query("SM_PLAN_NO");
			if (has11 == false)
			{
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "返送计划号[{0}]在计划表TPSSM11中不存在。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//已做过返送的不能再做
			if (tpssm11["STEEL_RETURN_CODE"].ToString().Trim() != "")
			{
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "返送计划号[{0}]已做过回炉和返送，不能再操作。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//如果开浇结束，不允许做回炉
			if (tpssm11["PONO_STATUS"].ToDecimal() >= 83)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炉次[{0}]已浇铸结束，不允许操作。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//4.如果是回炉，必须出钢结束
			if (return_mode.Trim() == "1" && tpssm11["PONO_STATUS"].ToDecimal() <23)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炉次[{0}]未出钢，不能做重装回炉。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//5.如果是返送-炉后，必须开浇，但不能浇铸结束，才可以做
			//if (return_mode.Trim() == "2" && tpssm11["PONO_STATUS"].ToDecimal() <82)
			//{
			//	CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "炉次[{0}]未开浇，不能做返送-炉后。", arguments, 1); //格式化字符串
			//	Log::Trace("", __FUNCTION__, "{0}", s.msg);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			////6.浇铸结束不能做返送-炉后
			//if (return_mode.Trim() == "2" && tpssm11["PONO_STATUS"].ToDecimal() == 83)
			//{
			//	CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "炉次[{0}]浇铸结束，不能做返送-炉后。", arguments, 1); //格式化字符串
			//	Log::Trace("", __FUNCTION__, "{0}", s.msg);
			//	throw CApplicationException(-1, s.msg, log.Location);
			/*}*/
			//查找铸坯是否产出
			sqlstr = " SELECT COUNT(*) FROM TMMSM02 "
				"  WHERE HEAT_NO = @heat_no ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("heat_no", tpssm11["HEAT_NO"].ToString());
			cmd_inq.ExecuteReader();
			int sj_count = cmd_inq.ExecuteScalar().ToInt32();
			Log::Info("", __FUNCTION__, " sj_count=[{0}]", sj_count);
			cmd_inq.Close();
			//7.返送-炉后必须有铸坯产出
			if (return_mode.Trim() == "2" && sj_count <= 0)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炉次[{0}]无铸坯产出，不能做返送-炉后。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//8.如果是返送-精炼，必须出钢结束
			if (return_mode.Trim() == "3" && tpssm11["PONO_STATUS"].ToDecimal() <23)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炉次[{0}]未出钢结束，不能做返送-精炼。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//9.如果是返送-精炼，不允许浇铸结束
			if (return_mode.Trim() == "3" && tpssm11["PONO_STATUS"].ToDecimal() == 83)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炉次[{0}]已浇完，不能做返送-精炼。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//10.如果是返送-精炼，不允许有铸坯产出
			if (return_mode.Trim() == "3" && sj_count>0)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炉次[{0}]有铸坯产出，不能做返送-精炼。", arguments, 1); //格式化字符串
				Log::Trace("", __FUNCTION__, "{0}", s.msg);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//查询目标制造命令号
			if (tpssm35["RETURN_PLAN_NO"].ToString().Trim() != "")
			{
				tpssm11_d["SM_PLAN_NO"] = tpssm35["RETURN_PLAN_NO"].ToString();
				sqlstr = "tpssm11_d.Query";
				bool has11 = tpssm11_d.Query("SM_PLAN_NO");
				if (has11 == false)
				{
					CFormattable arguments[] = { tpssm11_d["SM_PLAN_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "返送目的计划号[{0}]不存在，请重新选择。", arguments, 1); //格式化字符串
					Log::Trace("", __FUNCTION__, "{0}", s.msg);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			tpssm35["PONO"] = tpssm11["PONO"]; //	制造命令号
			tpssm35["HEAT_NO"] = tpssm11["HEAT_NO"]; //	出钢钢号
			tpssm35["ST_NO"] = tpssm11["ST_NO"]; //	出钢记号
			tpssm35["RETURN_PLAN_NO"] = tpssm11_d["SM_PLAN_NO"];
			tpssm35["RET_PONO"] = tpssm11_d["PONO"]; //	返送目的制造命令号
			tpssm35["STEEL_RETURN_CODE"] = return_mode; //	钢水返送代码:1-回炉；2-炉后（CRT空走）
			tpssm35["RETURN_HEAT_NO"] = tpssm11_d["HEAT_NO"];
			tpssm35["DEST_WP"]  = "";    //	返送目的工序
			tpssm35["RET_TIME"] = datetime;
			tpssm35["REC_CREATE_TIME"] = datetime;
			tpssm35["REC_CREATOR"] = s.userid;

			sqlstr = "tpssm10.Query";
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10.Query("PONO");
			tpssm35["LG_ST"] = tpssm10["LG_ST"];
			tpssm35.Print();
			sqlstr = "tpssm35.Insert()";
			tpssm35.TrimOrBlank();
			tpssm35.Insert();

			if (return_mode.Trim() == "1")
			{
				event_id = "21";
			}
			else if (return_mode.Trim() == "2")
			{
				event_id = "25";
			}
			else if (return_mode.Trim() == "3")
			{
				event_id = "26";
			}
			tpssm99["PONO"] = tpssm11["PONO"];
			tpssm99["EVENT_ID"] = event_id;
			tpssm99["VALID_FLAG"] = "0";
			tpssm99.MergeTo(pssm99trace.Tables["TRACE"], false);
		}//for

		//炉后精炼，置制造命令状态为出钢结束
		if (return_mode.Trim() == "3")
		{
			//更新tpssm11表的计划状态
			tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
			tpssm11["STEEL_RETURN_CODE"] = return_mode;
			tpssm11["PONO_STATUS"] = 23;
			tpssm11["RUN_STATUS"] = "36";
			tpssm11.Update("STEEL_RETURN_CODE,PONO_STATUS,RUN_STATUS", "HEAT_NO");

			//更新tpssm10表计划状态
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10["PONO_STATUS"] = 23;
			tpssm10["STEEL_RETURN_CODE"]  = return_mode;
			tpssm10.Update("STEEL_RETURN_CODE,PONO_STATUS", "PONO");

			//更新tpssm01表计划状态
			tpssm01["PONO"] = tpssm11["PONO"];
			tpssm01["PONO_STATUS"] = 23;
			tpssm01.Update("PONO_STATUS", "PONO");

			tpssm11.MergeTo(inBlock1.Tables[0], false);

		}
		//只有回炉和返送-炉后，置制造命令状态为浇铸结束
		if (return_mode.Trim() != "3")
		{
			//更新tpssm11表的计划状态
			tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
			tpssm11["STEEL_RETURN_CODE"] = return_mode;
			tpssm11["PONO_STATUS"] = 83;
			tpssm11["RUN_STATUS"] = "53";
			tpssm11.Update("STEEL_RETURN_CODE,PONO_STATUS,RUN_STATUS", "HEAT_NO");

			//更新tpssm10表计划状态
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10["PONO_STATUS"] = 83;
			tpssm10["STEEL_RETURN_CODE"]  = return_mode;
			tpssm10.Update("STEEL_RETURN_CODE,PONO_STATUS", "PONO");

			//更新tpssm01表计划状态
			tpssm01["PONO"] = tpssm11["PONO"];
			tpssm01["PONO_STATUS"] = 83;
			tpssm01.Update("PONO_STATUS", "PONO");

			//更新cc_seq
			tpssm10.Query("PONO");
			if (tpssm10["CC_SEQ"].ToDecimal() > 0)
			{
				//修改后续炉次的顺序号
				sqlstr = CString(
					" UPDATE TPSSM10 "
					"   SET CC_SEQ      = CC_SEQ - 1 "
					" WHERE CC_MACH_NO  = @cc_mach_no "
					"   AND CC_SEQ      > @cc_seq "
					"   AND CC_SEQ      < 900 "  //900以后的都是封锁的计划
					);

				cmd_upd.SetCommandText(sqlstr);
				cmd_upd.Parameters.Set("cc_mach_no", tpssm10["CC_MACH_NO"].ToString());
				cmd_upd.Parameters.Set("cc_seq", tpssm10["CC_SEQ"].ToDecimal());
				cmd_upd.ExecuteNonQuery();

				tpssm10["CC_SEQ"] = 0;
				sqlstr = "tpssm10.Update()";
				tpssm10.Update("CC_SEQ", "PONO");
			}
			tpssm26["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
			bool has26 = tpssm26.Query("CC_MACH_NO");
			//Log::Trace("", __FUNCTION__, "tpssm11["PONO_STATUS"] =[{0}]", tpssm11["PONO_STATUS"].ToDecimal());
			//更新26表的浇次号，对开浇或浇注完的信号, 需记录最新的CAST_NO
			if (tpssm11["PONO_STATUS"].ToDecimal() == 82 || tpssm11["PONO_STATUS"].ToDecimal() == 83) //82-开浇, 83-浇完
			{
				Log::Trace("", __FUNCTION__, "cast_no=[{0}], cast_div_no=[{1}]", tpssm26["CAST_NO"].ToString(), tpssm26["CAST_DIV_NO"].ToDecimal());
				Log::Trace("", __FUNCTION__, "cast_no=[{0}], cast_div_no=[{1}]", tpssm11["CAST_NO"].ToString(), tpssm11["CAST_DIV_NO"].ToDecimal());
				if (tpssm26["CAST_NO"].ToString() < tpssm11["CAST_NO"].ToString())
				{
					if (tpssm26["PREV_CAST_NO"].ToString()  != tpssm26["CAST_NO"].ToString())
					{
						tpssm26["PREV_CAST_NO"] = tpssm26["CAST_NO"].ToString();  //记录上一CAST号
						tpssm26["PREV_CAST_DIV_NO"] = tpssm26["CAST_DIV_NO"].ToString();
					}
					tpssm26["CAST_NO"] = tpssm11["CAST_NO"];
					tpssm26["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
				}
				else
				{
					if (tpssm26["CAST_NO"].ToString() == tpssm11["CAST_NO"].ToString() && tpssm26["CAST_DIV_NO"].ToDecimal() < tpssm11["CAST_DIV_NO"].ToDecimal())
					{
						if (tpssm26["PREV_CAST_DIV_NO"].ToDecimal() < tpssm26["CAST_DIV_NO"].ToDecimal())
						{
							tpssm26["PREV_CAST_NO"] = tpssm26["CAST_NO"];     //记录上一炉CAST号
							tpssm26["PREV_CAST_DIV_NO"] = tpssm26["CAST_DIV_NO"]; //记录上一炉CAST内顺序号
						}
						tpssm26["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
					}
				}
				Log::Trace("", __FUNCTION__, "cast_no=[{0}], cast_div_no=[{1}]", tpssm26["CAST_NO"].ToString(), tpssm26["CAST_DIV_NO"].ToDecimal());
				tpssm26["REC_CREATOR"] = CString(s.userid);
				tpssm26["REC_CREATE_TIME"] = datetime;
				tpssm26.Print();
				sqlstr = "tpssm26.Update()";
				tpssm26.Update("CAST_NO"
					",CAST_DIV_NO"
					",PREV_CAST_NO"
					",PREV_CAST_DIV_NO"
					",REC_REVISOR,REC_REVISE_TIME",
					"CC_MACH_NO");
			}
			tpssm11.MergeTo(inBlock1.Tables[0], false);

		}
		//记录编入计划成功的履历		
		ret = f_pssm99_trace(&pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//ret = f_pssm_nvnkg3_snd(&inBlock1, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
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
