/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   何云飞
Date:     2009-05-31 17:13:56
Version:  3.1.0
Description: 出钢计划钢水交换
Update：   2014-11-28  xuwen  炉次条件合并(小何写得很好，改动不大)
2015-03-20  lijie 整合数据表
**************************************************/
#include "stdafx.h"






int f_qmts_stno_chgd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//质量接口，钢水对换用接口
int f_mmsm0055_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//物料实绩接口
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表
//int f_cm_x1l262_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_pssm_reranking(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
#if defined _SYS_PES
//int f_cm_pam1p3_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //发送MMS电文-钢种变更
//int f_pssm_pas1p3_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //发送炼钢L2电文-钢种变更
//int f_cm_x1xb62_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_cm_eecm63_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_cm_x1l263_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//送炼钢L2电文
#endif


/*<remark>=========================================================
/// <summary>
/// 钢水交换
/// <para>钢水交换必须是二炉计划，分为已生产的和已生产的，已生产的和未生产的情况。</para>
/// <para>对计划主表，二炉计划交换heat_no。                          </para>
/// <para>对转炉/电炉工序，对应的熔炼号、时间和设备代码进行交换；    </para>
/// <para>对精炼工序：              </para>
/// <para>1)如果2个计划都没经过精炼，则交换精炼路径。                </para>
/// <para>2)如果其中一个经过精炼了，则保留该精炼的全部实绩，并且将另个计划的未走过的精炼追加上去。</para>
/// <para>3)如果本身未走过精炼，另外个计划已经走完精炼，则交换后该计划删除精炼，由调度人工确定。  </para>
/// <para>对连铸工序，直接交换PONO号和计划号                </para>
/// <para>数据库表：TPSSM11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：前台PSSM18画面（钢水交换）调用。              </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <param name="PONO1">制造命令号1                  </param>
/// <param name="PONO2">制造命令号2                  </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18a_chg_in)

int f_pssm18a_chg_in(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq = 0, rows = 0, i = 0;
	int     dummy = 0;
	CString v_factory_div = "";	//炼钢单元号
	CString 	backlog_ea = "";
	CString 	refine_code = "";
	CDecimal 	change_no = 0;
	int 	sr1 = 0;
	int 	sr2 = 0;
	int     x = 0;
	int     cc_charge_no_1 = 0;
	int     cc_charge_no_2 = 0;
	CString station_id_out = "";
	CString station_id_in = "";
	CTimeSpan proc_time;
	CDateTime start_time;
	CDateTime end_time;
	EIClass  inBlock;
	EIClass  outBlock;

	EIClass inBlock1;
	EIClass inBlock2;  //调用PM模块的接口
	EIClass inBlock3;//调用MM模块的接口
	EIClass inBlock4;//调用QM模块的接口
	EIClass inBlock5;//发送L2电文接口
	EIClass inBlock6;
	EIClass inBlock8;
	EIClass outBlock1;
	EIClass inSendL2;
	EIClass inBlock99; //调用炼钢计划履历用	

	CString sqlstr = "";

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssm11_1("TPSSM11");//钢水炉次1
	CModel tpssm11_2("TPSSM11");//钢水炉次2
	CModel tpssm12_1("TPSSM12");
	CModel tpssm12_2("TPSSM12");
	CModel tpssm12_3("TPSSM12");

	CModel tpssm12_51("TPSSM12");
	CModel tpssm12_52("TPSSM12");
	CModel tpssm99("TPSSM99");

	//定义出钢计划主表查询
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);

	//1)送MMS电文参数
	inBlock1.Tables[0].set_TableName("XX1L263");
	inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO_OLD");//老的制造命令号
	inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO_NEW");//新的制造命令号

	//设置与MMS接口的输入参数: 
	inBlock2.Tables[0].set_TableName("X200008");
	inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");    //主工序代码
	inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO_OLD");    //老的制造命令号
	inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO_NEW");     //新的制造命令号 
	//设置MM接口的输入参数:
	inBlock3.Tables[0].set_TableName("MMSM0055");
	inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_OUT");
	inBlock3.Tables[0].Columns.Add(DT_STRING, "ST_NO_OUT");
	inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_IN");
	inBlock3.Tables[0].Columns.Add(DT_STRING, "ST_NO_IN");

	//调用QM的接口的输入参数:
	inBlock4.Tables[0].set_TableName("STNO_CHG");
	inBlock4.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inBlock4.Tables[0].Columns.Add(DT_STRING, "WHOLE_BACKLOG_CODE_OUT");  //钢种变更的工序点
	inBlock4.Tables[0].Columns.Add(DT_STRING, "WHOLE_BACKLOG_CODE_IN");  //钢种变更的工序点
	inBlock4.Tables[0].Columns.Add(DT_STRING, "PONO_OUT");
	inBlock4.Tables[0].Columns.Add(DT_STRING, "ST_NO_OUT");	//质量中该参数不用
	inBlock4.Tables[0].Columns.Add(DT_STRING, "PONO_IN");
	inBlock4.Tables[0].Columns.Add(DT_STRING, "ST_NO_IN");

	//设置发送L2电文接口的输入参数:
	inBlock5.Tables[0].set_TableName("H4LG07");
	inBlock5.Tables[0].Columns.Add(DT_STRING, "PLAN_NO_1");//前炉计划号
	inBlock5.Tables[0].Columns.Add(DT_STRING, "PLAN_NO_2");//后炉计划号
	inBlock5.Tables[0].Columns.Add(DT_STRING, "PONO_1");//前炉制造命令号
	inBlock5.Tables[0].Columns.Add(DT_STRING, "PONO_2");//后炉制造命令号
	inBlock5.Tables[0].Columns.Add(DT_STRING, "ST_NO_1");//前炉内部钢种
	inBlock5.Tables[0].Columns.Add(DT_STRING, "ST_NO_2");//后炉内部钢种

	inBlock6.Tables[0].set_TableName("XX1L263"); ////PONO_CHG_2
	inBlock6.Tables[0].Columns.Add(DT_STRING, "OPER_FLAG");
	inBlock6.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");    //主工序代码
	inBlock6.Tables[0].Columns.Add(DT_STRING, "PONO_OLD");    //老的制造命令号
	inBlock6.Tables[0].Columns.Add(DT_STRING, "PONO_NEW");     //新的制造命令号 

	//inSendL2.Tables.Add();
	inSendL2.Tables[0].Columns.Add(DT_STRING, "ACTION_CODE");  //炼钢单元号
	inSendL2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inSendL2.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	inSendL2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inSendL2.Tables[0].Rows.Add();

	inBlock8.Tables[0].set_TableName("XX1L262");  //
	inBlock8.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	inBlock8.Tables[0].Columns.Add(DT_STRING, "OPER_FLAG");
	inBlock8.Tables[0].Rows.Add();

	//设置调用炼钢计划履历传入参数
	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);

	try
	{
		//-----------------------------------------------------
		// 获得输入参数，2条记录
		blkseq = bcls_rec->Tables.IndexOf("HEAT_CHG"); //钢水对换 数据块
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到钢水对换数据块[HEAT_CHG]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [HEAT_CHG] NOT EXIST in pssm11_chg().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		if (rows != 2)
		{
			strcpy(s.msg, _RES("PSSMS0000177"))/*钢水交换必须有二炉计划*/;
			//strcpy(s.sysmsg,"钢水交换必须有二炉计划");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];

		//读取传入的 2炉，做校验
		for (i = 1; i <= 2; i++)
		{

			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i - 1]["FACTORY_DIV"].ToString();
			tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[blkseq].Rows[i - 1]["SM_PLAN_NO"].ToString();

			Log::Trace("", __FUNCTION__, "钢水交换PONO[{0}]计划号[{1}]", i, tpssm11["SM_PLAN_NO"].ToString());

			sqlstr = "tpssm11.Query()";
			bool has11 = tpssm11.Query("FACTORY_DIV,SM_PLAN_NO");
			if (has11 == false)
			{
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString() };
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]不存在，请重新选择后操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm11.TrimOrBlank();

			if (tpssm11["RUN_STATUS"].ToString() >= "51" || tpssm11["PONO_STATUS"].ToDecimal() >= 83) //51-包到连铸（模铸）  
			{
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString() };
				CMessageFormat::Format(s.msg, "炼钢计划号[{0}]已浇注，不能做钢水交换。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}


			if (i == 1)
				tpssm11_1.CopyFrom(tpssm11);
			else
				tpssm11_2.CopyFrom(tpssm11);

		}


		//有一炉生产，即可做
		if (!(tpssm11_1["RUN_STATUS"].ToString() >= "33" || tpssm11_2["RUN_STATUS"].ToString() >= "33"))//20-进入生产
		{
			//strcpy(s.msg, _RES("PSSMS0000179"))/*交换的计划都没有生产或是都已经开浇，无法对换*/;
			CFormattable arguments[] = { tpssm11_1["SM_PLAN_NO"].ToString(), tpssm11_2["SM_PLAN_NO"].ToString() };
			CMessageFormat::Format(s.msg, "交换的两炉计划[{0}, {1}]都未到达吹炼开始状态，不能做钢水交换。", arguments, 2);
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//----------------------------------------------------------------
		//1. 修改主计划内容：二炉计划交换heat_no
		//1) 计划表TPSSM11修改  2->1

		if (tpssm11_1["CC_MACH_NO"].ToString() == tpssm11_2["CC_MACH_NO"].ToString())
		{
			change_no = 1;
		}
		else change_no = 0;

		if (change_no == 0 || change_no == 1)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"UPDATE TPSSM11 "
					"   SET	PONO  = @pono, "
					"		ST_NO = @st_no, "
					"		CC_MACH_NO  = @cc_mach_no, "
					"		CAST_NO     = @cast_no, "
					"		CAST_DIV_NO = @cast_div_no, "
					"		RESTRAND_FLG = @restrand_flg "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no "
					);
				break;
			}
			CDbCommand cmd_upd1(sqlstr, conn);
			cmd_upd1.Parameters.Set("pono", tpssm11_2["PONO"].ToString());
			cmd_upd1.Parameters.Set("st_no", tpssm11_2["ST_NO"].ToString());
			cmd_upd1.Parameters.Set("cc_mach_no", tpssm11_2["CC_MACH_NO"].ToString());
			cmd_upd1.Parameters.Set("cast_no", tpssm11_2["CAST_NO"].ToString());
			cmd_upd1.Parameters.Set("cast_div_no", tpssm11_2["CAST_DIV_NO"].ToDecimal());
			cmd_upd1.Parameters.Set("restrand_flg", tpssm11_2["RESTRAND_FLG"].ToString());
			cmd_upd1.Parameters.Set("factory_div", tpssm11_1["FACTORY_DIV"].ToString());
			cmd_upd1.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd1.ExecuteNonQuery();


			//2) 计划表TPSSM11修改  1->2
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"UPDATE TPSSM11 "
					"   SET	PONO  = @pono, "
					"		ST_NO = @st_no, "
					"		CC_MACH_NO  = @cc_mach_no, "
					"		CAST_NO     = @cast_no, "
					"		CAST_DIV_NO = @cast_div_no, "
					"		RESTRAND_FLG     = @restrand_flg "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no "
					);
				break;
			}
			CDbCommand cmd_upd3(sqlstr, conn);
			//cmd_upd3.Parameters.Set("pono", tpssm11_1["PONO"].ToString());
			//cmd_upd3.Parameters.Set("st_no", tpssm11_1["ST_NO"].ToString());
			//cmd_upd3.Parameters.Set("cc_mach_no", tpssm11_1["CC_MACH_NO"].ToString());
			//cmd_upd3.Parameters.Set("cast_no", tpssm11_1["CAST_NO"].ToString());
			//cmd_upd3.Parameters.Set("cast_div_no", tpssm11_1["CAST_DIV_NO"].ToDecimal());
			//cmd_upd3.Parameters.Set("restrand_flg", tpssm11_1["RESTRAND_FLG"].ToString());
			//cmd_upd3.Parameters.Set("factory_div", tpssm11_2["FACTORY_DIV"].ToString());
			//cmd_upd3.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd3.Parameters.Set("pono", tpssm11_1["PONO"].ToString());
			cmd_upd3.Parameters.Set("st_no", tpssm11_1["ST_NO"].ToString());
			cmd_upd3.Parameters.Set("cc_mach_no", tpssm11_1["CC_MACH_NO"].ToString());
			cmd_upd3.Parameters.Set("cast_no", tpssm11_1["CAST_NO"].ToString());
			cmd_upd3.Parameters.Set("cast_div_no", tpssm11_1["CAST_DIV_NO"].ToDecimal());
			cmd_upd3.Parameters.Set("restrand_flg", tpssm11_1["RESTRAND_FLG"].ToString());
			cmd_upd3.Parameters.Set("factory_div", tpssm11_2["FACTORY_DIV"].ToString());
			cmd_upd3.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd3.ExecuteNonQuery();
		}
		//if (change_no == 1)
		//{
		//	switch (conn->DatabaseKind)
		//	{
		//	case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		//	case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		//	case DB_KIND_MSSQL:         // MS SQL Server数据库
		//	case DB_KIND_ORACLE:        // Oracle 数据库
		//	default:  // 所有数据库适用，通用SQL语句
		//		sqlstr = CString(
		//			"UPDATE TPSSM11 "
		//			"   SET	PONO  = @pono, "
		//			"		ST_NO = @st_no, "
		//			"		CC_MACH_NO  = @cc_mach_no "
		//			/*"		CAST_NO     = @cast_no, "
		//			"		CAST_DIV_NO = @cast_div_no, "
		//			"		RESTRAND_FLG = @restrand_flg "*/
		//			" WHERE FACTORY_DIV = @factory_div "
		//			"	AND SM_PLAN_NO = @sm_plan_no "
		//			);
		//		break;
		//	}
		//	CDbCommand cmd_upd1(sqlstr, conn);
		//	cmd_upd1.Parameters.Set("pono", tpssm11_2["PONO"].ToString());
		//	cmd_upd1.Parameters.Set("st_no", tpssm11_2["ST_NO"].ToString());
		//	cmd_upd1.Parameters.Set("cc_mach_no", tpssm11_2["CC_MACH_NO"].ToString());
		//	cmd_upd1.Parameters.Set("cast_no", tpssm11_2["CAST_NO"].ToString());
		//	cmd_upd1.Parameters.Set("cast_div_no", tpssm11_2["CAST_DIV_NO"].ToDecimal());
		//	cmd_upd1.Parameters.Set("restrand_flg", tpssm11_2["RESTRAND_FLG"].ToString());
		//	cmd_upd1.Parameters.Set("factory_div", tpssm11_1["FACTORY_DIV"].ToString());
		//	cmd_upd1.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//	cmd_upd1.ExecuteNonQuery();


		//	//2) 计划表TPSSM11修改  1->2
		//	switch (conn->DatabaseKind)
		//	{
		//	case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		//	case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		//	case DB_KIND_MSSQL:         // MS SQL Server数据库
		//	case DB_KIND_ORACLE:        // Oracle 数据库
		//	default:  // 所有数据库适用，通用SQL语句
		//		sqlstr = CString(
		//			"UPDATE TPSSM11 "
		//			"   SET	PONO  = @pono, "
		//			"		ST_NO = @st_no, "
		//			"		CC_MACH_NO  = @cc_mach_no "
		//			/*"		CAST_NO     = @cast_no, "
		//			"		CAST_DIV_NO = @cast_div_no, "
		//			"		RESTRAND_FLG     = @restrand_flg "*/
		//			" WHERE FACTORY_DIV = @factory_div "
		//			"	AND SM_PLAN_NO = @sm_plan_no "
		//			);
		//		break;
		//	}
		//	CDbCommand cmd_upd3(sqlstr, conn);
		//	//cmd_upd3.Parameters.Set("pono", tpssm11_1["PONO"].ToString());
		//	//cmd_upd3.Parameters.Set("st_no", tpssm11_1["ST_NO"].ToString());
		//	//cmd_upd3.Parameters.Set("cc_mach_no", tpssm11_1["CC_MACH_NO"].ToString());
		//	//cmd_upd3.Parameters.Set("cast_no", tpssm11_1["CAST_NO"].ToString());
		//	//cmd_upd3.Parameters.Set("cast_div_no", tpssm11_1["CAST_DIV_NO"].ToDecimal());
		//	//cmd_upd3.Parameters.Set("restrand_flg", tpssm11_1["RESTRAND_FLG"].ToString());
		//	//cmd_upd3.Parameters.Set("factory_div", tpssm11_2["FACTORY_DIV"].ToString());
		//	//cmd_upd3.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//	cmd_upd3.Parameters.Set("pono", tpssm11_1["PONO"].ToString());
		//	cmd_upd3.Parameters.Set("st_no", tpssm11_1["ST_NO"].ToString());
		//	cmd_upd3.Parameters.Set("cc_mach_no", tpssm11_1["CC_MACH_NO"].ToString());
		//	cmd_upd3.Parameters.Set("cast_no", tpssm11_1["CAST_NO"].ToString());
		//	cmd_upd3.Parameters.Set("cast_div_no", tpssm11_1["CAST_DIV_NO"].ToDecimal());
		//	cmd_upd3.Parameters.Set("restrand_flg", tpssm11_1["RESTRAND_FLG"].ToString());
		//	cmd_upd3.Parameters.Set("factory_div", tpssm11_2["FACTORY_DIV"].ToString());
		//	cmd_upd3.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//	cmd_upd3.ExecuteNonQuery();
		//}

		//----------------------------------------------------------------
		//2. 修改工序计划(子计划)内容：

		//1 如果2个计划都没经过精炼，则交换精炼路径
		//2 如果其中一个经过精炼了，则保留该精炼的全部实绩，并且将另个计划的 未走过的精炼追加上去
		//3 如果本身未走过精炼，另外个计划已经走完精炼，则交换后该计划删除精炼，由调度人工确定
		//判断计划1 是否经过精炼 
		sr1 = 0;
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "SELECT * FROM TPSSM12 "
		//		" WHERE AREA_ID IN (4, 5) "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		"	AND CHARGE_NO > @change_no "
		//		" ORDER BY  charge_no ASC ";
		//	break;
		//}
		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//cmd_tpssm12_inq.Parameters.Set("change_no", change_no);
		//cmd_tpssm12_inq.ExecuteReader();

		//while (cmd_tpssm12_inq.Read())
		//{
		//	cmd_tpssm12_inq.Fetch(tpssm12_1);
		//	
		//}

		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "SELECT * FROM TPSSM12 "
		//		" WHERE AREA_ID = 4 "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		" ORDER BY  charge_no ASC ";
		//	break;
		//}

		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//cmd_tpssm12_inq.ExecuteReader();

		//while (cmd_tpssm12_inq.Read())
		//{
		//	cmd_tpssm12_inq.Fetch(tpssm12_1);
		//	
		//	if (tpssm12_1["DEV_CODE"].ToString().Substring(0, 1) == "A")
		//	{
		//		continue;
		//	}			

		//	if (tpssm12_1["CHARGE_NO"].ToDecimal() <= tpssm11_1["CURR_WP_NO"].ToDecimal())
		//	{
		//		//当前工序已经生产结束，保留该精炼				
		//	}
		//	else
		//	{
		//		sr1++;
		//		//该精炼还未生产，则将其转给计划2 ,CHARGENO 先写个99
		//		Log::Trace("", __FUNCTION__, "2 该精炼还未生产，则将其转给计划2 ,CHARGENO 先写个99 sr1=[{0}],tpssm12_1.charge_no=[{1}] ", sr1, tpssm12_1["CHARGE_NO"].ToDecimal().ToInt32());

		//		switch (conn->DatabaseKind)
		//		{
		//		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//		case DB_KIND_ORACLE:	        // Oracle 数据库
		//		default: // 所有数据库适用，通用SQL语句
		//			sqlstr = "UPDATE TPSSM12 "
		//				"	SET	SM_PLAN_NO = @sm_plan_no, "
		//				"		CHARGE_NO = @sr1+80 "
		//				" WHERE FACTORY_DIV = @factory_div "
		//				"	AND SM_PLAN_NO = @sm_plan_no1 "
		//				"	AND CHARGE_NO =@charge_no ";
		//			break;
		//		}

		//		CDbCommand cmd_upd12(sqlstr, conn);
		//		cmd_upd12.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//		cmd_upd12.Parameters.Set("sr1", sr1);
		//		cmd_upd12.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//		cmd_upd12.Parameters.Set("sm_plan_no1", tpssm11_1["SM_PLAN_NO"].ToString());
		//		cmd_upd12.Parameters.Set("charge_no", tpssm12_1["CHARGE_NO"].ToDecimal());
		//		cmd_upd12.ExecuteNonQuery();
		//	}
		//}
		//cmd_tpssm12_inq.Close();


		////判断计划2 是否经过精炼 
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "SELECT CHARGE_NO FROM TPSSM12 "
		//		" WHERE FACTORY_DIV = @factory_div "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		"	AND AREA_ID =5 ";
		//	break;
		//}

		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//cmd_tpssm12_inq.ExecuteReader();

		//while (cmd_tpssm12_inq.Read())
		//{
		//	cc_charge_no_1 = cmd_tpssm12_inq.GetInt32(1);

		//}
		//cmd_tpssm12_inq.Close();

		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "SELECT CHARGE_NO FROM TPSSM12 "
		//		" WHERE FACTORY_DIV = @factory_div "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		"	AND AREA_ID =5 ";
		//	break;
		//}


		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//cmd_tpssm12_inq.ExecuteReader();

		//while (cmd_tpssm12_inq.Read())
		//{
		//	cc_charge_no_2 = cmd_tpssm12_inq.GetInt32(1);

		//}
		//cmd_tpssm12_inq.Close();

		sr2 = 0;
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "SELECT * FROM TPSSM12 "
		//		" WHERE AREA_ID = 4 "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		" ORDER BY  charge_no ASC ";
		//	break;
		//}


		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//cmd_tpssm12_inq.ExecuteReader();

		//while (cmd_tpssm12_inq.Read())
		//{
		//	cmd_tpssm12_inq.Fetch(tpssm12_2);

		//	if (tpssm12_2["DEV_CODE"].ToString().Substring(0, 1) == "A")
		//	{
		//		continue;
		//	}

		//	if (tpssm12_2["CHARGE_NO"].ToDecimal() <= tpssm11_2["CURR_WP_NO"].ToDecimal())
		//	{
		//		//当前工序已经生产结束，保留该精炼
		//	}
		//	else
		//	{
		//		if (tpssm12_2["CHARGE_NO"].ToDecimal() > 80)
		//		{
		//			//走到这里，说明之前没走过的精炼都已经更新过
		//			//更新连铸机CHARGE NO
		//			//获得连铸机当前工序号，然后每次向后加1
		//			switch (conn->DatabaseKind)
		//			{
		//			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//			case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//			case DB_KIND_ORACLE:	        // Oracle 数据库
		//			default: // 所有数据库适用，通用SQL语句
		//				sqlstr = "UPDATE TPSSM12 "
		//					"	SET	CHARGE_NO = @cc_charge_no_2-@sr2+ @tpssm12_2.charge_no-80 "
		//					" WHERE FACTORY_DIV = @factory_div "
		//					"	AND SM_PLAN_NO = @sm_plan_no "
		//					"	AND AREA_ID =5 ";
		//				break;
		//			}

		//			CDbCommand cmd_upd14(sqlstr, conn);
		//			cmd_upd14.Parameters.Set("cc_charge_no_2", cc_charge_no_2);
		//			cmd_upd14.Parameters.Set("sr2", sr2);
		//			cmd_upd14.Parameters.Set("tpssm12_2.charge_no", tpssm12_2["CHARGE_NO"].ToDecimal());
		//			cmd_upd14.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//			cmd_upd14.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//			cmd_upd14.ExecuteNonQuery();

		//			switch (conn->DatabaseKind)
		//			{
		//			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//			case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//			case DB_KIND_ORACLE:	        // Oracle 数据库
		//			default: // 所有数据库适用，通用SQL语句
		//				sqlstr = "UPDATE TPSSM12 "
		//					"	SET	CHARGE_NO = @cc_charge_no_2-@sr2+ @tpssm12_2.charge_no-80 -1 "
		//					" WHERE FACTORY_DIV = @factory_div "
		//					"	AND SM_PLAN_NO = @sm_plan_no "
		//					"	AND    charge_no  =@charge_no ";
		//				break;
		//			}

		//			CDbCommand cmd_upd15(sqlstr, conn);
		//			cmd_upd15.Parameters.Set("cc_charge_no_2", cc_charge_no_2);
		//			cmd_upd15.Parameters.Set("sr2", sr2);
		//			cmd_upd15.Parameters.Set("tpssm12_2.charge_no", tpssm12_2["CHARGE_NO"].ToDecimal());
		//			cmd_upd15.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//			cmd_upd15.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//			cmd_upd15.Parameters.Set("charge_no", tpssm12_2["CHARGE_NO"].ToDecimal());
		//			cmd_upd15.ExecuteNonQuery();


		//		}
		//		else
		//		{
		//			//该精炼还未生产，则将其转给计划2

		//			sr2++;
		//			switch (conn->DatabaseKind)
		//			{
		//			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//			case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//			case DB_KIND_ORACLE:	        // Oracle 数据库
		//			default: // 所有数据库适用，通用SQL语句
		//				sqlstr = "UPDATE TPSSM12 "
		//					"	SET	SM_PLAN_NO = @sm_plan_no, "
		//					"		CHARGE_NO = @sr2+80 "
		//					" WHERE FACTORY_DIV = @factory_div "
		//					"	AND SM_PLAN_NO = @sm_plan_no1 "
		//					"	AND CHARGE_NO =@charge_no ";
		//				break;
		//			}

		//			CDbCommand cmd_upd16(sqlstr, conn);
		//			cmd_upd16.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//			cmd_upd16.Parameters.Set("sr2", sr2);
		//			cmd_upd16.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//			cmd_upd16.Parameters.Set("sm_plan_no1", tpssm11_2["SM_PLAN_NO"].ToString());
		//			cmd_upd16.Parameters.Set("charge_no", tpssm12_2["CHARGE_NO"].ToDecimal());
		//			cmd_upd16.ExecuteNonQuery();


		//		}
		//	}
		//}
		//cmd_tpssm12_inq.Close();



		////修正计划1的精炼CHARGENO
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "SELECT * FROM TPSSM12 "
		//		" WHERE AREA_ID = 4 "
		//		"	AND	SM_PLAN_NO = @sm_plan_no "
		//		"	AND	CHARGE_NO >80 "
		//		" ORDER BY  charge_no ASC ";
		//	break;
		//}


		//cmd_tpssm12_inq.SetCommandText(sqlstr);
		//cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//cmd_tpssm12_inq.ExecuteReader();

		//while (cmd_tpssm12_inq.Read())
		//{
		//	cmd_tpssm12_inq.Fetch(tpssm12_3);

		//	switch (conn->DatabaseKind)
		//	{
		//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//	case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//	case DB_KIND_ORACLE:	        // Oracle 数据库
		//	default: // 所有数据库适用，通用SQL语句
		//		sqlstr = "UPDATE TPSSM12 "
		//			"	SET	CHARGE_NO = @cc_charge_no_1-@sr1+ @tpssm12_3.charge_no-80 "
		//			" WHERE FACTORY_DIV = @factory_div "
		//			"	AND SM_PLAN_NO = @sm_plan_no "
		//			"	AND AREA_ID =5 ";
		//		break;
		//	}

		//	CDbCommand cmd_upd17(sqlstr, conn);
		//	cmd_upd17.Parameters.Set("cc_charge_no_1", cc_charge_no_1);
		//	cmd_upd17.Parameters.Set("sr1", sr1);
		//	cmd_upd17.Parameters.Set("tpssm12_3.charge_no", tpssm12_3["CHARGE_NO"].ToDecimal());
		//	cmd_upd17.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//	cmd_upd17.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//	cmd_upd17.ExecuteNonQuery();

		//	switch (conn->DatabaseKind)
		//	{
		//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//	case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//	case DB_KIND_ORACLE:	        // Oracle 数据库
		//	default: // 所有数据库适用，通用SQL语句
		//		sqlstr = "UPDATE TPSSM12 "
		//			"	SET	CHARGE_NO = @cc_charge_no_1-@sr1+ @tpssm12_3.charge_no-80 -1 "
		//			" WHERE FACTORY_DIV = @factory_div "
		//			"	AND SM_PLAN_NO = @sm_plan_no "
		//			"	AND CHARGE_NO  =@charge_no ";
		//		break;
		//	}

		//	CDbCommand cmd_upd18(sqlstr, conn);
		//	cmd_upd18.Parameters.Set("cc_charge_no_1", cc_charge_no_1);
		//	cmd_upd18.Parameters.Set("sr1", sr1);
		//	cmd_upd18.Parameters.Set("tpssm12_3.charge_no", tpssm12_3["CHARGE_NO"].ToDecimal());
		//	cmd_upd18.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//	cmd_upd18.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//	cmd_upd18.Parameters.Set("charge_no", tpssm12_3["CHARGE_NO"].ToDecimal());
		//	cmd_upd18.ExecuteNonQuery();


		//}
		//cmd_tpssm12_inq.Close();

		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "UPDATE TPSSM12 "
		//		"	SET	CHARGE_NO = @cc_charge_no_1-@sr1+ @sr2 "
		//		" WHERE FACTORY_DIV = @factory_div "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		"	AND AREA_ID =5 ";
		//	break;
		//}

		//CDbCommand cmd_upd19(sqlstr, conn);
		//cmd_upd19.Parameters.Set("cc_charge_no_1", cc_charge_no_1);
		//cmd_upd19.Parameters.Set("sr1", sr1);
		//cmd_upd19.Parameters.Set("sr2", sr2);
		//cmd_upd19.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//cmd_upd19.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		//cmd_upd19.ExecuteNonQuery();

		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default: // 所有数据库适用，通用SQL语句
		//	sqlstr = "UPDATE TPSSM12 "
		//		"	SET CHARGE_NO = @cc_charge_no_2-@sr2+ @sr1 "
		//		" WHERE FACTORY_DIV = @factory_div "
		//		"	AND SM_PLAN_NO = @sm_plan_no "
		//		"	AND AREA_ID =5 ";
		//	break;
		//}

		//CDbCommand cmd_upd20(sqlstr, conn);
		//cmd_upd20.Parameters.Set("cc_charge_no_2", cc_charge_no_2);
		//cmd_upd20.Parameters.Set("sr2", sr2);
		//cmd_upd20.Parameters.Set("sr1", sr1);
		//cmd_upd20.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//cmd_upd20.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
		//cmd_upd20.ExecuteNonQuery();


		//连铸
		//直接交换 ,最好是计划号交换
		//分别读取两个计划的连铸信息
		tpssm12_51["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm12_51["SM_PLAN_NO"] = tpssm11_1["SM_PLAN_NO"];
		tpssm12_51["AREA_ID"] = 5;
		tpssm12_51.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");


		tpssm12_52["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm12_52["SM_PLAN_NO"] = tpssm11_2["SM_PLAN_NO"];
		tpssm12_52["AREA_ID"] = 5;
		tpssm12_52.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "UPDATE TPSSM12 "
				"	SET CHARGE_NO = CHARGE_NO + 80 "
				" WHERE FACTORY_DIV = @factory_div "
				"	AND SM_PLAN_NO = @sm_plan_no "
				"	AND AREA_ID =5 ";
			break;
		}

		CDbCommand cmd_upd19(sqlstr, conn);
		cmd_upd19.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd19.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		cmd_upd19.ExecuteNonQuery();
		
		Log::Trace("", __FUNCTION__, "change_no=[{0}]", change_no);
		if (change_no == 0)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = "UPDATE TPSSM12 "
					"	SET	DEV_CODE = @dev_code, "
					"		PRE_PROC_NO = @pre_proc_no, "
					"		PROC_NO = @proc_no, "
					"		START_TIME = @start_time, "
					"		END_TIME = @end_time, "
					"		PROC_TIME = @proc_time, "
					"		LADLE_ARRIVE_TIME = @ladle_arrive_time, "
					"		LADLE_LEAVE_TIME = @ladle_leave_time, "
					"		CHARGE_NO = @charge_no "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no "
					"	AND AREA_ID =5 ";
				break;
			}

			CDbCommand cmd_upd21(sqlstr, conn);
			cmd_upd21.Parameters.Set("dev_code", tpssm12_52["DEV_CODE"].ToString());
			cmd_upd21.Parameters.Set("pre_proc_no", tpssm12_52["PRE_PROC_NO"].ToString());
			cmd_upd21.Parameters.Set("proc_no", tpssm12_52["PROC_NO"].ToString());
			cmd_upd21.Parameters.Set("start_time", tpssm12_52["START_TIME"].ToString());
			cmd_upd21.Parameters.Set("end_time", tpssm12_52["END_TIME"].ToString());
			cmd_upd21.Parameters.Set("proc_time", tpssm12_52["PROC_TIME"].ToDecimal());
			cmd_upd21.Parameters.Set("ladle_arrive_time", tpssm12_52["LADLE_ARRIVE_TIME"].ToString());
			cmd_upd21.Parameters.Set("ladle_leave_time", tpssm12_52["LADLE_LEAVE_TIME"].ToString());
			cmd_upd21.Parameters.Set("charge_no", tpssm12_51["CHARGE_NO"].ToDecimal());
			cmd_upd21.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd21.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd21.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = "UPDATE TPSSM12 "
					"	SET	DEV_CODE = @dev_code, "
					"		PRE_PROC_NO = @pre_proc_no, "
					"		PROC_NO = @proc_no, "
					"		START_TIME = @start_time, "
					"		END_TIME = @end_time, "
					"		PROC_TIME = @proc_time, "
					"		LADLE_ARRIVE_TIME = @ladle_arrive_time, "
					"		LADLE_LEAVE_TIME = @ladle_leave_time, "
					"		CHARGE_NO = @charge_no "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no "
					"	AND AREA_ID =5 ";
				break;
			}

			CDbCommand cmd_upd22(sqlstr, conn);
			cmd_upd22.Parameters.Set("dev_code", tpssm12_51["DEV_CODE"].ToString());
			cmd_upd22.Parameters.Set("pre_proc_no", tpssm12_51["PRE_PROC_NO"].ToString());
			cmd_upd22.Parameters.Set("proc_no", tpssm12_51["PROC_NO"].ToString());
			cmd_upd22.Parameters.Set("start_time", tpssm12_51["START_TIME"].ToString());
			cmd_upd22.Parameters.Set("end_time", tpssm12_51["END_TIME"].ToString());
			cmd_upd22.Parameters.Set("proc_time", tpssm12_51["PROC_TIME"].ToDecimal());
			cmd_upd22.Parameters.Set("ladle_arrive_time", tpssm12_51["LADLE_ARRIVE_TIME"].ToString());
			cmd_upd22.Parameters.Set("ladle_leave_time", tpssm12_51["LADLE_LEAVE_TIME"].ToString());
			cmd_upd22.Parameters.Set("charge_no", tpssm12_52["CHARGE_NO"].ToDecimal());
			cmd_upd22.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd22.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd22.ExecuteNonQuery();
		}

		if (change_no == 1)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = "UPDATE TPSSM12 "
					"	SET	DEV_CODE = @dev_code, "
					"		PRE_PROC_NO = @pre_proc_no, "
					"		PROC_NO = @proc_no, "
					"		START_TIME = @start_time, "
					"		END_TIME = @end_time, "
					"		PROC_TIME = @proc_time, "
					"		LADLE_ARRIVE_TIME = @ladle_arrive_time, "
					"		LADLE_LEAVE_TIME = @ladle_leave_time, "
					"		CHARGE_NO = @charge_no "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no "
					"	AND AREA_ID =5 ";
				break;
			}

			CDbCommand cmd_upd21(sqlstr, conn);
			cmd_upd21.Parameters.Set("dev_code", tpssm12_52["DEV_CODE"].ToString());
			cmd_upd21.Parameters.Set("pre_proc_no", tpssm12_52["PRE_PROC_NO"].ToString());
			cmd_upd21.Parameters.Set("proc_no", tpssm12_52["PROC_NO"].ToString());
			cmd_upd21.Parameters.Set("start_time", tpssm12_52["START_TIME"].ToString());
			cmd_upd21.Parameters.Set("end_time", tpssm12_52["END_TIME"].ToString());
			cmd_upd21.Parameters.Set("proc_time", tpssm12_52["PROC_TIME"].ToDecimal());
			cmd_upd21.Parameters.Set("ladle_arrive_time", tpssm12_52["LADLE_ARRIVE_TIME"].ToString());
			cmd_upd21.Parameters.Set("ladle_leave_time", tpssm12_52["LADLE_LEAVE_TIME"].ToString());
			cmd_upd21.Parameters.Set("charge_no", tpssm12_51["CHARGE_NO"].ToDecimal());
			cmd_upd21.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd21.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
			cmd_upd21.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = "UPDATE TPSSM12 "
					"	SET	DEV_CODE = @dev_code, "
					"		PRE_PROC_NO = @pre_proc_no, "
					"		PROC_NO = @proc_no, "
					"		START_TIME = @start_time, "
					"		END_TIME = @end_time, "
					"		PROC_TIME = @proc_time, "
					"		LADLE_ARRIVE_TIME = @ladle_arrive_time, "
					"		LADLE_LEAVE_TIME = @ladle_leave_time, "
					"		CHARGE_NO = @charge_no "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND SM_PLAN_NO = @sm_plan_no "
					"	AND AREA_ID =5 ";
				break;
			}

			CDbCommand cmd_upd22(sqlstr, conn);
			cmd_upd22.Parameters.Set("dev_code", tpssm12_51["DEV_CODE"].ToString());
			cmd_upd22.Parameters.Set("pre_proc_no", tpssm12_51["PRE_PROC_NO"].ToString());
			cmd_upd22.Parameters.Set("proc_no", tpssm12_51["PROC_NO"].ToString());
			cmd_upd22.Parameters.Set("start_time", tpssm12_51["START_TIME"].ToString());
			cmd_upd22.Parameters.Set("end_time", tpssm12_51["END_TIME"].ToString());
			cmd_upd22.Parameters.Set("proc_time", tpssm12_51["PROC_TIME"].ToDecimal());
			cmd_upd22.Parameters.Set("ladle_arrive_time", tpssm12_51["LADLE_ARRIVE_TIME"].ToString());
			cmd_upd22.Parameters.Set("ladle_leave_time", tpssm12_51["LADLE_LEAVE_TIME"].ToString());
			cmd_upd22.Parameters.Set("charge_no", tpssm12_52["CHARGE_NO"].ToDecimal());
			cmd_upd22.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd22.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());
			cmd_upd22.ExecuteNonQuery();
		}

		//交换完成后，需要重新计算精炼路径和钢区工艺途径 
		//重新计算钢区工艺途径，和精炼路径

		//计划1
		backlog_ea = "";
		refine_code = "";
		Log::Trace("", __FUNCTION__, "tpssm11_1.sm_plan_no=[{0}],m=[{1}]", tpssm11_1["SM_PLAN_NO"].ToString(), tpssm11["FACTORY_DIV"].ToString());

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "SELECT * FROM   TPSSM12  "
				" WHERE FACTORY_DIV = @factory_div "
				"	AND SM_PLAN_NO = @sm_plan_no ";
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());
		cmd_tpssm12_inq.ExecuteReader();

		while (cmd_tpssm12_inq.Read())
		{
			cmd_tpssm12_inq.Fetch(tpssm12);
			tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
			tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];

			tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
			backlog_ea = backlog_ea  + tpssmd1["DEV_CODE"].ToString();
			if (tpssm12["AREA_ID"].ToDecimal() == 4)
			{
				refine_code = refine_code + tpssmd1["DEV_CODE"].ToString();//upd by 180502
			}

		}
		cmd_tpssm12_inq.Close();

		if (strcmp(refine_code, "") == 0)
		{
			refine_code = " ";
		}
		Log::Trace("", __FUNCTION__, "backlog_ea=[{0}]", backlog_ea);
		Log::Trace("", __FUNCTION__, "refine_code=[{0}]", refine_code);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "UPDATE TPSSM11 "
				" SET BACKLOG_EA = @backlog_ea, "
				" REFINE_ROUTE_CODE = @refine_code "
				" WHERE FACTORY_DIV = @factory_div "
				" AND SM_PLAN_NO = @sm_plan_no ";
			break;
		}

		CDbCommand cmd_upd23(sqlstr, conn);
		cmd_upd23.Parameters.Set("backlog_ea", backlog_ea);
		cmd_upd23.Parameters.Set("refine_code", refine_code);
		cmd_upd23.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd23.Parameters.Set("sm_plan_no", tpssm11_1["SM_PLAN_NO"].ToString());

		cmd_upd23.ExecuteNonQuery();

		//计划2
		backlog_ea = "";
		refine_code = "";
		Log::Trace("", __FUNCTION__, "tpssm11_2.sm_plan_no=[{0}],m=[{1}]", tpssm11_2["SM_PLAN_NO"].ToString(), tpssm11["FACTORY_DIV"].ToString());

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "SELECT * FROM   TPSSM12  "
				" WHERE FACTORY_DIV = @factory_div "
				"	AND SM_PLAN_NO = @sm_plan_no ";
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());

		cmd_tpssm12_inq.ExecuteReader();


		while (cmd_tpssm12_inq.Read())
		{
			cmd_tpssm12_inq.Fetch(tpssm12);
			tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
			tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
			tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];

			tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
			backlog_ea = backlog_ea+ tpssmd1["DEV_CODE"].ToString();
			if (tpssm12["AREA_ID"].ToDecimal() == 4)
			{
				refine_code = refine_code + tpssmd1["DEV_CODE"].ToString();//upd by 180502
			}

		}
		cmd_tpssm12_inq.Close();
		if (strcmp(refine_code, "") == 0)
		{
			refine_code = " ";
		}
		Log::Trace("", __FUNCTION__, "backlog_ea=[{0}]", backlog_ea);
		Log::Trace("", __FUNCTION__, "refine_code=[{0}]", refine_code);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "UPDATE TPSSM11 "
				"	SET	BACKLOG_EA = @backlog_ea, "
				"		REFINE_ROUTE_CODE =	@refine_code "
				" WHERE FACTORY_DIV = @factory_div "
				"	AND SM_PLAN_NO	= @sm_plan_no ";
			break;
		}

		CDbCommand cmd_upd24(sqlstr, conn);
		cmd_upd24.Parameters.Set("backlog_ea", backlog_ea);
		cmd_upd24.Parameters.Set("refine_code", refine_code);
		cmd_upd24.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd24.Parameters.Set("sm_plan_no", tpssm11_2["SM_PLAN_NO"].ToString());

		cmd_upd24.ExecuteNonQuery();

		Log::Trace("", __FUNCTION__, "更新11表");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DISTINCT AREA_ID, DEV_CODE  FROM TPSSM12 \
						WHERE SM_PLAN_NO	= @tpssm11_1.SM_PLAN_NO   \
						AND CHARGE_NO		= @tpssm11_1.CURR_WP_NO ";
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("tpssm11_1.SM_PLAN_NO", tpssm11_1["SM_PLAN_NO"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm11_1.CURR_WP_NO", tpssm11_1["CURR_WP_NO"].ToDecimal());
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
			tpssmd1["AREA_ID"] = cmd_tpssm12_inq.GetDecimal(1);
			tpssmd1["DEV_CODE"] = cmd_tpssm12_inq.GetString(2);
		}
		cmd_tpssm12_inq.Close();

		//更新12表的熔炼号
		tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm12["SM_PLAN_NO"] = tpssm11_1["SM_PLAN_NO"];
		tpssm12["HEAT_NO"] = tpssm11_1["HEAT_NO"];
		tpssm12.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");

		tpssm12["SM_PLAN_NO"] = tpssm11_2["SM_PLAN_NO"];
		tpssm12["HEAT_NO"] = tpssm11_2["HEAT_NO"];
		tpssm12.Update("HEAT_NO", "FACTORY_DIV,SM_PLAN_NO");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DISTINCT STATION_ID FROM TPSSMD1 \
						WHERE AREA_ID  = @tpssmd1.AREA_ID \
						AND DEV_CODE = @tpssmd1.DEV_CODE ";
			break;
		}

		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
		cmd_tpssmd1_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		if (cmd_tpssmd1_inq.Read())
		{
			station_id_out = cmd_tpssmd1_inq.GetString(1);
			station_id_out = station_id_out.TrimOrBlank();
		}
		cmd_tpssmd1_inq.Close();
		Log::Trace("", __FUNCTION__, "查询");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DISTINCT AREA_ID, DEV_CODE FROM TPSSM12 \
						WHERE SM_PLAN_NO	= @tpssm11_2.SM_PLAN_NO   \
						AND CHARGE_NO		= @tpssm11_2.CURR_WP_NO ";
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("tpssm11_2.SM_PLAN_NO", tpssm11_2["SM_PLAN_NO"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm11_2.CURR_WP_NO", tpssm11_2["CURR_WP_NO"].ToDecimal());
		cmd_tpssm12_inq.ExecuteReader();
		if (cmd_tpssm12_inq.Read())
		{
			tpssmd1["AREA_ID"] = cmd_tpssm12_inq.GetDecimal(1);
			tpssmd1["DEV_CODE"] = cmd_tpssm12_inq.GetString(2);
		}
		cmd_tpssm12_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DISTINCT STATION_ID FROM TPSSMD1 \
						WHERE AREA_ID	= @tpssmd1.AREA_ID \
						AND DEV_CODE	= @tpssmd1.DEV_CODE ";
			break;
		}

		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssmd1.AREA_ID", tpssmd1["AREA_ID"].ToDecimal());
		cmd_tpssmd1_inq.Parameters.Set("tpssmd1.DEV_CODE", tpssmd1["DEV_CODE"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		if (cmd_tpssmd1_inq.Read())
		{
			station_id_in = cmd_tpssmd1_inq.GetString(1);
			station_id_in = station_id_in.TrimOrBlank();
		}
		cmd_tpssmd1_inq.Close();

		Log::Trace("", __FUNCTION__, "调用接口");
		//通知MM
		//通知L2
#ifdef _SYS_PES
		
		////MMS接口	
		//CDataRow &row2 = inBlock2.Tables["X200008"].Rows.Add();
		//row2["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		//row2["PONO_OLD"] = tpssm11_1["PONO"];
		//row2["PONO_NEW"] = tpssm11_2["PONO"];
		////发送MMS电文
		//ret = f_cm_pam1p3_snd(&inBlock2, &outBlock1, conn);
		//if (ret != 0)
		//{
		//	Log::Trace("", __FUNCTION__, "{0}. f_cm_pam1p3_snd()调用出错.", s.msg);
		//	strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
		CDataRow &row1 = inBlock1.Tables[0].Rows.Add();
		row1["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		row1["PONO_OLD"] = tpssm11_1["PONO"];
		row1["PONO_NEW"] = tpssm11_2["PONO"];
		//ret = f_cm_eecm63_snd(&inBlock1, &outBlock, conn);
		if (ret != 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//发送炼钢L2电文
		CDataRow &row6 = inBlock6.Tables["XX1L263"].Rows.Add();
		row6["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		row6["PONO_OLD"] = tpssm11_1["PONO"];
		row6["PONO_NEW"] = tpssm11_2["PONO"];
		row6["OPER_FLAG"] = "2";
		//ret = f_cm_x1l263_snd(&inBlock6, &outBlock1, conn);
		//if (ret != 0)
		//{
		//	Log::Trace("", __FUNCTION__, "{0}. f_cm_x1l263_snd()调用出错.", s.msg);
		//	strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//row6["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		//row6["PONO_OLD"] = tpssm11_2["PONO"];
		//row6["PONO_NEW"] = tpssm11_1["PONO"];
		//row6["OPER_FLAG"] = "2";
		//ret = f_cm_x1l263_snd(&inBlock6, &outBlock1, conn);
		//if (ret != 0)
		//{
		//	Log::Trace("", __FUNCTION__, "{0}. f_cm_x1l263_snd()调用出错.", s.msg);
		//	strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}


#endif		
		////发送炼钢L2电文
		//CDataRow &row6 = inBlock6.Tables["PONO_CHG_2"].Rows.Add();
		//row6["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		//row6["PONO_OLD"] = tpssm11_1["PONO"];
		//row6["PONO_NEW"] = tpssm11_2["PONO"];
		//row6["OPER_FLAG"] = "2";
		//ret = f_pssm_pas1p3_snd(&inBlock6, &outBlock1, conn);
		//if (ret != 0)
		//{
		//	Log::Trace("", __FUNCTION__, "{0}. f_pssm_pas1p3_snd()调用出错.", s.msg);
		//	strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		CDataRow &row3 = inBlock3.Tables["MMSM0055"].Rows.Add();
		row3["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		row3["PONO_OUT"] = tpssm11_1["PONO"];
		row3["ST_NO_OUT"] = tpssm11_1["ST_NO"];
		row3["PONO_IN"] = tpssm11_2["PONO"];
		row3["ST_NO_IN"] = tpssm11_2["ST_NO"];
		//实绩模块函数接口函数
		ret = f_mmsm0055_proc(&inBlock3, &outBlock1, conn);
		if (ret < 0)
		{
			Log::Trace("", __FUNCTION__, "{0}. f_mmsm0055_proc()调用出错.", s.msg);
			strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//调用质量模块接口
		CDataRow &row4 = inBlock4.Tables["STNO_CHG"].Rows.Add();
		row4["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		row4["PONO_OUT"] = tpssm11_1["PONO"];
		row4["ST_NO_OUT"] = tpssm11_1["ST_NO"];
		row4["PONO_IN"] = tpssm11_2["PONO"];
		row4["ST_NO_IN"] = tpssm11_2["ST_NO"];
		row4["WHOLE_BACKLOG_CODE_OUT"] = station_id_out;
		row4["WHOLE_BACKLOG_CODE_IN"] = station_id_in;

		////质量模块函数接口
		ret = f_qmts_stno_chgd(&inBlock4, &outBlock1, conn);
		if (ret < 0)
		{
			//outBlock1.GetSYS(&s);
			Log::Trace("", __FUNCTION__, "{0}. f_qmts_stno_chgd()调用出错.", s.msg);
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			//throw CApplicationException(-1, s.msg, log.Location);
			doFlag = 0;
			s.flag = 0;
		}

		Log::Trace("", __FUNCTION__, "调用炼钢计划履历函数");


		//重新排转炉
		Log::Trace("", __FUNCTION__, "重新排转炉");
		//ret = f_pssm_reranking(&inBlock8, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


		Log::Trace("", __FUNCTION__, "调用函数f_cm_x1l262_snd开始---准备下发L2出钢计划和铸坯命令");

		inBlock8.Tables["XX1L262"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		inBlock8.Tables["XX1L262"].Rows[0]["OPER_FLAG"] = "1";//新增

		//ret = f_cm_x1l262_snd(&inBlock8, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//1-2
		//写履历表----dclian---add----2015-11-20
		//针对被交换炉次PONO1,1-->2
		tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = tpssm11_1["PONO"];
		tpssm99["HEAT_NO"] = tpssm11_1["HEAT_NO"];
		tpssm99["PONO_STATUS"] = tpssm11_1["PONO_STATUS"];
		tpssm99["ST_NO"] = tpssm11_1["ST_NO"];
		tpssm99["EVENT_ID"] = "E1";
		tpssm99["PONO_OLD"] = tpssm11_2["PONO"];
		tpssm99["HEAT_NO_OLD"] = tpssm11_2["HEAT_NO"];
		tpssm99["ST_NO_OLD"] = tpssm11_2["ST_NO"];
		tpssm99["VALID_FLAG"] = "1";
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		
		//针对交换炉次PONO2,2-->1
		tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = tpssm11_2["PONO"];
		tpssm99["HEAT_NO"] = tpssm11_2["HEAT_NO"];
		tpssm99["PONO_STATUS"] = tpssm11_2["PONO_STATUS"];
		tpssm99["ST_NO"] = tpssm11_2["ST_NO"];
		tpssm99["EVENT_ID"] = "E1";
		tpssm99["VALID_FLAG"] = "1";
		tpssm99["PONO_OLD"] = tpssm11_1["PONO"];
		tpssm99["HEAT_NO_OLD"] = tpssm11_1["HEAT_NO"];
		tpssm99["ST_NO_OLD"] = tpssm11_1["ST_NO"];
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		Log::Trace("", __FUNCTION__, "事件号tpssm99.EVENT_ID =[{0}]", tpssm99["EVENT_ID"].ToString());
		Log::Trace("", __FUNCTION__, "记录履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		//记录履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		
	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{

		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = -1;
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}

