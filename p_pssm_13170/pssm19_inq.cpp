/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   WCY
Date:     2024-3-5
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
BM2F_ENTERACE(pssm19_inq)

int f_pssm19_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int blkseq = 0;
	int ret = 0;
	int fetchRowCount = 0;

	CString v_factory_div = "";	//炼钢单元号
	CString base_time = "";
	CString history_time = "";
	CString history_time_s = "";
	CString history_time_e = "";
	CString cast_lot_no_and_div = "";
	CDecimal  pour_time1 = 0;               /* 连铸机浇注时间*/
	CDecimal  pour_time = 0;               /* 连铸机浇注时间*/
	CString dev_move_start = "";
	CString dev_move_end = "";
	CString dev_code = "";
	CString query_type = "";
	CString v_slab_dest = "";
	CDecimal prod_density = 0; //板坯密度
	CDecimal thick_range = 0;
	int k = 0;
	int mode = 1;
	CString lslab_no = "";
	CString strand_no = "";
	CDecimal seq = 0;
	CString pono = "";
	CString show_flag = "";

	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssm19("TPSSM19");
	CModel tpssmd9("TPSSMD9");
	//CModel tep0002("TEP0002");


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
	CDataTable tb_tep0002("TEP0002");//保存去向小代码

	try
	{
		base_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		sqlstr = " SELECT CODE_DESC_1_CONTENT from  tep0002 where CODE_CLASS='PSAT2N' AND CODE = 'A' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			thick_range = cmd_inq.GetDecimal(1);
		}
		cmd_inq.Close();

		//----------------------------------------------------------
		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		query_type = bcls_rec->Tables[0].Rows[0]["QUERY_TYPE"].ToString().Trim();
		show_flag = bcls_rec->Tables[0].Rows[0]["SHOW_FLAG"].ToString().Trim();

		if (bcls_rec->Tables[0].Columns.Contains("HISTROY_TIME"))
		{
			history_time = bcls_rec->Tables[0].Rows[0]["HISTROY_TIME"].ToString().Trim();
			
			if (history_time.Trim().SubstringNE(0, 1) == "0" || history_time.Trim() == "")
			{

			}
			else
			{
				history_time_s = (CDateTime::Parse(history_time).AddDays(-1)).ToString("yyyyMMddHHmmss");
				history_time_e = (CDateTime::Parse(history_time).AddDays(1)).ToString("yyyyMMddHHmmss");
			}
		}
		Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}],query_type=[{1}],history_time = [{2}]", v_factory_div, query_type, history_time);

		//查询去向小代码
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
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SM_PLAN_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PROC_SUB");
		//bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SLAB_DEST");

		//第三块，设备代码 3
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("DEV");  //设备信息
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");  //设备代码
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_NAME"); //设备名称
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TYPE");  //设备类型:细分同一类型设备的工艺区分
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CLASS_ID");  //区域标识
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "NUMB");      //甘特图显示顺序
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE");      //上限
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "LOWER_LIMIT_VALUE");      //下限
		//第四块，制造命令相关 2,3
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
		//bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO");
		//bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO2");

		//第五块，设备维修 1
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("STOP");  //设备维修
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_STATUS_REMARK");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STOP_FLAG");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "STATUS_AREA");

		//增加第六块 根据钢种提供的各设备处理时间 3
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("PROC_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_FLAG");//用来区分双联和常规的脱碳处理时间、脱磷时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PROC_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_FLAG2");//wcy 用来区分AOD冶炼模式
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME");

		//增加第七块 传搁时间 （考虑在画面载入时传入） 3
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("MOVE_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "START_DEV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "END_DEV");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "MOVE_TIME");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "TRAN_TYPE");
		//增加第八块 连铸的工艺时间 3
		blkseq++;
		bcls_ret->Tables.Add();
		bcls_ret->Tables[blkseq - 1].set_TableName("CC_TIME");  //
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_MACH_NO");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "BILLET_TYPE");
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME_W0");//浇次间准备时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME_W1");//浇次间准备时间
		bcls_ret->Tables[blkseq - 1].Columns.Add(DT_STRING, "PREP_TIME_IN");//浇次内准备时间

		//增加第九块 设备定检修类型 3
		bcls_ret->Tables.Add("DEV_SET");
		bcls_ret->Tables["DEV_SET"].Columns.Add(tpssm19);

		//增加第十块 钢种-路径包 3
		bcls_ret->Tables.Add("ST_NO_ROUTEBAG");
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "ST_NO");//钢种
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "ROUTEBAGKEY");//路径包
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_STRING, "REMARK");//注释
		bcls_ret->Tables["ST_NO_ROUTEBAG"].Columns.Add(DT_DECIMAL, "COST_ST_LINE");//成本

		//增加第十一块 路径包-路径 3
		bcls_ret->Tables.Add("ROUTEBAG_ROUTE");
		bcls_ret->Tables["ROUTEBAG_ROUTE"].Columns.Add(DT_STRING, "ROUTEBAGKEY");//路径包
		bcls_ret->Tables["ROUTEBAG_ROUTE"].Columns.Add(DT_STRING, "ROUTELIST");//路径
		bcls_ret->Tables["ROUTEBAG_ROUTE"].Columns.Add(DT_DECIMAL, "PLANTSECTIONTYPE");//默认路径（1）

		//增加第十二块 路径-设备列表 3
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

		//增加第十三块 预定板坯号 1 4
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
		bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "SLAB_SIZE");//牌号
		//bcls_ret->Tables["PONO_SLAB"].Columns.Add(DT_STRING, "FACTORY_NEXT");//下游工厂

		//增加第十四块 画面区分 1
		bcls_ret->Tables.Add("SHOW_FLAG");
		bcls_ret->Tables["SHOW_FLAG"].Columns.Add(DT_STRING, "SHOW_FLAG");
		bcls_ret->Tables["SHOW_FLAG"].Columns.Add(DT_STRING, "SAVE_TIME");
		bcls_ret->Tables["SHOW_FLAG"].Rows.Add();
		bcls_ret->Tables["SHOW_FLAG"].Rows[0]["SHOW_FLAG"] = show_flag;


		//增加第十五块 双工位交错时间 3
		bcls_ret->Tables.Add("SHAREEQUIPMENTINFO");
		bcls_ret->Tables["SHAREEQUIPMENTINFO"].Columns.Add(DT_STRING, "STATION_NAME");
		bcls_ret->Tables["SHAREEQUIPMENTINFO"].Columns.Add(DT_STRING, "TD_TYPE");
		bcls_ret->Tables["SHAREEQUIPMENTINFO"].Columns.Add(DT_STRING, "STAG_TIME");
		bcls_ret->Tables["SHAREEQUIPMENTINFO"].Columns.Add(DT_STRING, "MOVE_TIME");

		//增加第十六块 极薄标识 1
		bcls_ret->Tables.Add("ORDER_THICK");
		bcls_ret->Tables["ORDER_THICK"].Columns.Add(DT_STRING, "PONO");
		bcls_ret->Tables["ORDER_THICK"].Columns.Add(DT_DECIMAL, "ORDER_THICK");
		bcls_ret->Tables["ORDER_THICK"].Columns.Add(DT_DECIMAL, "THICK_RANGE");


		//if (query_type != "2")
		//{
		//ret = f_pssm_push_time(bcls_rec, bcls_ret, conn);
		//if (ret < 0)
		//{
		//throw CApplicationException(-1, s.msg, log.Location);
		//}
		//----------------------------------------------------------
		//查询当前计划中最早执行的时间
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = CString(
		//		" SELECT MIN(START_TIME_REAL), MIN(START_TIME) "
		//		"   FROM TPSSM12 "
		//		"  WHERE FACTORY_DIV = @v_factory_div "
		//		);
		//	break;
		//}

		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("v_factory_div", v_factory_div);
		//cmd_tpssm12_inq.ExecuteReader();
		//if (cmd_tpssm12_inq.Read())
		//{
		//	tpssm12["START_TIME_REAL"] = cmd_tpssm12_inq.GetString(1);
		//	tpssm12["START_TIME"] = cmd_tpssm12_inq.GetString(2);
		//}
		//else
		//{
		//	tpssm12["START_TIME_REAL"] = " ";
		//	tpssm12["START_TIME"] = " ";
		//}
		//cmd_tpssm12_inq.Close();


		if (query_type == "1" || query_type == "3")
		{
			//----------------------------------------------------------
			//查询数据送到前台
			Log::Trace("", __FUNCTION__, "1.主计划数据查询开始");//wcy 过去24小时内计划查询
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				if (history_time.Trim().SubstringNE(0, 1) == "0" || history_time.Trim() == "")
				{
					Log::Trace("", __FUNCTION__, "now");
					sqlstr = "  with temp AS  (   "
						"  SELECT  a.*, B.CC_SEQ,B.SG_SIGN,B.CAST_LOT_NO,B.CAST_LOT_DIV_NO,B.CAST_LOT_SUM,B.REFINE_DIV,B.PLAN_POUR_WT,B.NEW_TEST_NO,B.PLAN_DATE   "
						"  ,B.CC_PREP_TIME,B.POUR_TIME,B.CC_REQ_TIME_FLAG,B.HOT_SEND_FLAG,B.HOT_CHARGE_FLAG,B.CARRY_DIV,B.SLAB_DEST,B.BILLET_TYPE,B.SLAB_THICK   "
						"  ,B.SLAB_WIDTH,B.SLAB_LEN,B.SHIFT_NO,B.SHIFT_GROUP,B.C_DIV,B.CC_REQ_TIMEL4,B.CAST_LOT_NO2,B.CAST_LOT_DIV_NO2   "
						"  ,B.CAST_LOT_SUM2 FROM TPSSM11 A ,TPSSM10 B,TPSSM12 C WHERE a.FACTORY_DIV = @v_factory_div    "
						"  and a.factory_div = b.factory_div and a.pono = b.pono AND a.SM_PLAN_NO = c.SM_PLAN_NO AND c.AREA_ID = 3    "
						"  AND ((c.END_TIME_REAL > TO_CHAR(SYSDATE - 2, 'YYYYMMDDHH24MISS') OR c.END_TIME_REAL = ' ' ) OR a.RUN_STATUS < '53')    "
						"  UNION ALL    "
						"  SELECT  D.*, E.CC_SEQ,E.SG_SIGN,E.CAST_LOT_NO,E.CAST_LOT_DIV_NO,E.CAST_LOT_SUM,E.REFINE_DIV,E.PLAN_POUR_WT,E.NEW_TEST_NO,E.PLAN_DATE   "
						"  ,E.CC_PREP_TIME,E.POUR_TIME,E.CC_REQ_TIME_FLAG,E.HOT_SEND_FLAG,E.HOT_CHARGE_FLAG,E.CARRY_DIV,E.SLAB_DEST,E.BILLET_TYPE,E.SLAB_THICK   "
						"  ,E.SLAB_WIDTH,E.SLAB_LEN,E.SHIFT_NO,E.SHIFT_GROUP,E.C_DIV,E.CC_REQ_TIMEL4,E.CAST_LOT_NO2,E.CAST_LOT_DIV_NO2   "
						"  ,E.CAST_LOT_SUM2 FROM TPSSM41 D ,TPSSM40 E,TPSSM42 F WHERE D.FACTORY_DIV = @v_factory_div   "
						"  and D.factory_div = E.factory_div and D.pono = E.pono AND D.SM_PLAN_NO = F.SM_PLAN_NO AND F.AREA_ID = 3    "
						"  AND ((F.END_TIME_REAL > TO_CHAR(SYSDATE - 2, 'YYYYMMDDHH24MISS') ) OR D.RUN_STATUS < '53') )    "
						"  SELECT * FROM TEMP  ORDER BY CAST_NO ASC, CAST_DIV_NO ASC    ";
				}
				else
				{
					Log::Trace("", __FUNCTION__, "history");
					sqlstr = "  with temp AS  (   "
						"  SELECT  a.*, B.CC_SEQ,B.SG_SIGN,B.CAST_LOT_NO,B.CAST_LOT_DIV_NO,B.CAST_LOT_SUM,B.REFINE_DIV,B.PLAN_POUR_WT,B.NEW_TEST_NO,B.PLAN_DATE   "
						"  ,B.CC_PREP_TIME,B.POUR_TIME,B.CC_REQ_TIME_FLAG,B.HOT_SEND_FLAG,B.HOT_CHARGE_FLAG,B.CARRY_DIV,B.SLAB_DEST,B.BILLET_TYPE,B.SLAB_THICK   "
						"  ,B.SLAB_WIDTH,B.SLAB_LEN,B.SHIFT_NO,B.SHIFT_GROUP,B.C_DIV,B.CC_REQ_TIMEL4,B.CAST_LOT_NO2,B.CAST_LOT_DIV_NO2   "
						"  ,B.CAST_LOT_SUM2 FROM TPSSM11 A ,TPSSM10 B,TPSSM12 C WHERE a.FACTORY_DIV = @v_factory_div    "
						"  and a.factory_div = b.factory_div and a.pono = b.pono AND a.SM_PLAN_NO = c.SM_PLAN_NO AND c.AREA_ID = 3    "
						"  AND ((c.END_TIME_REAL >= @history_time_s and c.END_TIME_REAL <= @history_time_e ) OR a.RUN_STATUS < '53')    "
						"  UNION ALL    "
						"  SELECT  D.*, E.CC_SEQ,E.SG_SIGN,E.CAST_LOT_NO,E.CAST_LOT_DIV_NO,E.CAST_LOT_SUM,E.REFINE_DIV,E.PLAN_POUR_WT,E.NEW_TEST_NO,E.PLAN_DATE   "
						"  ,E.CC_PREP_TIME,E.POUR_TIME,E.CC_REQ_TIME_FLAG,E.HOT_SEND_FLAG,E.HOT_CHARGE_FLAG,E.CARRY_DIV,E.SLAB_DEST,E.BILLET_TYPE,E.SLAB_THICK   "
						"  ,E.SLAB_WIDTH,E.SLAB_LEN,E.SHIFT_NO,E.SHIFT_GROUP,E.C_DIV,E.CC_REQ_TIMEL4,E.CAST_LOT_NO2,E.CAST_LOT_DIV_NO2   "
						"  ,E.CAST_LOT_SUM2 FROM TPSSM41 D ,TPSSM40 E,TPSSM42 F WHERE D.FACTORY_DIV = @v_factory_div   "
						"  and D.factory_div = E.factory_div and D.pono = E.pono AND D.SM_PLAN_NO = F.SM_PLAN_NO AND F.AREA_ID = 3    "
						"  AND ((F.END_TIME_REAL >= @history_time_s and F.END_TIME_REAL <= @history_time_e ) OR D.RUN_STATUS < '53') )    "
						"  SELECT * FROM TEMP  ORDER BY CAST_NO ASC, CAST_DIV_NO ASC    ";
				}
				break;
			}
			//Log::Trace("", __FUNCTION__, "主计划数据查询开始sqlstr = [{0}]", sqlstr);
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("v_factory_div", v_factory_div.Trim());
			cmd_tpssm11_inq.Parameters.Set("history_time_s", history_time_s.Trim());
			cmd_tpssm11_inq.Parameters.Set("history_time_e", history_time_e.Trim());
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				//plan_num ++;
				k = cmd_tpssm11_inq.Fetch(tpssm11);
				//Log::Trace("", __FUNCTION__, "k = [{0}]", k);
				k = cmd_tpssm11_inq.Fetch(tpssm10);
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
				if (history_time.Trim().SubstringNE(0, 1) == "0" || history_time.Trim() == "")
				{
					row_plan["BASE_TIME"] = base_time;
				}
				else
				{
					row_plan["BASE_TIME"] = history_time;
				}
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

				if (row_plan["LADLE_NO"].ToString().GetLength() > 1 && row_plan["LADLE_NO"].ToString().Substring(0, 1) == "U")
				{
					row_plan["LADLE_NO"] = " ";
				}
			}
			cmd_tpssm11_inq.Close();
		}

		if (query_type == "3")
		{
			//----------------------------------------------------------
			//第三块，设备代码
			Log::Trace("", __FUNCTION__, "2.设备代码查询");
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
		}
		//}
		if (query_type == "2" || query_type == "3") //查询制造命令信息，第4块
		{
			//----------------------------------------------------------
			//第四块，制造命令相关
			Log::Trace("", __FUNCTION__, "3.制造命令查询");
			CModel tpssmda("TPSSMDA");
			CModel tqmts0x("TQMTS0X");

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT A.*, NVL(B.CODE_DESC_1_CONTENT, ' ') FROM TPSSM10 A "
					"	LEFT OUTER JOIN (SELECT * FROM TEP0002 WHERE CODE_CLASS = 'PM16') B "
					"	ON A.SLAB_DEST = B.CODE "
					"  WHERE A.FACTORY_DIV = @v_factory_div "
					"    AND A.PONO_STATUS IN (15, 16) "
					"  ORDER BY A.CC_MACH_NO, A.CC_SEQ ASC "  //因没有浇铸顺画面，暂时用此方式排序 
					);
				break;
			}
			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("v_factory_div", v_factory_div);

			//Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm10_inq.ExecuteReader();
			while (cmd_tpssm10_inq.Read())
			{
				k = cmd_tpssm10_inq.Fetch(tpssm10, 1);
				v_slab_dest = cmd_tpssm10_inq.GetString(k);
				//精炼工序的第一个charge_no 必定不为0
				//Log::Trace("", __FUNCTION__, "PONO = [{0}]", tpssm10["PONO"].ToString());
				//Log::Info("", __FUNCTION__, "v_slab_dest =[{0}]", v_slab_dest);

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

				cast_lot_no_and_div = tpssm10["CAST_LOT_NO"].ToString() + "-" + tpssm10["CAST_LOT_DIV_NO"].ToString();
				row_pono["CAST_LOT_NO"] = cast_lot_no_and_div;
				cast_lot_no_and_div = tpssm10["CAST_LOT_NO2"].ToString() + "-" + tpssm10["CAST_LOT_DIV_NO2"].ToString();
				row_pono["CAST_LOT_NO2"] = cast_lot_no_and_div;

				row_pono["CAST_LOT_NO_00"] = tpssm10["CAST_LOT_NO"].ToString();
				row_pono["CAST_LOT_DIV_NO_00"] = tpssm10["CAST_LOT_DIV_NO"].ToString();

				row_pono["POUR_TIME"] = tpssm10["POUR_TIME"];
				row_pono["SLAB_DEST"] = v_slab_dest;
				row_pono["CI_DIV"] = tpssm10["BACKLOG_EA"].ToString().Substring((tpssm10["BACKLOG_EA"].ToString().GetLength() - 1), 1);
				row_pono["C_DIV"] = tpssm10["C_DIV"];
				row_pono["CC_REQ_TIME"] = tpssm10["CC_REQ_TIME"];
				row_pono["CC_REQ_TIMEL4"] = tpssm10["CC_REQ_TIMEL4"];
				row_pono["ROUTELIST"] = tpssm10["ROUTELIST"];
				row_pono["ROUTEBAGKEY"] = tpssm10["ROUTEBAGKEY"];
				row_pono["SLAB_THICK"] = tpssm10["SLAB_THICK"].ToDecimal();
				row_pono["SLAB_WIDTH"] = tpssm10["SLAB_WIDTH"].ToDecimal();
				row_pono["PLAN_TAP_WT"] = tpssm10["PLAN_TAP_WT"].ToDecimal();

				prod_density = 7.85;
				tpssmd9["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];
				tpssmd9["BILLET_TYPE"] = tpssm10["BILLET_TYPE"];
				tpssmd9["CAST_THICK"] = tpssm10["SLAB_THICK"];
				tpssmd9["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
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

				if (tpssmda["CAST_SPEED"].ToDecimal() == 0)
				{
					//tqmts0x.Reset();
					//tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
					if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'C' || tpssm10["ROUTEBAGKEY"].ToString()[0] == 'S')
					{
						//tqmts0x.Query("ST_NO");
						CString c_div = " ";
						if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'C')
						{
							c_div = "DEFAULTC";
						}
						else if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'S')
						{
							c_div = "DEFAULTS";
						}
						//Log::Trace("", __FUNCTION__, "c_div=[{0}]]", c_div);
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
								" AND CC_MACH_NO		= @tpssm10.CC_MACH_NO "
								" AND BILLET_TYPE		= @tpssm10.BILLET_TYPE "
								" AND ST_NO			= @tpssm10.ST_NO  "
								" AND CAST_WIDTH_MIN <= @tpssm10.SLAB_WIDTH "
								" AND CAST_WIDTH_MAX >= @tpssm10.SLAB_WIDTH "
								" AND CAST_THICK_MIN <= @tpssm10.SLAB_THICK "
								" AND CAST_THICK_MAX >= @tpssm10.SLAB_THICK ";
							break;
						}


						cmd_tpssmda_inq.SetCommandText(sqlstr);
						cmd_tpssmda_inq.Parameters.Set("v_factory_div", v_factory_div);
						cmd_tpssmda_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
						cmd_tpssmda_inq.Parameters.Set("tpssm10.BILLET_TYPE", tpssm10["BILLET_TYPE"].ToString());
						cmd_tpssmda_inq.Parameters.Set("tpssm10.ST_NO", c_div);
						cmd_tpssmda_inq.Parameters.Set("tpssm10.SLAB_WIDTH", tpssm10["SLAB_WIDTH"].ToDecimal());
						cmd_tpssmda_inq.Parameters.Set("tpssm10.SLAB_THICK", tpssm10["SLAB_THICK"].ToDecimal());
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
				//Log::Info("", __FUNCTION__, "tpssmda[CAST_SPEED] = {0},tpssm10[PLAN_TAP_WT] = {1},tpssm10[SLAB_THICK] ={2},tpssm10[SLAB_WIDTH] = {3},tpssmd9[STRAND_NUM] = {4}", tpssmda["CAST_SPEED"].ToDecimal(), tpssm10["PLAN_TAP_WT"].ToDecimal(), tpssm10["SLAB_THICK"].ToDecimal(), tpssm10["SLAB_WIDTH"].ToDecimal(), tpssmd9["STRAND_NUM"].ToDecimal());
				if (tpssmda["CAST_SPEED"].ToDecimal() == 0 || tpssm10["PLAN_TAP_WT"].ToDecimal() == 0 || tpssm10["SLAB_THICK"].ToDecimal() == 0 || tpssm10["SLAB_WIDTH"].ToDecimal() == 0 || prod_density == 0 || tpssmd9["STRAND_NUM"].ToDecimal() == 0)
				{
					pour_time1 = 40;
				}
				else
				{
					pour_time1 = (tpssm10["PLAN_TAP_WT"].ToDecimal() * 1000 * 1000 * 1000) / (prod_density * tpssm10["SLAB_THICK"].ToDecimal() * tpssm10["SLAB_WIDTH"].ToDecimal() * tpssmda["CAST_SPEED"].ToDecimal() *tpssmd9["STRAND_NUM"].ToDecimal());	//
					//Log::Info("",__FUNCTION__, "计算得出浇铸时间 pour_time=[{0}]", pour_time1);
				}
				pour_time = pour_time1.ToInt32();
				row_pono["POUR_TIME"] = pour_time;

			}
			cmd_tpssm10_inq.Close();
		}


		//if (query_type != "2")
		//{
		if (query_type == "1" || query_type == "3")
		{
			//-------------------------------------------------------
			//第五块，设备维修
			Log::Trace("", __FUNCTION__, "4.设备特殊状态");
			CModel tpssm18("TPSSM18");

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
					"    AND DEV_STATUS = '1' "
					"    AND END_TIME > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') "
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
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第六块 各工序设备处理时间
			Log::Trace("", __FUNCTION__, "5.设备处理时间");
			CModel tpssmd3("TPSSMD3");
			CModel tpssmd3_s("TPSSMD3");

			EIClass TPSSMD3_SOURCE;
			EIClass TPSSMD3_CONDI;
			TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "ST_NO");
			TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
			TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
			TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "SMELT_MODE");
			TPSSMD3_CONDI.Tables[0].Rows.Add();
			EIClass TPSSMD3_RESULT;

			sqlstr = 
				" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
				" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,"
				" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
				" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE "
				" UNION"
				" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
				" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
				" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) ";
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
				//Log::Trace("", __FUNCTION__,"查询出钢记号ST_NO=[{0}]",tpssm10["ST_NO"].ToString());

				//////////////并行补丁////////////////
				//sqlstr = CString(
				//	" SELECT GRADE_ID,ACTIVITY_LABEL,ACTIVITY_DURATION FROM TPSSMD3_TEST WHERE GRADE_ID = @GRADE_ID AND ROUTEBAG_LABEL = 'NIL' "
				//	);
				//cmd_inq.SetCommandText(sqlstr);
				//cmd_inq.Parameters.Set("GRADE_ID", tpssm10["ST_NO"].ToString());
				//cmd_inq.ExecuteQuery(tb_tpssmd3);

				//for (int i = 0; i < tb_tpssmd3.Rows.get_Count(); i++)
				//{
				//	tpssmd3["ST_NO"] = tb_tpssmd3.Rows[i]["GRADE_ID"].ToString();
				//	tpssmd1["DEV_TECH_CODE"] = tb_tpssmd3.Rows[i]["ACTIVITY_LABEL"].ToString();
				//	tpssmd3["STD_PROC_TIME"] = tb_tpssmd3.Rows[i]["ACTIVITY_DURATION"].ToDecimal();

				//	if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "A" || tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "B" || tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "E")
				//	{
				//		tpssmd1["AREA_ID"] = 3;
				//	}
				//	else if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "F" || tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "R" || tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "S" || tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "V")
				//	{
				//		tpssmd1["AREA_ID"] = 4;
				//	}
				//	else if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Z")
				//	{
				//		tpssmd1["AREA_ID"] = 2;
				//	}

				//	tpssmd3["SMELT_MODE2"] = " ";
				//	tpssmd3["SMELT_MODE"] = 0;

				//	sqlstr = CString(
				//		" SELECT DISTINCT DEV_CODE FROM TPSSMD1 WHERE DEV_TECH_CODE = @DEV_TECH_CODE AND AREA_ID = @AREA_ID AND FACTORY_DIV = @FACTORY_DIV "
				//		);
				//	cmd_inq2.SetCommandText(sqlstr);
				//	cmd_inq2.Parameters.Set("DEV_TECH_CODE", tpssmd1["DEV_TECH_CODE"].ToString());
				//	cmd_inq2.Parameters.Set("AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
				//	cmd_inq2.Parameters.Set("FACTORY_DIV", v_factory_div);
				//	cmd_inq2.ExecuteReader();
				//	Log::Trace("", __FUNCTION__, "v_factory_div =[{0}]", v_factory_div);
				//	Log::Trace("", __FUNCTION__, "while里查询到DEV_TECH_CODE=[{0}]", tpssmd1["DEV_TECH_CODE"].ToString());
				//	Log::Trace("", __FUNCTION__, "while里查询到tpssmd1[AREA_ID] =[{0}]", tpssmd1["AREA_ID"].ToDecimal());

				//	while (cmd_inq2.Read())
				//	{
				//		tpssmd1["DEV_CODE"] = cmd_inq2.GetString(1);
				//		Log::Trace("", __FUNCTION__, "DEV_CODE =[{0}]", tpssmd1["DEV_CODE"].ToString());
				//		if ((tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "1" || tpssmd3["SMELT_MODE"].ToDecimal().ToString().Trim() == "0") && tpssmd1["AREA_ID"].ToString().Trim() != "2")
				//		{
				//			CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				//			row_dt["ST_NO"] = tpssmd3["ST_NO"];
				//			row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				//			row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				//			row_dt["SMELT_FLAG"] = "1";//常规
				//			row_dt["SMELT_FLAG2"] = tpssmd3["SMELT_MODE2"];
				//			row_dt["PREP_TIME"] = "0";

				//			if (tpssmd1["DEV_CODE"][0] == 'B' || tpssmd1["DEV_CODE"][0] == 'E')
				//			{
				//				Log::Trace("", __FUNCTION__, "DEV_CODE2 =[{0}]", tpssmd1["DEV_CODE"].ToString());
				//				CDataRow & row_dt2 = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				//				row_dt2["ST_NO"] = tpssmd3["ST_NO"];
				//				row_dt2["DEV_CODE"] = tpssmd1["DEV_CODE"];
				//				row_dt2["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				//				row_dt2["SMELT_FLAG"] = "2";//常规
				//				row_dt2["SMELT_FLAG2"] = tpssmd3["SMELT_MODE2"];
				//				row_dt2["PREP_TIME"] = "0";
				//			}
				//		}

				//		else if (tpssmd1["AREA_ID"].ToString().Trim() == "2" && tpssmd1["DEV_CODE"][0] == 'Z')
				//		{
				//			CDataRow & row_dt = bcls_ret->Tables["PROC_TIME"].Rows.Add();
				//			row_dt["ST_NO"] = tpssmd3["ST_NO"];
				//			row_dt["DEV_CODE"] = tpssmd1["DEV_CODE"];
				//			row_dt["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
				//			row_dt["SMELT_FLAG"] = "1";//常规
				//			row_dt["SMELT_FLAG2"] = " ";
				//			row_dt["PREP_TIME"] = "0";
				//		}
				//		else
				//		{

				//		}


				//	}
				//	cmd_inq2.Close();

				//}

				//////////////////////////////////////

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
						" SELECT DISTINCT d3.ST_NO, d1.DEV_TECH_CODE, d3.STD_PROC_TIME, d1.AREA_ID, d3.SMELT_MODE, d3.SMELT_MODE2, d3.STD_PREP_TIME"
						" FROM (SELECT t1.ST_NO,t1.SMELT_MODE,t1.SMELT_MODE2,t1.STD_PREP_TIME,t1.FACTORY_DIV,t1.DEV_CODE,"
						" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
						" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE"
						" UNION SELECT t1.ST_NO,t1.SMELT_MODE,t1.SMELT_MODE2,t1.STD_PREP_TIME,t1.FACTORY_DIV,t1.DEV_CODE,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
						" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) "
						" ) d3"
						" ,TPSSMD1 d1 "
						"  WHERE d3.ST_NO         = @st_no "
						"    AND d3.DEV_CODE = d1.DEV_TECH_CODE  "
						"   AND d3.factory_div = d1.factory_div "
						"   AND d1.factory_div = @tpssm10.factory_div "
						"   AND d3.SMELT_MODE2 = ' ' "
						);
					break;
				}

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("st_no", tpssm10["ST_NO"].ToString());
				cmd_inq.Parameters.Set("tpssm10.factory_div", v_factory_div);
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
			cmd_tpssm10_inq.Close();

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

				sqlstr = " SELECT T.ST_NO,A.DEV_CODE,T.STD_PROC_TIME ,A.AREA_ID, T.SMELT_MODE, T.SMELT_MODE2, T.STD_PREP_TIME "
					" FROM "
					" (SELECT t1.ST_NO,t1.SMELT_MODE,t1.SMELT_MODE2,t1.STD_PREP_TIME,t1.FACTORY_DIV,t1.DEV_CODE,"
					" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
					" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE"
					" UNION"
					" SELECT t1.ST_NO,t1.SMELT_MODE,t1.SMELT_MODE2,t1.STD_PREP_TIME,t1.FACTORY_DIV,t1.DEV_CODE,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
					" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) "
					" ) T"
					",TPSSMD1 A "
					" WHERE T.DEV_CODE = A.DEV_CODE "
					"   AND T.factory_div = A.factory_div "
					"   AND T.factory_div = @tpssm10.factory_div"
					"   AND T.st_no in ('DEFAULTC','DEFAULTS') "
					"   AND T.SMELT_MODE2 = ' ' "
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
					//tpssmd3_s["FACTORY_DIV"] = v_factory_div;
					//tpssmd3_s["DEV_CODE"] = tpssmd1["DEV_CODE"];
					//tpssmd3_s["ST_NO"] = tpssmd3["ST_NO"];
					//tpssmd3_s["SMELT_MODE"] = "0";
					//tpssmd3_s.Query("FACTORY_DIV,DEV_CODE,ST_NO,SMELT_MODE");

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
			//}
		}
		//PrintDataTable(bcls_ret->Tables["PROC_TIME"]);
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
		//

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第七块  传搁时间
			Log::Trace("", __FUNCTION__, "6.设备传搁时间");
			CModel tpssmd1_s("TPSSMD1");
			CModel tpssmd1_e("TPSSMD1");
			CModel tpssmd6("TPSSMD6");

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


				sqlstr =
					" SELECT t1.DEV_MOVE_START,COALESCE(T2.MOVE_TIME, T1.MOVE_TIME) AS MOVE_TIME,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV"
					" FROM TPSSMD6 t1 LEFT JOIN TPSSMD6_X t2"
					" ON  t1.DEV_MOVE_START = t2.DEV_MOVE_START AND  t1.DEV_MOVE_END = t2.DEV_MOVE_END"
					" WHERE t1.FACTORY_DIV=@v_factory_div "
					" UNION"
					" SELECT t1.DEV_MOVE_START,T1.MOVE_TIME,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV"
					" FROM TPSSMD6_X t1   WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD6) AND  t1.FACTORY_DIV=@v_factory_div ";
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
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第八块  连铸工艺时间
			Log::Trace("", __FUNCTION__, "7.连铸工艺时间");

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:


				sqlstr = " SELECT * FROM TPSSMD9 WHERE FACTORY_DIV = @v_factory_div  ";
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
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第九块 设备定检修类型配置 
			Log::Trace("", __FUNCTION__, "8.设备定检修类型配置");


			sqlstr = " SELECT * FROM TPSSM19 WHERE FACTORY_DIV = @v_factory_div ORDER BY AREA_ID ";
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
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第十块 钢种-路径包配置 
			Log::Trace("", __FUNCTION__, "9.钢种-路径包配置");
			CModel tpssmdh("TPSSMDH");

			sqlstr = " SELECT * FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm10) ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				cmd_inq.Fetch(tpssmdh);

				CDataRow & row_dh = bcls_ret->Tables["ST_NO_ROUTEBAG"].Rows.Add();
				row_dh.Merge(tpssmdh);
			}
			cmd_inq.Close();
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第十一块 路径包-路径配置 
			Log::Trace("", __FUNCTION__, "10.路径包-路径配置");
			CModel tpssmdj("TPSSMDJ");

			sqlstr = " SELECT * FROM TPSSMDJ ";
			cmd_inq.SetCommandText(sqlstr);
			//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				cmd_inq.Fetch(tpssmdj);

				CDataRow & row_dj = bcls_ret->Tables["ROUTEBAG_ROUTE"].Rows.Add();
				row_dj.Merge(tpssmdj);
			}
			cmd_inq.Close();
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第十二块 路径-设备列表配置 
			Log::Trace("", __FUNCTION__, "11.路径-设备列表配置");
			CModel tpssmd7("TPSSMD7");

			sqlstr = " SELECT * FROM TPSSMD7 WHERE FACTORY_DIV = @v_factory_div ORDER BY ROUTELIST,CHARGE_NO ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				cmd_inq.Fetch(tpssmd7);

				CDataRow & row_d7 = bcls_ret->Tables["ROUTE_DEV"].Rows.Add();
				row_d7.Merge(tpssmd7);
			}
			cmd_inq.Close();
		}

		if (query_type == "4")
		{
			//-------------------------------------------------------
			//第十三块 预定板坯号 
			Log::Trace("", __FUNCTION__, "12.预定板坯号数据");
			CModel tpssm03("TPSSM03");
			/*EIClass TPSSM11_SOURCE;

			EIClass TPSSM11_CONDI;

			TPSSM11_CONDI.Tables[0].Columns.Add(DT_STRING, "PONO");
			TPSSM11_CONDI.Tables[0].Rows.Add();

			EIClass TPSSM11_RESULT;*/
			//sqlstr = " SELECT * FROM TPSSM03 WHERE PONO IN ( SELECT PONO FROM TPSSM11 a,TPSSM12 b WHERE a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 3 AND ((b.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.END_TIME_REAL = ' ' ) OR a.RUN_STATUS < '53')) ORDER BY PONO,LSLAB_NO,SLAB_NO ";
			sqlstr = " SELECT * FROM TPSSM03 WHERE PONO IN "
				" ( SELECT PONO FROM TPSSM11 a,TPSSM12 b WHERE a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 3 "
				" AND ((b.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.END_TIME_REAL = ' ' ) OR a.RUN_STATUS < '53') "
				" union SELECT PONO "
				" FROM TPSSM41 c, TPSSM42 d "
				" WHERE c.SM_PLAN_NO = d.SM_PLAN_NO "
				" AND d.AREA_ID = 3 "
				" AND d.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') "
				") "
				" ORDER BY PONO,LSLAB_NO,SLAB_NO ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(tb_tpssm03);
			cmd_inq.Close();

			/*sqlstr = " SELECT * FROM TPSSM11 ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(TPSSM11_SOURCE.Tables[0]);
			cmd_inq.Close();*/

			if (tb_tpssm03.Rows.get_Count() > 0)
			{
				for (int i = 0; i < tb_tpssm03.Rows.get_Count(); i++)
				{
					tpssm03.MergeFrom(tb_tpssm03.Rows[i]);
					if (pono != tpssm03["PONO"].ToString())
					{
						pono = tpssm03["PONO"].ToString();

						//TPSSM11_CONDI.Tables[0].Rows[0]["PONO"] = pono;
						//TPSSM11_RESULT.Tables[0].Clear();
						//f_pssm_query(TPSSM11_CONDI, TPSSM11_SOURCE, TPSSM11_RESULT, conn);
						////PrintDataTable(TPSSM11_RESULT.Tables[0]);
						//tpssm11.MergeFrom(TPSSM11_RESULT.Tables[0].Rows[0]);

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
						row_03["SLAB_SIZE"] = row_03["SLAB_THICK"].ToString() + "*" + row_03["SLAB_WIDTH"].ToString() + "*" + row_03["SLAB_LEN"].ToString();
						row_03["ORDER_NO"] = tpssm03["ORDER_NO"].ToString();//合同号
						//row_03["SLAB_DEST"];//材料去向
						row_03["SG_SIGN"] = tpssm03["SG_SIGN"].ToString();//牌号
						//row_03["FACTORY_NEXT"];//下游工厂
						lslab_no = tb_tpssm03.Rows[i]["LSLAB_NO"].ToString();
					}
				}
			}
		}

		if (query_type == "1" || query_type == "3")
		{
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
			//}
		}

		if (query_type == "3")
		{
			//-------------------------------------------------------
			//第十五块 双工位交错时间 
			Log::Trace("", __FUNCTION__, "14.双工位交错时间配置");
			CModel tapbd006s2n("TAPBD006S2N");

			sqlstr = " SELECT * FROM TAPBD006S2N ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				cmd_inq.Fetch(tapbd006s2n);

				CDataRow & row_15 = bcls_ret->Tables["SHAREEQUIPMENTINFO"].Rows.Add();
				row_15["STATION_NAME"] = tapbd006s2n["STATION_NAME"].ToString() + "," + tapbd006s2n["STATION_NAME_2"].ToString();
				row_15["TD_TYPE"] = tapbd006s2n["TD_TYPE"].ToString();
				row_15["STAG_TIME"] = tapbd006s2n["STAG_TIME"].ToString();
				row_15["MOVE_TIME"] = tapbd006s2n["MOVE_TIME"].ToString();
			}
			cmd_inq.Close();
		}

		if (query_type == "1" || query_type == "3")
		{
			//-------------------------------------------------------
			//第十六块 极薄标识
			Log::Trace("", __FUNCTION__, "16.极薄标识");
		
			if (history_time.Trim().SubstringNE(0, 1) == "0" || history_time.Trim() == "")
			{
				sqlstr = " SELECT "
					" A.PONO,																  "
					" MIN(B.ORDER_THICK)													  "
					" FROM																	  "
					" (																		  "
					" SELECT																  "
					" ORDER_NO, PONO														  "
					" FROM																	  "
					" TPSSM03																  "
					" WHERE																	  "
					" PONO IN(																  "
					" SELECT																  "
					" *																		  "
					" FROM																	  "
					" (																		  "
					" SELECT																  "
					" PONO																	  "
					" FROM																	  "
					" tpssm11 C, tpssm12 D													  "
					" WHERE																	  "
					" C.SM_PLAN_NO = D.SM_PLAN_NO											  "
					" AND D.AREA_ID = 3														  "
					" AND((D.END_TIME_REAL > TO_CHAR(SYSDATE - 2, 'YYYYMMDDHH24MISS'))		  "
					" OR C.RUN_STATUS < '53')												  "
					" UNION ALL																  "
					" SELECT																  "
					" PONO																	  "
					" FROM																	  "
					" tpssm41 E, tpssm42 F													  "
					" WHERE																	  "
					" E.SM_PLAN_NO = F.SM_PLAN_NO											  "
					" AND F.AREA_ID = 3														  "
					" AND((F.END_TIME_REAL > TO_CHAR(SYSDATE - 2, 'YYYYMMDDHH24MISS'))		  "
					" OR E.RUN_STATUS < '53')))) A,											  "
					" TQMOM01 B																  "
					" WHERE																	  "
					" A.ORDER_NO = B.ORDER_NO												  "
					" GROUP BY																  "
					" A.PONO ";																  
			}
			else
			{
				sqlstr = " SELECT "
					" A.PONO,																"
					" MIN(B.ORDER_THICK)													"
					" FROM																	"
					" (																		"
					" SELECT																"
					" ORDER_NO, PONO														"
					" FROM																	"
					" TPSSM03																"
					" WHERE																	"
					" PONO IN(																"
					" SELECT																"
					" *																		"
					" FROM																	"
					" (																		"
					" SELECT																"
					" PONO																	"
					" FROM																	"
					" tpssm11 C, tpssm12 D													"
					" WHERE																	"
					" C.SM_PLAN_NO = D.SM_PLAN_NO											"
					" AND D.AREA_ID = 3														"
					" AND((D.END_TIME_REAL >= @history_time_s and D.END_TIME_REAL <= @history_time_e)		"
					" OR C.RUN_STATUS < '53')												"
					" UNION ALL																"
					" SELECT																"
					" PONO																	"
					" FROM																	"
					" tpssm41 E, tpssm42 F													"
					" WHERE																	"
					" E.SM_PLAN_NO = F.SM_PLAN_NO											"
					" AND F.AREA_ID = 3														"
					" AND((F.END_TIME_REAL >= @history_time_s and F.END_TIME_REAL <= @history_time_e)		"
					" OR E.RUN_STATUS < '53')))) A,											"
					" TQMOM01 B																"
					" WHERE																	"
					" A.ORDER_NO = B.ORDER_NO												"
					" GROUP BY																"
					" A.PONO																";
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("history_time_s", history_time_s.Trim());
			cmd_inq.Parameters.Set("history_time_e", history_time_e.Trim());
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				CDataRow & row_16 = bcls_ret->Tables["ORDER_THICK"].Rows.Add();
				row_16["PONO"] = cmd_inq.GetString(1);
				row_16["ORDER_THICK"] = cmd_inq.GetDecimal(2);
				row_16["THICK_RANGE"] = thick_range;
			}
			cmd_inq.Close();
		}

		//if (query_type != "2")
		//{
		//	sqlstr = "SELECT * FROM TPSSM11 WHERE FACTORY_DIV=@v_factory_div AND PONO_STATUS<83 ";
		//	sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		//	cmd_inq.SetCommandText(sqlstr);
		//	cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		//	cmd_inq.ExecuteQuery(tb_tpssm11.Tables[0]);
		//	Log::Trace("", __FUNCTION__, "1=[{0}]", tb_tpssm11.Tables[0].Rows.get_Count());
		//	//校验可编计划数如果大于0才需要优化
		//	if (tb_tpssm11.Tables[0].Rows.get_Count() > 0)
		//	{
		//		ret = f_pssm_call_tps_n(v_factory_div, mode, conn);
		//		if (ret < 0)
		//		{
		//			throw CApplicationException(-1, s.msg, log.Location);
		//		}
		//	}
		//	cmd_inq.Close();
		//}
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
