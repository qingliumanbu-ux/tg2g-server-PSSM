/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2016-01-14 19:13:56
Description: 故障信息一览查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/




/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 故障信息一览查询
/// <para>
/// 1.根据factory_div,时间等条件进行出钢计划查询。
///
/// </para>
/// <para>数据库表：TPSSM18故障信息记录表)     </para>
/// <para>主调用函数：前台PSSM35画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm36_inq)

int f_pssm36_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	CModel tpssm18("TPSSM18");



	CDbCommand cmd_tpssm18_inq(conn);


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
		tpssm18.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		prod_date_from = bcls_rec->Tables[0].Rows[0]["DATE_TIME_F"].ToString().Trim();
		prod_date_to = bcls_rec->Tables[0].Rows[0]["DATE_TIME_T"].ToString().Trim();
		/* ***** 打印输入参数 ***** */

		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", tpssm18["FACTORY_DIV"].ToString());



		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr_count = " SELECT COUNT(1) "
				"   FROM  TPSSM18  A"
				"   WHERE 1  = 1 ";

			sqlstr = " SELECT * "
				"   FROM TPSSM18  A"
				"   WHERE 1  = 1  ";

			if (tpssm18["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND A.FACTORY_DIV	= @tpssm18.FACTORY_DIV ";
			}

			if (tpssm18["AREA_ID"].ToDecimal() != 0)
			{
				sqlstr_temp += " AND A.AREA_ID = @tpssm18.AREA_ID ";
			}



			if (prod_date_from.Trim() != "")
			{
				sqlstr_temp += "AND A.START_TIME >= @prod_date_from  ";

			}
			if (prod_date_to.Trim() != "")
			{
				sqlstr_temp += " AND END_TIME <= @prod_date_to ";
			}
			sqlstr_temp_where += " ORDER BY TO_NUMBER(a.MT_SEQ_NO) desc";//设备检修流水号倒序排列

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_where;
			break;
		}
		cmd_tpssm18_inq.Parameters.Set("tpssm18.FACTORY_DIV", tpssm18["FACTORY_DIV"].ToString());
		cmd_tpssm18_inq.Parameters.Set("tpssm18.AREA_ID", tpssm18["AREA_ID"].ToDecimal());

		cmd_tpssm18_inq.Parameters.Set("prod_date_from", prod_date_from);
		cmd_tpssm18_inq.Parameters.Set("prod_date_to", prod_date_to);

		cmd_tpssm18_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm18_inq.ExecuteScalar().ToInt32();
		//分页获取
		cmd_tpssm18_inq.SetCommandText(sqlstr);
		cmd_tpssm18_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);

		cmd_tpssm18_inq.Close();

		/*设置系统返回参数*/
		{
			//_RES("GCRSS0000004")//查询到[{0}]条记录。
			CFormattable arguments[] = { fetchRowCount }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004"), arguments, 1); //格式化字符串 
		}


		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

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
