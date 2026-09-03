/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:  1.0
Date:     2011-12-30
Description: PONO删除
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)

#endif

#if defined _SYS_MES

#endif

int f_pmom_pssm_del_pono(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //PONO删除调用材料申请功能。2014-1-28 xuwen 替换原有f_pmomhr_pssm_del_pono、f_pmomsm_pssm_del_pono、f_pmombw_pssm_del_pono函数

int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
int f_qmtqhp_bujt(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

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
BM2F_ENTERACE(pssm01af6_del)

int f_pssm01af6_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret = 0;
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	bool ishave = false;

	/* 业务变量 */
	CString sqlstr;
	CDecimal total_count;
	CDecimal cc_seq;
	CString   temp_factory_div = "";
	CString   v_pes_host_no = "";
	CString	  v_pes_host_name = "";

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
	CModel tpmouhp31("TPMOUHP31");
#endif

#if defined _SYS_MES
	CModel tpssm10("TPSSM10");
#endif

	/* 数据库操作类定义 */
	CDbCommand cmd_tep0002_inq(conn);
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm01_upd(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_del(conn);
	CDbCommand cmd_tpssm03_inq(conn);

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
	CDbCommand cmd_tpmouhp31_inq(conn);
#endif

	EIClass bcls_rec_bujt;
	EIClass bcls_ret_bujt;

	EIClass inBlock;//调用生产接口
	EIClass inBlock1; //调用电文接口
	EIClass inBlock99; //调用炼钢履历跟踪
	EIClass outBlock;
	EIClass temp;

	try
	{
		//声明调用生产的接口参数
		inBlock.Tables[0].set_TableName("DELETEPONO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");

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

		if (!bcls_rec_bujt.Tables.Contains("QMZSBlock"))
		{
			bcls_rec_bujt.Tables.Add("QMZSBlock");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "OUHP_MAT_TYPE");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "PONO");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "TMP_SLAB_NO");
			bcls_rec_bujt.Tables["QMZSBlock"].Columns.Add(DT_STRING, "SAMPLE_LOT_STATUS");
			bcls_rec_bujt.Tables["QMZSBlock"].Rows.Add();
		}

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, "rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			//获取传入参数
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//打印传入参数
			////Log::Trace("", __FUNCTION__, "pssm01af6_del>tpssm01["PONO"] = [{0}]", tpssm01["PONO"].ToString());
			////Log::Trace("", __FUNCTION__, "pssm01af6_del>tpssm01["CAST_LOT_NO"] = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "pssm01af6_del>tpssm01["FACTORY_DIV"] = [{0}]", tpssm01["FACTORY_DIV"].ToString());

			sqlstr = "tpssm01.Query()";
			ishave = tpssm01.Query("FACTORY_DIV,PONO");
			if (ishave == false)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]已删除，请重新查询数据。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm01.TrimOrBlank();

			if (temp_factory_div != tpssm01["FACTORY_DIV"].ToString())
			{
				v_pes_host_no = "";
				v_pes_host_name = "";

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = CString(" SELECT CODE_DESC_1_CONTENT, CODE_DESC_2_CONTENT FROM TEP0002 "
						" WHERE  CODE_CLASS = 'PSAT' "
						"   AND  CODE = @tpssm01.FACTORY_DIV ");
					break;
				}
				cmd_tep0002_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
				cmd_tep0002_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tep0002_inq.ExecuteReader();

				if (cmd_tep0002_inq.Read())
				{
					v_pes_host_no = cmd_tep0002_inq.GetString(1);
					v_pes_host_name = cmd_tep0002_inq.GetString(2);
				}
				cmd_tep0002_inq.Close();

				temp_factory_div = tpssm01["FACTORY_DIV"];
			}

			////Log::Trace("", __FUNCTION__, "tpssm01["PONO_STATUS"] = [{0}]", tpssm01["PONO_STATUS"].ToDecimal());

			//校验PONO状态，是否排入出钢计划
			//HYF 20130401 之前是校验>15的状态，16是PES命令接收状态，18才是排入计划
			if (tpssm01["PONO_STATUS"].ToDecimal() >= 18)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]已排入出钢计划。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
			sqlstr = " SELECT PONO,SLAB_NO FROM TPSSM03 "
				" WHERE SLAB_PROD_FLAG = '0' "
				"   AND FACTORY_DIV = @tpssm01.FACTORY_DIV "
				"   AND PONO = @tpssm01.PONO ";

			cmd_tpssm03_inq.SetCommandText(sqlstr);
			////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
			cmd_tpssm03_inq.Parameters.Clear();
			cmd_tpssm03_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();

			while (cmd_tpssm03_inq.Read())
			{
				tpssm03["PONO"] = cmd_tpssm03_inq.GetString(1);
				tpssm03["SLAB_NO"] = cmd_tpssm03_inq.GetString(2);

				sqlstr = " SELECT MAT_DESIGN_KIND,TMP_SLAB_NO "
					" FROM TPMOUHP31 t "
					" WHERE PONO_SLAB = @tpssm03.SLAB_NO ";

				cmd_tpmouhp31_inq.SetCommandText(sqlstr);
				////Log::Trace("", __FUNCTION__, "sqlstr =[{0}]", sqlstr);
				cmd_tpmouhp31_inq.Parameters.Clear();
				cmd_tpmouhp31_inq.Parameters.Set("tpssm03.SLAB_NO", tpssm03["SLAB_NO"].ToString());
				cmd_tpmouhp31_inq.ExecuteReader();

				if (cmd_tpmouhp31_inq.Read())
				{
					tpmouhp31["MAT_DESIGN_KIND"] = cmd_tpmouhp31_inq.GetString(1);
					tpmouhp31["TMP_SLAB_NO"] = cmd_tpmouhp31_inq.GetString(2);

					bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["OUHP_MAT_TYPE"] = tpmouhp31["MAT_DESIGN_KIND"];
					bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["PONO"] = tpssm03["PONO"];
					bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["TMP_SLAB_NO"] = tpmouhp31["TMP_SLAB_NO"];
					//bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["SAMPLE_LOT_STATUS"] = "14";//12 试材回退 14命令回退
					bcls_rec_bujt.Tables["QMZSBlock"].Rows[0]["SAMPLE_LOT_STATUS"] = "12";//12 试材回退 14命令回退 与孙羽田确认，传12 2017-06-29

					doFlag = f_qmtqhp_bujt(&bcls_rec_bujt, &bcls_ret_bujt, conn);

					////Log::Trace("", __FUNCTION__, "doFlag = [{0}]", doFlag);

					if (doFlag < 0)
					{
						strcpy(s.msg, CString::Format("调用质量的试材回退函数失败。[%d][%s]", doFlag, (const char*)s.msg));
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				cmd_tpmouhp31_inq.Close();
			}
			cmd_tpssm03_inq.Close();
#endif

			//根据去向调用不同的函数
			//switch(conn->DatabaseKind)
			//{
			//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
			//	case DB_KIND_ORACLE:	    // Oracle 数据库
			//	default:
			//		
			//	sqlstr = CString(" SELECT MAT_DESTION "
			//					 " FROM   TPSSM01 "
			//					 " WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
			//					 " AND    PONO = @tpssm01.PONO ");
			//	break;
			//}

			//cmd_tpssm01_inq.SetCommandText(sqlstr);
			//cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			//cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			//cmd_tpssm01_inq.ExecuteReader();

			//if(cmd_tpssm01_inq.Read())
			//{	
			//	tpssm01.MAT_DESTION = cmd_tpssm01_inq.GetString(1).TrimOrBlank();
			//}

			//---------------------------------------------------------
			//PONO删除调用材料申请功能。几条产线的合并。

			//调用生产函数
			inBlock.Tables[0].Rows.Clear();
			inBlock.Tables[0].Rows.Add();
			inBlock.Tables[0].Rows[0]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			inBlock.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];

			//ret = f_pmom_pssm_del_pono(&inBlock, &outBlock, conn);

			/*
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			*/

			//删除TPSSM01
			//tpssm01.Delete("FACTORY_DIV, PONO");

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
			////Log::Trace("", __FUNCTION__, "pssm01af6_del>tpssm10["PONO"] = [{0}]", tpssm01["PONO"].ToString());

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

			if (v_pes_host_no.Trim() != "")
			{
				//调用计划下发电文
				inBlock1.Tables[0].Rows.Clear();
				inBlock1.Tables[0].Rows.Add();
				inBlock1.Tables[0].Rows[0]["MARKS1"] = 3;
				inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
				//增加主工序代码 HYF 20130401
				inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			}

			CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
			row99["EVENT_ID"] = "1D"; //PONO删除
			row99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			row99["PONO"] = tpssm01["PONO"];
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
		/*
#ifdef _SYS_MMS 
		ret = f_cm_002021_snd(&inBlock1, &outBlock, conn);

		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
#endif

		if (v_pes_host_no.Trim() != "")
		{
			if (v_pes_host_no == "00")
			{
				ret = f_cm_002021_snd(&inBlock1, &outBlock, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}
		*/
		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	//cmd_tpssm01_inq.Close();

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
