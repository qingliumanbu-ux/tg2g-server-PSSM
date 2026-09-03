/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2023
Author:      郑强强
Version:     1.0
Date:        2023-01-12 13:44:44
Description: 单表通用保存-信融专用后台
**************************************************/

#include "stdafx.h"

BM2F_ENTERACE(pssmd1_save)


int f_pssmd1_save(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;

	CString sqlstr = " ";
	CString table_name = " ";
	CString msgstr = "提示信息:";	//提示信息。
	int proc_sum = 0;				//操作总数

	CDbCommand cmd_sql(conn);
	CDataTable temp_table;
	try
	{
		//获取传入参数
		CString  nowTime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		table_name = bcls_rec->Tables["PARA"].Rows[0]["TABLE_NAME"].ToString();

		Log::Trace("", "", "获取传入参数...");
		Log::Trace("", "", "传入表名：table_name=[{0}]", table_name);
		Log::Trace("", "", "当前时间：nowTime=[{0}]", nowTime);

		CModel model = CModel(table_name);

		/************************新增*******************************************/
		if (bcls_rec->Tables.Contains("ADD"))
		{
			Log::Trace("", "", "新增开始,count=[{0}]", bcls_rec->Tables["ADD"].Rows.get_Count());
			for (int i = 0; i < bcls_rec->Tables["ADD"].Rows.get_Count(); i++)
			{
				model.Reset();
				model.MergeFrom(bcls_rec->Tables["ADD"].Rows[i]);
				model["REC_CREATOR"] = s.userid;
				model["REC_CREATE_TIME"] = s.datetime;
				model.TrimOrBlank();

				if (model.Query())
				{
					msgstr += msgstr.Format("第%d条记录已存在，无法新增。", i + 1);
					continue;
				}

			
				sqlstr = "INSERT INTO " + table_name;
				model.Insert();

			}

			
		}

		/************************修改*******************************************/
		if (bcls_rec->Tables.Contains("UPD"))
		{
			Log::Trace("", "", "修改开始,count=[{0}]", bcls_rec->Tables["UPD"].Rows.get_Count());
			for (int i = 0; i < bcls_rec->Tables["UPD"].Rows.get_Count(); i++)
			{
				model.Reset();
				model.MergeFrom(bcls_rec->Tables["UPD"].Rows[i]);
				model.TrimOrBlank();
				model.Print();
				if (!model.Query()){
					////Log::Debug("", __FUNCTION__, "未找到第{0}条记录，无法修改。(是否新增？)", i + 1);
					msgstr += msgstr.Format("未找到第%d条记录，无法删除。", i + 1);
					continue;
				}
				else
				{
					model.Delete();

					model.MergeFrom(bcls_rec->Tables["UPD"].Rows[i]);
					model["REC_REVISOR"] = s.userid;
					model["REC_REVISE_TIME"] = s.datetime;
					model.TrimOrBlank();
					model.Insert();
				}

				model.Print();


			}
		}


		/************************删除*******************************************/
		if (bcls_rec->Tables.Contains("DEL"))
		{
			Log::Trace("", "", "删除开始,count=[{0}]", bcls_rec->Tables["DEL"].Rows.get_Count());
			for (int i = 0; i < bcls_rec->Tables["DEL"].Rows.get_Count(); i++)
			{
				model.Reset();
				model.MergeFrom(bcls_rec->Tables["DEL"].Rows[i]);

				//if (model.QueryCount(condition) != 1){
				if (!model.Query()){
					////Log::Debug("", __FUNCTION__, "未找到第{0}条记录，无法删除。", i + 1);
					msgstr += msgstr.Format("未找到第%d条记录，无法删除。", i + 1);
					continue;
				}
				sqlstr = "DELETE FROM " + table_name;
				proc_sum += model.Delete();
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


