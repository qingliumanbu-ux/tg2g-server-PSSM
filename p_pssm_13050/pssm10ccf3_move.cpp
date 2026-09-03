/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2015-07-08
Description: 制造命令多条铸顺调整
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 1.将所有编入计划的CC_SEQ重置;
/// 2.取计划下发的最大CC_SEQ_PLAN
/// 3.必须是计划编制状态的Pono才可以调整顺序；将已排入计划的pono从队列（move_list）中移除，发生移除 from_plan = true
/// if(from_plan) 
      begin_CC_SEQ = CC_SEQ_PLAN; 
    else 
	  begin_CC_SEQ = move_list[1].CC_SEQ
/// 4.如果move_type=="auto"： 队列move_list, 找出炉内调宽的炉次(转接炉次)；普通炉次pono[i]尝试调用f_check_can_plan; yes pono1.CC_SEQ = begin_CC_SEQ + i;
/// 5.如果move_type=="hand"： 
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表) TPSSM10                 </para>
/// <para>主调用函数： 前台PSSM10CC画面F3 自动移动调用 F4 手动移动  </para>
/// </summary>
/// <param name="pono">制造命令   </param>
/// <returns>顺序调整后的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm10ccf3_move)
//-EP_SYSTEM_HEAD_END
int f_pssm10ccf3_move(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows, j, k, m;
	int ret = 0;
	int pono_rows;
	int pono_rows_order_first = -1;
	int CAST_DIV_NO = 0;
	bool from_plan = false;
	CString move_type = "";

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息。
	CString sqlstr = "";
	CString plan_date = "";
	CString CAST_LOT_NO = "";
	int CAST_LOT_DIV_NO = 0;
	int v_lack_per = 0;
	int cc_seq = 0;
	CString v_restrand_flg = "";
	CString cast_no_name[100];
	CString castname = "";
	CString pono = "";
	CString factory_div = "";

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm10("TPSSM10");

	EIClass inBlock3; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm01_upd(conn);
	CDbCommand cmd_tpssm01_upd2(conn);
	CDbCommand cmd_tpssm10_upd(conn);

	try
	{

		//调用炼钢履历跟踪
		inBlock3.Tables[0].set_TableName("TRACE");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		//获取前台传入参数
		tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		tpssm10["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().TrimOrBlank();
		move_type = bcls_rec->Tables[0].Rows[0]["MOVE_TYPE"].ToString().TrimOrBlank();
		
		//找到计划下发的CC_SEQ最大值
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			if (strcmp(move_type, "hand") == 0)
			{
				sqlstr = CString(
					" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG "
					" from TPSSM10 A "
					" where PONO_STATUS < '83' AND    CC_SEQ != 999 "
					"   and CC_MACH_NO = @CC_MACH_NO "
					" order by CASE WHEN PONO_STATUS>'16' THEN 0 ELSE 1000 END + CC_SEQ "
					);
			}
			else
			{
				/*sqlstr = CString(
					" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, B.*,C.*,D.*, nvl(E.TM_IDX,A.ST_NO) CAN_MIX_GROUP,nvl(F.TM_IDX,A.ST_NO) WORRY_MIX_GROUP "
					" from TPSSM10 A LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX1, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN1, PONO "
					" from TPSSM03 where PONO in(select PONO from TPSSM01) AND STRAND_NO = '1' group by PONO) B ON A.PONO = B.PONO "
					" LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX2, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN2, PONO "
					" from TPSSM03 where PONO in(select PONO from TPSSM01) AND STRAND_NO = '1' group by PONO) C ON A.PONO = C.PONO "
					" LEFT JOIN(SELECT TA.*,TB.ST_NO FROM TPSSMDC TA left join TQMTS2X TB ON TA.SEQ_NO=TB.SEQ_NO) D ON A.ST_NO=D.ST_NO "
					" LEFT JOIN TPSSM46 E ON E.ST_NO = A.ST_NO "
					" LEFT JOIN TPSSM47 F ON F.TM_IDX_INFO = E.TM_IDX "
					" where PONO_STATUS < '83' AND    CC_SEQ != 999 "
					"   and CC_MACH_NO = @CC_MACH_NO "
					" order by CASE WHEN PONO_STATUS>'16' THEN 0 ELSE 1000 END + CC_SEQ "
					);*/
				sqlstr = CString(
					" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, B.*,C.*,D.*, nvl(E.ST_NO_TO,A.ST_NO) CAN_MIX_GROUP,nvl(F.IDX_NO,A.ST_NO) WORRY_MIX_GROUP "
					" from TPSSM10 A LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX1, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN1, PONO "
					" from TPSSM03 where PONO in(select PONO from TPSSM10) AND STRAND_NO = '1' group by PONO) B ON A.PONO = B.PONO "
					" LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX2, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN2, PONO "
					" from TPSSM03 where PONO in(select PONO from TPSSM10) AND STRAND_NO = '2' group by PONO) C ON A.PONO = C.PONO "
					" LEFT JOIN(SELECT TA.*,TB.ST_NO FROM TPSSMDC TA left join TQMTS2X TB ON TA.SEQ_NO=TB.SEQ_NO) D ON A.ST_NO=D.ST_NO "
					" LEFT JOIN TQMTS13 E ON E.ST_NO_FROM = A.ST_NO "
					" LEFT JOIN TQMTS14 F ON F.IDX_NO_01 = E.ST_NO_TO "
					" where PONO_STATUS < '83' AND    CC_SEQ != 999 "
					"   and CC_MACH_NO = @CC_MACH_NO "
					" order by CASE WHEN PONO_STATUS>'16' THEN 0 ELSE 1000 END + CC_SEQ "
					);
			}
			break;
		}

		cmd_tpssm01_inq.SetCommandText(sqlstr);
		//cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
		cmd_tpssm01_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_tpssm01_inq.Close();

		pono_rows = bcls_ret->Tables[0].Rows.get_Count(); //待排序的行
		Log::Trace("", __FUNCTION__, " pono_rows = [{0}]", pono_rows);

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Trace("", __FUNCTION__, " rows = [{0}]", rows);
		if (rows <= 0) {
			throw CApplicationException(-1, "排序行数0", log.Location);
		}
		//0.传入行的信息同步表最新
		Log::Trace("", __FUNCTION__, " 传入行的信息同步表最新");
		if (!bcls_rec->Tables[0].Columns.Contains("PONO_STATUS"))
			bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "PONO_STATUS");
		if (!bcls_rec->Tables[0].Columns.Contains("RUN_STATUS"))
			bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "RUN_STATUS");
		if (!bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "ST_NO");
		if (!bcls_rec->Tables[0].Columns.Contains("TD_CHG_FLG"))
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "TD_CHG_FLG");

		if (!bcls_rec->Tables[0].Columns.Contains("RESTRAND_FLG"))
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "RESTRAND_FLG");

		if (strcmp(move_type, "hand") != 0)
		{
			if (!bcls_rec->Tables[0].Columns.Contains("SLAB_WIDTH_MAX1"))
				bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_WIDTH_MAX1");
			if (!bcls_rec->Tables[0].Columns.Contains("SLAB_WIDTH_MIN1"))
				bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_WIDTH_MIN1");
			if (!bcls_rec->Tables[0].Columns.Contains("SLAB_WIDTH_MAX2"))
				bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_WIDTH_MAX2");
			if (!bcls_rec->Tables[0].Columns.Contains("SLAB_WIDTH_MIN2"))
				bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "SLAB_WIDTH_MIN2");
			//排序规则数据
			if (!bcls_rec->Tables[0].Columns.Contains("STEEL_TYPE_CNAME")) //规则大类
				bcls_rec->Tables[0].Columns.Add(DT_STRING, "STEEL_TYPE_CNAME");
			if (!bcls_rec->Tables[0].Columns.Contains("TD_STATUS")) //单中包
				bcls_rec->Tables[0].Columns.Add(DT_STRING, "TD_STATUS");
			if (!bcls_rec->Tables[0].Columns.Contains("REQ_TD_POS")) //中包位置
				bcls_rec->Tables[0].Columns.Add(DT_STRING, "REQ_TD_POS");
			if (!bcls_rec->Tables[0].Columns.Contains("TD_CONNECT_NUM")) //最大中包炉数
				bcls_rec->Tables[0].Columns.Add(DT_DECIMAL, "TD_CONNECT_NUM");
			if (!bcls_rec->Tables[0].Columns.Contains("ADJUST_WIDTH_PATTERN")) //限制调宽
				bcls_rec->Tables[0].Columns.Add(DT_STRING, "ADJUST_WIDTH_PATTERN");

			if (!bcls_rec->Tables[0].Columns.Contains("CAN_MIX_GROUP")) //可以混浇
				bcls_rec->Tables[0].Columns.Add(DT_STRING, "CAN_MIX_GROUP");
			if (!bcls_rec->Tables[0].Columns.Contains("WORRY_MIX_GROUP")) //谨慎混浇
				bcls_rec->Tables[0].Columns.Add(DT_STRING, "WORRY_MIX_GROUP");
		}
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j<pono_rows; j++)
			{
				if (strcmp(bcls_rec->Tables[0].Rows[i]["PONO"].ToString(), bcls_ret->Tables[0].Rows[j]["PONO"].ToString()) == 0)
				{
					bcls_rec->Tables[0].Rows[i]["PONO_STATUS"] = bcls_ret->Tables[0].Rows[j]["PONO_STATUS"];
					//bcls_rec->Tables[0].Rows[i]["RUN_STATUS"] = bcls_ret->Tables[0].Rows[j]["RUN_STATUS"];
					bcls_rec->Tables[0].Rows[i]["ST_NO"] = bcls_ret->Tables[0].Rows[j]["ST_NO"];
					bcls_rec->Tables[0].Rows[i]["TD_CHG_FLG"] = bcls_ret->Tables[0].Rows[j]["TD_CHG_FLG"];
					bcls_rec->Tables[0].Rows[i]["RESTRAND_FLG"] = bcls_ret->Tables[0].Rows[j]["RESTRAND_FLG"];
					if (strcmp(move_type, "hand") != 0)
					{
						bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX1"] = bcls_ret->Tables[0].Rows[j]["SLAB_WIDTH_MAX1"];
						bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN1"] = bcls_ret->Tables[0].Rows[j]["SLAB_WIDTH_MIN1"];
						bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX2"] = bcls_ret->Tables[0].Rows[j]["SLAB_WIDTH_MAX2"];
						bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN2"] = bcls_ret->Tables[0].Rows[j]["SLAB_WIDTH_MIN2"];

						bcls_rec->Tables[0].Rows[i]["STEEL_TYPE_CNAME"] = bcls_ret->Tables[0].Rows[j]["STEEL_TYPE_CNAME"];
						bcls_rec->Tables[0].Rows[i]["TD_STATUS"] = bcls_ret->Tables[0].Rows[j]["TD_STATUS"];
						bcls_rec->Tables[0].Rows[i]["REQ_TD_POS"] = bcls_ret->Tables[0].Rows[j]["REQ_TD_POS"];
						bcls_rec->Tables[0].Rows[i]["TD_CONNECT_NUM"] = bcls_ret->Tables[0].Rows[j]["TD_CONNECT_NUM"];
						bcls_rec->Tables[0].Rows[i]["ADJUST_WIDTH_PATTERN"] = bcls_ret->Tables[0].Rows[j]["ADJUST_WIDTH_PATTERN"];

						bcls_rec->Tables[0].Rows[i]["CAN_MIX_GROUP"] = bcls_ret->Tables[0].Rows[j]["CAN_MIX_GROUP"];
						bcls_rec->Tables[0].Rows[i]["WORRY_MIX_GROUP"] = bcls_ret->Tables[0].Rows[j]["WORRY_MIX_GROUP"];

						Log::Info("", __FUNCTION__, "PONO=[{0}]PONO_STATUS[{1}]", bcls_rec->Tables[0].Rows[i]["PONO"].ToString(), bcls_rec->Tables[0].Rows[i]["PONO_STATUS"].ToDecimal());

					}
					break;
				}
			}
		}
		//1.第一行基准行
		Log::Trace("", __FUNCTION__, " 第一行基准行");
		tpssm10["PONO_STATUS"] = bcls_rec->Tables[0].Rows[0]["PONO_STATUS"].ToDecimal();
		if (tpssm10["PONO_STATUS"].ToDecimal() > 80){
			from_plan = true;
		}
		//2.传入行中删除已经开浇的 -- 排入计划的依然可以调整
		Log::Trace("", __FUNCTION__, " 传入行中删除开浇的");
		for (i = rows - 1; i >= 0; i--)
		{
			if (bcls_rec->Tables[0].Rows[i]["RUN_STATUS"].ToDecimal() > 80)
			{
				bcls_rec->Tables[0].Rows.Remove(i);
			}
		}
		rows = bcls_rec->Tables[0].Rows.get_Count();
		if (rows <= 0) {
			Log::Trace("", __FUNCTION__, " 需排序行数0 ");
			throw CApplicationException(-1, "需排序行数0", log.Location);
		}

		//3.待排序的行已排入计划的进行CC_SEQ整理
		Log::Trace("", __FUNCTION__, " 待排序的行已排入计划的进行CC_SEQ整理");
		cc_seq = 1; CAST_DIV_NO = 1;
		for (i = 0; i < pono_rows; i++)
		{
			if (strcmp(bcls_ret->Tables[0].Rows[i]["PONO_STATUS"].ToString(), "80") <= 0)
			{
				//cc_seq = tpssm01["CC_SEQ"].ToDecimal().ToInt32();
				Log::Trace("", __FUNCTION__, " cc_seq = [{0}]", cc_seq);
				if (from_plan)
				{
					break;
				}
				//传入pono中包含的continue
				for (j = 0; j < rows; j++)
				{
					if (strcmp(bcls_rec->Tables[0].Rows[j]["PONO"].ToString(), bcls_ret->Tables[0].Rows[i]["PONO"].ToString()) == 0)
					{
						bcls_ret->Tables[0].Rows[i]["ORDER_CC_SEQ_FLAG"] = "1";
						break;
					}
				}
				if (j == 0)
					break;
				else if (j < rows)
					continue;
			}
						
			bcls_ret->Tables[0].Rows[i]["ORDER_CC_SEQ_FLAG"] = "1";
			tpssm10["CC_SEQ"] = bcls_ret->Tables[0].Rows[i]["CC_SEQ"].ToDecimal().ToInt32();
			if (tpssm10["CC_SEQ"].ToDecimal().ToInt32() != cc_seq)
			{
				tpssm10["CC_SEQ"] = cc_seq;
				tpssm10["PONO"] = bcls_ret->Tables[0].Rows[i]["PONO"].ToString();
				tpssm10.Update("CC_SEQ", "PONO");
			}
			if (strcmp(move_type, "hand") != 0 && pono_rows_order_first >= 0)
			{
				if (strcmp(bcls_ret->Tables[0].Rows[i]["ST_NO"].ToString(), bcls_ret->Tables[0].Rows[pono_rows_order_first]["ST_NO"].ToString()) != 0
					&& strcmp(bcls_ret->Tables[0].Rows[i]["REQ_TD_POS"].ToString(), "3") == 0)
					bcls_ret->Tables[0].Rows[i]["REQ_TD_POS"] = "2";
			}
			pono_rows_order_first = i;
			cc_seq++;
			if (strcmp(bcls_ret->Tables[0].Rows[i]["RESTRAND_FLG"].ToString(), "T") == 0 
				|| strcmp(bcls_ret->Tables[0].Rows[i]["TD_CHG_FLG"].ToString(), "1") == 0)
				CAST_DIV_NO = 1;
			else
				CAST_DIV_NO++;
		}
		
		//4. 对传入的行进行处理
		Log::Trace("", __FUNCTION__, " 对传入的行进行处理");
		if (strcmp(move_type, "hand") == 0)
		{
			for (i = 0; i < rows; i++)
			{
				// 获取前台传入参数
				tpssm10["CC_SEQ"] = cc_seq;
				tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
				//tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();

				//tpssm10.Query("PONO,FACTORY_DIV");
				//tpssm10.TrimOrBlank();

				Log::Info("", __FUNCTION__, "PONO=[{0}]PONO_STATUS[{1}]", tpssm10["PONO"].ToString(), tpssm10["PONO_STATUS"].ToDecimal());

				//更新修改者，修改时间
				tpssm10["REC_REVISOR"] = CString(s.userid); //构造函数初始化
				tpssm10["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

				v_update = "REC_REVISOR,REC_REVISE_TIME,CC_SEQ,RESTRAND_FLG";//修改字段信息。
				v_condi = "FACTORY_DIV,PONO"; //查询条件
				//tpssm01.Update(v_update, v_condi);
				tpssm10.Update(v_update, v_condi);

				//炼钢履历跟踪
				CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
				row3["EVENT_ID"] = "1U"; //铸顺调整
				row3["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				row3["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
				row3["PONO"] = tpssm10["PONO"];

				for (j = 0; j < pono_rows; j++)
				{
					if (strcmp(bcls_rec->Tables[0].Rows[i]["PONO"].ToString(), bcls_ret->Tables[0].Rows[j]["PONO"].ToString()) == 0)
					{
						bcls_ret->Tables[0].Rows[j]["ORDER_CC_SEQ_FLAG"] = "1";
						break;
					}
				}
				cc_seq++;
			}
		}
		else if (strcmp(move_type, "auto") == 0)
		{
			/**
			*自动排序：300.排序规则(中包位置	最大中包炉数)
			            99.同出钢记号，同规格；95炉内调宽同规格
			            91.同出钢记号，不同规格，判断是否允许调宽；
						89.单中包且规则大类相同，同规格，判读是否允许混浇；
						81.单中包且规则大类相同，不同规格，判读是否允许混浇、调宽；
			            19.不同出钢记号，同规格，判读是否允许混浇；
					    11.不同出钢记号，不同规格,判读是否允许混浇、调宽。
						中包位置要求 "2尾"或"3首或尾"的 -10；
						当前出钢记号与上一炉次出钢记号不同的，中包位置要求3首或尾 -> 2尾
				不满足：寻找 中包位置 1首的 设置快换
			*   规则大类	是否单中包	混浇	限制调宽	中包位置	最大中包炉数
			*/
			while (rows > 0)
			{
				Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}]", rows);
				//寻找到可以排入的行
				int max_score = 0, max_score_row = -1;
				if (pono_rows_order_first < 0)
				{
					max_score_row = 0;
				}					
				else
				{
					CDataRow &row_pre_pono = bcls_ret->Tables[0].Rows[pono_rows_order_first];
					Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}]，row_pre_pono[{1}]", rows, row_pre_pono["PONO"].ToString());
					for (i = 0; i < rows; i++)
					{
						int row_score = 0;
						//a.同出钢记号，同规格
						Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}] for begin", rows);
						if (strcmp(bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString(), row_pre_pono["ST_NO"].ToString()) == 0)
						{
							Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}] 同出钢记号，同规格", rows);
							row_score = 290;
						}
						else
						{
							//判断是否允许混浇
							if (strcmp(bcls_rec->Tables[0].Rows[i]["CAN_MIX_GROUP"].ToString(), row_pre_pono["CAN_MIX_GROUP"].ToString()) == 0)
							{
								Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}] 允许混浇", rows);
								row_score += 210;
							}
							else if (strcmp(bcls_rec->Tables[0].Rows[i]["WORRY_MIX_GROUP"].ToString(), row_pre_pono["WORRY_MIX_GROUP"].ToString()) == 0)
							{
								Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}] 谨慎混浇", rows);
								row_score += 110;
							}

						}

						Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}] STEEL_TYPE_CNAME[{1}]", rows, bcls_rec->Tables[0].Rows[i]["STEEL_TYPE_CNAME"].ToString());
						if (row_score >= 100)
						{
							//b.单中包且规则大类相同
							//b1 89.单中包且规则大类相同，同规格
							//b2 81.单中包且规则大类相同，不同规格，判读是否允许调宽；
							if (row_score < 290)
							{
								if (strcmp(bcls_rec->Tables[0].Rows[i]["STEEL_TYPE_CNAME"].ToString(), row_pre_pono["STEEL_TYPE_CNAME"].ToString()) == 0
									&& strcmp(bcls_rec->Tables[0].Rows[i]["TD_STATUS"].ToString(), "1") == 0)
								{
									row_score += 80;
								}
							}

							//c1 19.不同出钢记号，同规格
							//c2 11.不同出钢记号，不同规格, 判读是否允许混浇、调宽。
							//a1.同出钢记号，同规格
							if (bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX1"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MAX1"].ToDecimal()
								&& bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN1"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MIN1"].ToDecimal()
								&& bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX2"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MAX2"].ToDecimal()
								&& bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN2"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MIN2"].ToDecimal())
							{
								row_score += 9;
							}
							//a2.同出钢记号，同规格2
							else if ((bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX1"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MAX1"].ToDecimal()
								|| bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX1"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MIN1"].ToDecimal()
								|| bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN1"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MAX1"].ToDecimal()
								|| bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN1"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MIN1"].ToDecimal())
								&& (bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX2"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MAX2"].ToDecimal()
								|| bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MAX2"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MIN2"].ToDecimal()
								|| bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN2"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MAX2"].ToDecimal()
								|| bcls_rec->Tables[0].Rows[i]["SLAB_WIDTH_MIN2"].ToDecimal() == row_pre_pono["SLAB_WIDTH_MIN2"].ToDecimal()))
							{
								row_score += 5;
							}
							//a3.同出钢记号，不同规格，判断是否允许调宽
							else if (strcmp(bcls_rec->Tables[0].Rows[i]["ADJUST_WIDTH_PATTERN"].ToString(), " ") != 0 || strcmp(bcls_rec->Tables[0].Rows[i]["ADJUST_WIDTH_PATTERN"].ToString(), "1") != 0)
							{
								row_score += 1;
							}
						}

						Log::Trace("", __FUNCTION__, " 迭代：rows = [{0}] 最大中包炉数[{1}]", rows, bcls_rec->Tables[0].Rows[i]["TD_CONNECT_NUM"].ToDecimal().ToInt32());
						//最大中包炉数
						if (row_score > 0 && (0 >= bcls_rec->Tables[0].Rows[i]["TD_CONNECT_NUM"].ToDecimal() || CAST_DIV_NO < bcls_rec->Tables[0].Rows[i]["TD_CONNECT_NUM"].ToDecimal()))
							row_score += 100;
						else
							row_score = 0;
						//中包位置
						//0 无要求	1 首	2 尾	3 首或尾	4 “2-5”
						if (row_score > 0 && row_score < 290)
						{
							if (strcmp(bcls_rec->Tables[0].Rows[i]["REQ_TD_POS"].ToString(), "1") == 0 || strcmp(row_pre_pono["REQ_TD_POS"].ToString(), "2") == 0)
							{
								//混浇 当前pono中包位置要 1首，或者上炉次要求2尾
								row_score = 0;
							}
							else if (strcmp(bcls_rec->Tables[0].Rows[i]["REQ_TD_POS"].ToString(), "2") == 0 || strcmp(bcls_rec->Tables[0].Rows[i]["REQ_TD_POS"].ToString(), "3") == 0)
							{
								//混浇 当前pono中包位置要 2尾 往后排
								row_score -= 10;
							}
						}
						

						if (row_score >= 399)
						{
							max_score = row_score, max_score_row = i;
							break;
						}
						else if (row_score > max_score)
						{
							max_score = row_score, max_score_row = i;
						}
					}
					Log::Trace("", __FUNCTION__, " rows = [{0}]找到合适的行：max_score_row[{1}],max_score[{2}]", rows, max_score_row, max_score);
					//没用找到可以接入上一pono的,设置快换
					if (max_score_row < 0 && rows>0)
					{
						//寻找 中包位置 1首的
						max_score_row = 0;
						for (i = 0; i < rows; i++)
						{
							if (strcmp(bcls_rec->Tables[0].Rows[i]["REQ_TD_POS"].ToString(), "1") == 0)
							{
								//如果要求中包位置1
								max_score_row = i;
								break;
							}
						}
						Log::Trace("", __FUNCTION__, " rows = [{0}]没到找到合适的行", rows);
						bcls_rec->Tables[0].Rows[max_score_row]["TD_CHG_FLG"] = 1;
						bcls_rec->Tables[0].Rows[max_score_row]["RESTRAND_FLG"] = "T";
					}//end for
				} //if (row_pre_pono == NULL)
				
				//更新表
				if (max_score_row >= 0)
				{
					Log::Trace("", __FUNCTION__, " rows = [{0}]找到合适的行，update", rows);
					//预排成功
					tpssm10["CC_SEQ"] = cc_seq;
					tpssm10["PONO"] = bcls_rec->Tables[0].Rows[max_score_row]["PONO"].ToString();
					tpssm10["RESTRAND_FLG"] = bcls_rec->Tables[0].Rows[max_score_row]["RESTRAND_FLG"].ToString();
					tpssm10["TD_CHG_FLG"] = bcls_rec->Tables[0].Rows[max_score_row]["TD_CHG_FLG"].ToDecimal();
					//更新修改者，修改时间
					tpssm10["REC_REVISOR"] = CString(s.userid); //构造函数初始化
					tpssm10["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

					v_update = "REC_REVISOR,REC_REVISE_TIME,CC_SEQ,RESTRAND_FLG,TD_CHG_FLG";//修改字段信息。
					v_condi = "FACTORY_DIV,PONO"; //查询条件
					//tpssm01.Update(v_update, v_condi);
					tpssm10.Update(v_update, v_condi);

					//炼钢履历跟踪
					CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
					row3["EVENT_ID"] = "1U"; //铸顺调整
					row3["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
					row3["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
					row3["PONO"] = tpssm10["PONO"];

					Log::Trace("", __FUNCTION__, " rows = [{0}]找到合适的行，update end", rows);
					for (j = 0; j < pono_rows; j++)
					{
						if (strcmp(bcls_rec->Tables[0].Rows[max_score_row]["PONO"].ToString(), bcls_ret->Tables[0].Rows[j]["PONO"].ToString()) == 0)
						{
							bcls_ret->Tables[0].Rows[j]["ORDER_CC_SEQ_FLAG"] = "1";
							//j行 中包位置要求 3首或尾的明确“2尾”处理
							if (pono_rows_order_first >= 0)
							{
								if (strcmp(bcls_ret->Tables[0].Rows[j]["ST_NO"].ToString(), bcls_ret->Tables[0].Rows[pono_rows_order_first]["ST_NO"].ToString()) != 0
									&& strcmp(bcls_ret->Tables[0].Rows[j]["REQ_TD_POS"].ToString(), "3") == 0)
									bcls_ret->Tables[0].Rows[j]["REQ_TD_POS"] = "2";
							}								
							pono_rows_order_first = j;
							break;
						}
					}
					cc_seq++;
					
					if (strcmp(bcls_rec->Tables[0].Rows[max_score_row]["RESTRAND_FLG"].ToString(), "T") == 0
						|| strcmp(bcls_rec->Tables[0].Rows[max_score_row]["TD_CHG_FLG"].ToString(), "1") == 0)
						CAST_DIV_NO = 1;
					else
						CAST_DIV_NO++;
					bcls_rec->Tables[0].Rows.Remove(max_score_row);
					rows--;
					i = 0;
					Log::Trace("", __FUNCTION__, " rows = [{0}] 准备下次循环 end", rows);
				}

			}//end while
			
			Log::Trace("", __FUNCTION__, " 还需排序的行rows = [{0}] auto End", rows);
		}
		
		//5. 对pono_rows中，未排序处理的赋值
		for (i = 0; i < pono_rows; i++)
		{			
			if (strcmp(bcls_ret->Tables[0].Rows[i]["ORDER_CC_SEQ_FLAG"].ToString(), "1") >= 0)
				continue;
			//提升效率：20炉后铸顺跳号不处理
			if (bcls_ret->Tables[0].Rows[i]["CC_SEQ"].ToDecimal() >= cc_seq && cc_seq>20) 
				break;

			bcls_ret->Tables[0].Rows[i]["ORDER_CC_SEQ_FLAG"] = "1";
			if (bcls_ret->Tables[0].Rows[i]["CC_SEQ"].ToDecimal().ToInt32() != cc_seq)
			{
				tpssm10["CC_SEQ"] = cc_seq;
				tpssm10["PONO"] = bcls_ret->Tables[0].Rows[i]["PONO"].ToString();
				tpssm10.Update("CC_SEQ", "PONO");
			}
			cc_seq++;
		}

		//有必要吗？看看日后要不要去掉; noted by YYP 2021/1/14
		sqlstr = " UPDATE TPSSM10 "
			"  SET  CC_SEQ = 0 "
			"  WHERE PONO_STATUS IN ( 83,91 ) ";

		cmd_tpssm10_upd.SetCommandText(sqlstr);
		cmd_tpssm10_upd.ExecuteNonQuery();
		cmd_tpssm10_upd.Close();

		sqlstr = " UPDATE TPSSM01 A "
			"  SET  CC_SEQ = (SELECT CC_SEQ FROM TPSSM10 B WHERE B.PONO=A.PONO) "
			"  WHERE PONO in(SELECT PONO FROM TPSSM10)";

		cmd_tpssm01_upd.SetCommandText(sqlstr);
		cmd_tpssm01_upd.ExecuteNonQuery();
		cmd_tpssm01_upd.Close();

		//-----------------------------------------------
		//炼钢履历跟踪
		//ret = f_pssm99_trace(&inBlock3, bcls_ret, conn);
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
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1);
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	/*cmd_tpssm01_inq.Close();*/

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}

