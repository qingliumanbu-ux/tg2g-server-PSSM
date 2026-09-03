/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: yss
日期: 2018-7-12 10:00:00
功能: 根据ED54的配置信息_信息修改。
修改历史：
日期:________；修改人：________; 需求提出人________
变更内容:
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h" 


/*<remark>=========================================================
/// <summary>
///  根据ED54的配置信息_信息修改。
/// <para>
///    功能叙述段落
/// </para>
/// </summary>
/// <param name="bcls_rec">传入块  </param>
/// <param name="bcls_ret">返回块  </param>
/// <returns>返回：是否成功</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(pssmsif_upd)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssmsif_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/****** 定义函数名称 ***** */
	CString FunctionEname = "f_pssmsif_upd";                //定义函数英文名称  
	CString FunctionCname = "根据ED54的配置信息_信息修改。";              //定义函数中文名称
	////LogTrace(1, 1, " **************%s begin*****************", (const char*)FunctionEname);

	//程序用变量
	int doFlag = 0;

	CString sqlstr = "";	//SQL 信息。 
	CString msgstr = "";	//提示信息。
	int update_sum = 0;		//更新记录总数

	try{
		CString table_name(bcls_rec->Tables[0].get_TableName());
		CModel model(table_name);
		CString fieldToUpdate("");	//可编辑的列 col_name1{[,col_name2]...}
		CString item_ename("");		//ED54列名
		CString form_edit_flag("");	//ED54可编辑属性 1可编辑 0不可编辑
		for (size_t i = 0; i < bcls_rec->Tables[1].Rows.get_Count(); i++)
		{
			item_ename = bcls_rec->Tables[1].Rows[i]["item_ename"].ToString().Trim();
			form_edit_flag = bcls_rec->Tables[1].Rows[i]["form_edit_flag"].ToString();
			if ("1" == form_edit_flag && model.GetFields().Contains(item_ename))
			{
				if (fieldToUpdate != "") fieldToUpdate += ",";
				fieldToUpdate += item_ename;
			}
		}
		if (fieldToUpdate == "") throw CApplicationException(-1, "缺少修改列。", s.svc_name);
		for (size_t i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			model.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			if (model.Query()){
				model["REC_REVISOR"] = s.userid;
				model["REC_REVISE_TIME"] = s.datetime;
				model.TrimOrBlank();
				sqlstr = "UPDATE " + table_name;
				update_sum += model.Update(fieldToUpdate);
			}
			else{
				throw CApplicationException(-1, "未找到该记录，无法修改。(是否新增？)", s.svc_name);
			}
		}

		msgstr = msgstr.Format("%d条记录更新成功。", update_sum);
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
