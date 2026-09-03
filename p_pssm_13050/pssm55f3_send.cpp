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
#include "tpssm05.h"
#include "tmmsm01.h"

BM2F_ENTERACE(pssm55f3_send)

int f_pssm55f3_send(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal v_mat_seq_no = 0;

	CString v_plan_no_pre = " ";

	EIClass inBlock;
	EIClass outBlock;

	CTPSSM05 tpssm05(conn);
	CTMMSM01 tmmsm01(conn);
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm05_inq(conn);
	CDbCommand cmd_tqmtjf1_upd(conn);
	CDbCommand cmd_tep0002_inq(conn);

	CString sqlstr;

	CTracer log(__FUNCTION__);
	try
	{
		userid = s.userid;

		//2.  获取第一块输入参数；
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Info("", __FUNCTION__, "rows = [{0}]", rows);

		for (i = 0; i < rows; i++)
		{
			//取得入口信息
			tpssm05.FACTORY_DIV = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];
			tpssm05.PLAN_BACKLOG_CODE = bcls_rec->Tables[0].Rows[i]["PLAN_BACKLOG_CODE"];
			tpssm05.PLAN_NO = bcls_rec->Tables[0].Rows[i]["PLAN_NO"];
			tpssm05.IN_MAT_NO = bcls_rec->Tables[0].Rows[i]["IN_MAT_NO"];

			if (tpssm05.Query("FACTORY_DIV, PLAN_BACKLOG_CODE, PLAN_NO, IN_MAT_NO") == false)
			{
				CFormattable arguments[] = { tmmsm01.MAT_NO };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]在命令信息中不存在，不能执行当前操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (tpssm05.PLAN_STATUS >= "08")
			{
				CFormattable arguments[] = { tpssm05.IN_MAT_NO };// 定义参数列表的数组
				CMessageFormat::Format(s.msg, "材料[{0}]命令状态已经下发。", arguments, 1);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tpssm05.START_TIME = bcls_rec->Tables[0].Rows[0]["START_TIME"];
			tpssm05.STATION_NO = bcls_rec->Tables[0].Rows[0]["STATION_NO"];
			tpssm05.STATION_ID = "E";

			tpssm05.REC_REVISOR = s.userid;
			tpssm05.REC_REVISE_TIME = datetime;

			tpssm05.PLAN_STATUS = "08";

			Log::Info("", __FUNCTION__, "tpssm05.FACTORY_DIV = [{0}]", tpssm05.FACTORY_DIV);
			Log::Info("", __FUNCTION__, "tpssm05.PLAN_BACKLOG_CODE = [{0}]", tpssm05.PLAN_BACKLOG_CODE);
			Log::Info("", __FUNCTION__, "tpssm05.PLAN_NO = [{0}]", tpssm05.PLAN_NO);
			Log::Info("", __FUNCTION__, "tpssm05.IN_MAT_NO = [{0}]", tpssm05.IN_MAT_NO);
			Log::Info("", __FUNCTION__, "tpssm05.START_TIME = [{0}]", tpssm05.START_TIME);
			Log::Info("", __FUNCTION__, "tpssm05.STATION_NO = [{0}]", tpssm05.STATION_NO);

			tpssm05.Update("REC_REVISOR, REC_REVISE_TIME, PLAN_STATUS, START_TIME, STATION_ID, STATION_NO", "FACTORY_DIV, PLAN_BACKLOG_CODE, IN_MAT_NO, PLAN_NO");
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