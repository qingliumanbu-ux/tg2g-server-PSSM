/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: yss
日期: 2018-8-22 14:00:00
功能: 炼钢计划单表维护。
修改历史：
日期:________；修改人：________; 需求提出人________
变更内容:
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h" 


/// <summary>
/// <para>
///		炼钢计划单表维护。
/// </para>
/// </summary>
/// <param name="bcls_rec">传入块  </param>
/// <param name="bcls_ret">返回块  </param>
/// <returns>返回：是否成功</returns>

// service入口
BM2F_ENTERACE(pssmdk_save)
//-EP_SYSTEM_HEAD_END   
int f_pssmdk_save(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "SQL 语句:";	//SQL 语句。 
	CString msgstr = "提示信息:";	//提示信息。
	CString table_name = "表名";	//表名
	CString proc_div = "操作类型";	//操作类型
	CString func_id = "功能号";		//功能号
	CString condition = "";			//主键
	int proc_sum = 0;				//操作总数

	CModel tpssmdk("TPSSMDK");

	try{
		/*if (!bcls_rec->Tables.Contains("PARA") || bcls_rec->Tables["PARA"].Rows.get_Count() < 1)	throw CApplicationException(-1, "缺少参数。", s.svc_name);
		if (!bcls_rec->Tables["PARA"].Columns.Contains("TABLE_NAME")) throw CApplicationException(-1, "缺少表名。", s.svc_name);
		if (!bcls_rec->Tables["PARA"].Columns.Contains("PROC_DIV")) throw CApplicationException(-1, "缺少操作类型。", s.svc_name);


		table_name = bcls_rec->Tables["PARA"].Rows[0]["TABLE_NAME"].ToString();
		proc_div = bcls_rec->Tables["PARA"].Rows[0]["PROC_DIV"].ToString();

		Log::Trace("", __FUNCTION__, "table_name=[{0}]", table_name);
		Log::Trace("", __FUNCTION__, "proc_div=[{0}]", proc_div);*/


		//CDbCommand cmd_inq(conn);
		//sqlstr = " SELECT ITEM_ENAME,ITEM_KEY_FLAG FROM TED54 WHERE FUNC_ID = @FUNC_ID ";
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("FUNC_ID", func_id);
		//////Log::Debug("", __FUNCTION__, "sqlstr = " + sqlstr);
		//cmd_inq.ExecuteReader();
		//while (cmd_inq.Read())
		//{
		//	//ITEM_KEY_FLAG = 1是主键
		//	if ("1" == cmd_inq.GetString(2)) condition += ("" == condition ? " " : ",") + cmd_inq.GetString(1);
		//}
		//cmd_inq.Close();
		//////Log::Debug("", __FUNCTION__, "QueryCount:condition = " + condition);

		if (bcls_rec->Tables.Contains("TPSSMDK_ADD"))
		{
			for (int i = 0; i < bcls_rec->Tables["TPSSMDK_ADD"].Rows.get_Count(); i++)
			{
				tpssmdk.Reset();
				tpssmdk.MergeFrom(bcls_rec->Tables["TPSSMDK_ADD"].Rows[i]);
				tpssmdk["REC_CREATOR"] = s.userid;
				tpssmdk["REC_CREATE_TIME"] = s.datetime;
				tpssmdk.TrimOrBlank();
				tpssmdk.Print();
				//if (tpssmdk.QueryCount(condition) == 1){
				if (tpssmdk.Query()){
					////Log::Debug("", __FUNCTION__, "第{0}条记录已存在，无法新增。(是否修改？)", i + 1);
					msgstr += msgstr.Format("第%d条记录已存在，无法新增。", i + 1);
					continue;
				}
				sqlstr = "INSERT INTO TPSSMDK" ;
				proc_sum += tpssmdk.Insert();
			}
		}
		if (bcls_rec->Tables.Contains("TPSSMDK_MODIFY"))
		{
			for (int i = 0; i < bcls_rec->Tables["TPSSMDK_MODIFY"].Rows.get_Count(); i++)
			{
				tpssmdk.Reset();
				tpssmdk.MergeFrom(bcls_rec->Tables["TPSSMDK_MODIFY"].Rows[i]);
				tpssmdk.TrimOrBlank();
				tpssmdk.Print();
				if (!tpssmdk.Query()){
					////Log::Debug("", __FUNCTION__, "未找到第{0}条记录，无法修改。(是否新增？)", i + 1);
					msgstr += msgstr.Format("未找到第%d条记录，无法删除。", i + 1);
					continue;
				}
				tpssmdk.MergeFrom(bcls_rec->Tables["TPSSMDK_MODIFY"].Rows[i]);
				tpssmdk["REC_REVISOR"] = s.userid;
				tpssmdk["REC_REVISE_TIME"] = s.datetime;
				tpssmdk.TrimOrBlank();
				tpssmdk.Print();
				//if (tpssmdk.QueryCount(condition) != 1){

				//////Log::Debug("", __FUNCTION__, "tpssmdk[REC_REVISOR]{0}", tpssmdk["REC_REVISOR"]);
				sqlstr = "UPDATE  TPSSMDK SET ";
				proc_sum += tpssmdk.Update("*");
			}
		}
		if (bcls_rec->Tables.Contains("TPSSMDK_DELETE"))
		{
			for (int i = 0; i < bcls_rec->Tables["TPSSMDK_DELETE"].Rows.get_Count(); i++)
			{
				tpssmdk.Reset();
				tpssmdk.MergeFrom(bcls_rec->Tables["TPSSMDK_DELETE"].Rows[i]);
				tpssmdk.Print();
				//if (tpssmdk.QueryCount(condition) != 1){
				if (!tpssmdk.Query()){
					////Log::Debug("", __FUNCTION__, "未找到第{0}条记录，无法删除。", i + 1);
					msgstr += msgstr.Format("未找到第%d条记录，无法删除。", i + 1);
					continue;
				}
				sqlstr = "DELETE FROM TPSSMDK" ;
				proc_sum += tpssmdk.Delete();
			}
		}

		msgstr += msgstr.Format("%d条记录操作成功。", proc_sum);
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
		////Log::Warn("", __FUNCTION__, "CDbException: {0}", s.msg);
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
		////Log::Error("", __FUNCTION__, "CApplicationException: {0}", ex.GetMsg());
	}
	catch (CException& ex)
	{
		strncpy(s.sysmsg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
		////Log::Fatal("", __FUNCTION__, "CException: {0}", ex.GetMsg());
	}
	return doFlag;
}
