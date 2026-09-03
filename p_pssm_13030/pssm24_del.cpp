/*************************************************

版权: Baosight Software LTD.co Copyright (c) 2012

作者: 涂献计

日期: 2012-05-22

功能: 制造命令日备注信息表维护_删除

修改历史:

日期:________;修改人:________; 需求提出人:________

变更内容:



**************************************************/



#include "stdafx.h"




/*<remark >========================================================= 

/// <summary > 

///制造命令日备注信息表_删除

/// <para > 

/// 1.根据传入的区分代码，删除制造命令日备注信息表记录；

/// </para > 

/// <para > 数据库表：TPSSM24(制造命令日备注表)      </para > 

/// <para > 主调用函数：前台PSSM24画面F5(删除)调用。   </para > 

/// </summary > 

/// <returns > 成功：0</returns > 

/// <returns > 失败：-1</returns > 
=========================================================== </remark > */

BM2F_ENTERACE(pssm24_del)



int f_pssm24_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 

{

	/*打印程序起止LOG*/

	CTracer log(__FUNCTION__);



	/*程序内部变量*/	

	int doFlag = 0;	//返回值



	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/

	CString sqlstr = "";

	CString factory_div = "";

	CString plandate = "";

	try

	{	

		/*实体类定义*/

		//CTPSSM24 tpssm24_inf(conn);	

		////记录总条数

		//factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString();

		//plandate= bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString();

		//int count = bcls_rec->Tables[0].Rows.get_Count(); 

		////循环处理

		//for (int i = 0; i < count ; i++)

		//{

		//	//取得单行传入信息 

		//	tpssm24_inf.Reset();

		//	tpssm24_inf.MergeFrom(bcls_rec->Tables[0].Rows[i]);

		//	//根据传入信息删除制造命令日备注信息

		//	tpssm24_inf.FACTORY_DIV = factory_div;

		//	tpssm24_inf.PLAN_DATE = plandate;

		//	sqlstr = "DELETE TPSSM24";

		//	tpssm24_inf.Delete();	

		//}
		CModel tpssm24 = CModel("TPSSM24");
		if (bcls_rec->AtBlkName("DEL") > 0)
		{
			for (int i = 0; i < bcls_rec->Tables["DEL"].Rows.get_Count(); i++)
			{
				tpssm24.Reset();
				tpssm24.MergeFrom(bcls_rec->Tables["DEL"].Rows[i]);
				tpssm24.Delete();
			}
			////Log::Trace("", "", "删除了{0}条记录", bcls_rec->Tables["DEL"].Rows.get_Count());
		}
		int count = bcls_rec->Tables[0].Rows.get_Count();

		CFormattable arguments[] = {count};

		CMessageFormat::Format(s.msg,_RES("PMOMS0000341")/*共删除了[{0}]条记录*/, arguments, 1);	



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