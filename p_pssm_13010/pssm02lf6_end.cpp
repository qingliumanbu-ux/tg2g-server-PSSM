/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:  1.0
Date:     2011-12-30
Description: LOT完成
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



#if defined _SYS_MES

#endif

int f_pmom_pssm_del_pono(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //PONO删除调用材料申请功能。2014-1-28 xuwen 替换原有f_pmomhr_pssm_del_pono、f_pmomsm_pssm_del_pono、f_pmombw_pssm_del_pono函数
int f_pmom_lot_confm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsm_heat_confirm_lot(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#if defined _SYS_MMS
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif
int f_pssm99_trace_l4(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 制造命令删除
/// <para>
/// 1.校验是否已排入出钢计划
/// 2.删除连铸制造命令炉次表、板坯表、铸机流表、LOT表
/// 3.调用PM函数
/// 4.下达PES命令删除电文
/// </para>
/// <para>数据库表：TPSSM01(制造命令炉次表)/TPSSM02(制造命令LOT表)/TPSSM03(制造命令板坯表)/TPSSM04(制造命令铸机流)</para>
/// <para>主调用函数：前台PSSM01画面F7 PONO删除调用。															  </para>
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别代码    </param>
/// <param name="pono">制造命令						</param>
/// <returns>炉次制造命令</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm02lf6_end)

int f_pssm02lf6_end(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret = 0;
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	bool ishave = false;
	CString  v_code_desc_5_content = "";

	/* 业务变量 */
	CString sqlstr;
	CDecimal total_count;
	CDecimal cc_seq;
	int rownum_l = 0;

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");

#if defined _SYS_MES
	CModel tpssm10("TPSSM10");
#endif

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm01_upd(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_tpssm03_upd(conn);
	CDbCommand cmd_sql(conn);
	CDbCommand cmd_del(conn);


	EIClass inBlock;//调用生产接口
	EIClass inBlock1; //调用电文接口
	EIClass inBlock99; //调用炼钢履历跟踪
	EIClass outBlock;
	EIClass temp;
	EIClass inBlock_cfm;
	EIClass outBlock_cfm;
	EIClass inBlock_mm;
	EIClass outBlock_mm;

	try
	{
		//声明调用生产的接口参数
		inBlock.Tables[0].set_TableName("DELETEPONO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");

		inBlock_cfm.Tables[0].set_TableName("LOTCONFM");
		inBlock_cfm.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");

		inBlock_mm.Tables[0].set_TableName("MMSMCONFMLOT");
		inBlock_mm.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");

		//声明调用电文的接口参数
		inBlock1.Tables[0].set_TableName("PONOSEND");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "MARKS1");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");

		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, "rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			tpssm02.Reset();
			//获取传入参数
			tpssm02.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//打印传入参数
			////Log::Trace("", __FUNCTION__, "pssm02f6_end>tpssm01["CAST_LOT_NO"] = [{0}]", tpssm02["CAST_LOT_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "pssm02f6_end>tpssm01["FACTORY_DIV"] = [{0}]", tpssm02["FACTORY_DIV"].ToString());

			sqlstr = "tpssm02.Query()";
			ishave = tpssm02.Query("FACTORY_DIV, CAST_LOT_NO");
			if (ishave == false)
			{
				CFormattable arguments[] = { tpssm02["CAST_LOT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]已删除，请重新查询数据。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm02.TrimOrBlank();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT CODE_DESC_5_CONTENT FROM TEP0002  "
					"	WHERE CODE_CLASS = 'PSA6' "
					"	  AND CODE = @tpssm02.BILLET_TYPE ";
				break;
			}
			cmd_sql.SetCommandText(sqlstr);
			cmd_sql.Parameters.Clear();
			cmd_sql.Parameters.Set("tpssm02.BILLET_TYPE", tpssm02["BILLET_TYPE"].ToString());
			cmd_sql.ExecuteReader();
			if (cmd_sql.Read())
			{
				v_code_desc_5_content = cmd_sql.GetString(1);
			}
			cmd_sql.Close();

			if (v_code_desc_5_content.Trim() == "P")
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = " SELECT * "
						" FROM TPSSM01 "
						" WHERE CAST_LOT_NO	= @tpssm01.CAST_LOT_NO "
						"   AND PONO_STATUS	< '91' ";
					break;
				}
				cmd_sql.SetCommandText(sqlstr);
				cmd_sql.Parameters.Clear();
				cmd_sql.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
				cmd_sql.ExecuteReader();

				while (cmd_sql.Read())
				{
					cmd_sql.Fetch(tpssm01);

					////Log::Trace("", __FUNCTION__, "tpssm01["PONO_STATUS"] = [{0}]", tpssm01["PONO_STATUS"].ToDecimal());

					//校验PONO状态，是否排入出钢计划
					//HYF 20130401 之前是校验>15的状态，16是PES命令接收状态，18才是排入计划
					if (tpssm01["PONO_STATUS"].ToDecimal() >= 18)
					{
						CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "制造命令号[{0}]已排入出钢计划。", arguments, 1);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//---------------------------------------------------------
					//PONO删除调用材料申请功能。几条产线的合并。

					//调用生产函数
					inBlock.Tables[0].Rows.Clear();
					inBlock.Tables[0].Rows.Add();
					inBlock.Tables[0].Rows[0]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
					inBlock.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];

					ret = f_pmom_pssm_del_pono(&inBlock, &outBlock, conn);

					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//删除TPSSM01
					tpssm01.Delete("FACTORY_DIV, PONO");

					//删除TPSSM03
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:

						sqlstr = CString(" DELETE FROM TPSSM03 "
							" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
							" AND    PONO = @tpssm01.PONO ");
						break;
					}

					cmd_del.SetCommandText(sqlstr);
					cmd_del.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());   //设置条件数据项
					cmd_del.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());   //设置条件数据项
					cmd_del.ExecuteNonQuery();	              //执行删除
					cmd_del.Close();

	#ifdef _SYS_MES
					//读取要删除PONO信息
					tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					tpssm10["PONO"] = tpssm01["PONO"];
					sqlstr = "tpssm10.Query()";
					bool has10 = tpssm10.Query();

					if (has10 == true) //有记录
					{
						//2）"T"标记传递给下一炉
						if (tpssm10["RESTRAND_FLG"].ToString().Trim() != "")
						{

							////Log::Trace("", __FUNCTION__, "cc_mach_no =[{0}]", tpssm10["CC_MACH_NO"].ToString());
							//读取下一炉的cc_seq（考虑跳号可能性。重号就没办法了）
							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:         // MS SQL Server数据库
							case DB_KIND_ORACLE:        // Oracle 数据库
							default:  // 所有数据库适用，通用SQL语句。
								sqlstr = CString(
									" SELECT MIN(cc_seq) FROM TPSSM10 "
									"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
									"    AND CC_MACH_NO = @tpssm10.CC_MACH_NO "
									"    AND CC_SEQ     > @tpssm10.CC_SEQ "
									);
								break;
							}
							cmd_tpssm10_inq.SetCommandText(sqlstr);
							cmd_tpssm10_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
							cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
							cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
							cmd_tpssm10_inq.ExecuteReader();
							if (cmd_tpssm10_inq.Read())
							{
								cc_seq = cmd_tpssm10_inq.GetDecimal(1);
							}
							else
							{
								cc_seq = 0;
							}

							if (cc_seq > 0)//有下一炉
							{
								switch (conn->DatabaseKind)
								{
								case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
								case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
								case DB_KIND_MSSQL:         // MS SQL Server数据库
								case DB_KIND_ORACLE:        // Oracle 数据库
								default:  // 所有数据库适用，通用SQL语句。
									sqlstr = CString(
										" UPDATE TPSSM10 "
										"    SET RESTRAND_FLG = 'T', "
										"        CC_PREP_TIME = @cc_prep_time "  //炉间准备时间
										"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
										"    AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
										"    AND CC_SEQ     = @cc_seq "
										"    AND RESTRAND_FLG <> 'T' "  //下一炉如果有T标记，不做修改
										);
									break;
								}
								cmd_tpssm10_upd.SetCommandText(sqlstr);
								cmd_tpssm10_upd.Parameters.Set("cc_prep_time", tpssm10["CC_PREP_TIME"].ToDecimal());
								cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString().Trim());
								cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString().Trim());
								cmd_tpssm10_upd.Parameters.Set("cc_seq", cc_seq);
								cmd_tpssm10_upd.ExecuteNonQuery();


							}//if (cc_seq > 0) 有下一炉

						}//if 带T


						//删除PONO
						sqlstr = "tpssm10.Delete()";
						tpssm10.Delete();

						////Log::Trace("", __FUNCTION__, "删除tpssm10");


						//后序的炉次浇注顺序号向上移
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:
							sqlstr = CString(
								" UPDATE TPSSM10 "
								"   SET CC_SEQ = CC_SEQ - 1 "
								"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
								"    AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
								"    AND CC_SEQ     > @tpssm10.CC_SEQ "
								"    AND CC_SEQ	    < 900 "
								);
							break;
						}
						cmd_tpssm10_upd.SetCommandText(sqlstr);
						cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
						cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
						cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
						cmd_tpssm10_upd.ExecuteNonQuery();
					}
	#endif

	#ifdef _SYS_MMS
					//调用计划下发电文
					inBlock1.Tables[0].Rows.Clear();
					inBlock1.Tables[0].Rows.Add();
					inBlock1.Tables[0].Rows[0]["MARKS1"] = 3;
					inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
					//增加主工序代码 HYF 20130401
					inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
	#endif

					CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
					row99["EVENT_ID"] = "1D"; //PONO删除
					row99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					row99["PONO"] = tpssm01["PONO"];
				}
				cmd_sql.Close();
				
				//判断该LOT下的PONO是否都已经删除
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = CString(" SELECT COUNT(1) FROM TPSSM01 "
						" WHERE  CAST_LOT_NO = @tpssm01.CAST_LOT_NO ");
					break;
				}

				cmd_tpssm01_inq.SetCommandText(sqlstr);
				cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV ", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm01_inq.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
				total_count = cmd_tpssm01_inq.ExecuteScalar();
				cmd_tpssm01_inq.Close();

				if (total_count == 0)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:

						sqlstr = CString(" DELETE FROM TPSSM02 "
							" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
							" AND    CAST_LOT_NO = @tpssm01.CAST_LOT_NO ");
						break;
					}

					cmd_del.SetCommandText(sqlstr);
					cmd_del.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());   //设置条件数据项
					cmd_del.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());   //设置条件数据项
					cmd_del.ExecuteNonQuery();	              //执行删除
					cmd_del.Close();
				}
				else
				{
					////更新TPSSM01.CAST_LOT_SUM
					//sqlstr = CString(
					//	" UPDATE  TPSSM01"
					//	" SET 	  CAST_LOT_SUM = @CAST_LOT_SUM "
					//	" WHERE   CAST_LOT_NO = @CAST_LOT_NO "
					//	);

					//cmd_tpssm01_upd.SetCommandText(sqlstr);
					//cmd_tpssm01_upd.Parameters.Set("CAST_LOT_SUM", total_count);   //设置修改数据项
					//cmd_tpssm01_upd.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());     //设置条件数据项
					//cmd_tpssm01_upd.ExecuteNonQuery();	             //执行修改

					////更新TPSSM02.CAST_LOT_SUM
					//tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
					//tpssm02["CAST_LOT_SUM"] = total_count;

					//sqlstr = "tpssm02.Update()";
					//tpssm02.Update("CAST_LOT_SUM", "CAST_LOT_NO");

					//更新TPSSM02表 LOT状态（LOT_STATUS），浇铸批号下所有PONO都炉次确定则LOT_STATUS = 9

					sqlstr = " SELECT DISTINCT PONO_STATUS FROM TPSSM01 WHERE CAST_LOT_NO = @CAST_LOT_NO ";

					cmd_tpssm01_inq.SetCommandText(sqlstr);
					cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
					cmd_tpssm01_inq.ExecuteQuery(temp.Tables[0]);

					ret = temp.Tables[0].Rows.get_Count();
					if (ret == 1)
					{
						if (temp.Tables[0].Rows[0]["PONO_STATUS"].ToString() == "91")
						{
							tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
							tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

							tpssm02["LOT_STATUS"] = 9; //炉次确定
							tpssm02["REC_REVISOR"] = s.userid;
							tpssm02["REC_REVISE_TIME"] = dateNow;

							sqlstr = "tpssm02.update()";
							tpssm02.Update("LOT_STATUS,REC_REVISOR,REC_REVISE_TIME", "CAST_LOT_NO,FACTORY_DIV");

						}
					}
				}

				////Log::Trace("", __FUNCTION__, "--tpssm01["PONO"] = [{0}]", tpssm01["PONO"].ToString());
				////Log::Trace("", __FUNCTION__, "--tpssm01["FACTORY_DIV"] = [{0}]", tpssm01["FACTORY_DIV"].ToString());

	#ifdef _SYS_MMS 
				ret = f_cm_002021_snd(&inBlock1, &outBlock, conn);

				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
	#endif

				//-----------------------------------------------
				//炼钢履历跟踪
				ret = f_pssm99_trace_l4(&inBlock99, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = " SELECT * "
						" FROM TPSSM01 "
						" WHERE CAST_LOT_NO	= @tpssm02.CAST_LOT_NO "
						"   AND PONO_STATUS	< '91' ";
					break;
				}
				cmd_sql.SetCommandText(sqlstr);
				cmd_sql.Parameters.Clear();
				cmd_sql.Parameters.Set("tpssm02.CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());
				cmd_sql.ExecuteReader();

				while (cmd_sql.Read())
				{
					cmd_sql.Fetch(tpssm01);

					////Log::Trace("", __FUNCTION__, "tpssm01["PONO_STATUS"] = [{0}]", tpssm01["PONO_STATUS"].ToDecimal());

					if (tpssm01["PONO_STATUS"].ToDecimal() >= 83)
					{
						CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "制造命令号[{0}]未炉次确定，请核实。", arguments, 1);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//校验PONO状态，是否排入出钢计划
					//HYF 20130401 之前是校验>15的状态，16是PES命令接收状态，18才是排入计划
					if (tpssm01["PONO_STATUS"].ToDecimal() >= 18)
					{
						CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "制造命令号[{0}]已排入出钢计划。", arguments, 1);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//删除TPSSM01
					tpssm01.Delete("FACTORY_DIV, PONO");

#ifdef _SYS_MES
					//读取要删除PONO信息
					tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					tpssm10["PONO"] = tpssm01["PONO"];
					sqlstr = "tpssm10.Query()";
					bool has10 = tpssm10.Query();

					if (has10 == true) //有记录
					{
						//2）"T"标记传递给下一炉
						if (tpssm10["RESTRAND_FLG"].ToString().Trim() != "")
						{

							////Log::Trace("", __FUNCTION__, "cc_mach_no =[{0}]", tpssm10["CC_MACH_NO"].ToString());
							//读取下一炉的cc_seq（考虑跳号可能性。重号就没办法了）
							switch (conn->DatabaseKind)
							{
							case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:         // MS SQL Server数据库
							case DB_KIND_ORACLE:        // Oracle 数据库
							default:  // 所有数据库适用，通用SQL语句。
								sqlstr = CString(
									" SELECT MIN(cc_seq) FROM TPSSM10 "
									"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
									"    AND CC_MACH_NO = @tpssm10.CC_MACH_NO "
									"    AND CC_SEQ     > @tpssm10.CC_SEQ "
									);
								break;
							}
							cmd_tpssm10_inq.SetCommandText(sqlstr);
							cmd_tpssm10_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
							cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
							cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
							cmd_tpssm10_inq.ExecuteReader();
							if (cmd_tpssm10_inq.Read())
							{
								cc_seq = cmd_tpssm10_inq.GetDecimal(1);
							}
							else
							{
								cc_seq = 0;
							}

							if (cc_seq > 0)//有下一炉
							{
								switch (conn->DatabaseKind)
								{
								case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
								case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
								case DB_KIND_MSSQL:         // MS SQL Server数据库
								case DB_KIND_ORACLE:        // Oracle 数据库
								default:  // 所有数据库适用，通用SQL语句。
									sqlstr = CString(
										" UPDATE TPSSM10 "
										"    SET RESTRAND_FLG = 'T', "
										"        CC_PREP_TIME = @cc_prep_time "  //炉间准备时间
										"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
										"    AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
										"    AND CC_SEQ     = @cc_seq "
										"    AND RESTRAND_FLG <> 'T' "  //下一炉如果有T标记，不做修改
										);
									break;
								}
								cmd_tpssm10_upd.SetCommandText(sqlstr);
								cmd_tpssm10_upd.Parameters.Set("cc_prep_time", tpssm10["CC_PREP_TIME"].ToDecimal());
								cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString().Trim());
								cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString().Trim());
								cmd_tpssm10_upd.Parameters.Set("cc_seq", cc_seq);
								cmd_tpssm10_upd.ExecuteNonQuery();


							}//if (cc_seq > 0) 有下一炉

						}//if 带T


						//删除PONO
						sqlstr = "tpssm10.Delete()";
						tpssm10.Delete();

						////Log::Trace("", __FUNCTION__, "删除tpssm10");


						//后序的炉次浇注顺序号向上移
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:
							sqlstr = CString(
								" UPDATE TPSSM10 "
								"   SET CC_SEQ = CC_SEQ - 1 "
								"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
								"    AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
								"    AND CC_SEQ     > @tpssm10.CC_SEQ "
								"    AND CC_SEQ	    < 900 "
								);
							break;
						}
						cmd_tpssm10_upd.SetCommandText(sqlstr);
						cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
						cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
						cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
						cmd_tpssm10_upd.ExecuteNonQuery();
					}
#endif

#ifdef _SYS_MMS
					//调用计划下发电文
					inBlock1.Tables[0].Rows.Clear();
					inBlock1.Tables[0].Rows.Add();
					inBlock1.Tables[0].Rows[0]["MARKS1"] = 3;
					inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
					//增加主工序代码 HYF 20130401
					inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
#endif

					CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
					row99["EVENT_ID"] = "1D"; //PONO删除
					row99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					row99["PONO"] = tpssm01["PONO"];
				}
				cmd_sql.Close();
				
				sqlstr = " UPDATE TPSSM03 "
					"  SET  SLAB_PROD_FLAG = '9' "
					" WHERE SLAB_PROD_FLAG = '0' "
					"   AND FACTORY_DIV = @tpssm02.FACTORY_DIV "
					"   AND CAST_LOT_NO = @tpssm02.CAST_LOT_NO ";

				cmd_tpssm03_upd.SetCommandText(sqlstr);
				cmd_tpssm03_upd.Parameters.Clear();
				cmd_tpssm03_upd.Parameters.Set("tpssm02.FACTORY_DIV", tpssm02["FACTORY_DIV"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm02.CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());
				cmd_tpssm03_upd.ExecuteNonQuery();

				//调用生产函数
				inBlock_cfm.Tables["LOTCONFM"].Rows.Clear();
				inBlock_cfm.Tables["LOTCONFM"].Rows.Add();
				inBlock_cfm.Tables["LOTCONFM"].Rows[0]["CAST_LOT_NO"] = tpssm02["CAST_LOT_NO"];

				ret = f_pmom_lot_confm(&inBlock_cfm, &outBlock_cfm, conn);

				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				inBlock_mm.Tables["MMSMCONFMLOT"].Rows.Clear();
				inBlock_mm.Tables["MMSMCONFMLOT"].Rows.Add();
				inBlock_mm.Tables["MMSMCONFMLOT"].Rows[0]["CAST_LOT_NO"] = tpssm02["CAST_LOT_NO"];

				doFlag = f_mmsm_heat_confirm_lot(&inBlock_mm, &outBlock_mm, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//判断该LOT下的PONO是否都已经删除
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = CString(" SELECT COUNT(1) FROM TPSSM01 "
						" WHERE  CAST_LOT_NO = @tpssm02.CAST_LOT_NO ");
					break;
				}

				cmd_tpssm01_inq.SetCommandText(sqlstr);
				cmd_tpssm01_inq.Parameters.Set("tpssm02.FACTORY_DIV ", tpssm02["FACTORY_DIV"].ToString());
				cmd_tpssm01_inq.Parameters.Set("tpssm02.CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());
				total_count = cmd_tpssm01_inq.ExecuteScalar();
				cmd_tpssm01_inq.Close();

				if (total_count == 0)
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:

						sqlstr = CString(" DELETE FROM TPSSM02 "
							" WHERE  FACTORY_DIV = @tpssm02.FACTORY_DIV "
							" AND    CAST_LOT_NO = @tpssm02.CAST_LOT_NO ");
						break;
					}

					cmd_del.SetCommandText(sqlstr);
					cmd_del.Parameters.Set("tpssm02.FACTORY_DIV", tpssm02["FACTORY_DIV"].ToString());   //设置条件数据项
					cmd_del.Parameters.Set("tpssm02.CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());   //设置条件数据项
					cmd_del.ExecuteNonQuery();	              //执行删除
					cmd_del.Close();
				}
				else
				{
					////更新TPSSM01.CAST_LOT_SUM
					//sqlstr = CString(
					//	" UPDATE  TPSSM01"
					//	" SET 	  CAST_LOT_SUM = @CAST_LOT_SUM "
					//	" WHERE   CAST_LOT_NO = @CAST_LOT_NO "
					//	);

					//cmd_tpssm01_upd.SetCommandText(sqlstr);
					//cmd_tpssm01_upd.Parameters.Set("CAST_LOT_SUM", total_count);   //设置修改数据项
					//cmd_tpssm01_upd.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());     //设置条件数据项
					//cmd_tpssm01_upd.ExecuteNonQuery();	             //执行修改

					////更新TPSSM02.CAST_LOT_SUM
					//tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
					//tpssm02["CAST_LOT_SUM"] = total_count;

					//sqlstr = "tpssm02.Update()";
					//tpssm02.Update("CAST_LOT_SUM", "CAST_LOT_NO");

					//更新TPSSM02表 LOT状态（LOT_STATUS），浇铸批号下所有PONO都炉次确定则LOT_STATUS = 9

					sqlstr = " SELECT DISTINCT PONO_STATUS FROM TPSSM01 WHERE CAST_LOT_NO = @CAST_LOT_NO ";

					cmd_tpssm01_inq.SetCommandText(sqlstr);
					cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm02["CAST_LOT_NO"].ToString());
					cmd_tpssm01_inq.ExecuteQuery(temp.Tables[0]);

					ret = temp.Tables[0].Rows.get_Count();
					if (ret == 1)
					{
						if (temp.Tables[0].Rows[0]["PONO_STATUS"].ToString() == "91")
						{
							tpssm02["LOT_STATUS"] = 9; //炉次确定
							tpssm02["REC_REVISOR"] = s.userid;
							tpssm02["REC_REVISE_TIME"] = dateNow;

							sqlstr = "tpssm02.update()";
							tpssm02.Update("LOT_STATUS,REC_REVISOR,REC_REVISE_TIME", "CAST_LOT_NO,FACTORY_DIV");

						}
					}
				}

				////Log::Trace("", __FUNCTION__, "--tpssm02["CAST_LOT_NO"] = [{0}]", tpssm02["CAST_LOT_NO"].ToString());
				////Log::Trace("", __FUNCTION__, "--tpssm02["FACTORY_DIV"] = [{0}]", tpssm02["FACTORY_DIV"].ToString());

#ifdef _SYS_MMS 
				ret = f_cm_002021_snd(&inBlock1, &outBlock, conn);

				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
#endif

				//-----------------------------------------------
				//炼钢履历跟踪
				ret = f_pssm99_trace_l4(&inBlock99, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}
	}

	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	//cmd_tpssm01_inq.Close();

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
