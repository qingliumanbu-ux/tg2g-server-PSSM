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
BM2F_ENTERACE(pssm20t_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm20t_inq(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
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
	CString v_sm_plan_no = "";	//
	int	TotalRecordCount = 0;	//总记录数

	//系统的分页类信息。
	//CPageInfo pageInfo;			//分页信息

	try
	{
		//try
		//{//获取前台DEV控件传入的分页信息
		//	pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		//}
		//catch (CException& cex)
		//{
		//	pageInfo.RecordFrom = 0;
		//	pageInfo.PageSize = 1000;
		//}

		//获取传入参数
		v_sm_plan_no = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"].ToString().Trim();

		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "FACTORY_DIV =[{0}]", v_sm_plan_no);
		////Log::Trace("", __FUNCTION__, "func_id =[{0}]", func_id);

		sqlstr = " select * from tpssm11 a "
			" WHERE a.SM_PLAN_NO = '" + v_sm_plan_no + "' ORDER BY a.heat_no ";
		Db::QueryTable(sqlstr, bcls_ret->Tables[0]);

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