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
int f_order_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;

	CString lpsz_tc_no = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	EPEX epex(&s, conn);

	CString sqlstr;
	CString dev_tech_code = " ";
	CString text = " ";

	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm11("TPSSM11");

	CDbCommand cmd_inq(conn);

	try
	{

		/*初始化表结构变量*/

		/* ***** 获取输入参数 ***** */
		dev_tech_code = bcls_rec->Tables[0].Rows[0]["DEV_TECH_CODE"].ToString();
		text = bcls_rec->Tables[0].Rows[0]["TEXT"].ToString();

		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "factory_div=[{0}]", tpssm11["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "pono=[{0}]", tpssm11["PONO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status=[{0}]", tpssm11["PONO_STATUS"].ToDecimal());
		////Log::Info("", __FUNCTION__, "heat_no=[{0}]", tpssm11["HEAT_NO"].ToString());

		/* ***** 检查输入参数合法性 ***** */
		if (dev_tech_code == "A")
		{
			/* *****	发送电文开始 ***** */
			lpsz_tc_no = "T8E2R1";/*赋电文号*/

			/*初始化*/
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文T8E2R1初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*拼电文数据*/
			if (epex.SetValue("INT_AOD_INSTRUCTION", "INSTRUCTION_TIME", 0, dateNow) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R1拼接失败");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue("INT_AOD_INSTRUCTION", "INSTRUCTION", 0, text) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R1拼接失败");
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
		else if (dev_tech_code == "B")
		{
			/* *****	发送电文开始 ***** */
			lpsz_tc_no = "T8E2R2";/*赋电文号*/

			/*初始化*/
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文T8E2R2初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*拼电文数据*/
			if (epex.SetValue("INT_BOF_INSTRUCTION", "INSTRUCTION_TIME", 0, dateNow) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R2拼接失败");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue("INT_BOF_INSTRUCTION", "INSTRUCTION", 0, text) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R2拼接失败");
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
		else if (dev_tech_code == "E")
		{
			/* *****	发送电文开始 ***** */
			lpsz_tc_no = "T8E2R3";/*赋电文号*/

			/*初始化*/
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文T8E2R3初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*拼电文数据*/
			if (epex.SetValue("INT_EAF_INSTRUCTION", "INSTRUCTION_TIME", 0, dateNow) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R3拼接失败");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue("INT_EAF_INSTRUCTION", "INSTRUCTION", 0, text) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R3拼接失败");
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
		else if (dev_tech_code == "F")
		{
			/* *****	发送电文开始 ***** */
			lpsz_tc_no = "T8E2R4";/*赋电文号*/

			/*初始化*/
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文T8E2R4初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*拼电文数据*/
			if (epex.SetValue("INT_LF_INSTRUCTION", "INSTRUCTION_TIME", 0, dateNow) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R4拼接失败");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue("INT_LF_INSTRUCTION", "INSTRUCTION", 0, text) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R4拼接失败");
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
		else if (dev_tech_code == "R")
		{
			/* *****	发送电文开始 ***** */
			lpsz_tc_no = "T8E2R5";/*赋电文号*/

			/*初始化*/
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文T8E2R5初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*拼电文数据*/
			if (epex.SetValue("INT_RH_INSTRUCTION", "INSTRUCTION_TIME", 0, dateNow) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R5拼接失败");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue("INT_RH_INSTRUCTION", "INSTRUCTION", 0, text) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R5拼接失败");
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
		else if (dev_tech_code == "C")
		{
			/* *****	发送电文开始 ***** */
			lpsz_tc_no = "T8E2R6";/*赋电文号*/

			/*初始化*/
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文T8E2R6初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*拼电文数据*/
			if (epex.SetValue("INT_CCM_INSTRUCTION", "INSTRUCTION_TIME", 0, dateNow) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R6拼接失败");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue("INT_CCM_INSTRUCTION", "INSTRUCTION", 0, text) < 0)
			{
				sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
				sprintf(s.sysmsg, "电文T8E2R6拼接失败");
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
