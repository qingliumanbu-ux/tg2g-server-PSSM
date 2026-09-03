/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-26
Version:  3.1.0
Description: 甘特图出钢计划编制保存
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件





//#include "tpssm26.h"

int f_pssm11_ins_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划主表新增，单记录处理
int f_pssm_call_tps_n(CString factory_div, int mode, CDbConnection * conn);
int f_pssm12_save_job_n_2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划之工序计划保存
int f_pssm11_del_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢作业计划删除,标志为“D”的炉次
int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//甘特图炉次生成CAST号
int f_pssm11_planno_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);  //出钢计划的计划号生成
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划各工序作业顺序号生成（包括HEAT_NO）
int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
int f_t8e2s1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm12_treatmentcount(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);
int f_pssm_seq_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_pssm_pas1p1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//出钢计划发送 

/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划编制保存
/// <para>根据Client甘特图输入的数据，修改出钢计划。  </para>
/// <para>
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
BM2F_ENTERACE(pssm18_pssm12_add)

int f_pssm18_pssm12_add(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	CString v_pono = "";
	CString v_restrand_flg = "";     //连浇标记
	CString v_cc_req_time = "";  //开浇时刻
	CString v_tpd_start_time = "";  //倒罐开始时刻
	CDecimal v_td_chg_flg = 0, v_smelt_mode = 0;
	CDecimal charge_no = 0;      //工序charge号
	CString datetime = "";
	CString routebagkey = "";
	CString routelist = "";
	CString smelt_mode2 = "";

	EIClass inblk;        //调用函数用
	EIClass in_pssm99trace;//调用履历函数
	EIClass in_pssm18;//调用发送停机实绩的函数
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm18_del(conn);
	CDbCommand cmd_tpssm11_inq(conn);

	CDataTable tb_tpssm11("TPSSM11");

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		CModel tpssm10("TPSSM10");
		CModel tpssm11("TPSSM11");
		CModel tpssm41("TPSSM41");
		CModel tpssm12("TPSSM12");
		CModel tpssm15("TPSSM15");
		CModel tpssm16("TPSSM16");
		CModel tpssm12_ds("TPSSM12");//脱硫
		CModel tpssm12_sd("TPSSM12");//预溶液
		CModel tpssm12_dp("TPSSM12");//转炉脱磷
		CModel tpssm12_bof("TPSSM12");//转炉/电炉工序
		CModel tpssm12_sr("TPSSM12");//精炼
		CModel tpssm12_cc("TPSSM12");//连铸
		CModel tpssm18("TPSSM18");//设备状态
		CModel tpssm99("TPSSM99");
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
		table.Columns.Add(DT_STRING, "ROUTEBAGKEY");   //
		table.Columns.Add(DT_STRING, "ROUTELIST");
		table.Columns.Add(DT_STRING, "SMELT_MODE2");
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
		table2.Columns.Add(DT_DECIMAL, "CHARGE_NO");

		//4、出钢计划下发
		//CDataTable &table3 = inblk.Tables.Add("PAS");  //出钢计划下发
		//table3.Columns.Add(DT_STRING, "FACTORY_DIV");    //炼钢单元号
		//table3.Columns.Add(DT_STRING, "OPER_FLAG");    //操作标志：I：新增，D删除。
		//table3.Rows.Add();  //只定义一行


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
		sd_1_id              脱硫工位（新增）
		sd_1_wait_start_time
		sd_1_end_time
		sd_2_id              预溶液工位（新增）1
		sd_2_wait_start_time
		sd_2_end_time
		sd_3_id              预溶液工位（新增）2
		sd_3_wait_start_time
		sd_3_end_time
		sd_4_id              预溶液工位（新增）3
		sd_4_wait_start_time
		sd_4_end_time
		sd_5_id              预溶液工位（新增）4
		sd_5_wait_start_time
		sd_5_end_time
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
		finery_5_id        精炼5 工位
		finery_5_start_time
		finery_5_end_time
		finery_6_id        精炼6 工位
		finery_6_start_time
		finery_6_end_time
		finery_7_id        精炼7 工位
		finery_7_start_time
		finery_7_end_time
		finery_8_id        精炼8 工位
		finery_8_start_time
		finery_8_end_time
		cast_1_wait_id     连铸等待工位
		cast_1_wait_start_time
		cast_1_wait_end_time
		cast_1_id          连铸工位
		cast_1_start_time
		cast_1_end_time
		steel_return_code  返送代码
		sg_sign
		routebagkey 工艺路线包
		routelist 工艺路线
		smelt_mode2 AOD前路径
		*--------------------------------------------------------*/


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
		v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];

		inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		tpssm11["FACTORY_DIV"] = v_factory_div;

		/* ***** 获取输入参数 ***** */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rows; i++)
		{
			v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();
			v_td_chg_flg = bcls_rec->Tables[0].Rows[i]["TD_CHG_FLG"].ToDecimal();//换中包标记
			v_restrand_flg = bcls_rec->Tables[0].Rows[i]["RESTRAND_FLG"].ToString().TrimOrBlank();//重引锭标记
			v_cc_req_time = bcls_rec->Tables[0].Rows[i]["CC_REQ_TIME"].ToString().Trim();
			if (bcls_rec->Tables[0].Columns.Contains("ROUTEBAGKEY"))
			{
				routebagkey = bcls_rec->Tables[0].Rows[i]["ROUTEBAGKEY"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("ROUTELIST"))
			{
				routelist = bcls_rec->Tables[0].Rows[i]["ROUTELIST"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("SMELT_MODE2"))
			{
				smelt_mode2 = bcls_rec->Tables[0].Rows[i]["SMELT_MODE2"].ToString().Trim();
			}
			tpssm11["PLAN_STYLE"] = bcls_rec->Tables[0].Rows[i]["PLAN_STYLE"].ToString().Trim();
			tpssm11["STEEL_RETURN_CODE"] = bcls_rec->Tables[0].Rows[i]["STEEL_RETURN_CODE"].ToString().TrimOrBlank();
			v_smelt_mode = bcls_rec->Tables[0].Rows[i]["SMELT_MODE"].ToDecimal();


			Log::Info("", __FUNCTION__, "pono = [{0}]", v_pono);
			//Log::Info("", __FUNCTION__, "快换中包标记 tpssm11["TD_CHG_FLG"] =[{0}]", v_td_chg_flg.ToInt32());
			//Log::Info("", __FUNCTION__, "重引锭标记 tpssm10["RESTRAND_FLG"] =[{0}]", v_restrand_flg);
			//Log::Info("", __FUNCTION__, "CC_REQ_TIME =[{0}]", v_cc_req_time);
			//Log::Info("", __FUNCTION__, "tpssm11["PLAN_STYLE"] =[{0}]", tpssm11["PLAN_STYLE"].ToString());
			//Log::Info("", __FUNCTION__, "routelist =[{0}]", routelist);
			//Log::Info("", __FUNCTION__, "routebagkey =[{0}]", routebagkey);
			//Log::Info("", __FUNCTION__, "smelt_mode2 =[{0}]", smelt_mode2);

			if (v_restrand_flg.Trim() == "1")
			{
				v_restrand_flg = "T";
				v_cc_req_time = bcls_rec->Tables[0].Rows[i]["CAST_1_START_TIME"].ToString().Trim();
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

			tpssm41["FACTORY_DIV"] = v_factory_div;
			tpssm41["PONO"] = v_pono;
			sqlstr = "tpssm41.Query()";
			bool has41 = tpssm41.Query("FACTORY_DIV,PONO");//wcy 历史表数据跳过

			if (has41 == false)
			{

				if (has11 == false) //计划表中没有该炉次，新增
				{
					//新增
					inblk.Tables["PONO"].Rows[0]["PLID"] = i + 1;  //ID:参与临时计划号计算用
					inblk.Tables["PONO"].Rows[0]["PONO"] = v_pono;
					inblk.Tables["PONO"].Rows[0]["TD_CHG_FLG"] = v_td_chg_flg;
					inblk.Tables["PONO"].Rows[0]["RESTRAND_FLG"] = v_restrand_flg;
					inblk.Tables["PONO"].Rows[0]["CC_REQ_TIME"] = v_cc_req_time;
					inblk.Tables["PONO"].Rows[0]["SMELT_MODE"] = v_smelt_mode;
					inblk.Tables["PONO"].Rows[0]["ROUTEBAGKEY"] = routebagkey;
					inblk.Tables["PONO"].Rows[0]["ROUTELIST"] = routelist;
					inblk.Tables["PONO"].Rows[0]["SMELT_MODE2"] = smelt_mode2;

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
					tpssm11["ROUTEBAGKEY"] = routebagkey.TrimOrBlank();
					tpssm11["ROUTELIST"] = routelist.TrimOrBlank();
					tpssm11["SMELT_MODE2"] = smelt_mode2.TrimOrBlank();
					//tpssm11.TPD_START_TIME = v_tpd_start_time.TrimOrBlank();
					tpssm11["TD_CHG_FLG"] = v_td_chg_flg;
					tpssm11["PLAN_EDIT_FLAG"] = "U";
					tpssm11["SMELT_MODE"] = v_smelt_mode;
					sqlstr = "tpssm11.Update()";
					tpssm11.Update(
						//"TPD_START_TIME,"
						"RESTRAND_FLG,"
						"ROUTEBAGKEY,"
						"ROUTELIST,"
						"SMELT_MODE2,"
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


					tpssm11["PLAN_EDIT_FLAG"] = "U";
					sqlstr = "tpssm11.Update()";
					tpssm11.Update(
						"PLAN_EDIT_FLAG",
						"FACTORY_DIV,SM_PLAN_NO");

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
				//1、脱硫工序（甘特图单独工序管理）
				if (bcls_rec->Tables[0].Columns.Contains("SD_1_ID"))
				{
					tpssm12_ds["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_1_ID"].ToString().Trim();
					tpssm12_ds["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_1_WAIT_START_TIME"].ToString().Trim();
					tpssm12_ds["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_1_END_TIME"].ToString().Trim();
					if (tpssm12_ds["DEV_CODE"].ToString().Trim() != "")  //有脱硫设备，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["SD_1_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 1;   //1-脱硫
						row["DEV_CODE"] = tpssm12_ds["DEV_CODE"];
						row["START_TIME"] = tpssm12_ds["START_TIME"];
						row["END_TIME"] = tpssm12_ds["END_TIME"];
						if (bcls_rec->Tables[0].Rows[i]["SD_1_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}
				Log::Trace("", __FUNCTION__, "脱硫({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_ds["DEV_CODE"].ToString(), tpssm12_ds["START_TIME"].ToString(), tpssm12_ds["END_TIME"].ToString(), charge_no);

				//------------------------
				//1.5、预溶液工序
				if (bcls_rec->Tables[0].Columns.Contains("SD_2_ID")){
					tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_2_ID"].ToString().Trim();
					tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_2_WAIT_START_TIME"].ToString().Trim();
					tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_2_END_TIME"].ToString().Trim();
					//v_smelt_mode = 1;
					if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预溶液，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["SD_2_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 2;   //2-预溶液
						row["DEV_CODE"] = tpssm12_sd["DEV_CODE"];
						row["START_TIME"] = tpssm12_sd["START_TIME"];
						row["END_TIME"] = tpssm12_sd["END_TIME"];
						//v_smelt_mode = 2;
						if (bcls_rec->Tables[0].Rows[i]["SD_2_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}
				Log::Trace("", __FUNCTION__, "预溶液({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sd["DEV_CODE"].ToString(), tpssm12_sd["START_TIME"].ToString(), tpssm12_sd["END_TIME"].ToString(), charge_no);

				if (bcls_rec->Tables[0].Columns.Contains("SD_3_ID")){
					tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_3_ID"].ToString().Trim();
					tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_3_WAIT_START_TIME"].ToString().Trim();
					tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_3_END_TIME"].ToString().Trim();
					//v_smelt_mode = 1;
					if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预溶液，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["SD_3_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 2;   //2-预溶液
						row["DEV_CODE"] = tpssm12_sd["DEV_CODE"];
						row["START_TIME"] = tpssm12_sd["START_TIME"];
						row["END_TIME"] = tpssm12_sd["END_TIME"];
						//v_smelt_mode = 2;
						if (bcls_rec->Tables[0].Rows[i]["SD_3_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}
				Log::Trace("", __FUNCTION__, "预溶液({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sd["DEV_CODE"].ToString(), tpssm12_sd["START_TIME"].ToString(), tpssm12_sd["END_TIME"].ToString(), charge_no);

				if (bcls_rec->Tables[0].Columns.Contains("SD_4_ID")){
					tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_4_ID"].ToString().Trim();
					tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_4_WAIT_START_TIME"].ToString().Trim();
					tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_4_END_TIME"].ToString().Trim();
					//v_smelt_mode = 1;
					if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预溶液，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["SD_4_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 2;   //2-预溶液
						row["DEV_CODE"] = tpssm12_sd["DEV_CODE"];
						row["START_TIME"] = tpssm12_sd["START_TIME"];
						row["END_TIME"] = tpssm12_sd["END_TIME"];
						//v_smelt_mode = 2;
						if (bcls_rec->Tables[0].Rows[i]["SD_4_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}
				Log::Trace("", __FUNCTION__, "预溶液({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sd["DEV_CODE"].ToString(), tpssm12_sd["START_TIME"].ToString(), tpssm12_sd["END_TIME"].ToString(), charge_no);

				if (bcls_rec->Tables[0].Columns.Contains("SD_5_ID")){
					tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_5_ID"].ToString().Trim();
					tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_5_WAIT_START_TIME"].ToString().Trim();
					tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_5_END_TIME"].ToString().Trim();
					//v_smelt_mode = 1;
					if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预溶液，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["SD_5_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 2;   //2-预溶液
						row["DEV_CODE"] = tpssm12_sd["DEV_CODE"];
						row["START_TIME"] = tpssm12_sd["START_TIME"];
						row["END_TIME"] = tpssm12_sd["END_TIME"];
						//v_smelt_mode = 2;
						if (bcls_rec->Tables[0].Rows[i]["SD_5_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}
				Log::Trace("", __FUNCTION__, "预溶液({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sd["DEV_CODE"].ToString(), tpssm12_sd["START_TIME"].ToString(), tpssm12_sd["END_TIME"].ToString(), charge_no);

				//------------------------
				//2、转炉脱P工序（甘特图没有）
				tpssm12_dp["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_1_ID"].ToString().Trim();
				tpssm12_dp["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_WAIT_START_TIME"].ToString().Trim();
				tpssm12_dp["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_END_TIME"].ToString().Trim();
				v_smelt_mode = 1;
				if (tpssm12_dp["DEV_CODE"].ToString().Trim() != "")  //有脱P设备，则走该工序
				{
					if (bcls_rec->Tables[0].Rows[i]["LD_1_FLAG"].ToString().Trim() == "1")//新增标识
					{

					}
					else charge_no = charge_no + 1;  //指定charge号
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 2;   //2-脱P
					row["DEV_CODE"] = tpssm12_dp["DEV_CODE"];
					row["START_TIME"] = tpssm12_dp["START_TIME"];
					row["END_TIME"] = tpssm12_dp["END_TIME"];
					v_smelt_mode = 2;
					if (bcls_rec->Tables[0].Rows[i]["LD_1_FLAG"].ToString().Trim() == "1")//新增标识
					{
						row["CHARGE_NO"] = 0;
					}
					else row["CHARGE_NO"] = charge_no;
				}
				Log::Trace("", __FUNCTION__, "脱P ({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_dp["DEV_CODE"].ToString(), tpssm12_dp["START_TIME"].ToString(), tpssm12_dp["END_TIME"].ToString(), charge_no);


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
				Log::Trace("", __FUNCTION__, "转炉({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_bof["DEV_CODE"].ToString(), tpssm12_bof["START_TIME"].ToString(), tpssm12_bof["END_TIME"].ToString(), charge_no);


				//------------------------
				//4、精炼1
				tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_ID"].ToString().Trim();
				tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_START_TIME"].ToString().Trim();
				tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_1_END_TIME"].ToString().Trim();
				if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
				{
					if (bcls_rec->Tables[0].Rows[i]["FINERY_1_FLAG"].ToString().Trim() == "1")//新增标识
					{

					}
					else charge_no = charge_no + 1;  //指定charge号
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 4;   //4-精炼
					row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
					row["START_TIME"] = tpssm12_sr["START_TIME"];
					row["END_TIME"] = tpssm12_sr["END_TIME"];
					if (bcls_rec->Tables[0].Rows[i]["FINERY_1_FLAG"].ToString().Trim() == "1")//新增标识
					{
						row["CHARGE_NO"] = 0;
					}
					else row["CHARGE_NO"] = charge_no;
				}
				Log::Trace("", __FUNCTION__, "精炼1({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


				//------------------------
				//5、精炼2
				tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_ID"].ToString().Trim();
				tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_START_TIME"].ToString().Trim();
				tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_2_END_TIME"].ToString().Trim();
				if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼2设备，则走该工序
				{
					if (bcls_rec->Tables[0].Rows[i]["FINERY_2_FLAG"].ToString().Trim() == "1")//新增标识
					{

					}
					else charge_no = charge_no + 1;  //指定charge号
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 4;   //4-精炼
					row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
					row["START_TIME"] = tpssm12_sr["START_TIME"];
					row["END_TIME"] = tpssm12_sr["END_TIME"];
					if (bcls_rec->Tables[0].Rows[i]["FINERY_2_FLAG"].ToString().Trim() == "1")//新增标识
					{
						row["CHARGE_NO"] = 0;
					}
					else row["CHARGE_NO"] = charge_no;
				}
				Log::Trace("", __FUNCTION__, "精炼2({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


				//------------------------
				//6、精炼3
				tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_ID"].ToString().Trim();
				tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_START_TIME"].ToString().Trim();
				tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_3_END_TIME"].ToString().Trim();
				if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
				{
					if (bcls_rec->Tables[0].Rows[i]["FINERY_3_FLAG"].ToString().Trim() == "1")//新增标识
					{

					}
					else charge_no = charge_no + 1;  //指定charge号
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 4;   //4-精炼
					row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
					row["START_TIME"] = tpssm12_sr["START_TIME"];
					row["END_TIME"] = tpssm12_sr["END_TIME"];
					if (bcls_rec->Tables[0].Rows[i]["FINERY_3_FLAG"].ToString().Trim() == "1")//新增标识
					{
						row["CHARGE_NO"] = 0;
					}
					else row["CHARGE_NO"] = charge_no;
				}
				Log::Trace("", __FUNCTION__, "精炼3({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);


				//------------------------
				//7、精炼4
				tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_ID"].ToString().Trim();
				tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_START_TIME"].ToString().Trim();
				tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_4_END_TIME"].ToString().Trim();
				if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
				{
					if (bcls_rec->Tables[0].Rows[i]["FINERY_4_FLAG"].ToString().Trim() == "1")//新增标识
					{

					}
					else charge_no = charge_no + 1;  //指定charge号
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 4;   //4-精炼
					row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
					row["START_TIME"] = tpssm12_sr["START_TIME"];
					row["END_TIME"] = tpssm12_sr["END_TIME"];
					if (bcls_rec->Tables[0].Rows[i]["FINERY_4_FLAG"].ToString().Trim() == "1")//新增标识
					{
						row["CHARGE_NO"] = 0;
					}
					else row["CHARGE_NO"] = charge_no;
				}
				Log::Trace("", __FUNCTION__, "精炼4({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_sr["DEV_CODE"].ToString(), tpssm12_sr["START_TIME"].ToString(), tpssm12_sr["END_TIME"].ToString(), charge_no);

				//如果多于4重精炼，在此处扩展。
				if (bcls_rec->Tables[0].Columns.Contains("FINERY_5_ID")){
					tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_5_ID"].ToString().Trim();
					tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_5_START_TIME"].ToString().Trim();
					tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_5_END_TIME"].ToString().Trim();
					if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["FINERY_5_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 4;   //4-精炼
						row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
						row["START_TIME"] = tpssm12_sr["START_TIME"];
						row["END_TIME"] = tpssm12_sr["END_TIME"];
						if (bcls_rec->Tables[0].Rows[i]["FINERY_5_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}

				if (bcls_rec->Tables[0].Columns.Contains("FINERY_6_ID")){
					tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_6_ID"].ToString().Trim();
					tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_6_START_TIME"].ToString().Trim();
					tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_6_END_TIME"].ToString().Trim();
					if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["FINERY_6_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 4;   //4-精炼
						row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
						row["START_TIME"] = tpssm12_sr["START_TIME"];
						row["END_TIME"] = tpssm12_sr["END_TIME"];
						if (bcls_rec->Tables[0].Rows[i]["FINERY_6_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}

				if (bcls_rec->Tables[0].Columns.Contains("FINERY_7_ID")){
					tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_7_ID"].ToString().Trim();
					tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_7_START_TIME"].ToString().Trim();
					tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_7_END_TIME"].ToString().Trim();
					if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["FINERY_7_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 4;   //4-精炼
						row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
						row["START_TIME"] = tpssm12_sr["START_TIME"];
						row["END_TIME"] = tpssm12_sr["END_TIME"];
						if (bcls_rec->Tables[0].Rows[i]["FINERY_7_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}

				if (bcls_rec->Tables[0].Columns.Contains("FINERY_8_ID")){
					tpssm12_sr["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["FINERY_8_ID"].ToString().Trim();
					tpssm12_sr["START_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_8_START_TIME"].ToString().Trim();
					tpssm12_sr["END_TIME"] = bcls_rec->Tables[0].Rows[i]["FINERY_8_END_TIME"].ToString().Trim();
					if (tpssm12_sr["DEV_CODE"].ToString().Trim() != "")  //有精炼1设备，则走该工序
					{
						if (bcls_rec->Tables[0].Rows[i]["FINERY_8_FLAG"].ToString().Trim() == "1")//新增标识
						{

						}
						else charge_no = charge_no + 1;  //指定charge号
						CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
						row["AREA_ID"] = 4;   //4-精炼
						row["DEV_CODE"] = tpssm12_sr["DEV_CODE"];
						row["START_TIME"] = tpssm12_sr["START_TIME"];
						row["END_TIME"] = tpssm12_sr["END_TIME"];
						if (bcls_rec->Tables[0].Rows[i]["FINERY_8_FLAG"].ToString().Trim() == "1")//新增标识
						{
							row["CHARGE_NO"] = 0;
						}
						else row["CHARGE_NO"] = charge_no;
					}
				}
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
				Log::Trace("", __FUNCTION__, "连铸({3})：设备=[{0}] 开始时刻=[{1}]  结束时刻=[{2}]", tpssm12_cc["DEV_CODE"].ToString(), tpssm12_cc["START_TIME"].ToString(), tpssm12_cc["END_TIME"].ToString(), charge_no);

				//Log::Trace("", __FUNCTION__, "STEEL_RETURN_CODE=[{0}] ", tpssm11["STEEL_RETURN_CODE"].ToString());
				if (tpssm11["STEEL_RETURN_CODE"].ToString() != "1" && tpssm11["STEEL_RETURN_CODE"].ToString() != "2")
				{
					//------------------------
					//调用工序计划保存处理函数



					ret = f_pssm12_save_job_n_2(&inblk, bcls_ret, conn);  //只新增炉次
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				tpssm11["SMELT_MODE"] = v_smelt_mode;
				tpssm11.Update("SMELT_MODE", "FACTORY_DIV,PONO");
			}

		}

		//重新计算处理号
		////Log::Trace("", __FUNCTION__, "重新计算处理号");
		ret = f_pssm12_sequ_calc_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


		ret = f_pssm12_treatmentcount(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sqlstr = " UPDATE TPSSM11 SET TIME_1 = @TIME ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("TIME", datetime);
		cmd_inq.ExecuteNonQuery();
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

	return doFlag;
}
