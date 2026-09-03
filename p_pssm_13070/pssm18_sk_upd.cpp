/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2012-01-16
Version:1.0
Description: 状态回退
Update: 2015-03-19 lijie  数据表调整
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件







int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 状态回退
/// <para>数据库表：tpssm10/11/12/13/14/31/25                    </para>
/// <para>主调用函数：PSSM18S画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns></returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_sk_upd)
//-EP_SYSTEM_HEAD_END
int f_pssm18_sk_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	
	//程序用变量
	int doFlag = 0; 
	int ret = 0;
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm12_2("TPSSM12");
	CModel tpssm25("TPSSM25");
	CModel tpssm26("TPSSM26");
	CModel tpssm99("TPSSM99");//履历
	CModel tpssmd1("TPSSMD1");

	CString sqlstr = "";
	CDecimal curr_wp_no = 0;
	CString cc_proc_no ="";
	CString lf_proc_no = "";
	CString v_proc_no = "";
	CString cast_no = "";
	CString new_cast_no = "";
	CDecimal cast_div_no = 0;
	CDecimal v_cast_no = 0;
	CDecimal area_id = 0;
	CString v_heat_no = "";
	CString v_run_status = "";
	CString dev_code = "";
	CDecimal refine_count = 0;

	CString cast_no_26 = "";
	CDecimal cast_div_no_26 = 0;

	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm26_inq(conn);
	CDbCommand cmd_tpssm26_upd(conn);
	CDbCommand cmd_tqmts29_inq(conn);
	CDbCommand cmd_tmmsm33_inq(conn);
	int v_count_prod = 0;

	EIClass in_pssm99trace;//调用履历函数
	//计划履历按一炉为单位
	in_pssm99trace.Tables[0].set_TableName("TRACE");

	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("HEAT_NO"))
		{
			tpssm11["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NO"];
		}
		if (bcls_rec->Tables[0].Columns.Contains("HTNO"))
		{
			tpssm11["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HTNO"];
		}
		if (bcls_rec->Tables[0].Columns.Contains("SM_PLAN_NO"))
		{
			tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"];
		}


	
		tpssm12["AREA_ID"] = bcls_rec->Tables[0].Rows[0]["AREA_ID"];
		////Log::Trace("", __FUNCTION__, "传入heat_no=[{0}],area_id=[{1}]", tpssm11["HEAT_NO"].ToString(),tpssm12["AREA_ID"].ToDecimal().ToInt32());

		area_id = tpssm12["AREA_ID"];

		tpssm99["HEAT_NO"] = tpssm11["HEAT_NO"];

		if (tpssm11["HEAT_NO"].ToString().Trim() != "")
		{
			//已经回炉的不能状态回退
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = " SELECT RUN_STATUS,PONO,STEEL_RETURN_CODE,SM_PLAN_NO,FACTORY_DIV,CC_MACH_NO,CAST_NO,CAST_DIV_NO \
						 					  ,RESTRAND_FLG,TD_CHG_FLG,ST_NO  \
											  					  FROM TPSSM11 \
							WHERE HEAT_NO = @tpssm11.HEAT_NO AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO ";
				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			if (cmd_tpssm11_inq.Read())
			{
				tpssm11["RUN_STATUS"] = cmd_tpssm11_inq.GetString(1);
				tpssm11["PONO"] = cmd_tpssm11_inq.GetString(2);
				tpssm11["STEEL_RETURN_CODE"] = cmd_tpssm11_inq.GetString(3);
				tpssm11["SM_PLAN_NO"] = cmd_tpssm11_inq.GetString(4);
				tpssm11["FACTORY_DIV"] = cmd_tpssm11_inq.GetString(5);
				tpssm11["CC_MACH_NO"] = cmd_tpssm11_inq.GetString(6);
				tpssm11["CAST_NO"] = cmd_tpssm11_inq.GetString(7);
				tpssm11["CAST_DIV_NO"] = cmd_tpssm11_inq.GetDecimal(8);
				tpssm11["RESTRAND_FLG"] = cmd_tpssm11_inq.GetString(9);
				tpssm11["TD_CHG_FLG"] = cmd_tpssm11_inq.GetDecimal(10);
				tpssm11["ST_NO"] = cmd_tpssm11_inq.GetString(11);
			}
			cmd_tpssm11_inq.Close();
		}
		else
		{
			//已经回炉的不能状态回退
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = " SELECT RUN_STATUS,PONO,STEEL_RETURN_CODE,SM_PLAN_NO,FACTORY_DIV,CC_MACH_NO,CAST_NO,CAST_DIV_NO \
						 						 					  ,RESTRAND_FLG,TD_CHG_FLG,ST_NO  \
						FROM TPSSM11 \
						WHERE SM_PLAN_NO = @tpssm11.SM_PLAN_NO ";
				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			if (cmd_tpssm11_inq.Read())
			{
				tpssm11["RUN_STATUS"] = cmd_tpssm11_inq.GetString(1);
				tpssm11["PONO"] = cmd_tpssm11_inq.GetString(2);
				tpssm11["STEEL_RETURN_CODE"] = cmd_tpssm11_inq.GetString(3);
				tpssm11["SM_PLAN_NO"] = cmd_tpssm11_inq.GetString(4);
				tpssm11["FACTORY_DIV"] = cmd_tpssm11_inq.GetString(5);
				tpssm11["CC_MACH_NO"] = cmd_tpssm11_inq.GetString(6);
				tpssm11["CAST_NO"] = cmd_tpssm11_inq.GetString(7);
				tpssm11["CAST_DIV_NO"] = cmd_tpssm11_inq.GetDecimal(8);
				tpssm11["RESTRAND_FLG"] = cmd_tpssm11_inq.GetString(9);
				tpssm11["TD_CHG_FLG"] = cmd_tpssm11_inq.GetDecimal(10);
				tpssm11["ST_NO"] = cmd_tpssm11_inq.GetString(11);
			}
			cmd_tpssm11_inq.Close();
		}
	
		
		//获取26表
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			
			sqlstr = " select cast_no,cast_div_no from TPSSM26 \
					 	WHERE CC_MACH_NO = @tpssm11.CC_MACH_NO \
						  AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
			break;
		}
	

		cmd_tpssm26_inq.SetCommandText(sqlstr);
		cmd_tpssm26_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
		cmd_tpssm26_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm26_inq.ExecuteReader();
				
		
		if (cmd_tpssm26_inq.Read())
		{
		
			tpssm26["CAST_NO"] = cmd_tpssm26_inq.GetString(1);
			tpssm26["CAST_DIV_NO"] = cmd_tpssm26_inq.GetDecimal(2);

			cast_no_26 = tpssm26["CAST_NO"];
			cast_div_no_26 = tpssm26["CAST_DIV_NO"];
			////Log::Trace("", __FUNCTION__, "cast_no_26=[{0}],cast_div_no_26=[{0}]", cast_no_26, cast_div_no_26);
		}

	
		cmd_tpssm26_inq.Close();
		CDecimal stream_no = stream_no.Parse(tpssm26["CAST_NO"].ToString().SubstringNE(2));
		Log::Trace("", __FUNCTION__, "stream_no=[{0}]", stream_no);

		if (stream_no == 0)
		{
			//年份不同，找前一年CAST号最大的一个
			sqlstr = " select MAX(CAST_NO) from tpssm11 "
				" where CC_MACH_NO =@tpssm11.CC_MACH_NO "
				;
			cmd_tpssm26_inq.SetCommandText(sqlstr);
			cmd_tpssm26_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
			cmd_tpssm26_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
			cmd_tpssm26_inq.ExecuteReader();
			if (cmd_tpssm26_inq.Read())
			{
				tpssm26["CAST_NO"] = cmd_tpssm26_inq.GetString(1);
			}
			cmd_tpssm26_inq.Close();

			//根据前一炉找出最大的cast_div_no
			sqlstr = " SELECT MAX(CAST_DIV_NO) FROM TPSSM11 "
				" WHERE CAST_NO = @cast_no";
			cmd_tpssm26_inq.SetCommandText(sqlstr);
			cmd_tpssm26_inq.Parameters.Set("cast_no", tpssm26["CAST_NO"].ToString());//往前退一个浇次号，取出对应的最大分割号
			tpssm26["CAST_DIV_NO"] = cmd_tpssm26_inq.ExecuteScalar();
			cmd_tpssm26_inq.Close();
			////Log::Trace("", __FUNCTION__, "前一个浇次号cast_no=[{0}]，最大分割号 = [{1}]更新到26表对应的记录里", tpssm26["CAST_NO"].ToString(), tpssm26["CAST_DIV_NO"].ToDecimal());
		}
		//if (tpssm11["CAST_NO"].ToString().SubstringNE(1).Trim() != tpssm26["CAST_NO"].ToString().SubstringNE(1).Trim())
		//{
		//	CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), tpssm26["CAST_NO"].ToString() };
		//	CMessageFormat::Format(s.msg, "浇次号={0}不是当前浇次[{1}],不能回退！", arguments, 2);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
		//if (tpssm11["CAST_DIV_NO"].ToDecimal() != tpssm26["CAST_DIV_NO"].ToDecimal())
		//{
		//	CFormattable arguments[] = { tpssm11["CAST_DIV_NO"].ToDecimal(), tpssm26["CAST_DIV_NO"].ToDecimal() };
		//	CMessageFormat::Format(s.msg, "浇次分割号={0}不是当前分割号[{1}],不能回退！", arguments, 2);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		if(tpssm11["STEEL_RETURN_CODE"].ToString().Trim() == "1")
		{
			CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() };
			CMessageFormat::Format(s.msg,_RES("PSSMS0000197")/*熔炼号=[%s]已经回炉不能回退。*/, arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//获取上浇次的CAST_no
		if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm11["TD_CHG_FLG"].ToDecimal() == 1)//
		{
			if (tpssm11["HEAT_NO"].ToString().Trim() != "")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " select substr(CAST_NO, 1, 2) || lpad(to_char((to_number(substr(CAST_NO,3,length(CAST_NO))-1))),4,0) as  CAST_NO_new    \
							 from tpssm11  WHERE  HEAT_NO = @tpssm11.HEAT_NO AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO \
							AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				cmd_tpssm25_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.ExecuteReader();
				if (cmd_tpssm25_inq.Read())
				{
					cast_no = cmd_tpssm25_inq.GetString(1);
					Log::Trace("", __FUNCTION__, "原浇次号cast_no=[{0}]，CC_MACH_NO = [{1}]", tpssm11["CAST_NO"].ToString(), tpssm11["CC_MACH_NO"].ToString());
					Log::Trace("", __FUNCTION__, "计算浇次号cast_no=[{0}],RESTRAND_FLG=[1],TD_CHG_FLG=[{2}]", cast_no, tpssm11["RESTRAND_FLG"].ToString(), tpssm11["TD_CHG_FLG"].ToDecimal());
				}
				cmd_tpssm25_inq.Close();
			}
			else
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " select substr(CAST_NO, 1, 2) || lpad(to_char((to_number(substr(CAST_NO,3,length(CAST_NO))-1))),4,0) as  CAST_NO_new    \
							 from tpssm11  WHERE SM_PLAN_NO = @tpssm11.SM_PLAN_NO \
							AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				//cmd_tpssm25_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.ExecuteReader();
				if (cmd_tpssm25_inq.Read())
				{
					cast_no = cmd_tpssm25_inq.GetString(1);
					Log::Trace("", __FUNCTION__, "原浇次号cast_no=[{0}]，CC_MACH_NO = [{1}]", tpssm11["CAST_NO"].ToString(), tpssm11["CC_MACH_NO"].ToString());
					Log::Trace("", __FUNCTION__, "计算浇次号cast_no=[{0}],RESTRAND_FLG=[1],TD_CHG_FLG=[{2}]", cast_no, tpssm11["RESTRAND_FLG"].ToString(), tpssm11["TD_CHG_FLG"].ToDecimal());
				}
				cmd_tpssm25_inq.Close();
			}

		}
		
		
		v_heat_no = tpssm11["HEAT_NO"];
		v_run_status = tpssm11["RUN_STATUS"];


		//--------------------------------------
		//条件校验1.已经回炉的不能状态回退
		/*if (v_heat_no.Trim() == "" && (tpssm12["AREA_ID"].ToDecimal() == 5 || tpssm12["AREA_ID"].ToDecimal() == 4 || tpssm12["AREA_ID"].ToDecimal() == 3))
		{
			CFormattable arguments[] = { v_heat_no.Trim() };
			CMessageFormat::Format(s.msg, "本炉次还未进行吹炼,炉号为[{0}],不能进行信号回退操作！！", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}*/

		//---------------------------------------------------------
		//条件校验2  已经有切割产出实绩，不能回退
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT COUNT(1) FROM TMMSM33 WHERE HEAT_NO    = @tpssm11.HEAT_NO AND PONO = @tpssm11.PONO ";
			break;
		}
		cmd_tmmsm33_inq.SetCommandText(sqlstr);
		cmd_tmmsm33_inq.Parameters.Set("tpssm11.HEAT_NO", v_heat_no.Trim());
		cmd_tmmsm33_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString().Trim());

		v_count_prod = cmd_tmmsm33_inq.ExecuteScalar().ToInt32();

		if (v_count_prod > 0)
		{
			CFormattable arguments[] = { v_heat_no.Trim(), tpssm11["PONO"].ToString().Trim() };
			CMessageFormat::Format(s.msg, "熔炼号=[{0}],制造命令号[{1}]已经有切割产出实绩，不能回退。", arguments, 2);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//---------------------------------------------------------
		//条件校验3  已经有切割代表成分，不能回退
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT COUNT(1) FROM TQMTS29 A "
				"  WHERE A.HEAT_NO = @tpssm11.HEAT_NO ";
			break;
		}
		cmd_tqmts29_inq.SetCommandText(sqlstr);
		cmd_tqmts29_inq.Parameters.Set("tpssm11.HEAT_NO", v_heat_no.Trim());

		v_count_prod = cmd_tqmts29_inq.ExecuteScalar().ToInt32();

		if (v_count_prod > 0)
		{
			CFormattable arguments[] = { v_heat_no.Trim() };
			CMessageFormat::Format(s.msg, "熔炼号=[{0}]已经有代表成分,请联系质量部取消代表成分。", arguments, 1);
			//throw CApplicationException(-1, s.msg, log.Location);
		}

#pragma region 状态回退到到回转台之前，精炼结束
		//20130427 HYF SubstringNE
		if (tpssm12["AREA_ID"].ToDecimal() == 5)
		{
			tpssm99["RET_POS"] = "开浇前";
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0,1) < "5")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(),tpssm11["PONO"].ToString().Trim() };
				CMessageFormat::Format(s.msg, "炉号 = [{0}], 制造命令号[{1}]没有到达连铸工序不能回退。" , arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = " SELECT CHARGE_NO FROM TPSSM12 \
						    WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO  \
						      AND AREA_ID = @tpssm12.AREA_ID \
							  AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID",tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			curr_wp_no = cmd_tpssm12_inq.ExecuteScalar();

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = " SELECT PROC_NO,DEV_CODE FROM TPSSM12 \
						    WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO \
						      AND AREA_ID = 5 \
							  AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				cc_proc_no = cmd_tpssm12_inq.GetString(1);
				dev_code = cmd_tpssm12_inq.GetString(2);
			}
			cmd_tpssm12_inq.Close();

			//修改25表连铸处理号
			if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " UPDATE TPSSM25 \
							 						      SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0) \
														  						    WHERE CURR_PROC_NO = @cc_proc_no \
																												  AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				//cmd_tpssm25_inq.Parameters.Set("v_cc_proc",v_proc_no);
				cmd_tpssm25_inq.Parameters.Set("cc_proc_no", cc_proc_no);
				cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.ExecuteNonQuery();
				cmd_tpssm25_inq.Close();
			}
			//修改26表连铸浇次号
			if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
			{
				if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm11["TD_CHG_FLG"].ToDecimal() == 1)//
				{
					//这里取分割号不准，考虑到第一炉回炉后下一炉一般会设置成下一浇次的首炉，分割号没价值，暂时不改
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " SELECT MAX(CAST_DIV_NO) FROM TPSSM11 WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm11_inq.SetCommandText(sqlstr);
					cmd_tpssm11_inq.Parameters.Set("tpssm11.CAST_NO", cast_no);//往前退一个浇次号，取出对应的最大分割号
					cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cast_div_no = cmd_tpssm11_inq.ExecuteScalar();
					////Log::Trace("", __FUNCTION__, "前一个浇次号cast_no=[{0}]，最大分割号 = [{1}]更新到26表对应的记录里", cast_no, cast_div_no);

					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO = @cast_div_no, CAST_NO = @cast_no WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm26_upd.SetCommandText(sqlstr);
					cmd_tpssm26_upd.Parameters.Set("cast_div_no", cast_div_no);
					cmd_tpssm26_upd.Parameters.Set("cast_no", cast_no);
					cmd_tpssm26_upd.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
					cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm26_upd.ExecuteNonQuery();

				}
				else
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO =  CAST_DIV_NO-1 WHERE CC_MACH_NO = @tpssm11.CC_MACH_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm26_upd.SetCommandText(sqlstr);
					cmd_tpssm26_upd.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
					cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm26_upd.ExecuteNonQuery();
					////Log::Trace("", __FUNCTION__, "并非本浇次第一炉,则直接更新tpssm26--浇次分割号");
				}
			}


			tpssm10["PONO_STATUS"] = 20;
			tpssm10["PONO"] = tpssm11["PONO"];	
			tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm10.Update("PONO_STATUS","FACTORY_DIV,PONO");

			tpssm11["RUN_STATUS"] = "44";
			tpssm11["PONO_STATUS"] = 20;
			tpssm11["CURR_WP_NO"] = curr_wp_no-1;
			tpssm11.Update("PONO_STATUS,RUN_STATUS,CURR_WP_NO","FACTORY_DIV,PONO");

			tpssm12["PROC_NO"] = " ";
			tpssm12["START_TIME_REAL"] = " ";
			tpssm12["END_TIME_REAL"] = " ";
			tpssm12["ARRIVE_REAL_TIME"] = " ";
			tpssm12["LEAVE_REAL_TIME"] = " ";
			tpssm12["AREA_ID"] = 5;
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");

			tpssm12.Update("START_TIME,END_TIME,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","FACTORY_DIV,SM_PLAN_NO,AREA_ID");			
		}
#pragma endregion

#pragma region 状态回退到精炼包到前
		//20130427 HYF SubstringNE
		if (tpssm12["AREA_ID"].ToDecimal() == 4)
		{
			tpssm99["RET_POS"] = "精炼前";
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1)<"4")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(), tpssm11["PONO"].ToString().Trim() };
				CMessageFormat::Format(s.msg, "炉号=[{0}],制造命令号[{1}]没有到达精炼工序不能回退。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			/*if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1)>"4")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, "炉号=[{0}]已经到达连铸区域不能回退，请先回退连铸。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}*/
			//---------------------------------------------------------
			//更新表 10 11 12表信息
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT CHARGE_NO FROM TPSSM12  "
					"   WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO   "
					"    AND AREA_ID = @tpssm12.AREA_ID  "
					"  AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"  AND START_TIME_REAL <> ' ' "
					"  ORDER BY CHARGE_NO DESC ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				curr_wp_no = cmd_tpssm12_inq.GetDecimal(1);
			}

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT COUNT(1) FROM TPSSM12  "
					"   WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO   "
					"    AND AREA_ID = @tpssm12.AREA_ID  "
					"  AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"  AND START_TIME_REAL <> ' ' ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			refine_count = cmd_tpssm12_inq.ExecuteScalar();

			
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT PROC_NO,DEV_CODE FROM TPSSM12 "
					"      WHERE SM_PLAN_NO = @tpssm11.SM_PLAN_NO  "
					"     AND AREA_ID = 4   "
					"	  AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"	  AND CHARGE_NO = @curr_wp_no ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("curr_wp_no", curr_wp_no);
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				lf_proc_no = cmd_tpssm12_inq.GetString(1);
				dev_code = cmd_tpssm12_inq.GetString(2);
			}
			cmd_tpssm12_inq.Close();

			tpssmd1.Reset();
			tpssm25.Reset();
			tpssmd1["DEV_CODE"] = dev_code;
			tpssmd1["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			if (tpssmd1.QueryCount("DEV_CODE,FACTORY_DIV") != 1)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(), tpssm11["PONO"].ToString().Trim() };
				CMessageFormat::Format(s.msg, "炉号=[{0}],制造命令号[{1}]设备静态数据有误，不能回退。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				tpssmd1.Query("DEV_CODE,FACTORY_DIV");
				tpssm25["STATION_ID"] = tpssmd1["STATION_ID"];
				tpssm25["STATION_NO"] = tpssmd1["STATION_NO"];
				tpssm25["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				if (tpssm25.QueryCount("FACTORY_DIV,STATION_NO,STATION_ID") == 1)
				{
					tpssm25.Query("FACTORY_DIV,STATION_NO,STATION_ID");
				}
				else
				{
					CFormattable arguments[] = { dev_code };
					CMessageFormat::Format(s.msg, "设备号=[{0}]处理号数据有误，不能回退。", arguments, 1); 
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			//到达精炼工序的更新处理号
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1) > "3")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " UPDATE TPSSM25 SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0)  WHERE CURR_PROC_NO = @lf_proc_no AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				cmd_tpssm25_inq.Parameters.Set("lf_proc_no", lf_proc_no);
				cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.ExecuteNonQuery();
				cmd_tpssm25_inq.Close();
			}
			//到达连铸工序的更新处理号、浇次号
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1) > "4")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT PROC_NO,DEV_CODE FROM TPSSM12 WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO AND AREA_ID = 5 AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				if (cmd_tpssm12_inq.Read())
				{
					cc_proc_no = cmd_tpssm12_inq.GetString(1);
				}
				cmd_tpssm12_inq.Close();
				//修改25表连铸处理号
				if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = " UPDATE TPSSM25 SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0) WHERE CURR_PROC_NO = @cc_proc_no AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm25_inq.SetCommandText(sqlstr);
					//cmd_tpssm25_inq.Parameters.Set("v_cc_proc",v_proc_no);
					cmd_tpssm25_inq.Parameters.Set("cc_proc_no", cc_proc_no);
					cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm25_inq.ExecuteNonQuery();
					cmd_tpssm25_inq.Close();
				}
				if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm11["TD_CHG_FLG"].ToDecimal() == 1)// 
					{
						//这里取分割号不准，考虑到第一炉回炉后下一炉一般会设置成下一浇次的首炉，分割号没价值，暂时不改
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:


							sqlstr = " SELECT MAX(CAST_DIV_NO) FROM TPSSM11 WHERE CAST_NO = @tpssm11.CAST_NO  AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
							break;
						}
						cmd_tpssm11_inq.SetCommandText(sqlstr);
						cmd_tpssm11_inq.Parameters.Set("tpssm11.CAST_NO", cast_no);//往前退一个浇次号，取出对应的最大分割号
						cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						cast_div_no = cmd_tpssm11_inq.ExecuteScalar();
						Log::Trace("", __FUNCTION__, "前一个浇次号cast_no=[{0}]，最大分割号 = [{1}]更新到26表对应的记录里", cast_no, cast_div_no);

						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:


							sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO = @cast_div_no, CAST_NO = @cast_no WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
							break;
						}
						cmd_tpssm26_upd.SetCommandText(sqlstr);
						cmd_tpssm26_upd.Parameters.Set("cast_div_no", cast_div_no);
						cmd_tpssm26_upd.Parameters.Set("cast_no", cast_no);
						cmd_tpssm26_upd.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
						cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						cmd_tpssm26_upd.ExecuteNonQuery();

					}
					else
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:


							sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO   =  CAST_DIV_NO-1 WHERE CC_MACH_NO = @tpssm11.CC_MACH_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
							break;
						}
						cmd_tpssm26_upd.SetCommandText(sqlstr);
						cmd_tpssm26_upd.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
						cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						cmd_tpssm26_upd.ExecuteNonQuery();
						Log::Trace("", __FUNCTION__, "并非本浇次第一炉,则直接更新tpssm26--浇次分割号");
					}
				}
			}
			

			curr_wp_no = curr_wp_no - 1;

			tpssm10["PONO_STATUS"] = 20;
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm10.Update("PONO_STATUS", "PONO, FACTORY_DIV");

			if (refine_count == 1)
			{
				tpssm11["RUN_STATUS"] = "36";
			}
			else
			{
				tpssm11["RUN_STATUS"] = "44";
			}

			tpssm11["PONO_STATUS"] = 20;
			tpssm11["CURR_WP_NO"] = curr_wp_no;
			tpssm11.Update("PONO_STATUS,RUN_STATUS,CURR_WP_NO", "PONO, FACTORY_DIV");

			tpssm12["PROC_NO"] = " ";
			tpssm12["START_TIME_REAL"] = " ";
			tpssm12["END_TIME_REAL"] = " ";
			tpssm12["ARRIVE_REAL_TIME"] = " ";
			tpssm12["LEAVE_REAL_TIME"] = " ";
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm12["CHARGE_NO"] = curr_wp_no + 1;

			tpssm12["AREA_ID"] = 4;

			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			tpssm12_2["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID,CHARGE_NO");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");

			tpssm12.Update("START_TIME,END_TIME,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME", "FACTORY_DIV, SM_PLAN_NO, AREA_ID, CHARGE_NO");

			tpssm12["AREA_ID"] = 5;
			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			//tpssm12_2["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");
			tpssm12.Update("START_TIME,END_TIME,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME", "FACTORY_DIV, SM_PLAN_NO, AREA_ID");
		}
#pragma endregion


#pragma region  状态回退到脱碳转炉吹炼前
		//状态回退到吹炼前
		if (tpssm12["AREA_ID"].ToDecimal() == 3)
		{	
			tpssm99["RET_POS"] = "吹炼前";
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0,1) < "3")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(),tpssm11["PONO"].ToString().Trim() };
				CMessageFormat::Format(s.msg, "炉号 = [{0}], 制造命令号[{1}]没有到达脱碳工序不能回退。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = "SELECT CHARGE_NO,DEV_CODE FROM TPSSM12 \
						   WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO \
						     AND AREA_ID = @tpssm12.AREA_ID \
							 AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID",tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if (cmd_tpssm12_inq.Read())
			{
				curr_wp_no = cmd_tpssm12_inq.GetDecimal(1);
				dev_code = cmd_tpssm12_inq.GetString(2);
			}

			tpssmd1.Reset();
			tpssm25.Reset();
			tpssmd1["DEV_CODE"] = dev_code;
			tpssmd1["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
			Log::Trace("", __FUNCTION__, "dev_code =[{0}]", tpssmd1["DEV_CODE"].ToString());
			Log::Trace("", __FUNCTION__, "FACTORY_DIV =[{0}]", tpssmd1["FACTORY_DIV"].ToString());
			if (tpssmd1.QueryCount("DEV_CODE,FACTORY_DIV,AREA_ID") != 1)
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(), tpssm11["PONO"].ToString().Trim() };
				CMessageFormat::Format(s.msg, "炉号=[{0}],制造命令号[{1}]设备静态数据有误，不能回退。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				tpssmd1.Query("DEV_CODE,FACTORY_DIV,AREA_ID");
				tpssm25["STATION_ID"] = tpssmd1["STATION_ID"];
				tpssm25["STATION_NO"] = tpssmd1["STATION_NO"];
				tpssm25["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				if (tpssm25.QueryCount("FACTORY_DIV,STATION_NO,STATION_ID") == 1)
				{
					tpssm25.Query("FACTORY_DIV,STATION_NO,STATION_ID");
				}
				else
				{
					CFormattable arguments[] = { dev_code };
					CMessageFormat::Format(s.msg, "设备号=[{0}]处理号数据有误，不能回退。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			//到达转炉工序的更新熔炼号
			if (tpssm11["RUN_STATUS"].ToString().Trim() >= "31")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = "UPDATE TPSSM25 SET CURR_PROC_NO   =  substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0) WHERE CURR_PROC_NO   = @tpssm11.HEAT_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				//cmd_tpssm25_inq.Parameters.Set("v_cc_proc",v_proc_no);
				cmd_tpssm25_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.ExecuteNonQuery();
			}
			//到达精炼工序的更新处理号
			if (tpssm11["RUN_STATUS"].ToString().Trim() >= "41")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT PROC_NO FROM TPSSM12  "
						"   WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO   "
						"    AND AREA_ID = 4 "
						"  AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
						"  AND START_TIME_REAL <> ' ' "
						"  ORDER BY CHARGE_NO DESC ";
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				while(cmd_tpssm12_inq.Read())
				{
					lf_proc_no = cmd_tpssm12_inq.GetString(1);
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " UPDATE TPSSM25 SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0)  WHERE CURR_PROC_NO = @lf_proc_no AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm25_inq.SetCommandText(sqlstr);
					cmd_tpssm25_inq.Parameters.Set("lf_proc_no", lf_proc_no);
					cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm25_inq.ExecuteNonQuery();
					cmd_tpssm25_inq.Close();
				}
				cmd_tpssm12_inq.Close();
			}
			//到达连铸工序的更新处理号、浇次号
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1) > "4")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT PROC_NO,DEV_CODE FROM TPSSM12 WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO AND AREA_ID = 5 AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				if (cmd_tpssm12_inq.Read())
				{
					cc_proc_no = cmd_tpssm12_inq.GetString(1);
				}
				cmd_tpssm12_inq.Close();
				//修改25表连铸处理号
				if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = " UPDATE TPSSM25 SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0) WHERE CURR_PROC_NO = @cc_proc_no AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm25_inq.SetCommandText(sqlstr);
					//cmd_tpssm25_inq.Parameters.Set("v_cc_proc",v_proc_no);
					cmd_tpssm25_inq.Parameters.Set("cc_proc_no", cc_proc_no);
					cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm25_inq.ExecuteNonQuery();
					cmd_tpssm25_inq.Close();
				}
				if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm11["TD_CHG_FLG"].ToDecimal() == 1)//重引锭或者快换中包
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:


							sqlstr = " SELECT MAX(CAST_DIV_NO) FROM TPSSM11 WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
							break;
						}
						cmd_tpssm11_inq.SetCommandText(sqlstr);
						cmd_tpssm11_inq.Parameters.Set("tpssm11.CAST_NO", cast_no);
						cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						cast_div_no = cmd_tpssm11_inq.ExecuteScalar();


						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:


							sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO = @cast_div_no, CAST_NO  = @cast_no WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
							break;
						}
						cmd_tpssm26_upd.SetCommandText(sqlstr);
						cmd_tpssm26_upd.Parameters.Set("cast_div_no", cast_div_no);
						cmd_tpssm26_upd.Parameters.Set("cast_no", cast_no);
						cmd_tpssm26_upd.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
						cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						cmd_tpssm26_upd.ExecuteNonQuery();

					}
					else
					{

						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:


							sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO   =  CAST_DIV_NO-1 WHERE CC_MACH_NO = @tpssm11.CC_MACH_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
							break;
						}
						cmd_tpssm26_upd.SetCommandText(sqlstr);
						cmd_tpssm26_upd.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
						cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
						cmd_tpssm26_upd.ExecuteNonQuery();

						////Log::Trace("", __FUNCTION__, "并非本浇次第一炉,则直接更新tpssm26--浇次分割号");
					}
				}
			}

			
			tpssm10["PONO_STATUS"] = 20;
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm10.Update("PONO_STATUS","PONO, FACTORY_DIV");

			tpssm11["HEAT_NO"] = " ";
			tpssm11["RUN_STATUS"] = "30";
			tpssm11["PONO_STATUS"] = 20;
			tpssm11["CURR_WP_NO"] = curr_wp_no-1;
			tpssm11.Update("HEAT_NO,PONO_STATUS,RUN_STATUS,CURR_WP_NO","PONO, FACTORY_DIV");

			tpssm12["PROC_NO"] = " ";
			tpssm12["HEAT_NO"] = " ";
			tpssm12["START_TIME_REAL"] = " ";
			tpssm12["END_TIME_REAL"] = " ";
			tpssm12["ARRIVE_REAL_TIME"] = " ";
			tpssm12["LEAVE_REAL_TIME"] = " ";
			tpssm12["AREA_ID"] = 3;
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			//tpssm12_2["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");

			tpssm12.Update("START_TIME,END_TIME,HEAT_NO, PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","SM_PLAN_NO,AREA_ID,FACTORY_DIV");
			
			tpssm12["AREA_ID"] = 4;
			tpssm12.Update("HEAT_NO, PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","SM_PLAN_NO,AREA_ID,FACTORY_DIV");
			
			tpssm12["AREA_ID"] = 5;

			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			//tpssm12_2["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");

			tpssm12.Update("START_TIME,END_TIME,HEAT_NO, PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","SM_PLAN_NO,AREA_ID,FACTORY_DIV");
		}
#pragma endregion  
#pragma region  状态回退到未生产
		
		if (tpssm12["AREA_ID"].ToDecimal() == 0)
		{			
			tpssm99["RET_POS"] = "未生产";
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0,2) == "00")
			{
				CFormattable arguments[] = { tpssm11["HEAT_NO"].ToString(), tpssm11["PONO"].ToString().Trim() };
				CMessageFormat::Format(s.msg, "炉号=[{0}],制造命令号[{1}]没有开始生产不能回退。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//到达转炉工序的更新熔炼号
			if (tpssm11["RUN_STATUS"].ToString() >= "31")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					//原有产品化编码是8位，且编码格式不一样
					/*sqlstr = "UPDATE TPSSM25 \
					SET CURR_PROC_NO   =  substr(CURR_PROC_NO, 1, 2) || lpad(to_char((to_number(substr(CURR_PROC_NO,3,6)-1))),6,0) \
					WHERE CURR_PROC_NO   = @tpssm11.HEAT_NO ";*/
					sqlstr = "UPDATE TPSSM25 SET CURR_PROC_NO   =  substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0) WHERE CURR_PROC_NO   = @tpssm11.HEAT_NO AND FACTORY_DIV    = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm25_inq.SetCommandText(sqlstr);
				//cmd_tpssm25_inq.Parameters.Set("v_cc_proc",v_proc_no);
				cmd_tpssm25_inq.Parameters.Set("tpssm11.HEAT_NO", tpssm11["HEAT_NO"].ToString());
				cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm25_inq.ExecuteNonQuery();
			}
			//到达精炼工序的更新处理号
			if (tpssm11["RUN_STATUS"].ToString().Trim() >= "41")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT PROC_NO FROM TPSSM12  "
						"   WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO   "
						"    AND AREA_ID = 4 "
						"  AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
						"  AND START_TIME_REAL <> ' ' "
						"  ORDER BY CHARGE_NO DESC ";
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID", tpssm12["AREA_ID"].ToDecimal());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				while (cmd_tpssm12_inq.Read())
				{
					lf_proc_no = cmd_tpssm12_inq.GetString(1);
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:


						sqlstr = " UPDATE TPSSM25 SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0)  WHERE CURR_PROC_NO = @lf_proc_no AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm25_inq.SetCommandText(sqlstr);
					cmd_tpssm25_inq.Parameters.Set("lf_proc_no", lf_proc_no);
					cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm25_inq.ExecuteNonQuery();
					cmd_tpssm25_inq.Close();
				}
				cmd_tpssm12_inq.Close();
			}
			//到达连铸工序的更新处理号、浇次号
			if (tpssm11["RUN_STATUS"].ToString().SubstringNE(0, 1) > "4")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT PROC_NO,DEV_CODE FROM TPSSM12 WHERE SM_PLAN_NO    = @tpssm11.SM_PLAN_NO AND AREA_ID = 5 AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.ExecuteReader();
				if (cmd_tpssm12_inq.Read())
				{
					cc_proc_no = cmd_tpssm12_inq.GetString(1);
				}
				cmd_tpssm12_inq.Close();
				//修改25表连铸处理号
				if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = " UPDATE TPSSM25 SET CURR_PROC_NO   = substr(CURR_PROC_NO, 1, 3) || lpad(to_char((to_number(substr(CURR_PROC_NO,4,5)-1))),5,0) WHERE CURR_PROC_NO = @cc_proc_no AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
						break;
					}
					cmd_tpssm25_inq.SetCommandText(sqlstr);
					cmd_tpssm25_inq.Parameters.Set("cc_proc_no", cc_proc_no);
					cmd_tpssm25_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm25_inq.ExecuteNonQuery();
					cmd_tpssm25_inq.Close();
				}
				//到达连铸工序的修改26表连铸浇次号
				if (cast_no_26.Trim() == tpssm11["CAST_NO"].ToString() && cast_div_no_26 == tpssm11["CAST_DIV_NO"].ToDecimal())
				{
					if (tpssm11["RUN_STATUS"].ToString() >= "52")
					{
						if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm11["TD_CHG_FLG"].ToDecimal() == 1)//重引锭或者快换中包,浇次号都要重新计算
						{
							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:	        // MS SQL Server数据库
							case DB_KIND_ORACLE:	        // Oracle 数据库
							default:


								sqlstr = " SELECT MAX(CAST_DIV_NO) FROM TPSSM11 WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
								break;
							}
							cmd_tpssm11_inq.SetCommandText(sqlstr);
							cmd_tpssm11_inq.Parameters.Set("tpssm11.CAST_NO", cast_no);
							cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
							cast_div_no = cmd_tpssm11_inq.ExecuteScalar();


							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:	        // MS SQL Server数据库
							case DB_KIND_ORACLE:	        // Oracle 数据库
							default:


								sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO = @cast_div_no, CAST_NO  = @cast_no WHERE CAST_NO = @tpssm11.CAST_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
								break;
							}
							cmd_tpssm26_upd.SetCommandText(sqlstr);
							cmd_tpssm26_upd.Parameters.Set("cast_div_no", cast_div_no);
							cmd_tpssm26_upd.Parameters.Set("cast_no", cast_no);
							cmd_tpssm26_upd.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
							cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
							cmd_tpssm26_upd.ExecuteNonQuery();

						}
						else
						{
							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:	        // MS SQL Server数据库
							case DB_KIND_ORACLE:	        // Oracle 数据库
							default:


								sqlstr = " UPDATE TPSSM26 SET CAST_DIV_NO   =  CAST_DIV_NO-1 WHERE CC_MACH_NO = @tpssm11.CC_MACH_NO AND FACTORY_DIV = @tpssm11.FACTORY_DIV ";
								break;
							}
							cmd_tpssm26_upd.SetCommandText(sqlstr);
							cmd_tpssm26_upd.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
							cmd_tpssm26_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
							cmd_tpssm26_upd.ExecuteNonQuery();
							////Log::Trace("", __FUNCTION__, "并非本浇次第一炉,则直接更新tpssm26--浇次分割号");
						}
					}//到达连铸工序的修改26表连铸浇次号
				}
			}
						
			tpssm10["PONO_STATUS"] = 19;
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm10.Update("PONO_STATUS","FACTORY_DIV,PONO");

			tpssm11["HEAT_NO"] = " ";
			tpssm11["RUN_STATUS"] = "00";
			tpssm11["PONO_STATUS"] = 19;
			tpssm11["CURR_WP_NO"] = 0;
			tpssm11.Update("HEAT_NO,PONO_STATUS,RUN_STATUS,CURR_WP_NO","FACTORY_DIV,PONO");

			tpssm12["HEAT_NO"] = " ";
			tpssm12["PROC_NO"] = " ";
			tpssm12["START_TIME_REAL"] = " ";
			tpssm12["END_TIME_REAL"] = " ";
			tpssm12["ARRIVE_REAL_TIME"] = " ";
			tpssm12["LEAVE_REAL_TIME"] = " ";

			tpssm12["AREA_ID"] = 3;
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			//tpssm12_2["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");

			tpssm12.Update("START_TIME,END_TIME,HEAT_NO,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			
			tpssm12["AREA_ID"] = 2;
			tpssm12.Update("HEAT_NO,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","FACTORY_DIV,SM_PLAN_NO,AREA_ID");

			tpssm12["AREA_ID"] = 4;
			tpssm12.Update("HEAT_NO,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			
			tpssm12["AREA_ID"] = 5;

			tpssm12_2.Reset();
			tpssm12_2["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssm12_2["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
			tpssm12_2["AREA_ID"] = tpssm12["AREA_ID"];
			//tpssm12_2["CHARGE_NO"] = tpssm12["CHARGE_NO"];
			tpssm12_2.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm12["START_TIME"] = tpssm12_2["START_TIME"];
			tpssm12["END_TIME"] = (CDateTime::Parse(tpssm12_2["START_TIME"]).AddMinutes(tpssm12_2["PROC_TIME"].ToDouble())).ToString("yyyyMMddHHmmss");

			tpssm12.Update("START_TIME,END_TIME,HEAT_NO,PROC_NO,START_TIME_REAL,END_TIME_REAL,ARRIVE_REAL_TIME,LEAVE_REAL_TIME","FACTORY_DIV,SM_PLAN_NO,AREA_ID");		
		}

		//dclian---add---2015-11-16------
		////Log::Trace("", __FUNCTION__, "状态回退");
		//tpssm11["HEAT_NO"] = tpssm99["HEAT_NO"];
		//tpssm11.Query("HEAT_NO");
		tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = tpssm11["PONO"];
		
		if (area_id == 0)
		{
			tpssm99["EVENT_ID"] = "D0"; //D0	状态回退生产前
		}
		else if (area_id == 3)
		{
			tpssm99["EVENT_ID"] = "D3"; //D3	状态回退吹炼前
		}
		else if (area_id == 4)
		{
			tpssm99["EVENT_ID"] = "D4"; //D5	状态回退开浇前
		}
		else if (area_id == 5)
		{
			tpssm99["EVENT_ID"] = "D5"; //D5	状态回退开浇前
		}
		tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		tpssm99["VALID_FLAG"] = "1";
		in_pssm99trace.Tables[0].Clone(tpssm99);
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

		////Log::Trace("", __FUNCTION__, "记录失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		//记录编入计划成功的履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
#pragma endregion  
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
	cmd_tpssm11_inq.Close();
	return doFlag;

}
