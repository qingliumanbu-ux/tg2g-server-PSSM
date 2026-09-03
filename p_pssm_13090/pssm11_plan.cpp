/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Date:     2023-6-1
Version:  3.1.0
Description: 表单出钢计划编制
 参考pssm18_save（排入计划） 和 pssm18_create（模型优化）结合
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

//#include "tpssmsg.h"
/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author: 172125
Date: 2023/6/1 11:16:45
Description: 计划单例[TPSSMSG]
//计划单例 保存设备代码、设备传搁时间等配置
//跟踪在线计划TPSSM11
//生产在线跟踪TPSSM33
**************************************************/
#ifndef _TPSSMSG_H
#define _TPSSMSG_H

#if !defined (BM2_LACKS_PRAGMA_ONCE)
# pragma once
#endif

class TpssmSg
{
private:
	//CDbConnection * _conn;          // 数据库联接

public:
	CString REC_ID = "123";   //记录ID
	EIClass bcls_pst;
	map<CString, int> dev_map;
	map<CString, int> dev_tech_map;

	//--------------单例----------------------
private:
	TpssmSg(){ };
	//TpssmSg(CDbConnection* conn);
	~TpssmSg(){ };
	TpssmSg(const TpssmSg&);
	TpssmSg& operator=(const TpssmSg&);
public:
	static TpssmSg& getInstance()
	{
		static TpssmSg instance;
		return instance;
	}
public:

	void Reset(void);

	//void setConn(CDbConnection* conn);

};
#endif
#ifdef _INC_IDENTITY_OBJECT_IMPLEMENT_
//-----------------------------------------------------------------------
//      概述:
//               将字段重置为默认值。
//      说明:
//               
//-----------------------------------------------------------------------

void TpssmSg::Reset(void)
{
	// 对象属性赋默认值
	this->REC_ID = " "; //记录ID

}
#endif


int f_pssm11_ins_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划主表新增，单记录处理
int f_pssm12_save_job_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划之工序计划保存
int f_pssm11_del_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢作业计划删除,标志为“D”的炉次
int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//甘特图炉次生成CAST号

int f_pssm_call_tps_n(CString main_backlog_code, int mode, CDbConnection * conn);//模型优化

int f_pssm11_planno_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);  //出钢计划的计划号生成
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划各工序作业顺序号生成（包括HEAT_NO）
int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
//int f_pssm_pas1p1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//出钢计划发送

/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划编制保存
/// <para>根据Client甘特图输入的数据，修改出钢计划。  </para>
/// <para> 读取输入参数,编制计划数
///   1.出钢计划信息写入:主计划与工序计划
///   2.删除排除的炉次
///   3.CAST号计算
///   4.计划号计算
///   5.处理号计算
/// <para>数据库表：TPSSM11/12                   </para>
/// <para>主调用函数：PSSM18画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>制造命令号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_plan)

int f_pssm11_plan(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	int v_mode_type = -1; //模型优化模式
	CString v_pono = "";
	CString v_restrand_flg = "";     //连浇标记
	CString v_cc_req_time = "";  //开浇时刻
	CString v_tpd_start_time = "";  //倒罐开始时刻
	CDecimal v_td_chg_flg = 0, v_smelt_mode = 0;
	CDecimal charge_no = 0;      //工序charge号
	CString datetime = "";
	
	CString sr_dev_col_name,sr_start_col_name,sr_end_col_name;

	EIClass ccPlanBlk;
	EIClass inblk;        //调用函数用
	EIClass subblk;
	EIClass in_pssm99trace;//调用履历函数
	EIClass in_pssm18;//调用发送停机实绩的函数
	CString sqlstr, sqlstrCount, sqlstr_tpssm12, sqlstr_tpssm35;
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm18_del(conn);
	CDbCommand cmd_tpssm11_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		CModel tpssm10("TPSSM10");
		CModel tpssm11("TPSSM11");
		CModel tpssm12("TPSSM12");
		CModel tpssm12_ds("TPSSM12");//脱硫
		CModel tpssm12_dp("TPSSM12");//转炉脱磷
		CModel tpssm12_bof("TPSSM12");//转炉/电炉工序
		CModel tpssm12_sr("TPSSM12");//精炼
		CModel tpssm12_cc("TPSSM12");//连铸
		CModel tpssm18("TPSSM18");//设备状态
		CModel tpssm99("TPSSM99");
		CModel tpssmd1("TPSSMD1"); //设备代码
		CModel tpssm35("TPSSM35"); //炉次返送
		//--------------------------------
		//定义函数调用信息结构
		//1、总体计划块
		inblk.Tables[0].set_TableName("PLAN");  //
		inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
		inblk.Tables[0].Rows.Add();

		//2、新增炉次块，f_pssm11_ins_heat()新增计划用，单记录
		CDataTable &table = inblk.Tables.Add("PONO");
		table.Columns.Add(DT_DECIMAL, "PLID");       //ID:参与临时计划号计算用
		table.Columns.Add(DT_STRING, "PONO");        //制造命令号
		table.Columns.Add(DT_STRING, "ST_NO");       //钢种
		table.Columns.Add(DT_STRING, "TD_CHG_FLG");    //换中包标记
		table.Columns.Add(DT_STRING, "RESTRAND_FLG");       //重引锭标记
		table.Columns.Add(DT_STRING, "CC_REQ_TIME");   //
		table.Columns.Add(DT_DECIMAL, "SMELT_MODE");   //吹炼方式
		table.Columns.Add(DT_STRING, "PONO_STATUS");  //炉次返送用
		table.Columns.Add(DT_STRING, "HEAT_NO");
		table.Columns.Add(DT_STRING, "RUN_STATUS");
		table.Rows.Add();  //只定义一行

		//3、工序计划 按一炉为单位操作
		CDataTable &table1 = inblk.Tables.Add("TPSSM11");  //主计划信息，单记录
		table1.Columns.Add(DT_STRING, "FACTORY_DIV");    //炼钢单元号
		table1.Columns.Add(DT_STRING, "SM_PLAN_NO");    //炼钢计划号
		table1.Columns.Add(DT_STRING, "PONO");          //制造命令号
		CDataTable &table2 = inblk.Tables.Add("TPSSM12");  //子计划信息，多记录
		table2.Columns.Add(DT_DECIMAL, "AREA_ID");      //炼钢区域标识
		table2.Columns.Add(DT_STRING, "DEV_CODE");      //设备代码
		table2.Columns.Add(DT_STRING, "START_TIME");    //开始时刻
		table2.Columns.Add(DT_STRING, "END_TIME");      //结束时刻

		//4、出钢计划下发
		CDataTable &table3 = inblk.Tables.Add("PAS");  //出钢计划下发
		table3.Columns.Add(DT_STRING, "FACTORY_DIV");    //炼钢单元号
		table3.Columns.Add(DT_STRING, "OPER_FLAG");    //操作标志：I：新增，D删除。
		table3.Rows.Add();  //只定义一行


		//5、计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);
		//6.发送停机实绩到铁区
		in_pssm18.Tables[0].set_TableName("TJ");
		/*	in_pssm18.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");*/
		/* -----------输入参数 说明 ---------------------------------
		甘特图是以一条记录将一炉次的所有计划内容传入
		pono
		heat_no
		st_no
		cc_mark
		plan_style
		td_chg_flg
		refine_route_code  精炼路径
		pono_status        PONO状态
		cc_req_time        ??????? 甘特图有指定？
		tpd_id             倒罐
		tpd_start_time     倒罐开始
		tpd_end_time       倒罐结束
		kr_id              脱S 工位
		kr_start_time
		kr_end_time
		ld_1_id            转炉脱P 工位
		ld_1_wait_start_time
		ld_1_end_time
		ld_2_id            转炉脱C/电炉 工位
		ld_2_wait_start_time
		ld_2_end_time
		finery_1_id        精炼1 工位
		finery_1_start_time
		finery_1_end_time");
		finery_2_id        精炼2 工位
		finery_2_start_time
		finery_2_end_time
		finery_3_id        精炼3 工位
		finery_3_start_time
		finery_3_end_time
		finery_4_id        精炼4 工位
		finery_4_start_time
		finery_4_end_time
		cast_1_wait_id     连铸等待工位
		cast_1_wait_start_time
		cast_1_wait_end_time
		cast_1_id          连铸工位
		cast_1_start_time
		cast_1_end_time
		steel_return_code  返送代码
		sg_sign
		*--------------------------------------------------------*/

		sqlstr_tpssm12 = CString(
			" SELECT * FROM TPSSM12 "
			"  WHERE FACTORY_DIV = @tpssm11_FACTORY_DIV "
			"    AND SM_PLAN_NO = @tpssm11_SM_PLAN_NO "
			"    AND SUB_CHARGE_NO = 0 "  //0-主工序. 对甘特图只读取主工序的.
			"  ORDER BY CHARGE_NO ASC "
			);

		//-----------------------------------------------------
		// 读取输入信息，单记录
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in pssm21_save().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"].ToString().Trim();
		v_mode_type = bcls_rec->Tables[blkseq].Rows[0]["MODE_TYPE"].ToDecimal().ToInt32();

		if (blkseq == 0)
		{
			//bcls_rec->Tables[0] 存放组织PONO数据
			bcls_rec->Tables.Add().set_Ordinal(0);
		}

		//准备数据-dev_map
		if (TpssmSg::getInstance().dev_map.empty())
		{
			sqlstr = CString(
				" SELECT nvl(b.STD_PROC_TIME,30) STD_PROC_TIME,a.* FROM TPSSMD1 a,(select max(std_proc_time) std_proc_time, dev_code from TPSSMD3 group by dev_code) b "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND a.DEV_CODE = b.DEV_CODE(+) "
				// "    AND AREA_ID    >= 3 "  //从转炉脱C
				" ORDER BY AREA_ID, STATION_ID, STATION_NO "
				);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				cmd_inq.Fetch(tpssmd1);
				int std_proc_time = cmd_inq.GetInt32(1);
				if (!TpssmSg::getInstance().bcls_pst.Tables.Contains("DEV"))
				{
					CDataTable &tableDev = TpssmSg::getInstance().bcls_pst.Tables.Add("DEV");
					TpssmSg::getInstance().bcls_pst.Tables["DEV"].Columns.Add(tpssmd1);
					TpssmSg::getInstance().bcls_pst.Tables["DEV"].Columns.Add(DT_INT32, "STD_PROC_TIME");
				}
				int mi = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows.get_Count();
				CDataRow &row_dev = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows.Add();
				tpssmd1.MergeTo(row_dev);
				row_dev["STD_PROC_TIME"] = std_proc_time;
				TpssmSg::getInstance().dev_map.insert(make_pair(tpssmd1["DEV_CODE"].ToString(), mi));
				TpssmSg::getInstance().dev_tech_map.insert(make_pair(tpssmd1["DEV_TECH_CODE"].ToString(), mi));
			}
			cmd_inq.Close();
		}
		//--------组织数据 bcls_rec->Tables[0] 存排入的计划（包括原来的)----------
		//1.按铸机 查询TPSSM10表数据
		CDateTime bof1_can_start_dt = CDateTime::Now();
		CDateTime bof2_can_start_dt = bof1_can_start_dt, bof3_can_start_dt = bof1_can_start_dt;
		for (int cci = 0; cci < bcls_rec->Tables[blkseq].Rows.get_Count(); cci++)
		{
			int startWorkPlanRowNum = 0;
			CString cc_math_no = bcls_rec->Tables[blkseq].Rows[cci]["CC_MACH_NO"].ToString().Trim();
			int cc_plan_count = bcls_rec->Tables[blkseq].Rows[cci]["PONO_NUM"].ToDecimal().ToInt32();

			sqlstr = CString(" SELECT row_number() over(ORDER BY CC_SEQ) rowNum, a.*, b.PONO b_PONO ,b.PONO_STATUS b_PONO_STATUS, b.* "
				" FROM TPSSM10 a, TPSSM11 b "
				" WHERE a.FACTORY_DIV = @tpssm10_FACTORY_DIV "
				"  and a.CC_MACH_NO = @tpssm10_CC_MACH_NO"
				"  and a.pono = b.pono(+) "
				"   AND a.PONO_STATUS < 83 "
				" ORDER BY CC_SEQ "
				);
			sqlstrCount = " select max(rowNum) from(" + sqlstr + ") where b_PONO is not null and b_PONO_STATUS >= 20 ";
			cmd_inq.SetCommandText(sqlstrCount);
			cmd_inq.Parameters.Set("tpssm10_FACTORY_DIV", v_factory_div);
			cmd_inq.Parameters.Set("tpssm10_CC_MACH_NO", cc_math_no);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				//plan_num ++;
				startWorkPlanRowNum = cmd_inq.GetInt32(1); //开始生产的计划被排到了第几行
			}
			cmd_inq.Close();

			cmd_inq.SetCommandText(sqlstr);
			//2.比较判断：开始生产的计划是否排到了编排的计划数之后
			if (startWorkPlanRowNum > cc_plan_count && startWorkPlanRowNum > 0)
			{
				//取得 startWorkPlanRowNum 的 PONO号；提示
				cmd_inq.ExecuteQuery(bcls_ret->Tables[0], startWorkPlanRowNum-1, 1);//分页获取
				cmd_inq.Close();
				sprintf(s.msg, "[%s]连铸机下PONO[%s]排在第[%d]行已经开始生产，不可排出计划!", (const char *)cc_math_no, (const char *)bcls_ret->Tables[0].Rows[0]["PONO"].ToString(), startWorkPlanRowNum);
				sprintf(s.sysmsg, "CC_MACH_NO[%s] set PLAN_COUNT[%d]<[%d] too small in pssm11_plan().", (const char *)cc_math_no, cc_plan_count, startWorkPlanRowNum);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//3.组织数据
			cmd_inq.ExecuteQuery(ccPlanBlk.Tables[0], 0, cc_plan_count);//分页获取
			cmd_inq.Close();
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "KR_ID"); //脱P设备
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "KR_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "KR_END_TIME");

			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "LD_1_ID"); //脱P设备
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "LD_1_WAIT_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "LD_1_END_TIME");	
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "LD_2_ID"); //炉坐号
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "LD_2_WAIT_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "LD_2_END_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_1_ID"); //精炼1
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_1_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_1_END_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_2_ID"); //精炼2
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_2_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_2_END_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_3_ID"); //精炼3
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_3_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_3_END_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_4_ID"); //精炼4
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_4_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "FINERY_4_END_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "CAST_1_ID");   //CC
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "CAST_1_START_TIME");
			ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "CAST_1_END_TIME");
			//炉次返送用 - 两个字段都已经存在
			//ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
			//ccPlanBlk.Tables[0].Columns.Add(DT_STRING, "RUN_STATUS");

			if (cci == 0){
				bcls_rec->Tables[0].Copy(ccPlanBlk.Tables[0]);//字段设置
				bcls_rec->Tables[0].Rows.Clear();
			}

			CDateTime cc_can_start_time; CString cc_can_start_time_str = "";
			for (int ri = 0; ri < ccPlanBlk.Tables[0].Rows.get_Count(); ri++)
			{
				ccPlanBlk.Tables[0].Rows[ri]["KR_ID"] = "S1";
				ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"] = "B4";
				ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"] = "C" + ccPlanBlk.Tables[0].Rows[ri]["CC_MACH_NO"].ToString();

				//粗略排时间：1.使用实际时刻；2.重引锭优先使用CC要求时刻，否则使用计划开浇时刻；如果第一炉没有CC_START_TIME,从转炉开始推算一个
				CString cc_start_time = "", cc_start_time_real = "", cc_end_time = "", cc_end_time_real = "";
				CString sr_start_time[] = { "", "", "", "" }, sr_start_time_real[] = { "", "", "", "" }, sr_end_time[] = { "", "", "", "" }, sr_end_time_real[] = { "", "", "", "" };
				//CString sr2_start_time = "", sr2_start_time_real = "", sr2_end_time = "", sr2_end_time_real = "";
				//CString sr3_start_time = "", sr3_start_time_real = "", sr3_end_time = "", sr3_end_time_real = "";
				//CString sr4_start_time = "", sr4_start_time_real = "", sr4_end_time = "", sr4_end_time_real = "";
				CString bof_start_time = "", bof_start_time_real = "", bof_end_time = "", bof_end_time_real = "";
				CString kr_start_time = "", kr_start_time_real = "", kr_end_time = "", kr_end_time_real = "";
				int cc_proc_time = -1,bof_proc_time = -1, kr_proc_time = -1;
				int sr_proc_time[] = { -1, -1, -1, -1 };

				//炉次返送的目的侧PONO处理
				if (ccPlanBlk.Tables[0].Rows[ri]["PONO"].ToString().GetAt(3) == '9')
				{
					//查找炉次返送信息
					sqlstr_tpssm35 = CString(" SELECT * FROM TPSSM35 WHERE RET_PONO=@PONO ORDER BY RET_TIME ");
					cmd_inq.SetCommandText(sqlstr_tpssm35);
					cmd_inq.Parameters.Set("PONO", ccPlanBlk.Tables[0].Rows[ri]["PONO"].ToString());
					cmd_inq.ExecuteReader();
					while (cmd_inq.Read())
					{
						cmd_inq.Fetch(tpssm35); //炉次返送信息
						ccPlanBlk.Tables[0].Rows[ri]["HEAT_NO"] = tpssm35["RET_HEAT_NO"];
						ccPlanBlk.Tables[0].Rows[ri]["REFINE_DIV"] = " "; //工艺技术路径
						switch (tpssm35["RET_DEST"][0])
						{
						case 'B':
							ccPlanBlk.Tables[0].Rows[ri]["RUN_STATUS"] = "33"; //冶炼开始
							break;
						case 'C':
							ccPlanBlk.Tables[0].Rows[ri]["RUN_STATUS"] = "51"; //连铸包到
							break;
						default:
							ccPlanBlk.Tables[0].Rows[ri]["RUN_STATUS"] = "36"; //出钢结束
							//精炼路径添加
							if (TpssmSg::getInstance().dev_map.find(tpssm35["RET_DEST"]) != TpssmSg::getInstance().dev_map.end())
							{
								int dmi = TpssmSg::getInstance().dev_map.find(tpssm35["RET_DEST"])->second;
								ccPlanBlk.Tables[0].Rows[ri]["REFINE_DIV"] = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows[dmi]["DEV_TECH_CODE"].ToString();
							}
							break;
						}
						Log::Info("", __FUNCTION__, "返送炉次PONO[{0}]： 排入计划HEAT_NO[{1}] REFINE_DIV[{2}] RUN_STATUS[{3}]", ccPlanBlk.Tables[0].Rows[ri]["PONO"].ToString(), ccPlanBlk.Tables[0].Rows[ri]["HEAT_NO"].ToString(), ccPlanBlk.Tables[0].Rows[ri]["REFINE_DIV"].ToString(), ccPlanBlk.Tables[0].Rows[ri]["RUN_STATUS"].ToString() );
					}
					cmd_inq.Close();
					if (tpssm35["PONO"].ToString().Trim() == "")
					{
						sprintf(s.msg, "[%s]连铸机:第[%d]行PONO[%s]为返送炉次，未做炉次返送前，不可排入计划!", (const char *)cc_math_no, ri + 1, (const char *)ccPlanBlk.Tables[0].Rows[ri]["PONO"].ToString());
						sprintf(s.sysmsg, "%s", s.msg);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				//炉次返送的目的侧PONO处理 end

				//计划开始结束、实际开始结束
				if (ccPlanBlk.Tables[0].Rows[ri]["SM_PLAN_NO"].ToString().Trim().GetLength()>0)
				{
					cmd_inq.SetCommandText(sqlstr_tpssm12);
					cmd_inq.Parameters.Set("tpssm11_FACTORY_DIV", v_factory_div);
					cmd_inq.Parameters.Set("tpssm11_SM_PLAN_NO", ccPlanBlk.Tables[0].Rows[ri]["SM_PLAN_NO"].ToString());
					cmd_inq.ExecuteQuery(subblk.Tables[0]);
					int sri = -1;
					for (int si = 0; si < subblk.Tables[0].Rows.get_Count(); si++)
					{
						switch (subblk.Tables[0].Rows[si]["AREA_ID"].ToDecimal().ToInt32())
						{
						case 1:
							ccPlanBlk.Tables[0].Rows[ri]["KR_ID"] = subblk.Tables[0].Rows[si]["DEV_CODE"];
							kr_start_time = subblk.Tables[0].Rows[si]["START_TIME"].ToString();
							kr_start_time_real = subblk.Tables[0].Rows[si]["START_TIME_REAL"].ToString();
							kr_end_time = subblk.Tables[0].Rows[si]["END_TIME"].ToString();
							kr_end_time_real = subblk.Tables[0].Rows[si]["END_TIME_REAL"].ToString();
							kr_proc_time = subblk.Tables[0].Rows[si]["PROC_TIME"].ToDecimal().ToInt32();
							break;
						case 3:
							ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"] = subblk.Tables[0].Rows[si]["DEV_CODE"];
							bof_start_time = subblk.Tables[0].Rows[si]["START_TIME"].ToString();
							bof_start_time_real = subblk.Tables[0].Rows[si]["START_TIME_REAL"].ToString();
							bof_end_time = subblk.Tables[0].Rows[si]["END_TIME"].ToString();
							bof_end_time_real = subblk.Tables[0].Rows[si]["END_TIME_REAL"].ToString();
							bof_proc_time = subblk.Tables[0].Rows[si]["PROC_TIME"].ToDecimal().ToInt32();
							break;
						case 5:
							cc_start_time = subblk.Tables[0].Rows[si]["START_TIME"].ToString();
							cc_start_time_real = subblk.Tables[0].Rows[si]["START_TIME_REAL"].ToString();
							cc_end_time = subblk.Tables[0].Rows[si]["END_TIME"].ToString();
							cc_end_time_real = subblk.Tables[0].Rows[si]["END_TIME_REAL"].ToString();
							cc_proc_time = subblk.Tables[0].Rows[si]["PROC_TIME"].ToDecimal().ToInt32();
							break;
						case 4:
							//精炼		
							sri++;
							sr_start_time[sri] = subblk.Tables[0].Rows[si]["START_TIME"].ToString();
							sr_start_time_real[sri] = subblk.Tables[0].Rows[si]["START_TIME_REAL"].ToString();
							sr_end_time[sri] = subblk.Tables[0].Rows[si]["END_TIME"].ToString();
							sr_end_time_real[sri] = subblk.Tables[0].Rows[si]["END_TIME_REAL"].ToString();
							sr_proc_time[sri] = subblk.Tables[0].Rows[si]["PROC_TIME"].ToDecimal().ToInt32();
							break;
						default:
							break;
						}
					}
					cmd_inq.Close();					
				}
				//1.cc_start_time
				if (cc_start_time_real.GetLength() == 14)
				{
					//实际开始
					cc_start_time = cc_start_time_real;					
				}
				else if (ccPlanBlk.Tables[0].Rows[ri]["CC_REQ_TIME_FLAG"].ToString().Trim() != "")
				{
					//指定开浇时刻
					cc_start_time = ccPlanBlk.Tables[0].Rows[ri]["CC_REQ_TIME"].ToString();					
				}
				else if (cc_can_start_time_str != "")
				{
					//上炉结束时刻
					cc_start_time = cc_can_start_time.ToString("yyyyMMddHHmmss");
				}
				else if (cc_start_time == "")
				{
					Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] cc_start_time没取到值，从bof开始推，逻辑未实现!", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);
					cc_start_time = datetime;
				}
				ccPlanBlk.Tables[0].Rows[ri]["CAST_1_START_TIME"] = cc_start_time;
				cc_can_start_time = CDateTime::Parse(cc_start_time); //准备倒推
				Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 完成计算 cc_start_time !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);
				
				//2.cc_end_time
				Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 准备计算 cc_end_time !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);
				if (cc_end_time_real.GetLength() == 14)
				{
					//实际结束
					cc_end_time = cc_end_time_real;
				}
				else {
					if (cc_proc_time < 0)
					{
						if (TpssmSg::getInstance().dev_map.find(ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString()) != TpssmSg::getInstance().dev_map.end())
						{
							//Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 取单例begin cc_proc_time !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);
							int mi = TpssmSg::getInstance().dev_map.find(ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString())->second;
							cc_proc_time = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows[mi]["STD_PROC_TIME"].ToDecimal().ToInt32();
							//Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 取单例end cc_proc_time[{2}] !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri, cc_proc_time);
						}
						if (cc_proc_time < 0)
						{
							Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],没取到值STD_PROC_TIME 意外的逻辑 使用30!", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString());
							cc_proc_time = 30;
						}
					}
					cc_end_time = cc_can_start_time.AddMinutes(cc_proc_time).ToString("yyyyMMddHHmmss");
				}
				ccPlanBlk.Tables[0].Rows[ri]["CAST_1_END_TIME"] = cc_end_time;
				//Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 完成计算 cc_end_time !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);

				//2.cc_end_time
				Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 准备计算 sr_time !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);
				//3.精炼 - 四重精炼
				CString v_plan_REFINE_DIV = ccPlanBlk.Tables[0].Rows[ri]["REFINE_DIV"].ToString().Trim();
				CString v_set_sr_route = ccPlanBlk.Tables[0].Rows[ri]["REFINE_ROUTE_CODE"].ToString().Trim();
				//3-1. 如果精炼路径为空，读取计划精炼区分作为默认值
				int sr_len = 0;
				if (ccPlanBlk.Tables[0].Rows[ri]["SM_PLAN_NO"].ToString().Trim().GetLength() <= 0){
					sr_len = v_plan_REFINE_DIV.GetLength();
					for (int sri = 0; sri < sr_len; sri++){
						//根据工艺代码查找
						if (TpssmSg::getInstance().dev_tech_map.find(v_plan_REFINE_DIV.Substring(sri, 1)) != TpssmSg::getInstance().dev_tech_map.end()){
							int mi = TpssmSg::getInstance().dev_tech_map.find(v_plan_REFINE_DIV.Substring(sri, 1))->second;
							Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],设备工艺代码[{1}]找到缓存数据[{2}] !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), v_plan_REFINE_DIV.Substring(sri, 1), mi);
							v_set_sr_route += TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows[mi]["DEV_CODE"].ToString();
						}
						else{
							Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],设备工艺代码[{1}] 未找到对应的设备代码 !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), v_plan_REFINE_DIV.Substring(sri, 1));
						}
						
					}					
				}
				//else{
					sr_len = v_set_sr_route.GetLength() / 2;
				//}
				for (int sri = 0; sri < 4 && sri < sr_len; sri++)
				{
					//3-2. 精炼结束开始粗算
					CString sr_dev_code = v_set_sr_route.Substring(sri * 2, 2);
					sr_dev_col_name = CString::Format("FINERY_%d_ID", (sri + 1));
					ccPlanBlk.Tables[0].Rows[ri][sr_dev_col_name] = sr_dev_code;
					if (sr_end_time_real[sri].GetLength() == 14)
					{
						//实际结束
						sr_end_time[sri] = sr_end_time_real[sri];
					}
					else{
						sr_end_time[sri] = cc_can_start_time.AddMinutes(-20).ToString("yyyyMMddHHmmss"); //传搁时间
					}
					sr_end_col_name = CString::Format("FINERY_%d_END_TIME", (sri + 1));
					ccPlanBlk.Tables[0].Rows[ri][sr_end_col_name] = sr_end_time[sri];
					//3-4.精炼开始
					if (sr_start_time_real[sri].GetLength() == 14)
					{
						//实际开始
						sr_start_time[sri] = sr_start_time_real[sri];
					}
					else
					{
						if (sr_proc_time[sri] < 0) //精炼处理时间
						{
							if (TpssmSg::getInstance().dev_map.find(sr_dev_code) != TpssmSg::getInstance().dev_map.end())
							{
								//Log::Info("", __FUNCTION__, "SR[{0}],第[{1}] 取单例begin sr_proc_time !", sr_dev_code, ri);
								int mi = TpssmSg::getInstance().dev_map.find(sr_dev_code)->second;
								sr_proc_time[sri] = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows[mi]["STD_PROC_TIME"].ToDecimal().ToInt32();
								//Log::Info("", __FUNCTION__, "SR[{0}],第[{1}] 取单例end sr_proc_time[{2}] !", sr_dev_code, ri, sr_proc_time[sri]);
							}
							if (sr_proc_time[sri] < 0)
							{
								Log::Info("", __FUNCTION__, "SR[{0}],没取到值STD_PROC_TIME 意外的逻辑 使用20!", sr_dev_code);
								sr_proc_time[sri] = 20;
							}
						}
						sr_start_time[sri] = CDateTime::Parse(sr_end_time[sri]).AddMinutes(-sr_proc_time[sri]).ToString("yyyyMMddHHmmss");
					}
					sr_start_col_name = CString::Format("FINERY_%d_START_TIME", (sri + 1));
					ccPlanBlk.Tables[0].Rows[ri][sr_start_col_name] = sr_start_time[sri];
					cc_can_start_time = CDateTime::Parse(sr_start_time[sri]); //准备倒推
				}//end 精炼时间粗算

				//4-1.bof_end_time
				if (bof_end_time_real.GetLength() == 14)
				{
					//实际结束
					bof_end_time = bof_end_time_real;
				}
				else{
					bof_end_time = cc_can_start_time.AddMinutes(-20).ToString("yyyyMMddHHmmss"); //传搁时间
				}
				ccPlanBlk.Tables[0].Rows[ri]["LD_2_END_TIME"] = bof_end_time;
				//4-2.bof_start_time
				if (bof_start_time_real.GetLength() == 14)
				{
					//实际开始
					bof_start_time = bof_start_time_real;
				}
				else{
					if (bof_proc_time < 0) //转炉处理时间
					{
						if (TpssmSg::getInstance().dev_map.find(ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"].ToString()) != TpssmSg::getInstance().dev_map.end())
						{
							//Log::Info("", __FUNCTION__, "BOF[{0}],第[{1}] 取单例begin bof_proc_time !", ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"].ToString(), ri);
							int mi = TpssmSg::getInstance().dev_map.find(ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"].ToString())->second;
							bof_proc_time = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows[mi]["STD_PROC_TIME"].ToDecimal().ToInt32();
							//Log::Info("", __FUNCTION__, "BOF[{0}],第[{1}] 取单例end bof_proc_time[{2}] !", ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"].ToString(), ri, bof_proc_time);
						}
						if (bof_proc_time < 0)
						{
							Log::Info("", __FUNCTION__, "BOF[{0}],没取到值STD_PROC_TIME 意外的逻辑 使用30!", ccPlanBlk.Tables[0].Rows[ri]["LD_2_ID"].ToString());
							bof_proc_time = 30;
						}
					}
					bof_start_time = CDateTime::Parse(bof_end_time).AddMinutes(-bof_proc_time).ToString("yyyyMMddHHmmss");
				}
				ccPlanBlk.Tables[0].Rows[ri]["LD_2_WAIT_START_TIME"] = bof_start_time;

				//5-1. 脱硫
				cc_can_start_time = CDateTime::Parse(bof_start_time); //准备倒推
				if (kr_end_time_real.GetLength() == 14)
				{
					//实际结束
					kr_end_time = kr_end_time_real;
				}
				else{
					kr_end_time = cc_can_start_time.AddMinutes(-20).ToString("yyyyMMddHHmmss"); //传搁时间
				}
				ccPlanBlk.Tables[0].Rows[ri]["KR_END_TIME"] = kr_end_time;
				//5-2.kr_start_time
				if (kr_start_time_real.GetLength() == 14)
				{
					//实际开始
					kr_start_time = kr_start_time_real;
				}
				else{
					if (kr_proc_time < 0) //转炉处理时间
					{
						if (TpssmSg::getInstance().dev_map.find(ccPlanBlk.Tables[0].Rows[ri]["KR_ID"].ToString()) != TpssmSg::getInstance().dev_map.end())
						{
							//Log::Info("", __FUNCTION__, "BOF[{0}],第[{1}] 取单例begin kr_proc_time !", ccPlanBlk.Tables[0].Rows[ri]["KR_ID"].ToString(), ri);
							int mi = TpssmSg::getInstance().dev_map.find(ccPlanBlk.Tables[0].Rows[ri]["KR_ID"].ToString())->second;
							kr_proc_time = TpssmSg::getInstance().bcls_pst.Tables["DEV"].Rows[mi]["STD_PROC_TIME"].ToDecimal().ToInt32();
							//Log::Info("", __FUNCTION__, "BOF[{0}],第[{1}] 取单例end kr_proc_time[{2}] !", ccPlanBlk.Tables[0].Rows[ri]["KR_ID"].ToString(), ri, kr_proc_time);
						}
						if (kr_proc_time < 0)
						{
							Log::Info("", __FUNCTION__, "脱硫[{0}],没取到值STD_PROC_TIME 意外的逻辑 使用30!", ccPlanBlk.Tables[0].Rows[ri]["KR_ID"].ToString());
							kr_proc_time = 30;
						}
					}
					kr_start_time = CDateTime::Parse(kr_end_time).AddMinutes(-kr_proc_time).ToString("yyyyMMddHHmmss");
				}
				ccPlanBlk.Tables[0].Rows[ri]["KR_START_TIME"] = kr_start_time;

				Log::Info("", __FUNCTION__, "CC_MACH_NO[{0}],第[{1}] 粗算完成 !", ccPlanBlk.Tables[0].Rows[ri]["CAST_1_ID"].ToString(), ri);
				//for end下一次循环
				cc_can_start_time = CDateTime::Parse(cc_end_time);
				cc_can_start_time_str = cc_end_time;

				CDataRow &mfPlanQtIn = bcls_rec->Tables[0].Rows.Add();//逐个CC的计划压入bcls_rec->Tables[0]
				mfPlanQtIn.Merge(ccPlanBlk.Tables[0].Rows[ri]);
			}		
			////逐个CC的计划压入bcls_rec->Tables[0]
			//bcls_rec->Tables[0].Rows.Add()
		}


		//------------------------原pssm18_save处理逻辑------------------------
		Log::Info("", __FUNCTION__, "FACTORY_DIV=[{0}]", v_factory_div);

		inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;

		//在做出钢计划保存前，当前出钢计划置删除标记"D"
		tpssm11["FACTORY_DIV"] = v_factory_div;
		/*tpssm11["PLAN_EDIT_FLAG"] = "D";
		sqlstr = "tpssm11.Update(PLAN_EDIT_FLAG = D)";
		tpssm11.Update("PLAN_EDIT_FLAG", "FACTORY_DIV");*/

		sqlstr = CString(
			" UPDATE TPSSM11 "
			"    SET PLAN_EDIT_FLAG = 'D'"
			"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
			"    AND PONO_STATUS    < 20 "
			);

		cmd_upd.SetCommandText(sqlstr);
		cmd_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd.ExecuteNonQuery();

		/* ***** 获取输入参数 ***** */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Info("", __FUNCTION__, "编制总计划数 = [{0}]", rows);
		for (int i = 0; i < rows; i++)
		{
			v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();
			v_td_chg_flg = bcls_rec->Tables[0].Rows[i]["TD_CHG_FLG"].ToDecimal();//换中包标记
			v_restrand_flg = bcls_rec->Tables[0].Rows[i]["RESTRAND_FLG"].ToString().TrimOrBlank();//重引锭标记
			v_cc_req_time = bcls_rec->Tables[0].Rows[i]["CC_REQ_TIME"].ToString().Trim();
			tpssm11["PLAN_STYLE"] = " ";//2023-6-2 bcls_rec->Tables[0].Rows[i]["PLAN_STYLE"].ToString().Trim();
			tpssm11["STEEL_RETURN_CODE"] = bcls_rec->Tables[0].Rows[i]["STEEL_RETURN_CODE"].ToString().TrimOrBlank();
			v_smelt_mode = bcls_rec->Tables[0].Rows[i]["SMELT_MODE"].ToDecimal();


			////Log::Info("", __FUNCTION__, "pono = [{0}]", v_pono);
			////Log::Info("", __FUNCTION__, "快换中包标记 tpssm11["TD_CHG_FLG"] =[{0}]", v_td_chg_flg.ToInt32());
			////Log::Info("", __FUNCTION__, "重引锭标记 tpssm10["RESTRAND_FLG"] =[{0}]", v_restrand_flg);
			////Log::Info("", __FUNCTION__, "tpssm11["CC_REQ_TIME"] =[{0}]", v_cc_req_time);
			////Log::Info("", __FUNCTION__, "tpssm11["PLAN_STYLE"] =[{0}]", tpssm11["PLAN_STYLE"].ToString());

			if (v_restrand_flg.Trim() == "1" || v_restrand_flg.Trim() == "T")
			{
				v_restrand_flg = "T";
			}
			else
			{
				v_restrand_flg = " ";
			}

			//不能同时快换中包和重引锭
			if (v_td_chg_flg == 1 && v_restrand_flg == "T")
			{
				CFormattable arguments[] = { v_pono };
				CMessageFormat::Format(s.msg, "炉次[{0}]同时快换中包和重引锭，不符合规则，请重新选定方式后继续保存计划。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/* ***** 检查输入参数合法性 ***** */
			if (v_pono.GetLength() <= 0)
			{
				doFlag = -11;
				//sprintf(s.msg, "收到的PONO号[%s]长度有误！",(const char*)tpssm11["PONO"].ToString());
				CFormattable arguments[] = { v_pono }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}


			//-------------------------------------------------------------------
			//读取传入PONO, 判断新增还是修改处理
			tpssm11["FACTORY_DIV"] = v_factory_div;
			tpssm11["PONO"] = v_pono;
			sqlstr = "tpssm11.Query()";
			bool has11 = tpssm11.Query("FACTORY_DIV,PONO");
			if (has11 == false) //计划表中没有该炉次，新增
			{
				//新增
				inblk.Tables["PONO"].Rows[0]["PLID"] = i + 1;  //ID:参与临时计划号计算用
				inblk.Tables["PONO"].Rows[0]["PONO"] = v_pono;
				inblk.Tables["PONO"].Rows[0]["TD_CHG_FLG"] = v_td_chg_flg;
				inblk.Tables["PONO"].Rows[0]["RESTRAND_FLG"] = v_restrand_flg;
				inblk.Tables["PONO"].Rows[0]["CC_REQ_TIME"] = v_cc_req_time;
				inblk.Tables["PONO"].Rows[0]["SMELT_MODE"] = v_smelt_mode;
				//炉次返送
				inblk.Tables["PONO"].Rows[0]["PONO_STATUS"] = bcls_rec->Tables[0].Rows[i]["PONO_STATUS"].ToString().Trim();
				inblk.Tables["PONO"].Rows[0]["HEAT_NO"] = bcls_rec->Tables[0].Rows[i]["HEAT_NO"].ToString().Trim();
				inblk.Tables["PONO"].Rows[0]["RUN_STATUS"] = bcls_rec->Tables[0].Rows[i]["RUN_STATUS"].ToString().Trim();

				////写计划履历表
				//tpssm99["EVENT_ID"] = "01";
				//tpssm99["FACTORY_DIV"] = v_factory_div;
				//tpssm99["PONO"] = v_pono;
				//新增炉次，单记录处理
				ret = f_pssm11_ins_heat_n(&inblk, bcls_ret, conn);
				if (ret < 0)
				{
					////dclian---add---2015-11-16------
					//tpssm10["FACTORY_DIV"] = v_factory_div;
					//tpssm10["PONO"] = v_pono;
					//tpssm10.Query("FACTORY_DIV,PONO");
					//tpssm99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
					//tpssm99["VALID_FLAG"] = "0";//操作失败
					///*in_pssm99trace.Tables[0].Clone(tpssm99);*/
					//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
					//////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
					//tpabort(0);
					//tpbegin(0, 0);
					//////Log::Trace("", __FUNCTION__, "记录失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
					////记录编入计划失败的履历
					//ret = 0;
					//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
					//if (ret < 0)
					//{
					//	throw CApplicationException(-1, s.msg, log.Location);
					//}
					//tpcommit(0);
					//tpbegin(0, 0);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//获取新增炉次的计划号，给后续工序计划使用。
				tpssm11["SM_PLAN_NO"] = bcls_ret->Tables["PONO"].Rows[0]["SM_PLAN_NO"].ToString();
				//tpssm11.Query("FACTORY_DIV,PONO");
				//tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
				//tpssm99["VALID_FLAG"] = "1";//操作成功
				///*in_pssm99trace.Tables[0].Clone(tpssm99);*/
				//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
				//////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

				//////Log::Trace("", __FUNCTION__, "记录成功履历，传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
				////记录编入计划成功的履历
				//ret = 0;
				//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				//if (ret < 0)
				//{
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				//dclian---add---2015-11-16------
			}
			else //出钢计划修改
			{

				//炉次未开浇，可以修改浇铸要求
				if (tpssm11["RUN_STATUS"].ToString() == "52" || tpssm11["RUN_STATUS"].ToString() == "53") //52-开浇; 53-浇铸完
				{
					CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组

					//修改开浇标志 甘特图传入数据有误，临时注释
					//if (tpssm11["RESTRAND_FLG"].ToString().Trim() != v_restrand_flg.Trim())
					//{
					//	CMessageFormat::Format(s.msg, "制造命令[{0}]已开始浇铸，不能设置连浇标记。", arguments, 1);
					//	throw CApplicationException(-1, s.msg, log.Location);
					//}
					//if (tpssm11["CC_REQ_TIME"].ToString().Trim() != v_cc_req_time.Trim())
					//{
					//	CMessageFormat::Format(s.msg, "制造命令[{0}]已开始浇铸，不能修改开浇时刻。", arguments, 1);
					//	throw CApplicationException(-1, s.msg, log.Location);
					//}
					//if (tpssm11["TD_CHG_FLG"].ToDecimal().ToInt32() != v_td_chg_flg.ToInt32())
					//{
					//	CMessageFormat::Format(s.msg, "制造命令[{0}]已开始浇铸，不能设置快换中包。", arguments, 1);
					//	throw CApplicationException(-1, s.msg, log.Location);
					//}

				}

				//修改出钢计划表TPSSM11
				tpssm11["FACTORY_DIV"] = v_factory_div;
				tpssm11["PONO"] = v_pono;
				tpssm11["RESTRAND_FLG"] = v_restrand_flg;
				tpssm11["CC_REQ_TIME"] = v_cc_req_time.TrimOrBlank();
				//tpssm11.TPD_START_TIME = v_tpd_start_time.TrimOrBlank();
				tpssm11["TD_CHG_FLG"] = v_td_chg_flg;
				tpssm11["PLAN_EDIT_FLAG"] = "U";
				tpssm11["SMELT_MODE"] = v_smelt_mode;
				sqlstr = "tpssm11.Update()";
				tpssm11.Update(
					//"TPD_START_TIME,"
					"RESTRAND_FLG,"
					"CC_REQ_TIME,"
					"TD_CHG_FLG,"
					"PLAN_EDIT_FLAG",
					"FACTORY_DIV,SM_PLAN_NO");

				//修改浇铸计划表TPSSM10
				tpssm10["FACTORY_DIV"] = v_factory_div;
				tpssm10["PONO"] = tpssm11["PONO"];
				tpssm10["RESTRAND_FLG"] = v_restrand_flg;
				tpssm10["TD_CHG_FLG"] = v_td_chg_flg;
				tpssm10["SMELT_MODE"] = v_smelt_mode;
				if (v_cc_req_time == "")  //没有指定
				{
					tpssm10["CC_REQ_TIME"] = " ";
					tpssm10["CC_REQ_TIME_FLAG"] = " ";
				}
				else
				{
					tpssm10["CC_REQ_TIME"] = v_cc_req_time;
					tpssm10["CC_REQ_TIME_FLAG"] = "1";
				}
				sqlstr = "tpssm10.Update()";
				tpssm10.Update(
					"RESTRAND_FLG,"
					"TD_CHG_FLG,"
					"CC_REQ_TIME,"
					"CC_REQ_TIME_FLAG",
					"FACTORY_DIV,PONO");

			}

			////Log::Trace("", __FUNCTION__, "炉次：PONO=[{0}]， SM_PLAN_NO=[{1}]", tpssm11["PONO"].ToString(), tpssm11["SM_PLAN_NO"].ToString());


			//-------------------------------------------------------------------
			//工序计划处理: 整理输入工序计划数据项
			charge_no = 0;
			//1）主计划赋值
			if (inblk.Tables["TPSSM11"].Rows.get_Count() == 0)
				inblk.Tables["TPSSM11"].Rows.Add();
			inblk.Tables["TPSSM11"].Rows[0]["FACTORY_DIV"] = v_factory_div;
			inblk.Tables["TPSSM11"].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			inblk.Tables["TPSSM11"].Rows[0]["PONO"] = v_pono;
			//2）子计划记录清空
			inblk.Tables["TPSSM12"].Rows.Clear();

			//------------------------
			//1、脱硫工序（甘特图没有）
			if (false && bcls_rec->Tables[0].Columns.Contains("KR_ID"))
			{
				tpssm12_ds["DEV_CODE"]   = bcls_rec->Tables[0].Rows[i]["KR_ID"].ToString().Trim();
				tpssm12_ds["START_TIME"] = bcls_rec->Tables[0].Rows[i]["KR_START_TIME"].ToString().Trim();
				tpssm12_ds["END_TIME"]   = bcls_rec->Tables[0].Rows[i]["KR_END_TIME"].ToString().Trim();
				if (tpssm12_ds["DEV_CODE"].ToString().Trim() != "")  //有脱硫设备，则走该工序
				{
					charge_no = charge_no + 1;  //指定charge号
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 1;   //1-脱硫
					row["DEV_CODE"] = tpssm12_ds["DEV_CODE"];
					row["START_TIME"] = tpssm12_ds["START_TIME"];
					row["END_TIME"] = tpssm12_ds["END_TIME"];
				}
			}			
			////Log::Trace("", __FUNCTION__, "脱硫({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_ds["DEV_CODE"].ToString(), tpssm12_ds["START_TIME"].ToString(), tpssm12_ds["END_TIME"].ToString(), charge_no);


			//------------------------
			//2、转炉脱P工序（甘特图没有）
			tpssm12_dp["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_1_ID"].ToString().Trim();
			tpssm12_dp["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_WAIT_START_TIME"].ToString().Trim();
			tpssm12_dp["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_END_TIME"].ToString().Trim();
			v_smelt_mode = 1;
			if (tpssm12_dp["DEV_CODE"].ToString().Trim() != "")  //有脱P设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 2;   //2-脱P
				row["DEV_CODE"] = tpssm12_dp["DEV_CODE"];
				row["START_TIME"] = tpssm12_dp["START_TIME"];
				row["END_TIME"] = tpssm12_dp["END_TIME"];
				v_smelt_mode = 2;
			}
			////Log::Trace("", __FUNCTION__, "脱P ({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_dp["DEV_CODE"].ToString(), tpssm12_dp["START_TIME"].ToString(), tpssm12_dp["END_TIME"].ToString(), charge_no);


			//------------------------
			//3、转炉脱C、电炉工序
			tpssm12_bof["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_2_ID"].ToString().Trim();
			tpssm12_bof["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_2_WAIT_START_TIME"].ToString().Trim();
			tpssm12_bof["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_2_END_TIME"].ToString().Trim();
			if (tpssm12_bof["DEV_CODE"].ToString().Trim() != "")  //有转炉设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 3;   //3-脱C
				row["DEV_CODE"] = tpssm12_bof["DEV_CODE"];
				row["START_TIME"] = tpssm12_bof["START_TIME"];
				row["END_TIME"] = tpssm12_bof["END_TIME"];
			}
			else
			{
				CFormattable arguments[] = { v_pono, tpssm12_bof["DEV_CODE"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划炉次[{0}]的转炉/电炉设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Trace("", __FUNCTION__, "转炉({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_bof["DEV_CODE"].ToString(), tpssm12_bof["START_TIME"].ToString(), tpssm12_bof["END_TIME"].ToString(), charge_no);


			//------------------------
			//4、精炼1
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 4;   //4-精炼
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
				row["START_TIME"] = tpssm12_sr["START_TIME"];
				row["END_TIME"] = tpssm12_sr["END_TIME"];
			}
			////Log::Trace("", __FUNCTION__, "精炼1({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


			//------------------------
			//5、精炼2
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼2设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 4;   //4-精炼
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
				row["START_TIME"] = tpssm12_sr["START_TIME"];
				row["END_TIME"] = tpssm12_sr["END_TIME"];
			}
			////Log::Trace("", __FUNCTION__, "精炼2({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


			//------------------------
			//6、精炼3
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 4;   //4-精炼
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
				row["START_TIME"] = tpssm12_sr["START_TIME"];
				row["END_TIME"] = tpssm12_sr["END_TIME"];
			}
			////Log::Trace("", __FUNCTION__, "精炼3({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


			//------------------------
			//7、精炼4
			tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_ID"].ToString().Trim();
			tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_START_TIME"].ToString().Trim();
			tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_END_TIME"].ToString().Trim();
			if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 4;   //4-精炼
				row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
				row["START_TIME"] = tpssm12_sr["START_TIME"];
				row["END_TIME"] = tpssm12_sr["END_TIME"];
			}
			////Log::Trace("", __FUNCTION__, "精炼4({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);

			//如果多于4重精炼，在此处扩展。


			//------------------------
			//8、连铸、模铸工序
			tpssm12_cc["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["CAST_1_ID"].ToString().Trim();
			tpssm12_cc["START_TIME"] = bcls_rec->Tables[0].Rows[i]["CAST_1_START_TIME"].ToString().Trim();
			tpssm12_cc["END_TIME"] = bcls_rec->Tables[0].Rows[i]["CAST_1_END_TIME"].ToString().Trim();
			if (tpssm12_cc["DEV_CODE"].ToString().Trim() != "")  //有浇铸设备，则走该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 5;   //5-浇铸
				row["DEV_CODE"] = tpssm12_cc["DEV_CODE"];
				row["START_TIME"] = tpssm12_cc["START_TIME"];
				row["END_TIME"] = tpssm12_cc["END_TIME"];
			}
			else
			{
				CFormattable arguments[] = { v_pono, tpssm12_cc["DEV_CODE"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "炼钢计划炉次[{0}]的浇铸设备不能为空, 请输入后操作。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Trace("", __FUNCTION__, "连铸({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_cc["DEV_CODE"].ToString(), tpssm12_cc["START_TIME"].ToString(), tpssm12_cc["END_TIME"].ToString(), charge_no);


			//------------------------
			//调用工序计划保存处理函数
			ret = f_pssm12_save_job_n(&inblk, bcls_ret, conn);  //只新增炉次
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm11["SMELT_MODE"] = v_smelt_mode;
			tpssm11.Update("SMELT_MODE", "FACTORY_DIV,PONO");

		}//for 前台输入

		////dclian---add---2015-11-16------
		////为写履历赋值----
		///*in_pssm99trace.Tables[0].Clone(tpssm99);*/
		//tpssm99["FACTORY_DIV"] = v_factory_div;;
		//tpssm99["EVENT_ID"] = "02";
		////查出需要删除的炉次
		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:         // MS SQL Server数据库
		//case DB_KIND_ORACLE:        // Oracle 数据库
		//default:  // 所有数据库适用，通用SQL语句
		//	sqlstr = CString(
		//		" SELECT * FROM TPSSM11 "
		//		"  WHERE FACTORY_DIV     = @v_factory_div "
		//		"    AND PLAN_EDIT_FLAG = 'D' "
		//		);
		//	break;
		//}
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.Parameters.Set("v_factory_div", v_factory_div);
		//cmd_tpssm11_inq.ExecuteReader();
		//while (cmd_tpssm11_inq.Read())
		//{
		//	cmd_tpssm11_inq.Fetch(tpssm11);
		//	tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		//	tpssm99["PONO"] = tpssm11["PONO"];
		//	tpssm99["VALID_FLAG"] = "1";
		//	tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		//	////Log::Info(" ", __FUNCTION__, "tpssm11["PONO"] ={0}", tpssm11["PONO"].ToString());
		//}
		//cmd_tpssm11_inq.Close();

		////Log::Trace("", __FUNCTION__, "调用函数f_pssm_pas1p1_snd开始-先删除已下发L2的出钢计划和铸坯命令");
		//先发删除计划给L2，然后再把咱们计划表中对应数据进行删除。
		inblk.Tables["PAS"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		inblk.Tables["PAS"].Rows[0]["OPER_FLAG"] = "D";//删除


		//ret = f_pssm_pas1p1_snd(&inblk, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}


		//2. 删除编制标志“D”的炉次，必须先删，再做后续的计算
		////Log::Trace("", __FUNCTION__, "检查是否有需要删除的PONO");
		ret = f_pssm11_del_heat_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
			////Log::Trace("", __FUNCTION__, "计划移除失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
			//tpabort(0);
			//tpbegin(0, 0);
			//for (int i = 0; i < in_pssm99trace.Tables[0].Rows.get_Count(); i++)
			//{
			//	
			//	in_pssm99trace.Tables[0].Rows[i]["VALID_FLAG"] = "0";//失败履历
			//}
			////记录删除计划失败的履历
			//ret = 0;
			//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			//

			//tpcommit(0);
			//tpbegin(0, 0);
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//重新计算浇次号
		////Log::Trace("", __FUNCTION__, "重新计算CAST号");
		ret = f_pssm21_cast_cre_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//--------------------------------------------------
		//时间推移计算
		if (true)
		{
			ret = f_pssm_call_tps_n(v_factory_div, v_mode_type, conn);
			if (ret < 0)
			{
				for (int i = 0; i < in_pssm99trace.Tables[0].Rows.get_Count(); i++)
				{
					in_pssm99trace.Tables[0].Rows[i]["VALID_FLAG"] = "0";//记录失败履历
				}
				tpabort(0);
				tpbegin(0, 0);
				//记录失败的履历
				ret = 0;
				ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpcommit(0);
				tpbegin(0, 0);
				strcpy(s.msg, "时间优化推移出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}		
		//--------------------------------------------------

		//重新计算计划号
		////Log::Trace("", __FUNCTION__, "重新计算计划号");
		ret = f_pssm11_planno_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//重新计算处理号
		////Log::Trace("", __FUNCTION__, "重新计算处理号");
		ret = f_pssm12_sequ_calc_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//-----------------------------------------------
		
		//////下发L2出钢计划和铸坯命令
		//sqlstr = "SELECT a.*  \
				//		 	FROM tpssm11 a, tpssm12 b \
				//			WHERE a.factory_div = @factory_div \
				//			AND a.sm_plan_no = b.sm_plan_no \
				//			AND b.area_id = 3 \
				//		 AND a.run_status        < 52 ";
		//cmd_tpssm11_inq.SetCommandText(sqlstr);
		//cmd_tpssm11_inq.Parameters.Set("factory_div", v_factory_div);
		//cmd_tpssm11_inq.ExecuteReader();
		//while (cmd_tpssm11_inq.Read())
		//{
		//	cmd_tpssm11_inq.Fetch(tpssm11);//把数据都压在头文件里面
		//	tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		//	tpssm99["PONO"] = tpssm11["PONO"];
		//	tpssm99["EVENT_ID"] = "04";
		//	tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		//	tpssm99["VALID_FLAG"] = "1";//默认为成功
		//	tpssm99["EVENT_DATETIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");//当前系统时间
		//	/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
		//	tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

		//}
		////Log::Trace("", __FUNCTION__, "调用函数f_pssm_pas1p1_snd开始---准备下发L2出钢计划和铸坯命令");

		inblk.Tables["PAS"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		inblk.Tables["PAS"].Rows[0]["OPER_FLAG"] = "I";//新增

		//ret = f_pssm_pas1p1_snd(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			CFormattable arguments[] = { v_pono, tpssm12_bof["DEV_CODE"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "电文发送失败", arguments, 2);
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

	return doFlag;
}
