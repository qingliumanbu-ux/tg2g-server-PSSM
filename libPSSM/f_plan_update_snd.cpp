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


int f_plan_delete_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_plan_insert_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_plan_update_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
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
int f_plan_update_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;

	CString lpsz_tc_no = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	EPEX epex(&s, conn);

	CString sqlstr;
	CString sqlstr1;
	CString sqlstr2;
	CString tablename1 = "TPSSM11";//主表
	CString tablename2 = "TPSSM12";//子表
	CString tablename3 = "TPSSM13";//复制主表
	CString tablename4 = "TPSSM14";//复制子表
	CString action_flag = " ";
	CString compare_fields11 = " SM_PLAN_NO, ST_NO, CAST_DIV_NO, CAST_PONO_SUM ";
	CString compare_fields12 = " SM_PLAN_NO, CHARGE_NO, DEV_CODE, START_TIME, END_TIME ";
	CString sm_plan_no = ""; //比较计划号范围字符串
	CString sm_plan_no11 = ""; //11表有变动字符串
	CString sm_plan_no12 = ""; //12表有变动字符串
	CString sm_plan_no_in = "";
	CString sm_plan_no_aa = "";
	CString plan_no = "";
	CString DEV_CODE_OLD = "";
	CString DEV_CODE_NEW = "";
	CDecimal spilt_sum = 0;
	CDecimal treatment_count = 0;
	CDecimal complete_flag = 0;
	//CString dev_code = "";
	/*实体类定义*/
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm13("TPSSM13");
	CModel tpssm14("TPSSM14");

	CDataTable tb_tpssm12("TPSSM12");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq1(conn);
	CDbCommand cmd_tpssm12_inq2(conn);

	EIClass inblock;
	try
	{

		/*初始化表结构变量*/

		/* ***** 获取输入参数 ***** */
		//tpssm11["FACTORY_DIV"] = bcls_rec->Tables["X200009"].Rows[0]["FACTORY_DIV"].ToString();
		//tpssm11["PONO"] = bcls_rec->Tables["X200009"].Rows[0]["PONO"].ToString();
		//tpssm11["PONO_STATUS"] = bcls_rec->Tables["X200009"].Rows[0]["PONO_STATUS"].ToDecimal();
		inblock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		//inblock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO_IN");
		inblock.Tables[0].Rows.Add();

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
		Log::Info("", __FUNCTION__, "sm_plan_no upd =[{0}]", sm_plan_no);
		Log::Info("", __FUNCTION__, "sm_plan_no_aa =[{0}]", sm_plan_no_aa);
		////Log::Info("", __FUNCTION__, "pono=[{0}]", tpssm11["PONO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status=[{0}]", tpssm11["PONO_STATUS"].ToDecimal());
		////Log::Info("", __FUNCTION__, "heat_no=[{0}]", tpssm11["HEAT_NO"].ToString());

		/* ***** 检查输入参数合法性 ***** */

		/* ***** 程序处理 ***** */
		if (sm_plan_no.Trim() == "")
		{
			sqlstr = " SELECT SM_PLAN_NO FROM " + tablename1 + " WHERE SM_PLAN_NO IN ( SELECT SM_PLAN_NO FROM " + tablename3 + " ) AND PONO_STATUS < 83 ";
			cmd_tpssm11_inq.SetCommandText(sqlstr);
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
		Log::Info("", __FUNCTION__, "sm_plan_no upd =[{0}]", sm_plan_no);

		if (sm_plan_no.Trim() != "")
		{
			//主表变换（包含钢种变更、浇次顺序变化、浇次炉数变化）update
			sqlstr = " SELECT " + compare_fields11 + " FROM " + tablename1 + " WHERE SM_PLAN_NO IN(" + sm_plan_no + ") MINUS SELECT " + compare_fields11 + " FROM " + tablename3 + " WHERE SM_PLAN_NO IN(" + sm_plan_no + ")";
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				if (sm_plan_no11.Trim() == "") sm_plan_no11 = "A" + cmd_tpssm11_inq.GetString(1) + "A";
				else sm_plan_no11 = sm_plan_no11 + "," + "A" + cmd_tpssm11_inq.GetString(1) + "A";
			}
			cmd_tpssm11_inq.Close();
			Log::Info("", __FUNCTION__, "sm_plan_no11=[{0}]", sm_plan_no11);
			//子表变换（设备号、开始时间、结束时间）update/insert/delete
			sqlstr = " SELECT " + compare_fields12 + " FROM " + tablename2 + " WHERE SM_PLAN_NO IN(" + sm_plan_no + ") MINUS SELECT " + compare_fields12 + " FROM " + tablename4 + " WHERE SM_PLAN_NO IN(" + sm_plan_no + ")";
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				if (sm_plan_no11.Trim() == "") sm_plan_no11 = "A" + cmd_tpssm11_inq.GetString(1) + "A";
				else
				{
					if (sm_plan_no11.Find("A" + cmd_tpssm11_inq.GetString(1) + "A") < 0)
					{
						sm_plan_no11 = sm_plan_no11 + "," + "A" + cmd_tpssm11_inq.GetString(1) + "A";
					}
				}
			}
			cmd_tpssm11_inq.Close();
			Log::Info("", __FUNCTION__, "sm_plan_no11=[{0}]", sm_plan_no11);

			sqlstr = " SELECT SM_PLAN_NO FROM " + tablename1 + " WHERE SM_PLAN_NO IN ( SELECT SM_PLAN_NO FROM " + tablename3 + " ) ";
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				plan_no = cmd_tpssm11_inq.GetString(1);

				if (sm_plan_no11.Find("A" + plan_no + "A") >= 0)
				{
					inblock.Tables[0].Rows[0]["SM_PLAN_NO"] = plan_no;
					//inblock.Tables[0].Rows[0]["SM_PLAN_NO_IN"] = plan_no;
					ret = f_plan_delete_snd(&inblock, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					ret = f_plan_insert_snd(&inblock, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//if (sm_plan_no11.Find(plan_no) >= 0)//钢种变更、浇次顺序变化、浇次炉数变化
					//{
						ret = f_plan_update_snd2(&inblock, bcls_ret, conn);
						if (ret < 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					//}
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
