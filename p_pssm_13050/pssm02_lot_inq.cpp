/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   周平
Version:    1.0
Date:     2015-11-7 17:13:56
Description: 炼钢计划LOT信息查询
update:zhengqiangqiang 20230701
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
BM2F_ENTERACE(pssm02_lot_inq)

int f_pssm02_lot_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	CString sqlstr = "";
	int num = 0;	
	int fetchRowCount = 0;

	CString v_plan_date_start = " ";
	CString v_plan_date_end = " ";
	CString v_cast_lot_no_pre = " ";
	CDecimal v_lack_per_pre = 0;
	CString v_plan_date_pre = " ";
	CString v_mat_specs = "";
	int v_slab_thick = 0;
	int v_slab_width = 0;
	CString v_lack_per = "";
	CString ischeck = "";
	CString old_st_no = "";
	CString st_no_desc = "";
	CDecimal cast_lot_sum_2 = 0;
	CDecimal v_prod_wt = 0;
	CDecimal v_plan_tap_wt = 0;
	CDecimal slab_num_l = 0;

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tqmts0x("TQMTS0X");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm01_inq2(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tmmsm33_inq(conn);  //与DB 建立连接。

	try
	{
		//--------------------------------
		//获取传入参数
		tpssm01["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"];
		v_plan_date_start = bcls_rec->Tables[0].Rows[0]["PLAN_DATE_START"].ToString().SubstringNE(0,8);
		v_plan_date_end = bcls_rec->Tables[0].Rows[0]["PLAN_DATE_END"].ToString().SubstringNE(0, 8);;
		ischeck = bcls_rec->Tables[0].Rows[0]["IS_CHECK"];
		
		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "pssm02_lot_inq->cc_mach_no = [{0}]", tpssm01["CC_MACH_NO"].ToString());
		Log::Info("", __FUNCTION__, "pssm02_lot_inq->v_plan_date_start = [{0}]", v_plan_date_start);
		Log::Info("", __FUNCTION__, "pssm02_lot_inq->v_plan_date_end = [{0}]", v_plan_date_end);
		Log::Info("", __FUNCTION__, "pssm02_lot_inq->ischeck = [{0}]", ischeck);
			

		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");	//厂别
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PLAN_DATE");	//生产日期
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LACK_PER");		//顺序号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "RESTRAND_FLG");	//RESTRAND_FLG
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");	//浇铸批号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LOT_STATUS");	//炉次状态
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO");		//出钢记号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPECS");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_SUM"); //炉数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_LOT_SUM_L"); //可编炉数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_NUM"); //支数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_NUM_PROD"); //产出支数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "REMARK");		//备注
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_EA");	//工序路径
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "APP_TYPE");		//材料申请类型
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "OLD_ST_NO");//原出钢记号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO_DESC");	//出钢记号描述
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PROD_WT");		//产出重量
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_TAP_WT");		//目标重量

		//取返回块对应值
		switch (conn->DatabaseKind)
		{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT * "
					" FROM TPSSM01 A "
					" WHERE A.PLAN_DATE <= @v_plan_date_end "
					" AND A.PLAN_DATE >= @v_plan_date_start "
					" AND A.CC_MACH_NO = @tpssm01.CC_MACH_NO ";
					if (ischeck == "1")
					{
						sqlstr += " AND A.CAST_LOT_NO IN ( SELECT CAST_LOT_NO FROM TPSSM02 WHERE LOT_STATUS = 9 ) ";
					}
					else
					{
						sqlstr += " AND A.CAST_LOT_NO IN ( SELECT CAST_LOT_NO FROM TPSSM02 WHERE LOT_STATUS <> 9 ) ";
					}
					
					sqlstr += " ORDER BY A.PLAN_DATE, A.CC_MACH_NO,A.CC_SEQ, A.CAST_LOT_NO ASC, A.CAST_LOT_DIV_NO ASC ";
				
				break;
		}
		cmd_tpssm01_inq.Parameters.Set("v_plan_date_end", v_plan_date_end);
		cmd_tpssm01_inq.Parameters.Set("v_plan_date_start", v_plan_date_start);
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());

		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.ExecuteReader();
		
		v_cast_lot_no_pre = " ";
		while(cmd_tpssm01_inq.Read())
		{
			cmd_tpssm01_inq.Fetch(tpssm01);			
				
			if(tpssm01["CAST_LOT_NO"].ToString() == v_cast_lot_no_pre)
			{
				continue;
			}
			else
			{
				Log::Info("", __FUNCTION__, "tpssm01.CAST_LOT_NO = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
			}
			
			tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm02.Query("FACTORY_DIV, CAST_LOT_NO");	

			Log::Info("", __FUNCTION__, "tpssm02.LOT_STATUS = [{0}]", tpssm02["LOT_STATUS"].ToDecimal());	


			tpssm03["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			tpssm03["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm03["SLAB_PROD_FLAG"] = "1";
			slab_num_l = tpssm03.QueryCount("FACTORY_DIV,CAST_LOT_NO,SLAB_PROD_FLAG");

			Log::Info("", __FUNCTION__, "==slab_num_l=[{0}][{1}]", slab_num_l, tpssm02["SLAB_NUM"].ToDecimal());


			//原出钢记号，出钢记号描述
			//tqmts0x.OLD_ST_NO = " ";
			//tqmts0x["ST_NO"]_DESC = " ";
			//tqmts0x["ST_NO"] = tpssm01["ST_NO"];
			//
			//tqmts0x["VALID_FLAG"] = "1";
			//sqlstr = "tqmts0x.Query()";
			//if (tqmts0x.Query("ST_NO, VALID_FLAG") == false)
			//{
			//	tqmts0x.OLD_ST_NO = "未获取";
			//	tqmts0x.ST_NO_DESC = "工艺卡有误或失效！";
			//}

			//old_st_no = tqmts0x.OLD_ST_NO;
			//st_no_desc = tqmts0x["ST_NO_DESC"].ToString();

			//Log::Info("", __FUNCTION__, "ST_NO_DESC = [{0}]", tqmts0x.ST_NO_DESC);
			
			//查询断面规格
			sqlstr = " SELECT DISTINCT TO_CHAR(CEIL(SLAB_THICK))||'*'||TO_CHAR(CEIL(SLAB_WIDTH)) "
				" FROM TPSSM03 "
				" WHERE PONO = @pono "
				" AND FACTORY_DIV = @factory_div "
				" FETCH FIRST 1 ROWS ONLY ";
			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div",tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();
			
			if(cmd_tpssm03_inq.Read())
			{
				v_mat_specs = cmd_tpssm03_inq.GetString(1);
			}
			cmd_tpssm03_inq.Close();

			//Log::Info("", __FUNCTION__, "v_mat_specs = [{0}]",v_mat_specs);	

			//可编计划炉数
			sqlstr = " select count(*) "
				" from tpssm01 "
				" where cast_lot_no = @cast_lot_no "
				" and pono_status < 18 ";
			cmd_tpssm01_inq2.SetCommandText(sqlstr);
			cmd_tpssm01_inq2.Parameters.Set("cast_lot_no", tpssm01["CAST_LOT_NO"].ToString());
			cast_lot_sum_2 = cmd_tpssm01_inq2.ExecuteScalar();

			Log::Info("", __FUNCTION__, "tpssm02.LACK_PER = [{0}],v_lack_per_pre = [{1}]", tpssm02["LACK_PER"].ToDecimal(), v_lack_per_pre);
			Log::Info("", __FUNCTION__, "tpssm01.PLAN_DATE = [{0}],v_plan_date_pre = [{1}]", tpssm01["PLAN_DATE"].ToString(), v_plan_date_pre);

			if (tpssm02["LACK_PER"].ToDecimal() == v_lack_per_pre && v_plan_date_pre.Compare(tpssm01["PLAN_DATE"].ToString()) >= 0)
			{
				num++;
			}
			else
			{
				num = 1;
			}
			/*if (tpssm02["LACK_PER"].ToDecimal() != v_lack_per_pre)
			{
			num = 1;
			}*/
			Log::Info("", __FUNCTION__, "num = [{0}]", num);	


			//取产出重量
			sqlstr = "SELECT SUM(SLAB_WT) FROM TMMSM33 WHERE PONO  IN ( SELECT PONO FROM TPSSM01 WHERE CAST_LOT_NO=@tpssm02.CAST_LOT_NO )";
			cmd_tmmsm33_inq.SetCommandText(sqlstr);
			cmd_tmmsm33_inq.Parameters.Set("tpssm02.CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());
			cmd_tmmsm33_inq.ExecuteReader();
			if (cmd_tmmsm33_inq.Read())
			{
				v_prod_wt = cmd_tmmsm33_inq.GetDecimal(1);
			}
			cmd_tmmsm33_inq.Close();

			//取计划重量
			sqlstr = "SELECT SUM(PLAN_TAP_WT) FROM TPSSM01 WHERE CAST_LOT_NO=@tpssm02.CAST_LOT_NO ";
			cmd_tmmsm33_inq.SetCommandText(sqlstr);
			cmd_tmmsm33_inq.Parameters.Set("tpssm02.CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());
			cmd_tmmsm33_inq.ExecuteReader();
			if (cmd_tmmsm33_inq.Read())
			{
				v_plan_tap_wt = cmd_tmmsm33_inq.GetDecimal(1);
			}
			cmd_tmmsm33_inq.Close();


			Log::Info("", __FUNCTION__, "==v_prod_wt=[{0}]", v_prod_wt);

			//块赋值
			CDataRow& row = bcls_ret->Tables[0].Rows.Add();		

			//row["LACK_PER"] = tpssm02["LACK_PER"].ToDecimal().ToString() + "-" + num.ToString();
			row["LACK_PER"] = CString::Format("%s-%d", (const char*)tpssm02["LACK_PER"].ToDecimal().ToString(), num);
			row["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			row["RESTRAND_FLG"] = tpssm01["RESTRAND_FLG"];
			row["RESTRAND_FLG"] = tpssm01["RESTRAND_FLG"];
			row["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			row["LOT_STATUS"] = tpssm02["LOT_STATUS"];
			row["ST_NO"] = tpssm01["ST_NO"]; 
			row["OLD_ST_NO"] = old_st_no;
			row["ST_NO_DESC"] = st_no_desc;
			row["BACKLOG_EA"] = tpssm01["BACKLOG_EA"];
			row["CAST_LOT_SUM"] = tpssm02["CAST_LOT_SUM"];
			row["CAST_LOT_SUM_L"] = cast_lot_sum_2;
			row["APP_TYPE"] = tpssm02["APP_TYPE"];
			row["SLAB_NUM"] = tpssm02["SLAB_NUM"];
			row["SLAB_NUM_PROD"] = slab_num_l;
			row["PROD_WT"] = v_prod_wt;
			row["PLAN_TAP_WT"] = v_plan_tap_wt;
			row["MAT_SPECS"] = v_mat_specs;
			row["REMARK"] = tpssm01["REMARK"];


			if(tpssm01["PLAN_DATE"].ToString() == v_plan_date_pre)
			{
				row["PLAN_DATE"] = " ";
			}
			else
			{
				row["PLAN_DATE"] = tpssm01["PLAN_DATE"];
			}			
			row["PLAN_DATE"] = tpssm01["PLAN_DATE"];
			v_lack_per_pre = tpssm02["LACK_PER"];
			v_plan_date_pre = tpssm01["PLAN_DATE"];
			v_cast_lot_no_pre = tpssm01["CAST_LOT_NO"];
			old_st_no = "";
			st_no_desc = "";

			Log::Info("", __FUNCTION__, "v_lack_per_pre = [{0}]", v_lack_per_pre);
			Log::Info("", __FUNCTION__, "v_cast_lot_no_pre = [{0}]", v_cast_lot_no_pre);
			
			
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
