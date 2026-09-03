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

//程序用头文件














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
BM2F_ENTERACE(pssm41_inq)

int f_pssm41_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量

	int doFlag = 0;
	int blkseq = 0;
	int fetchRowCount = 0;

	CString v_factory_div = "";	//炼钢单元号
	CString base_time = "";
	CString v_start_time = "";
	CString v_end_time = "";
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

	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm03("TPSSM03");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
	CModel tpssm19("TPSSM19");
	CModel tpssmd6("TPSSMD6");
	CModel tpssm18("TPSSM18");
	CModel tpssmd9("TPSSMD9");
	CModel tpssmda("TPSSMDA");
	CModel tep0002("TEP0002");
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
	CString sqlstr;

	try
	{
		v_start_time = bcls_rec->Tables[0].Rows[0]["PROD_DATE_FROM"];
		v_end_time = bcls_rec->Tables[0].Rows[0]["PROD_DATE_TO"];

		base_time = v_start_time;
		EDLog(1, 1, "base_time = [%s]", (const char*)base_time);
		//----------------------------------------------------------
		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		query_type = bcls_rec->Tables[0].Rows[0]["QUERY_TYPE"].ToString().Trim();
		////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}],query_type=[{1}]", v_factory_div, query_type);


		//---------------------------------------------------
		//设置返回块参数
		//第一块，主计划
		blkseq = 1;
		bcls_ret->Tables[blkseq - 1].set_TableName("PLAN");  //计划块
		bcls_ret->Tables[blkseq - 1].Columns.Add(tpssm11);
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "BOF_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CCM_NO");		//为显示颜色用
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "BASE_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "GUIGE2"); //厚*宽*长    
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SG_SIGN");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");

		//第二块，子计划
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("SUB");  //子计划块
		bcls_ret->Tables[blkseq - 1].Columns.Add(tpssm12); //增加一个12表结构体
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO"); //增加一个12表结构体		
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");

		//第三块，设备代码
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("DEV");  //设备信息
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");  //设备代码
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_NAME"); //设备名称
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TYPE");  //设备类型:细分同一类型设备的工艺区分
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CLASS_ID");  //区域标识
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "NUMB");      //甘特图显示顺序

		//第四块，制造命令相关
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("PONO");  //制造命令相关
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_MACH_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "REFINE_ROUTE_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_MODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_DIV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "RESTRAND_FLG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_SEQ");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "GUIGE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "POUR_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO_PLAN_DATE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SG_SIGN");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");

		//第五块，设备维修
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("STOP");  //设备维修
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_STATUS_REMARK");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STOP_FLAG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STATUS_AREA");

		//增加第六块 根据钢种提供的各设备处理时间
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("PROC_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_FLAG");//用来区分双联和常规的脱碳处理时间、脱磷时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PROC_TIME");

		//增加第七块 传搁时间 （考虑在画面载入时传入）
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("MOVE_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_DEV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_DEV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "MOVE_TIME");

		//增加第八块 连铸的工艺时间 
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("CC_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_MACH_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "BILLET_TYPE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME_W0");//浇次间准备时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME_W1");//浇次间准备时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME_IN");//浇次内准备时间

		//增加第九块 设备定检修类型
		bcls_ret->Tables.Add("DEV_SET");
		bcls_ret->Tables["DEV_SET"].Columns.Add(tpssm19);


		//----------------------------------------------------------
		//查询当前计划中最早执行的时间
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT MIN(START_TIME_REAL), MIN(START_TIME) "
				"   FROM TPSSM12 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				);
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
			tpssm12["START_TIME_REAL"] = cmd_tpssm12_inq.GetString(1);
			tpssm12["START_TIME"] = cmd_tpssm12_inq.GetString(2);
		}
		else
		{
			tpssm12["START_TIME_REAL"] = " ";
			tpssm12["START_TIME"] = " ";
		}
		cmd_tpssm12_inq.Close();

		//----------------------------------------------------------
		//查询数据送到前台
		////Log::Trace("", __FUNCTION__, "主计划数据查询开始");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			sqlstr = CString(
				" SELECT * FROM ( "
				" SELECT a.*, b.* FROM TPSSM11 a, TPSSM10 b "
				"  WHERE a.FACTORY_DIV = @v_factory_div "
				"	 AND a.FACTORY_DIV = b.FACTORY_DIV and a.PONO = b.PONO "
				"	 AND a.SM_PLAN_NO IN (SELECT SM_PLAN_NO FROM TPSSM12 WHERE AREA_ID ='3' AND START_TIME <= @v_end_time AND START_TIME >= @v_start_time) "
				" UNION "
				" SELECT c.*, d.* FROM TPSSM41 c, TPSSM40 d "
				"  WHERE c.FACTORY_DIV = @v_factory_div "
				"	 and c.FACTORY_DIV = d.FACTORY_DIV and c.PONO = d.PONO "
				"	 AND c.SM_PLAN_NO IN (SELECT SM_PLAN_NO FROM TPSSM42 WHERE AREA_ID ='3' AND START_TIME <= @v_end_time AND START_TIME >= @v_start_time)) "
				"  ORDER BY CAST_NO ASC, CAST_DIV_NO ASC "
				);
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				//" SELECT * FROM ( "
				" SELECT a.*, b.* FROM TPSSM11 a, TPSSM10 b "
				"  WHERE a.FACTORY_DIV = @v_factory_div "
				"	 AND a.FACTORY_DIV = b.FACTORY_DIV and a.PONO = b.PONO "
				"	 AND a.SM_PLAN_NO IN (SELECT SM_PLAN_NO FROM TPSSM12 WHERE AREA_ID ='3' AND START_TIME <= @v_end_time AND START_TIME >= @v_start_time) "
				" UNION "
				" SELECT c.*, d.* FROM TPSSM41 c, TPSSM40 d "
				"  WHERE c.FACTORY_DIV = @v_factory_div "
				"	 and c.FACTORY_DIV = d.FACTORY_DIV and c.PONO = d.PONO "
				"	 AND c.SM_PLAN_NO IN (SELECT SM_PLAN_NO FROM TPSSM42 WHERE AREA_ID ='3' AND START_TIME <= @v_end_time AND START_TIME >= @v_start_time) "
				//"  )ORDER BY CAST_NO ASC, CAST_DIV_NO ASC "
				);

			break;
		}
		////Log::Trace("", __FUNCTION__, "主计划数据查询开始sqlstr = [{0}]", sqlstr);
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("v_factory_div", v_factory_div.Trim());
		cmd_tpssm11_inq.Parameters.Set("v_start_time", v_start_time.Trim());
		cmd_tpssm11_inq.Parameters.Set("v_end_time", v_end_time.Trim());
		cmd_tpssm11_inq.ExecuteReader();
		while (cmd_tpssm11_inq.Read())
		{
			//plan_num ++;
			k = cmd_tpssm11_inq.Fetch(tpssm11, 1);
			k = cmd_tpssm11_inq.Fetch(tpssm10, k);
			tpssm11.TrimOrBlank();
			tpssm10.TrimOrBlank();
			////Log::Trace("", __FUNCTION__, "k = [{0}]", k);

			//精炼工序的第一个charge_no 必定不为0
			////Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm10["PONO"].ToString());
			tep0002["CODE_DESC_1_CONTENT"] = " ";
			tep0002["CODE"] = tpssm10["SLAB_DEST"];
			tep0002["CODE_CLASS"] = "PM16";
			tep0002.Query("CODE, CODE_CLASS");
			v_slab_dest = tep0002["CODE_DESC_1_CONTENT"];

			//查询子工序表, 将内容写入相应的列中
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM ("
					" SELECT * FROM TPSSM12 "
					"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					"    AND SUB_CHARGE_NO = 0 "  //0-主工序. 对甘特图只读取主工序的.
					" UNION "
					" SELECT * FROM TPSSM42 "
					"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					"    AND SUB_CHARGE_NO = 0) "  //0-主工序. 对甘特图只读取主工序的.
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

				//如果实绩结束时间有但是实绩开始时间没有，把计划开始时间传给实绩开始时间				
				if (tpssm12["END_TIME_REAL"].ToString().Compare(" ") != 0 && tpssm12["START_TIME_REAL"].ToString().Compare(" ") == 0)
				{
					tpssm12["START_TIME_REAL"] = tpssm12["START_TIME"];
				}

				//回炉的特殊处理
				if (tpssm11["STEEL_RETURN_CODE"].ToString().Trim() == "1")
				{
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "" && tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{
						tpssm12["START_TIME"] = "19801124080000";
						tpssm12["END_TIME"] = "19801124081000";
					}
				}
				CDataRow & row_sub = bcls_ret->Tables["SUB"].Rows.Add();
				row_sub.Merge(tpssm12);
				row_sub["PONO"] = tpssm11["PONO"];
				row_sub["SLAB_DEST"] = v_slab_dest;

			}
			cmd_tpssm12_inq.Close();


			//给第一块赋值
			CDataRow& row_plan = bcls_ret->Tables["PLAN"].Rows.Add();   //新增空行
			row_plan["BASE_TIME"] = base_time;
			row_plan.Merge(tpssm11);

			//读取重引锭标记
			row_plan["RESTRAND_FLG"] = (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" ? 1 : 0);
			row_plan["SG_SIGN"] = tpssm10["SG_SIGN"];
			//板坯规格信息
			row_plan["GUIGE2"] = tpssm10["SLAB_THICK"].ToDecimal().ToString() + "*" + tpssm10["SLAB_WIDTH"].ToDecimal().ToString() + "*" + tpssm10["SLAB_LEN"].ToDecimal().ToString();

			row_plan["SLAB_DEST"] = v_slab_dest;
		}
		cmd_tpssm11_inq.Close();

		//----------------------------------------------------------
		//第三块，设备代码
		////Log::Trace("", __FUNCTION__, "设备代码查询");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * FROM TPSSMD1 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND AREA_ID    >= 3 "  //从转炉脱C
				" ORDER BY AREA_ID, DEV_TECH_CODE, STATION_NO "
				);
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssmd1_inq.ExecuteReader();
		fetchRowCount = 0;
		while (cmd_tpssmd1_inq.Read())
		{
			cmd_tpssmd1_inq.Fetch(tpssmd1);
			fetchRowCount++;
			tpssmd1.TrimOrBlank();
			CDataRow & row_dev = bcls_ret->Tables["DEV"].Rows.Add();

			row_dev["DEV_TYPE"] = tpssmd1["DEV_TECH_CODE"];
			row_dev["CLASS_ID"] = tpssmd1["AREA_ID"];
			row_dev["DEV_CODE"] = tpssmd1["DEV_CODE"];
			row_dev["STATION_NAME"] = tpssmd1["STATION_NAME"];
			row_dev["NUMB"] = fetchRowCount;
		}
		cmd_tpssmd1_inq.Close();

		//----------------------------------------------------------
		//第四块，制造命令相关
		////Log::Trace("", __FUNCTION__, "制造命令查询");
		if (query_type == "2") //查询制造命令信息，第4块
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT A.*, NVL(B.CODE_DESC_1_CONTENT, ' ') FROM TPSSM10 A "
					"	LEFT OUTER JOIN (SELECT * FROM TEP0002 WHERE CODE = 'PM16') B "
					"	ON A.SLAB_DEST = B.CODE "
					"  WHERE A.FACTORY_DIV = @v_factory_div "
					"    AND A.PONO_STATUS IN (15, 16) "
					"  ORDER BY A.CC_MACH_NO, A.CC_SEQ ASC "  //因没有浇铸顺画面，暂时用此方式排序
					);
				break;
			}
			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_tpssm10_inq.ExecuteReader();
			while (cmd_tpssm10_inq.Read())
			{
				k = cmd_tpssm10_inq.Fetch(tpssm10, 1);
				v_slab_dest = cmd_tpssm10_inq.GetString(k);
				//精炼工序的第一个charge_no 必定不为0
				////Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm10["PONO"].ToString());
				////Log::Info("", __FUNCTION__, "v_slab_dest =[{0}]", v_slab_dest);

				tpssm10.TrimOrBlank();
				CDataRow& row_pono = bcls_ret->Tables["PONO"].Rows.Add();   //新增空行

				//命令数据赋值
				row_pono["PONO"] = tpssm10["PONO"];
				row_pono["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];
				row_pono["ST_NO"] = tpssm10["ST_NO"];
				row_pono["REFINE_ROUTE_CODE"] = tpssm10["REFINE_DIV"];
				row_pono["SMELT_MODE"] = tpssm10["SMELT_MODE"];
				row_pono["SMELT_DIV"] = tpssm10["SMELT_DIV"];
				row_pono["RESTRAND_FLG"] = (tpssm10["RESTRAND_FLG"].ToString().Trim() == "T" ? 1 : 0);
				row_pono["CC_SEQ"] = tpssm10["CC_SEQ"];
				row_pono["PONO_PLAN_DATE"] = tpssm10["PLAN_DATE"];
				row_pono["SG_SIGN"] = tpssm10["SG_SIGN"];
				row_pono["GUIGE"] = tpssm10["SLAB_THICK"].ToDecimal().ToString() + "*" + tpssm10["SLAB_WIDTH"].ToDecimal().ToString() + "*" + tpssm10["SLAB_LEN"].ToDecimal().ToString();

				cast_lot_no_and_div.Format("%6.6s%-02d", (const char*)tpssm10["CAST_LOT_NO"].ToString(), tpssm10["CAST_LOT_DIV_NO"].ToDecimal().ToInt32());
				row_pono["CAST_LOT_NO"] = cast_lot_no_and_div;
				row_pono["POUR_TIME"] = tpssm10["POUR_TIME"];
				row_pono["SLAB_DEST"] = v_slab_dest;

				prod_density = 7.85;
				tpssmd9["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];
				tpssmd9["BILLET_TYPE"] = tpssm10["BILLET_TYPE"];
				tpssmd9["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssmd9.Query("FACTORY_DIV,BILLET_TYPE,CC_MACH_NO");

				tpssmda["CAST_SPEED"] = 0;
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT CAST_SPEED FROM TPSSMDA \
							 							 								WHERE FACTORY_DIV	= @v_factory_div \
																																													AND CC_MACH_NO		= @tpssm10.CC_MACH_NO \
																																																																												AND BILLET_TYPE		= @tpssm10.BILLET_TYPE \
																																																																																																																			AND ST_NO			= @tpssm10.ST_NO \
																																																																																																																																																																		AND CAST_WIDTH_MIN	<= @tpssm10.SLAB_WIDTH \
																																																																																																																																																																																																																									AND CAST_WIDTH_MAX	>= @tpssm10.SLAB_WIDTH \
																																																																																																																																																																																																																																																																																								AND CAST_THICK_MIN	<= @tpssm10.SLAB_THICK \
																																																																																																																																																																																																																																																																																																																																																															AND CAST_THICK_MAX	>= @tpssm10.SLAB_THICK ";
					break;
				}

				cmd_tpssmda_inq.SetCommandText(sqlstr);
				cmd_tpssmda_inq.Parameters.Set("v_factory_div", v_factory_div);
				cmd_tpssmda_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssmda_inq.Parameters.Set("tpssm10.BILLET_TYPE", tpssm10["BILLET_TYPE"].ToString());
				cmd_tpssmda_inq.Parameters.Set("tpssm10.ST_NO", tpssm10["ST_NO"].ToString());
				cmd_tpssmda_inq.Parameters.Set("tpssm10.SLAB_WIDTH", tpssm10["SLAB_WIDTH"].ToDecimal());
				cmd_tpssmda_inq.Parameters.Set("tpssm10.SLAB_THICK", tpssm10["SLAB_THICK"].ToDecimal());
				cmd_tpssmda_inq.ExecuteReader();
				if (cmd_tpssmda_inq.Read())
				{
					tpssmda["CAST_SPEED"] = cmd_tpssmda_inq.GetDecimal(1);
				}
				cmd_tpssmda_inq.Close();
				////Log::Info("", __FUNCTION__, "tpssmda["CAST_SPEED"] = {0},tpssm10["PLAN_TAP_WT"] = {1},tpssm10["SLAB_THICK"] ={2},tpssm10["SLAB_WIDTH"] = {3}"
					//, tpssmda["CAST_SPEED"].ToDecimal(), tpssm10["PLAN_TAP_WT"].ToDecimal(), tpssm10["SLAB_THICK"].ToDecimal(), tpssm10["SLAB_WIDTH"].ToDecimal());

				if (tpssmda["CAST_SPEED"].ToDecimal() == 0 || tpssm10["PLAN_TAP_WT"].ToDecimal() == 0 || tpssm10["SLAB_THICK"].ToDecimal() == 0 || tpssm10["SLAB_WIDTH"].ToDecimal() == 0 || prod_density == 0)
				{
					////Log::Info("", __FUNCTION__, "tpssm10["CC_MACH_NO"] =[{0}]", tpssm10["CC_MACH_NO"].ToString());

					pour_time1 = 40;
				}
				else
				{
					pour_time1 = (tpssm10["PLAN_TAP_WT"].ToDecimal() * 1000 * 1000 * 1000) / (prod_density * tpssm10["SLAB_THICK"].ToDecimal() * tpssm10["SLAB_WIDTH"].ToDecimal() * tpssmda["CAST_SPEED"].ToDecimal() *tpssmd9["STRAND_NUM"].ToDecimal());	//
					////Log::Info("", __FUNCTION__, "计算得出浇铸时间 pour_time=[{0}]", pour_time1);
				}
				pour_time = pour_time1.ToInt32();
				row_pono["POUR_TIME"] = pour_time;

			}
			cmd_tpssm10_inq.Close();

		}
		//-------------------------------------------------------
		//第五块，设备维修
		////Log::Trace("", __FUNCTION__, "设备特殊状态");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * FROM TPSSM18 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND DEV_STATUS = '1'"
				);
			// lj删除 20150918
			break;
		}

		cmd_tpssm21_inq.SetCommandText(sqlstr);
		cmd_tpssm21_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm21_inq.ExecuteReader();
		while (cmd_tpssm21_inq.Read())
		{
			cmd_tpssm21_inq.Fetch(tpssm18);
			tpssm18.TrimOrBlank();

			CDataRow & row_stop = bcls_ret->Tables["STOP"].Rows.Add();
			row_stop["DEV_CODE"] = tpssm18["DEV_CODE"];
			row_stop["START_TIME"] = tpssm18["START_TIME"];
			row_stop["END_TIME"] = tpssm18["END_TIME"];
			row_stop["DEV_STATUS_REMARK"] = tpssm18["DEV_STATUS_REMARK"];
			row_stop["STOP_FLAG"] = tpssm18["STOP_FLAG"];
			row_stop["STATUS_AREA"] = tpssm18["AREA_ID"];
		}
		cmd_tpssm21_inq.Close();


		//-------------------------------------------------------
		//第六块 各工序设备处理时间
		////Log::Trace("", __FUNCTION__, "设备处理时间");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT DISTINCT(ST_NO) FROM TPSSM10 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND PONO_STATUS < 83 "
				);
			break;
		}
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm10_inq.ExecuteReader();
		while (cmd_tpssm10_inq.Read())
		{
			tpssm10["ST_NO"] = cmd_tpssm10_inq.GetString(1);
			////Log::Trace("", __FUNCTION__, "查询出钢记号ST_NO=[{0}]", tpssm10["ST_NO"].ToString());

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				/*sqlstr = CString(
				" SELECT DISTINCT d3.ST_NO, d1.DEV_CODE, d3.STD_PROC_TIME, d1.AREA_ID, d3.SMELT_MODE "
				"   FROM TPSSMD3 d3,  TPSSMD1 d1 "
				"  WHERE d3.ST_NO         = @st_no "
				"    AND d3.DEV_TECH_CODE = d1.DEV_TECH_CODE "

				);*/
				sqlstr = CString(
					" SELECT DISTINCT d3.ST_NO, d1.DEV_CODE, d3.STD_PROC_TIME, d1.AREA_ID, d3.SMELT_MODE "
					"   FROM TPSSMD3 d3,  TPSSMD1 d1 "
					"  WHERE d3.ST_NO         = @st_no "
					"    AND d3.DEV_CODE = d1.DEV_CODE  "
					"   AND d3.factory_div = d1.factory_div "
					"   AND d1.factory_div = @tpssm10.factory_div"
					);
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tpssm10["ST_NO"].ToString());
			cmd_inq.Parameters.Set("tpssm10.factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			////Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
			while (cmd_inq.Read())
			{

				tpssmd3["ST_NO"] = cmd_inq.GetString(1);
				tpssmd1["DEV_CODE"] = cmd_inq.GetString(2);
				tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(3);
				tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(4);
				tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(5);

				/*////Log::Trace("", __FUNCTION__, "while里查询到tpssmd3["STD_PROC_TIME"] =[{0}]", tpssmd3["STD_PROC_TIME"].ToDecimal());
				////Log::Trace("", __FUNCTION__, "while里查询到DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
				////Log::Trace("", __FUNCTION__, "while里查询到tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());
				*/


				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] = tpssmd3["ST_NO"];
				row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0")
					row_dt["SMELT_FLAG"] = "1";//常规
				else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "2")
					row_dt["SMELT_FLAG"] = "2";//双联脱磷处理时间
				else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "3")
					row_dt["SMELT_FLAG"] = "3";//双联脱碳处理时间
			}
			cmd_inq.Close();
		}
		cmd_tpssm10_inq.Close();

		////Log::Trace("", __FUNCTION__, "查询Rows=[{0}]", bcls_ret->Tables["PROC_TIME"].Rows.get_Count());
		////Log::Trace("", __FUNCTION__, "查询DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
		////Log::Trace("", __FUNCTION__, "查询tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());

		//没有查询到记录时，读取全部?????????
		if (bcls_ret->Tables["PROC_TIME"].Rows.get_Count() < 1)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				/*sqlstr = " SELECT T.ST_NO,A.DEV_CODE,C.STD_PROC_TIME ,A.AREA_ID, C.SMELT_MODE  \
				FROM TPSSMD4 T ,TPSSMD1 A ,TPSSMD5A C  \
				WHERE T.DEV_TECH_CODE = A.DEV_TECH_CODE \
				AND T.PTN_NO            = C.PTN_NO \
				AND T.DEV_TECH_CODE = C.DEV_TECH_CODE ";*/

				sqlstr = " SELECT T.ST_NO,A.DEV_CODE,T.STD_PROC_TIME ,A.AREA_ID, T.SMELT_MODE  \
						 						 									FROM TPSSMD3 T ,TPSSMD1 A   \
																																														WHERE T.DEV_CODE = A.DEV_CODE"
																																														"   AND T.factory_div = A.factory_div "
																																														"   AND T.factory_div = @tpssm10.factory_div"
																																														;


				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tpssm10.factory_div", v_factory_div);

			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				tpssmd3["ST_NO"] = cmd_inq.GetString(1);
				tpssmd1["DEV_CODE"] = cmd_inq.GetString(2);
				tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(3);
				tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(4);
				tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(5);

				/*////Log::Trace("", __FUNCTION__, "用st_no查不到while里查询到tpssmd3["STD_PROC_TIME"] =[{0}]", tpssmd3["STD_PROC_TIME"].ToDecimal());
				////Log::Trace("", __FUNCTION__, "while里查询到DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
				////Log::Trace("", __FUNCTION__, "while里查询到tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());
				*/

				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] = tpssmd3["ST_NO"];
				row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0")
					row_dt["SMELT_FLAG"] = "1";//常规
				else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "2")
					row_dt["SMELT_FLAG"] = "2";//双联脱磷处理时间
				else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "3")
					row_dt["SMELT_FLAG"] = "3";//双联脱碳处理时间
			}
			cmd_inq.Close();
		}

		////增加默认处理时间		
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default:
		//	/*sqlstr = " SELECT DISTINCT A.DEV_CODE,C.STD_PROC_TIME ,A.AREA_ID, C.SMELT_MODE  \
						//				FROM  TPSSMD1 A ,TPSSMD5 C  \
						//				WHERE C.DEV_TECH_CODE = A.DEV_TECH_CODE \
						//				AND C.PTN_NO            = '1' ";*/

		//	sqlstr = " SELECT DISTINCT A.DEV_CODE,C.STD_PROC_TIME ,A.AREA_ID, C.SMELT_MODE  \
						//				FROM  TPSSMD1 A ,TPSSMD3 C  \
						//				WHERE C.DEV_CODE = A.DEV_CODE \
						//				AND ROWNUM=1 ";

		//	break;
		//}

		//cmd_inq.SetCommandText(sqlstr);
		//////Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
		//cmd_inq.ExecuteReader();
		//while (cmd_inq.Read())
		//{			
		//	tpssmd1["DEV_CODE"] = cmd_inq.GetString(1);
		//	tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(2);
		//	tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(3);
		//	tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(4);

		//	////Log::Trace("", __FUNCTION__, "默认值--查询到tpssmd3["STD_PROC_TIME"] =[{0}]", tpssmd3["STD_PROC_TIME"].ToDecimal());
		//	////Log::Trace("", __FUNCTION__, "while里查询到DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
		//	////Log::Trace("", __FUNCTION__, "while里查询到tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());

		//	CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
		//	row_dt["ST_NO"] = "DEFAULT";
		//	row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
		//	row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
		//	if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0")
		//		row_dt["SMELT_FLAG"] = "1";//常规
		//	else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "2")
		//		row_dt["SMELT_FLAG"] = "2";//双联脱磷处理时间
		//	else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "3")
		//		row_dt["SMELT_FLAG"] = "3";//双联脱碳处理时间
		//}
		//cmd_inq.Close();
		//

		//-------------------------------------------------------
		//第七块  传搁时间
		////Log::Trace("", __FUNCTION__, "设备传搁时间");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DEV_MOVE_START,MOVE_TIME,DEV_MOVE_END FROM TPSSMD6 \
					 					 					 	WHERE FACTORY_DIV = @v_factory_div  ";
			break;
		}

		cmd_tpssmd6_inq.SetCommandText(sqlstr);
		cmd_tpssmd6_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssmd6_inq.ExecuteReader();
		while (cmd_tpssmd6_inq.Read())
		{
			tpssmd6["DEV_MOVE_START"] = cmd_tpssmd6_inq.GetString(1);
			tpssmd6["MOVE_TIME"] = cmd_tpssmd6_inq.GetDecimal(2);
			tpssmd6["DEV_MOVE_END"] = cmd_tpssmd6_inq.GetString(3);

			CDataRow & row_mt = bcls_ret->Tables["MOVE_TIME"].Rows.Add();
			row_mt["START_DEV"] = tpssmd6["DEV_MOVE_START"];
			row_mt["END_DEV"] = tpssmd6["DEV_MOVE_END"];
			row_mt["MOVE_TIME"] = tpssmd6["MOVE_TIME"];

		}
		cmd_tpssmd6_inq.Close();

		//-------------------------------------------------------
		//第八块  连铸工艺时间
		////Log::Trace("", __FUNCTION__, "连铸工艺时间");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT * FROM TPSSMD9 \
					 					 					 	WHERE FACTORY_DIV = @v_factory_div  ";
			break;
		}

		cmd_tpssmd9_inq.SetCommandText(sqlstr);
		cmd_tpssmd9_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssmd9_inq.ExecuteReader();
		while (cmd_tpssmd9_inq.Read())
		{
			cmd_tpssmd9_inq.Fetch(tpssmd9);

			CDataRow & row_ct = bcls_ret->Tables["CC_TIME"].Rows.Add();
			row_ct["CC_MACH_NO"] = tpssmd9["CC_MACH_NO"];
			row_ct["BILLET_TYPE"] = tpssmd9["BILLET_TYPE"];
			row_ct["PREP_TIME_W0"] = tpssmd9["TT_PREP_W0_CAST"]; //无调宽
			row_ct["PREP_TIME_W1"] = tpssmd9["TT_PREP_W1_CAST"];//有调宽
			row_ct["PREP_TIME_IN"] = tpssmd9["TT_PREP_LAST_2CH"];//浇次内准备时间

		}
		cmd_tpssmd9_inq.Close();

		//-------------------------------------------------------
		//第九块 设备定检修类型配置 
		////Log::Trace("", __FUNCTION__, "设备定检修类型配置");


		sqlstr = " SELECT * FROM TPSSM19 \
				 				 					 	WHERE FACTORY_DIV = @v_factory_div \
																														ORDER BY AREA_ID ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm19);
			if (tpssm19["AREA_ID"].ToDecimal() == 2) //2、3一般是同一个物理设备
			{
				tpssm19["AREA_ID"] = 3;
			}
			CDataRow & row_19 = bcls_ret->Tables["DEV_SET"].Rows.Add();
			row_19.Merge(tpssm19);
			tpssm19.Print();
		}
		cmd_inq.Close();

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
