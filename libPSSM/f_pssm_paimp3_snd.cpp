/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dclian
Version:    1.0
Date:     2016-4-14
Description:	 发送转炉开吹时刻给铁区。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

#include "tpssms1.h"
#include "xpaimp3.h"


/*<remark>=========================================================
/// <summary>
///  发送停机实绩给铁区。
///<para>1.读取传入的计划号、制造命令号、熔炼号</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：保存下发调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_paimp3_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;
	int fetchRowCount1 = 0;

	CString sqlstr = "";
	int send_flag = 0;

	CString lpsz_tc_no = " ";
	int tele_num = 1;

	CDecimal d_total_tel_num = 0;
	CDecimal v_count = 0;
	CDecimal plan_charge_num = 0;
	CString v_bof_no = " ";
	CString v_sg_sign = " ";
	CString v_ladle_level = " ";
	CString factory_div = " ";//分区标志
	CString oper_flag = " ";//操作区分标志
	CDecimal max_num = 15; //电文循环数

	EPEX epex(&s, conn, 1);

	CTPSSMS1 tpssms1(conn);
	CPAIMP3 xpaimp3(conn);//利用头文件压值
	CDbCommand cmd_inq(conn);
	EIClass inblk;        //调用函数用


	try
	{
		
		lpsz_tc_no = "PAIMP3";/*一区赋电文号*/

		v_count = bcls_rec->Tables[0].Rows.get_Count();
		
	
		Log::Trace("", __FUNCTION__, "转炉开吹信息传入数v_count[{0}]", v_count.ToInt32());

		if (v_count != 0)
		{
			 xpaimp3.MergeFrom( bcls_rec->Tables[0].Rows[0]);
			

			 Log::Trace("", __FUNCTION__, "传入sm_plan_no=[{0}]", xpaimp3.SM_PLAN_NO);
		

			//初始化
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				Log::Trace("", __FUNCTION__, "初始化出错.");
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (epex.SetValue(0, xpaimp3) < 0)
			{
				sprintf(s.msg, "压值=[%s]", epex.GetMsg());
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

		

			//发送电文
			if (epex.SendTele() < 0)
			{
				Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			// 释放
			epex.Uninitialize();
			Log::Trace("", __FUNCTION__, "计划成功= [{0}]", tele_num);

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

