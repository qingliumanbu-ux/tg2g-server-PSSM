/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-10-12
Version:1.0
Description: 炉次状态查询
Update:
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 炉次状态查询
/// <para>查询run_status。                            </para>
/// <para>数据库表：tpssm11                    </para>
/// <para>主调用函数：PSSM21O画面调用。                </para>
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别区分     </param>
/// <returns>run_status</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_eaf_add)
//-EP_SYSTEM_HEAD_END
int f_pssm18_eaf_add(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CString v_dev_code_e = ""; //DEV_CODE-CHARGE_NO
	CString dev_code_3 = ""; //DEV_CODE
	CString dateNow14 = "";
	CString start_time_a = "";
	CString start_time_a_real = "";
	CDecimal charge_no = 0; //CHARGE编号
	CDecimal count_e = 0;
	CDecimal move_time = 0;
	CDecimal proc_time = 0;

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
	CModel tpssmd6("TPSSMD6");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_upd12(conn);
	CDbCommand cmd(conn);

	try
	{
		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得输入参数
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"];
		//tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"];
		tpssm11.Query("PONO");
		v_dev_code_e = bcls_rec->Tables[0].Rows[0]["DEV_CODE_E"];

		Log::Trace("", __FUNCTION__, "v_dev_code_e =[{0}]", v_dev_code_e);
		//Log::Trace("", __FUNCTION__, "charge_no =[{0}]", charge_no);
		Log::Trace("", __FUNCTION__, "FACTORY_DIV =[{0}]", tpssm11["FACTORY_DIV"].ToString());
		Log::Trace("", __FUNCTION__, "PONO =[{0}]", tpssm11["PONO"].ToString());
		Log::Trace("", __FUNCTION__, "SM_PLAN_NO =[{0}]", tpssm11["SM_PLAN_NO"].ToString());

		if (v_dev_code_e.Trim() == "")
		{
			Log::Trace("", __FUNCTION__, "计划[{0}]添加的电炉设备不能为空", tpssm11["SM_PLAN_NOL2"].ToString());
			CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划[{0}]添加的电炉设备不能为空", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sqlstr = " SELECT CHARGE_NO,DEV_CODE,START_TIME,START_TIME_REAL FROM TPSSM12 WHERE AREA_ID = 3 AND SM_PLAN_NO = @sm_plan_no ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		if (cmd_tpssm11_inq.Read())
		{
			dev_code_3 = cmd_tpssm11_inq.GetString(2);
			charge_no = cmd_tpssm11_inq.GetDecimal(1);
			start_time_a = cmd_tpssm11_inq.GetString(3);
			start_time_a_real = cmd_tpssm11_inq.GetString(4);

			if (start_time_a_real.Trim() != "")
			{
				start_time_a = start_time_a_real;
			}
		}

		cmd_tpssm11_inq.Close();

		if (dev_code_3.Trim().Substring(0, 1) != "A")
		{
			Log::Trace("", __FUNCTION__, "计划[{0}]不在AOD排产，不能添加电炉工序", tpssm11["SM_PLAN_NOL2"].ToString());
			CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划[{0}]不在AOD排产，不能添加电炉工序", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}


		sqlstr = " SELECT COUNT(1) FROM TPSSM12 WHERE AREA_ID = 2 AND DEV_CODE LIKE 'E%' AND SM_PLAN_NO = @sm_plan_no ";
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
		count_e = cmd_tpssm11_inq.ExecuteScalar();
		cmd_tpssm11_inq.Close();

		if (count_e != 0)
		{
			Log::Trace("", __FUNCTION__, "计划[{0}]在AOD前已经存在[{1}]个电炉工序，暂不允许添加", tpssm11["SM_PLAN_NOL2"].ToString(), count_e);
			CFormattable arguments[] = { tpssm11["SM_PLAN_NOL2"].ToString(), count_e }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划[{0}]在AOD前已经存在[{1}]个电炉工序，暂不允许添加", arguments, 2); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//传搁时间
		tpssmd6.Reset();
		tpssmd6["FACTORY_DIV"] = tpssm11["FACTORY_DIV"].ToString();
		tpssmd6["DEV_MOVE_START"] = v_dev_code_e;
		tpssmd6["DEV_MOVE_END"] = dev_code_3;
		if (tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END"))
		{
			move_time = tpssmd6["MOVE_TIME"].ToDecimal();
		}
		else
		{
			move_time = 60;
		}


		//处理时间
		tpssmd3["ST_NO"] = tpssm11["ST_NO"].ToString();
		tpssmd3["SMELT_MODE"] = 0;
		tpssmd3["FACTORY_DIV"] = tpssm11["FACTORY_DIV"].ToString();
		tpssmd3["DEV_CODE"] = v_dev_code_e.Substring(0,1);
		tpssmd3["SMELT_MODE2"] = " ";

		if (tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2"))
		{
			proc_time = tpssmd3["STD_PROC_TIME"].ToDecimal();
		}
		else
		{
			tpssmd3["SMELT_MODE"] = 2;
			tpssmd3["ST_NO"] = "DEFAULTS";
			if (tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2"))
			{
				proc_time = tpssmd3["STD_PROC_TIME"].ToDecimal();
			}
			else
			{
				proc_time = 85;
			}
		}

		//////
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "UPDATE TPSSM12 "
				"	SET	CHARGE_NO = CHARGE_NO*10 "
				" WHERE FACTORY_DIV = @factory_div "
				"	AND SM_PLAN_NO = @sm_plan_no ";
			break;
		}
		cmd_upd12.SetCommandText(sqlstr);
		cmd_upd12.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd12.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
		cmd_upd12.ExecuteNonQuery();
		cmd_upd12.Close();

		tpssm12.Reset();
		tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"].ToString();
		tpssm12["AREA_ID"] = 3;
		tpssm12.Query("SM_PLAN_NO,AREA_ID");

		tpssm12["AREA_ID"] = 2; //区域
		tpssm12["DEV_CODE"] = v_dev_code_e; //设备
		
		tpssm12["END_TIME"] = (CDateTime::Parse(start_time_a).AddMinutes(move_time.ToDouble() * -1)).ToString("yyyyMMddHHmmss");   
		tpssm12["START_TIME"] = (CDateTime::Parse(tpssm12["END_TIME"].ToString()).AddMinutes(proc_time.ToDouble() * -1)).ToString("yyyyMMddHHmmss");
	
		tpssm12["PROC_TIME"] = proc_time;
		tpssm12["CHARGE_NO"] = charge_no * 10 - 1;
		//Log::Info("", __FUNCTION__, " AREA_ID=[{0}] DEV_CODE=[{1}] START_TIME=[{2}] END_TIME=[{3}]",
		//tpssm12["AREA_ID"].ToDecimal(), tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString());

		//1)查询设备配置信息
		tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
		tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
		tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
		sqlstr = "tpssmd1.Query()";
		bool hasd1 = tpssmd1.Query("FACTORY_DIV,AREA_ID,DEV_CODE");
		if (hasd1 == false) //没查询到记录
		{
			CFormattable arguments[] = { tpssmd1["AREA_ID"].ToDecimal().ToString(), tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "区域[{0}]下的设备[{1}]配置信息读取不到，请联系系统维护人员。", arguments, 2); //格式化字符串
			CMessageFormat::Format(s.sysmsg, "设备配置表(TPSSMD1)中没有区域[{0}]、设备[{1}]的配置信息.", arguments, 2);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//有脱硫，主计划的脱硫指示置"1"
		if (tpssmd1["AREA_ID"].ToDecimal() == 1)	tpssm11["DE_SULFUR_FLAG"] = "1";
		else tpssm11["DE_SULFUR_FLAG"] = " ";

		//有预溶液，子计划预溶液指示标记置"1" 20231107 wcy
		if (tpssmd1["AREA_ID"].ToDecimal() == 2 && (tpssmd1["STATION_ID"].ToString().Trim() == "X" || tpssmd1["STATION_ID"].ToString().Trim() == "Y" || tpssmd1["STATION_ID"].ToString().Trim() == "Z"))	tpssm12["PRE_SOLUTION_FLAG"] = "1";
		else tpssm12["PRE_SOLUTION_FLAG"] = " ";

		tpssm12["SUB_CHARGE_NO"] = 0;

		tpssm12["REC_CREATE_TIME"] = dateNow14;
		tpssm12["REC_CREATOR"] = CString(s.userid);
		tpssm12["REC_REVISOR"] = " ";
		tpssm12["REC_REVISE_TIME"] = " ";
		tpssm12["PRE_PROC_NO"] = " ";
		tpssm12["PROC_NO"] = " ";
		tpssm12["LADLE_ARRIVE_TIME"] = " ";
		tpssm12["LADLE_LEAVE_TIME"] = " ";
		tpssm12["START_TIME_REAL"] = " ";
		tpssm12["END_TIME_REAL"] = " ";
		tpssm12["ARRIVE_REAL_TIME"] = " ";
		tpssm12["LEAVE_REAL_TIME"] = " ";
		tpssm12["PRACT_RCV_FLAG"] = " ";
		//tpssm12["PROC_NO"] = " ";
		//tpssm12["PRE_PROC_NO"] = " ";
		tpssm12["TREATMENT_COUNTER"] = 1;
		tpssm12["TIME_1"] = " ";
		tpssm12["TIME_2"] = " ";
		tpssm12["TIME_3"] = " ";
		tpssm12["TIME_4"] = " ";
		tpssm12["TIME_5"] = " ";
		tpssm12["TIME_6"] = " ";
		tpssm12["TIME_7"] = " ";
		tpssm12["TIME_8"] = " ";
		tpssm12["TIME_9"] = " ";
		tpssm12["TIME_10"] = " ";

		//新增
		tpssm12.TrimOrBlank();
		sqlstr = "tpssm12.Insert()";
		tpssm12.Insert();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = " UPDATE TPSSM12 "
				" SET CHARGE_NO = (SELECT RN FROM(SELECT CHARGE_NO, "
				" ROW_NUMBER() OVER(ORDER BY CHARGE_NO ASC) AS RN "
				" FROM TPSSM12 "
				" WHERE FACTORY_DIV = @factory_div AND SM_PLAN_NO = @sm_plan_no ) A WHERE TPSSM12.CHARGE_NO = A.CHARGE_NO) "
				" WHERE FACTORY_DIV = @factory_div AND SM_PLAN_NO = @sm_plan_no ";
			break;
		}
		cmd_upd12.SetCommandText(sqlstr);
		cmd_upd12.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd12.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
		cmd_upd12.ExecuteNonQuery();
		cmd_upd12.Close();

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
	cmd_tpssm11_inq.Close();
	return doFlag;

}
