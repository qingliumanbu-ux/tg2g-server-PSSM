/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-26
Version:  3.1.0
Description: 甘特图出钢计划编制保存
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
//int f_pssm_castlot_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划编制保存
/// <para>根据Client甘特图输入的数据，修改出钢计划。  </para>
/// <para>
///   1.出钢计划信息写入:主计划与工序计划
///   2.删除排除的炉次
///   3.CAST号计算
///   4.计划号计算
///   5.处理号计算
/// <para>数据库表：TPSSM11/12                   </para>
/// <para>主调用函数：PSSM18画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>制造命令号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_pssm12)

int f_pssm18_pssm12(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	CString sm_plan_no = "";
	CDecimal charge_no = 0;      //工序charge号
	CString password = "";
	CDecimal area_id = 0;
	CString datetime = "";

	EIClass inblock;
	EIClass outblock;
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		CModel tpssm11("TPSSM11");
		CModel tpssm12("TPSSM12");

		CModel tpssm41("TPSSM41");
		CModel tpssm42("TPSSM42");
		
		//--------------------------------
		/* ***** 获取输入参数 ***** */
		
		sm_plan_no = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"].ToString().Trim();
		charge_no = bcls_rec->Tables[0].Rows[0]["CHARGE_NO"].ToDecimal();
		password = bcls_rec->Tables[0].Rows[0]["PASSWORD"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "打印输入参数sm_plan_no = [{0}]", sm_plan_no);
		Log::Trace("", __FUNCTION__, "打印输入参数charge_no = [{0}]", charge_no);
		Log::Trace("", __FUNCTION__, "打印输入参数password = [{0}]", password);

		sqlstr = " SELECT CODE_DESC_1_CONTENT FROM TEP0002  "
			"	WHERE CODE_CLASS = 'PSAS2N' "
			"	  AND CODE = '1' "
			"	  AND CODE_DESC_2_CONTENT = '1' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetString(1).Trim() != password.Trim())
			{
				CFormattable arguments[] = { sm_plan_no }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]删除失败,密码错误", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		cmd_inq.Close();

		tpssm11["SM_PLAN_NO"] = sm_plan_no;
		tpssm11.Query("SM_PLAN_NO");
		if (tpssm11.Query("SM_PLAN_NO"))
		{
			if (tpssm11["CURR_WP_NO"].ToDecimal() > charge_no)
			{
				tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["CHARGE_NO"] = charge_no;
				tpssm12.Query("SM_PLAN_NO,CHARGE_NO");

				if (tpssm12["AREA_ID"].ToDecimal() == 4 || tpssm12["AREA_ID"].ToDecimal() == 2)
				{
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "" && tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{
						tpssm12.Delete("SM_PLAN_NO,CHARGE_NO");

						sqlstr = "UPDATE TPSSM12 "
							"	SET	CHARGE_NO = CHARGE_NO - 1 "
							"	WHERE SM_PLAN_NO = @sm_plan_no "
							"	AND CHARGE_NO > @charge_no ";

						cmd_upd.Parameters.Set("sm_plan_no", sm_plan_no);
						cmd_upd.Parameters.Set("charge_no", charge_no);
						cmd_upd.SetCommandText(sqlstr);
						cmd_upd.ExecuteNonQuery();

						sqlstr = "UPDATE TPSSM11 "
							"	SET	CURR_WP_NO = CURR_WP_NO - 1 "
							"	WHERE SM_PLAN_NO = @sm_plan_no ";

						cmd_upd.Parameters.Set("sm_plan_no", sm_plan_no);
						cmd_upd.SetCommandText(sqlstr);
						cmd_upd.ExecuteNonQuery();
						cmd_upd.Close();
					}
					else
					{
						CFormattable arguments[] = { tpssm12["START_TIME_REAL"].ToString(), tpssm12["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "存在实绩时间，实绩开始时间[{0}],实绩结束时间[{1}]", arguments, 2); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else
				{
					CFormattable arguments[] = { tpssm12["AREA_ID"].ToDecimal() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "不允许删除脱碳或者连铸设备", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (tpssm11["CURR_WP_NO"].ToDecimal() < charge_no)
			{
				tpssm12["SM_PLAN_NO"] = sm_plan_no;
				tpssm12["CHARGE_NO"] = charge_no;
				tpssm12.Query("SM_PLAN_NO,CHARGE_NO");
				if (tpssm12["AREA_ID"].ToDecimal() == 4 || tpssm12["AREA_ID"].ToDecimal() == 2)
				{
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "" && tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{
						tpssm12.Delete("SM_PLAN_NO,CHARGE_NO");

						sqlstr = "UPDATE TPSSM12 "
							"	SET	CHARGE_NO = CHARGE_NO - 1 "
							"	WHERE SM_PLAN_NO = @sm_plan_no "
							"	AND CHARGE_NO > @charge_no ";

						cmd_upd.Parameters.Set("sm_plan_no", sm_plan_no);
						cmd_upd.Parameters.Set("charge_no", charge_no);
						cmd_upd.SetCommandText(sqlstr);
						cmd_upd.ExecuteNonQuery();
						cmd_upd.Close();
					}
					else
					{
						CFormattable arguments[] = { tpssm12["START_TIME_REAL"].ToString(), tpssm12["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "存在实绩时间，实绩开始时间[{0}],实绩结束时间[{1}]", arguments, 2); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else
				{
					CFormattable arguments[] = { tpssm12["AREA_ID"].ToDecimal() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "不允许删除脱碳或者连铸设备", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}
		else
		{
			tpssm41["SM_PLAN_NO"] = sm_plan_no;
			if (tpssm41.Query("SM_PLAN_NO"))
			{
				if (tpssm41["CURR_WP_NO"].ToDecimal() > charge_no)
				{
					tpssm42["SM_PLAN_NO"] = sm_plan_no;
					tpssm42["CHARGE_NO"] = charge_no;
					tpssm42.Query("SM_PLAN_NO,CHARGE_NO");

					if (tpssm42["AREA_ID"].ToDecimal() == 4 || tpssm42["AREA_ID"].ToDecimal() == 2)
					{
						if (tpssm42["START_TIME_REAL"].ToString().Trim() == "" && tpssm42["END_TIME_REAL"].ToString().Trim() == "")
						{
							tpssm42.Delete("SM_PLAN_NO,CHARGE_NO");

							sqlstr = "UPDATE TPSSM42 "
								"	SET	CHARGE_NO = CHARGE_NO - 1 "
								"	WHERE SM_PLAN_NO = @sm_plan_no "
								"	AND CHARGE_NO > @charge_no ";

							cmd_upd.Parameters.Set("sm_plan_no", sm_plan_no);
							cmd_upd.Parameters.Set("charge_no", charge_no);
							cmd_upd.SetCommandText(sqlstr);
							cmd_upd.ExecuteNonQuery();

							sqlstr = "UPDATE TPSSM41 "
								"	SET	CURR_WP_NO = CURR_WP_NO - 1 "
								"	WHERE SM_PLAN_NO = @sm_plan_no ";

							cmd_upd.Parameters.Set("sm_plan_no", sm_plan_no);
							cmd_upd.SetCommandText(sqlstr);
							cmd_upd.ExecuteNonQuery();
							cmd_upd.Close();
						}
						else
						{
							CFormattable arguments[] = { tpssm42["START_TIME_REAL"].ToString(), tpssm42["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "存在实绩时间，实绩开始时间[{0}],实绩结束时间[{1}]", arguments, 2); //格式化字符串
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					else
					{
						CFormattable arguments[] = { tpssm42["AREA_ID"].ToDecimal() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "不允许删除脱碳或者连铸设备", arguments, 1); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else if (tpssm41["CURR_WP_NO"].ToDecimal() < charge_no)
				{
					tpssm42["SM_PLAN_NO"] = sm_plan_no;
					tpssm42["CHARGE_NO"] = charge_no;
					tpssm42.Query("SM_PLAN_NO,CHARGE_NO");
					if (tpssm42["AREA_ID"].ToDecimal() == 4 || tpssm42["AREA_ID"].ToDecimal() == 2)
					{
						if (tpssm42["START_TIME_REAL"].ToString().Trim() == "" && tpssm42["END_TIME_REAL"].ToString().Trim() == "")
						{
							tpssm42.Delete("SM_PLAN_NO,CHARGE_NO");

							sqlstr = "UPDATE TPSSM42 "
								"	SET	CHARGE_NO = CHARGE_NO - 1 "
								"	WHERE SM_PLAN_NO = @sm_plan_no "
								"	AND CHARGE_NO > @charge_no ";

							cmd_upd.Parameters.Set("sm_plan_no", sm_plan_no);
							cmd_upd.Parameters.Set("charge_no", charge_no);
							cmd_upd.SetCommandText(sqlstr);
							cmd_upd.ExecuteNonQuery();
							cmd_upd.Close();
						}
						else
						{
							CFormattable arguments[] = { tpssm42["START_TIME_REAL"].ToString(), tpssm42["END_TIME_REAL"].ToString() }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "存在实绩时间，实绩开始时间[{0}],实绩结束时间[{1}]", arguments, 2); //格式化字符串
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					else
					{
						CFormattable arguments[] = { tpssm42["AREA_ID"].ToDecimal() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "不允许删除脱碳或者连铸设备", arguments, 1); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
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
