/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    向萍
Version:   1.0
Date:      2015-07-22
Description: MMS系统接受PES的计划状态
**************************************************************************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/





#include "epex.h" 

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)

#endif


/* ***** 外部函数申明 ***** */

int f_pmom_pono_confm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pmom_lot_confm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsm_heat_confirm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_mmsm_heat_confirm_lot(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
int f_qmtqhp_dele_resv_chg_new(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_qmtqhp_bujt(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// MMS系统接受PES的计划状态
/// <para>
/// 1.读取传入参数
/// 2.修改炼钢计划状态
/// 3.命令接收应答至MMS
/// </para>
/// </summary>
/// <param name="FACTORY_DIV">主工序代码</param>
/// <param name="PONO">制造命令号</param>
/// <param name="PONO_STATUS">PONO状态</param>
/// <returns>炉次制造命令。</returns>
===========================================================</remark>*/


// service入口
BM2F_ENTERACE_TELE(cm_200009_rcv)


int f_cm_200009_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/*程序用变量*/
	int doFlag = 0;
	int ret=0;
	int n_count = 0;
	int i = 0;

	/*业务变量*/
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString  sqlstr;
	CDecimal pono_status = 0;
	CString	 heat_no = "";
	CString v_code_desc_5_content = "";
	int row_num = 0;
	int row_num_p = 0;
	int row_num_l = 0;
	int v_count = 0;

	/*实体类定义*/
	
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");

	EIClass inBlock99; //调用炼钢履历跟踪
	EIClass inBlock_pmconfm;
	EIClass inBlock_mmconfm;

	EIClass bcls_rec_dele_resv;
	EIClass bcls_ret_dele_resv;

	EIClass bcls_rec_bujt;
	EIClass bcls_ret_bujt;

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
 	CDbCommand cmd_upd(conn); //与DB 建立连接。
	CDbCommand cmd_tpssm03_upd(conn);
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
#if defined _LINE_HP && (defined _SYS_MMS || defined _SYS_MES)
	CModel tpmouhp31("TPMOUHP31");
	CDbCommand cmd_tpmouhp31_inq(conn);
#endif

	try
	{
		inBlock_pmconfm.Clear();
		if (!inBlock_pmconfm.Tables.Contains("PONOCONFM"))
		{
			inBlock_pmconfm.Tables.Add("PONOCONFM");
		}
		if (!inBlock_pmconfm.Tables["PONOCONFM"].Columns.Contains("PONO"))
		{
			inBlock_pmconfm.Tables["PONOCONFM"].Columns.Add(DT_STRING, "PONO");
		}

		if (!inBlock_pmconfm.Tables.Contains("LOTCONFM"))
		{
			inBlock_pmconfm.Tables.Add("LOTCONFM");
		}
		if (!inBlock_pmconfm.Tables["LOTCONFM"].Columns.Contains("CAST_LOT_NO"))
		{
			inBlock_pmconfm.Tables["LOTCONFM"].Columns.Add(DT_STRING, "CAST_LOT_NO");
		}

		inBlock_mmconfm.Clear();
		//modify by xp 2016/11/29 应张颖要求炉次确定增加调用合同跟踪处理
		if (!inBlock_mmconfm.Tables.Contains("MMSMCONFM"))
		{
			inBlock_mmconfm.Tables.Add("MMSMCONFM");
		}
		if (!inBlock_mmconfm.Tables["MMSMCONFM"].Columns.Contains("PONO"))
		{
			inBlock_mmconfm.Tables["MMSMCONFM"].Columns.Add(DT_STRING, "PONO");
		}

		if (!inBlock_mmconfm.Tables.Contains("MMSMCONFMLOT"))
		{
			inBlock_mmconfm.Tables.Add("MMSMCONFMLOT");
		}
		if (!inBlock_mmconfm.Tables["MMSMCONFMLOT"].Columns.Contains("CAST_LOT_NO"))
		{
			inBlock_mmconfm.Tables["MMSMCONFMLOT"].Columns.Add(DT_STRING, "CAST_LOT_NO");
		}

		if (!bcls_rec_dele_resv.Tables.Contains("QMZSBlock"))
		{
			bcls_rec_dele_resv.Tables.Add("QMZSBlock");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "PONO_SLAB");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "PONO");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "EVENT_ID");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "EVENT_DESC");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "SYSTEM_ID");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "FUNC_ID");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "FORM_CODE");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "MAT_KIND");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Columns.Add(DT_STRING, "MAT_NO");
			bcls_rec_dele_resv.Tables["QMZSBlock"].Rows.Add();
		}

		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
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

		tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		pono_status = tpssm01["PONO_STATUS"];

		if ((pono_status != 84) && (pono_status != 85))
		{
			sqlstr = "tpssm01.Query()";
			tpssm01.Query("FACTORY_DIV, PONO");

			if (tpssm01["PONO_STATUS"].ToDecimal() == 91)
			{
				strcpy(s.msg, "[" + tpssm01["PONO"].ToString() + "]已炉次确定。");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//更新TPSSM01表制造命令状态
			sqlstr = CString(
				" UPDATE TPSSM01"
				" SET PONO_STATUS = @tpssm01.PONO_STATUS "
				" WHERE PONO = @tpssm01.PONO "
				);

			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("tpssm01.PONO_STATUS", pono_status);   //设置修改数据项
			cmd_upd.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());     //设置条件数据项
			cmd_upd.ExecuteNonQuery();
		}

		if (pono_status == 15) //命令下达
		{
			//更新TPSSM02表 LOT状态（LOT_STATUS），浇铸批号下所有PONO都命令下达则LOT_STATUS = 3

			sqlstr = " SELECT COUNT(*) FROM TPSSM01 WHERE PONO_STATUS > 15 and CAST_LOT_NO = @CAST_LOT_NO ";

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			ret = cmd_tpssm01_inq.ExecuteScalar().ToInt32();

			if (ret == 0)
			{
				tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

				tpssm02["LOT_STATUS"] = 3; //命令下达
				tpssm02["REC_REVISOR"] = s.userid;
				tpssm02["REC_REVISE_TIME"] = dateNow;

				sqlstr = "tpssm02.update()";
				tpssm02.Update("LOT_STATUS,REC_REVISOR,REC_REVISE_TIME", "CAST_LOT_NO,FACTORY_DIV");
			}
		}
		else if (pono_status == 18) //出钢计划
		{
			//更新TPSSM02表 LOT状态（LOT_STATUS)

			tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			sqlstr = "tpssm02.Query()";
			tpssm02.Query("CAST_LOT_NO,FACTORY_DIV");

			if (tpssm02["LOT_STATUS"].ToDecimal() < 4)
			{
				tpssm02["LOT_STATUS"] = 4; //出钢计划
				tpssm02["REC_REVISOR"] = s.userid;
				tpssm02["REC_REVISE_TIME"] = dateNow;

				sqlstr = "tpssm02.update()";
				tpssm02.Update("LOT_STATUS,REC_REVISOR,REC_REVISE_TIME", "CAST_LOT_NO,FACTORY_DIV");
			}
		}
		else if (pono_status == 83) //浇注终了
		{
			//更新TPSSM02表 LOT状态（LOT_STATUS），浇铸批号下所有PONO都浇注终了后置8

			sqlstr = " SELECT COUNT(*) FROM TPSSM01 WHERE PONO_STATUS < 83 and CAST_LOT_NO = @CAST_LOT_NO ";

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			ret = cmd_tpssm01_inq.ExecuteScalar().ToInt32();
			cmd_tpssm01_inq.Close();
			if (ret == 0)
			{
				tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

				tpssm02["LOT_STATUS"] = 8; //浇注终了
				tpssm02["REC_REVISOR"] = s.userid;
				tpssm02["REC_REVISE_TIME"] = dateNow;

				sqlstr = "tpssm02.update()";
				tpssm02.Update("LOT_STATUS,REC_REVISOR,REC_REVISE_TIME", "CAST_LOT_NO,FACTORY_DIV");
			}
		}
		else if (pono_status == 91) //炉次确定
		{
			//更新TPSSM01表制造命令状态
			sqlstr = CString(
				" UPDATE TPSSM01"
				" SET PONO_STATUS = @tpssm01.PONO_STATUS "
				" WHERE PONO = @tpssm01.PONO "
				);

			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("tpssm01.PONO_STATUS", pono_status);   //设置修改数据项
			cmd_upd.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());     //设置条件数据项
			cmd_upd.ExecuteNonQuery();

			//sqlstr = " UPDATE TPSSM03 "
			//	"  SET  SLAB_PROD_FLAG = '9' "
			//	" WHERE SLAB_PROD_FLAG = '0' "
			//	"   AND PONO = @tpssm01.PONO ";

			//cmd_tpssm03_upd.SetCommandText(sqlstr);
			//cmd_tpssm03_upd.Parameters.Clear();
			//cmd_tpssm03_upd.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			//cmd_tpssm03_upd.ExecuteNonQuery();

			//更新TPSSM02表 LOT状态（LOT_STATUS），浇铸批号下所有PONO都炉次确定后置9

			sqlstr = " SELECT COUNT(*) FROM TPSSM01 WHERE PONO_STATUS < 91 and CAST_LOT_NO = @CAST_LOT_NO ";

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			ret = cmd_tpssm01_inq.ExecuteScalar().ToInt32();
			cmd_tpssm01_inq.Close();
			if (ret == 0)
			{
				tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

				tpssm02["LOT_STATUS"] = 9; //炉次确定
				tpssm02["REC_REVISOR"] = s.userid;
				tpssm02["REC_REVISE_TIME"] = dateNow;

				sqlstr = "tpssm02.update()";
				tpssm02.Update("LOT_STATUS,REC_REVISOR,REC_REVISE_TIME", "CAST_LOT_NO,FACTORY_DIV");
			}

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT CODE_DESC_5_CONTENT FROM TEP0002  "
					"	WHERE CODE_CLASS = 'PSA6' "
					"	  AND CODE = "
					"			(SELECT BILLET_TYPE "
					"			 FROM TPSSM02 "
					"			 WHERE CAST_LOT_NO	= @tpssm01.CAST_LOT_NO) ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Clear();
			cmd_inq.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				v_code_desc_5_content = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			////Log::Info("", __FUNCTION__, "v_code_desc_5_content = [{0}]", v_code_desc_5_content);

			//if (v_code_desc_5_content == "P")
			if (v_code_desc_5_content == "P" && tpssm01["SLAB_DEST"].ToString()!="11")//非热轧去向，热轧按LOT，厚板按PONO
			{

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

					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["PONO_SLAB"] = tpssm03["SLAB_NO"];
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["PONO"] = tpssm03["PONO"];

					

					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["EVENT_ID"] = "PSA1";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["EVENT_DESC"] = "炉次确定未产出";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["SYSTEM_ID"] = "PSSM";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["FUNC_ID"] = "cm_200009_rcv";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["FORM_CODE"] = "200009";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["MAT_KIND"] = "SM";
					bcls_rec_dele_resv.Tables["QMZSBlock"].Rows[0]["MAT_NO"] = tpssm03["SLAB_NO"];

					////Log::Trace("", __FUNCTION__, "★★★★★f_qmtqhp_dele_resv_chg_new start★★★★★");
					ret = f_qmtqhp_dele_resv_chg_new(&bcls_rec_dele_resv, &bcls_ret_dele_resv, conn);

					if (ret != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

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

				sqlstr = " UPDATE TPSSM03 "
					"  SET  SLAB_PROD_FLAG = '9' "
					" WHERE SLAB_PROD_FLAG = '0' "
					"   AND FACTORY_DIV = @tpssm01.FACTORY_DIV "
					"   AND PONO = @tpssm01.PONO ";

				cmd_tpssm03_upd.SetCommandText(sqlstr);
				cmd_tpssm03_upd.Parameters.Clear();
				cmd_tpssm03_upd.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm03_upd.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
				cmd_tpssm03_upd.ExecuteNonQuery();

				inBlock_pmconfm.Tables["PONOCONFM"].Rows.Add();
				inBlock_pmconfm.Tables["PONOCONFM"].Rows[row_num_p]["PONO"] = tpssm01["PONO"];

				inBlock_mmconfm.Tables["MMSMCONFM"].Rows.Add();
				inBlock_mmconfm.Tables["MMSMCONFM"].Rows[row_num_p]["PONO"] = tpssm01["PONO"];
				row_num_p++;
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
					sqlstr = " SELECT COUNT(1) "
						" FROM TPSSM01 "
						" WHERE CAST_LOT_NO	= @tpssm01.CAST_LOT_NO "
						"   AND PONO_STATUS	< '91' ";
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Clear();
				cmd_inq.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					v_count = cmd_inq.GetInt32(1);
				}
				cmd_inq.Close();

				if (v_count == 0)
				{
					sqlstr = " UPDATE TPSSM03 "
						"  SET  SLAB_PROD_FLAG = '9' "
						" WHERE SLAB_PROD_FLAG = '0' "
						"   AND FACTORY_DIV = @tpssm11.FACTORY_DIV "
						"   AND CAST_LOT_NO = @tpssm01.CAST_LOT_NO ";

					cmd_tpssm03_upd.SetCommandText(sqlstr);
					cmd_tpssm03_upd.Parameters.Clear();
					cmd_tpssm03_upd.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
					cmd_tpssm03_upd.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
					cmd_tpssm03_upd.ExecuteNonQuery();

					inBlock_mmconfm.Tables["MMSMCONFMLOT"].Rows.Add();
					inBlock_mmconfm.Tables["MMSMCONFMLOT"].Rows[row_num_l]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

					inBlock_pmconfm.Tables["LOTCONFM"].Rows.Add();
					inBlock_pmconfm.Tables["LOTCONFM"].Rows[row_num_l]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

					row_num_l++;
				}
			}

			row_num = 0;
			row_num = inBlock_pmconfm.Tables["PONOCONFM"].Rows.get_Count();
			////Log::Info("", __FUNCTION__, "PONOCONFM:row_num=[{0}]", row_num);

			if (row_num > 0)
			{
				doFlag = f_pmom_pono_confm(&inBlock_pmconfm, bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			row_num = 0;
			row_num = inBlock_pmconfm.Tables["LOTCONFM"].Rows.get_Count();
			////Log::Info("", __FUNCTION__, "LOTCONFM:row_num=[{0}]", row_num);

			if (row_num > 0)
			{
				doFlag = f_pmom_lot_confm(&inBlock_pmconfm, bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			row_num = 0;
			row_num = inBlock_mmconfm.Tables["MMSMCONFM"].Rows.get_Count();
			////Log::Info("", __FUNCTION__, "MMSMCONFM:row_num=[{0}]", row_num);

			if (row_num > 0)
			{
				doFlag = f_mmsm_heat_confirm(&inBlock_mmconfm, bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			row_num = 0;
			row_num = inBlock_mmconfm.Tables["MMSMCONFMLOT"].Rows.get_Count();
			////Log::Info("", __FUNCTION__, "MMSMCONFMLOT:row_num=[{0}]", row_num);

			if (row_num > 0)
			{
				doFlag = f_mmsm_heat_confirm_lot(&inBlock_mmconfm, bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}

		//炼钢履历跟踪
		CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
		row99["EVENT_ID"] = "1I"; //状态接收
		row99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
		row99["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
		row99["PONO"] = tpssm01["PONO"];
		row99["PONO_STATUS"] = tpssm01["PONO_STATUS"];

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

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
