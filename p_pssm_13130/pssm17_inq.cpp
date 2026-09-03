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
BM2F_ENTERACE(pssm17_inq)

int f_pssm17_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int blkseq = 0;
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
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssm17("TPSSM17");
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm17_inq(conn);
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

	CDataTable tb_tpssm03("TPSSM03");
	//CDataTable tb_tpssm03("TPSSM03");
	CDataTable tb_tpssmd3("TPSSMD3_TEST");
	CDataTable tb_tep0002("TEP0002");//保存去向小代码

	try
	{
		base_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//----------------------------------------------------------
		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		query_type = bcls_rec->Tables[0].Rows[0]["QUERY_TYPE"].ToString().Trim();
		show_flag = bcls_rec->Tables[0].Rows[0]["SHOW_FLAG"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}],query_type=[{1}]", v_factory_div, query_type);

		EIClass TPSSMD3_SOURCE2;
		EIClass TPSSMD3_CONDI2;
		TPSSMD3_CONDI2.Tables[0].Columns.Add(DT_STRING, "ST_NO");
		TPSSMD3_CONDI2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		TPSSMD3_CONDI2.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
		TPSSMD3_CONDI2.Tables[0].Columns.Add(DT_STRING, "SMELT_MODE");
		TPSSMD3_CONDI2.Tables[0].Columns.Add(DT_STRING, "SMELT_MODE2");
		TPSSMD3_CONDI2.Tables[0].Rows.Add();
		EIClass TPSSMD3_RESULT2;

		sqlstr = "SELECT * FROM TPSSMD3 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD3_SOURCE2.Tables[0]);
		cmd_inq.Close();

		//查询去向小代码
		sqlstr = " SELECT * from  tep0002 where CODE_CLASS='PM16' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(tb_tep0002);
		cmd_inq.Close();
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
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CI_DIV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "HEAT_NO_FLAG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PLAN_FLAG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "COST_HJ");

		//第二块，子计划
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("SUB");  //子计划块
		//bcls_ret->Tables[blkseq-1].Columns.Add(tpssm12); //增加一个12表结构体
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME_REAL");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME_REAL");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "AREA_ID");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO"); //增加一个12表结构体		
		//bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");

		//第三块，设备代码
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("DEV");  //设备信息
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");  //设备代码
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_NAME"); //设备名称
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TYPE");  //设备类型:细分同一类型设备的工艺区分
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CLASS_ID");  //区域标识
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "NUMB");      //甘特图显示顺序bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE");      //上限
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE");      //上限
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "LOWER_LIMIT_VALUE");      //下限

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
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CI_DIV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "HEAT_NO_FLAG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "C_DIV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_REQ_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_REQ_TIMEL4");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTELIST");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTEBAGKEY");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO2");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_THICK");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_WIDTH");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PLAN_TAP_WT");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO_00");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_DIV_NO_00");

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
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_FLAG2");//wcy 用来区分AOD冶炼模式
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME");

		//增加第七块 传搁时间 （考虑在画面载入时传入）
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("MOVE_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_DEV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_DEV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "MOVE_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "TRAN_TYPE");

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

		//增加第十块 钢种-路径包
		bcls_ret->Tables.Add("ST_NO_ROUTEBAG");
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "ST_NO");//钢种
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "ROUTEBAGKEY");//路径包
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "REMARK");//注释
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_DECIMAL, "COST_ST_LINE");//成本

		//增加第十一块 路径包-路径
		bcls_ret->Tables.Add("ROUTEBAG_ROUTE");
		bcls_ret->Tables["ROUTEBAG_ROUTE"].Columns.Add(DT_STRING, "ROUTEBAGKEY");//路径包
		bcls_ret->Tables["ROUTEBAG_ROUTE"].Columns.Add(DT_STRING, "ROUTELIST");//路径
		bcls_ret->Tables["ROUTEBAG_ROUTE"].Columns.Add(DT_DECIMAL, "PLANTSECTIONTYPE");//默认路径（1）

		//增加第十二块 路径-设备列表
		bcls_ret->Tables.Add("ROUTE_DEV");
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "ROUTELIST");//路径
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_DECIMAL, "CHARGE_NO");//路径顺序
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_DECIMAL, "AREA_ID");//区域代码
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "DEV_TECH_CODE");//设备工艺代码（与第3块DEV_TYPE一致）
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "PRE_SOLUTION_FLAG");//预溶液代码
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "FLAG_POS_1");//扒渣标记
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "FLAG_POS_2");//分包标记
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "FLAG_POS_3");//等待标记
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "FLAG_POS_4");//路径标记暂定
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "FLAG_POS_5");//路径标记暂定
		bcls_ret->Tables["ROUTE_DEV"].Columns.Add(DT_STRING, "ROUTE_DIV");

		//增加第十三块 预定板坯号
		bcls_ret->Tables.Add("PONO_SLAB");
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "SLAB_NO");//预定材料号
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "STRAND_NO");//流号
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_THICK");//材料厚度
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_WIDTH");//材料宽度
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_LEN");//材料目标长度
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_MAX_LEN");//材料最大长度
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_MIN_LEN");//材料最小长度
		//bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_WT");//材料重量（t）
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "ORDER_NO");//合同号
		//bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "SLAB_DEST");//材料去向
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "SG_SIGN");//牌号
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_DECIMAL, "SLAB_SEQ_2");//牌号
		//bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "FACTORY_NEXT");//下游工厂

		//增加第十四块 画面区分 
		bcls_ret->Tables.Add("SHOW_FLAG");
		bcls_ret->Tables["SHOW_FLAG"].Columns.Add(DT_STRING, "SHOW_FLAG");
		bcls_ret->Tables["SHOW_FLAG"].Columns.Add(DT_STRING, "SAVE_TIME");
		bcls_ret->Tables["SHOW_FLAG"].Rows.Add();
		bcls_ret->Tables["SHOW_FLAG"].Rows[0]["SHOW_FLAG"] = show_flag;

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
		Log::Trace("", __FUNCTION__, "主计划数据查询开始");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT a.*, b.* FROM TPSSM15 a, TPSSM17 b "
				"  WHERE a.FACTORY_DIV = @v_factory_div "
				"	 and a.factory_div = b.factory_div and a.pono = b.pono "
				"  ORDER BY a.CAST_NO ASC, a.CAST_DIV_NO ASC "
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
			k = cmd_tpssm11_inq.Fetch(tpssm17, k);
			tpssm11.TrimOrBlank();
			tpssm17.TrimOrBlank();

			//精炼工序的第一个charge_no 必定不为0
			//Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm17["PONO"].ToString());
			//去向
			v_slab_dest = " ";
			for (int i_step = 0; i_step < tb_tep0002.Rows.get_Count(); i_step++)
			{
				if (tb_tep0002.Rows[i_step]["CODE"].ToString() == tpssm17["SLAB_DEST"].ToString())
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
					" SELECT * FROM TPSSM16 "
					"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"    AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO "
					"    AND SUB_CHARGE_NO = 0 "  //0-主工序. 对甘特图只读取主工序的.
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

			TPSSMD3_CONDI2.Tables[0].Rows[0]["ST_NO"] = tpssm11["ST_NO"].ToString();
			TPSSMD3_CONDI2.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
			TPSSMD3_CONDI2.Tables[0].Rows[0]["DEV_CODE"] = "A";
			TPSSMD3_CONDI2.Tables[0].Rows[0]["SMELT_MODE"] = "0";
			TPSSMD3_CONDI2.Tables[0].Rows[0]["SMELT_MODE2"] = tpssm11["SMELT_MODE2"].ToString();
			TPSSMD3_RESULT2.Tables[0].Clear();
			f_pssm_query(TPSSMD3_CONDI2, TPSSMD3_SOURCE2, TPSSMD3_RESULT2, conn);
			if (TPSSMD3_RESULT2.Tables[0].Rows.get_Count() == 1)
			{
				row_plan["COST_HJ"] = TPSSMD3_RESULT2.Tables[0].Rows[0]["COST_HJ"].ToDecimal();
			}
			else
			{
				row_plan["COST_HJ"] = 0;
			}

			//读取重引锭标记
			row_plan["RESTRAND_FLG"] = (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T" ? 1 : 0);
			row_plan["SG_SIGN"] = tpssm17["SG_SIGN"].ToString();
			//板坯规格信息
			row_plan["GUIGE2"] = tpssm17["SLAB_THICK"].ToDecimal().Round(0).ToString() + "*" + tpssm17["SLAB_WIDTH"].ToDecimal().Round(0).ToString() + "*" + tpssm17["SLAB_LEN"].ToDecimal().Round(0).ToString();

			row_plan["SLAB_DEST"] = v_slab_dest;

			row_plan["CI_DIV"] = tpssm11["BACKLOG_EA"].ToString().Substring((tpssm11["BACKLOG_EA"].ToString().GetLength() - 1), 1);

			row_plan["PLAN_FLAG"] = 0;
		}
		cmd_tpssm11_inq.Close();

		//----------------------------------------------------------
		//第三块，设备代码
		Log::Trace("", __FUNCTION__, "设备代码查询");
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
				"    AND AREA_ID    >= 2 "  //脱硫工序单独工序，吹氩工序做吹氩指示
				//"    AND STATION_ID<> 'A'  "
				" ORDER BY AREA_ID, decode(DEV_TECH_CODE,'Z',1,'B',2,'E',3,'A',4,'S',5,'V',6,'F',7,'R',8,'C',9), decode(DEV_CODE,'B0','B3',DEV_CODE) "
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
			row_dev["UPPER_LIMIT_VALUE"] = tpssmd1["UPPER_LIMIT_VALUE"];
			row_dev["LOWER_LIMIT_VALUE"] = tpssmd1["LOWER_LIMIT_VALUE"];
		}
		cmd_tpssmd1_inq.Close();

		//----------------------------------------------------------
		//第四块，制造命令相关
		Log::Trace("", __FUNCTION__, "制造命令查询");
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
					" SELECT A.*, NVL(B.CODE_DESC_1_CONTENT, ' ') FROM TPSSM17 A "
					"	LEFT OUTER JOIN (SELECT * FROM TEP0002 WHERE CODE_CLASS = 'PM16') B "
					"	ON A.SLAB_DEST = B.CODE "
					"  WHERE A.FACTORY_DIV = @v_factory_div "
					"    AND A.PONO_STATUS IN (15, 16) "
					"  ORDER BY A.CC_MACH_NO, A.CC_SEQ ASC "  //因没有浇铸顺画面，暂时用此方式排序 
					);
				break;
			}
			cmd_tpssm17_inq.SetCommandText(sqlstr);
			cmd_tpssm17_inq.Parameters.Set("v_factory_div", v_factory_div);

			//Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm17_inq.ExecuteReader();
			while (cmd_tpssm17_inq.Read())
			{
				k = cmd_tpssm17_inq.Fetch(tpssm17, 1);
				v_slab_dest = cmd_tpssm17_inq.GetString(k);
				//精炼工序的第一个charge_no 必定不为0
				Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm17["PONO"].ToString());
				//Log::Info("", __FUNCTION__, "v_slab_dest =[{0}]", v_slab_dest);

				tpssm17.TrimOrBlank();
				tpssm15["PONO"] = tpssm17["PONO"].ToString();
				bool has15 = tpssm15.Query("PONO");
				if (has15)
				{
					continue;
				}

				CDataRow& row_pono = bcls_ret->Tables["PONO"].Rows.Add();   //新增空行

				//命令数据赋值
				row_pono["PONO"] = tpssm17["PONO"];
				row_pono["CC_MACH_NO"] = tpssm17["CC_MACH_NO"];
				row_pono["ST_NO"] = tpssm17["ST_NO"];
				row_pono["REFINE_ROUTE_CODE"] = tpssm17["REFINE_DIV"];
				row_pono["SMELT_MODE"] = tpssm17["SMELT_MODE"];
				row_pono["SMELT_DIV"] = tpssm17["SMELT_DIV"];
				row_pono["RESTRAND_FLG"] = (tpssm17["RESTRAND_FLG"].ToString().Trim() == "T" ? 1 : 0);
				row_pono["CC_SEQ"] = tpssm17["CC_SEQ"];
				row_pono["PONO_PLAN_DATE"] = tpssm17["PLAN_DATE"];
				row_pono["SG_SIGN"] = tpssm17["SG_SIGN"];
				row_pono["GUIGE"] = tpssm17["SLAB_THICK"].ToDecimal().ToString() + "*" + tpssm17["SLAB_WIDTH"].ToDecimal().ToString() + "*" + tpssm17["SLAB_LEN"].ToDecimal().ToString();

				cast_lot_no_and_div = tpssm17["CAST_LOT_NO"].ToString() + "-" + tpssm17["CAST_LOT_DIV_NO"].ToString();
				row_pono["CAST_LOT_NO"] = cast_lot_no_and_div;
				cast_lot_no_and_div = tpssm17["CAST_LOT_NO2"].ToString() + "-" + tpssm17["CAST_LOT_DIV_NO2"].ToString();
				row_pono["CAST_LOT_NO2"] = cast_lot_no_and_div;
				row_pono["POUR_TIME"] = tpssm17["POUR_TIME"];
				row_pono["SLAB_DEST"] = v_slab_dest;
				row_pono["CI_DIV"] = tpssm17["BACKLOG_EA"].ToString().Substring((tpssm17["BACKLOG_EA"].ToString().GetLength() - 1), 1);
				row_pono["C_DIV"] = tpssm17["C_DIV"];
				row_pono["CC_REQ_TIME"] = tpssm17["CC_REQ_TIME"];
				row_pono["CC_REQ_TIMEL4"] = tpssm17["CC_REQ_TIMEL4"];
				row_pono["ROUTELIST"] = tpssm17["ROUTELIST"];
				row_pono["ROUTEBAGKEY"] = tpssm17["ROUTEBAGKEY"];
				row_pono["SLAB_THICK"] = tpssm17["SLAB_THICK"].ToDecimal();
				row_pono["SLAB_WIDTH"] = tpssm17["SLAB_WIDTH"].ToDecimal();
				row_pono["PLAN_TAP_WT"] = tpssm17["PLAN_TAP_WT"].ToDecimal();
				row_pono["CAST_LOT_NO_00"] = tpssm17["CAST_LOT_NO"].ToString();
				row_pono["CAST_LOT_DIV_NO_00"] = tpssm17["CAST_LOT_DIV_NO"].ToString();

				prod_density = 7.85;
				tpssmd9["CC_MACH_NO"] = tpssm17["CC_MACH_NO"];
				tpssmd9["BILLET_TYPE"] = tpssm17["BILLET_TYPE"];
				tpssmd9["CAST_THICK"] = tpssm17["SLAB_THICK"];
				tpssmd9["FACTORY_DIV"] = tpssm17["FACTORY_DIV"];
				tpssmd9.Query("FACTORY_DIV,BILLET_TYPE,CC_MACH_NO,CAST_THICK");

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
																							AND CC_MACH_NO		= @tpssm17.CC_MACH_NO \
																															AND BILLET_TYPE		= @tpssm17.BILLET_TYPE \
																																							AND ST_NO			= @tpssm17.ST_NO \
																																															AND CAST_WIDTH_MIN	<= @tpssm17.SLAB_WIDTH \
																																																							AND CAST_WIDTH_MAX	>= @tpssm17.SLAB_WIDTH \
																																																															AND CAST_THICK_MIN	<= @tpssm17.SLAB_THICK \
																																																																							AND CAST_THICK_MAX	>= @tpssm17.SLAB_THICK ";
					break;
				}

				cmd_tpssmda_inq.SetCommandText(sqlstr);
				cmd_tpssmda_inq.Parameters.Set("v_factory_div", v_factory_div);
				cmd_tpssmda_inq.Parameters.Set("tpssm17.CC_MACH_NO", tpssm17["CC_MACH_NO"].ToString());
				cmd_tpssmda_inq.Parameters.Set("tpssm17.BILLET_TYPE", tpssm17["BILLET_TYPE"].ToString());
				cmd_tpssmda_inq.Parameters.Set("tpssm17.ST_NO", tpssm17["ST_NO"].ToString());
				cmd_tpssmda_inq.Parameters.Set("tpssm17.SLAB_WIDTH", tpssm17["SLAB_WIDTH"].ToDecimal());
				cmd_tpssmda_inq.Parameters.Set("tpssm17.SLAB_THICK", tpssm17["SLAB_THICK"].ToDecimal());
				cmd_tpssmda_inq.ExecuteReader();
				if (cmd_tpssmda_inq.Read())
				{
					tpssmda["CAST_SPEED"] = cmd_tpssmda_inq.GetDecimal(1);
				}
				cmd_tpssmda_inq.Close();
				//Log::Info("",__FUNCTION__,"tpssmda["CAST_SPEED"] = {0},tpssm17["PLAN_TAP_WT"] = {1},tpssm17["SLAB_THICK"] ={2},tpssm17["SLAB_WIDTH"] = {3}"
				//	,tpssmda["CAST_SPEED"].ToDecimal(),tpssm17["PLAN_TAP_WT"].ToDecimal(),tpssm17["SLAB_THICK"].ToDecimal(),tpssm17["SLAB_WIDTH"].ToDecimal());

				if (tpssmda["CAST_SPEED"].ToDecimal() == 0)
				{
					//tqmts0x.Reset();
					//tqmts0x["ST_NO"] = tpssm17["ST_NO"].ToString().Trim();
					if (tpssm17["ROUTEBAGKEY"].ToString()[0] == 'C' || tpssm17["ROUTEBAGKEY"].ToString()[0] == 'S')
					{
						//tqmts0x.Query("ST_NO");
						CString c_div = " ";
						if (tpssm17["ROUTEBAGKEY"].ToString()[0] == 'C')
						{
							c_div = "DEFAULTC";
						}
						else if (tpssm17["ROUTEBAGKEY"].ToString()[0] == 'S')
						{
							c_div = "DEFAULTS";
						}
						//2碳钢 1不锈钢
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:	        // Oracle 数据库
						default:

							//CAST_SPEED
							sqlstr = " SELECT CAST_SPEED FROM TPSSMDA "
								" WHERE FACTORY_DIV = @v_factory_div "
								" AND CC_MACH_NO		= @tpssm17.CC_MACH_NO "
								" AND BILLET_TYPE		= @tpssm17.BILLET_TYPE "
								" AND ST_NO			= @tpssm17.ST_NO  "
								" AND CAST_WIDTH_MIN <= @tpssm17.SLAB_WIDTH "
								" AND CAST_WIDTH_MAX >= @tpssm17.SLAB_WIDTH "
								" AND CAST_THICK_MIN <= @tpssm17.SLAB_THICK "
								" AND CAST_THICK_MAX >= @tpssm17.SLAB_THICK ";
							break;
						}


						cmd_tpssmda_inq.SetCommandText(sqlstr);
						cmd_tpssmda_inq.Parameters.Set("v_factory_div", v_factory_div);
						cmd_tpssmda_inq.Parameters.Set("tpssm17.CC_MACH_NO", tpssm17["CC_MACH_NO"].ToString());
						cmd_tpssmda_inq.Parameters.Set("tpssm17.BILLET_TYPE", tpssm17["BILLET_TYPE"].ToString());
						cmd_tpssmda_inq.Parameters.Set("tpssm17.ST_NO", c_div);
						cmd_tpssmda_inq.Parameters.Set("tpssm17.SLAB_WIDTH", tpssm17["SLAB_WIDTH"].ToDecimal());
						cmd_tpssmda_inq.Parameters.Set("tpssm17.SLAB_THICK", tpssm17["SLAB_THICK"].ToDecimal());
						cmd_tpssmda_inq.ExecuteReader();
						if (cmd_tpssmda_inq.Read())
						{
							tpssmda["CAST_SPEED"] = cmd_tpssmda_inq.GetDecimal(1);
						}
						cmd_tpssmda_inq.Close();
					}
					else
					{
						tpssmda["CAST_SPEED"] = 1400;
					}
				}

				if (tpssmda["CAST_SPEED"].ToDecimal() == 0 || tpssm17["PLAN_TAP_WT"].ToDecimal() == 0 || tpssm17["SLAB_THICK"].ToDecimal() == 0 || tpssm17["SLAB_WIDTH"].ToDecimal() == 0 || prod_density == 0 || tpssmd9["STRAND_NUM"].ToDecimal() == 0)
				{
					pour_time1 = 40;
				}
				else
				{
					pour_time1 = (tpssm17["PLAN_TAP_WT"].ToDecimal() * 1000 * 1000 * 1000) / (prod_density * tpssm17["SLAB_THICK"].ToDecimal() * tpssm17["SLAB_WIDTH"].ToDecimal() * tpssmda["CAST_SPEED"].ToDecimal() *tpssmd9["STRAND_NUM"].ToDecimal());	//
					Log::Info("", __FUNCTION__, "计算得出浇铸时间 pour_time=[{0}]", pour_time1);
				}
				pour_time = pour_time1.ToInt32();
				row_pono["POUR_TIME"] = pour_time;

			}
			cmd_tpssm17_inq.Close();

		}
		//-------------------------------------------------------
		//第五块，设备维修
		Log::Trace("", __FUNCTION__, "设备特殊状态");
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
			row_stop["DEV_STATUS_REMARK"] = tpssm18["DEV_STATUS_REMARK"].ToString();
			row_stop["STOP_FLAG"] = tpssm18["STOP_FLAG"];
			row_stop["STATUS_AREA"] = tpssm18["AREA_ID"];
		}
		cmd_tpssm21_inq.Close();


		//-------------------------------------------------------
		//第六块 各工序设备处理时间
		Log::Trace("", __FUNCTION__, "设备处理时间");

		EIClass TPSSMD3_SOURCE;
		EIClass TPSSMD3_CONDI;
		TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "ST_NO");
		TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
		TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "SMELT_MODE");
		TPSSMD3_CONDI.Tables[0].Rows.Add();
		EIClass TPSSMD3_RESULT;

		sqlstr = "SELECT * FROM TPSSMD3 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD3_SOURCE.Tables[0]);
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT DISTINCT(ST_NO) FROM TPSSM17 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND PONO_STATUS < 83 "
				);
			break;
		}
		cmd_tpssm17_inq.SetCommandText(sqlstr);
		cmd_tpssm17_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm17_inq.ExecuteReader();
		while (cmd_tpssm17_inq.Read())
		{
			tpssm17["ST_NO"] = cmd_tpssm17_inq.GetString(1);
			//Log::Trace("", __FUNCTION__, "查询出钢记号ST_NO=[{0}]", tpssm17["ST_NO"].ToString());

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
					" SELECT DISTINCT d3.ST_NO, d1.DEV_TECH_CODE, d3.STD_PROC_TIME, d1.AREA_ID, d3.SMELT_MODE, d3.SMELT_MODE2, d3.STD_PREP_TIME "
					"   FROM TPSSMD3 d3,  TPSSMD1 d1 "
					"  WHERE d3.ST_NO         = @st_no "
					"    AND d3.DEV_CODE = d1.DEV_TECH_CODE  "
					"   AND d3.factory_div = d1.factory_div "
					"   AND d1.factory_div = @tpssm17.factory_div"
					"   AND d3.SMELT_MODE2 = ' ' "
					);
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tpssm17["ST_NO"].ToString());
			cmd_inq.Parameters.Set("tpssm17.factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			//Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
			while (cmd_inq.Read())
			{

				tpssmd3["ST_NO"] = cmd_inq.GetString(1);
				tpssmd1["DEV_TECH_CODE"] = cmd_inq.GetString(2);
				tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(3);
				tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(4);
				tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(5);
				tpssmd3["SMELT_MODE2"] = cmd_inq.GetString(6);
				tpssmd3["STD_PREP_TIME"] = cmd_inq.GetDecimal(7);

				sqlstr = CString(
					" SELECT DISTINCT DEV_CODE FROM TPSSMD1 WHERE DEV_TECH_CODE = @DEV_TECH_CODE AND AREA_ID = @AREA_ID AND FACTORY_DIV = @FACTORY_DIV "
					);
				cmd_inq2.SetCommandText(sqlstr);
				cmd_inq2.Parameters.Set("DEV_TECH_CODE", tpssmd1["DEV_TECH_CODE"].ToString());
				cmd_inq2.Parameters.Set("AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				cmd_inq2.Parameters.Set("FACTORY_DIV", v_factory_div);
				cmd_inq2.ExecuteReader();
				//Log::Trace("", __FUNCTION__, "v_factory_div =[{0}]", v_factory_div);
				//Log::Trace("", __FUNCTION__, "while里查询到DEV_TECH_CODE=[{0}]", tpssmd1["DEV_TECH_CODE"].ToString());
				//Log::Trace("", __FUNCTION__, "while里查询到tpssmd1[AREA_ID] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());

				while (cmd_inq2.Read())
				{
					tpssmd1["DEV_CODE"] = cmd_inq2.GetString(1);
					//Log::Trace("", __FUNCTION__, "DEV_CODE =[{0}]", tpssmd1["DEV_CODE"].ToString());
					if ((tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0") && tpssmd1["AREA_ID"].ToString().Trim() != "2")
					{
						CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
						row_dt["ST_NO"] = tpssmd3["ST_NO"];
						row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
						row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
						row_dt["SMELT_FLAG"] = "1";//常规
						row_dt["SMELT_FLAG2"] = tpssmd3["SMELT_MODE2"];
						row_dt["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];
					}

					else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2" && tpssmd1["AREA_ID"].ToString().Trim() == "2")
					{
						CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
						row_dt["ST_NO"] = tpssmd3["ST_NO"];
						row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
						row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
						row_dt["SMELT_FLAG"] = "2";//双联脱磷处理时间
						row_dt["SMELT_FLAG2"] = " ";
						row_dt["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];
					}

					else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2" && tpssmd1["AREA_ID"].ToString().Trim() == "3")
					{
						CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
						row_dt["ST_NO"] = tpssmd3["ST_NO"];
						row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
						row_dt["SMELT_FLAG"] = "3";//双联脱碳处理时间
						tpssmd3_s.Reset();
						tpssmd3_s["FACTORY_DIV"] = v_factory_div;
						tpssmd3_s["DEV_CODE"] = tpssmd1["DEV_CODE"];
						tpssmd3_s["ST_NO"] = tpssmd3["ST_NO"];
						tpssmd3_s["SMELT_MODE"] = "0";
						tpssmd3_s.Query("FACTORY_DIV,DEV_CODE,ST_NO,SMELT_MODE");
						row_dt["PROC_TIME"] = tpssmd3_s["STD_PROC_TIME"];
						row_dt["SMELT_FLAG2"] = " ";
						row_dt["PREP_TIME"] = tpssmd3_s["STD_PREP_TIME"];
					}

					else if (tpssmd1["AREA_ID"].ToString().Trim() == "2" && tpssmd1["DEV_CODE"][0] == 'Z')
					{
						CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
						row_dt["ST_NO"] = tpssmd3["ST_NO"];
						row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
						row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
						row_dt["SMELT_FLAG"] = "1";//常规
						row_dt["SMELT_FLAG2"] = " ";
						row_dt["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];
					}
					else
					{

					}
				}
				cmd_inq2.Close();
			}
			cmd_inq.Close();
		}
		cmd_tpssm17_inq.Close();

		//Log::Trace("", __FUNCTION__, "查询Rows=[{0}]", bcls_ret->Tables["PROC_TIME"].Rows.get_Count());
		//Log::Trace("", __FUNCTION__, "查询DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
		//Log::Trace("", __FUNCTION__, "查询tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());

		//没有查询到记录时，读取全部?????????
		//if (bcls_ret->Tables["PROC_TIME"].Rows.get_Count() < 1)
		//{
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

			sqlstr = " SELECT T.ST_NO,A.DEV_CODE,T.STD_PROC_TIME ,A.AREA_ID, T.SMELT_MODE, T.SMELT_MODE2, T.STD_PREP_TIME"
				" FROM TPSSMD3 T ,TPSSMD1 A "
				" WHERE T.DEV_CODE = A.DEV_CODE "
				"   AND T.factory_div = A.factory_div "
				"   AND T.factory_div = @tpssm17.factory_div"
				"   AND T.st_no in ('DEFAULTC','DEFAULTS')"
				"   AND T.SMELT_MODE2 = ' ' "
				;


			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm17.factory_div", v_factory_div);

		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssmd3["ST_NO"] = cmd_inq.GetString(1);
			tpssmd1["DEV_CODE"] = cmd_inq.GetString(2);
			tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(3);
			tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(4);
			tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(5);
			tpssmd3["SMELT_MODE2"] = cmd_inq.GetString(6);
			tpssmd3["STD_PREP_TIME"] = cmd_inq.GetDecimal(7);

			/*Log::Trace("", __FUNCTION__, "用st_no查不到while里查询到tpssmd3["STD_PROC_TIME"] =[{0}]", tpssmd3["STD_PROC_TIME"].ToDecimal());
			Log::Trace("", __FUNCTION__, "while里查询到DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
			Log::Trace("", __FUNCTION__, "while里查询到tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());*/

			if ((tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0") && tpssmd1["AREA_ID"].ToString().Trim() != "2")
			{
				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] = tpssmd3["ST_NO"];
				row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				row_dt["SMELT_FLAG"] = "1";//常规
				row_dt["SMELT_FLAG2"] = tpssmd3["SMELT_MODE2"];
				row_dt["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];
			}

			else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2" && tpssmd1["AREA_ID"].ToString().Trim() == "2")
			{
				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] = tpssmd3["ST_NO"];
				row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				row_dt["SMELT_FLAG"] = "2";//双联脱磷处理时间
				row_dt["SMELT_FLAG2"] = " ";
				row_dt["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];
			}

			else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2" && tpssmd1["AREA_ID"].ToString().Trim() == "3")
			{
				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] = tpssmd3["ST_NO"];
				row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				row_dt["SMELT_FLAG"] = "3";//双联脱碳处理时间
				tpssmd3_s.Reset();
				/*tpssmd3_s["FACTORY_DIV"] = v_factory_div;
				tpssmd3_s["DEV_CODE"] = tpssmd1["DEV_CODE"];
				tpssmd3_s["ST_NO"] = tpssmd3["ST_NO"];
				tpssmd3_s["SMELT_MODE"] = "0";
				tpssmd3_s.Query("FACTORY_DIV,DEV_CODE,ST_NO,SMELT_MODE");*/

				TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = tpssmd3["ST_NO"];
				TPSSMD3_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
				TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssmd1["DEV_CODE"];
				TPSSMD3_CONDI.Tables[0].Rows[0]["SMELT_MODE"] = "0";
				TPSSMD3_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);

				tpssmd3_s.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);

				row_dt["PROC_TIME"] = tpssmd3_s["STD_PROC_TIME"];
				row_dt["SMELT_FLAG2"] = " ";
				row_dt["PREP_TIME"] = tpssmd3_s["STD_PREP_TIME"];
			}

			else if (tpssmd1["AREA_ID"].ToString().Trim() == "2" && tpssmd1["DEV_CODE"][0] == 'Z')
			{
				CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				row_dt["ST_NO"] = tpssmd3["ST_NO"];
				row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				row_dt["SMELT_FLAG"] = "1";//常规
				row_dt["SMELT_FLAG2"] = " ";
				row_dt["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];
			}
			else
			{

			}

			//CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
			//row_dt["ST_NO"] =  tpssmd3["ST_NO"];
			//row_dt["DEV_CODE"] =  tpssmd1["DEV_CODE"];
			//row_dt["PROC_TIME"] =  tpssmd3["STD_PROC_TIME"];
			//if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0")
			//	row_dt["SMELT_FLAG"] = "1";//常规
			//else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "2")
			//	row_dt["SMELT_FLAG"] = "2";//双联脱磷处理时间
			//else if (tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "2"&& tpssmd1["AREA_ID"].ToDecimal().ToString().Trim() == "3")
			//	row_dt["SMELT_FLAG"] = "3";//双联脱碳处理时间
		}
		cmd_inq.Close();

		//PrintDataTable(bcls_ret->Tables["PROC_TIME"]);
		//}

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
		//Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
		//cmd_inq.ExecuteReader();
		//while (cmd_inq.Read())
		//{			
		//	tpssmd1["DEV_CODE"] = cmd_inq.GetString(1);
		//	tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(2);
		//	tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(3);
		//	tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(4);

		//	Log::Trace("", __FUNCTION__, "默认值--查询到tpssmd3["STD_PROC_TIME"] =[{0}]", tpssmd3["STD_PROC_TIME"].ToDecimal());
		//	Log::Trace("", __FUNCTION__, "while里查询到DEV_CODE=[{0}]", tpssmd1["DEV_CODE"].ToString());
		//	Log::Trace("", __FUNCTION__, "while里查询到tpssmd1["AREA_ID"] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());

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
	
			//-------------------------------------------------------
			//第十四块 保存时间 
			Log::Trace("", __FUNCTION__, "13.保存时间");
			sqlstr = " SELECT MAX(TIME_1) FROM TPSSM11 ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				bcls_ret->Tables["SHOW_FLAG"].Rows[0]["SAVE_TIME"] = cmd_inq.GetString(1);
			}
			else bcls_ret->Tables["SHOW_FLAG"].Rows[0]["SAVE_TIME"] = " ";
			cmd_inq.Close();
		//-------------------------------------------------------
		//第七块  传搁时间
		Log::Trace("", __FUNCTION__, "设备传搁时间");

		EIClass TPSSMD1_SOURCE;
		EIClass TPSSMD1_CONDI;
		TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_ID");
		TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_NO");
		TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		TPSSMD1_CONDI.Tables[0].Rows.Add();

		EIClass TPSSMD1_RESULT;

		sqlstr = "SELECT * FROM TPSSMD1 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DEV_MOVE_START,MOVE_TIME,DEV_MOVE_END,TRAN_TYPE FROM TPSSMD6 \
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
			tpssmd6["TRAN_TYPE"] = cmd_tpssmd6_inq.GetString(4);

			tpssmd1_s.Reset();
			//tpssmd1_s["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			//tpssmd1_s["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			//tpssmd1_s["FACTORY_DIV"] = v_factory_div;

			//tpssmd1_s.Query("STATION_ID,STATION_NO,FACTORY_DIV");

			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
			TPSSMD1_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
			tpssmd1_s.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

			tpssmd1_e.Reset();
			//tpssmd1_e["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			//tpssmd1_e["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);
			//tpssmd1_e["FACTORY_DIV"] = v_factory_div;

			//tpssmd1_e.Query("STATION_ID,STATION_NO,FACTORY_DIV");

			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
			TPSSMD1_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
			tpssmd1_e.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

			CDataRow & row_mt = bcls_ret->Tables["MOVE_TIME"].Rows.Add();
			/*row_mt["START_DEV"] = tpssmd6.DEV_MOVE_START;
			row_mt["END_DEV"] = tpssmd6.DEV_MOVE_END;*/
			row_mt["START_DEV"] = tpssmd1_s["DEV_CODE"];
			row_mt["END_DEV"] = tpssmd1_e["DEV_CODE"];
			row_mt["MOVE_TIME"] = tpssmd6["MOVE_TIME"];
			row_mt["TRAN_TYPE"] = tpssmd6["TRAN_TYPE"];

		}
		cmd_tpssmd6_inq.Close();

		//-------------------------------------------------------
		//第八块  连铸工艺时间
		Log::Trace("", __FUNCTION__, "连铸工艺时间");

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
		Log::Trace("", __FUNCTION__, "设备定检修类型配置");


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
			//tpssm19.Print();
		}
		cmd_inq.Close();

		//-------------------------------------------------------
		//第十块 钢种-路径包配置 
		Log::Trace("", __FUNCTION__, "钢种-路径包配置");


		sqlstr = " SELECT * FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm17) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdh);

			CDataRow & row_dh = bcls_ret->Tables["ST_NO_ROUTEBAG"].Rows.Add();
			row_dh.Merge(tpssmdh);
			//tpssm19.Print();
		}
		cmd_inq.Close();


		//-------------------------------------------------------
		//第十一块 路径包-路径配置 
		Log::Trace("", __FUNCTION__, "路径包-路径配置");


		sqlstr = " SELECT * FROM TPSSMDJ ";
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdj);

			CDataRow & row_dj = bcls_ret->Tables["ROUTEBAG_ROUTE"].Rows.Add();
			row_dj.Merge(tpssmdj);
			//tpssm19.Print();
		}
		cmd_inq.Close();

		//-------------------------------------------------------
		//第十二块 路径-设备列表配置 
		Log::Trace("", __FUNCTION__, "路径-设备列表配置");


		sqlstr = " SELECT * FROM TPSSMD7 WHERE FACTORY_DIV = @v_factory_div ORDER BY ROUTELIST,CHARGE_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd7);

			CDataRow & row_d7 = bcls_ret->Tables["ROUTE_DEV"].Rows.Add();
			row_d7.Merge(tpssmd7);
			//tpssm19.Print();
		}
		cmd_inq.Close();


		//-------------------------------------------------------
		//第十三块 预定板坯号 
		Log::Trace("", __FUNCTION__, "预定板坯号数据");

		/*sqlstr = " SELECT * FROM TPSSM03 WHERE PONO IN ( SELECT PONO FROM TPSSM11 ) ORDER BY PONO,LSLAB_NO,SLAB_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(tb_tpssm03);
		cmd_inq.Close();

		if (tb_tpssm03.Rows.get_Count() > 0)
		{
			for (int i = 0; i < tb_tpssm03.Rows.get_Count(); i++)
			{
				tpssm03.MergeFrom(tb_tpssm03.Rows[i]);
				if (pono != tpssm03["PONO"].ToString())
				{
					pono = tpssm03["PONO"].ToString();
					tpssm11["PONO"] = pono;
					tpssm11.Query("PONO");
				}

				if (lslab_no != tpssm03["LSLAB_NO"].ToString())
				{
					CDataRow & row_03 = bcls_ret->Tables["PONO_SLAB"].Rows.Add();

					if (tpssm11["CC_MACH_NO"].ToString() == "0")
					{
						strand_no = "Z";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "1")
					{
						strand_no = "A";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "2")
					{
						strand_no = "B";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "1")
					{
						strand_no = "C";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "2")
					{
						strand_no = "D";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "1")
					{
						strand_no = "E";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "2")
					{
						strand_no = "F";
					}

					row_03["PONO"] = tpssm03["PONO"].ToString();
					row_03["SLAB_NO"] = tpssm03["LSLAB_NO"].ToString();//预定材料号
					row_03["STRAND_NO"] = strand_no;//流号
					row_03["SLAB_THICK"] = tpssm03["SLAB_THICK"].ToDecimal();//材料厚度
					row_03["SLAB_WIDTH"] = tpssm03["SLAB_WIDTH"].ToDecimal();//材料宽度

					if (tpssm03["LSLAB_NO_LENGTH"].ToDecimal() != 0)
					{
						row_03["SLAB_LEN"] = tpssm03["LSLAB_NO_LENGTH"].ToDecimal();//材料目标长度
						row_03["SLAB_MAX_LEN"] = tpssm03["LSLAB_NO_LENGTH_MAX"].ToDecimal();//材料最大长度
						row_03["SLAB_MIN_LEN"] = tpssm03["LSLAB_NO_LENGTH_MIN"].ToDecimal();//材料最小长度

						sqlstr = " SELECT sum(SLAB_SEQ_2) FROM TPSSM03 WHERE LSLAB_NO = @LSLAB_NO ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("LSLAB_NO", tpssm03["LSLAB_NO"].ToString());
						row_03["SLAB_SEQ_2"] = cmd_inq.ExecuteScalar();
						cmd_inq.Close();
						//row_03["SLAB_SEQ_2"] = tpssm03["SLAB_SEQ_2"].ToDecimal();
					}
					else
					{
						row_03["SLAB_LEN"] = tpssm03["SLAB_LEN"].ToDecimal();//材料目标长度
						row_03["SLAB_MAX_LEN"] = tpssm03["SLAB_MAX_LEN"].ToDecimal();//材料最大长度
						row_03["SLAB_MIN_LEN"] = tpssm03["SLAB_MIN_LEN"].ToDecimal();//材料最小长度
						row_03["SLAB_SEQ_2"] = tpssm03["SLAB_SEQ_2"].ToDecimal();
					}
					row_03["ORDER_NO"] = tpssm03["ORDER_NO"].ToString();//合同号
					//row_03["SLAB_DEST"];//材料去向
					row_03["SG_SIGN"] = tpssm03["SG_SIGN"].ToString();//牌号
					//row_03["FACTORY_NEXT"];//下游工厂
					lslab_no = tb_tpssm03.Rows[i]["LSLAB_NO"].ToString();
				}
			}
		}*/

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
	cmd_tpssm17_inq.Close();
	cmd_tpssm11_inq.Close();
	cmd_tpssmd1_inq.Close();
	cmd_tpssm03_inq.Close();
	cmd_tpssm02_inq.Close();
	cmd_tpssmda_inq.Close();
	cmd_tpssm21_inq.Close();
	cmd_inq.Close();

	return doFlag;

}