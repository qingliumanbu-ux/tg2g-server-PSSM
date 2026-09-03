/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-18
Description: 连铸浇铸计划查询
**************************************************/
#include "stdafx.h"

#include "tpssmd1.h"
#include "tpssm10.h"
#include "tpssm11.h"


/*<remark>=========================================================
/// <summary>
/// 连铸浇铸计划查询
/// <para>1.根据传入的炼钢厂别和连铸机号，查询当前连铸机在线的浇注作业计划。</para>
/// <para>2.查询条件：炉次状态大于15（制造命令状态 = 16 计划下达），
/// 小于（83制造命令状态 = 83 炉次浇铸完了），从tpssm10表搜索对应的炉次信息； </para>
/// <para>3.排序方式：cc_seq ASC；                                  </para>
/// <para>4.查询该PONO下的板坯规格信息：厚、宽、长；                </para>
/// <para>数据库表：TPSSM11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：前台PSSM11画面F3(计划新增)调用。              </para>
/// </summary>
/// <param name="cc_mach_no">连铸机号                </param>
/// <returns>指定连铸机下的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11u_inq)


int f_pssm11u_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blknum;

	/* 业务变量 */
	CString	 v_factory_div = "";	//炼钢厂别
	CString  v_cc_mach_no = "";
	CString  carry_div = "";
	CString  cast_lot_no = "";  //记录上一炉次的LOT，用于规格查询
	CDecimal wdchg_count = 0;   //调宽回数
	CDecimal slab_width_min = 0;   //最小宽
	CDecimal slab_width_max = 0;   //最小宽

	CString sqlstr = "";

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);

	try
	{
		// 定义表的实体对象
		CTPSSM10 tpssm10(conn);
		CTPSSM11 tpssm11(conn);
		CTPSSMD1 tpssmd1(conn);

		//--------------------------------
		//设定返回块信息结构（模板）
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("TPSSM10");
		CDataTable * ptable = &(bcls_ret->Tables[blknum]);  //定义表指针，用于模板复制
		bcls_ret->Tables[blknum].Columns.Add(tpssm10);   //从实体对象创建架构
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CAST_SHOW");  //CAST号，显示用
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "LOT_SHOW");   //浇次号，显示用
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SLAB_THICK_SHOW");  //铸坯厚度
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "WDCHG_COU_ODD");   //奇流调宽回数
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "WDCHG_COU_EVEN");  //偶流调宽回数
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SLAB_WIDTH_ODD");  //奇流宽(mm)
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SLAB_WIDTH_EVEN"); //偶流宽(mm)

		//返回参数表
		//bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SLAB_WIDTH_1");   //1流板坯宽度
		//bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SLAB_WIDTH_2");   //2流板坯宽度

		//---------------------------------------------------
		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		v_cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"];
		Log::Info("", __FUNCTION__, "cc_mach_no=[{1}]", v_cc_mach_no);

		//---------------------------------------------------
		//查询对应铸机下的浇铸信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT b.CAST_NO, b.CAST_DIV_NO, a.* FROM TPSSM10 a left join TPSSM11 b "
				"    ON( a.pono = b.pono )"
				"  WHERE a.FACTORY_DIV = @FACTORY_DIV "
				"	 AND a.CC_MACH_NO = @CC_MACH_NO "
				"    AND a.PONO_STATUS < 83 "  //浇注完毕不显示
				" ORDER BY a.CC_MACH_NO ASC, a.CC_SEQ ASC, a.CAST_NO ASC, a.CAST_DIV_NO ASC "
				);
			break;
		}
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.Parameters.Set("CC_MACH_NO", v_cc_mach_no);
		cmd_tpssm10_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
		cmd_tpssm10_inq.ExecuteReader();
		while (cmd_tpssm10_inq.Read())
		{
			tpssm11.CAST_NO = cmd_tpssm10_inq.GetString(1);
			tpssm11.CAST_DIV_NO = cmd_tpssm10_inq.GetDecimal(2);
			cmd_tpssm10_inq.Fetch(tpssm10, 3);

			//CDataRow & row = bcls_ret->Tables[tabname].Rows.Add();
			CDataRow & row = bcls_ret->Tables["TPSSM10"].Rows.Add();
			row.Merge(tpssm10);

			//去小数
			tpssm10.SLAB_THICK = tpssm10.SLAB_THICK.ToInt32();
			row["SLAB_THICK"] = tpssm10.SLAB_THICK.ToString();

			if (tpssm11.CAST_NO.Trim() == "")
			{
				row["CAST_SHOW"] = "";
			}
			else
			{
				row["CAST_SHOW"] = tpssm11.CAST_NO.Trim() + "-" + tpssm11.CAST_DIV_NO.ToString();
			}
			row["LOT_SHOW"] = tpssm10.CAST_LOT_NO.Trim() + "-" + tpssm10.CAST_LOT_DIV_NO.ToString();

			//快换中包
			row["TD_CHG_FLAG"] = tpssm10.TD_CHG_FLG;

			//规格数据
			//tpssm10.SLAB_THICK = tpssm10.SLAB_THICK.ToInt32();
			//tpssm10.SLAB_WIDTH = tpssm10.SLAB_WIDTH.ToInt32();
			//tpssm10.SLAB_LEN   = tpssm10.SLAB_LEN.ToInt32();
			//row["SLAB_SPEC"] = tpssm10.SLAB_THICK.ToString() + "*" + tpssm10.SLAB_WIDTH.ToString() + "*" + tpssm10.SLAB_LEN.ToString();
			if (cast_lot_no == tpssm10.CAST_LOT_NO.Trim())
			{
			}
			else  //不同LOT时，查询炉次的板坯规格
			{
				//查询奇流板坯数据
				sqlstr = CString(
					"SELECT min(PLAN_SLAB_WIDTH), max(PLAN_SLAB_WIDTH), count(*) FROM ( "
					"  SELECT PLAN_SLAB_WIDTH FROM TPSSM03 "
					"   WHERE CAST_LOT_NO = @cast_lot_no "
					"     AND mod(Integer(STRAND_NO), 2) = 1 "  //奇流
					"   GROUP BY PLAN_SLAB_WIDTH "
					" ) "
					);
				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("cast_lot_no", tpssm10.CAST_LOT_NO.Trim());
				cmd_tpssm03_inq.ExecuteReader();
				if (cmd_tpssm03_inq.Read())
				{
					slab_width_min = cmd_tpssm03_inq.GetDecimal(1).ToInt32();
					slab_width_max = cmd_tpssm03_inq.GetDecimal(2).ToInt32();
					wdchg_count = cmd_tpssm03_inq.GetDecimal(3).ToInt32() - 1; //回数
				}
				cmd_tpssm03_inq.Close();

				if (wdchg_count >= 0)
				{
					row["WDCHG_COU_ODD"] = wdchg_count.ToString(); //奇流调宽回数
					row["SLAB_WIDTH_ODD"] = slab_width_max.ToString() + "-" + slab_width_min.ToString(); //奇流调宽回数
				}

				//查询偶流板坯数据
				sqlstr = CString(
					"SELECT min(PLAN_SLAB_WIDTH), max(PLAN_SLAB_WIDTH), count(*) FROM ( "
					"  SELECT PLAN_SLAB_WIDTH FROM TPSSM03 "
					"   WHERE CAST_LOT_NO = @cast_lot_no "
					"     AND mod(Integer(STRAND_NO), 2) = 0 "  //偶流
					"   GROUP BY PLAN_SLAB_WIDTH "
					" ) "
					);
				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("cast_lot_no", tpssm10.CAST_LOT_NO.Trim());
				cmd_tpssm03_inq.ExecuteReader();
				if (cmd_tpssm03_inq.Read())
				{
					slab_width_min = cmd_tpssm03_inq.GetDecimal(1);
					slab_width_max = cmd_tpssm03_inq.GetDecimal(2);
					wdchg_count = cmd_tpssm03_inq.GetDecimal(3) - 1; //回数
				}
				cmd_tpssm03_inq.Close();

				if (wdchg_count >= 0)
				{
					row["WDCHG_COU_EVEN"] = wdchg_count.ToString(); //偶流调宽回数
					row["SLAB_WIDTH_EVEN"] = slab_width_max.ToString() + "-" + slab_width_min.ToString(); //偶流调宽回数
				}


			}//if 不同LOT


			//-----------------------------------------
			//根据“热送标记”和“热装标记”形成“搬送区分”
			//搬送区分:1:HCR;2:HDR;3:DHCR;4:CCR1;5:CCR2
			/* 热送标记: 0—板坯计划下线;  1—板坯计划热送;  2—板坯必须热送
			热装标记: 0—冷装;  1—热装;  2—保温坑热装;  3—直接轧制
			*/
			//if (tpssm10.HOT_SEND_FLAG.Compare("0") == 0)
			//{
			//	//CCR1(板坯冷装方式入炼钢板坯库)（热送区分“0”）
			//	carry_div = "CCR1";
			//}
			//else
			//{
			//	if (tpssm10.HOT_CHARGE_FLAG.Compare("0") == 0)
			//	{
			//		//CCR2(板坯冷装方式入轧钢板坯库)（热送区分“1”或“2”+热装标记“0”）
			//		carry_div = "CCR2";
			//	}
			//	else if (tpssm10.HOT_CHARGE_FLAG.Compare("1") == 0)
			//	{
			//		//HCR(板坯保温装炉)（热送区分“1”或“2”+热装标志“1”）
			//		carry_div = "HCR";
			//	}
			//	else if (tpssm10.HOT_CHARGE_FLAG.Compare("2") == 0)
			//	{
			//		//DHCR(热坯直接装炉)（热送区分“1”或“2”+热装标志“2”）
			//		carry_div = "DHCR";
			//	}
			//	else if (tpssm10.HOT_CHARGE_FLAG.Compare("3") == 0)
			//	{
			//		//HDR(直接热轧) （热送区分“1”或“2”+热装标志“3”）
			//		carry_div = "HDR";
			//	}

			//}
			//row["CARRY_DIV"] = carry_div;

			//Log::Trace("", __FUNCTION__, "pono=[{0}], carry_div=[{1}]", tpssm10.PONO, carry_div);


			cast_lot_no = tpssm10.CAST_LOT_NO.Trim(); //记录

		}//while
		cmd_tpssm10_inq.Close();  //DB2下一定要Close。否则报CLI0115E  Invalid cursor state. SQLSTATE=24000 


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