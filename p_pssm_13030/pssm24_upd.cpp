/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-05-22
功能: 制造命令日备注信息表维护_修改
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
///制造命令日备注信息表_修改
/// <para > 
/// 1.根据传入的区分代码，修改制造命令日备注信息表记录；
/// </para > 
/// <para > 数据库表：TPSSM24(制造命令日备注信息表)      </para > 
/// <para > 主调用函数：前台PSSM24画面F4(修改)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
BM2F_ENTERACE(pssm24_upd)

int f_pssm24_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值
	CString sqlstr = "";
	///*数据库SQL操作字符串，用于捕获数据库操作异常情况*/  2012-12-21 修改
	//CString sqlstr = "";
	//CString updstr = "";
	//CString factory_div = "";
	//CString plandate;
	////定义变量--取系统当前时间
	//CString datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{	
		/*实体类定义*/  //2012 - 12 - 21 修改
		//CTPSSM24 tpssm24_inf(conn);
		//factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString();
		//plandate = bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString();
		////记录总条数
		//int count = bcls_rec->Tables[0].Rows.get_Count();

		// //循环处理
		//for (int i = 0; i < count ; i++)
		//{
		//	// 取得单行传入信息 
		//	tpssm24_inf.Reset();
		//	tpssm24_inf.MergeFrom(bcls_rec->Tables[0].Rows[i]);
		//	//设置修改者信息
		//	tpssm24_inf.REC_REVISOR = s.userid;
		//	tpssm24_inf.REC_REVISE_TIME = datetimeNow; 
		//	tpssm24_inf.FACTORY_DIV = factory_div;
		//	tpssm24_inf.PLAN_DATE = plandate;

		//	//根据传入信息更新制造命令日备注信息表
		//	sqlstr = "UPDATE TPSSM24"; //用于捕获数据库操作异常情况
		//	updstr = " REC_REVISOR,REC_REVISE_TIME,SEQ_NO,REMARK ";
		//	int con = tpssm24_inf.Update(updstr);
		//	if(con <= 0)   
		//	{
		//		strcpy(s.msg,_RES("PSTBS0000002")/*记录修改失败!*/);
		//		throw CApplicationException(-1, s.msg, log.Location);			}				
		//}
		CModel tpssm24 = CModel("TPSSM24");
		
		//记录总条数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		if (bcls_rec->AtBlkName("UPD") > 0)
		{
			for (int i = 0; i < bcls_rec->Tables["UPD"].Rows.get_Count(); i++)
			{
				tpssm24.Reset();
				tpssm24.MergeFrom(bcls_rec->Tables["UPD"].Rows[i]);
				tpssm24.Update("*", "PLAN_DATE,SEQ_NO");
			}
			////Log::Trace("", "", "修改了{0}条记录", bcls_rec->Tables["UPD"].Rows.get_Count());
		}


		CFormattable arguments[] = {count};
		CMessageFormat::Format(s.msg,_RES("PMOMS0000340")/*共修改了[{0}]条记录*/, arguments, 1);


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str =  sqlstr+"\r\n" + ex.GetMsg();
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