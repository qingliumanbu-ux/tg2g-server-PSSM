/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-18
Description: 连铸浇铸计划开浇时刻设置查询
**************************************************/
#include "stdafx.h"

#include "tpssm10.h"
#include "tpssm11.h"


/*<remark>=========================================================
/// <summary>
/// 连铸浇铸计划开浇时刻设置查询
/// <para>1.根据传入的PONO号，查询当前炉次信息。</para>
/// <para>数据库表：TPSSM10(炼钢设备配置表)                    </para>
/// <para>主调用函数：前台PSSM10P画面开浇时刻设置时，对话框初始化调用。    </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <param name="pono">制造命令     </param>
/// <returns>炉次信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11cctime_inq)


int f_pssm11cctime_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blknum;

	/* 业务变量 */
	CString v_pono = "";

	CString sqlstr = "";

	CDbCommand cmd_inq(conn);

	try
	{
		// 定义表的实体对象
		CTPSSM10 tpssm10(conn);
		CTPSSM11 tpssm11(conn);

		//--------------------------------
		//设定返回查询记录信息结构
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("TPSSM10_DLG");  //与Client端dsPSSM10P.TPSSM10_DLG表名一致
		bcls_ret->Tables[blknum].Columns.Add(tpssm10);  //连铸机号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SM_PLAN_NO");
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "HEAT_NO");
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CAST_SHOW");  //CAST号，显示用
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "LOT_SHOW");   //浇次号，显示用
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SLAB_SPEC");  //铸坯规格(厚宽长)



		//---------------------------------------------------
		//获得输入参数，单记录
		v_pono = bcls_rec->Tables[0].Rows[0]["PONO"];

		Log::Info("", __FUNCTION__, "pono=[{1}]", v_pono);

		//---------------------------------------------------
		//查询连铸浇铸信息
		tpssm10.PONO = v_pono;
		sqlstr = "tpssm10.Query()";
		bool has10 = tpssm10.Query("PONO");
		if (has10 == false)
		{
			CFormattable arguments[] = { tpssm10.PONO }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "没有制造命令号[{0}]的炉次信息，请查询后操作。", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		tpssm11.PONO = v_pono;
		sqlstr = "tpssm11.Query()";
		bool has11 = tpssm11.Query("PONO");
		if (has11 == false)
		{
			//没读到数据，说明没有编入到出钢计划
		}

		//--------------------
		//返回数据
		CDataRow & row = bcls_ret->Tables["TPSSM10_DLG"].Rows.Add();
		row.Merge(tpssm10);
		row["HEAT_NO"] = tpssm11.HEAT_NO.Trim();

		if (tpssm11.CAST_NO.Trim() == "")
		{
			row["CAST_SHOW"] = "";
		}
		else
		{
			row["CAST_SHOW"] = tpssm11.CAST_NO.Trim() + "-" + tpssm11.CAST_DIV_NO.ToString();
		}
		row["LOT_SHOW"] = tpssm10.CAST_LOT_NO.Trim() + "-" + tpssm10.CAST_LOT_DIV_NO.ToString();
		row["SLAB_SPEC"] = tpssm10.SLAB_THICK.ToString() + "*" + tpssm10.SLAB_WIDTH.ToString() + "*" + tpssm10.SLAB_LEN.ToString();


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