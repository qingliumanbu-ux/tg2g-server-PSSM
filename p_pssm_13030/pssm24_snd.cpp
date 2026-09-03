/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-05-22
功能: 制造命令日备注信息表维护_下发
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm24.h"

int f_pssm2104_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
/*<remark >========================================================= 
/// <summary > 
///制造命令日备注信息表_删除
/// <para > 
/// 1.根据传入的区分代码，下发制造命令日备注信息至1炼钢；
/// </para > 
/// <para > 数据库表：TPSSM24(制造命令日备注表)      </para > 
/// <para > 主调用函数：前台PSSM24画面F12(下达)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm24_snd)


int f_pssm24_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString factory_div = "";
	CString plandate = "";
	/*实体类定义*/
	CTPSSM24 tpssm24(conn);

	try
	{	
		//记录总条数
		//int count = bcls_rec->Tables[0].Rows.get_Count(); 
		//Log::Debug("", __FUNCTION__ ,"count = [{0}]",count);
		plandate = bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString();
		factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString();
		Log::Debug("", __FUNCTION__, "plandate = [{0}] factory_div = [{1}]", plandate, factory_div);
		
		//处理

		CString sql_sm24 = " SELECT * "
						   " FROM TPSSM24 "
						   " WHERE  PLAN_DATE = @plan_date "
						   " AND    FACTORY_DIV = @factory_div"
						   " ORDER BY SEQ_NO ";
		CDbCommand cmd_sm24(sql_sm24, conn);
		cmd_sm24.Parameters.Clear();
		cmd_sm24.Parameters.Set("plan_date", plandate);
		cmd_sm24.Parameters.Set("factory_div", factory_div);
		cmd_sm24.ExecuteQuery(bcls_rec->Tables[0]);
		cmd_sm24.Close();

		//为传入块设置块名
		bcls_rec->Tables[0].set_TableName("WS2104");
		bcls_rec->Tables[1].set_TableName("WS2104_1");
		Log::Debug("", __FUNCTION__ ,"开始调用f_pssm2104_snd ");
		doFlag = f_pssm2104_snd(bcls_rec,bcls_ret,conn);
		if (doFlag < 0)
		{
			Log::Trace("", __FUNCTION__ ,"调用f_pssm2104_snd,返回信息 = [{0}]",s.msg);
			throw CApplicationException(-1,s.msg,s.svc_name);
		}

		strcpy(s.msg,_RES("GCRSS0000031")/*电文发送成功。*/);
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

	return doFlag;
}