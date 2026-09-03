
/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2010
Author:		yanl
Version:    1.0
Date:		2014-05-30
Description:预计划查询
**************************************************/
#include "stdafx.h" 

//程序用头文件


//#include "tqmto02.h"


BM2F_ENTERACE(pssm01p_plan_inq)

int f_pssm01p_plan_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int		doFlag = 0;
	int		RowCount = 0;
	int		newbk = 1;

	/* 业务变量 */
	CString		v_plan_backlog_code = " ";/* 机组代码 */
	CString		v_order_no = " ";
	CString		v_pono = " ";
	CString		v_roll_plan_no = " ";
	CString		v_his_flag = " ";
	CString		v_prod_date_from = "";
	CString		v_prod_date_to = "";
	CString		function_id = "PSSM01P_PLAN_INQ";/* 预计划主体信息 */
	CString		v_sample_lot_no = "";
	CString		v_plan_status = "";		/* 计划状态，02－新增计划查询；其它－还是一般计划查询 */
	CDecimal	v_lack_wt = 0;  //差欠重量
	CDecimal	v_std_devo_rate = 0;  //标准投料率
	CDecimal	v_prod_wt = 0;   //已产出重量
	CDecimal	v_prod_rate = 0;  //计划完成率
	int			v_cnt = 0;
	CString		v_function_id = " ";/* 工序定制功能号 */
	CString		v_mat_specs = " ";
	CString		v_plan_status_s = " ";
	CString		v_plan_status_e = " ";
	CString		v_whole_backlog_code;

	/* 实体类定义 */
	CModel tpsbwa1("TPSBWA1");
	CModel tom01("TOM01");
	//CTQMTO02 tqmto02(conn);
	CModel tsi0015("TSI0015");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tqmto76_inq(conn);
	CDbCommand cmd_inq3(conn);
	CDbCommand cmd_tmmbw01_inq(conn);

	try
	{
		//获得输入参数
		tpsbwa1["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
		v_order_no = bcls_rec->Tables[0].Rows[0]["ORDER_NO"];


		Log::Trace("", __FUNCTION__, "=== = [{0}][{1}] === "
			, tpsbwa1["PLAN_BACKLOG_CODE"].ToString(), v_order_no);
		Log::Trace("", __FUNCTION__, "=== = [{0}]  ", v_roll_plan_no);

		//获取工序类型代码
		tsi0015["UNIT_CODE"] = tpsbwa1["PLAN_BACKLOG_CODE"];
		tsi0015.Query("UNIT_CODE");

		//如果前台传了FUNCTION_ID，后台直接调用就可以
		if (bcls_rec->Tables[0].Columns.IndexOf("function_id") < 0)
		{
			v_function_id = function_id + "-" + tsi0015["PS_BACKLOG_TYPE_CODE"].ToString();

			//查询当前工序类型是否存在个性显示配置
			sqlstr = "SELECT COUNT(FUNC_ID) FROM TED53 WHERE FUNC_ID = @v_function_id ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_function_id", v_function_id);
			v_cnt = cmd_inq.ExecuteScalar().ToInt16();
			if (v_cnt > 0)
			{
				function_id = v_function_id;
			}

			//设置返回块
			//设置查询结果的功能号
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "function_id");
			bcls_rec->Tables[0].Rows[0]["function_id"] = function_id;
		}
		Log::Trace("", __FUNCTION__, "===function_id = [{0}]  ", bcls_rec->Tables[0].Rows[0]["function_id"].ToString());

		if (bcls_ret->GetBlkNum() < 1)
		{
			//在bcls_rec 中增加一个块，放查询结果
			newbk = bcls_ret->AddBlock();
		}
		bcls_ret->blk_now = 1; //返回到第一块中

		f_edsetcustominfo(bcls_rec, bcls_ret);


		Log::Trace("", __FUNCTION__, "-- get_Count[{0}]--- ", bcls_ret->Tables[0].Columns.get_Count());

		switch (conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库

			default: // 所有数据库适用，通用SQL语句
				sqlstr = "SELECT  TO_CHAR(DECIMAL(A.OUT_MAT_THICK,10,1)) ||'*'|| TO_CHAR(DECIMAL(A.OUT_MAT_WIDTH,10,1))  AS MAT_SPECS,A.*,B.*   ";
				sqlstr = sqlstr + "   FROM TPSBWA1 A,TOM01 B ";
				sqlstr = sqlstr + "  WHERE  PLAN_BACKLOG_CODE = @tpsbwa1.PLAN_BACKLOG_CODE  "
					"    AND  A.PLAN_CLASS = 'B1'   "
				  //"	 AND A.PLAN_FUR_MODE in('2','5')    "
					"    AND  A.ORDER_NO = B.ORDER_NO  ";

			if (v_plan_status.Trim() == "02")
			{
				sqlstr = sqlstr + " AND A.PLAN_STATUS = '02' "
					" AND A.REC_CREATOR = @userid ";
			}
			else if (v_plan_status.Trim() == "04")
			{
				sqlstr = sqlstr + " AND A.PLAN_STATUS = '04' ";
			}
			else if (v_plan_status.Trim() == "05")
			{
				sqlstr = sqlstr + " AND A.PLAN_STATUS >= '05' ";
			}
			else
			{
				sqlstr = sqlstr + " AND A.PLAN_STATUS >= '04' ";
			}

			if (v_order_no.Trim() != "")
			{
				sqlstr = sqlstr + " AND A.ORDER_NO LIKE @v_order_no" + "||'%' ";
			}

			sqlstr = sqlstr + " ORDER BY A.PLAN_STATUS DESC,A.PLAN_EXEC_SEQ_NO ,A.REC_CREATE_TIME";

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "-- sqlstr=[{0}]--- ", sqlstr);
		cmd_inq.Parameters.Set("tpsbwa1.PLAN_BACKLOG_CODE", tpsbwa1["PLAN_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("v_order_no", v_order_no);
		cmd_inq.ExecuteReader();

		//循环从游标中取数据，压回前台
		RowCount = 0;
		while (cmd_inq.Read())
		{
			v_mat_specs = cmd_inq.GetString(1);
			int k = 2;
			k = cmd_inq.Fetch(tpsbwa1, k);
			k = cmd_inq.Fetch(tom01, k);



			//获取标准投料系数
			sqlstr = " SELECT STD_DEVO_RATE  "
				"  FROM TSI0001 "
				" WHERE  WHOLE_BACKLOG_CODE = @tpsbwa1.WHOLE_BACKLOG_CODE "
				;
			Log::Trace("", __FUNCTION__, "-- sqlstr=[{0}]--- ",sqlstr);
			cmd_inq3.SetCommandText(sqlstr);
			cmd_inq3.Parameters.Set("tpsbwa1.WHOLE_BACKLOG_CODE", tpsbwa1["WHOLE_BACKLOG_CODE"].ToString());
			v_std_devo_rate = cmd_inq3.ExecuteScalar();

			// 将结果放入返回块
			CDataRow& row = bcls_ret->Tables[0].Rows.Add(); //新增一行

			//计算投料完成率，利用HL_WT放投料完成率。
			tpsbwa1["HL_WT"] = (tpsbwa1["DEVO_WT"].ToDecimal() / tpsbwa1["APP_DEVO_WT"].ToDecimal()) * 100;
			tpsbwa1["HL_WT"] = tpsbwa1["HL_WT"].ToDecimal().Round(1);
			//计算差欠重量
			v_lack_wt = tpsbwa1["APP_DEVO_WT"].ToDecimal() - tpsbwa1["DEVO_WT"];
			row.Merge(tom01);
			row.Merge(tpsbwa1);

			//row["LACK_WT"] = v_lack_wt;
			//row["STD_DEVO_RATE"] = v_std_devo_rate;
			//if (tpsbwa1["OUT_MAT_WIDTH"].ToDecimal() > 0)
			//{
			//	row["MAT_SPECS"] = v_mat_specs;
			//}
			//else
			//{

			//	row["MAT_SPECS"] = tpsbwa1["OUT_MAT_THICK"].ToDecimal().ToDouble();
			//}

			//v_prod_wt = 0;

			////2014-06-25 yanl add 
			////计算已产出量：按轧制计划号统计材料主档在线及历史表
			//sqlstr = " SELECT NVL(sum(mat_act_wt),0)  "
			//	"   FROM TMMBW01 A,TPSBWA1 B "
			//	"  WHERE B.PLAN_NO_CON  = @tpsbwa1.ROLL_PLAN_NO "
			//	"    AND B.ROLL_PLAN_NO = A.ROLL_PLAN_NO "
			//	;
			////Log::Trace("", __FUNCTION__, "-- sqlstr=[{0}]--- ",sqlstr);
			//cmd_tmmbw01_inq.SetCommandText(sqlstr);
			//cmd_tmmbw01_inq.Parameters.Set("tpsbwa1.ROLL_PLAN_NO", tpsbwa1["ROLL_PLAN_NO"].ToString());
			//v_prod_wt = v_prod_wt + cmd_tmmbw01_inq.ExecuteScalar();

			//sqlstr = " SELECT NVL(sum(mat_act_wt),0)  "
			//	"   FROM TMMBW01 A,HPSBWA1 B "
			//	"  WHERE B.PLAN_NO_CON  = @tpsbwa1.ROLL_PLAN_NO "
			//	"    AND B.ROLL_PLAN_NO = A.ROLL_PLAN_NO "
			//	;
			////Log::Trace("", __FUNCTION__, "-- sqlstr=[{0}]--- ",sqlstr);
			//cmd_tmmbw01_inq.SetCommandText(sqlstr);
			//cmd_tmmbw01_inq.Parameters.Set("tpsbwa1.ROLL_PLAN_NO", tpsbwa1["ROLL_PLAN_NO"].ToString());
			//v_prod_wt = v_prod_wt + cmd_tmmbw01_inq.ExecuteScalar();

			////计算计划（产出）完成率
			//v_prod_rate = (v_prod_wt / tpsbwa1["TOTAL_MAT_WT"].ToDecimal()) * 100;
			//v_prod_rate = v_prod_rate.Round(1);

			//row["PROD_WT"] = v_prod_wt;
			//row["PROD_RATE"] = v_prod_rate;



			RowCount++;

		}//while
		cmd_inq.Close();

		{
			CFormattable arguments[] = { bcls_ret->Tables[0].Rows.get_Count() };// 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004")/*查询到[{0}]条记录。*/, arguments, 1);//格式化字符串
		}
		Log::Trace("", __FUNCTION__, "-- query records.[{0}]--- ", bcls_ret->Tables[0].Rows.get_Count());
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
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

	return (doFlag);
}
