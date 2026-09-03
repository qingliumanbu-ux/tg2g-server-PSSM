/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2016-01-14 19:13:56
Description: 钢水返送信息一览查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/




/* ***** 静态函数申明 ***** */
int f_order_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//质量接口，钢水对换用接口

/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 钢水返送信息一览查询
/// <para>
/// 1.根据factory_div,时间等条件进行出钢计划查询。
///
/// </para>
/// <para>数据库表：TPSSM35钢水返送信息记录表)     </para>
/// <para>主调用函数：前台PSSM35画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm82_snd)

int f_pssm82_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString str = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_where = "";
	CString	prod_date_from = "";
	CString	prod_date_to = "";
	CString	start_time = "";
	CString	confrm_time = "";
	CString colname = " ";
	CString colname2 = " ";
	CString cs_dev_code = "";
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag = "0";
	CString cast_no_show = "";
	CString proc_no = "";    /* 生产处理号 */
	CString online_flag = "";
	int		TotalRecordCount = 0;
	int fetchRowCount = 0;
	int first_srf = 0; //精炼工序的第一个charge_no

	CModel tpssm82("TPSSM82");

	EIClass in_pssm82;//调用履历函数
	in_pssm82.Tables[0].Columns.Add(DT_STRING,"DEV_TECH_CODE");
	in_pssm82.Tables[0].Columns.Add(DT_STRING, "TEXT");
	in_pssm82.Tables[0].Rows.Add();
	int ll;



	CDbCommand cmd_tpssm82_inq(conn);


	try
	{


		//--------------------------------
		//获取传入参数
		tpssm82.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		tpssm82["DATI_MSG_SENT"] = CDateTime::Now().ToString("yyyyyMMddHHmmss");
		str = " SELECT LPAD(PSSM82_ID.nextval, 6, '0') FROM dual   ";
		cmd_tpssm82_inq.SetCommandText(str);
		cmd_tpssm82_inq.ExecuteReader();

		if (cmd_tpssm82_inq.Read())
		{
			tpssm82["SEQ_NO"] = cmd_tpssm82_inq.GetDecimal(1);
		}

		tpssm82.Insert();
		/* ***** 打印输入参数 ***** */
		in_pssm82.Tables[0].Rows[0]["DEV_TECH_CODE"] = tpssm82["DEV_CODE"].ToString();
		in_pssm82.Tables[0].Rows[0]["TEXT"] = tpssm82["MEMO_DETAIL"].ToString();
		int ret = f_order_snd(&in_pssm82, bcls_ret, conn);
		if (ret != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		



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
