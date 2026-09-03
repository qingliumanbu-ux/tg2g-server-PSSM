/*=========================================================================
//程序名称:	    pssm82f3_ins
//隶属子系统:   PSSM
//产品名称:	    BSM1/BM2PES
//创建人员:     zhp
//创建时间:     2016-10-16
//修改人员:
//修改日期:
//-----------------------------------------------------------------------
//功能描述:     炼钢精整命令-新增-确定
//数据库表:     tpssm05
//主调用函数:   炼钢精整命令-新增-确定
//
//需调用函数:
//-----------------------------------------------------------------------
//函数功能:     炼钢精整命令-新增-确定
//传入参数:
//传出参数:
//处理流程:
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



#if defined _LINE_HP

#endif



//合同跟踪函数
int f_pmof99_v3(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//物料跟踪
int f_mmsm99(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2F_ENTERACE(pssm25add_ins)

int f_pssm25add_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount, samprow;
	int blkNum = 0;
	int i = 0;
	int fetchRowCount1 = 0;
	int fetchRowCount2 = 0;
	int rows = 0;

	int v_rownum_pm99 = 0;
	int v_rownum_mm99 = 0;

	CString userid = " ";                  /* 登陆用户 */
	int v_cnt = 0;
	CDecimal v_cnt1 = 0;
	int v_cnt2 = 0;
	CDecimal v_cnt3 = 0;
	int v_mat_num = 0;
	CDecimal v_mat_wt = 0;
	CString v_errmsg = " ";         /* 错误信息 */
	CDecimal v_plan_exec_seq_no_max = 0;      /* 新顺序号 */
	int v_seq = 0;
	CString v_product_code = "";
	CString v_backlog_code = "";
	CString v_plan_no = " "; 				/* 计划号*/
	CString v_factory_div = " ";
	CString v_plan_backlog_code = " ";
	CString v_plan_date = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString v_year = datetime.Substring(3, 1);	 /* 年 */
	CString v_month = datetime.Substring(4, 2); /* 月 */
	CString v_day = datetime.Substring(6, 2); /* 日 */
	CDecimal v_mat_seq_no = 0;


	EIClass inMMSM;
	EIClass outMMSM;
	
	CModel tpssm05("TPSSM05");
	CModel tpmof03("TPMOF03");
	CModel tmmsm01("TMMSM01");


#if defined _LINE_HP
	CModel tmmhp13("TMMHP13");
#endif

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm05_inq(conn);
	CDbCommand cmd_tmmhp13_inq(conn);
	CDbCommand cmd_tom01_inq(conn);

	CString sqlstr;

	CTracer log(__FUNCTION__);
	try
	{
		//为物料跟踪新增一个类
		blkNum = inMMSM.Tables.IndexOf("MM0099");
		////Log::Trace("", __FUNCTION__, "=== MM0099 = [{0}] ", blkNum);
		if (blkNum < 0)
		{
			inMMSM.Tables.Add("MM0099");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_ID");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_LINE_TYPE");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "SYSTEM_ID");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "FUNC_ID");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "MAT_NO");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "PLAN_NO");
			inMMSM.Tables["MM0099"].Columns.Add(DT_STRING, "REPAIR_FLAG");
		}

		//----------------------------------------------------------------------
		//调用合同跟踪：   
		//----------------------------------------------------------------------
		//抛合同跟着新增类
		EIClass inPMOF99;
		EIClass outPMOF99;
		int v_rownum_pm99 = 0;
		inPMOF99.Tables.Add();
		//----------------------------------------------------------------------

		userid = s.userid;

		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		v_plan_backlog_code = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
		v_plan_date = bcls_rec->Tables[0].Rows[0]["PLAN_PROD_TIME"];

		////Log::Trace("", __FUNCTION__, "tpssm05.FACTORY_DIV[{0}],[{1}],[{2}]", v_factory_div, v_plan_backlog_code, v_plan_date);

		//机组号4＋年 1 ＋月1 ＋日2 +流水号2（01）
		v_plan_no = tpssm05["PLAN_BACKLOG_CODE"].ToString() + v_year;
		if (v_month == "10")
			v_plan_no = v_plan_no + "A";
		else if (v_month == "11")
			v_plan_no = v_plan_no + "B";
		else if (v_month == "12")
			v_plan_no = v_plan_no + "C";

		v_plan_no = v_plan_no + v_month.Substring(1, 1) + v_day + "01";

		////Log::Info("", __FUNCTION__, "v_plan_no = [{0}]", v_plan_no);
		
		tpssm05["FACTORY_DIV"] = v_factory_div;
		tpssm05["PLAN_BACKLOG_CODE"] = "AE";
		tpssm05["PLAN_MAKE_TIME"] = datetime;
		tpssm05["PLAN_DATE"] = v_plan_date;

		//获取当前工序已释放的计划的最大计划执行顺序号,
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库			
		default: // 所有数据库适用，通用SQL语句
			sqlstr = " SELECT NVL(MAX(MAT_SEQ_NO),0)  FROM TPSSM05 "
				" WHERE FACTORY_DIV = @tpssm05.FACTORY_DIV "
				"   AND PLAN_BACKLOG_CODE = @tpssm05.PLAN_BACKLOG_CODE "
				"   AND PLAN_NO = @v_plan_no ";
			break;
		}
		cmd_tpssm05_inq.SetCommandText(sqlstr);
		cmd_tpssm05_inq.Parameters.Set("tpssm05.FACTORY_DIV", tpssm05["FACTORY_DIV"].ToString());
		cmd_tpssm05_inq.Parameters.Set("tpssm05.PLAN_BACKLOG_CODE", tpssm05["PLAN_BACKLOG_CODE"].ToString());
		cmd_tpssm05_inq.Parameters.Set("v_plan_no", v_plan_no);
		cmd_tpssm05_inq.ExecuteReader();

		if (cmd_tpssm05_inq.Read())
		{
			v_mat_seq_no = cmd_tpssm05_inq.GetDecimal(1);
		}
		cmd_tpssm05_inq.Close();

		////Log::Info("", __FUNCTION__, "v_mat_seq_no = [{0}]", v_mat_seq_no);

		//2.  获取第一块输入参数；
		rows = bcls_rec->Tables[0].Rows.get_Count();

		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tmmsm01["MAT_NO"] = bcls_rec->Tables[0].Rows[i]["MAT_NO"];

			////Log::Info("", __FUNCTION__, "tmmsm01["MAT_NO"] = [{0}]", tmmsm01["MAT_NO"].ToString());

			if (tmmsm01.Query("MAT_NO") == false)
			{
				CFormattable arguments[] = { tmmsm01["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在主档不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm05.CopyFrom(tmmsm01);

			if (tmmsm01["MAT_STATUS"].ToString() != "23")
			{
				CFormattable arguments[] = { tmmsm01["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]状态必须在可编计划状态。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			//2获取合同信息
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT ORDER_THICK,  "
					"	ORDER_WIDTH, "
					"	ORDER_LEN, "
					"   ORDER_UNIT_AIM_WT, "
					"   INGOT_CODE "
					"   FROM TOM01 "
					"  WHERE ORDER_NO = @tpssm05.ORDER_NO "
					;
				break;
			}
			cmd_tom01_inq.SetCommandText(sqlstr);
			cmd_tom01_inq.Parameters.Set("tpssm05.ORDER_NO", tpssm05["ORDER_NO"].ToString());
			cmd_tom01_inq.ExecuteReader();
			if (cmd_tom01_inq.Read())
			{
				tpssm05["OUT_MAT_THICK"] = cmd_tom01_inq.GetDecimal(1);
				tpssm05["OUT_MAT_WIDTH"] = cmd_tom01_inq.GetDecimal(2);
				tpssm05["OUT_MAT_LEN"] = cmd_tom01_inq.GetDecimal(3);
				tpssm05["OUT_MAT_WT"] = cmd_tom01_inq.GetDecimal(4);
				tpssm05["INGOT_CODE"] = cmd_tom01_inq.GetString(5);

				tpssm05["OUT_MAT_MIN_LEN"] = tpssm05["OUT_MAT_LEN"];
				tpssm05["OUT_MAT_MAX_LEN"] = tpssm05["OUT_MAT_LEN"];
				tpssm05["OUT_MAT_MIN_WIDTH"] = tpssm05["OUT_MAT_WIDTH"];
				tpssm05["OUT_MAT_MAX_WIDTH"] = tpssm05["OUT_MAT_WIDTH"];
				tpssm05["OUT_MAT_MAX_THICK"] = tpssm05["OUT_MAT_THICK"];
				tpssm05["OUT_MAT_MIN_THICK"] = tpssm05["OUT_MAT_THICK"];
				tpssm05["OUT_MAT_MIN_WT"] = tpssm05["OUT_MAT_WT"];
				tpssm05["OUT_MAT_MAX_WT"] = tpssm05["OUT_MAT_WT"];
			}
			else
			{
				cmd_tom01_inq.Close();
				/*PS00S0000887 材料[{0}]没有合同信息，不能编入计划。*/
				CFormattable arguments[] = { tpssm05["ORDER_NO"].ToString() };
				CMessageFormat::Format(s.msg, "未查询到材料对应的合同信息。{0}", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			cmd_tom01_inq.Close();

			////Log::Info("", __FUNCTION__, "tmmsm01["MAT_NO"] = [{0}]", tmmsm01["MAT_NO"].ToString());

#if defined _LINE_HP
			//3获取电极坯实绩信息
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT ACT_SLAB_NO1,  "
					"	ACT_SLAB_NO3, "
					"	ACT_SLAB_NO4, "
					"   ACT_SLAB_NO4, "
					"   ACT_SLAB_NO5 "
					"   FROM TMMHP13 "
					"  WHERE MAT_NO = @tmmsm01.MAT_NO "
					;
				break;
			}
			cmd_tmmhp13_inq.SetCommandText(sqlstr);
			cmd_tmmhp13_inq.Parameters.Set("tmmsm01.MAT_NO", tmmsm01["MAT_NO"].ToString());
			cmd_tmmhp13_inq.ExecuteReader();
			if (cmd_tmmhp13_inq.Read())
			{
				tmmhp13["ACT_SLAB_NO1"] = cmd_tmmhp13_inq.GetString(1);
				tmmhp13["ACT_SLAB_NO2"] = cmd_tmmhp13_inq.GetString(2);
				tmmhp13["ACT_SLAB_NO3"] = cmd_tmmhp13_inq.GetString(3);
				tmmhp13["ACT_SLAB_NO4"] = cmd_tmmhp13_inq.GetString(4);
				tmmhp13["ACT_SLAB_NO5"] = cmd_tmmhp13_inq.GetString(5);
			}
			else
			{
				cmd_tmmhp13_inq.Close();
				/*PS00S0000887 材料[{0}]没有合同信息，不能编入计划。*/
				CFormattable arguments[] = { tpssm05["IN_MAT_NO"].ToString() };
				CMessageFormat::Format(s.msg, "未查询到材料对应的电极坯实绩信息。{0}", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			cmd_tmmhp13_inq.Close();
#endif


			tpssm05["REC_CREATOR"] = s.userid;
			tpssm05["REC_CREATE_TIME"] = datetime;
			tpssm05["REC_REVISOR"] = s.userid;
			tpssm05["REC_REVISE_TIME"] = datetime;

			v_mat_seq_no = v_mat_seq_no + 1;
			tpssm05["IN_MAT_NO"] = tmmsm01["MAT_NO"];
			tpssm05["PLAN_NO"] = v_plan_no;
			tpssm05["MAT_SEQ_NO"] = v_mat_seq_no;
			tpssm05["PLAN_STATUS"] = "04";

			// 更新材料主档表当前材料的材料状态、计划号；
			//调用物料跟踪函数,修改厚板主档表，材料状态＝23，
			////Log::Trace("", __FUNCTION__, "更新材料主档表当前材料的材料状态、计划号");
			inMMSM.Tables["MM0099"].Rows.Add();

			inMMSM.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_ID"] = "PS01";
			inMMSM.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_LINE_TYPE"] = "00";
			inMMSM.Tables["MM0099"].Rows[v_rownum_mm99]["SYSTEM_ID"] = "PSSM";
			inMMSM.Tables["MM0099"].Rows[v_rownum_mm99]["FUNC_ID"] = s.svc_name;
			inMMSM.Tables["MM0099"].Rows[v_rownum_mm99]["MAT_NO"] = tpssm05["IN_MAT_NO"]; //材料号
			inMMSM.Tables["MM0099"].Rows[v_rownum_mm99]["PLAN_NO"] = tpssm05["PLAN_NO"];

			v_rownum_mm99++;

			tpmof03["ORDER_NO"] = tmmsm01["ORDER_NO"];
			tpmof03["WHOLE_BACKLOG_NO"] = tmmsm01["WHOLE_BACKLOG_NO"];
			tpmof03["NUM"] = tmmsm01["MAT_NUM"];
			tpmof03["WT"] = tmmsm01["MAT_ACT_WT"];
			tpmof03["WHOLE_BACKLOG_CODE"] = tmmsm01["NEXT_WHOLE_BACKLOG_CODE"];
			tpmof03["WHOLE_BACKLOG_SEQ"] = tmmsm01["NEXT_WHOLE_BACKLOG_SEQ"];
			tpmof03["MAT_STATUS"] = tmmsm01["MAT_STATUS"];
			tpmof03["WHOLE_BACKLOG"] = tmmsm01["WHOLE_BACKLOG"];

			tpmof03["MAT_NO"] = tmmsm01["MAT_NO"];
			tpmof03["CUST_MAT_NO"] = tmmsm01["MAT_NO"];
			tpmof03["SUB_BACKLOG_CODE"] = tmmsm01["NEXT_SUB_BACKLOG_CODE"];
			tpmof03["SUB_BACKLOG_SEQ"] = tmmsm01["NEXT_SUB_BACKLOG_SEQ"];

			tpmof03["CALL_FLAG"] = 1;  //只抛合同跟踪

			tpmof03["EVENT_ID"] = "50";

			tpmof03["SYSTEM_ID"] = "PS";
			tpmof03["FUNC_ID"] = "pssm25add_ins";

			tpmof03.TrimOrBlank();
			tpmof03.MergeTo(inPMOF99.Tables[0], false);

			v_rownum_pm99++;

			tpssm05.TrimOrBlank();
			tpssm05.Insert();
		}

		////Log::Trace("", __FUNCTION__, "v_rownum_mm99[{0}]", v_rownum_mm99);
		if (v_rownum_mm99 > 0)
		{
			doFlag = f_mmsm99(&inMMSM, &outMMSM, conn);

			if (doFlag != 0)
			{
				CFormattable arguments[] = { tpssm05["IN_MAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PS00S0000148")/*材料[{0}]修改物料跟踪信息出错。*/, arguments, 1); //格式化字符串 
				throw CApplicationException(doFlag, s.msg, log.Location);
			}
		}

		//调用合同跟踪	---------------------------------------------------------
		if (v_rownum_pm99 > 0)
		{
			doFlag = f_pmof99_v3(&inPMOF99, &outPMOF99, conn);
			if (doFlag != 0)
			{
				//合同跟踪失败
				////Log::Trace("", __FUNCTION__, "=== = 合同跟踪出错");
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

	return (doFlag);

}
