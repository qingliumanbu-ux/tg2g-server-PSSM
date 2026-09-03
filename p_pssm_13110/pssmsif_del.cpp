/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: yss
日期: 2018-7-12 10:00:00
功能: 根据ED54的配置信息_信息删除。
修改历史：
日期:________；修改人：________; 需求提出人________
变更内容:
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h" 


/*<remark>=========================================================
/// <summary>
///  根据ED54的配置信息_信息删除。
/// <para>
///    功能叙述段落
/// </para>
/// </summary>
/// <param name="bcls_rec">传入块  </param>
/// <param name="bcls_ret">返回块  </param>
/// <returns>返回：是否成功</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(pssmsif_del)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssmsif_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/****** 定义函数名称 ***** */
	CString FunctionEname = "f_pssmsif_del";                //定义函数英文名称  
	CString FunctionCname = "根据ED54的配置信息_信息删除。";              //定义函数中文名称
	////LogTrace(1, 1, " **************%s begin*****************", (const char*)FunctionEname);

	//程序用变量
	int   doFlag = 0;

	CString sqlstr = "";	//SQL 信息。 
	CString msgstr = "";	//提示信息。
	int delete_sum = 0;		//删除记录总数

	try{
		CString table_name(bcls_rec->Tables[0].get_TableName());
		CModel model(table_name);

		for (size_t i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			model.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			model.TrimOrBlank();
			sqlstr = "delete from " + table_name + " where ?=?";
			delete_sum += model.Delete();
		}

		msgstr += msgstr.Format("%d条记录删除成功。", delete_sum);
		strncpy(s.msg, (const char*)msgstr, sizeof(s.msg) - 1);
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
