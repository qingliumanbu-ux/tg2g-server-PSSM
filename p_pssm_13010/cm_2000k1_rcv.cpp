/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:ZhengQiangQiang
Date:2020-7-1
Version:1.0
Description: 热送炉次信息接收电文
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"
#include "epex.h"

/***** C++ 的业务头文件部分 *****/




//-----------------------------------------------------------------------
//功能描述:		新增热送炉次
//数据库表:     TPSBW01
//表中文名:     棒线热送炉次信息表
//表主键：		HEAT_NO

// service入口
BM2F_ENTERACE_TELE(cm_2000k1_rcv)

int f_cm_2000k1_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int fetchRowCount = 0;
	CString sqlstr = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CString heat_no = "";
	CString stock_oper_order = "";
	CString stock_no_fr = "";
	CString stock_no_to = "";
	CString mat_line_type = "";
	CString ingot_code = "";
	CString lpsz_tc_no = "";

	//定义表结构
	CModel tpsbws1("TPSBWS1");
	CModel tpsbwa1("TPSBWA1");

	/* 数据库操作类定义 */
	EPEX epex(&s, conn);
	//CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn); //与DB 建立连接。



	try
	{

		//-----------------------------------------------------------------
		//获取传入参数
		heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "heat_no[{0}]", heat_no);
		if (heat_no.Trim() == "")
		{
			strncpy(s.msg, "传入参数heat_no不能为空。", sizeof(s.msg) - 1);
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		tpsbws1.Reset();
		tpsbws1.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		tpsbws1["REC_CREATOR"] = s.userid;
		tpsbws1["REC_CREATE_TIME"] = datetime;
		tpsbws1["COMPANY_CODE"] = "S";

		if (tpsbws1.QueryCount("HEAT_NO") > 0)
		{
			//清除S1表数据
			//tpsbws1.Delete();

			sqlstr = CString(" DELETE FROM TPSBWS1 WHERE heat_no=@heat_no ");

			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("heat_no", heat_no);   //设置修改数据项
			cmd_upd.ExecuteNonQuery();
		}

		tpsbwa1["ROLL_PLAN_NO"] = tpsbws1["PLAN_NO_CON"];
		if (tpsbwa1.Query("ROLL_PLAN_NO") == true)
		{
			tpsbws1["PLAN_BACKLOG_CODE"] = tpsbwa1["PLAN_BACKLOG_CODE"];
		}

		//打印参数
		tpsbws1.Print();

		tpsbws1.Insert();

		////-----------------------------------------------------------------
		////电文发送
		////定义电文号
		//lpsz_tc_no = "0070K1";

		////初始化电文
		//if (epex.Initialize(lpsz_tc_no) < 0)
		//{
		//	throw CApplicationException(-1, s.msg, s.svc_name);
		//}
		///*拼电文数据*/
		//if (epex.SetValue(0, tpsbws1) < 0)
		//{
		//	sprintf(s.msg, epex.GetMsg());
		//	sprintf(s.sysmsg, epex.GetMsg());
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//if (epex.SendTele() < 0)
		//{
		//	strcpy(s.msg, _RES("GCRSS0000032")/*电文发送失败。*/);
		//	strcpy(s.sysmsg, "电文发送失败");
		//	Log::Trace("", __FUNCTION__, "电文发送失败 = [{0}]", epex.GetMsg());
		//	throw CApplicationException(-1, s.msg, s.svc_name);
		//}

		////电文释放
		//epex.Uninitialize();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
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
