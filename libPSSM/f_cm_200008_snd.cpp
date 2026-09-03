/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-27
Description:	 发送钢种变更信息至MMS
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"


/*<remark>=========================================================
/// <summary>
/// 发送钢种变更信息至MMS
/// <para>1.读取传入的厂别区分、原制造命令号、新制造命令号</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：由钢种变更调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono_old">原制造命令号          </param>
/// <param name="pono_new">新制造命令号          </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_cm_200008_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	CString lpsz_tc_no = " ";

	CString sqlstr = "";
	CString v_factory_div = " "; //厂别区分
	CString v_pono_old = "";//老制造命令号
	CString v_pono_new = "";//新制造命令号

	EPEX epex(&s, conn);


	try
	{


		/*初始化表结构变量*/
		v_factory_div = bcls_rec->Tables["X200008"].Rows[0]["FACTORY_DIV"].ToString();
		v_pono_old = bcls_rec->Tables["X200008"].Rows[0]["PONO_OLD"].ToString();
		v_pono_new = bcls_rec->Tables["X200008"].Rows[0]["PONO_NEW"].ToString();

		/* ***** 打印输入参数 ***** */
		//////Log::Info("", __FUNCTION__, "factory_div=[{0}]", x200008.FACTORY_DIV);
		////Log::Info("", __FUNCTION__, "pono_old=[{0}]", v_pono_old);
		////Log::Info("", __FUNCTION__, "pono_new=[{0}]", v_pono_new);

		/* *****	发送电文开始 ***** */
		lpsz_tc_no = "200008";// "200008";/*赋电文号*/

		/*初始化*/
		if (epex.Initialize(lpsz_tc_no) < 0)
		{
			strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			sprintf(s.sysmsg, "电文200008初始化出错！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		/*拼电文数据*/
		if (epex.SetValue("FACTORY_DIV", 0, v_factory_div) < 0)
		{
			////Log::Debug("", __FUNCTION__, "SetValue FACTORY_DIV:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (epex.SetValue("PONO_OLD", 0, v_pono_old) < 0)
		{
			////Log::Debug("", __FUNCTION__, "SetValue PONO_OLD:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (epex.SetValue("PONO_NEW", 0, v_pono_new) < 0)
		{
			////Log::Debug("", __FUNCTION__, "SetValue PONO_NEW:{0}", epex.GetMsg());
			sprintf(s.msg, epex.GetMsg());
			sprintf(s.sysmsg, epex.GetMsg());
			throw CApplicationException(-1, s.msg, log.Location);
		}

		/*发送电文*/
		if (epex.SendTele() < 0)
		{
			////Log::Trace("", __FUNCTION__, "发送钢种变更电文 = [%s]", (const char*)epex.GetMsg());
			sprintf(s.msg, _RES("GCRSS0000032")/*电文发送失败。*/);
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
