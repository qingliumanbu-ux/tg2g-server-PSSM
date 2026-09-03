/*=========================================================================
//程序名称:	    pssm82f3_ins
//隶属子系统:   PSSM
//产品名称:	    BSM1/BM2PES
//创建人员:     zhp
//创建时间:     2016-10-16
//修改人员:
//修改日期:
//-----------------------------------------------------------------------
//功能描述:     炼钢精整命令-删除-确定
//数据库表:     tpssm81
//主调用函数:   炼钢精整命令-删除-确定
//
//需调用函数:
//-----------------------------------------------------------------------
//函数功能:     炼钢精整命令-删除-确定
//传入参数:
//传出参数:
//处理流程:
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



int f_mmsm99(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2F_ENTERACE(pssm81f5_del)

int f_pssm81f5_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	CString v_backlog_code = "";
	CDecimal v_mat_seq_no = 0;
	CString v_plan_no_pre = "";


	EIClass inBlock;
	EIClass outBlock;

	CModel tpssm81("TPSSM81");
	CModel tmmsm01("TMMSM01");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm81_inq(conn);
	CDbCommand cmd_tqmtjf1_upd(conn);
	CDbCommand cmd_tep0002_inq(conn);

	CString sqlstr;

	CTracer log(__FUNCTION__);
	try
	{
		//为物料跟踪新增一个类
		blkNum = inBlock.Tables.IndexOf("MM0099");
		////Log::Trace("", __FUNCTION__, "=== MM0099 = [{0}] ", blkNum);
		if (blkNum < 0)
		{
			inBlock.Tables.Add("MM0099");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_ID");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_LINE_TYPE");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "SYSTEM_ID");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "FUNC_ID");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "MAT_NO");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "PLAN_NO");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "REPAIR_FLAG");
		}

		userid = s.userid;
		
		//获取输入参数；
		rows = bcls_rec->Tables[0].Rows.get_Count();

		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm81["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
			tpssm81["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
			tpssm81["MAT_NO"] = bcls_rec->Tables[0].Rows[i]["MAT_NO"];

			////Log::Trace("", __FUNCTION__, "tpssm81.FACTORY_DIV[{0}],[{1}],[{2}]", tpssm81["FACTORY_DIV"].ToString(), tpssm81["PLAN_BACKLOG_CODE"].ToString(), tpssm81["MAT_NO"].ToString());

			if (tpssm81.Query("FACTORY_DIV, PLAN_BACKLOG_CODE, MAT_NO") == false)
			{
				CFormattable arguments[] = { tpssm81["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在计划中不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tpssm81["PLAN_STATUS"].ToString() > "08")
			{
				CFormattable arguments[] = { tpssm81["MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]命令状态已经下发。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm81.TrimOrBlank();

			if (tpssm81["REPAIR_FLAG"].ToString() == "1")
			{
				//修改返修处置表的记录状态：1,计划中
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:				// MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " UPDATE TQMTJF1 SET STATUS_FLAG = '0' "
						"  WHERE MAT_NO = @tpssm81.MAT_NO  "
						"	 AND SEQ_NO = "
						"				(SELECT  MAX(SEQ_NO) SEQ_NO "
						"						FROM TQMTJF1 A "
						"					WHERE STATUS_FLAG = '1' "
						"						AND IN_MAT_NO = @tpssm81.MAT_NO "
						"						AND REPAIR_BACKLOG_CODE = v_backlog_code ";
					break;
				}

				cmd_tqmtjf1_upd.SetCommandText(sqlstr);
				cmd_tqmtjf1_upd.Parameters.Set("tpssm81.MAT_NO", tpssm81["MAT_NO"].ToString());
				cmd_tqmtjf1_upd.Parameters.Set("v_backlog_code", v_backlog_code);
				cmd_tqmtjf1_upd.ExecuteNonQuery();
			}

			// 更新材料主档表当前材料的材料状态、计划号；
			//调用物料跟踪函数,修改厚板主档表，材料状态＝23，
			////Log::Trace("", __FUNCTION__, "更新材料主档表当前材料的材料状态、计划号");
			inBlock.Tables["MM0099"].Rows.Add();

			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_ID"] = "PS02";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_LINE_TYPE"] = "00";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["SYSTEM_ID"] = "PSSM";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["FUNC_ID"] = s.svc_name;
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["MAT_NO"] = tpssm81["MAT_NO"]; //材料号
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["PLAN_NO"] = tpssm81["PLAN_NO"];

			v_rownum_mm99++;

			tpssm81.Delete("FACTORY_DIV, PLAN_BACKLOG_CODE, MAT_NO");
		}

		//更新命令顺序号
		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm81["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
			tpssm81["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
			tpssm81["PLAN_NO"] = bcls_rec->Tables[0].Rows[i]["PLAN_NO"];

			if (v_plan_no_pre == tpssm81["PLAN_NO"].ToString())
			{
				continue;
			}

			//获取当前工序已释放的计划的最大计划执行顺序号,
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库			
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " SELECT NVL(MAX(MAT_SEQ_NO),0)  FROM TPSSM81 "
					" WHERE FACTORY_DIV = @tpssm81.FACTORY_DIV "
					"   AND PLAN_BACKLOG_CODE = @tpssm81.PLAN_BACKLOG_CODE "
					"   AND PLAN_NO = @tpssm81.PLAN_NO "
					"   AND PLAN_STATUS >= '08' ";
				break;
			}
			cmd_tpssm81_inq.SetCommandText(sqlstr);
			cmd_tpssm81_inq.Parameters.Set("tpssm81.FACTORY_DIV", tpssm81["FACTORY_DIV"].ToString());
			cmd_tpssm81_inq.Parameters.Set("tpssm81.PLAN_BACKLOG_CODE", tpssm81["PLAN_BACKLOG_CODE"].ToString());
			cmd_tpssm81_inq.Parameters.Set("tpssm81.PLAN_NO", tpssm81["PLAN_NO"].ToString());
			cmd_tpssm81_inq.ExecuteReader();

			if (cmd_tpssm81_inq.Read())
			{
				v_mat_seq_no = cmd_tpssm81_inq.GetDecimal(1);
			}
			cmd_tpssm81_inq.Close();

			//获取当前工序已释放的计划的最大计划执行顺序号,
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库			
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " SELECT PLAN_NO, MAT_NO FROM TPSSM81 "
					" WHERE FACTORY_DIV = @tpssm81.FACTORY_DIV "
					"   AND PLAN_BACKLOG_CODE = @tpssm81.PLAN_BACKLOG_CODE "
					"   AND PLAN_NO = @tpssm81.PLAN_NO "
					"   AND PLAN_STATUS < '08' "
					"   ORDER BY MAT_SEQ_NO ASC ";
				break;
			}
			cmd_tpssm81_inq.SetCommandText(sqlstr);
			cmd_tpssm81_inq.Parameters.Set("tpssm81.FACTORY_DIV", tpssm81["FACTORY_DIV"].ToString());
			cmd_tpssm81_inq.Parameters.Set("tpssm81.PLAN_BACKLOG_CODE", tpssm81["PLAN_BACKLOG_CODE"].ToString());
			cmd_tpssm81_inq.Parameters.Set("tpssm81.PLAN_NO", tpssm81["PLAN_NO"].ToString());
			cmd_tpssm81_inq.ExecuteReader();

			if (cmd_tpssm81_inq.Read())
			{
				tpssm81["PLAN_NO"] = cmd_tpssm81_inq.GetString(1);
				tpssm81["MAT_NO"] = cmd_tpssm81_inq.GetString(2);
				
				v_mat_seq_no = v_mat_seq_no + 1;
				tpssm81["MAT_SEQ_NO"] = v_mat_seq_no;

				tpssm81.Update("MAT_SEQ_NO","FACTORY_DIV, PLAN_BACKLOG_CODE, MAT_NO, PLAN_NO");
			}
			cmd_tpssm81_inq.Close();

			v_plan_no_pre = tpssm81["PLAN_NO"];
		}


		////Log::Trace("", __FUNCTION__, "v_rownum_mm99[{0}]", v_rownum_mm99);
		if (v_rownum_mm99 > 0)
		{
			doFlag = f_mmsm99(&inBlock, &outBlock, conn);

			if (doFlag != 0)
			{
				CFormattable arguments[] = { tpssm81["MAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PS00S0000148")/*材料[{0}]修改物料跟踪信息出错。*/, arguments, 1); //格式化字符串 
				throw CApplicationException(doFlag, s.msg, log.Location);
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
