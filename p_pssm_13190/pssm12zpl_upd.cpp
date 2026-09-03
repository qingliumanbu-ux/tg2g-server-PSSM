/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-26
Version:  3.1.0
Description: 炼钢计划炉次计划更新
**************************************************/
//标准工程头文件不可删除
#include "stdafx.h"

//工程头文件

// [优化] 增加STL容器头文件，用于缓存查询结果和批量更新数据
#include <vector>
#include <map>
#include <utility>




//#include "tpssm26.h"


/*<<remark>=========================================================
/// <summary>
/// 炼钢计划炉次计划更新
/// <para>接收Client炼钢图形化界面数据，修改出钢计划信息  </para>
/// <para>
///   1.出钢计划信息写入:子计划与工序计划
///   2.删除多余出钢炉次
///   3.CAST炉次计划
///   4.计划炉次计划
///   5.精炼炉次计划
/// <para>数据库表为TPSSM11/12                   </para>
/// <para>关联调用函数PSSM18等使用。                </para>
/// </summary>
/// <param name="main_backlog_code">主作业计划号     </param>
/// <returns>返回处理结果</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12zpl_upd)

// [优化] 批量更新记录缓存结构
struct Tpssm12UpdRec {
	CString sm_plan_no;
	CString split_indication;
	CString charge_no;
	CString dev_code;
	CString start_time;
	CString end_time;
	int area_id;
};

int f_pssm12zpl_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//函数返回变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "LG1";	//工厂单元号
	CString v_pono = "";
	CString v_restrand_flg = "";     //重排标志
	CString v_cc_req_time = "";  //连铸请求时间
	CString v_tpd_start_time = "";  //脱硫开始时间
	CDecimal v_td_chg_flg = 0, v_smelt_mode = 0;
	CDecimal charge_no = 0;      //炉次charge号
	CString datetime = "";
	CString routebagkey = "";
	CString routelist = "";
	CString smelt_mode2 = "";
	CString source_flag = "";

	EIClass inblk;        //调用函数用
	EIClass in_pssm99trace;//跟踪信息用
	EIClass in_pssm18;//调用发送停机实绩的函数
	//EIClass TPSSMD1_SOURCE;
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm18_del(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CModel tpssm12_ds("TPSSM12_ZPL");//脱硫
	CModel tpssm12_sd("TPSSM12_ZPL");//预处理液
	CModel tpssm12_dp("TPSSM12_ZPL");//转炉处理
	CModel tpssm12_bof("TPSSM12_ZPL");//转炉/BOF处理
	CModel tpssm12("TPSSM12_ZPL");
	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");
	CDataTable tb_tpssm11("TPSSM11");
	CDataTable &table2 = inblk.Tables.Add("TPSSM12");  //子计划信息新增记录
	table2.Columns.Add(DT_DECIMAL, "AREA_ID");      //区域标识
	table2.Columns.Add(DT_STRING, "DEV_CODE");      //设备代码
	table2.Columns.Add(DT_STRING, "START_TIME");    //开始时间
	table2.Columns.Add(DT_STRING, "END_TIME");      //结束时间

	// [优化] 批量更新数据缓存
	std::vector<Tpssm12UpdRec> updRecList;
	updRecList.reserve(64);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		rows = bcls_rec->Tables[0].Rows.get_Count();

		Log::Trace("", __FUNCTION__, "优化后的新");
		// ============================================================
		// [优化1] 预收集所有PONO，循环外批量查询TPSSM11/TPSSM41
		// 消除原循环内逐条查询导致的N+1性能问题
		// ============================================================
		std::vector<CString> ponoList;
		ponoList.reserve(rows);
		for (i = 0; i < rows; i++) {
			CString tmpPono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();
			if (!tmpPono.IsEmpty()) {
				ponoList.push_back(tmpPono);
			}
		}

		// 缓存查询结果: pono -> pair<<sm_plan_no, split_indication>
		std::map<CString, std::pair<CString, CString>> cache11;
		std::map<CString, std::pair<CString, CString>> cache41;

		if (!ponoList.empty()) {
			// 分批查询，每批最多50个PONO，避免IN子句过长导致SQL解析问题
			const size_t PONO_BATCH = 50;
			for (size_t batchStart = 0; batchStart < ponoList.size(); batchStart += PONO_BATCH) {
				CString inClause;
				size_t batchEnd = batchStart + PONO_BATCH;
				if (batchEnd > ponoList.size()) batchEnd = ponoList.size();

				for (size_t j = batchStart; j < batchEnd; j++) {
					if (j > batchStart) inClause += ",";
					// 简单转义单引号，防止SQL注入
					CString safePono = ponoList[j];
					safePono.Replace("'", "''");
					inClause += "'" + safePono + "'";
				}

				//Log::Trace("", __FUNCTION__, "inClause = [{0}]", inClause);

				// 批量查询TPSSM11
				CString query11 = CString(
					" SELECT FACTORY_DIV,PONO,SM_PLAN_NO,SPLIT_INDICATION FROM TPSSM11  "
					"   WHERE FACTORY_DIV = @v_factory_div AND PONO IN (@v_pono) "
					);
				

				CDataTable dt11("TPSSM11");
				cmd_inq.SetCommandText(query11);
				cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
				cmd_inq.Parameters.Set("v_pono", inClause);

				//Log::Trace("", __FUNCTION__, "TPSSM11的sqlstr = [{0}]", query11);

				cmd_inq.ExecuteQuery(dt11);

				int qRows = dt11.Rows.get_Count();
				for (int r = 0; r < qRows; r++) {
					CString key = dt11.Rows[r]["PONO"].ToString().Trim();
					CString planNo = dt11.Rows[r]["SM_PLAN_NO"].ToString().Trim();
					CString splitInd = dt11.Rows[r]["SPLIT_INDICATION"].ToString().Trim();
					cache11[key] = std::make_pair(planNo, splitInd);
				}

				// 批量查询TPSSM41
				CString query41 = CString(
					" SELECT FACTORY_DIV,PONO,SM_PLAN_NO,SPLIT_INDICATION FROM TPSSM41  "
					"   WHERE FACTORY_DIV = @v_factory_div AND PONO IN (@v_pono) "
					);

				

				CDataTable dt41("TPSSM41");
				cmd_inq.SetCommandText(query41);
				cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
				cmd_inq.Parameters.Set("v_pono", inClause);

				//Log::Trace("", __FUNCTION__, "TPSSM41的sqlstr = [{0}]", query41);

				cmd_inq.ExecuteQuery(dt41);


				qRows = dt41.Rows.get_Count();
				for (int r = 0; r < qRows; r++) {
					CString key = dt41.Rows[r]["PONO"].ToString().Trim();
					CString planNo = dt41.Rows[r]["SM_PLAN_NO"].ToString().Trim();
					CString splitInd = dt41.Rows[r]["SPLIT_INDICATION"].ToString().Trim();
					cache41[key] = std::make_pair(planNo, splitInd);
				}
			}
		}
		// ============================================================
		//Log::Trace("", __FUNCTION__, "查询结束");

		for (i = 0; i < rows; i++)
		{
			v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();
			v_td_chg_flg = bcls_rec->Tables[0].Rows[i]["TD_CHG_FLG"].ToDecimal();//铁水变更标志
			v_restrand_flg = bcls_rec->Tables[0].Rows[i]["RESTRAND_FLG"].ToString().TrimOrBlank();//重排标志
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
			v_smelt_mode = bcls_rec->Tables[0].Rows[i]["SMELT_MODE"].ToDecimal();

			charge_no = 0;

			// [优化] 从缓存获取计划信息，替代循环内单条Query
			CString v_sm_plan_no = "";
			CString v_split_indication = "";
			bool has41 = (cache41.find(v_pono) != cache41.end());
			bool has11 = (cache11.find(v_pono) != cache11.end());

			if (has41 == false)
			{
				if (has11 == false) //计划表中没有该炉次，报错
				{
					//Log::Trace("", __FUNCTION__, "无计划", v_pono);
					continue; // 跳过无计划记录，避免后续Update使用脏数据
				}
				else
				{
					v_sm_plan_no = cache11[v_pono].first;
					v_split_indication = cache11[v_pono].second;
				}
			}
			else
			{
				v_sm_plan_no = cache41[v_pono].first;
				v_split_indication = cache41[v_pono].second;
			}

			// 为各模型设置计划号（保持与原代码一致，便于后续扩展或调试）
			tpssm12_sd["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_sd["SPLIT_INDICATION"] = v_split_indication;
			tpssm12_dp["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_dp["SPLIT_INDICATION"] = v_split_indication;
			tpssm12_bof["SM_PLAN_NO"] = v_sm_plan_no;
			tpssm12_bof["SPLIT_INDICATION"] = v_split_indication;

			inblk.Tables["TPSSM12"].Rows.Clear();

			//------------------------
			//1、脱硫区（根据图没有，但代码保留，仅charge计数）
			if (bcls_rec->Tables[0].Columns.Contains("SD_1_ID"))
			{
				tpssm12_ds["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_1_ID"].ToString().Trim();
				tpssm12_ds["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_1_WAIT_START_TIME"].ToString().Trim();
				tpssm12_ds["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_1_END_TIME"].ToString().Trim();
				if (tpssm12_ds["DEV_CODE"].ToString().Trim() != "")  //有脱硫设备代码或该工序
				{
					charge_no = charge_no + 1;  //指定charge号
				}
			}

			//------------------------
			//1.5、预处理液区
			if (bcls_rec->Tables[0].Columns.Contains("SD_2_ID")){
				tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_2_ID"].ToString().Trim();
				tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_2_WAIT_START_TIME"].ToString().Trim();
				tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_2_END_TIME"].ToString().Trim();
				if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预处理液设备或该工序
				{
					charge_no = charge_no + 1;
					Tpssm12UpdRec rec;
					rec.sm_plan_no = v_sm_plan_no;
					rec.split_indication = v_split_indication;
					rec.charge_no.Format("%d", charge_no);
					rec.dev_code = tpssm12_sd["DEV_CODE"].ToString().Trim();
					rec.start_time = tpssm12_sd["START_TIME"].ToString().Trim();
					rec.end_time = tpssm12_sd["END_TIME"].ToString().Trim();
					rec.area_id = 2;
					updRecList.push_back(rec);
				}
			}

			if (bcls_rec->Tables[0].Columns.Contains("SD_3_ID")){
				tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_3_ID"].ToString().Trim();
				tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_3_WAIT_START_TIME"].ToString().Trim();
				tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_3_END_TIME"].ToString().Trim();
				if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预处理液设备或该工序
				{
					charge_no = charge_no + 1;
					Tpssm12UpdRec rec;
					rec.sm_plan_no = v_sm_plan_no;
					rec.split_indication = v_split_indication;
					rec.charge_no.Format("%d", charge_no);
					rec.dev_code = tpssm12_sd["DEV_CODE"].ToString().Trim();
					rec.start_time = tpssm12_sd["START_TIME"].ToString().Trim();
					rec.end_time = tpssm12_sd["END_TIME"].ToString().Trim();
					rec.area_id = 2;
					updRecList.push_back(rec);
				}
			}

			if (bcls_rec->Tables[0].Columns.Contains("SD_4_ID")){
				tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_4_ID"].ToString().Trim();
				tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_4_WAIT_START_TIME"].ToString().Trim();
				tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_4_END_TIME"].ToString().Trim();
				if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预处理液设备或该工序
				{
					charge_no = charge_no + 1;
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 2;   //2-预处理液
					row["DEV_CODE"] = tpssm12_sd["DEV_CODE"];
					row["START_TIME"] = tpssm12_sd["START_TIME"];
					row["END_TIME"] = tpssm12_sd["END_TIME"];

					Tpssm12UpdRec rec;
					rec.sm_plan_no = v_sm_plan_no;
					rec.split_indication = v_split_indication;
					rec.charge_no.Format("%d", charge_no);
					rec.dev_code = tpssm12_sd["DEV_CODE"].ToString().Trim();
					rec.start_time = tpssm12_sd["START_TIME"].ToString().Trim();
					rec.end_time = tpssm12_sd["END_TIME"].ToString().Trim();
					rec.area_id = 2;
					updRecList.push_back(rec);
				}
			}

			if (bcls_rec->Tables[0].Columns.Contains("SD_5_ID")){
				tpssm12_sd["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["SD_5_ID"].ToString().Trim();
				tpssm12_sd["START_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_5_WAIT_START_TIME"].ToString().Trim();
				tpssm12_sd["END_TIME"] = bcls_rec->Tables[0].Rows[i]["SD_5_END_TIME"].ToString().Trim();
				if (tpssm12_sd["DEV_CODE"].ToString().Trim() != "")  //有预处理液设备或该工序
				{
					charge_no = charge_no + 1;
					CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
					row["AREA_ID"] = 2;   //2-预处理液
					row["DEV_CODE"] = tpssm12_sd["DEV_CODE"];
					row["START_TIME"] = tpssm12_sd["START_TIME"];
					row["END_TIME"] = tpssm12_sd["END_TIME"];

					Tpssm12UpdRec rec;
					rec.sm_plan_no = v_sm_plan_no;
					rec.split_indication = v_split_indication;
					rec.charge_no.Format("%d", charge_no);
					rec.dev_code = tpssm12_sd["DEV_CODE"].ToString().Trim();
					rec.start_time = tpssm12_sd["START_TIME"].ToString().Trim();
					rec.end_time = tpssm12_sd["END_TIME"].ToString().Trim();
					rec.area_id = 2;
					updRecList.push_back(rec);
				}
			}

			//------------------------
			//2、转炉P区（根据图没有，但代码保留）
			tpssm12_dp["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_1_ID"].ToString().Trim();
			tpssm12_dp["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_WAIT_START_TIME"].ToString().Trim();
			tpssm12_dp["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_1_END_TIME"].ToString().Trim();
			v_smelt_mode = 1;
			if (tpssm12_dp["DEV_CODE"].ToString().Trim() != "")  //有转炉P设备代码或该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 2;   //2-转P
				row["DEV_CODE"] = tpssm12_dp["DEV_CODE"];
				row["START_TIME"] = tpssm12_dp["START_TIME"];
				row["END_TIME"] = tpssm12_dp["END_TIME"];

				Tpssm12UpdRec rec;
				rec.sm_plan_no = v_sm_plan_no;
				rec.split_indication = v_split_indication;
				rec.charge_no.Format("%d", charge_no);
				rec.dev_code = tpssm12_dp["DEV_CODE"].ToString().Trim();
				rec.start_time = tpssm12_dp["START_TIME"].ToString().Trim();
				rec.end_time = tpssm12_dp["END_TIME"].ToString().Trim();
				rec.area_id = 2;
				updRecList.push_back(rec);
				v_smelt_mode = 2;
			}

			//------------------------
			//3、转炉C/BOF区（不能为空）
			tpssm12_bof["DEV_CODE"] = bcls_rec->Tables[0].Rows[i]["LD_2_ID"].ToString().Trim();
			tpssm12_bof["START_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_2_WAIT_START_TIME"].ToString().Trim();
			tpssm12_bof["END_TIME"] = bcls_rec->Tables[0].Rows[i]["LD_2_END_TIME"].ToString().Trim();
			if (tpssm12_bof["DEV_CODE"].ToString().Trim() != "")  //有转炉设备代码或该工序
			{
				charge_no = charge_no + 1;
				CDataRow &row = inblk.Tables["TPSSM12"].Rows.Add();
				row["AREA_ID"] = 3;   //3-转C
				row["DEV_CODE"] = tpssm12_bof["DEV_CODE"];
				row["START_TIME"] = tpssm12_bof["START_TIME"];
				row["END_TIME"] = tpssm12_bof["END_TIME"];

				Tpssm12UpdRec rec;
				rec.sm_plan_no = v_sm_plan_no;
				rec.split_indication = v_split_indication;
				rec.charge_no.Format("%d", charge_no);
				rec.dev_code = tpssm12_bof["DEV_CODE"].ToString().Trim();
				rec.start_time = tpssm12_bof["START_TIME"].ToString().Trim();
				rec.end_time = tpssm12_bof["END_TIME"].ToString().Trim();
				rec.area_id = 3;
				updRecList.push_back(rec);
				v_smelt_mode = 2;
			}
			else
			{
				CFormattable arguments[] = { v_pono, tpssm12_bof["DEV_CODE"].ToString() }; // 格式化参数列表，对应消息
				CMessageFormat::Format(s.msg, "炼钢计划炉次[{0}]转炉/BOF设备代码为空, 请检查输入数据", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		//Log::Trace("", __FUNCTION__, "准备更新");
		// ============================================================
		// [优化2] 循环外批量执行UPDATE
		// 将循环内N次数据库往返压缩为 ceil(N/100) 次，大幅降低执行时间。
		// 
		// 【数据库适配说明】
		// 1) SQL Server / MySQL: 当前代码直接支持（分号分隔多语句）。
		// 2) Oracle: 需要将每批SQL外层包裹为 PL/SQL 匿名块：
		//    BEGIN
		//      UPDATE ...;
		//      UPDATE ...;
		//    END;
		//    只需将下面 batchSql 的拼接逻辑改为 "BEGIN\n" + singleSql + "END;\n" 即可。
		// 3) 如需极致性能（万级数据），建议使用“临时表+单条UPDATE”方案，
		//    仅需2~3次数据库交互，具体可联系DBA实施。
		// ============================================================
		if (!updRecList.empty()) {
			const size_t SQL_BATCH_SIZE = 100;
			CString batchSql;

			// [修正] 根据数据库类型决定是否使用 PL/SQL 块
			// 如果是 Oracle，设置为 true；SQL Server/MySQL 可保持 false
			bool isOracle = true;  // ← 请根据实际数据库类型修改此开关

			for (size_t idx = 0; idx < updRecList.size(); idx++) {
				Tpssm12UpdRec &rec = updRecList[idx];

				// 单引号转义
				CString safeDev = rec.dev_code;   safeDev.Replace("'", "''");
				CString safeStart = rec.start_time; safeStart.Replace("'", "''");
				CString safeEnd = rec.end_time;     safeEnd.Replace("'", "''");
				CString safePlan = rec.sm_plan_no;  safePlan.Replace("'", "''");
				CString safeSplit = rec.split_indication; safeSplit.Replace("'", "''");

				CString singleSql = CString(
					" UPDATE TPSSM12_ZPL SET DEV_CODE='" + safeDev + "', START_TIME='" + safeStart + "', END_TIME='" + safeEnd + "'  "
					"   WHERE CHARGE_NO='" + rec.charge_no + "' AND SM_PLAN_NO='" + safePlan + "' AND SPLIT_INDICATION='" + safeSplit + "'; "
					);

				batchSql += singleSql + "\n";

				// 达到批次上限或最后一条时执行
				if ((idx + 1) % SQL_BATCH_SIZE == 0 || idx == updRecList.size() - 1) {

					CString finalSql;

					if (isOracle) {
						// ✅ Oracle 必须用 BEGIN/END 包裹多条 UPDATE
						finalSql = "BEGIN\n" + batchSql + "END;";
					}
					else {
						// SQL Server / MySQL 可直接执行分号拼接的多语句
						finalSql = batchSql;
					}

					cmd_upd.SetCommandText(finalSql);
					cmd_upd.ExecuteNonQuery();
					batchSql.Empty();
				}
			}
		}
		//Log::Trace("", __FUNCTION__, "更新结束");
		// ============================================================
	}
	catch (CDbException& ex)  //捕获数据库处理异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理发生错误sqlcode=[{0}]，请联系系统管理员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台错误EI.EIInfo结构体sys_info.sysmsg中，对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将回滚
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