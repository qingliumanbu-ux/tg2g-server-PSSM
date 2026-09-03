/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-02-14
功能: 连铸机号查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
/// 连铸机号代码查询
/// <para > 
/// 1.根据传入的机组代码，查询连铸机号。
/// 2.查询条件：机组代码
/// </para > 
/// <para > 数据库表：TPSSM31(炼钢工序_连铸机类型与设备关系表)      </para > 
/// </summary > 
/// <param name = "v_unit_code" > 机组代码    </param > 
/// <returns > 指定机组代码下的连铸机号。</returns > 
=========================================================== </remark > */

//service入口
BM2F_ENTERACE(pssm04cc_inq)

int f_pssm04cc_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int rowCount = 0;

	//业务用变量
	CString v_factory_div = " ";		//机组代码

	//数据库SQL操作字符串
	CString sqlstr = "";

	//数据库操作类定义
	CDbCommand cmd_inq(conn);

	try
	{
		//获取传入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		////Log::Info("", __FUNCTION__, "v_factory_div = [{0}]", v_factory_div);
		//设置返回列
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"CC_MACH_NO");			//连铸机号
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"CC_DESC");				//连铸机描述

		sqlstr = "SELECT  DISTINCT CC_MACH_NO, CC_DESC  FROM  TPSSMD8  ";
		/*拼接条件语句*/
		if (v_factory_div != "")
		{
			sqlstr += "  WHERE  FACTORY_DIV = @factory_div ";  
		}

		sqlstr += " ORDER BY CC_MACH_NO ASC ";

		////Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr);
		/*给查询SQL赋条件值*/
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		bcls_ret->Tables[0].set_TableName("TPSSMD8");
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	cmd_inq.Close();
	return doFlag;
}