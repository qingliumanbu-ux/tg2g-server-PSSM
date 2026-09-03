/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-10-12
Version:1.0
Description: 炉次状态查询
Update:
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

// service入口
BM2F_ENTERACE(pssm18_flag_proc)
//-EP_SYSTEM_HEAD_END
int f_pssm18_flag_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CString v_dev_code = ""; //DEV_CODE-CHARGE_NO
	CString dev_code = ""; //DEV_CODE
	CDecimal charge_no = 0; //CHARGE编号

	CModel tpssm11("TPSSM11");

	CDbCommand cmd_tpssm11_inq(conn);

	try
	{
		//获得输入参数
		tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"].ToString().TrimOrBlank();
		tpssm11["FLAG_POS_1"] = bcls_rec->Tables[0].Rows[0]["FLAG_POS_1"].ToString().TrimOrBlank();//高硅
		tpssm11["FLAG_POS_2"] = bcls_rec->Tables[0].Rows[0]["FLAG_POS_2"].ToString().TrimOrBlank();//电炉
		tpssm11["FLAG_POS_3"] = bcls_rec->Tables[0].Rows[0]["FLAG_POS_3"].ToString().TrimOrBlank();//测试
		tpssm11["FLAG_POS_4"] = bcls_rec->Tables[0].Rows[0]["FLAG_POS_4"].ToString().TrimOrBlank();//低p
		tpssm11["FLAG_POS_5"] = bcls_rec->Tables[0].Rows[0]["FLAG_POS_5"].ToString().TrimOrBlank();//进料加工

		Log::Trace("", __FUNCTION__, "SM_PLAN_NO =[{0}]", tpssm11["SM_PLAN_NO"].ToString());
		Log::Trace("", __FUNCTION__, "FLAG_POS_1 =[{0}]", tpssm11["FLAG_POS_1"].ToString());
		Log::Trace("", __FUNCTION__, "FLAG_POS_2 =[{0}]", tpssm11["FLAG_POS_2"].ToString());
		Log::Trace("", __FUNCTION__, "FLAG_POS_3 =[{0}]", tpssm11["FLAG_POS_3"].ToString());
		Log::Trace("", __FUNCTION__, "FLAG_POS_4 =[{0}]", tpssm11["FLAG_POS_4"].ToString());
		Log::Trace("", __FUNCTION__, "FLAG_POS_5 =[{0}]", tpssm11["FLAG_POS_5"].ToString());

		tpssm11.Update("FLAG_POS_1,FLAG_POS_2,FLAG_POS_3,FLAG_POS_4,FLAG_POS_5","SM_PLAN_NO");
		

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
