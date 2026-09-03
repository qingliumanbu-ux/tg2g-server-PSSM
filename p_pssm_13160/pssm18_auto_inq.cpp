/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2014-3-5
Version:1.0
Description: 出钢计划甘特图查询
Update：
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "CUtils.h"
//程序用头文件













void WriteXmlFile(string DataSetName, string TableName, string strnamespace, int blk, string filename);
int f_pssm_push_time(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
int f_pssm_call_tps_n(CString main_backlog_code, int mode, CDbConnection * conn);
int f_pssm_query(EIClass inblock_condition, EIClass inblock_source, EIClass& outblock_result, CDbConnection * conn);

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
BM2F_ENTERACE(pssm18_auto_inq)

int f_pssm18_auto_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int blkseq = 0;
	int ret = 0;
	int fetchRowCount = 0;

	CString v_factory_div = "";	//炼钢单元号
	CString base_time = "";
	CString cast_lot_no_and_div = "";
	CDecimal  pour_time1 = 0;               /* 连铸机浇注时间*/
	CDecimal  pour_time = 0;               /* 连铸机浇注时间*/
	CString dev_move_start = "";
	CString dev_move_end = "";
	CString dev_code = "";
	CString query_type = "";
	CString v_slab_dest = "";
	CDecimal prod_density = 0; //板坯密度
	int k = 0;
	int mode = 1;
	CString lslab_no = "";
	CString strand_no = "";
	CDecimal seq = 0;
	CString pono = "";
	CString show_flag = "";

	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm03("TPSSM03");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd1_s("TPSSMD1");
	CModel tpssmd1_e("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
	CModel tpssmd3_s("TPSSMD3");
	CModel tpssm19("TPSSM19");
	CModel tpssmd6("TPSSMD6");
	CModel tpssm18("TPSSM18");
	CModel tpssmd9("TPSSMD9");
	CModel tpssmda("TPSSMDA");
	CModel tpssmdh("TPSSMDH");
	CModel tpssmdj("TPSSMDJ");
	CModel tpssmd7("TPSSMD7");
	CModel tep0002("TEP0002");
	CModel tqmts0x("TQMTS0X");
	CModel tapbd006s2n("TAPBD006S2N");
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm21_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssmd4_inq(conn);
	CDbCommand cmd_tpssmd6_inq(conn);
	CDbCommand cmd_tpssmd9_inq(conn);
	CDbCommand cmd_tpssmda_inq(conn);
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);
	CString sqlstr;

	EIClass tb_tpssm11;

	CDataTable tb_tpssm03("TPSSM03");
	CDataTable tb_tpssmd3("TPSSMD3_TEST");
	CDataTable tb_tep0002("TEP0002");

	try
	{
		base_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//----------------------------------------------------------
		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}],query_type=[{1}]", v_factory_div, query_type);

		sqlstr = " SELECT * from  tep0002 where CODE_CLASS='PM16' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(tb_tep0002);
		cmd_inq.Close();

		//---------------------------------------------------
		//设置返回块参数
		//第一块，主计划 1
		blkseq = 1;
		bcls_ret->Tables[blkseq - 1].set_TableName("PLAN");  //计划块
		bcls_ret->Tables[blkseq - 1].Columns.Add(tpssm11);
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "BOF_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CCM_NO");		//为显示颜色用
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "BASE_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "GUIGE2"); //厚*宽*长    
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SG_SIGN");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CI_DIV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "HEAT_NO_FLAG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "C_DIV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO2");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CHECK_FLAG");

		//第二块，子计划 1
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("SUB");  //子计划块
		//bcls_ret->Tables[blkseq-1].Columns.Add(tpssm12); //增加一个12表结构体
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME");//计划冶时
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME");//计划开始时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME");//计划结束时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME_REAL");//实绩开始时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME_REAL");//实绩结束时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "AREA_ID");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO"); //增加一个12表结构体	
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PROC_NO");
		//bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");

		//----------------------------------------------------------
		//查询数据送到前台
		Log::Trace("", __FUNCTION__, "主计划数据查询开始");//wcy 过去24小时内计划查询
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT a.*, b.* FROM(SELECT * FROM TPSSM11 UNION ALL SELECT * FROM TPSSM41) a, (SELECT * FROM TPSSM10 UNION ALL SELECT * FROM TPSSM40) b, (SELECT * FROM TPSSM12 UNION ALL SELECT * FROM TPSSM42) c "
				" WHERE a.FACTORY_DIV = @v_factory_div "
				" and a.factory_div = b.factory_div and a.pono = b.pono AND a.SM_PLAN_NO = c.SM_PLAN_NO AND a.PONO_STATUS >= 20 AND a.PONO_STATUS < 83 AND c.AREA_ID = 3 AND ((c.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR c.END_TIME_REAL = ' ' ) OR a.RUN_STATUS < '53') "
				" ORDER BY a.CAST_NO ASC, a.CAST_DIV_NO ASC "
				);
			break;
		}
		//Log::Trace("", __FUNCTION__, "主计划数据查询开始sqlstr = [{0}]", sqlstr);
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("v_factory_div", v_factory_div.Trim());
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			//plan_num ++;
			k = cmd_tpssm11_inq.Fetch(tpssm11, 1);
			//Log::Trace("", __FUNCTION__, "k = [{0}]", k);
			k = cmd_tpssm11_inq.Fetch(tpssm10, k);
			tpssm11.TrimOrBlank();
			tpssm10.TrimOrBlank();

			//精炼工序的第一个charge_no 必定不为0
			//Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm10["PONO"].ToString());
			/*tep0002["CODE_DESC_1_CONTENT"] = " ";
			tep0002["CODE"] = tpssm10["SLAB_DEST"];
			tep0002["CODE_CLASS"] = "PM16";
			tep0002.Query("CODE, CODE_CLASS");
			v_slab_dest = tep0002["CODE_DESC_1_CONTENT"];*/

			//去向
			v_slab_dest = " ";
			for (int i_step = 0; i_step < tb_tep0002.Rows.get_Count(); i_step++)
			{
				if (tb_tep0002.Rows[i_step]["CODE"].ToString() == tpssm10["SLAB_DEST"].ToString())
				{
					v_slab_dest = tb_tep0002.Rows[i_step]["CODE_DESC_1_CONTENT"].ToString();
				}
			}

			//查询子工序表, 将内容写入相应的列中
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					//" SELECT * FROM ( SELECT * FROM TPSSM12 UNION ALL SELECT * FROM TPSSM42 ) "
					//"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					//"    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					//"    AND SUB_CHARGE_NO = 0 "  //0-主工序. 对甘特图只读取主工序的.
					//"  ORDER BY CHARGE_NO ASC "

					" SELECT * FROM ( SELECT * FROM TPSSM12 "
					" WHERE SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					" UNION ALL SELECT * FROM TPSSM42 "
					" WHERE SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					" ) "
					"  ORDER BY CHARGE_NO ASC "
					);
				break;
			}
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			while (cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);
				tpssm12.TrimOrBlank();

				//Log::Trace("", __FUNCTION__, "sqlstr = [{0}],[{1}],[{2}]", tpssm12["SM_PLAN_NO"].ToString().Trim(),tpssm12["START_TIME_REAL"].ToString().Trim(), tpssm12["END_TIME_REAL"].ToString().Trim());

				//如果实绩结束时间有但是实绩开始时间没有，把计划开始时间传给实绩开始时间				
				if (tpssm12["END_TIME_REAL"].ToString().Compare(" ") != 0 && tpssm12["START_TIME_REAL"].ToString().Compare(" ") == 0)
				{
					tpssm12["START_TIME_REAL"] = tpssm12["START_TIME"];
				}

				//回炉的特殊处理
				if (tpssm11["STEEL_RETURN_CODE"].ToString().Trim() == "1")
				{
					//回炉处理，时间不更改，看看会有什么问题
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "" && tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{
						tpssm12["START_TIME"] = "19801124080000";
						tpssm12["END_TIME"] = "19801124081000";
					}
				}
				CDataRow & row_sub = bcls_ret->Tables["SUB"].Rows.Add();
				row_sub.Merge(tpssm12);
				row_sub["PONO"] = tpssm11["PONO"];
				//row_sub["SLAB_DEST"] = v_slab_dest;

			}
			cmd_tpssm12_inq.Close();


			//给第一块赋值
			CDataRow& row_plan = bcls_ret->Tables["PLAN"].Rows.Add();   //新增空行
			row_plan["BASE_TIME"] = base_time;
			row_plan.Merge(tpssm11);

			//读取重引锭标记
			row_plan["RESTRAND_FLG"] = (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" ? 1 : 0);
			row_plan["SG_SIGN"] = tpssm10["SG_SIGN"].ToString();
			//板坯规格信息
			row_plan["GUIGE2"] = tpssm10["SLAB_THICK"].ToDecimal().Round(0).ToString() + "*" + tpssm10["SLAB_WIDTH"].ToDecimal().Round(0).ToString() + "*" + tpssm10["SLAB_LEN"].ToDecimal().Round(0).ToString();

			row_plan["SLAB_DEST"] = v_slab_dest;

			row_plan["CI_DIV"] = tpssm11["BACKLOG_EA"].ToString().Substring((tpssm11["BACKLOG_EA"].ToString().GetLength() - 1), 1);
			row_plan["C_DIV"] = tpssm10["C_DIV"];
			cast_lot_no_and_div = tpssm10["CAST_LOT_NO"].ToString() + "-" + tpssm10["CAST_LOT_DIV_NO"].ToString();
			row_plan["CAST_LOT_NO"] = cast_lot_no_and_div;
			cast_lot_no_and_div = tpssm10["CAST_LOT_NO2"].ToString() + "-" + tpssm10["CAST_LOT_DIV_NO2"].ToString();
			row_plan["CAST_LOT_NO2"] = cast_lot_no_and_div;
			row_plan["CHECK_FLAG"] = " ";
		}
		cmd_tpssm11_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
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
	cmd_tpssm12_inq.Close();
	cmd_tpssm10_inq.Close();
	cmd_tpssm11_inq.Close();
	cmd_tpssmd1_inq.Close();
	cmd_tpssm03_inq.Close();
	cmd_tpssm02_inq.Close();
	cmd_tpssmda_inq.Close();
	cmd_tpssm21_inq.Close();
	cmd_inq.Close();

	return doFlag;

}
