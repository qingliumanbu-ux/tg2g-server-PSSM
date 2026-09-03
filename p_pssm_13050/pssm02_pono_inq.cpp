/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   周平
Version:    1.0
Date:     2015-11-7 17:13:56
Description: 炼钢计划LOT信息查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/










/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM01画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm02_pono_inq)

int f_pssm02_pono_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	CString sqlstr = "";
	int fetchRowCount = 0;
	int i = 0;
	CString ischeck = "";
	int v_slab_num = 0;//计划块数
	int slab_sum = 0;//产出块数

	CString heat_no = "";	
	CString v_lack_per = ""; //顺序号
	CString function_id = "PSSM02L_PONO";
	CString avg_wgt = "";

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tqmts0x("TQMTS0X");
	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");
	CModel tpssm12("TPSSM12");
	CModel tpssm42("TPSSM42");
	CModel tep0002("TEP0002");//找规格对应的具体值

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm33_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm41_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tep0002_inq(conn);

	try
	{
		//--------------------------------
		//获取传入参数
		tpssm01["CAST_LOT_NO"] = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO"];
		v_lack_per = bcls_rec->Tables[0].Rows[0]["LACK_PER"];
		tpssm01["PLAN_DATE"] = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"];
		tpssm01["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"];	
		ischeck = bcls_rec->Tables[0].Rows[0]["IS_CHECK"];

		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "pssm02l_pono_inq>CAST_LOT_NO = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
		Log::Info("", __FUNCTION__, "pssm02l_pono_inq>v_lack_per = [{0}]", v_lack_per);
		Log::Info("", __FUNCTION__, "pssm02l_pono_inq>CC_MACH_NO = [{0}]", tpssm01["CC_MACH_NO"].ToString());
		Log::Info("", __FUNCTION__, "pssm02l_pono_inq>PLAN_DATE = [{0}]", tpssm01["PLAN_DATE"].ToString());
		Log::Info("", __FUNCTION__, "pssm02l_lot_inq>ischeck = [{0}]", ischeck);

		i = v_lack_per.Find("-");
		v_lack_per = v_lack_per.Substring(0, i);
		Log::Info("", __FUNCTION__, "v_lack_per = [{0}]", v_lack_per);

		//如果前台传了FUNCTION_ID，后台直接调用就可以
		if (bcls_rec->Tables[0].Columns.IndexOf("function_id") < 0)
		{
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "FUNCTION_ID");
			bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"] = function_id;
		}
		else
		{
			function_id = bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"];
		}
		Log::Trace("", __FUNCTION__, "===function_id =  [{0}]", function_id);

		if (bcls_ret->Tables.get_Count() < 1)
		{
			//在bcls_ret 中增加一个块，放查询结果
			bcls_ret->Tables.Add();
		}
		f_edsetcustominfo(bcls_rec, bcls_ret);	

		if (tpssm01["PLAN_DATE"].ToString().Trim() == "")
		{
			//取计划日期
			sqlstr = " SELECT DISTINCT PLAN_DATE FROM TPSSM01 WHERE CAST_LOT_NO = @CAST_LOT_NO  ORDER BY PLAN_DATE DESC";
			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm01_inq.ExecuteReader();
			if (cmd_tpssm01_inq.Read())
			{
				tpssm01["PLAN_DATE"] = cmd_tpssm01_inq.GetString(1);
			}
			cmd_tpssm01_inq.Close();
		}		

		sqlstr = "SELECT * "
			" FROM TPSSM01 "
			" WHERE 1=1 "
			//" AND PLAN_DATE = @PLAN_DATE "
			" AND CC_MACH_NO = @CC_MACH_NO "
			" and cast_lot_no=@tpssm01.CAST_LOT_NO  ";
		if (ischeck == "1")
		{
			sqlstr += " AND CAST_LOT_NO IN ( SELECT CAST_LOT_NO  FROM TPSSM02  WHERE LACK_PER = @LACK_PER  AND LOT_STATUS = 9 ) ";
		}
		else
		{
			sqlstr += " AND CAST_LOT_NO IN ( SELECT CAST_LOT_NO  FROM TPSSM02  WHERE LACK_PER = @LACK_PER  AND LOT_STATUS <> 9 ) ";
		}
		sqlstr += " ORDER BY CC_SEQ, CAST_LOT_NO ,CAST_LOT_DIV_NO ASC ";			

		cmd_tpssm01_inq.SetCommandText(sqlstr);
		//cmd_tpssm01_inq.Parameters.Set("PLAN_DATE", tpssm01["PLAN_DATE"].ToString());
		cmd_tpssm01_inq.Parameters.Set("CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("LACK_PER", v_lack_per);	

		cmd_tpssm01_inq.ExecuteReader();
		while (cmd_tpssm01_inq.Read())
		{
			cmd_tpssm01_inq.Fetch(tpssm01);
			tpssm01.TrimOrBlank();

			tpssm01.MergeTo(bcls_ret->Tables[0]);
			if (bcls_ret->Tables[0].Columns.IndexOf("STATION_NO") < 0)
			{
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NO");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_CONFM_TIME");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "TAP_START_TIME");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "TAP_END_TIME");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_BEGIN_TIME");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_END_TIME");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROD_WT"); 
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_SUM");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "REFINE_ROUTE_CODE");
			}

			if (tpssm01["SEND_PLANNER"].ToString().Trim() != "")  ///20170301 hhn 新增查询定重目标重量
			{
				Log::Info("", __FUNCTION__, "pssm02l_lot_inq>SEND_PLANNER = [{0}]", tpssm01["SEND_PLANNER"].ToString());
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:				// MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT  CODE_DESC_2_CONTENT FROM TEP0002 WHERE CODE_CLASS='PSWT' AND CODE = @tpssm01.SEND_PLANNER ";  //根据规格找出对应值
					break;
				}

				cmd_tep0002_inq.SetCommandText(sqlstr);
				cmd_tep0002_inq.Parameters.Set("tpssm01.SEND_PLANNER", tpssm01["SEND_PLANNER"].ToString());
				cmd_tep0002_inq.ExecuteReader();
				if (cmd_tep0002_inq.Read())
				{
					avg_wgt = cmd_tep0002_inq.GetString(1);
				}
				bcls_ret->Tables[0].Rows[fetchRowCount]["CODE_DESC_2_CONTENT"] = avg_wgt;
				cmd_tep0002_inq.Close();

			}
			tpssm11["PONO"] = tpssm01["PONO"];
			sqlstr = "tpssm11.Query()";
			ret = tpssm11.QueryCount("PONO");
			if (ret == 1)
			{				
				tpssm11.Query("PONO");

				bcls_ret->Tables[0].Rows[fetchRowCount]["HEAT_NO"] = tpssm11["HEAT_NO"];
				bcls_ret->Tables[0].Rows[fetchRowCount]["STATION_NO"] = tpssm11["BACKLOG_EA"].ToString().SubstringNE(1, 1);
				//bcls_ret->Tables[0].Rows[fetchRowCount]["AR_START_TIME"] = tpssm11.AR_START_TIME;//吹氩开始时刻
				//bcls_ret->Tables[0].Rows[fetchRowCount]["AR_END_TIME"] = tpssm11.AR_END_TIME;//吹氩结束时刻
				bcls_ret->Tables[0].Rows[fetchRowCount]["HEAT_CONFM_TIME"] = tpssm11["HEAT_CONFM_TIME"];
				bcls_ret->Tables[0].Rows[fetchRowCount]["REFINE_ROUTE_CODE"] = tpssm11["REFINE_ROUTE_CODE"];
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["AREA_ID"] = 3;
				sqlstr = "tpssm12.Query(SM_PLAN_NO,AREA_ID3)";
				ret = tpssm12.QueryCount("SM_PLAN_NO,AREA_ID");
				//Log::Info("", __FUNCTION__, "--ret = [{0}]", ret);
				if (ret == 1)
				{					
					tpssm12.Query("SM_PLAN_NO,AREA_ID");
					
					bcls_ret->Tables[0].Rows[fetchRowCount]["TAP_START_TIME"] = tpssm12["START_TIME"];
					bcls_ret->Tables[0].Rows[fetchRowCount]["TAP_END_TIME"] = tpssm12["END_TIME"];
				}
				else
				{
					Log::Info("", __FUNCTION__, "PONO=[{0}]tpssm12表查出多条记录", tpssm11["PONO"].ToString());
				}


				tpssm12["DEV_CODE"] = "R1"; //RH
				sqlstr = "tpssm12.Query(SM_PLAN_NO,DEV_CODE)";
				ret = tpssm12.QueryCount("SM_PLAN_NO,DEV_CODE");
				//Log::Info("", __FUNCTION__, "--ret = [{0}]", ret);
				if (ret == 1)
				{					
					tpssm12.Query("SM_PLAN_NO,DEV_CODE");

					bcls_ret->Tables[0].Rows[fetchRowCount]["PROC_START_T"] = tpssm12["START_TIME"];
					bcls_ret->Tables[0].Rows[fetchRowCount]["PROC_END_T"] = tpssm12["END_TIME"];
				}
				else
				{
					Log::Info("", __FUNCTION__, "PONO=[{0}]tpssm12表查出多条记录", tpssm11["PONO"].ToString());
				}
				
				tpssm12["AREA_ID"] = 5;
				sqlstr = "tpssm12.Query(SM_PLAN_NO,AREA_ID5)";
				ret = tpssm12.QueryCount("SM_PLAN_NO,AREA_ID");
				if (ret == 1)
				{					
					tpssm12.Query("SM_PLAN_NO,AREA_ID");

					bcls_ret->Tables[0].Rows[fetchRowCount]["CC_BEGIN_TIME"] = tpssm12["START_TIME"];
					bcls_ret->Tables[0].Rows[fetchRowCount]["CC_END_TIME"] = tpssm12["END_TIME"];
				}
				else
				{
					Log::Info("", __FUNCTION__, "PONO=[{0}]tpssm12表查出多条记录", tpssm11["PONO"].ToString());
				}
			}
			else
			{
				tpssm41["PONO"] = tpssm01["PONO"];
				sqlstr = "tpssm41.Query()";
				ret = tpssm41.QueryCount("PONO");
				if (ret == 1)
				{					
					tpssm41.Query("PONO");

					bcls_ret->Tables[0].Rows[fetchRowCount]["HEAT_NO"] = tpssm41["HEAT_NO"];
					bcls_ret->Tables[0].Rows[fetchRowCount]["STATION_NO"] = tpssm41["BACKLOG_EA"].ToString().SubstringNE(1, 1);
					//bcls_ret->Tables[0].Rows[fetchRowCount]["AR_START_TIME"] = tpssm41.AR_START_TIME;
					//bcls_ret->Tables[0].Rows[fetchRowCount]["AR_END_TIME"] = tpssm41.AR_END_TIME;
					bcls_ret->Tables[0].Rows[fetchRowCount]["HEAT_CONFM_TIME"] = tpssm41["HEAT_CONFM_TIME"];
					bcls_ret->Tables[0].Rows[fetchRowCount]["REFINE_ROUTE_CODE"] = tpssm41["REFINE_ROUTE_CODE"];
					tpssm42["SM_PLAN_NO"] = tpssm41["SM_PLAN_NO"];
					tpssm42["AREA_ID"] = 3;

					sqlstr = "tpssm42.Query(SM_PLAN_NO,AREA_ID3)";
					ret = tpssm42.QueryCount("SM_PLAN_NO,AREA_ID");
					if (ret == 1)
					{						
						tpssm42.Query("SM_PLAN_NO,AREA_ID");
						
						bcls_ret->Tables[0].Rows[fetchRowCount]["TAP_START_TIME"] = tpssm42["START_TIME"];
						bcls_ret->Tables[0].Rows[fetchRowCount]["TAP_END_TIME"] = tpssm42["END_TIME"];
					}
					else
					{
						Log::Info("", __FUNCTION__, "PONO=[{0}]tpssm42表查出多条记录", tpssm41["PONO"].ToString());
					}


					tpssm42["DEV_CODE"] = "R1"; //RH
					sqlstr = "tpssm42.Query(SM_PLAN_NO,DEV_CODE)";
					ret = tpssm42.QueryCount("SM_PLAN_NO,DEV_CODE");
					if (ret == 1)
					{						
						tpssm42.Query("SM_PLAN_NO,DEV_CODE");

						bcls_ret->Tables[0].Rows[fetchRowCount]["PROC_START_T"] = tpssm42["START_TIME"];
						bcls_ret->Tables[0].Rows[fetchRowCount]["PROC_END_T"] = tpssm42["END_TIME"];
					}
					else
					{
						Log::Info("", __FUNCTION__, "PONO=[{0}]tpssm42表查出多条记录", tpssm41["PONO"].ToString());
					}
					
					tpssm42["AREA_ID"] = 5;
					sqlstr = "tpssm42.Query(SM_PLAN_NO,AREA_ID5)";
					ret = tpssm42.QueryCount("SM_PLAN_NO,AREA_ID");
					if (ret == 1)
					{						
						tpssm42.Query("SM_PLAN_NO,AREA_ID");

						bcls_ret->Tables[0].Rows[fetchRowCount]["CC_BEGIN_TIME"] = tpssm42["START_TIME"];
						bcls_ret->Tables[0].Rows[fetchRowCount]["CC_END_TIME"] = tpssm42["END_TIME"];
					}
					else
					{
						Log::Info("", __FUNCTION__, "PONO=[{0}]tpssm42表查出多条记录", tpssm41["PONO"].ToString());						
					}
				}
			}
			
			////取产出重量
			sqlstr = "SELECT SUM(SLAB_WT) FROM TMMSM33 WHERE PONO = @PONO ";

			cmd_tpssm33_inq.SetCommandText(sqlstr);
			cmd_tpssm33_inq.Parameters.Set("PONO", tpssm01["PONO"].ToString());
			cmd_tpssm33_inq.ExecuteReader();
			if (cmd_tpssm33_inq.Read())
			{
				bcls_ret->Tables[0].Rows[fetchRowCount]["PROD_WT"] = cmd_tpssm33_inq.GetString(1).Trim();
			}
			cmd_tpssm33_inq.Close();

			//2019zqq add一个PONO计划支数
			tpssm03["PONO"] = tpssm01["PONO"];
			v_slab_num = tpssm03.QueryCount("PONO");
			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_SUM"] = v_slab_num;

			fetchRowCount++;

		}
		cmd_tpssm01_inq.Close();
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
