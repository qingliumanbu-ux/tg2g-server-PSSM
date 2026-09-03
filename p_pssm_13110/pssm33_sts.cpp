/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   yl
Version:    1.0
Date:     2023-5-20
Description:工序状态跟踪查询（工序状态）
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include <cstring>

// service入口
BM2F_ENTERACE(pssm33_sts)

int f_pssm33_sts(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志
	//程序用变量
	int doFlag = 0;
	CString sqlstr;

	//业务变量
	CDbCommand cmd_inq(conn);
	
	//系统的分页类信息。

	try
	{
		//获得输入参数
		sqlstr = " SELECT * FROM TPSSM33 ";
		Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
