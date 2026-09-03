/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   姚世申
Version:    1.0
Date:     2018-04-01 17:13:56
Description:
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/


// service入口
BM2F_ENTERACE(pssm11a_plan_inq)

int f_pssm11a_plan_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";

	CModel tpssm12("TPSSM12");

	CDbCommand cmd_inq(conn);
	try
	{
		//获取传入参数
		tpssm12.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		//tpssm12.TrimOrBlank();

		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "FACTORY_DIV =[{0}]", tpssm12["FACTORY_DIV"].ToString());	//
		////Log::Info("", __FUNCTION__, "SM_PLAN_NO =[{0}]", tpssm12["SM_PLAN_NO"].ToString());	//
		CString st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"];					//
		////Log::Info("", __FUNCTION__, "ST_NO =[{0}]", st_no);						//

#pragma region 查询计划编辑的主体数据

		CString main_table = "PLAN";
		bcls_ret->Tables.Add(main_table);
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = "SELECT TPSSM12.DEV_CODE,TPSSM12.PROC_TIME,TPSSM12.START_TIME,TPSSM12.END_TIME,TPSSM12.AREA_ID,TPSSM12.CHARGE_NO"
				",TPSSM12.FACTORY_DIV,TPSSM12.SM_PLAN_NO"//tpssm12↑
				",TPSSMD1.STATION_NAME"//tpssmd1↑
				",TPSSM11.CURR_WP_NO,TPSSM11.HEAT_NO "//tpssm11↑
				" FROM (TPSSM12 "
				" INNER JOIN TPSSMD1 "
				" ON TPSSM12.FACTORY_DIV = TPSSMD1.FACTORY_DIV "
				" AND TPSSM12.DEV_CODE = TPSSMD1.DEV_CODE "
				" AND TPSSM12.AREA_ID = TPSSMD1.AREA_ID) "//from (tpssm12 inner join tpssmd1)↑
				" INNER JOIN TPSSM11 "
				" ON TPSSM12.SM_PLAN_NO = TPSSM11.SM_PLAN_NO "//inner join tpssm11↑把前面的加上()
				" WHERE 1=1 ";
			if (tpssm12["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr += " AND TPSSM12.FACTORY_DIV LIKE @tpssm12.FACTORY_DIV ||'%' ";
			}
			if (tpssm12["SM_PLAN_NO"].ToString().Trim() != "")
			{
				sqlstr += " AND TPSSM12.SM_PLAN_NO LIKE @tpssm12.SM_PLAN_NO ||'%' ";
			}
			sqlstr += " ORDER BY TPSSM12.AREA_ID ASC,TPSSM12.CHARGE_NO ASC";
			break;
		}
		////Log::Trace("", __FUNCTION__, "main_table [{0}] sqlstr = ↓\r\n{1}", main_table, sqlstr);
		cmd_inq.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[main_table]);//DEV_CODE,PROC_TIME,START_TIME,END_TIME,AREA_ID,CHARGE_NO,FACTORY_DIV,SM_PLAN_NO,STATION_NAME,CURR_WP_NO
		cmd_inq.Close();

#pragma endregion

#pragma region 查询该厂各设备类型的设备号及设备中文名 to lookup下拉框

		//查询该厂各设备类型的设备号及设备中文名 to lookup下拉框
		CString device_types = "DSDPDCSRCC";//设备类型汇总
		for (size_t area_id = 1; area_id < 6; area_id++)
		{
			CString dev_type = device_types.Substring((area_id - 1) * 2, 2);//设备类型
			bcls_ret->Tables.Add(dev_type);//为该设备类型创建对应的表 DEV_CODE STATION_NAME
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				if (tpssm12["FACTORY_DIV"].ToString().Trim() == "")break;
				sqlstr = "select tpssmd1.DEV_CODE,tpssmd1.STATION_NAME,tpssmd3.STD_PREP_TIME,tpssmd3.STD_PROC_TIME "
					" from tpssmd1 left join tpssmd3 on tpssmd1.factory_div = tpssmd3.factory_div and tpssmd1.dev_code = tpssmd3.dev_code and tpssmd3.st_no = @st_no "
					" where tpssmd1.factory_div = @factory_div and tpssmd1.area_id = @area_id "
					" order by tpssmd1.area_id";
				break;
			}
			////Log::Trace("", __FUNCTION__, "dev_type table [{0}] sqlstr = ↓\r\n{1}", dev_type, sqlstr);
			cmd_inq.Parameters.Set("factory_div", tpssm12["FACTORY_DIV"].ToString());
			cmd_inq.Parameters.Set("area_id", area_id);
			cmd_inq.Parameters.Set("st_no", st_no);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[dev_type]);
			cmd_inq.Close();
		}

#pragma endregion

#pragma region transformation 查询结果变换 制程编辑面板显示数据

		//transformation 查询结果变换 根据主体表 每行 area_id 值对应 TRANSFORMATION表 设备类型对应列
		CString ts_table = "TRANSFORMATION";//变换之后存入的表名
		bcls_ret->Tables.Add(ts_table);
		bcls_ret->Tables[ts_table].Clone(bcls_rec->Tables[0]);//从前台获取的结构  功能号：PSSM11A_CONST
		bcls_ret->Tables[ts_table].Rows.Add();//单记录 功能号：PSSM11A_CONST

		//写入一些必需参数    FACTORY_DIV    SM_PLAN_NO    CURR_WP_NO    EDITABLE_COL
		if (!bcls_ret->Tables[ts_table].Columns.Contains("FACTORY_DIV"))
			bcls_ret->Tables[ts_table].Columns.Add(DT_STRING, "FACTORY_DIV");
		bcls_ret->Tables[ts_table].Rows[0]["SM_PLAN_NO"] = tpssm12["FACTORY_DIV"];
		if (!bcls_ret->Tables[ts_table].Columns.Contains("SM_PLAN_NO"))
			bcls_ret->Tables[ts_table].Columns.Add(DT_STRING, "SM_PLAN_NO");
		bcls_ret->Tables[ts_table].Rows[0]["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
		if (!bcls_ret->Tables[ts_table].Columns.Contains("CURR_WP_NO"))
			bcls_ret->Tables[ts_table].Columns.Add(DT_STRING, "CURR_WP_NO");
		CString curr_wp_no = bcls_ret->Tables[main_table].Rows[0]["CURR_WP_NO"];
		bcls_ret->Tables[ts_table].Rows[0]["CURR_WP_NO"] = curr_wp_no;
		if (!bcls_ret->Tables[ts_table].Columns.Contains("EDITABLE_COL"))
			bcls_ret->Tables[ts_table].Columns.Add(DT_STRING, "EDITABLE_COL");//添加"可编辑的列" 例"DCSR1SR2CC" 

		auto transformation = [](EIClass * bcls_ret, CString main_table, CString ts_table, int i, CString dev_type)
		{
			bcls_ret->Tables[ts_table].Rows[0]["DEV_CODE_" + dev_type] = bcls_ret->Tables[main_table].Rows[i]["DEV_CODE"];
			bcls_ret->Tables[ts_table].Rows[0]["STATION_NAME_" + dev_type] = bcls_ret->Tables[main_table].Rows[i]["STATION_NAME"];
			bcls_ret->Tables[ts_table].Rows[0]["PROC_TIME_" + dev_type] = bcls_ret->Tables[main_table].Rows[i]["PROC_TIME"];
			bcls_ret->Tables[ts_table].Rows[0]["START_TIME_" + dev_type] = bcls_ret->Tables[main_table].Rows[i]["START_TIME"];
		};
		CString srnums = "123456789";
		CString editable_col = ""; //"可编辑的列"
		for (int i = 0, j = 0; i < bcls_ret->Tables[main_table].Rows.get_Count(); i++)
		{
			CString fieldname = "DS";
			int area_id = bcls_ret->Tables[main_table].Rows[i]["AREA_ID"].ToDecimal().ToInt32();
			switch (area_id){
			case 1:		fieldname = "DS";	break;
			case 2:		fieldname = "DP";	break;
			case 3:		fieldname = "DC";	break;
			case 4:		fieldname = "SR" + srnums.Substring(j++, 1); 	break;//每次遇到area_id=4 j后自加1
			case 5:		fieldname = "CC";	break;
			}
			if (bcls_ret->Tables[main_table].Rows[i]["CHARGE_NO"].ToDecimal()
				> bcls_ret->Tables[main_table].Rows[i]["CURR_WP_NO"].ToDecimal())
				editable_col += fieldname;//如果 CHARGE_NO > CURR_WP_NO 则可编辑：实际工序还没达到该工序
			transformation(bcls_ret, main_table, ts_table, i, fieldname);
			////Log::Trace("", __FUNCTION__, "[{0}] for i = [{1}] END j = [{2}] fieldname = [{3}]", ts_table, i, j, fieldname);
		}//for end
		////Log::Trace("", __FUNCTION__, "editable_col = {0}",editable_col);
		bcls_ret->Tables[ts_table].Rows[0]["EDITABLE_COL"] = editable_col; //"可编辑的列"
#pragma endregion

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。", arguments, 1);
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
