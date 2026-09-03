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
int f_pssm12_save_job_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划之工序计划保存
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
BM2F_ENTERACE(pssm18_save_delete)

int f_pssm18_save_delete(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	CString v_pono = "";
	CString v_pono_string = "";
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

			Log::Info("", __FUNCTION__, "pono = [{0}][{1}]", v_pono, rows);

			/* ***** 检查输入参数合法性 ***** */
			if (v_pono.GetLength() <= 0)
			{
				doFlag = -11;
				//sprintf(s.msg, "收到的PONO号[%s]长度有误！",(const char*)tpssm11["PONO"].ToString());
				CFormattable arguments[] = { v_pono }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (v_pono_string.Trim() == "")
			{
				v_pono_string = "'" + v_pono + "'";
			}
			else
			{
				v_pono_string = v_pono_string + "," + "'" + v_pono + "'";
			}

		}

		Log::Info("", __FUNCTION__, "删除计划delete v_pono_string = [{0}]", v_pono_string);

		sqlstr = CString(
			" UPDATE TPSSM11 "
			"    SET PLAN_EDIT_FLAG = 'D'"
			"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
			"    AND PONO_STATUS    < 20 "
			"    AND PONO IN ( " + v_pono_string + " ) "
			);

		cmd_upd.SetCommandText(sqlstr);
		cmd_upd.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_upd.ExecuteNonQuery();

		//删除编制标志“D”的炉次，必须先删，再做后续的计算
		ret = f_pssm11_del_heat_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//重新计算浇次号
		////Log::Trace("", __FUNCTION__, "重新计算CAST号");
		ret = f_pssm21_cast_cre_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//重新计算计划号
		////Log::Trace("", __FUNCTION__, "重新计算计划号");
		/*ret = f_pssm11_planno_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}*/


		//重新计算处理号
		////Log::Trace("", __FUNCTION__, "重新计算处理号");
		/*ret = f_pssm12_sequ_calc_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}*/


		/*ret = f_pssm12_treatmentcount(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}*/

		ret = f_pssm_seq_upd(&inblk, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//inblk.Tables["PAS"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		//inblk.Tables["PAS"].Rows[0]["OPER_FLAG"] = "I";//新增

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
