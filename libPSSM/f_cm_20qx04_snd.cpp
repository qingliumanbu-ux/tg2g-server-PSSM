/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:     王佳倩
Version:    1.0
Date:     2022-10-21
Description:	 给智慧质量发送炉次确定电文
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


//程序用头文件
#include "epex.h"


/*<remark>=========================================================
/// <summary>
/// 发送命令接收应答至MMS
/// <para>1.读取传入的厂别区分、制造命令号、应答代码</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：炉次确定调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="ack_code">应答代码         </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_cm_20qx04_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	//int  fetchRowCount = 0;
	CString lpsz_tc_no = " ";
	CString v_pono = " ";			//制造命令号
	CString v_heat_no = "";			//熔炼号
	CString v_st_no = " ";			//出钢记号
	CString v_factory_div = " ";	//厂别区分
	EPEX epex(&s, conn);

	CString sqlstr;

	try
	{
		/* ***** 获取输入参数 ***** */
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();
		v_heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString().Trim();
		v_st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().Trim();

		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "factory_div=[{0}]", v_factory_div);
		Log::Info("", __FUNCTION__, "pono=[{0}]", v_pono);
		Log::Info("", __FUNCTION__, "hrat_no=[{0}]", v_heat_no);
		Log::Info("", __FUNCTION__, "st_no=[{0}]", v_st_no);

		/* *****	发送电文开始 ***** */
		lpsz_tc_no = "20QX04";   /*赋电文号*/

		/*初始化*/
		if (epex.Initialize(lpsz_tc_no) < 0)
		{
			//strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			sprintf(s.sysmsg, "电文20QX04初始化出错！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		/*拼电文数据*/

		if (epex.SetValue("FACTORY_DIV", 0, v_factory_div) < 0)
		{
			Log::Debug("", __FUNCTION__, "SetValue FACTORY_DIV:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (epex.SetValue("PONO", 0, v_pono) < 0)
		{
			Log::Debug("", __FUNCTION__, "SetValue PONO:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (epex.SetValue("HEAT_NO", 0, v_heat_no) < 0)
		{
			Log::Debug("", __FUNCTION__, "SetValue HEAT_NO:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (epex.SetValue("ST_NO", 0, v_st_no) < 0)
		{
			Log::Debug("", __FUNCTION__, "SetValue ST_NO:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}

		/*发送电文*/
		if (epex.SendTele() < 0)
		{
			Log::Trace("", __FUNCTION__, "发送命令应答 = [{0}]", epex.GetMsg());
			//sprintf(s.msg, _RES("GCRSS0000032")/*电文发送失败。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		epex.Uninitialize();
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
