/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: yss
日期: 2018-7-12 10:00:00
功能: 炼钢计划单表查询。
修改历史：
日期:________；修改人：________; 需求提出人________
变更内容:
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h"


/// <summary>
/// <para>
///		炼钢计划单表查询。
/// </para>
/// </summary>
/// <param name="bcls_rec">传入块  </param>
/// <param name="bcls_ret">返回块  </param>
/// <returns>返回：是否成功</returns>

// service入口
BM2F_ENTERACE(pssmsif_inq)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssmsif_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";		//SQL 查询语句
	CString sqlstr_count = "";	//SQL 查询总记录数语句
	CString sqlstr_temp = "";	//SQL 临时变量
	CString sqlstr_order = "";	//SQL 排序
	CString table_name = "";	//表名
	CString func_id = "";		//功能号
	CString field_name = "";	//字段名
	CString msgstr = "";		//提示信息。
	int	TotalRecordCount = 0;	//总记录数

	//系统的分页类信息。
	CPageInfo pageInfo;			//分页信息

	try
	{
		try
		{//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& cex)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;
		}

		//获取传入参数
		table_name = bcls_rec->Tables["PARA"].Rows[0]["TABLE_NAME"].ToString().Trim();
		func_id = bcls_rec->Tables["PARA"].Rows[0]["FUNC_ID"].ToString().Trim();
		/* ***** 打印输入参数 ***** */
		////Log::Trace("", __FUNCTION__, "table_name =[{0}]", table_name);
		////Log::Trace("", __FUNCTION__, "func_id =[{0}]", func_id);

		CModel model(table_name);
		model.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		model.TrimOrBlank();

		CDbCommand cmd_inq(conn);
		sqlstr = " SELECT ITEM_ENAME,ITEM_KEY_FLAG FROM TED54 WHERE FUNC_ID = @FUNC_ID ORDER BY SEQ_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FUNC_ID", func_id);
		////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			if ("1" == cmd_inq.GetString(2))
				sqlstr_order += ("" == sqlstr_order ? " ORDER BY " : ",") + cmd_inq.GetString(1) + " ASC ";
		}
		cmd_inq.Parameters.Clear();
		cmd_inq.Close();

		sqlstr_count = " SELECT COUNT(1) FROM " + table_name;
		sqlstr = " SELECT * FROM " + table_name;

		for (size_t i = 0; i < bcls_rec->Tables[0].Columns.get_Count(); i++)
		{
			field_name = bcls_rec->Tables[0].Columns[i].get_ColumnName();

			if (model.GetFields().Contains(field_name) && "" != model[field_name].ToString().Trim())
			{
				//////Log::Trace("", __FUNCTION__, "{0}[{1}] =[{2}]", table_name, field_name, model[field_name].ToString().Trim());
				sqlstr_temp += "" == sqlstr_temp ? " WHERE " : " AND ";
				sqlstr_temp += field_name + " LIKE @" + field_name + " ||'%' ";
				cmd_inq.Parameters.Set(field_name, model[field_name].ToString().Trim());
			}
		}
		sqlstr += sqlstr_temp;
		sqlstr_count += sqlstr_temp;
		sqlstr += sqlstr_order;

		////Log::Trace("", __FUNCTION__, "sqlstr_count =[{0}]", sqlstr_count);
		cmd_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_inq.ExecuteScalar().ToInt32();
		if (pageInfo.RecordFrom >= TotalRecordCount)pageInfo.RecordFrom = 0;

		////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);

		cmd_inq.Close();

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

		msgstr += msgstr.Format("查询到%d条记录。", TotalRecordCount);
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
