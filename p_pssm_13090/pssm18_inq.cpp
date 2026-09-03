/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:1.0
Description: 出钢计划甘特图查询
Update：   2014-11-25  xuwen  表结构优化
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tpssm11.h"
#include "tpssm03.h"
#include "tpssm12.h"
//#include "tpssm26.h"
#include "tpssmd1.h"
#include "tpssmd3.h"
#include "tpssmd6.h"
#include "tpssm10.h"
#include "tpssm18.h"
#include "tpssmc2.h"
#include "tpssmc1.h"


void WriteXmlFile(string DataSetName, string TableName, string strnamespace, int blk, string filename);


/*<remark>=========================================================
/// <summary>
/// 甘特图计划信息查询
/// <para>主要数据：主计划及工序计划，设备信息及状态，浇铸信息,传搁时间信息。</para>
/// <para>数据库表：tpssm11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：PSSM18画面查询(甘特图)调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>出钢计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_inq)

int f_pssm18_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量

	int doFlag = 0;
	int blkseq=0;
	int fetchRowCount=0;

	CString v_sm_unit_no = "";	//炼钢单元号
	CString base_time = "";
	CString cast_lot_no_and_div="";
	CDecimal  pour_time1=0;               /* 连铸机浇注时间*/
	CDecimal  pour_time=0;               /* 连铸机浇注时间*/
	CString dev_move_start="";
	CString dev_move_end="";
	CString dev_qlty_req_code="";
	CString dev_code="";

	CTPSSM10 tpssm10(conn);
	CTPSSM11 tpssm11(conn);
	CTPSSM03 tpssm03(conn);
	CTPSSM12 tpssm12(conn);
	CTPSSMD1 tpssmd1(conn);
	CTPSSMD5 tpssmd3(conn);
	CTPSSMD6 tpssmd6(conn);
	CTPSSM18 tpssm18(conn);
	CTPSSMC2 tpssmc2(conn);
	CTPSSMC1 tpssmc1(conn);
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm18_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssmd4_inq(conn);
	CDbCommand cmd_tpssmd6_inq(conn);
	CDbCommand cmd_tpssmc1_inq(conn);
	CDbCommand cmd_tpssmc2_inq(conn);
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{
		base_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		//---------------------------------------------------
		//设置返回块参数
		//第一块，主计划
		blkseq = 1;
		bcls_ret->Tables[blkseq-1].set_TableName("PLAN");  //计划块
		bcls_ret->Tables[blkseq-1].Columns.Add(tpssm11);
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "CC_MARK");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "BOF_NO");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "CCM_NO");		//为显示颜色用
		//bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "cc_mach_no");	//显示注机号用
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "REFINE_ROUTE_CODE"); //精炼路径
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "BASE_TIME");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "TD_CHG_FLG");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "GUIGE2");     //规格?
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "SG_SIGN");

		//第二块，子计划
		blkseq ++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq-1].set_TableName("SUB");  //子计划块
		bcls_ret->Tables[blkseq-1].Columns.Add(tpssm12); //增加一个12表结构体

		//第三块，设备代码
		blkseq ++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq-1].set_TableName("DEV");  //设备信息
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "DEV_CODE");  //设备代码
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "STATION_NAME"); //设备名称
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "DEV_TYPE");  //设备类型:细分同一类型设备的工艺区分
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "CLASS_ID");  //??????
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "NUMB");      //甘特图显示顺序

		//第四块，制造命令相关
		blkseq ++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq-1].set_TableName("PONO");  //制造命令相关
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "CC_MACH_NO");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "REFINE_ROUTE_CODE");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "SMELT_MODE");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "RESTRAND_NO");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "CC_SEQ");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "GUIGE");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "POUR_TIME");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "PONO_PLAN_DATE");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "SG_SIGN");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "CAST_LOT_NO");
		//bcls_ret->Tables[2].Columns.Add(DT_STRING, "pono");

		//第五块，设备维修
		blkseq ++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq-1].set_TableName("STOP");  //设备维修
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "START_TIME");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "END_TIME");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "DEV_STATUS_REMARK");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "STOP_FLAG");

		//增加第六块 根据钢种提供的各设备处理时间
		blkseq ++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq-1].set_TableName("PROC_TIME");  //
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "PROC_TIME");

		//增加第七块 传搁时间 （考虑在画面载入时传入）
		blkseq ++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq-1].set_TableName("MOVE_TIME");  //
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "START_DEV");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "END_DEV");
		bcls_ret->Tables[blkseq-1].Columns.Add(DT_STRING, "MOVE_TIME");


		//----------------------------------------------------------
		//获得输入参数
		//v_sm_unit_no = bcls_rec->Tables[0].Rows[0]["SM_UNIT_NO"].ToString().Trim();
		//Log::Trace("", __FUNCTION__, "SM_UNIT_NO=[{0}]", v_sm_unit_no);


		//----------------------------------------------------------
		//查询当前计划中最早执行的时间
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT MIN(START_TIME_REAL), MIN(START_TIME) "
				"   FROM TPSSM12 "
				);
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.ExecuteReader();
		if(cmd_tpssm12_inq.Read())
		{
			tpssm12.START_TIME_REAL = cmd_tpssm12_inq.GetString(1);
			tpssm12.START_TIME      = cmd_tpssm12_inq.GetString(2);
		}
		else
		{
		    tpssm12.START_TIME_REAL = " ";
			tpssm12.START_TIME      = " ";  
		}
		cmd_tpssm12_inq.Close();



		//----------------------------------------------------------
		//查询数据送到前台
		Log::Trace("", __FUNCTION__,"主计划数据查询开始");
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT a.*, b.* FROM TPSSM11 a left join TPSSM10 b"
				"    ON(a.pono = b.pono) "
				"  ORDER BY a.CAST_NO ASC, a.CAST_DIV_NO ASC "
				);
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.ExecuteReader();
		while(cmd_tpssm11_inq.Read())
		{
			//plan_num ++;
			int k = cmd_tpssm11_inq.Fetch(tpssm11, 1);
			Log::Trace("", __FUNCTION__, "k = [{0}]", k);
			k = cmd_tpssm11_inq.Fetch(tpssm10, k);
			Log::Trace("", __FUNCTION__, "k = [{0}]", k);
			tpssm11.TrimOrBlank();
			tpssm10.TrimOrBlank();

			//精炼工序的第一个charge_no 必定不为0
			Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm10.PONO);

			//查询子工序表, 将内容写入相应的列中
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM TPSSM12 "
					"  WHERE SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					"    AND SUB_CHARGE_NO = 0 "  //0-主工序. 对甘特图只读取主工序的.
					"  ORDER BY CHARGE_NO ASC "
					);
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO",tpssm11.SM_PLAN_NO);
			cmd_tpssm12_inq.ExecuteReader();
			while(cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
				tpssm12.TrimOrBlank();

				//如果实绩结束时间有但是实绩开始时间没有，把计划开始时间传给实绩开始时间
				//ins by 180502 on 11-03-22
				if(tpssm12.END_TIME_REAL.Compare(" ") != 0 && tpssm12.START_TIME_REAL.Compare(" ") == 0)
				{
					tpssm12.START_TIME_REAL = tpssm12.START_TIME;
				}

				//回炉的特殊处理
				if (tpssm11.STEEL_RETURN_CODE.Trim() == "1")
				{
					if (tpssm12.START_TIME_REAL.Trim() == "" && tpssm12.END_TIME_REAL.Trim() == "")
					{
						tpssm12.START_TIME = "19801124080000";
						tpssm12.END_TIME = "19801124081000";
					}
				}
				CDataRow & row_sub = bcls_ret->Tables["SUB"].Rows.Add();
				row_sub.Merge(tpssm12);

				//脱P转炉的设备代码转换
				if (tpssm12.AREA_ID == 2)
				{
					if (tpssm12.DEV_CODE == "P1") row_sub["DEV_CODE"] = "B1";
					if (tpssm12.DEV_CODE == "P2") row_sub["DEV_CODE"] = "B2";
					if (tpssm12.DEV_CODE == "P3") row_sub["DEV_CODE"] = "B3";
				}
				
			}
			cmd_tpssm12_inq.Close();


			//给第一块赋值
			CDataRow& row_plan = bcls_ret->Tables["PLAN"].Rows.Add();   //新增空行
			row_plan["BASE_TIME"] =  base_time;
			row_plan.Merge(tpssm11);

			//特定字段转换
			row_plan["SMELT_MODE"] = (tpssm11.SMELT_MODE == "4")? "2" : "1"; //4-脱P
			row_plan["CC_MARK"] = (tpssm11.RESTRAND_FLAG.Trim() == "T" ? "1" : " ");//????????????????????????
			row_plan["REFINE_ROUTE_CODE"] = tpssm11.SR_ROUTE;
			row_plan["TD_CHG_FLG"] = tpssm11.TD_CHG_FLAG;
			row_plan["SG_SIGN"] = tpssm10.SG_SIGN ;
			//板坯规格信息
			row_plan["GUIGE2"] = tpssm10.SLAB_THICK.ToString() + "*" + tpssm10.SLAB_WIDTH.ToString() + "*" + tpssm10.SLAB_LEN.ToString();


		}
		cmd_tpssm11_inq.Close();


		//----------------------------------------------------------
		//第三块，设备代码
		Log::Trace("", __FUNCTION__, "设备代码查询");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * FROM TPSSMD1 "
				"  WHERE AREA_ID  > 2 "  //从转炉脱C
				" ORDER BY AREA_ID, DEV_CODE "
				);
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.ExecuteReader();
		fetchRowCount = 0;
		while(cmd_tpssmd1_inq.Read())
		{
			cmd_tpssmd1_inq.Fetch(tpssmd1);
			fetchRowCount ++;
			tpssmd1.TrimOrBlank();
			CDataRow & row_dev = bcls_ret->Tables["DEV"].Rows.Add();

			if(tpssmd1.AREA_ID == 3)
			{
				row_dev["DEV_TYPE"] = tpssmd1.DEV_ID;
				row_dev["CLASS_ID"] =  "1";
				row_dev["DEV_CODE"] =     tpssmd1.DEV_CODE;
				row_dev["STATION_NAME"] = tpssmd1.DEV_NAME;
				row_dev["NUMB"] =   fetchRowCount ;
			}
			else if(tpssmd1.AREA_ID == 4)
			{
				//			Log::Trace("", __FUNCTION__, "dev_code=[{0}]",tpssmd1.DEV_CODE);
				row_dev["DEV_TYPE"] =  tpssmd1.DEV_ID;//RH
				row_dev["CLASS_ID"] =  "3";
				row_dev["DEV_CODE"] =  tpssmd1.DEV_CODE;
				row_dev["STATION_NAME"] =  tpssmd1.DEV_NAME;
				row_dev["NUMB"] = fetchRowCount ;
			}
			else if(tpssmd1.AREA_ID==5)
			{
				row_dev["DEV_TYPE"] =  tpssmd1.DEV_ID;//连铸机
				row_dev["CLASS_ID"] =  "5";
				row_dev["NUMB"] = fetchRowCount ;
				row_dev["DEV_CODE"] =  tpssmd1.DEV_CODE;
				row_dev["STATION_NAME"] = tpssmd1.DEV_NAME;

			}
		}
		cmd_tpssmd1_inq.Close();


		//----------------------------------------------------------
		//第四块，制造命令相关
		Log::Trace("", __FUNCTION__, "制造命令查询");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * FROM TPSSM10 "
				"  WHERE PONO_STATUS < 18 "
				" ORDER BY CC_MACH_NO, CC_SEQ ASC "  //因没有浇铸顺画面，暂时用此方式排序
				);
			break;
		}
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.ExecuteReader();
		while(cmd_tpssm10_inq.Read())
		{
			cmd_tpssm10_inq.Fetch(tpssm10);
			tpssm10.TrimOrBlank();
			CDataRow& row_pono = bcls_ret->Tables["PONO"].Rows.Add();   //新增空行

			//命令数据赋值
			row_pono["PONO"] =  tpssm10.PONO;
			row_pono["CC_MACH_NO"] =  tpssm10.CC_MACH_NO;
			row_pono["ST_NO"] =  tpssm10.ST_NO;
			row_pono["REFINE_ROUTE_CODE"] = tpssm10.SR_DIV;
			row_pono["SMELT_MODE"] =  (tpssm10.SMELT_MODE == "4")? "2":"1";
			row_pono["RESTRAND_NO"] =  tpssm10.RESTRAND_FLAG;
			row_pono["CC_SEQ"] =  tpssm10.CC_SEQ;
			row_pono["PONO_PLAN_DATE"] =  tpssm10.PLAN_DATE;
			row_pono["SG_SIGN"] =  tpssm10.SG_SIGN;
			row_pono["GUIGE"] = tpssm10.SLAB_THICK.ToString() + "*" + tpssm10.SLAB_WIDTH.ToString() + "*" + tpssm10.SLAB_LEN.ToString();

			//sprintf(cast_lot_no_and_div,"%6.6s-%02d",(const char*)tpssm10.CAST_LOT_NO,(const char*)tpssm10.CAST_LOT_DIV_NO);
			cast_lot_no_and_div.Format("%6.6s%-02d",(const char*)tpssm10.CAST_LOT_NO,tpssm10.CAST_LOT_DIV_NO.ToInt32());
			row_pono["CAST_LOT_NO"] = cast_lot_no_and_div;
			row_pono["POUR_TIME"] = tpssm10.POUR_TIME;
			
		}
		cmd_tpssm10_inq.Close();


		//-------------------------------------------------------
		//第五块，设备维修
		Log::Trace("", __FUNCTION__,"设备特殊状态");
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * FROM TPSSM18 "
				"  WHERE DEV_STATUS = '1'"
				);
			break;
		}

		cmd_tpssm18_inq.SetCommandText(sqlstr);
		cmd_tpssm18_inq.ExecuteReader();
		while(cmd_tpssm18_inq.Read())
		{
			cmd_tpssm18_inq.Fetch(tpssm18);
			tpssm18.TrimOrBlank();

			CDataRow & row_stop = bcls_ret->Tables["STOP"].Rows.Add();
			row_stop["DEV_CODE"] =  tpssm18.DEV_CODE;
			row_stop["START_TIME"] =  tpssm18.START_TIME;
			row_stop["END_TIME"] =  tpssm18.END_TIME;
			row_stop["DEV_STATUS_REMARK"] = tpssm18.DEV_STATUS_REMARK ;
			row_stop["STOP_FLAG"] =  tpssm18.STOP_FLAG;
		}
		cmd_tpssm18_inq.Close();


		//-------------------------------------------------------
		//第六块 各工序设备处理时间
		Log::Trace("", __FUNCTION__,"设备处理时间");
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT DISTINCT(ST_NO) FROM TPSSM10 "
				"  WHERE PONO_STATUS < 83 "
				);
			break;
		}
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.ExecuteReader();
		while(cmd_tpssm10_inq.Read())
		{
			tpssm10.ST_NO = cmd_tpssm10_inq.GetString(1);
			Log::Trace("", __FUNCTION__,"ST_NO=[{0}]",tpssm10.ST_NO);


			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				//sqlstr = CString(
				//	" SELECT d4.ST_NO, d1.DEV_CODE, d5.STD_PROC_TIME "
				//	"   FROM TPSSMD4 d4,  TPSSMD1 d1,  TPSSMD5 d5 "
				//	"  WHERE d4.ST_NO         = @st_no "
				//	"    AND d4.DEV_ID = d1.DEV_ID "
				//	"    AND d4.DEV_ID = d5.DEV_ID "
				//	"    AND d4.PTN_NO        = d5.PTN_NO "
				//	);
				sqlstr = CString(
					" SELECT ST_NO, d1.DEV_CODE, d5.STD_PROC_TIME "
					"   FROM TPSSMD3 "
					"  WHERE ST_NO         = @st_no "
					
					);
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tpssm10.ST_NO);
			cmd_inq.ExecuteReader();
			while(cmd_inq.Read())
			{
				tpssmd4.ST_NO = cmd_inq.GetString(1);
				tpssmd1.DEV_CODE = cmd_inq.GetString(2);
				tpssmd5.STD_PROC_TIME = cmd_inq.GetDecimal(3);

				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] =  tpssmd4.ST_NO;
				row_dt["DEV_CODE"] =  tpssmd1.DEV_CODE;
				row_dt["PROC_TIME"] =  tpssmd5.STD_PROC_TIME;
			}
			cmd_inq.Close();
		}
		cmd_tpssm10_inq.Close();


		//没有查询到记录时，读取全部?????????
		if (bcls_ret->Tables["PROC_TIME"].Rows.get_Count() < 1)
		{
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = CString(
					" SELECT T.ST_NO,A.DEV_CODE,C.STD_PROC_TIME "
					"   FROM TPSSMD4 T ,TPSSMD1 A ,TPSSMD5 C "
					"  WHERE T.DEV_ID = A.DEV_ID "
					"    AND T.PTN_NO = C.PTN_NO "
					"    AND T.DEV_ID = C.DEV_ID "
					);
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("TPSSM10.ST_NO",tpssm10.ST_NO);
			cmd_inq.ExecuteReader();
			while(cmd_inq.Read())
			{
				tpssmd4.ST_NO = cmd_inq.GetString(1);
				tpssmd1.DEV_CODE = cmd_inq.GetString(2);
				tpssmd5.STD_PROC_TIME = cmd_inq.GetDecimal(3);
				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] =  tpssmd4.ST_NO;
				row_dt["DEV_CODE"] =  tpssmd1.DEV_CODE;
				row_dt["PROC_TIME"] =  tpssmd5.STD_PROC_TIME;
			}
			cmd_inq.Close();
		}


		//-------------------------------------------------------
		//第六块  传搁时间
		Log::Trace("", __FUNCTION__,"设备传搁时间");
		//D6表代码为工位代码+工位号组成tpssmd6.DEV_MOVE_START
		//需要将D6表的代码转换为设备代码和设备类型
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				//" SELECT DEV_MOVE_START,MOVE_TIME,DEV_MOVE_END "
				" SELECT DEV_MOVE_START, MOVE_TIME, DEV_MOVE_END "
				"   FROM TPSSMD6 "
				);
			break;
		}
		cmd_tpssmd6_inq.SetCommandText(sqlstr);
		cmd_tpssmd6_inq.ExecuteReader();
		while(cmd_tpssmd6_inq.Read())
		{
			tpssmd6.DEV_MOVE_START = cmd_tpssmd6_inq.GetString(1);
			tpssmd6.MOVE_TIME =  cmd_tpssmd6_inq.GetDecimal(2);
			tpssmd6.DEV_MOVE_END =  cmd_tpssmd6_inq.GetString(3);

			if (tpssmd6.DEV_MOVE_START.Trim() == "" || tpssmd6.DEV_MOVE_END.Trim() == "")
				continue;

			CDataRow & row_mt = bcls_ret->Tables["MOVE_TIME"].Rows.Add();
			row_mt["START_DEV"] = tpssmd6.DEV_MOVE_START;
			row_mt["END_DEV"] = tpssmd6.DEV_MOVE_END;
			row_mt["MOVE_TIME"] = tpssmd6.MOVE_TIME;

		}
		cmd_tpssmd6_inq.Close();


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	cmd_tpssm12_inq.Close();
	cmd_tpssm10_inq.Close();
	cmd_tpssm11_inq.Close();
	cmd_tpssmd1_inq.Close();
	cmd_tpssm03_inq.Close();
	cmd_tpssm02_inq.Close();
	cmd_tpssmc2_inq.Close();
	cmd_tpssm18_inq.Close();
	cmd_inq.Close();

	return doFlag;

}
