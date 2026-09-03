/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    lijie
Version:   1.0
Date:      2011-12-12
Description: 出钢计划新增及炉次条件写入
Update：   2014-11-10  xuwen  炉次条件合并，子工序计划
Modify：   2015-11-18 转炉装入不影响吹炼、出钢时刻，连铸包到不影响开浇
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件










#if defined _SYS_PES
//发送计划状态给MMS(不应该此处调用????，在所有计划处理成功后调用，避免异步电文在调用点就发送)
//int f_ps200021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

/*<remark>=========================================================
/// <summary>
/// 出钢计划新增及炉次条件写入
/// <para>1.读取指定的默认设备</para>
/// <para>2.读取编入计划的PONO</para>
/// <para>3.判断PONO状态,是否已编入计划</para>
/// <para>4.生成出钢计划主表数据, 生成临时计划号; 生成出钢计划子表记录</para>
/// <para>5.对新编入出钢计划的纪录, 修改制造命令炉次表(tpssm10)中状态信息;</para>
/// <para>6.修改制造命令LOT表(TPSSM04)中状态信息</para>
/// <para>7.向MMS的炼钢作业计划发送编入出钢计划电文。</para>
/// <para>数据库表：tpssm11/12(炼钢出钢计划主表)</para>
/// <para>前台画面PSSM11的新增计划操作，pssm11_add(出钢计划编入)调用</para>
/// </summary>
/// <param name="PONO">制造命令号</param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm11t_ins_pono(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	/* 程序用变量 */
	int doFlag = 0;
	int i, j, blkseq, rows;
	int charge_no;
	int srf_count = 0;		 //精炼路径长度计数
	int ret = 0;


	CString v_pono = "";	 //输入PONO
	CString v_restrand_flag = "";   // 输入连连铸标记
	CString v_cc_req_time = "";     // 输入开浇时刻
	CString dev_code[100];      //记录指定的默认设备, 数组下标为:工序代码-'0'
	EIClass inBlock2;        //调用函数用
	CString dateNow14 = "";
	CDecimal dummy;
	CDecimal pono_status;         /* PONO状态 */
	CString ccm_no = "";          //当前连铸机号,用于校验编入计划的次序问题
	int     cc_seq = 0;     //连铸顺序号, 用于校验编入计划的次序问题
	CString v_factory_div = "";
	CDecimal cc_pour_time = 0;
	CDecimal prod_density = 0;
	CString factory_div_pre = "";
	CString cc_mach_no_pre = "";
	CString billet_type_pre = "";
	CString st_no_pre = "";
	CDecimal slab_width_pre = 0;
	CDecimal slab_thick_pre = 0;
	CDecimal cc_pour_time_pre = 0;
	CDecimal cc_prep_time = 0;
	CDecimal cc_prep_time_pre = 0;

	CString sqlstr;

	CModel tpssmd1("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
	CModel tpssmd9("TPSSMD9");
	CModel tpssmda("TPSSMDA");
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	//CTPSSM16 tpssm16(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssmd3_inq(conn);
	CDbCommand cmd_tpssmd4_inq(conn);
	CDbCommand cmd_tpssm02_upd(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm12_ins(conn);
	CDbCommand cmd_tpssm16_inq(conn);
	CDbCommand cmd_tpssmda_inq(conn);

	try
	{
		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");


		//定义函数调用输入块结构
		inBlock2.Tables[0].set_TableName("X200021");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "SM_UNIT_NO");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock2.Tables[0].Rows.Add();

		tpssm11["REC_REVISOR"] = s.userid;
		tpssm11["REC_REVISE_TIME"] = dateNow14;


		//----------------------------------------------------------------------------------
		//获得输入参数
		//1. 读取指定的默认设备，存储在dev_code数组中
		// 增加了炉机对应关系，在f_pssm11_set_dev中处理，此处注释 xuwen 2015-4-1
		blkseq = bcls_rec->Tables.IndexOf("DEV");
		if (blkseq < 0)
		{
			strcpy(s.msg, "传入默认设备数据块[DEV]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [DEV] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			tpssmd1["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			tpssmd1["STATION_ID"] = bcls_rec->Tables[blkseq].Rows[i]["STATION_ID"];
			tpssmd1["STATION_NO"] = bcls_rec->Tables[blkseq].Rows[i]["STATION_NO"];
			tpssmd1["DEV_CODE"] = bcls_rec->Tables[blkseq].Rows[i]["DEV_CODE"];
			////Log::Info("", __FUNCTION__, "dev_code=[{0}]", tpssmd1["DEV_CODE"].ToString());
			j = tpssmd1["STATION_ID"].ToString()[0] - 'A';
			dev_code[j] = tpssmd1["DEV_CODE"];   //2014-10起，设备代码扩位到2位
		}//for



		//2.读取编入计划的PONO
		blkseq = bcls_rec->Tables.IndexOf("PONO");
		if (blkseq < 0)
		{
			strcpy(s.msg, "传入编制计划的命令数据块[PONO]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PONO] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			v_factory_div = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"].ToString().Trim();
			v_pono = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString().Trim();
			v_restrand_flag = bcls_rec->Tables[blkseq].Rows[i]["RESTRAND_FLG"].ToString().Trim();
			v_cc_req_time = bcls_rec->Tables[blkseq].Rows[i]["CC_REQ_TIME"].ToString().Trim();

			tpssm10["PONO"] = v_pono.TrimOrBlank();
			tpssm10["FACTORY_DIV"] = v_factory_div.TrimOrBlank();

			//前台时间控件为空时，得到的值是 00010101000000。 一个很严重的bug
			if (v_cc_req_time == "00010101000000")
			{
				v_cc_req_time = "";
			}

			////Log::Info("", __FUNCTION__, "pono=[{0}], factory_div=[{1}], restrand_flag=[{2}], cc_req_time=[{3}]", tpssm10["PONO"].ToString(), tpssm10["FACTORY_DIV"].ToString(), v_restrand_flag, v_cc_req_time);

			//校验该PONO是否在TPSSM10表中存在
			sqlstr = "tpssm10.Query(PONO)";
			bool has10 = tpssm10.Query("PONO, FACTORY_DIV");  //主键查询

			if (has10 == false)
			{
				CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000024")/*制造命令号[{0}]不存在。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm10.TrimOrBlank();


			//----------------------------------
			//1).判断PONO状态,是否已编入计划
			if (tpssm10["PONO_STATUS"].ToDecimal() >= 18) //18 - 编入计划
			{
				CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000084")/*制造命令号[{0}]已排入出钢计划。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//20160831 周平 产品化无分割PONO 暂时先注释
			////2).新增计划时, 对分割PONO, 一定要有返送代码才能编制
			////返送代码有值时, 必定在TPSSM35生成了返送计划
			//if ( tpssm10.DIV_FLAG[0] == '1' && //分割PONO，'0'-正常PONO; '1'-分割PONO; tpssm10["PONO"].ToString()[2] >= '9' 
			//	!(tpssm10.STEEL_RETURN_CODE[0] == '6' || tpssm10.STEEL_RETURN_CODE[0] == '8') // 6(分割后侧), 8(重装后侧)
			//	)
			//{
			//	CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, _RES("PSSMS0000152")/*分割制造命令号[{0}]未做返送操作，不能排入出钢计划。*/, arguments, 1); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}


			//20160831 周平 产品化暂时先注释
			////3).判断PONO编入计划的顺序与TPSSM10表一致（不能有跳跃）. CAST的计算有难度
			//if (ccm_no == tpssm10["CC_MACH_NO"].ToString().Trim())  //判断某一连铸机下选择的第1炉
			//{
			//	cc_seq++;
			//}
			//else  //另一连铸机时
			//{
			//	ccm_no = tpssm10["CC_MACH_NO"].ToString().Trim();

			//	//读取未编入计划最小顺序号
			//	sqlstr = CString(
			//		" SELECT MIN(CC_SEQ) FROM TPSSM10 "
			//		"  WHERE CC_MACH_NO = @tpssm10.CC_MACH_NO "
			//		"    AND PONO_STATUS <= 16 "
			//		);
			//	cmd_tpssm10_inq.SetCommandText(sqlstr);
			//	cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
			//	cmd_tpssm10_inq.ExecuteReader();
			//	if (cmd_tpssm10_inq.Read()) //读取一条
			//	{
			//		cc_seq = cmd_tpssm10_inq.GetDecimal(1).ToInt32();
			//	}
			//	else
			//	{
			//		cc_seq = 0;
			//	}
			//	cmd_tpssm10_inq.Close();
			//}

			////////Log::Trace("", __FUNCTION__, "cc_seq=[{0}], CC_SEQ=[{1}]", cc_seq, tpssm10["CC_SEQ"].ToDecimal());

			////判断当前炉次是否按顺序选择
			//if (cc_seq != tpssm10["CC_SEQ"].ToDecimal().ToInt32())  //当前炉次的顺序与计算的不一致
			//{
			//	CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "制造命令号[{0}]未按浇铸顺序选择，其前边有炉次跳过。", arguments, 1); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			////Log::Info("", __FUNCTION__, "CC_MACH_NO=[{0}], ST_NO=[{1}]", tpssm10["CC_MACH_NO"].ToString(), tpssm10["ST_NO"].ToString());


			



			//----------------------------------------------------------------------------------------
			//生成出钢计划主表数据
			tpssm11["REC_CREATOR"] = s.userid;
			tpssm11["REC_CREATE_TIME"] = dateNow14;
			tpssm11["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];   /* 炼钢单元号 */
			tpssm11["PONO"] = tpssm10["PONO"];			/* 制造命令号 */
			tpssm11["ST_NO"] = tpssm10["ST_NO"];			/* 出钢记号 */
			//tpssm11["REFINE_ROUTE_CODE"] = tpssm10["REFINE_DIV"]; /* 精炼路径代码 2014年10月，精炼路径代码扩位，每2位表示一工序。*/
			tpssm11["HEAT_NO"] = " ";                    /* 熔炼号 */
			//tpssm11["STEEL_RETURN_CODE"]  = tpssm10.STEEL_RETURN_CODE;   /* 钢水返送代码 */
			tpssm11["PONO_STATUS"] = 18;
			tpssm11["SMELT_MODE"] = tpssm10["SMELT_MODE"];			/* 冶炼模式 1-普通法, 2-双联法 */
			//20160831 周平 产品化无分割PONO 暂时先注释
			//tpssm11["PLAN_TAP_WT"]        = tpssm10.DIV_FLAG[0] == '1'? tpssm10.PLAN_POUR_WT : tpssm10["PLAN_TAP_WT"];	/* 分割炉次取浇铸量 */
			tpssm11["PLAN_TAP_WT"] = tpssm10["PLAN_POUR_WT"];
			tpssm11["RESTRAND_FLG"] = v_restrand_flag;   /* 连连铸标记,编制对话框给出 */
			if (v_restrand_flag == "T")
			{
				tpssm11["TD_CHG_FLG"] = 0;	               /* 中间包更换标志，编制对话框给出 */
				tpssm11["INS_FE_FLAG"] = " ";                 /* 连连铸标记,编制对话框给出 */
			}
			else if (v_restrand_flag == "D")
			{
				tpssm11["TD_CHG_FLG"] = 1;	   /* 中间包更换标志，编制对话框给出 */
				tpssm11["INS_FE_FLAG"] = " ";                 /* 连连铸标记,编制对话框给出 */
			}
			else if (v_restrand_flag == "X")
			{
				tpssm11["TD_CHG_FLG"] = 0;	               /* 中间包更换标志，编制对话框给出 */
				tpssm11["INS_FE_FLAG"] = v_restrand_flag;     /* 连连铸标记,编制对话框给出 */
			}

			//不能同时快换中包和重引锭
			if (tpssm11["TD_CHG_FLG"].ToDecimal() == 1 && tpssm11["RESTRAND_FLG"].ToString() == "T")
			{
				CFormattable arguments[] = { v_pono };
				CMessageFormat::Format(s.msg, "炉次[{0}]同时快换中包和重引锭，不符合规则，请重新选定方式后继续保存计划。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}


			tpssm11["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];   /* 连铸机号 */
			//tpssm11["CAST_NO"];                     /* 连连浇号(CAST号)，此处不赋值，f_pssm11_cast()函数中计算 */
			//tpssm11["CAST_DIV_NO"];                    /* CAST分割号 */
			//tpssm11["CAST_PONO_SUM"];                  /* CAST内炉数 */
			tpssm11["BACKLOG_EA"] = tpssm10["BACKLOG_EA"];   /* 钢区工序途径 */
			tpssm11["LADLE_NO"] = " ";                    /* 钢包号 */
			//tpssm11["PLAN_STYLE"] = "0";                  /* 计划编制类型: 0-手工; 1-模型。 */
			tpssm11["CC_REQ_TIME"] = " ";                  /* CC要求时刻: 此处不赋值,炉次条件中设置。或出钢计划第一炉设定，f_pssm11_pour_time() 中计算*/
			tpssm11["EARLY_TIME"] = 0;	                   /* 早到时间 */
			tpssm11["RUN_STATUS"] = "00";             /* 炉次状态: 新增的必然是'0' */
			tpssm11["OUT_STEEL_WT"] = tpssm11["PLAN_TAP_WT"];   /* 出钢钢水重量 */
			tpssm11["PLAN_EDIT_FLAG"] = "N";                  /* 计划编辑标记, N-新增计划，后续计算用 */
			tpssm11["IC_CC_FLAG"] = tpssm10["BACKLOG_EA"].ToString().Substring(tpssm10["BACKLOG_EA"].ToString().GetLength() - 1, 1);

			//生成临时计划号，这个计划生成后，f_pssm11_planno()函数统一计算。
			//tpssm11["SM_PLAN_NO"] = 90000000 + i;  //8位，9开头的做为新增计划的临时处理号，后边赋正式计划号
			tpssm11["SM_PLAN_NO"] = CString::Format("P%.5d", i);

			if ((factory_div_pre == tpssm10["FACTORY_DIV"].ToString())
				&& (cc_mach_no_pre == tpssm10["CC_MACH_NO"].ToString())
				&& (billet_type_pre == tpssm10["BILLET_TYPE"].ToString())
				&& (st_no_pre == tpssm10["ST_NO"].ToString())
				&& (slab_width_pre == tpssm10["SLAB_WIDTH"].ToDecimal())
				&& (slab_thick_pre == tpssm10["SLAB_THICK"].ToDecimal()))
			{
				cc_pour_time = cc_pour_time_pre;
			}
			else
			{
				//获取连铸开始时间
				prod_density = 7.85;
				tpssmda["CAST_SPEED"] = 0;
				tpssmd9["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];
				tpssmd9["BILLET_TYPE"] = tpssm10["BILLET_TYPE"];
				tpssmd9["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];

				tpssmd9["CAST_THICK"] = tpssm10["SLAB_THICK"];

				//模铸宽度默认维护成0
				if (tpssmd9["BILLET_TYPE"] = "5")
				{
					tpssmd9["CAST_THICK"] = 0;
				}

				tpssmd9.Query("FACTORY_DIV,BILLET_TYPE,CAST_THICK,CC_MACH_NO");

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT CAST_SPEED FROM TPSSMDA \
								WHERE FACTORY_DIV	= @tpssm10.FACTORY_DIV \
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
				cmd_tpssmda_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
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

					cc_pour_time = 40;
				}
				else
				{
					cc_pour_time = (tpssm10["PLAN_TAP_WT"].ToDecimal() * 1000 * 1000 * 1000) / (prod_density * tpssm10["SLAB_THICK"].ToDecimal() * tpssm10["SLAB_WIDTH"].ToDecimal() * tpssmda["CAST_SPEED"].ToDecimal() * tpssmd9["STRAND_NUM"].ToDecimal());	//
					////Log::Info("", __FUNCTION__, "计算得出浇铸时间 pour_time=[{0}]", cc_pour_time);
				}

				factory_div_pre = tpssm10["FACTORY_DIV"];
				cc_mach_no_pre = tpssm10["CC_MACH_NO"];
				billet_type_pre = tpssm10["BILLET_TYPE"];
				st_no_pre = tpssm10["ST_NO"];
				slab_width_pre = tpssm10["SLAB_WIDTH"];
				slab_thick_pre = tpssm10["SLAB_THICK"];
				cc_pour_time_pre = cc_pour_time;
			}

			//----------------------------------------------------------------------------------------
			// 生成出钢计划子表记录
			// 1.生成各工序, 子工序的记录
			// 2.当返送代码为:6(分割后侧), 8(重装后侧)的炉次, 取其前侧PONO的BOF工序的炉次条件
			//
			tpssm12["REC_CREATOR"] = s.userid;
			tpssm12["REC_CREATE_TIME"] = dateNow14;
			tpssm12["REC_REVISOR"] = s.userid;
			tpssm12["REC_REVISE_TIME"] = dateNow14;
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

			//根据 backlog_ea (钢区工序途径)生成各工序, 子工序的记录
			srf_count = 0;
			charge_no = 0;
			tpssm11["REFINE_ROUTE_CODE"] = "";  //精炼路径（设备代码组成）

			////Log::Trace("", __FUNCTION__, "BACKLOG_EA=[{0}]", tpssm11["BACKLOG_EA"].ToString());
			for (j = 0; j < tpssm11["BACKLOG_EA"].ToString().Trim().GetLength(); j++)
			{

				//获取对应工序代码
				tpssmd1["STATION_ID"] = tpssm11["BACKLOG_EA"].ToString().SubstringNE(j, 1);
				tpssmd1["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

				//根据工序读取区域号
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:         // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default:  // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT AREA_ID FROM TPSSMD1 "
						"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
						"    AND STATION_ID = @tpssmd1.STATION_ID "
						);
					break;
				}
				cmd_tpssmd1_inq.SetCommandText(sqlstr);
				cmd_tpssmd1_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
				cmd_tpssmd1_inq.Parameters.Set("tpssmd1.STATION_ID", tpssmd1["STATION_ID"].ToString());
				cmd_tpssmd1_inq.ExecuteReader();
				if (cmd_tpssmd1_inq.Read()) //读取一条
				{
					tpssmd1["AREA_ID"] = cmd_tpssmd1_inq.GetDecimal(1);
				}
				else
				{
					CFormattable arguments[] = { tpssm11["PONO"].ToString(), tpssmd1["STATION_ID"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000162")/*制造命令号[{0}]的工位[{1}]配置信息出错，请联系维护人员。*/, arguments, 2); //格式化字符串
					CMessageFormat::Format(s.sysmsg, "PONO[{0}]中工位标识[{1}]不能确定是炼钢区域。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				cmd_tpssmd1_inq.Close();

				//非炼钢工序不编入出钢计划
				if (tpssmd1["AREA_ID"].ToDecimal() < 2)
				{
					continue;
				}

				charge_no++;
				//给 charge_no 赋值, 必须是流水的
				tpssm12["CHARGE_NO"] = charge_no;
				tpssm12["SUB_CHARGE_NO"] = 0;     //0-主体工序。 1、2、3-子工序记录。

				//读取默认设备
				if (tpssmd1["AREA_ID"].ToDecimal() == 5) //连铸工序
				{

					////Log::Trace("", __FUNCTION__, "取连铸设备:DEV_ID=[{0}], CC_MACH_NO=[{1}]。", tpssmd1["STATION_ID"].ToString(), tpssm10["CC_MACH_NO"].ToString());

					//根据连铸机号，获取设备号，唯一决定
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:         // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default:  // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" SELECT DEV_CODE FROM TPSSMD1 "
							"  WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV "
							"    AND STATION_ID = @tpssmd1.STATION_ID "
							"    AND STATION_NO = @tpssmd1.STATION_NO "
							);
						break;
					}
					cmd_tpssmd1_inq.SetCommandText(sqlstr);
					cmd_tpssmd1_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
					cmd_tpssmd1_inq.Parameters.Set("tpssmd1.STATION_ID", tpssmd1["STATION_ID"].ToString());
					cmd_tpssmd1_inq.Parameters.Set("tpssmd1.STATION_NO", tpssmd1["STATION_NO"].ToString());
					cmd_tpssmd1_inq.ExecuteReader();
					if (cmd_tpssmd1_inq.Read()) //读取一条
					{
						tpssmd1["DEV_CODE"] = cmd_tpssmd1_inq.GetString(1);
					}
					else
					{
						CFormattable arguments[] = { tpssm11["PONO"].ToString(), tpssmd1["STATION_ID"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, _RES("PSSMS0000162")/*制造命令号[{0}]的工位[{1}]配置信息出错，请联系维护人员。*/, arguments, 2); //格式化字符串
						CMessageFormat::Format(s.sysmsg, "PONO[{0}]中工位标识[{1}]不能确定是炼钢区域。", arguments, 2);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					cmd_tpssmd1_inq.Close();

				}
				else if (tpssmd1["AREA_ID"].ToDecimal() == 1) //1-脱S工序
				{

					//对于脱硫，不应纳入炼钢调度中。模型计算及解消冲突没有意义。推算开始并记录在计划主表中即可
					charge_no = 1;
					tpssm12["CHARGE_NO"] = charge_no;

					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:         // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default:  // 所有数据库适用，通用SQL语句
						sqlstr = CString(
							" SELECT DEV_CODE FROM TPSSMD1 "
							"  WHERE DEV_ID	= 'S' "
							" ORDER BY DEFAULT_FLAG DESC "  //增加默认标记排序，保证能获取到设备
							);
						break;
					}
					cmd_tpssmd1_inq.SetCommandText(sqlstr);
					cmd_tpssmd1_inq.ExecuteReader();
					if (cmd_tpssmd1_inq.Read())
					{
						tpssmd1["DEV_CODE"] = cmd_tpssmd1_inq.GetString(1);
					}
					cmd_tpssmd1_inq.Close();

				}
				else
				{
					tpssmd1["DEV_CODE"] = dev_code[tpssmd1["STATION_ID"].ToString()[0] - 'A'];
				}

				//子工序区域、设备代码赋值
				tpssm12["DEV_CODE"] = tpssmd1["DEV_CODE"];
				tpssm12["AREA_ID"] = tpssmd1["AREA_ID"];
				////Log::Trace("", __FUNCTION__, "DEV_CODE = [{0}], AREA_ID = [{1}]", tpssm12["DEV_CODE"].ToString(), tpssm12["AREA_ID"].ToDecimal());


				//确定出钢计划的精炼路径，
				if (tpssmd1["AREA_ID"].ToDecimal() == 4)
				{
					tpssm11["REFINE_ROUTE_CODE"] = tpssm11["REFINE_ROUTE_CODE"].ToString() + tpssm12["DEV_CODE"].ToString();
				}

				//20160831 周平 产品化无分割PONO 暂时先注释
				////-----------------------------------------------------------------------------------
				//// 当返送代码为:6(分割后侧), 8(重装后侧)的炉次, 取其前侧PONO的BOF工序的炉次条件
				//if ( (tpssm10.STEEL_RETURN_CODE[0] == '6' || tpssm10.STEEL_RETURN_CODE[0] == '8') &&  // 6(分割后侧), 8(重装后侧)
				//	tpssmd1["AREA_ID"].ToDecimal() == 3  //当为分割炉次时, BOF工序的炉次条件复制前侧PONO的
				//	)
				//{
				//	continue;

				//}//当为分割炉次时的赋值

				//-----------------------------------------------------------------------------------
				//正常炉次时(非分割炉次)的赋值
				//1.根据工序标准处理时间(非连铸设备), 读取并写入各工序的标准处理时间
				if (tpssmd1["AREA_ID"].ToDecimal() == 5)  //连铸设备临时赋值。具体浇铸时间在浇铸函数中计算
				{
					tpssmd3["STD_PROC_TIME"] = cc_pour_time;
				}
				else
				{

					//读取工序标准作业时间：工序设备标识 -> PATTERN -> 处理时间				
					//1)根据 工序设备标识+出钢记号--> PATTERN号
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
							" SELECT * "
							"   FROM TPSSMD3 "
							"  WHERE ST_NO	= @tpssm11.ST_NO "
							"   AND SMELT_MODE	= @tpssm11.SMELT_MODE "
							"   AND FACTORY_DIV = @tpssm12.FACTORY_DIV "
							"   AND DEV_CODE	= @tpssm12.DEV_CODE "
							);
						break;
					}

					cmd_tpssmd3_inq.SetCommandText(sqlstr);
					cmd_tpssmd3_inq.Parameters.Set("tpssm11.ST_NO", tpssm11["ST_NO"].ToString());
					cmd_tpssmd3_inq.Parameters.Set("tpssm11.SMELT_MODE", tpssm11["SMELT_MODE"].ToDecimal());
					cmd_tpssmd3_inq.Parameters.Set("tpssm12.FACTORY_DIV", tpssm12["FACTORY_DIV"].ToString());
					cmd_tpssmd3_inq.Parameters.Set("tpssm12.DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cmd_tpssmd3_inq.ExecuteReader();

					if (cmd_tpssmd3_inq.Read())
					{
						cmd_tpssmd3_inq.Fetch(tpssmd3);
					}
					cmd_tpssmd3_inq.Close();
				}

				//-------------------------------------------------------------
				//炼钢计划的工序计划生成（主工序计划:SUB_CHARGE_NO = 0）
				tpssm12["SUB_CHARGE_NO"] = 0;     //0-主工序计划

				//2015-1-21 标准处理时间调整，准备和出钢时间都不在 D4+D5 表中设置
				if (tpssm12["AREA_ID"].ToDecimal() == 2 || tpssm12["AREA_ID"].ToDecimal() == 3)
				{
					if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
					{
						tpssmd3["STD_PROC_TIME"] = 40;
					}

					tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间 */

					/* 前处理时间 = 装入时间计入 2015-11-18 修改，模型的计划入口参数 */
					tpssm12["PRE_PROC_TIME"] = tpssmd3["FEED_TIME"];

					//对于转炉，主工序的处理时间 = 装入 + 处理 + 出钢
					//tpssm12["PROC_TIME"] = tpssmd3["FEED_TIME"].ToDecimal() + tpssmd5.STD_PROC_TIME + tpssmd3["DRAW_TIME"].ToDecimal();      /* 处理时间=装入+处理+出钢 */
					tpssm12["PROC_TIME"] = tpssmd3["STD_PROC_TIME"].ToDecimal() + tpssmd3["DRAW_TIME"].ToDecimal(); /* 处理时间= 处理+出钢 2015-11-18 修改*/

					tpssm12["POST_PROC_TIME"] = 0;     /* 后处理时间=0 */
				}
				else
				{
					if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
					{
						tpssmd3["STD_PROC_TIME"] = 40;
					}

					tpssm12["PRE_PROC_TIME"] = tpssmd3["FEED_TIME"];   	/* 前处理时间=进站时间 */
					tpssm12["PROC_TIME"] = tpssmd3["STD_PROC_TIME"];      /* 处理时间 */
					tpssm12["POST_PROC_TIME"] = tpssmd3["DRAW_TIME"];     /* 后处理时间 */
					tpssm12["PREP_TIME"] = tpssmd3["STD_PREP_TIME"];      /* 作业间准备时间, 除连铸，其他工序暂时没用到 */
				}
				tpssm12["REST_TIME"] = 0;                          /* 休止时间 */


				////Log::Trace("", __FUNCTION__, "工序计划新增：SM_PLAN_NO=[{0}], CHARGE_NO=[{1}], AREA_ID=[{2}], DEV_CODE=[{3}]",
					//tpssm12["SM_PLAN_NO"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["AREA_ID"].ToDecimal(), tpssm12["DEV_CODE"].ToString());
				//tpssm12.Print();

				sqlstr = "tpssm12.Insert()";
				tpssm12.Insert();

				////-------------------------
				////转炉细分，再生成3个子阶段：装入、吹炼、出钢
				//if (tpssm12["AREA_ID"].ToDecimal() == 2 || tpssm12["AREA_ID"].ToDecimal() == 3)
				//{
				//	//细分子工序的准备时间（PREP_TIME）、休止时间（REST_TIME）必须为0。只有主工序有值
				//	tpssm12["PREP_TIME"] = 0;
				//	tpssm12["REST_TIME"] = 0;           /* 休止时间 */
				//	tpssm12["PRE_PROC_TIME"] = 0;	     /* 前处理时间 */
				//	tpssm12["POST_PROC_TIME"] = 0;      /* 后处理时间 */

				//	//装入
				//	tpssm12["SUB_CHARGE_NO"] = 1;     //1-装入
				//	tpssm12["PROC_TIME"] = tpssmd3["FEED_TIME"];
				//	sqlstr = "tpssm12.Insert()-SUB_CHARGE_NO:1";
				//	tpssm12.Insert();

				//	//吹炼=处理+镇静
				//	tpssm12["SUB_CHARGE_NO"] = 2;     //2-吹炼
				//	tpssm12["PROC_TIME"] = tpssmd5.STD_PROC_TIME;
				//	sqlstr = "tpssm12.Insert()-SUB_CHARGE_NO:2";
				//	tpssm12.Insert();

				//	//出钢
				//	tpssm12["SUB_CHARGE_NO"] = 3;     //3-出钢
				//	tpssm12["PROC_TIME"] = tpssmd3["DRAW_TIME"];
				//	sqlstr = "tpssm12.Insert()-SUB_CHARGE_NO:3";
				//	tpssm12.Insert();

				//}//BOF_PLAN_DIV == "1"


			}//根据 backlog_ea 生成各工序


			tpssm11.TrimOrBlank();
			////Log::Trace("", __FUNCTION__, "INS PONO=[{0}], plan_no=[{1}], plan_edit_flag=[{2}]", tpssm11["PONO"].ToString(), tpssm11["SM_PLAN_NO"].ToString(), tpssm11["PLAN_EDIT_FLAG"].ToString());

			sqlstr = "tpssm11.Insert()";
			tpssm11.Insert();


			//-------------------------------------------
			//对新编入出钢计划的记录，修改状态信息
			if (tpssm10["PONO_STATUS"].ToDecimal() <= 18)//排入出钢计划
			{
				pono_status = 18;	/*排入出钢计划*/
			}
			else
			{
				pono_status = tpssm10["PONO_STATUS"];	/*保留原来的运行状态*/
			}

			//1)修改制造命令炉次表(tpssm10)中状态信息
			//对于重开浇指定的，如有输入时刻，赋值处理
			if (v_restrand_flag == "T" )  //指定开浇时刻
			{
				if ( v_cc_req_time.Trim() == "")
				{
					tpssm10["CC_REQ_TIME_FLAG"] = " ";
					tpssm11["CC_REQ_TIME"] = " ";
				}
				else
				{
					tpssm10["CC_REQ_TIME_FLAG"] = "1";
					tpssm10["CC_REQ_TIME"] = v_cc_req_time;
					tpssm11["CC_REQ_TIME"] = v_cc_req_time;
				}
			}
			////Log::Trace("", __FUNCTION__, "--------tpssm10["CC_REQ_TIME"] =[{0}]", tpssm10["CC_REQ_TIME"].ToString());
			tpssm10["RESTRAND_FLG"] = v_restrand_flag.TrimOrBlank();
			tpssm10["PONO_STATUS"] = pono_status;
			tpssm10["REC_REVISOR"] = CString(s.userid);
			tpssm10["REC_REVISE_TIME"] = dateNow14;
			sqlstr = "tpssm10.Update(PONO_STATUS)";
			tpssm10.Update(
				"PONO_STATUS,"
				"RESTRAND_FLG,"     //连连指定更新
				"CC_REQ_TIME_FLAG,"  //开浇时刻指定
				"CC_REQ_TIME,"
				"REC_REVISE_TIME,REC_REVISOR",
				"PONO, FACTORY_DIV"); //主键

			//2) 修改制造命令炉次表(tpssm01)中状态信息
			tpssm01["PONO_STATUS"] = pono_status;
			tpssm01["PONO"] = tpssm10["PONO"];
			sqlstr = "tpssm01.Update(PONO_STATUS)";
			tpssm01.Update(
				"PONO_STATUS",
				"PONO, FACTORY_DIV"); //主键

			//3) 修改制造命令LOT表(TPSSM04)中状态信息
			tpssm02["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
			tpssm02["LOT_STATUS"] = 4;
			sqlstr = "tpssm02.Update(LOT_STATUS)";
			tpssm02.Update(
				"LOT_STATUS",
				"CAST_LOT_NO"); //主键


#ifdef _SYS_PES    
			/***********2009-12-2 10:11 调用发送炉次状态的函数********start********/
			//////Log::Trace("", __FUNCTION__, "tpssm10.SM_UNIT_NO=[{0}]",(const char*)tpssm10.SM_UNIT_NO);
			//////Log::Trace("", __FUNCTION__, "tpssm10["PONO"] =[{0}]",(const char*)tpssm10["PONO"].ToString());
			//////Log::Trace("", __FUNCTION__, "pono_status =[{0}]", pono_status);

			////blkseq = 1;
			////发送炼钢PONO状态
			//
			//inBlock2.Tables[0].Rows[0]["SM_UNIT_NO"] =  tpssm10.SM_UNIT_NO;
			//inBlock2.Tables[0].Rows[0]["PONO"] =               tpssm10["PONO"];
			//inBlock2.Tables[0].Rows[0]["PONO_STATUS"] =        pono_status;

			//ret = f_ps200021_snd(&inBlock2, bcls_ret,conn);
			//if (ret != 0) //调用不成功
			//{
			//	//////Log::Trace("", __FUNCTION__, "f_ps200021_snd()发送炼钢PONO状态出错:[{0}]",(const char*) s.msg);
			//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			/***********2009-12-2 10:11 调用发送炉次状态的函数********end********/
#endif

		}//读取编入计划的PONO


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

	cmd_tpssmd1_inq.Close();
	cmd_tpssmd4_inq.Close();
	cmd_tpssm12_inq.Close();
	return doFlag;

}

