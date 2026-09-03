/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-13
Version:  3.1.0
Description: 甘特图出钢计划浇次号计算
Update：  2014-11-27  xuwen  优化
**************************************************/

/* C 的标准头文件部分 */ 
#include "stdafx.h"


//#include "tpssm12.h"
int f_pssm_castlot_upd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划CAST号计算--扩位：年末（1）+铸机号（1）+ 4位流水  modified by dclian 2016-01-12
/// <para>读取各连铸机下的当前CAST号；
/// <para>确定新增计划的基准CAST后,按浇铸顺,生成CAST号.
/// </para>
/// <para>数据库表：TPSSM11(炼钢出钢计划主表)</para>
/// <para>主调用函数：pssm18_save</para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码</param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	/* 程序用变量 */
	int  doFlag = 0;
	int  blkseq=0;
	int  ret = 0;
	CString datetime="";            /* 记录创建时刻 */

	//业务用变量
	CDecimal	   dummy=0;
	CString   cast_no="";               /* 计算CAST号用 */
	CDecimal   cast_div_no = 0;          /* CAST分割号 */	
	CDecimal    cast_stream_no=0;           /* cast流水号 */	
	CString sqlstr = "";

	CString special_flag = "";

	CDbCommand cmd_tpssm26_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm11_inq2(conn);

	CModel tpssm26("TPSSM26");
	CModel tpssm11("TPSSM11");
	CModel tpssm15("TPSSM15");
	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//----------------------------------------------------------------------------------
		//获得输入参数
		//1.读取编入计划的PONO
		blkseq = bcls_rec->Tables.IndexOf("PLAN"); //浇铸信息
		if (blkseq < 0) 
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm21_cast_cre_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		tpssm26["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		if (bcls_rec->Tables[blkseq].Columns.Contains("SPECIAL_FLAG"))
		{
			special_flag = bcls_rec->Tables[blkseq].Rows[0]["SPECIAL_FLAG"].ToString();
		}
		////Log::Info("", __FUNCTION__, "factory_div=[{0}]", (const char*)tpssm26["FACTORY_DIV"].ToString());

		tpssm26["REC_CREATOR"] = s.userid;
		tpssm26["REC_CREATE_TIME"] = datetime;
		
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句 -- ，查找所有铸机
			sqlstr =  " SELECT STATION_NO,DEV_CODE FROM TPSSMD1 "
					  " WHERE FACTORY_DIV = @tpssm26.FACTORY_DIV "
					  " AND AREA_ID = 5 "
					 // " AND STATION_ID = 'C' "
					  " ORDER BY STATION_NO ASC";
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssm26.FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while (cmd_tpssmd1_inq.Read())
		{
			tpssm26["CC_MACH_NO"] = cmd_tpssmd1_inq.GetString(1).Trim();
			tpssm26["DEV_CODE"] = cmd_tpssmd1_inq.GetString(2).Trim();
			dummy = 0;
			dummy = tpssm26.QueryCount("CC_MACH_NO,FACTORY_DIV,DEV_CODE");  //遍历铸机号，产生浇次流水号 -> 连铸机公共条件表
			if (dummy == 0)
			{
				//扩位：年末（1）+铸机号（1）+ 4位流水
				tpssm26["CAST_NO"] = (const char*)datetime.Trim().SubstringNE(3, 1)+tpssm26["CC_MACH_NO"].ToString() + "0000";
				tpssm26["CAST_DIV_NO"] = 1;//从第1开始			
				////Log::Info("", __FUNCTION__, "CAST_NO=[{0}]", (const char*)tpssm26["CAST_NO"].ToString());
				tpssm26.Insert();
			}
		}
		cmd_tpssmd1_inq.Close();

		if (special_flag != "1")//正常逻辑
		{

			//循环读取TPSSM26表中的计划编制数据
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"SELECT * FROM TPSSM26 "
					" WHERE FACTORY_DIV = @tpssm26.factory_div "
					" ORDER BY CC_MACH_NO ASC"
					);
				break;
			}
			cmd_tpssm26_inq.SetCommandText(sqlstr);
			cmd_tpssm26_inq.Parameters.Set("tpssm26.factory_div", tpssm26["FACTORY_DIV"].ToString());
			cmd_tpssm26_inq.ExecuteReader();
			while (cmd_tpssm26_inq.Read()) //遍历连铸机公共条件表：按铸机进行业务处理
			{
				cmd_tpssm26_inq.Fetch(tpssm26);
				tpssm26.TrimOrBlank();

				cast_no = tpssm26["CAST_NO"];
				cast_div_no = tpssm26["CAST_DIV_NO"];

				////Log::Info("", __FUNCTION__, "起始cast_no=[{0}], cast_div_no=[{1}]", (const char*)cast_no, cast_div_no.ToInt32() );


				//按照甘特图确定的浇铸开始时刻，依次读取pono记录，并赋CAST号
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						"SELECT a.RESTRAND_FLG,a.SM_PLAN_NO,a.TD_CHG_FLG  FROM TPSSM11 a JOIN  TPSSM12 b "
						"  ON (a.FACTORY_DIV = b.FACTORY_DIV AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 5)"
						" WHERE a.run_status <= '52' "
						"   AND a.FACTORY_DIV = @tpssm26.FACTORY_DIV "
						"   AND a.CC_MACH_NO = @tpssm26.CC_MACH_NO "
						"   AND (b.START_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.START_TIME_REAL = ' ') "
						" ORDER BY trim(b.START_TIME_REAL) NULLS LAST, b.START_TIME ASC "
						);
					/*sqlstr = CString(
						"SELECT a.RESTRAND_FLG,a.SM_PLAN_NO,a.TD_CHG_FLG  FROM TPSSM11 a, TPSSM10 b  "
						" WHERE a.run_status < '52' "
						" and a.pono = b.pono(+) "
						"   AND a.FACTORY_DIV = @tpssm26.FACTORY_DIV "
						"   AND a.CC_MACH_NO = @tpssm26.CC_MACH_NO "
						" ORDER BY b.CC_SEQ ASC "
						);*/
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm26.FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm26.CC_MACH_NO", tpssm26["CC_MACH_NO"].ToString().Trim());
				cmd_tpssm12_inq.ExecuteReader();
				while (cmd_tpssm12_inq.Read())
				{
					tpssm11["RESTRAND_FLG"] = cmd_tpssm12_inq.GetString(1).Trim();
					tpssm11["SM_PLAN_NO"] = cmd_tpssm12_inq.GetString(2).Trim();
					tpssm11["TD_CHG_FLG"] = cmd_tpssm12_inq.GetDecimal(3);
					////Log::Trace("", __FUNCTION__, "快换中包tpssm11["TD_CHG_FLG"] =[{0}]", tpssm11["TD_CHG_FLG"].ToDecimal());
					////Log::Trace("", __FUNCTION__, "重引锭tpssm11["RESTRAND_FLG"] =[{0}]", tpssm11["RESTRAND_FLG"].ToString());
					//柳钢定制化需求：因生产节奏快，所以需要在快换中包时重新计算浇次号
					if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm11["TD_CHG_FLG"].ToDecimal() == 1) //重引锭 // 或者快换中包
					{
						cast_stream_no = cast_stream_no.Parse(cast_no.Substring(2));//流水号--在前面扩1位后从第三位开始取，原本cast_no.Substring(1)
						cast_stream_no = cast_stream_no + 1;

						////Log::Trace("", __FUNCTION__, "计算cast_stream_no浇次号=[{0}]", cast_stream_no.ToInt32());

						cast_no = cast_no.Format("%s%s%.4d", (const char*)datetime.Trim().SubstringNE(3, 1), (const char*)tpssm26["CC_MACH_NO"].ToString().SubstringNE(0, 1), cast_stream_no.ToInt32());
						cast_div_no = 1;

					}
					else //不重引锭--连浇, cast_no不变
					{
						cast_div_no = cast_div_no + 1;
					}

					//~~~~~将cast_no, cast_div_no写入记录~~~~~
					//Log::Trace("", __FUNCTION__, "计算浇次号后cast_no=[{0}], cast_div_no=[{1}]", (const char*)cast_no, cast_div_no.ToInt32());


					tpssm11["CAST_NO"] = cast_no;
					tpssm11["CAST_DIV_NO"] = cast_div_no;
					tpssm11["FACTORY_DIV"] = tpssm26["FACTORY_DIV"];
					sqlstr = "tpssm11.Update(CAST_NO)";
					tpssm11.Update("CAST_NO,CAST_DIV_NO", "FACTORY_DIV,SM_PLAN_NO");


				}
				cmd_tpssm12_inq.Close();

				//读取TPSSM11表记录，按cast_no汇总
				//switch (conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:         // MS SQL Server数据库
				//case DB_KIND_ORACLE:        // Oracle 数据库
				//default:  // 所有数据库适用，通用SQL语句
				//	sqlstr = CString("SELECT FACTORY_DIV,SM_PLAN_NO, CAST_NO FROM TPSSM11 WHERE FACTORY_DIV = @FACTORY_DIV AND RUN_STATUS < '52' ");
				//	break;
				//}
				//cmd_tpssm11_inq.SetCommandText(sqlstr);
				//cmd_tpssm11_inq.Parameters.Set("FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
				//cmd_tpssm11_inq.ExecuteReader();
				//while (cmd_tpssm11_inq.Read())
				//{
				//	tpssm11["FACTORY_DIV"] = cmd_tpssm11_inq.GetString(1);
				//	tpssm11["SM_PLAN_NO"] = cmd_tpssm11_inq.GetString(2);
				//	tpssm11["CAST_NO"] = cmd_tpssm11_inq.GetString(3);

				//	sqlstr = "SELECT MAX(CAST_DIV_NO) FROM TPSSM11 WHERE CAST_NO = @CAST_NO  AND FACTORY_DIV = @FACTORY_DIV ";

				//	cmd_tpssm11_inq2.SetCommandText(sqlstr);
				//	cmd_tpssm11_inq2.Parameters.Set("CAST_NO", tpssm11["CAST_NO"].ToString());
				//	cmd_tpssm11_inq2.Parameters.Set("FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
				//	dummy = cmd_tpssm11_inq2.ExecuteScalar();

				//	/*dummy = tpssm11.QueryCount("FACTORY_DIV,CAST_NO");
				//	////Log::Trace("", __FUNCTION__, "tpssm11.QueryCount(CAST_NO)=[{0}]", dummy);*/

				//	//更新CAST内炉数
				//	tpssm11["CAST_PONO_SUM"] = dummy;
				//	sqlstr = "tpssm11.Update(CAST_PONO_SUM)";
				//	tpssm11.Update("CAST_PONO_SUM", "FACTORY_DIV,SM_PLAN_NO");
				//}
				//cmd_tpssm11_inq.Close();
			}
			cmd_tpssm26_inq.Close();

			sqlstr = CString(
				" UPDATE								  "
				" TPSSM11								  "
				" SET									  "
				" CAST_PONO_SUM = (						  "
				" SELECT								  "
				" MAX_DIV								  "
				" FROM									  "
				" (										  "
				" SELECT								  "
				" max(CAST_DIV_NO) AS MAX_DIV, CAST_NO	  "
				" FROM									  "
				" tpssm11								  "
				" WHERE									  "
				" CAST_NO <> ' '						  "
				" GROUP BY								  "
				" CAST_NO) A							  "
				" WHERE									  "
				" A.CAST_NO = TPSSM11.CAST_NO)			  "
				" WHERE									  "
				" CAST_NO <> ' '						  "
				" AND RUN_STATUS < '52'					  "
				);
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.ExecuteNonQuery();
			cmd_tpssm11_inq.Close();


			ret = f_pssm_castlot_upd2(bcls_rec, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		else//日平衡部分逻辑
		{
			//循环读取TPSSM26表中的计划编制数据
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"SELECT * FROM TPSSM26 "
					" WHERE FACTORY_DIV = @tpssm26.factory_div "
					" ORDER BY CC_MACH_NO ASC"
					);
				break;
			}
			cmd_tpssm26_inq.SetCommandText(sqlstr);
			cmd_tpssm26_inq.Parameters.Set("tpssm26.factory_div", tpssm26["FACTORY_DIV"].ToString());
			cmd_tpssm26_inq.ExecuteReader();
			while (cmd_tpssm26_inq.Read()) //遍历连铸机公共条件表：按铸机进行业务处理
			{
				cmd_tpssm26_inq.Fetch(tpssm26);
				tpssm26.TrimOrBlank();

				cast_no = tpssm26["CAST_NO"];
				cast_div_no = tpssm26["CAST_DIV_NO"];

				////Log::Info("", __FUNCTION__, "起始cast_no=[{0}], cast_div_no=[{1}]", (const char*)cast_no, cast_div_no.ToInt32() );


				//按照甘特图确定的浇铸开始时刻，依次读取pono记录，并赋CAST号
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT * "
						" FROM(SELECT a.RESTRAND_FLG, a.SM_PLAN_NO, a.TD_CHG_FLG, b.START_TIME, '1' AS table_seq "
						" FROM TPSSM11 a "
						" JOIN TPSSM12 b "
						" ON(a.FACTORY_DIV = b.FACTORY_DIV AND "
						" a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 5) "
						" WHERE a.run_status < '52' "
						" AND a.FACTORY_DIV = @tpssm26.FACTORY_DIV "
						" AND a.CC_MACH_NO = @tpssm26.CC_MACH_NO "
						" UNION ALL "
						" SELECT c.RESTRAND_FLG, c.SM_PLAN_NO, c.TD_CHG_FLG, d.START_TIME, '2' AS table_seq "
						" FROM TPSSM15 c "
						" JOIN TPSSM16 d "
						" ON(c.FACTORY_DIV = d.FACTORY_DIV AND "
						" c.SM_PLAN_NO = d.SM_PLAN_NO AND d.AREA_ID = 5) "
						" WHERE c.run_status < '52' "
						" AND c.FACTORY_DIV = @tpssm26.FACTORY_DIV " 
						" AND c.CC_MACH_NO = @tpssm26.CC_MACH_NO "
						" ) total "
						" ORDER BY table_seq asc, total.START_TIME ASC "
						);
					/*sqlstr = CString(
					"SELECT a.RESTRAND_FLG,a.SM_PLAN_NO,a.TD_CHG_FLG  FROM TPSSM11 a, TPSSM10 b  "
					" WHERE a.run_status < '52' "
					" and a.pono = b.pono(+) "
					"   AND a.FACTORY_DIV = @tpssm26.FACTORY_DIV "
					"   AND a.CC_MACH_NO = @tpssm26.CC_MACH_NO "
					" ORDER BY b.CC_SEQ ASC "
					);*/
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("tpssm26.FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
				cmd_tpssm12_inq.Parameters.Set("tpssm26.CC_MACH_NO", tpssm26["CC_MACH_NO"].ToString().Trim());
				cmd_tpssm12_inq.ExecuteReader();
				while (cmd_tpssm12_inq.Read())
				{
					tpssm15["RESTRAND_FLG"] = cmd_tpssm12_inq.GetString(1).Trim();
					tpssm15["SM_PLAN_NO"] = cmd_tpssm12_inq.GetString(2).Trim();
					tpssm15["TD_CHG_FLG"] = cmd_tpssm12_inq.GetDecimal(3);
					////Log::Trace("", __FUNCTION__, "快换中包tpssm15["TD_CHG_FLG"] =[{0}]", tpssm15["TD_CHG_FLG"].ToDecimal());
					////Log::Trace("", __FUNCTION__, "重引锭tpssm15["RESTRAND_FLG"] =[{0}]", tpssm15["RESTRAND_FLG"].ToString());
					//柳钢定制化需求：因生产节奏快，所以需要在快换中包时重新计算浇次号
					if (tpssm15["RESTRAND_FLG"].ToString().Trim() == "T" || tpssm15["TD_CHG_FLG"].ToDecimal() == 1) //重引锭 //||  或者快换中包
					{
						cast_stream_no = cast_stream_no.Parse(cast_no.Substring(2));//流水号--在前面扩1位后从第三位开始取，原本cast_no.Substring(1)
						cast_stream_no = cast_stream_no + 1;

						////Log::Trace("", __FUNCTION__, "计算cast_stream_no浇次号=[{0}]", cast_stream_no.ToInt32());

						cast_no = cast_no.Format("%s%s%.4d", (const char*)datetime.Trim().SubstringNE(3, 1), (const char*)tpssm26["CC_MACH_NO"].ToString().SubstringNE(0, 1), cast_stream_no.ToInt32());
						cast_div_no = 1;

					}
					else //不重引锭--连浇, cast_no不变
					{
						cast_div_no = cast_div_no + 1;
					}

					//~~~~~将cast_no, cast_div_no写入记录~~~~~
					//Log::Trace("", __FUNCTION__, "计算浇次号后cast_no=[{0}], cast_div_no=[{1}]", (const char*)cast_no, cast_div_no.ToInt32());

					bool has15 = tpssm15.Query("SM_PLAN_NO");
					if (has15)
					{
						tpssm15["CAST_NO"] = cast_no;
						tpssm15["CAST_DIV_NO"] = cast_div_no;
						tpssm15["FACTORY_DIV"] = tpssm26["FACTORY_DIV"];
						sqlstr = "tpssm15.Update(CAST_NO)";
						tpssm15.Update("CAST_NO,CAST_DIV_NO", "FACTORY_DIV,SM_PLAN_NO");
					}
				}
				cmd_tpssm12_inq.Close();

				//读取TPSSM15表记录，按cast_no汇总
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString("SELECT FACTORY_DIV,SM_PLAN_NO, CAST_NO FROM (SELECT * FROM TPSSM11 UNION ALL SELECT * FROM TPSSM15) WHERE FACTORY_DIV = @FACTORY_DIV AND RUN_STATUS < '52' ");
					break;
				}
				cmd_tpssm11_inq.SetCommandText(sqlstr);
				cmd_tpssm11_inq.Parameters.Set("FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
				cmd_tpssm11_inq.ExecuteReader();
				while (cmd_tpssm11_inq.Read())
				{
					tpssm15["FACTORY_DIV"] = cmd_tpssm11_inq.GetString(1);
					tpssm15["SM_PLAN_NO"] = cmd_tpssm11_inq.GetString(2);
					tpssm15["CAST_NO"] = cmd_tpssm11_inq.GetString(3);

					sqlstr = "SELECT MAX(CAST_DIV_NO) FROM (SELECT * FROM TPSSM11 UNION ALL SELECT * FROM TPSSM15) WHERE CAST_NO = @CAST_NO  AND FACTORY_DIV = @FACTORY_DIV ";

					cmd_tpssm11_inq2.SetCommandText(sqlstr);
					cmd_tpssm11_inq2.Parameters.Set("CAST_NO", tpssm11["CAST_NO"].ToString());
					cmd_tpssm11_inq2.Parameters.Set("FACTORY_DIV", tpssm26["FACTORY_DIV"].ToString());
					dummy = cmd_tpssm11_inq2.ExecuteScalar();

					/*dummy = tpssm11.QueryCount("FACTORY_DIV,CAST_NO");
					////Log::Trace("", __FUNCTION__, "tpssm11.QueryCount(CAST_NO)=[{0}]", dummy);*/
					bool has15 = tpssm15.Query("SM_PLAN_NO");
					if (has15)
					{
						//更新CAST内炉数
						tpssm15["CAST_PONO_SUM"] = dummy;
						sqlstr = "tpssm15.Update(CAST_PONO_SUM)";
						tpssm15.Update("CAST_PONO_SUM", "FACTORY_DIV,SM_PLAN_NO");
					}
				}
				cmd_tpssm11_inq.Close();
			}
			cmd_tpssm26_inq.Close();
		}

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
	cmd_tpssm26_inq.Close();
	cmd_tpssmd1_inq.Close();
	return doFlag;
}
