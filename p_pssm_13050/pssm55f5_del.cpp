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
//数据库表:     tpssm05
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


//#include "tpmof03.h"

//合同跟踪函数
//int f_pmof99_v3(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//物料跟踪
int f_mmsm99(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2F_ENTERACE(pssm55f5_del)

int f_pssm55f5_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	CModel tpssm05("TPSSM05");
	CModel tmmsm01("TMMSM01");
	CModel tpmof03("TPMOF03");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm05_inq(conn);
	CDbCommand cmd_tqmtjf1_upd(conn);
	CDbCommand cmd_tep0002_inq(conn);

	CString sqlstr;

	CTracer log(__FUNCTION__);
	try
	{
		//为物料跟踪新增一个类
		blkNum = inBlock.Tables.IndexOf("MM0099");
		Log::Trace("", __FUNCTION__, "=== MM0099 = [{0}] ", blkNum);
		if (blkNum < 0)
		{
			inBlock.Tables.Add("MM0099");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_ID");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_LINE_TYPE");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "SYSTEM_ID");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "FUNC_ID");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "IN_MAT_NO");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "PLAN_NO");
			inBlock.Tables["MM0099"].Columns.Add(DT_STRING, "REPAIR_FLAG");
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

		//获取输入参数；
		rows = bcls_rec->Tables[0].Rows.get_Count();

		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm05["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
			tpssm05["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
			tpssm05["IN_MAT_NO"] = bcls_rec->Tables[0].Rows[i]["IN_MAT_NO"];

			Log::Trace("", __FUNCTION__, "tpssm05.FACTORY_DIV[{0}],[{1}],[{2}]", tpssm05["FACTORY_DIV"].ToString(), tpssm05["PLAN_BACKLOG_CODE"].ToString(), tpssm05["IN_MAT_NO"].ToString());

			if (tpssm05.Query("FACTORY_DIV, PLAN_BACKLOG_CODE, IN_MAT_NO") == false)
			{
				CFormattable arguments[] = { tpssm05["IN_MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在计划中不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tpssm05["PLAN_STATUS"].ToString() >= "08")
			{
				CFormattable arguments[] = { tpssm05["IN_MAT_NO"].ToString() };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]命令状态已经下发。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm05.TrimOrBlank();

			// 更新材料主档表当前材料的材料状态、计划号；
			//调用物料跟踪函数,修改厚板主档表，材料状态＝23，
			Log::Trace("", __FUNCTION__, "更新材料主档表当前材料的材料状态、计划号");
			inBlock.Tables["MM0099"].Rows.Add();

			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_ID"] = "PS02";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["EVENT_LINE_TYPE"] = "00";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["SYSTEM_ID"] = "PSSM";
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["FUNC_ID"] = s.svc_name;
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["MAT_NO"] = tpssm05["IN_MAT_NO"]; //材料号
			inBlock.Tables["MM0099"].Rows[v_rownum_mm99]["PLAN_NO"] = tpssm05["PLAN_NO"];

			v_rownum_mm99++;

			tpmof03["ORDER_NO"] = tpssm05["ORDER_NO"];
			tpmof03["WHOLE_BACKLOG_NO"] = tpssm05["WHOLE_BACKLOG_NO"];
			tpmof03["NUM"] = 1;
			tpmof03["WT"] = tpssm05["MAT_ACT_WT"];
			tpmof03["WHOLE_BACKLOG_CODE"] = tpssm05["WHOLE_BACKLOG_CODE"];
			tpmof03["WHOLE_BACKLOG_SEQ"] = tpssm05["WHOLE_BACKLOG_SEQ"];
			tpmof03["MAT_STATUS"] = "24";
			tpmof03["WHOLE_BACKLOG"] = tpssm05["WHOLE_BACKLOG"];

			tpmof03["MAT_NO"] = tpssm05["IN_MAT_NO"];
			tpmof03["CUST_MAT_NO"] = tpssm05["IN_MAT_NO"];

			tpmof03["CALL_FLAG"] = 1;  //只抛合同跟踪
			tpmof03["EVENT_ID"] = "51";
			tpmof03["SYSTEM_ID"] = "PS";
			tpmof03["FUNC_ID"] = "pssm55f5_del";

			tpmof03.TrimOrBlank();
			tpmof03.MergeTo(inPMOF99.Tables[0], false);

			v_rownum_pm99++;

			tpssm05.Delete("FACTORY_DIV, PLAN_BACKLOG_CODE, IN_MAT_NO");
		}

		//更新命令顺序号
		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm05["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
			tpssm05["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["PLAN_BACKLOG_CODE"];
			tpssm05["PLAN_NO"] = bcls_rec->Tables[0].Rows[i]["PLAN_NO"];

			//获取当前工序已释放的计划的最大计划执行顺序号,
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库			
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " SELECT PLAN_NO, MAT_NO FROM TPSSM05 "
					" WHERE FACTORY_DIV = @tpssm05.FACTORY_DIV "
					"   AND PLAN_BACKLOG_CODE = @tpssm05.PLAN_BACKLOG_CODE "
					"   AND PLAN_NO = @tpssm05.PLAN_NO "
					"   AND PLAN_STATUS < '08' "
					"   ORDER BY MAT_SEQ_NO ASC ";
				break;
			}
			cmd_tpssm05_inq.SetCommandText(sqlstr);
			cmd_tpssm05_inq.Parameters.Set("tpssm05.FACTORY_DIV", tpssm05["FACTORY_DIV"].ToString());
			cmd_tpssm05_inq.Parameters.Set("tpssm05.PLAN_BACKLOG_CODE", tpssm05["PLAN_BACKLOG_CODE"].ToString());
			cmd_tpssm05_inq.Parameters.Set("tpssm05.PLAN_NO", tpssm05["PLAN_NO"].ToString());
			cmd_tpssm05_inq.ExecuteReader();

			if (cmd_tpssm05_inq.Read())
			{
				tpssm05["PLAN_NO"] = cmd_tpssm05_inq.GetString(1);
				tpssm05["IN_MAT_NO"] = cmd_tpssm05_inq.GetString(2);

				v_mat_seq_no = v_mat_seq_no + 1;
				tpssm05["MAT_SEQ_NO"] = v_mat_seq_no;

				tpssm05.Update("MAT_SEQ_NO", "FACTORY_DIV, PLAN_BACKLOG_CODE, IN_MAT_NO, PLAN_NO");
			}
			cmd_tpssm05_inq.Close();

			v_plan_no_pre = tpssm05["PLAN_NO"];
		}

		Log::Trace("", __FUNCTION__, "v_rownum_mm99[{0}]", v_rownum_mm99);
		if (v_rownum_mm99 > 0)
		{
			doFlag = f_mmsm99(&inBlock, &outBlock, conn);

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
			//doFlag = f_pmof99_v3(&inPMOF99, &outPMOF99, conn);
			if (doFlag != 0)
			{
				//合同跟踪失败
				Log::Trace("", __FUNCTION__, "=== = 合同跟踪出错");
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
