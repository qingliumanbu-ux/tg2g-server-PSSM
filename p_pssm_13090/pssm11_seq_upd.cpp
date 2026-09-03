/*=========================================================================
//程序名称:     pssm11_seq_inq
//隶属子系统:   PSSM
//产品名称:     BM2MES
//创建人员:     ZHP
//创建时间:     2016/11/16 9:52:35
//修改人员:
//修改日期:
//=========================================================================*/
#include "stdafx.h"





/*<remark>=========================================================
/// <summary>
/// 工序顺序调整查询
/// <para>根据指定的设备，按生产次序查询该设备下的计划。            </para>
/// <para>根据计划状态，分别查询已生产的工序计划和未生产的工序计划。</para>
/// <para>数据库表：TPSSM13/14(炼钢出钢计划跟踪表)                  </para>
/// <para>主调用函数：前台FormPSSM11SeqDlg画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <param name="dev_code">炼钢设备代码              </param>
/// <returns>指定设备下的出钢计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_seq_upd)


int f_pssm11_seq_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int i = 0;

	CString  factory_div = "";
	CString dev_code = "";
	CDecimal iproc_no = 0;
	CString  datetime = "";
	int v_count = 0;
	int rows = 0;

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	CModel tpssmd1("TPSSMD1");
	CModel tpssm25("TPSSM25");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn); 
	CDbCommand cmd_tpssm25_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------------------------------------
		//获得输入参数
		dev_code = bcls_rec->Tables[1].Rows[0]["DEV_CODE"].ToString().Trim();
		factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString().Trim();

		tpssmd1["DEV_CODE"] = dev_code;
		tpssmd1["FACTORY_DIV"] = factory_div;

		sqlstr = CString(
			" SELECT COUNT(1) FROM TPSSMD1 "
			" WHERE FACTORY_DIV = @factory_div "
			"   AND DEV_CODE = @dev_code "
			"   AND AREA_ID IN (3, 4) "
			);
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.Parameters.Set("dev_code", dev_code);
		v_count = cmd_inq.ExecuteScalar().ToInt32();

		if (v_count == 0)
		{
			CFormattable arguments[] = { tpssmd1["DEV_CODE"].ToString() };
			CMessageFormat::Format(s.msg, "输入设备[{0}]找不到配置信息", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sqlstr = CString(
			" SELECT * FROM TPSSMD1 "
			" WHERE FACTORY_DIV = @factory_div "
			"   AND DEV_CODE = @dev_code "
			"   AND AREA_ID IN (3, 4) "
			);
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.Parameters.Set("dev_code", dev_code);
		v_count = cmd_inq.ExecuteReader();

		if (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);
		}
		cmd_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT CURR_PROC_NO FROM TPSSM25 "
				"  WHERE FACTORY_DIV   = @tpssmd1.FACTORY_DIV "
				"    AND STATION_ID   = @tpssmd1.STATION_ID "
				"    AND STATION_NO   = @tpssmd1.STATION_NO "
				);
			break;
		}
		cmd_tpssm25_inq.SetCommandText(sqlstr);
		cmd_tpssm25_inq.Parameters.Set("tpssmd1.FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_ID", tpssmd1["STATION_ID"].ToString());
		cmd_tpssm25_inq.Parameters.Set("tpssmd1.STATION_NO", tpssmd1["STATION_NO"].ToString());
		cmd_tpssm25_inq.ExecuteReader();
		if (cmd_tpssm25_inq.Read())
		{
			tpssm25["CURR_PROC_NO"] = cmd_tpssm25_inq.GetString(1);
		}
		else
		{
			//dclian-- - 年末两位 + 工序标志 + 工位号 + 5位流水号-- - 2016 - 02 - 26
			//tpssm25["CURR_PROC_NO"] = pre_proc_no;
			tpssm25["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
			tpssm25["STATION_ID"] = tpssmd1["STATION_ID"];
			tpssm25["STATION_NO"] = tpssmd1["STATION_NO"];
			tpssm25["CURR_PROC_NO"] = tpssm25["CURR_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)datetime.Trim().SubstringNE(2, 2), (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1),
				(const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), 0);

			////Log::Trace("", __FUNCTION__, " tpssm25["CURR_PROC_NO"] =[{0}]", (const char*)tpssm25["CURR_PROC_NO"].ToString());

			tpssm25.Insert();
		}
		cmd_tpssm25_inq.Close();

		if (tpssm25["CURR_PROC_NO"].ToString().Trim().GetLength()>0)
		{
			iproc_no = iproc_no.Parse(tpssm25["CURR_PROC_NO"].ToString().Substring(4));
		}
		else
		{
			iproc_no = 0;
		}
		////Log::Trace("", __FUNCTION__, "当前流水号iproc_no=[{0}]", iproc_no);

		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm12["FACTORY_DIV"] = factory_div;
			tpssm12["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[i]["SM_PLAN_NO"].ToString().Trim();
			tpssm12["CHARGE_NO"] = bcls_rec->Tables[0].Rows[i]["CHARGE_NO"].ToDecimal();
			tpssm12["START_TIME"] = bcls_rec->Tables[0].Rows[i]["START_TIME"].ToString().Trim();

			////Log::Trace("", __FUNCTION__, "SM_PLAN_NO=[{0}]", tpssm12["SM_PLAN_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "CHARGE_NO=[{0}]", tpssm12["CHARGE_NO"].ToDecimal());
			////Log::Trace("", __FUNCTION__, "START_TIME=[{0}]", tpssm12["START_TIME"].ToString());

			tpssm12["DEV_CODE"] = dev_code;

			//判断是否有原记录
			if (tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, DEV_CODE") == false) //没有原记录
			{
				CFormattable arguments[] = { tpssm12["SM_PLAN_NO"].ToString(), tpssm12["DEV_CODE"].ToString(), tpssm12["PROC_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "出钢计划[{0}]在设备[{1}]已进入生产，不能调整次序。", arguments, 2); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//更新预定处理号
			iproc_no = (iproc_no.ToInt32() + 1) % 100000; // 5位流水
			//sprintf(tpssm12.pre_proc_no, "%c%c%c%.5d", tpssmd1.station_id[0], tpssmd1.station_no[0], date_time[3], stream_no);
			tpssm12["PRE_PROC_NO"] = tpssm12["PRE_PROC_NO"].ToString().Format("%s%s%s%.5d", (const char*)datetime.Trim().SubstringNE(2, 2), (const char*)tpssmd1["STATION_ID"].ToString().Trim().SubstringNE(0, 1), (const char*)tpssmd1["STATION_NO"].ToString().Trim().SubstringNE(0, 1), iproc_no.ToInt32());

			////Log::Trace("", __FUNCTION__, "PRE_PROC_NO=[{0}]", tpssm12["PRE_PROC_NO"].ToString());
			tpssm12.Update("PRE_PROC_NO, START_TIME", "FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, DEV_CODE");
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
