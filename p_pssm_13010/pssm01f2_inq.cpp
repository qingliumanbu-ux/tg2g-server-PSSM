/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56  
Description: 炼钢计划制造命令查询
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
BM2F_ENTERACE(pssm01f2_inq)

int f_pssm01f2_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;

	CString sqlstr					= "";
	CString sqlstr_count			= "";
	CString sqlstr_temp				= "";
	CString sqlstr_temp_order = "";
	int TotalRecordCount			= 0;

	int v_slab_thick = 0;
	int v_slab_width = 0;
	int v_slab_len = 0;
	CDecimal v_mat_tube = 0;
	CString v_billet_type;
	CString slab_thick = "";
	CString slab_width = "";
	CString slab_len = "";
	CString v_factory_div = "";
	CString v_mat_specs = "";
	CString v_cc_div = "";
	CString v_last_plan_date = "";
	int fetchRowCount = 0;	
	CString v_prec_roll_plan_no = " ";
	CDecimal v_prec_roll_seq_no = 0;
	CString v_ingot_code = "";

	//系统的分页类信息。
	CPageInfo pageInfo; 
 
	CModel tpssm01("TPSSM01");
		
	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。

	try
	{
		try
		{
			//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch(CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize   = 1000;
		}

		//--------------------------------
		//获取传入参数
		tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);
					
		/* ***** 打印输入参数 ***** */	
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]",tpssm01["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "cc_mach_no = [{0}]",tpssm01["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "cast_lot_no = [{0}]",tpssm01["CAST_LOT_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pono_status = [{0}]",tpssm01["PONO_STATUS"].ToDecimal().ToInt32());
		////Log::Info("", __FUNCTION__, "plan_date = [{0}]",tpssm01["PLAN_DATE"].ToString());
		
		////设置返回块参数
		//bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_DIV");
		//bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPECS");
		////bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MAT_TUBE");
		//bcls_ret->Tables[0].Columns.Add(DT_STRING, "BILLET_TYPE");

		if (tpssm01["PLAN_DATE"].ToString().Trim() != "")
		{
			tpssm01["PLAN_DATE"] = tpssm01["PLAN_DATE"].ToString().Substring(0, 8);

			Log::Info("", __FUNCTION__, "==PLAN_DATE = [{0}]", tpssm01["PLAN_DATE"].ToString().Substring(0, 8));
		}


		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr_count = " SELECT COUNT(1) "
					"   FROM TPSSM01 "
					"  WHERE 1=1 "
					;
				sqlstr	 = " SELECT * "
					"   FROM TPSSM01 "
					"  WHERE 1=1 "
					;

				if(tpssm01["FACTORY_DIV"].ToString().Trim() != "")
				{
					sqlstr_temp	+= " AND FACTORY_DIV		= @tpssm01.FACTORY_DIV"; 
				}

				if(tpssm01["CC_MACH_NO"].ToString().Trim() != "")
				{
					sqlstr_temp	+= " AND CC_MACH_NO		= @tpssm01.CC_MACH_NO"; 
				}
				if(tpssm01["PONO"].ToString().Trim() != "")
				{
					sqlstr_temp	+= " AND PONO			like '%'|| @tpssm01.PONO||'%'"; 
				}
				if(tpssm01["CAST_LOT_NO"].ToString().Trim() != "")
				{
					sqlstr_temp	+= " AND CAST_LOT_NO		= @tpssm01.CAST_LOT_NO"; 
				}
				if(tpssm01["PONO_STATUS"].ToDecimal() != 0 )
				{
					sqlstr_temp	+= " AND PONO_STATUS			= @tpssm01.PONO_STATUS"; 
				}				
				else
				{
					sqlstr_temp += " AND PONO_STATUS >=11 AND PONO_STATUS < 14 ";
				}
				if(tpssm01["PLAN_DATE"].ToString().Trim()!="")
				{
					sqlstr += " AND PLAN_DATE = @tpssm01.PLAN_DATE ";
				}
			
				sqlstr_temp_order += " ORDER BY PLAN_DATE,CC_MACH_NO,CC_SEQ,CAST_LOT_NO,CAST_LOT_DIV_NO ASC ";
			
				sqlstr_count = sqlstr_count + sqlstr_temp;
				sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
				break;
		}
		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV" ,tpssm01["FACTORY_DIV"].ToString()); 
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO"	,tpssm01["CC_MACH_NO"].ToString()); 
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO"	,tpssm01["PONO"].ToString()); 
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CAST_LOT_NO",tpssm01["CAST_LOT_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO_STATUS",tpssm01["PONO_STATUS"].ToDecimal());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PLAN_DATE",tpssm01["PLAN_DATE"].ToString());
	

		cmd_tpssm01_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm01_inq.ExecuteScalar().ToInt32(); 
		//分页获取
		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);
		cmd_tpssm01_inq.Close();

		//设置返回块参数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CC_DIV");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_SPECS"); //规格
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_THICK"); //厚度
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH"); //宽度
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "INGOT_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "MAT_TUBE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BILLET_TYPE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "OLD_ST_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PREC_ROLL_PLAN_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PREC_ROLL_SEQ_NO");

		for (fetchRowCount = 0; fetchRowCount < bcls_ret->Tables[0].Rows.get_Count(); fetchRowCount++)
		{
			tpssm01["PONO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["PONO"];
			tpssm01["CAST_LOT_NO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_NO"];
			tpssm01["CAST_LOT_SUM"] = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_SUM"];
			tpssm01["CAST_LOT_DIV_NO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["CAST_LOT_DIV_NO"];
			tpssm01["PLAN_DATE"] = bcls_ret->Tables[0].Rows[fetchRowCount]["PLAN_DATE"];
			tpssm01["FACTORY_DIV"] = bcls_ret->Tables[0].Rows[fetchRowCount]["FACTORY_DIV"];
			tpssm01["ST_NO"] = bcls_ret->Tables[0].Rows[fetchRowCount]["ST_NO"];

			////Log::Info("", __FUNCTION__, "pono  =[{0}]",tpssm01.PONO );
			////Log::Info("", __FUNCTION__, "cast_lot_no  =[{0}]",tpssm01.CAST_LOT_NO );
			////Log::Info("", __FUNCTION__, "FACTORY_DIV  =[{0}]", tpssm01["FACTORY_DIV"].ToString());
			
			////Log::Info("", __FUNCTION__, "CAST_LOT_SUM  = [{0}], tpssm01["CAST_LOT_DIV_NO"] = [{1}]", tpssm01["CAST_LOT_SUM"].ToDecimal(), tpssm01["CAST_LOT_DIV_NO"].ToDecimal());
			if(tpssm01["CAST_LOT_SUM"].ToDecimal() != 0)
			{
				v_cc_div = CString::Format("%d-%d", tpssm01["CAST_LOT_SUM"].ToDecimal().ToInt32(), tpssm01["CAST_LOT_DIV_NO"].ToDecimal().ToInt32());
			}
		    else
			{
				v_cc_div = " ";
			}
			////Log::Info("", __FUNCTION__, "v_cc_div  =[{0}]", v_cc_div);

			//板坯规格
		
			sqlstr = " SELECT DISTINCT SLAB_THICK,SLAB_WIDTH,SLAB_LEN,PREC_ROLL_PLAN_NO,PREC_ROLL_SEQ_NO,INGOT_CODE FROM TPSSM03 \
						WHERE PONO = @pono \
						  AND FACTORY_DIV = @factory_div ";
			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div",tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();
			
			if(cmd_tpssm03_inq.Read())
			{
				v_slab_thick = cmd_tpssm03_inq.GetDecimal(1).ToInt32();
				v_slab_width = cmd_tpssm03_inq.GetDecimal(2).ToInt32();
				v_slab_len = cmd_tpssm03_inq.GetDecimal(3).ToInt32();
				v_prec_roll_plan_no = cmd_tpssm03_inq.GetString(4);
				v_prec_roll_seq_no = cmd_tpssm03_inq.GetDecimal(5);
				v_ingot_code = cmd_tpssm03_inq.GetString(6);
			}
			cmd_tpssm03_inq.Close();
			slab_thick = CConvert::ToString(v_slab_thick);
			slab_width = CConvert::ToString(v_slab_width);
			slab_len   = CConvert::ToString(v_slab_len);

			v_mat_specs = slab_thick + "*" + slab_width + "*" + slab_len;

			////Log::Info("", __FUNCTION__, "v_mat_specs      = [{0}]",v_mat_specs);

			//铸坯类型		
			sqlstr = " SELECT BILLET_TYPE FROM TPSSM02 WHERE CAST_LOT_NO = @cast_lot_no \
       							AND FACTORY_DIV = @factory_div ";
			cmd_tpssm02_inq.SetCommandText(sqlstr);
			cmd_tpssm02_inq.Parameters.Set("factory_div",tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm02_inq.Parameters.Set("cast_lot_no", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm02_inq.ExecuteReader();
			
			if(cmd_tpssm02_inq.Read())
			{
				v_billet_type = cmd_tpssm02_inq.GetString(1);
			}
			cmd_tpssm02_inq.Close();

			////Log::Info("", __FUNCTION__, "v_billet_type      = [{0}]",v_billet_type);

			//板坯块数
			sqlstr = " SELECT SUM(SLAB_NUM) FROM TPSSM03 WHERE PONO = @pono \
       								AND FACTORY_DIV = @factory_div ";		   
			
			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div",tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString() );
			cmd_tpssm03_inq.ExecuteReader();
			if(cmd_tpssm03_inq.Read())
			{
				v_mat_tube = cmd_tpssm03_inq.GetDecimal(1);
			}
			cmd_tpssm03_inq.Close();

			//////Log::Info("", __FUNCTION__, "v_mat_tube      = [{0}]",v_mat_tube);

			bcls_ret->Tables[0].Rows[fetchRowCount]["CC_DIV"] =  v_cc_div;
			bcls_ret->Tables[0].Rows[fetchRowCount]["MAT_SPECS"] =  v_mat_specs;
			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_THICK"] = slab_thick;
			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_WIDTH"] = slab_width;
			bcls_ret->Tables[0].Rows[fetchRowCount]["INGOT_CODE"] = v_ingot_code;
			bcls_ret->Tables[0].Rows[fetchRowCount]["MAT_TUBE"] =  v_mat_tube;
			bcls_ret->Tables[0].Rows[fetchRowCount]["BILLET_TYPE"] =  v_billet_type;
			bcls_ret->Tables[0].Rows[fetchRowCount]["PREC_ROLL_PLAN_NO"] = v_prec_roll_plan_no;
			bcls_ret->Tables[0].Rows[fetchRowCount]["PREC_ROLL_SEQ_NO"] = v_prec_roll_seq_no;
			if(v_last_plan_date == tpssm01["PLAN_DATE"].ToString())
		    {
				bcls_ret->Tables[0].Rows[fetchRowCount]["PLAN_DATE"] =   " ";
		    }
			v_last_plan_date = tpssm01["PLAN_DATE"];
		}

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL,"TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;	

	 }
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
	return doFlag;

}
