/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Date:     2015-2-10
Version:  3.1.0
Description: PES系统接受对MMS系统下发的制造命令进行接受
Update：
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

#ifdef _SYS_PES
//int f_cm_200007_snd(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn); //接收应答
#endif

int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表

/*<remark>=========================================================
/// <summary>
/// 获取材料合同信息
/// <para>
///    功能叙述段落
///  注：本函数只在MMS层用，输入/输出参数都以bcls_rec下的[MMSMSM]块传递
/// </para>
/// </summary>
/// <param name="param1">参数1  </param>
/// <param name="param2">参数2  </param>
/// <returns>返回参数：调用是否成功</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE_TELE(cm_002124_rcv)

int f_cm_002124_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int    doFlag = 0;
	int    fetchRowCount = 0;
	int ret = 0;
	int rownum = 0;
	int rownum2 = 0;
	int flag99 = 0;

	CString   datetime = " ";
	int    blkseq = 0;
	EIClass inBlock1;  //调用发送反馈应答的函数
	EIClass inBlock2;  //调用删除棒线轧制计划函数用 f_psbw_roll_del()
	CDecimal    cc_seq = 0;                         /* 连铸顺序号 */
	CDecimal    dummy = 0;
	CDecimal    v_proc_div = 0;   //操作类别：1-新增；3-PONO删除
	CDecimal    wt = 0;
	CDecimal    prod_density = 7.85;
	CDecimal    cast_lot_sum = 0;
	CDecimal   cast_lot_div_num = 0;
	CString    cast_lot_div = "";
	CString   v_cast_lot_no = "";
	CString   v_table_name = "";
	CString   pono = "";
	CString   factory_div = "";
	CString   long_flag = "";
	CString   updatetpssm10_flag = "0";
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CModel tpssmdh("TPSSMDH");
	CModel tpssmdj("TPSSMDJ");
	CModel tpssmd7("TPSSMD7");
	CModel tpssm99("TPSSM99");//履历
	CModel tqmts0x("TQMTS0X");
	CDbCommand cmd_tpssm01_inq1(conn);
	CDbCommand cmd_tpssm03_sql(conn);
	CDbCommand cmd_tpssm03_upd(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_tpssmd7_inq(conn);
	CString sqlstr;

	EIClass in_pssm99trace;//调用履历函数
	EIClass in_pssm99trace2;//调用履历函数

	try
	{

		//计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);

		in_pssm99trace2.Tables[0].set_TableName("TRACE");
		in_pssm99trace2.Tables[0].Clone(tpssm99);
		//取系统日期、时间
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		tpssm01["REC_CREATE_TIME"] = datetime;

		//责任者
		tpssm01["REC_CREATOR"] = "XCOM";
		tpssm02["REC_CREATOR"] = "XCOM";
		tpssm03["REC_CREATOR"] = "XCOM";
		tpssm02["REC_CREATE_TIME"] = tpssm01["REC_CREATE_TIME"];
		tpssm03["REC_CREATE_TIME"] = tpssm01["REC_CREATE_TIME"];

		Log::Trace("", __FUNCTION__, "s.userid={0}", s.userid);
		Log::Trace("", __FUNCTION__, "s.username={0}", s.username);
		Log::Trace("", __FUNCTION__, "s.formname={0}", s.formname);

		//1)设置调用应答函数的输入参数:
		//blkseq = 1;  //第一块
		//inBlock1.Tables[blkseq - 1].set_TableName("X200007");
		//inBlock1.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");
		//inBlock1.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");
		//inBlock1.Tables[blkseq - 1].Columns.Add(DT_STRING, "ACK_CODE");
		//inBlock1.Tables[blkseq - 1].Rows.Add();  //单记录应答
		//--------------------------------------------------------
		//获取电文内容，单记录（每PONO）
		//tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		//tpssm01.Print();
		//tpssm02.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		rownum = bcls_rec->Tables["tpssm03"].Rows.get_Count();
		rownum2 = bcls_rec->Tables["tpssm03_S"].Rows.get_Count();
		if (rownum > 0)
		{
			v_proc_div = bcls_rec->Tables["tpssm03"].Rows[0]["proc_div"];
			pono = bcls_rec->Tables["tpssm03"].Rows[0]["pre_heat_no"].ToString();
		}
		else if (rownum2 > 0 && rownum == 0)
		{
			v_proc_div = bcls_rec->Tables["tpssm03_s"].Rows[0]["proc_div"];
			pono = bcls_rec->Tables["tpssm03_s"].Rows[0]["pre_heat_no"].ToString();
		}
		factory_div = "LG1";
		Log::Trace("", __FUNCTION__, "proc_div=[{0}]", v_proc_div.ToInt32());

		if (v_proc_div == 1)  //1-新增
		{
			rownum = bcls_rec->Tables["tpssm03"].Rows.get_Count();
			rownum2 = bcls_rec->Tables["tpssm03_S"].Rows.get_Count();

			if (rownum > 0)
			{
				for (int i = 0; i < rownum; i++)
				{
					//------------------------------
					//炉次命令赋值
					tpssm01["FACTORY_DIV"] = "LG1";
					tpssm01["PONO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_heat_no"].ToString();
					tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					tpssm10["PONO"] = tpssm01["PONO"];
					//sqlstr = "tpssm01.Delete()";
					//tpssm01.Delete("FACTORY_DIV,PONO");
					if (tpssm01.QueryCount("FACTORY_DIV,PONO") == 0)
					{
						tpssm01["PONO_STATUS"] = 16;
						tpssm01["CC_MACH_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_cc_mach_no"].ToString();
						tpssm01["PLAN_DATE"] = bcls_rec->Tables["tpssm03"].Rows[i]["plan_cast_time"].ToString().Substring(0, 8);
						tpssm01["CC_REQ_TIME"] = bcls_rec->Tables["tpssm03"].Rows[i]["plan_cast_time"].ToString();
						tpssm01["CC_SEQ"] = 0;
						tpssm01["ST_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["steel_grade"].ToString();
						tpssm01["CAST_LOT_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_cast_no"].ToString();
						//tpssm01["CAST_LOT_SUM"] = 
						//tpssm01["CAST_LOT_DIV_NO"] =
						//tpssm01["PLAN_TAP_WT"] =
						tpssm01["SLAB_DEST"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_dest"].ToString();
						tpssm01["HOT_CHARGE_FLAG"] = bcls_rec->Tables["tpssm03"].Rows[i]["hot_charge_flag"].ToString();
						tpssm01["HOT_SEND_DIV"] = bcls_rec->Tables["tpssm03"].Rows[i]["hot_send_div"].ToString();

						tpssmdh.Reset();
						tpssmdj.Reset();
						tpssmd7.Reset();
						tpssmdh["ST_NO"] = tpssm01["ST_NO"];
						tpssmdh["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
						tpssmdh.Query("ST_NO,FACTORY_DIV");
						tpssmdj["ROUTEBAGKEY"] = tpssmdh["ROUTEBAGKEY"];
						tpssmdj["PLANTSECTIONTYPE"] = 1;
						if (tpssmdj.QueryCount("ROUTEBAGKEY,PLANTSECTIONTYPE") == 1)
						{
							tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
							flag99 = 0;
						}
						else
						{
							Log::Trace("", __FUNCTION__, "取不到默认路径={0}", tpssmdj["ROUTEBAGKEY"].ToString());
							if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'S')
							{
								tpssmdj["ROUTEBAGKEY"] = "S_DEFAULT";
							}
							else if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'C')
							{
								tpssmdj["ROUTEBAGKEY"] = "C_DEFAULT";
							}
							//Log::Trace("", __FUNCTION__, "取不到默认路径={0}",);
							tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
							flag99 = 1;
							tpssmdj["ROUTEBAGKEY"] = tpssmdh["ROUTEBAGKEY"];
						}
						tpssmd7["ROUTELIST"] = tpssmdj["ROUTELIST"];

						tpssm01["ROUTEBAGKEY"] = tpssmdj["ROUTEBAGKEY"];
						tpssm01["ROUTELIST"] = tpssmdj["ROUTELIST"];

						if (bcls_rec->Tables.Contains("tpssm01"))
						{
							if (bcls_rec->Tables["tpssm01"].Rows[0]["backlog_ea"].ToString().Trim() != "")
							{
								tpssmdj["ROUTEBAGKEY"] = bcls_rec->Tables["tpssm01"].Rows[0]["backlog_ea"].ToString();
								tpssmdj["PLANTSECTIONTYPE"] = 1;
								if (tpssmdj.QueryCount("ROUTEBAGKEY,PLANTSECTIONTYPE") == 1)
								{
									tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
									flag99 = 0;
								}
								else
								{
									if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'S')
									{
										tpssmdj["ROUTEBAGKEY"] = "S_DEFAULT";
									}
									else if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'C')
									{
										tpssmdj["ROUTEBAGKEY"] = "C_DEFAULT";
									}
									tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
									flag99 = 1;
									tpssmdj["ROUTEBAGKEY"] = bcls_rec->Tables["tpssm01"].Rows[0]["backlog_ea"].ToString();
								}
								tpssmd7["ROUTELIST"] = tpssmdj["ROUTELIST"];

								tpssm01["ROUTEBAGKEY"] = tpssmdj["ROUTEBAGKEY"];
								tpssm01["ROUTELIST"] = tpssmdj["ROUTELIST"];
							}
						}

						tpssm01["CC_TYPE"] = "1";
						tpssm01["SMELT_MODE"] = 0;
						tpssm01["SMELT_DIV"] = "B";
						tpssm01["REFINE_DIV"] = " ";
						tpssm01["BACKLOG_EA"] = " ";
						tpssm01["PLAN_TAP_WT"] = 0;

						cast_lot_div = bcls_rec->Tables["tpssm03"].Rows[i]["pre_heat_no"].ToString().Substring(bcls_rec->Tables["tpssm03"].Rows[i]["pre_heat_no"].ToString().GetLength() - 1, 1);
						if (cast_lot_div == "1") cast_lot_div_num = 1;
						else if (cast_lot_div == "2") cast_lot_div_num = 2;
						else if (cast_lot_div == "3") cast_lot_div_num = 3;
						else if (cast_lot_div == "4") cast_lot_div_num = 4;
						else if (cast_lot_div == "5") cast_lot_div_num = 5;
						else if (cast_lot_div == "6") cast_lot_div_num = 6;
						else if (cast_lot_div == "7") cast_lot_div_num = 7;
						else if (cast_lot_div == "8") cast_lot_div_num = 8;
						else if (cast_lot_div == "9") cast_lot_div_num = 9;
						else if (cast_lot_div == "A") cast_lot_div_num = 10;
						else if (cast_lot_div == "B") cast_lot_div_num = 11;
						else if (cast_lot_div == "C") cast_lot_div_num = 12;
						else if (cast_lot_div == "D") cast_lot_div_num = 13;
						else if (cast_lot_div == "E") cast_lot_div_num = 14;
						else if (cast_lot_div == "F") cast_lot_div_num = 15;
						else if (cast_lot_div == "G") cast_lot_div_num = 16;
						else if (cast_lot_div == "H") cast_lot_div_num = 17;
						else if (cast_lot_div == "I") cast_lot_div_num = 18;
						else if (cast_lot_div == "J") cast_lot_div_num = 19;
						else if (cast_lot_div == "K") cast_lot_div_num = 20;
						else if (cast_lot_div == "L") cast_lot_div_num = 21;
						else if (cast_lot_div == "M") cast_lot_div_num = 22;
						else if (cast_lot_div == "N") cast_lot_div_num = 23;
						else if (cast_lot_div == "O") cast_lot_div_num = 24;
						else if (cast_lot_div == "P") cast_lot_div_num = 25;
						else if (cast_lot_div == "Q") cast_lot_div_num = 26;
						else if (cast_lot_div == "R") cast_lot_div_num = 27;
						else if (cast_lot_div == "S") cast_lot_div_num = 28;
						else if (cast_lot_div == "T") cast_lot_div_num = 29;
						else if (cast_lot_div == "U") cast_lot_div_num = 30;
						else if (cast_lot_div == "V") cast_lot_div_num = 31;
						else if (cast_lot_div == "W") cast_lot_div_num = 32;
						else if (cast_lot_div == "X") cast_lot_div_num = 33;
						else if (cast_lot_div == "Y") cast_lot_div_num = 34;
						else if (cast_lot_div == "Z") cast_lot_div_num = 35;

						tpssm01["CAST_LOT_DIV_NO"] = cast_lot_div_num;
						if (cast_lot_div_num != 1)
						{
							tpssm01["RESTRAND_FLG"] = " ";
						}
						else
						{
							tpssm01["RESTRAND_FLG"] = "T";
						}

						sqlstr = " select DEV_TECH_CODE from tpssmd7 where FACTORY_DIV = @FACTORY_DIV and area_id = 4 and ROUTELIST = @ROUTELIST order by charge_no ";
						cmd_tpssmd7_inq.SetCommandText(sqlstr);
						cmd_tpssmd7_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssmd7_inq.Parameters.Set("ROUTELIST", tpssmd7["ROUTELIST"].ToString());
						cmd_tpssmd7_inq.ExecuteReader();
						while (cmd_tpssmd7_inq.Read())
						{
							tpssm01["REFINE_DIV"] = tpssm01["REFINE_DIV"].ToString() + cmd_tpssmd7_inq.GetString(1);
						}

						sqlstr = " select DEV_TECH_CODE from tpssmd7 where FACTORY_DIV = @FACTORY_DIV and ROUTELIST = @ROUTELIST order by charge_no ";
						cmd_tpssmd7_inq.SetCommandText(sqlstr);
						cmd_tpssmd7_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssmd7_inq.Parameters.Set("ROUTELIST", tpssmd7["ROUTELIST"].ToString());
						cmd_tpssmd7_inq.ExecuteReader();
						while (cmd_tpssmd7_inq.Read())
						{
							tpssm01["BACKLOG_EA"] = tpssm01["BACKLOG_EA"].ToString() + cmd_tpssmd7_inq.GetString(1);
						}
						cmd_tpssmd7_inq.Close();

						tpssm01.TrimOrBlank();
						//CFormattable arguments[] = { tpssm01["PONO"].ToString() };
						//CMessageFormat::Format(s.msg, _RES("该PONO已经存在")/*该PONO已经存在。*/, arguments, 1);
						//throw CApplicationException(-1, s.msg, log.Location);

						sqlstr = "tpssm01.Insert()";
						tpssm01.Insert();

						sqlstr = " select max(CAST_LOT_DIV_NO) from tpssm01 where FACTORY_DIV = @FACTORY_DIV  and CAST_LOT_NO = @CAST_LOT_NO ";
						cmd_tpssm01_inq.SetCommandText(sqlstr);
						cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
						cmd_tpssm01_inq.ExecuteReader();
						if (cmd_tpssm01_inq.Read())
						{
							tpssm01["CAST_LOT_SUM"] = cmd_tpssm01_inq.GetDecimal(1);
						}
						else
						{
							tpssm01["CAST_LOT_SUM"] = 0;
						}
						cmd_tpssm01_inq.Close();
						tpssm01.Update("CAST_LOT_SUM", "CAST_LOT_NO");

						tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
						tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
						if (tpssm02.QueryCount("FACTORY_DIV,CAST_LOT_NO") == 0)
						{
							//------------------------------
							//LOT信息赋值
							sqlstr = "tpssm02.Insert()";
							tpssm02["LOT_STATUS"] = 3;
							tpssm02["ST_NO"] = tpssm01["ST_NO"];
							tpssm02["SLAB_THICK"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_thick"].ToDecimal();
							tqmts0x.Reset();
							tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
							tqmts0x.Query("ST_NO");
							tpssm02["SG_SIGN"] = tqmts0x["SG_GRADE_1"];//bcls_rec->Tables["tpssm03"].Rows[i]["sg_sign"].ToString();
							tpssm02["BILLET_TYPE"] = "1";

							tpssm02.TrimOrBlank();
							tpssm02.Insert();
						}

						//tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
						tpssm02["CAST_LOT_SUM"] = tpssm01["CAST_LOT_SUM"];
						tpssm02.Update("CAST_LOT_SUM", "CAST_LOT_NO,FACTORY_DIV");


						//------------------------------
						//写入连铸计划表（TPSSM10）
						//取当前出钢计划中最大的浇注顺序号和PONO
						cc_seq = 0;
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:         // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default:  // 所有数据库适用，通用SQL语句。SQL 语句标准化修改:去掉NVL。 xuwen 2013-4-10
							sqlstr = CString(
								" SELECT MAX(cc_seq) FROM TPSSM10 "
								"  WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
								"    AND CC_MACH_NO = @tpssm01.CC_MACH_NO "
								//"    AND PONO_STATUS >= 16 "
								"    AND CC_SEQ     < 900 "   //900以后是挂起的炉次
								);
							break;
						}
						cmd_tpssm10_inq.SetCommandText(sqlstr);
						cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssm10_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
						cmd_tpssm10_inq.ExecuteReader();
						if (cmd_tpssm10_inq.Read())
						{
							cc_seq = cmd_tpssm10_inq.GetDecimal(1);
						}
						else
						{
							cc_seq = 0;
						}
						cmd_tpssm10_inq.Close();
						Log::Trace("", __FUNCTION__, "当前出钢计划中最大的浇注顺序号cc_seq = [{0}]", cc_seq.ToInt32());


						//如果要新增的PONO已存在，那么删除以前的（再排计划时），重新接受.
						//如果要新增的PONO已编入出钢计划，此处的状态校验，在MMS层校验
						//删除该PONO后，不用考虑cc_seq 跳号，只要顺序排列出就行
						tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];  /* 炼钢单元号 -PK*/
						tpssm10["PONO"] = tpssm01["PONO"];
						sqlstr = "tpssm10.Delete()";
						tpssm10.Delete("FACTORY_DIV,PONO");

						//新增计划
						tpssm10.CopyFrom(tpssm01);
						tpssm10["CAST_LOT_NO2"] = tpssm01["CAST_LOT_NO"];
						tpssm10["CAST_LOT_DIV_NO2"] = tpssm01["CAST_LOT_DIV_NO"];
						tpssm10["PONO_STATUS"] = 16;                    /* 制造命令状态 */
						if (cc_seq == 0)
						{
							cc_seq = 1;
						}
						else
						{
							cc_seq = cc_seq + 1;
						}
						tpssm10["CC_SEQ"] = cc_seq;                /* 连铸顺序号 */
						Log::Trace("", __FUNCTION__, "lxx1 = [{0}]", tpssm10["ROUTEBAGKEY"].ToString());
						if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'C')
						{
							tpssm10["C_DIV"] = "2";
						}
						else if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'S')
						{
							tpssm10["C_DIV"] = "1";
						}
						if (tpssm10["C_DIV"].ToString().Trim() == "")
						{
							tqmts0x.Reset();
							tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
							tqmts0x.Query("ST_NO");
							tpssm10["C_DIV"] = tqmts0x["C_DIV"].ToString();
						}

						tpssm10["TD_CHG_FLG"] = 0;                     /* 中间包更换标志 */
						//zxl20160822方坯T默认不要
						tpssm10["BILLET_TYPE"] = "1";

						if (tpssm01["RESTRAND_FLG"].ToString().Trim() == "")//连浇
						{
							tpssm10["CC_PREP_TIME"] = 2;  // 炉间准备时间
						}
						else
						{
							tpssm10["CC_PREP_TIME"] = 100;  // 炉间准备时间
						}

						tpssm10["POUR_TIME"] = 40; //浇注时间
						tpssm10["CC_REQ_TIMEL4"] = bcls_rec->Tables["tpssm03"].Rows[i]["plan_cast_time"].ToString();
						tpssm10["CC_REQ_TIME"] = " ";
						tqmts0x.Reset();
						tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
						tqmts0x.Query("ST_NO");
						tpssm10["SG_SIGN"] = tqmts0x["SG_GRADE_1"];//bcls_rec->Tables["tpssm03"].Rows[i]["sg_sign"].ToString();
						tpssm10.TrimOrBlank();
						sqlstr = "tpssm10.Insert()";
						tpssm10.Insert();

						tpssm10["CAST_LOT_SUM"] = tpssm01["CAST_LOT_SUM"];
						tpssm10["CAST_LOT_SUM2"] = tpssm01["CAST_LOT_SUM"];
						tpssm10.Update("CAST_LOT_SUM,CAST_LOT_SUM2", "CAST_LOT_NO,FACTORY_DIV");

						tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
						tpssm99["PONO"] = tpssm10["PONO"];
						tpssm99["EVENT_ID"] = "A1";
						tpssm99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
						tpssm99["VALID_FLAG"] = "1";

						tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
						if (flag99 == 1)
						{
							tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
							tpssm99["PONO"] = tpssm10["PONO"];
							tpssm99["EVENT_ID"] = "1Z";
							tpssm99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
							tpssm99["VALID_FLAG"] = "1";

							tpssm99.MergeTo(in_pssm99trace2.Tables[0], false);
						}
						//Log::Trace("", __FUNCTION__, "接收MMS下发的预计划，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
						//Log::Trace("", __FUNCTION__, "记录履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
					}
					tpssm01.Query("FACTORY_DIV,PONO");

					wt = wt + bcls_rec->Tables["tpssm03"].Rows[i]["slab_thick"].ToDecimal() * bcls_rec->Tables["tpssm03"].Rows[i]["slab_width"].ToDecimal() * bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_aim"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
					tpssm01["PLAN_TAP_WT"] = wt.Round(3);

					if (bcls_rec->Tables.Contains("tpssm01"))
					{
						if (bcls_rec->Tables["tpssm01"].Rows[0]["plan_tap_wt"].ToDecimal() != 0)
						{
							tpssm01["PLAN_TAP_WT"] = bcls_rec->Tables["tpssm01"].Rows[0]["plan_tap_wt"].ToDecimal();
						}
					}

					tpssm10["PLAN_TAP_WT"] = tpssm01["PLAN_TAP_WT"];
					tpssm01.Update("PLAN_TAP_WT", "FACTORY_DIV,PONO");
					tpssm10.Update("PLAN_TAP_WT", "FACTORY_DIV,PONO");

					//------------------------------
					//板坯命令赋值
					if (i == 0 && rownum2 != 0)
					{
						for (int j = 0; j < rownum2; j++)
						{
							tpssm03["FACTORY_DIV"] = "LG1";
							tpssm03["SLAB_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["pre_slab_no"].ToString();
							tpssm03["CAST_LOT_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["pre_cast_no"].ToString();
							tpssm03["PONO"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["pre_heat_no"].ToString();
							tpssm03["STRAND_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["pre_strand_no"].ToString();
							tpssm03["BILLET_TYPE"] = "1";
							tpssm03["INGOT_CODE"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["ingot_code"].ToString();
							tpssm03["SLAB_NUM"] = 1;
							tpssm03["SLAB_THICK"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_thick"].ToDecimal();
							tpssm03["MATIRAL_CODE"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["matiral_code"].ToString();
							tpssm03["SLAB_WIDTH"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_width"].ToDecimal();
							tpssm03["SLAB_LEN"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_length_aim"].ToDecimal();
							tpssm03["SLAB_MAX_LEN"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_length_max"].ToDecimal();
							tpssm03["SLAB_MIN_LEN"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_length_min"].ToDecimal();
							//tpssm03["SLAB_WT"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_thick"].ToDecimal() * bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_width"].ToDecimal() * bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_length_aim"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
							//tpssm03["SLAB_WT"] = tpssm03["SLAB_WT"].ToDecimal().Round(3);
							tpssm03["SLAB_WT"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_wt"].ToDecimal();
							tpssm03["ORDER_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["order_no"].ToString();
							tpssm03["SLAB_DEST"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_dest"].ToString();
							tpssm03["HOT_SEND_FLAG"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["hot_send_div"].ToString();
							tpssm03["HOT_CHARGE_FLAG"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["hot_charge_flag"].ToString();
							tpssm03["SLAB_SEQ_2"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["pre_strand_seq_no"].ToDecimal();
							tpssm03["SG_SIGN"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["sg_sign"].ToString();
							tpssm03["FACTORY_NEXT"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["plant_next"].ToString();
							tpssm03["APN"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["slab_fin_use"].ToString();
							tpssm03["LSLAB_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[j]["l_slabno"].ToString();
						
							tpssm03["LSLAB_T_B_FLAG"] = "1";
							tpssm03["SLAB_PROD_FLAG"] = "0";


							//删除原铸坯
							sqlstr = "tpssm03.Delete(SLAB_NO)";
							tpssm03.Delete("SLAB_NO");

							//长坯号为空时，板坯号赋给长坯号
							//if (tpssm03["LSLAB_NO"].ToString().Trim() == "")
							//{
							//tpssm03["LSLAB_NO"] = tpssm03["SLAB_NO"];
							//}

							//新增铸坯
							sqlstr = "tpssm03.Insert()";
							tpssm03.TrimOrBlank();
							tpssm03.Insert();

							if (j == 0)
							{
								//更新浇铸计划表
								tpssm10["FACTORY_DIV"] = tpssm03["FACTORY_DIV"];
								tpssm10["PONO"] = tpssm03["PONO"];
								tpssm10["SLAB_THICK"] = tpssm03["SLAB_THICK"];
								tpssm10["SLAB_WIDTH"] = tpssm03["SLAB_WIDTH"];
								tpssm10["SLAB_LEN"] = tpssm03["SLAB_LEN"];
								sqlstr = "tpssm10.Update()";
								tpssm10.Update(
									"SLAB_THICK,"
									"SLAB_WIDTH,"
									"SLAB_LEN",
									"FACTORY_DIV,PONO");

								updatetpssm10_flag = "1";
							}
						}
					}

					long_flag = bcls_rec->Tables["tpssm03"].Rows[i]["long_slab_flag"].ToString();
					if (long_flag == "0")
					{
						tpssm03["FACTORY_DIV"] = "LG1";
						tpssm03["SLAB_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_slab_no"].ToString();
						tpssm03["CAST_LOT_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_cast_no"].ToString();
						tpssm03["PONO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_heat_no"].ToString();
						tpssm03["STRAND_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_strand_no"].ToString();
						tpssm03["BILLET_TYPE"] = "1";
						tpssm03["INGOT_CODE"] = bcls_rec->Tables["tpssm03"].Rows[i]["ingot_code"].ToString();
						tpssm03["SLAB_NUM"] = 1;
						tpssm03["SLAB_THICK"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_thick"].ToDecimal();
						tpssm03["MATIRAL_CODE"] = bcls_rec->Tables["tpssm03"].Rows[i]["matiral_code"].ToString();
						tpssm03["SLAB_WIDTH"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_width"].ToDecimal();
						tpssm03["SLAB_LEN"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_aim"].ToDecimal();
						tpssm03["SLAB_MAX_LEN"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_max"].ToDecimal();
						tpssm03["SLAB_MIN_LEN"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_min"].ToDecimal();
						//tpssm03["SLAB_WT"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_thick"].ToDecimal() * bcls_rec->Tables["tpssm03"].Rows[i]["slab_width"].ToDecimal() * bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_aim"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
						//tpssm03["SLAB_WT"] = tpssm03["SLAB_WT"].ToDecimal().Round(3);
						tpssm03["SLAB_WT"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_wt"].ToDecimal();
						tpssm03["ORDER_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["order_no"].ToString();
						tpssm03["SLAB_DEST"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_dest"].ToString();
						tpssm03["HOT_SEND_FLAG"] = bcls_rec->Tables["tpssm03"].Rows[i]["hot_send_div"].ToString();
						tpssm03["HOT_CHARGE_FLAG"] = bcls_rec->Tables["tpssm03"].Rows[i]["hot_charge_flag"].ToString();
						tpssm03["SLAB_SEQ_2"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_strand_seq_no"].ToDecimal();
						tpssm03["SG_SIGN"] = bcls_rec->Tables["tpssm03"].Rows[i]["sg_sign"].ToString();
						tpssm03["WORK_SPECIAL_REQ"] = bcls_rec->Tables["tpssm03"].Rows[i]["WORK_SPECIAL_REQ"].ToString();
						tpssm03["FACTORY_NEXT"] = bcls_rec->Tables["tpssm03"].Rows[i]["plant_next"].ToString();
						tpssm03["APN"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_fin_use"].ToString();
						tpssm03["LSLAB_T_B_FLAG"] = "0";
						tpssm03["LSLAB_NO"] = tpssm03["SLAB_NO"];
						tpssm03["SLAB_PROD_FLAG"] = "0";

						//非长坯不需要此数据
						tpssm03["LSLAB_NO_LENGTH"] = 0;
						tpssm03["LSLAB_NO_LENGTH_MIN"] = 0;
						tpssm03["LSLAB_NO_LENGTH_MAX"] = 0;
						tpssm03["LSLAB_NO_WT"] = 0;

						tpssm03["WHOLE_BACKLOG"] = bcls_rec->Tables["tpssm03"].Rows[i]["whole_backlog"].ToString();
						tpssm03["WHOLE_BACKLOG_CODE"] = bcls_rec->Tables["tpssm03"].Rows[i]["whole_backlog_code"].ToString();
						tpssm03["WHOLE_BACKLOG_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["whole_backlog_no"].ToDecimal();
						tpssm03["WHOLE_BACKLOG_SEQ"] = bcls_rec->Tables["tpssm03"].Rows[i]["whole_backlog_seq"].ToDecimal();

						//删除原铸坯
						sqlstr = "tpssm03.Delete(SLAB_NO)";
						tpssm03.Delete("SLAB_NO");

						//长坯号为空时，板坯号赋给长坯号
						//if (tpssm03["LSLAB_NO"].ToString().Trim() == "")
						//{
						//tpssm03["LSLAB_NO"] = tpssm03["SLAB_NO"];
						//}

						//新增铸坯
						sqlstr = "tpssm03.Insert()";
						tpssm03.TrimOrBlank();
						tpssm03.Insert();

						if (updatetpssm10_flag == "0")
						{
							//更新浇铸计划表
							tpssm10["FACTORY_DIV"] = tpssm03["FACTORY_DIV"];
							tpssm10["PONO"] = tpssm03["PONO"];
							tpssm10["SLAB_THICK"] = tpssm03["SLAB_THICK"];
							tpssm10["SLAB_WIDTH"] = tpssm03["SLAB_WIDTH"];
							tpssm10["SLAB_LEN"] = tpssm03["SLAB_LEN"];
							sqlstr = "tpssm10.Update()";
							tpssm10.Update(
								"SLAB_THICK,"
								"SLAB_WIDTH,"
								"SLAB_LEN",
								"FACTORY_DIV,PONO");

							updatetpssm10_flag = "1";
						}
					}
					else if (long_flag == "1")
					{
						tpssm03["LSLAB_NO"] = bcls_rec->Tables["tpssm03"].Rows[i]["pre_slab_no"].ToString();
						tpssm03["LSLAB_NO_LENGTH"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_aim"].ToDecimal();
						tpssm03["LSLAB_NO_LENGTH_MIN"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_min"].ToDecimal();
						tpssm03["LSLAB_NO_LENGTH_MAX"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_length_max"].ToDecimal();
						tpssm03["LSLAB_NO_WT"] = bcls_rec->Tables["tpssm03"].Rows[i]["slab_wt"].ToDecimal();
						//tpssm03.Update("LSLAB_NO_LENGTH,LSLAB_NO_LENGTH_MIN,LSLAB_NO_LENGTH_MAX", "LSLAB_NO");
						tpssm03.Update("LSLAB_NO_LENGTH,LSLAB_NO_LENGTH_MIN,LSLAB_NO_LENGTH_MAX,LSLAB_NO_WT", "LSLAB_NO");
					}
#ifdef _SYS_PES
					//Log::Trace("", __FUNCTION__, "发送应答电文赋值");

					//inBlock1.Tables["X200007"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					//inBlock1.Tables["X200007"].Rows[0]["PONO"] = tpssm01["PONO"];
					//inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "2";  // 2计划接收正常   
#endif      	
				}
			}
			else if (rownum2 > 0 && rownum == 0)
			{
				Log::Trace("", __FUNCTION__, "异常");
				for (int i = 0; i < rownum2; i++)
				{
					//------------------------------
					//炉次命令赋值
					tpssm01["FACTORY_DIV"] = "LG1";
					tpssm01["PONO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_heat_no"].ToString();
					tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					tpssm10["PONO"] = tpssm01["PONO"];
					//sqlstr = "tpssm01.Delete()";
					//tpssm01.Delete("FACTORY_DIV,PONO");
					if (tpssm01.QueryCount("FACTORY_DIV,PONO") == 0)
					{
						tpssm01["PONO_STATUS"] = 16;
						tpssm01["CC_MACH_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_cc_mach_no"].ToString();
						tpssm01["PLAN_DATE"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["plan_cast_time"].ToString().Substring(0, 8);
						tpssm01["CC_REQ_TIME"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["plan_cast_time"].ToString();
						tpssm01["CC_SEQ"] = 0;
						tpssm01["ST_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["steel_grade"].ToString();
						tpssm01["CAST_LOT_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_cast_no"].ToString();
						//tpssm01["CAST_LOT_SUM"] = 
						//tpssm01["CAST_LOT_DIV_NO"] =
						//tpssm01["PLAN_TAP_WT"] =
						tpssm01["SLAB_DEST"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_dest"].ToString();
						tpssm01["HOT_CHARGE_FLAG"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["hot_charge_flag"].ToString();
						tpssm01["HOT_SEND_DIV"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["hot_send_div"].ToString();

						tpssmdh.Reset();
						tpssmdj.Reset();
						tpssmd7.Reset();
						tpssmdh["ST_NO"] = tpssm01["ST_NO"];
						tpssmdh["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
						tpssmdh.Query("ST_NO,FACTORY_DIV");
						tpssmdj["ROUTEBAGKEY"] = tpssmdh["ROUTEBAGKEY"];
						tpssmdj["PLANTSECTIONTYPE"] = 1;
						if (tpssmdj.QueryCount("ROUTEBAGKEY,PLANTSECTIONTYPE") == 1)
						{
							tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
							flag99 = 0;
						}
						else
						{
							Log::Trace("", __FUNCTION__, "取不到默认路径={0}", tpssmdj["ROUTEBAGKEY"].ToString());
							if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'S')
							{
								tpssmdj["ROUTEBAGKEY"] = "S_DEFAULT";
							}
							else if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'C')
							{
								tpssmdj["ROUTEBAGKEY"] = "C_DEFAULT";
							}
							tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
							flag99 = 1;
							tpssmdj["ROUTEBAGKEY"] = tpssmdh["ROUTEBAGKEY"];
						}
						tpssmd7["ROUTELIST"] = tpssmdj["ROUTELIST"];

						tpssm01["ROUTEBAGKEY"] = tpssmdj["ROUTEBAGKEY"];
						tpssm01["ROUTELIST"] = tpssmdj["ROUTELIST"];

						if (bcls_rec->Tables.Contains("tpssm01"))
						{
							if (bcls_rec->Tables["tpssm01"].Rows[0]["backlog_ea"].ToString().Trim() != "")
							{
								tpssmdj["ROUTEBAGKEY"] = bcls_rec->Tables["tpssm01"].Rows[0]["backlog_ea"].ToString();
								tpssmdj["PLANTSECTIONTYPE"] = 1;
								if (tpssmdj.QueryCount("ROUTEBAGKEY,PLANTSECTIONTYPE") == 1)
								{
									tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
									flag99 = 0;
								}
								else
								{
									if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'S')
									{
										tpssmdj["ROUTEBAGKEY"] = "S_DEFAULT";
									}
									else if (tpssmdj["ROUTEBAGKEY"].ToString()[0] == 'C')
									{
										tpssmdj["ROUTEBAGKEY"] = "C_DEFAULT";
									}
									tpssmdj.Query("ROUTEBAGKEY,PLANTSECTIONTYPE");
									flag99 = 1;
									tpssmdj["ROUTEBAGKEY"] = tpssmdh["ROUTEBAGKEY"];
								}
								tpssmd7["ROUTELIST"] = tpssmdj["ROUTELIST"];

								tpssm01["ROUTEBAGKEY"] = tpssmdj["ROUTEBAGKEY"];
								tpssm01["ROUTELIST"] = tpssmdj["ROUTELIST"];
							}
						}

						tpssm01["CC_TYPE"] = "1";
						tpssm01["SMELT_MODE"] = 0;
						tpssm01["SMELT_DIV"] = "B";
						tpssm01["REFINE_DIV"] = " ";
						tpssm01["BACKLOG_EA"] = " ";
						tpssm01["PLAN_TAP_WT"] = 0;

						cast_lot_div = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_heat_no"].ToString().Substring(bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_heat_no"].ToString().GetLength() - 1, 1);
						if (cast_lot_div == "1") cast_lot_div_num = 1;
						else if (cast_lot_div == "2") cast_lot_div_num = 2;
						else if (cast_lot_div == "3") cast_lot_div_num = 3;
						else if (cast_lot_div == "4") cast_lot_div_num = 4;
						else if (cast_lot_div == "5") cast_lot_div_num = 5;
						else if (cast_lot_div == "6") cast_lot_div_num = 6;
						else if (cast_lot_div == "7") cast_lot_div_num = 7;
						else if (cast_lot_div == "8") cast_lot_div_num = 8;
						else if (cast_lot_div == "9") cast_lot_div_num = 9;
						else if (cast_lot_div == "A") cast_lot_div_num = 10;
						else if (cast_lot_div == "B") cast_lot_div_num = 11;
						else if (cast_lot_div == "C") cast_lot_div_num = 12;
						else if (cast_lot_div == "D") cast_lot_div_num = 13;
						else if (cast_lot_div == "E") cast_lot_div_num = 14;
						else if (cast_lot_div == "F") cast_lot_div_num = 15;
						else if (cast_lot_div == "G") cast_lot_div_num = 16;
						else if (cast_lot_div == "H") cast_lot_div_num = 17;
						else if (cast_lot_div == "I") cast_lot_div_num = 18;
						else if (cast_lot_div == "J") cast_lot_div_num = 19;
						else if (cast_lot_div == "K") cast_lot_div_num = 20;
						else if (cast_lot_div == "L") cast_lot_div_num = 21;
						else if (cast_lot_div == "M") cast_lot_div_num = 22;
						else if (cast_lot_div == "N") cast_lot_div_num = 23;
						else if (cast_lot_div == "O") cast_lot_div_num = 24;
						else if (cast_lot_div == "P") cast_lot_div_num = 25;
						else if (cast_lot_div == "Q") cast_lot_div_num = 26;
						else if (cast_lot_div == "R") cast_lot_div_num = 27;
						else if (cast_lot_div == "S") cast_lot_div_num = 28;
						else if (cast_lot_div == "T") cast_lot_div_num = 29;
						else if (cast_lot_div == "U") cast_lot_div_num = 30;
						else if (cast_lot_div == "V") cast_lot_div_num = 31;
						else if (cast_lot_div == "W") cast_lot_div_num = 32;
						else if (cast_lot_div == "X") cast_lot_div_num = 33;
						else if (cast_lot_div == "Y") cast_lot_div_num = 34;
						else if (cast_lot_div == "Z") cast_lot_div_num = 35;

						tpssm01["CAST_LOT_DIV_NO"] = cast_lot_div_num;
						if (cast_lot_div_num != 1)
						{
							tpssm01["RESTRAND_FLG"] = " ";
						}
						else
						{
							tpssm01["RESTRAND_FLG"] = "T";
						}

						sqlstr = " select DEV_TECH_CODE from tpssmd7 where FACTORY_DIV = @FACTORY_DIV and area_id = 4 and ROUTELIST = @ROUTELIST order by charge_no ";
						cmd_tpssmd7_inq.SetCommandText(sqlstr);
						cmd_tpssmd7_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssmd7_inq.Parameters.Set("ROUTELIST", tpssmd7["ROUTELIST"].ToString());
						cmd_tpssmd7_inq.ExecuteReader();
						while (cmd_tpssmd7_inq.Read())
						{
							tpssm01["REFINE_DIV"] = tpssm01["REFINE_DIV"].ToString() + cmd_tpssmd7_inq.GetString(1);
						}

						sqlstr = " select DEV_TECH_CODE from tpssmd7 where FACTORY_DIV = @FACTORY_DIV and ROUTELIST = @ROUTELIST order by charge_no ";
						cmd_tpssmd7_inq.SetCommandText(sqlstr);
						cmd_tpssmd7_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssmd7_inq.Parameters.Set("ROUTELIST", tpssmd7["ROUTELIST"].ToString());
						cmd_tpssmd7_inq.ExecuteReader();
						while (cmd_tpssmd7_inq.Read())
						{
							tpssm01["BACKLOG_EA"] = tpssm01["BACKLOG_EA"].ToString() + cmd_tpssmd7_inq.GetString(1);
						}
						cmd_tpssmd7_inq.Close();

						tpssm01.TrimOrBlank();
						//CFormattable arguments[] = { tpssm01["PONO"].ToString() };
						//CMessageFormat::Format(s.msg, _RES("该PONO已经存在")/*该PONO已经存在。*/, arguments, 1);
						//throw CApplicationException(-1, s.msg, log.Location);

						sqlstr = "tpssm01.Insert()";
						tpssm01.Insert();

						sqlstr = " select max(CAST_LOT_DIV_NO) from tpssm01 where FACTORY_DIV = @FACTORY_DIV  and CAST_LOT_NO = @CAST_LOT_NO ";
						cmd_tpssm01_inq.SetCommandText(sqlstr);
						cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
						cmd_tpssm01_inq.ExecuteReader();
						if (cmd_tpssm01_inq.Read())
						{
							tpssm01["CAST_LOT_SUM"] = cmd_tpssm01_inq.GetDecimal(1);
						}
						else
						{
							tpssm01["CAST_LOT_SUM"] = 0;
						}
						cmd_tpssm01_inq.Close();
						tpssm01.Update("CAST_LOT_SUM", "CAST_LOT_NO");

						tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
						tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
						if (tpssm02.QueryCount("FACTORY_DIV,CAST_LOT_NO") == 0)
						{
							//------------------------------
							//LOT信息赋值
							sqlstr = "tpssm02.Insert()";
							tpssm02["LOT_STATUS"] = 3;
							tpssm02["ST_NO"] = tpssm01["ST_NO"];
							tpssm02["SLAB_THICK"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_thick"].ToDecimal();
							tqmts0x.Reset();
							tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
							tqmts0x.Query("ST_NO");
							tpssm02["SG_SIGN"] = tqmts0x["SG_GRADE_1"];//bcls_rec->Tables["tpssm03_s"].Rows[i]["sg_sign"].ToString();
							tpssm02["BILLET_TYPE"] = "1";

							tpssm02.TrimOrBlank();
							tpssm02.Insert();
						}

						//tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
						tpssm02["CAST_LOT_SUM"] = tpssm01["CAST_LOT_SUM"];
						tpssm02.Update("CAST_LOT_SUM", "CAST_LOT_NO,FACTORY_DIV");


						//------------------------------
						//写入连铸计划表（TPSSM10）
						//取当前出钢计划中最大的浇注顺序号和PONO
						cc_seq = 0;
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:         // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default:  // 所有数据库适用，通用SQL语句。SQL 语句标准化修改:去掉NVL。 xuwen 2013-4-10
							sqlstr = CString(
								" SELECT MAX(cc_seq) FROM TPSSM10 "
								"  WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
								"    AND CC_MACH_NO = @tpssm01.CC_MACH_NO "
								//"    AND PONO_STATUS >= 16 "
								"    AND CC_SEQ     < 900 "   //900以后是挂起的炉次
								);
							break;
						}
						cmd_tpssm10_inq.SetCommandText(sqlstr);
						cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
						cmd_tpssm10_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
						cmd_tpssm10_inq.ExecuteReader();
						if (cmd_tpssm10_inq.Read())
						{
							cc_seq = cmd_tpssm10_inq.GetDecimal(1);
						}
						else
						{
							cc_seq = 0;
						}
						cmd_tpssm10_inq.Close();
						Log::Trace("", __FUNCTION__, "当前出钢计划中最大的浇注顺序号cc_seq = [{0}]", cc_seq.ToInt32());


						//如果要新增的PONO已存在，那么删除以前的（再排计划时），重新接受.
						//如果要新增的PONO已编入出钢计划，此处的状态校验，在MMS层校验
						//删除该PONO后，不用考虑cc_seq 跳号，只要顺序排列出就行
						tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];  /* 炼钢单元号 -PK*/
						tpssm10["PONO"] = tpssm01["PONO"];
						sqlstr = "tpssm10.Delete()";
						tpssm10.Delete("FACTORY_DIV,PONO");

						//新增计划
						tpssm10.CopyFrom(tpssm01);
						tpssm10["CAST_LOT_NO2"] = tpssm01["CAST_LOT_NO"];
						tpssm10["CAST_LOT_DIV_NO2"] = tpssm01["CAST_LOT_DIV_NO"];
						tpssm10["PONO_STATUS"] = 16;                    /* 制造命令状态 */
						if (cc_seq == 0)
						{
							cc_seq = 1;
						}
						else
						{
							cc_seq = cc_seq + 1;
						}
						tpssm10["CC_SEQ"] = cc_seq;                /* 连铸顺序号 */

						if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'C')
						{
							tpssm10["C_DIV"] = "2";
						}
						else if (tpssm10["ROUTEBAGKEY"].ToString()[0] == 'S')
						{
							tpssm10["C_DIV"] = "1";
						}
						if (tpssm10["C_DIV"].ToString().Trim() == "")
						{
							tqmts0x.Reset();
							tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
							tqmts0x.Query("ST_NO");
							tpssm10["C_DIV"] = tqmts0x["C_DIV"].ToString();
						}

						tpssm10["TD_CHG_FLG"] = 0;                     /* 中间包更换标志 */
						//zxl20160822方坯T默认不要
						tpssm10["BILLET_TYPE"] = "1";

						if (tpssm01["RESTRAND_FLG"].ToString().Trim() == "")//连浇
						{
							tpssm10["CC_PREP_TIME"] = 2;  // 炉间准备时间
						}
						else
						{
							tpssm10["CC_PREP_TIME"] = 100;  // 炉间准备时间
						}

						tpssm10["POUR_TIME"] = 40; //浇注时间
						tpssm10["CC_REQ_TIMEL4"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["plan_cast_time"].ToString();
						tpssm10["CC_REQ_TIME"] = " ";
						tqmts0x.Reset();
						tqmts0x["ST_NO"] = tpssm10["ST_NO"].ToString().Trim();
						tqmts0x.Query("ST_NO");
						tpssm10["SG_SIGN"] = tqmts0x["SG_GRADE_1"];//bcls_rec->Tables["tpssm03_s"].Rows[i]["sg_sign"].ToString();
						tpssm10.TrimOrBlank();
						sqlstr = "tpssm10.Insert()";
						tpssm10.Insert();

						tpssm10["CAST_LOT_SUM"] = tpssm01["CAST_LOT_SUM"];
						tpssm10["CAST_LOT_SUM2"] = tpssm01["CAST_LOT_SUM"];
						tpssm10.Update("CAST_LOT_SUM,CAST_LOT_SUM2", "CAST_LOT_NO,FACTORY_DIV");

						tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
						tpssm99["PONO"] = tpssm10["PONO"];
						tpssm99["EVENT_ID"] = "A1";
						tpssm99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
						tpssm99["VALID_FLAG"] = "1";

						tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

						if (flag99 == 1)
						{
							tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
							tpssm99["PONO"] = tpssm10["PONO"];
							tpssm99["EVENT_ID"] = "1Z";
							tpssm99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
							tpssm99["VALID_FLAG"] = "1";

							tpssm99.MergeTo(in_pssm99trace2.Tables[0], false);
						}
						//Log::Trace("", __FUNCTION__, "接收MMS下发的预计划，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
						//Log::Trace("", __FUNCTION__, "记录履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
					}
					tpssm01.Query("FACTORY_DIV,PONO");

					wt = wt + bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_thick"].ToDecimal() * bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_width"].ToDecimal() * bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_length_aim"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
					tpssm01["PLAN_TAP_WT"] = wt.Round(3);

					if (bcls_rec->Tables.Contains("tpssm01"))
					{
						if (bcls_rec->Tables["tpssm01"].Rows[0]["plan_tap_wt"].ToDecimal() != 0)
						{
							tpssm01["PLAN_TAP_WT"] = bcls_rec->Tables["tpssm01"].Rows[0]["plan_tap_wt"].ToDecimal();
						}
					}

					tpssm10["PLAN_TAP_WT"] = tpssm01["PLAN_TAP_WT"];
					tpssm01.Update("PLAN_TAP_WT", "FACTORY_DIV,PONO");
					tpssm10.Update("PLAN_TAP_WT", "FACTORY_DIV,PONO");

					//------------------------------
					//板坯命令赋值
					long_flag = bcls_rec->Tables["tpssm03_s"].Rows[i]["l_slabno"].ToString();
					if (long_flag == "0")
					{
						tpssm03["FACTORY_DIV"] = "LG1";
						tpssm03["SLAB_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_slab_no"].ToString();
						tpssm03["CAST_LOT_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_cast_no"].ToString();
						tpssm03["PONO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_heat_no"].ToString();
						tpssm03["STRAND_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_strand_no"].ToString();
						tpssm03["BILLET_TYPE"] = "1";
						tpssm03["INGOT_CODE"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["ingot_code"].ToString();
						tpssm03["SLAB_NUM"] = 1;
						tpssm03["SLAB_THICK"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_thick"].ToDecimal();
						tpssm03["MATIRAL_CODE"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["matiral_code"].ToString();
						tpssm03["SLAB_WIDTH"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_width"].ToDecimal();
						tpssm03["SLAB_LEN"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_length_aim"].ToDecimal();
						tpssm03["SLAB_MAX_LEN"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_length_max"].ToDecimal();
						tpssm03["SLAB_MIN_LEN"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_length_min"].ToDecimal();
						//tpssm03["SLAB_WT"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_thick"].ToDecimal() * bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_width"].ToDecimal() * bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_length_aim"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
						//tpssm03["SLAB_WT"] = tpssm03["SLAB_WT"].ToDecimal().Round(3);
						tpssm03["SLAB_WT"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_wt"].ToDecimal();
						tpssm03["ORDER_NO"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["order_no"].ToString();
						tpssm03["SLAB_DEST"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_dest"].ToString();
						tpssm03["HOT_SEND_FLAG"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["hot_send_div"].ToString();
						tpssm03["HOT_CHARGE_FLAG"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["hot_charge_flag"].ToString();
						tpssm03["SLAB_SEQ_2"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["pre_strand_seq_no"].ToDecimal();
						tpssm03["SG_SIGN"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["sg_sign"].ToString();
						tpssm03["FACTORY_NEXT"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["plant_next"].ToString();
						tpssm03["APN"] = bcls_rec->Tables["tpssm03_s"].Rows[i]["slab_fin_use"].ToString();
						tpssm03["LSLAB_T_B_FLAG"] = "0";
						tpssm03["LSLAB_NO"] = tpssm03["SLAB_NO"];
						tpssm03["SLAB_PROD_FLAG"] = "0";

						//删除原铸坯
						sqlstr = "tpssm03.Delete(SLAB_NO)";
						tpssm03.Delete("SLAB_NO");

						//长坯号为空时，板坯号赋给长坯号
						//if (tpssm03["LSLAB_NO"].ToString().Trim() == "")
						//{
						//tpssm03["LSLAB_NO"] = tpssm03["SLAB_NO"];
						//}

						//新增铸坯
						sqlstr = "tpssm03.Insert()";
						tpssm03.TrimOrBlank();
						tpssm03.Insert();

						if (updatetpssm10_flag == "0")
						{
							//更新浇铸计划表
							tpssm10["FACTORY_DIV"] = tpssm03["FACTORY_DIV"];
							tpssm10["PONO"] = tpssm03["PONO"];
							tpssm10["SLAB_THICK"] = tpssm03["SLAB_THICK"];
							tpssm10["SLAB_WIDTH"] = tpssm03["SLAB_WIDTH"];
							tpssm10["SLAB_LEN"] = tpssm03["SLAB_LEN"];
							sqlstr = "tpssm10.Update()";
							tpssm10.Update(
								"SLAB_THICK,"
								"SLAB_WIDTH,"
								"SLAB_LEN",
								"FACTORY_DIV,PONO");

							updatetpssm10_flag = "1";
						}
					}
				}
			}
		}
		else if (v_proc_div == 3) //3-炉次删除
		{
			tpssm01["FACTORY_DIV"] = "LG1";
			tpssm01["PONO"] = bcls_rec->Tables["tpssm03"].Rows[0]["pre_heat_no"].ToString();
			tpssm01.Query("FACTORY_DIV,PONO");
			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:         // MS SQL Server数据库
			//case DB_KIND_ORACLE:        // Oracle 数据库
			//default:  // 所有数据库适用，通用SQL语句。
			//	sqlstr = CString(
			//		" SELECT FACTORY_DIV FROM TPSSM01  "
			//		" WHERE PONO = @tpssm01.PONO "

			//		);
			//	break;
			//}
			//cmd_tpssm01_inq1.SetCommandText(sqlstr);
			//cmd_tpssm01_inq1.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			//cmd_tpssm01_inq1.ExecuteReader();
			//if (cmd_tpssm01_inq1.Read())
			//{
			//	tpssm01["FACTORY_DIV"] = cmd_tpssm01_inq1.GetString(1);
			//}
			//cmd_tpssm01_inq1.Close();
			/* 特别注释：2014-12-03，xuwen修改，PONO删除逻辑
			1、判断当前PONO状态，如已编入出钢计划(18)，报错退出
			2、对于TPSSM10表，是开浇第一炉(带“T”)，T传递给下一炉，包括准备时间
			3、对于TPSSM01表，删除PONO及对应铸坯（对于浮动铸坯命令，不删除）
			*/
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["PONO"] =[{0}]", tpssm01["PONO"].ToString());
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["FACTORY_DIV"] =[{0}]", tpssm01["FACTORY_DIV"].ToString());
			//dclian---add---2015-11-16------
			tpssm99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm99["PONO"] = tpssm01["PONO"];
			tpssm99["EVENT_ID"] = "A3";
			tpssm99["VALID_FLAG"] = "1";
			tpssm99["PONO_STATUS"] = tpssm01["PONO_STATUS"];
			//正应答
			//inBlock1.Tables["X200007"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			//inBlock1.Tables["X200007"].Rows[0]["PONO"] = tpssm01["PONO"];
			//inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "3";  // PONO删除成功

			//--------------------------
			//1、浇铸计划表处理
			//读取要删除PONO信息
			tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm10["PONO"] = tpssm01["PONO"];
			sqlstr = "tpssm10.Query()";
			bool has10 = tpssm10.Query();

			if (has10 == true) //有记录
			{
				//1）状态校验
				if (tpssm10["PONO_STATUS"].ToDecimal() >= 18) //编入出钢计划或生产，不能删除，给负应答
				{

					tpssm99["VALID_FLAG"] = "0";

					tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

					//inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "4";  // PONO删除失败

					tpabort(0);
					tpbegin(0, 0);
					Log::Trace("", __FUNCTION__, "★★★★★记录LOT收回失败履历★★★★★");
					//记录履历
					ret = 0;
					ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

#ifdef _SYS_PES
					Log::Trace("", __FUNCTION__, "★★★★★发送应答电文开始★★★★★");
					//发送命令接收应答至MMS--
					//ret = f_cm_200007_snd(&inBlock1, bcls_ret, conn);
					//if (ret != 0)
					//{
					//发送应答电文失败，不应该影响接收炉次制造命令电文
					//}
					Log::Trace("", __FUNCTION__, "★★★★★发送应答f_cm_200007_snd★★★★★");
#endif
					tpcommit(0);
					tpbegin(0, 0);
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "要删除炉次[{0}]已排入出钢计划，不能删除。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//dclian---add---2015-11-16------
				/*		Log::Trace("", __FUNCTION__, "接收MMS下发的LOT收回，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());*/


				//2）"T"标记传递给下一炉
				if (tpssm10["RESTRAND_FLG"].ToString().Trim() != "")
				{

					Log::Trace("", __FUNCTION__, "cc_mach_no =[{0}]", tpssm10["CC_MACH_NO"].ToString());
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

				Log::Trace("", __FUNCTION__, "删除tpssm10");


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

				//dclian---add---2015-11-16------

				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

				Log::Trace("", __FUNCTION__, "回收LOT,传入in_pssm99trace=[{0}]", in_pssm99trace.Tables[0].Rows[0]["EVENT_ID"].ToString());
				//dclian---add---2015-11-16------

			}//if  TPSSM10 有记录

			//--------------------------
			//2、炉次命令表（TPSSM01）处理
			//读取要删除PONO信息

			sqlstr = "tpssm01.Query()";
			dummy = tpssm01.QueryCount("FACTORY_DIV,PONO");

			//删除命令炉次表
			sqlstr = "tpssm01.Delete()";
			tpssm01.Delete();

			Log::Trace("", __FUNCTION__, "删除tpssm01");
			//--------------------------
			//3、删除LOT表(TPSSM02)
			if (dummy>0) //TPSSM01有记录
			{
				//查询炉次表中指定LOT下的PONO是否还有, 没有就将LOT删除
				sqlstr = "tpssm01.QueryCount()";
				dummy = tpssm01.QueryCount("FACTORY_DIV,CAST_LOT_NO");

				if (dummy <= 0) //没有炉次，LOT删除
				{
					tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"].ToString().Trim();
					tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"].ToString().Trim();
					sqlstr = "tpssm02.Delete()";
					tpssm02.Delete("FACTORY_DIV,CAST_LOT_NO");
				}
				Log::Trace("", __FUNCTION__, "删除tpssm02");
			}

			//--------------------------
			//4、删除板坯表(TPSSM03)
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = CString(
					" DELETE TPSSM03 "
					"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
					"    AND PONO	= @tpssm10.PONO "
					);
				break;
			}
			cmd_tpssm03_upd.SetCommandText(sqlstr);
			cmd_tpssm03_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
			cmd_tpssm03_upd.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
			cmd_tpssm03_upd.ExecuteNonQuery();

			Log::Trace("", __FUNCTION__, "删除tpssm03");

#ifdef _LINE_BW
			////HYF 20140502调用轧钢函数，删除棒线轧制计划
			//CDataRow &row2 = inBlock2.Tables["PSBW"].Rows.Add();
			//row2["PONO"] = tpssm10["PONO"];
			////
			//ret = f_psbw_roll_del(&inBlock2, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
#endif


		}//if 4-炉次收回，按指定PONO删除
		else if (v_proc_div == 4)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句。
				sqlstr = CString(
					" SELECT FACTORY_DIV FROM TPSSM01  "
					" WHERE PONO = @tpssm01.PONO "

					);
				break;
			}
			cmd_tpssm01_inq1.SetCommandText(sqlstr);
			cmd_tpssm01_inq1.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm01_inq1.ExecuteReader();
			if (cmd_tpssm01_inq1.Read())
			{
				tpssm01["FACTORY_DIV"] = cmd_tpssm01_inq1.GetString(1);
			}
			cmd_tpssm01_inq1.Close();
			/* 特别注释：2014-12-03，xuwen修改，PONO删除逻辑
			1、判断当前PONO状态，如已编入出钢计划(18)，报错退出
			2、对于TPSSM10表，是开浇第一炉(带“T”)，T传递给下一炉，包括准备时间
			3、对于TPSSM01表，删除PONO及对应铸坯（对于浮动铸坯命令，不删除）
			*/
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["PONO"] =[{0}]", tpssm01["PONO"].ToString());
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["FACTORY_DIV"] =[{0}]", tpssm01["FACTORY_DIV"].ToString());
			//dclian---add---2015-11-16------
			tpssm99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm99["PONO"] = tpssm01["PONO"];
			tpssm99["EVENT_ID"] = "A3";
			tpssm99["VALID_FLAG"] = "1";
			tpssm99["PONO_STATUS"] = tpssm01["PONO_STATUS"];
			//正应答
			//inBlock1.Tables["X200007"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			//inBlock1.Tables["X200007"].Rows[0]["PONO"] = tpssm01["PONO"];
			//inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "0";  // LOT收回正常

			//--------------------------
			//1、浇铸计划表处理
			//读取要删除PONO信息
			tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm10["PONO"] = tpssm01["PONO"];
			sqlstr = "tpssm10.Query()";
			bool has10 = tpssm10.Query();

			if (has10 == true) //有记录
			{
				//1）状态校验
				if (tpssm10["PONO_STATUS"].ToDecimal() >= 18) //编入出钢计划或生产，不能删除，给负应答
				{

					tpssm99["VALID_FLAG"] = "0";

					tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

					//inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "1";  // 收回失败1

					tpabort(0);
					tpbegin(0, 0);
					Log::Trace("", __FUNCTION__, "★★★★★记录LOT收回失败履历★★★★★");
					//记录履历
					ret = 0;
					ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

#ifdef _SYS_PES
					Log::Trace("", __FUNCTION__, "★★★★★发送应答电文开始★★★★★");
					//发送命令接收应答至MMS--
					//ret = f_cm_200007_snd(&inBlock1, bcls_ret, conn);
					//if (ret != 0)
					//{
					//发送应答电文失败，不应该影响接收炉次制造命令电文
					//}
					Log::Trace("", __FUNCTION__, "★★★★★发送应答f_cm_200007_snd★★★★★");
#endif

					tpcommit(0);
					tpbegin(0, 0);
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "要删除炉次[{0}]已排入出钢计划，不能删除。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//dclian---add---2015-11-16------
				/*	Log::Trace("", __FUNCTION__, "接收MMS下发的LOT收回，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());*/


				//2）"T"标记传递给下一炉
				if (tpssm10["RESTRAND_FLG"].ToString().Trim() != "")
				{

					Log::Trace("", __FUNCTION__, "cc_mach_no =[{0}]", tpssm10["CC_MACH_NO"].ToString());
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

				Log::Trace("", __FUNCTION__, "删除tpssm10");


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

				//dclian---add---2015-11-16------

				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

				Log::Trace("", __FUNCTION__, "回收LOT,传入in_pssm99trace=[{0}]", in_pssm99trace.Tables[0].Rows[0]["EVENT_ID"].ToString());
				//dclian---add---2015-11-16------

			}//if  TPSSM10 有记录

			//--------------------------
			//2、炉次命令表（TPSSM01）处理
			//读取要删除PONO信息

			sqlstr = "tpssm01.Query()";
			dummy = tpssm01.QueryCount("FACTORY_DIV,PONO");

			//删除命令炉次表
			sqlstr = "tpssm01.Delete()";
			tpssm01.Delete();

			Log::Trace("", __FUNCTION__, "删除tpssm01");
			//--------------------------
			//3、删除LOT表(TPSSM02)
			if (dummy>0) //TPSSM01有记录
			{
				//查询炉次表中指定LOT下的PONO是否还有, 没有就将LOT删除
				sqlstr = "tpssm01.QueryCount()";
				dummy = tpssm01.QueryCount("FACTORY_DIV,CAST_LOT_NO");

				if (dummy <= 0) //没有炉次，LOT删除
				{
					tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"].ToString().Trim();
					tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"].ToString().Trim();
					sqlstr = "tpssm02.Delete()";
					tpssm02.Delete("FACTORY_DIV,CAST_LOT_NO");
				}
				Log::Trace("", __FUNCTION__, "删除tpssm02");
			}

			//--------------------------
			//4、删除板坯表(TPSSM03)
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = CString(
					" DELETE TPSSM03 "
					"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
					"    AND PONO	= @tpssm10.PONO "
					);
				break;
			}
			cmd_tpssm03_upd.SetCommandText(sqlstr);
			cmd_tpssm03_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
			cmd_tpssm03_upd.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
			cmd_tpssm03_upd.ExecuteNonQuery();

			Log::Trace("", __FUNCTION__, "删除tpssm02");

#ifdef _LINE_BW
			////HYF 20140502调用轧钢函数，删除棒线轧制计划
			//CDataRow &row2 = inBlock2.Tables["PSBW"].Rows.Add();
			//row2["PONO"] = tpssm10["PONO"];
			////
			//ret = f_psbw_roll_del(&inBlock2, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
#endif


		}//if 4-炉次收回，按指定PONO删除

#ifdef _SYS_PES
		//Log::Trace("", __FUNCTION__, "最后发送正应答至MMS=[{0}]", inBlock1.Tables[0].Rows.get_Count());
		//发送命令接收应答至MMS--
		//ret = f_cm_200007_snd(&inBlock1, bcls_ret, conn);
		//if (ret != 0)
		//{
		//发送应答电文失败，不应该影响接收炉次制造命令电文
		//}
		//Log::Trace("", __FUNCTION__, "最终记录成功履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
#endif

		//记录编入计划成功的履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (flag99 == 1)
		{
			ret = 0;
			ret = f_pssm99_trace(&in_pssm99trace2, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
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
	cmd_tpssm03_upd.Close();
	cmd_tpssm01_inq.Close();
	cmd_tpssm10_inq.Close();
	return doFlag;

}
