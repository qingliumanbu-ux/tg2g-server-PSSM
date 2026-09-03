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
BM2F_ENTERACE(pssm35tcf2_inq)

int f_pssm35tcf2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag = "0";
	CString cast_no_show = "";
	CString proc_no = "";    /* 生产处理号 */
	CString online_flag = "";
	int		TotalRecordCount = 0;
	int fetchRowCount = 0;
	int first_srf = 0; //精炼工序的第一个charge_no

	int ll;


	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm35("TPSSM35");



	CDbCommand cmd_tpssm35_inq(conn);


	try
	{
		try
		{//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;
		}


		//--------------------------------
		//获取传入参数
		tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		
		/* ***** 打印输入参数 ***** */

	


			sqlstr = " SELECT  ACTRESULT ,DEV_CODE FROM TMMSM27  WHERE HEAT_NO = @tpssm35.HEAT_NO  "
				" UNION ALL   "
				" SELECT  ACTRESULT,DEV_CODE FROM TMMSM21  WHERE HEAT_NO = @tpssm35.HEAT_NO  "
				" UNION ALL   "
				" SELECT  ACTRESULT,DEV_CODE FROM TMMSM23  WHERE HEAT_NO = @tpssm35.HEAT_NO  "
				" UNION ALL   "
				" SELECT  MOLTIRON_WT,DEV_CODE AS ACTRESULT FROM TMMSM24  WHERE HEAT_NO = @tpssm35.HEAT_NO  "
				" UNION ALL   "
				" SELECT  ACTRESULT,DEV_CODE FROM TMMSM25  WHERE HEAT_NO = @tpssm35.HEAT_NO  "
				" UNION ALL   "
				" SELECT  STEEL_WT,DEV_CODE AS ACTRESULT FROM TMMSM26  WHERE HEAT_NO = @tpssm35.HEAT_NO  ";

			
		
		cmd_tpssm35_inq.Parameters.Set("tpssm35.HEAT_NO", tpssm35["HEAT_NO"].ToString());
		cmd_tpssm35_inq.SetCommandText(sqlstr);
		cmd_tpssm35_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_tpssm35_inq.Close();

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
