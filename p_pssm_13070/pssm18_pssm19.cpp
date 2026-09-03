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
//int f_pssm_castlot_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

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
BM2F_ENTERACE(pssm18_pssm19)

int f_pssm18_pssm19(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	CString datetime = "";
	CModel tpssm18("TPSSM18");//设备状态
	EIClass inblock;
	EIClass outblock;
	EIClass in_pssm18;
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);

	try
	{
		

		v_factory_div = "LG1";
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//-----------------------------------------------
		//保存设备特殊状态
		//1)先删除原有记录
		tpssm18["FACTORY_DIV"] = v_factory_div;
		sqlstr = "tpssm18.Delete()";
		tpssm18.Delete("FACTORY_DIV");
		//2)读取输入参数并新增记录
		rows = bcls_rec->Tables["TPSSM18"].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			tpssm18["DEV_CODE"] = bcls_rec->Tables["TPSSM18"].Rows[i]["DEV_CODE"];
			tpssm18["START_TIME"] = bcls_rec->Tables["TPSSM18"].Rows[i]["START_TIME"];
			tpssm18["END_TIME"] = bcls_rec->Tables["TPSSM18"].Rows[i]["END_TIME"];
			tpssm18["DEV_STATUS_REMARK"] = bcls_rec->Tables["TPSSM18"].Rows[i]["DEV_STATUS_REMARK"];
			tpssm18["STOP_FLAG"] = bcls_rec->Tables["TPSSM18"].Rows[i]["STOP_FLAG"];
			tpssm18["REC_CREATE_TIME"] = datetime;
			tpssm18["AREA_ID"] = bcls_rec->Tables["TPSSM18"].Rows[i]["STATUS_AREA"];
			tpssm18["MT_SEQ_NO"] = datetime + CString::Format("%0.4d", i + 1);
			tpssm18["DEV_STATUS"] = "1";

			////Log::Trace("", __FUNCTION__, "dev_code=[{0}]", tpssm18["DEV_CODE"].ToString());
			////Log::Trace("", __FUNCTION__, "start_time=[{0}]", tpssm18["START_TIME"].ToString());
			////Log::Trace("", __FUNCTION__, "end_time=[{0}]", tpssm18["END_TIME"].ToString());
			////Log::Trace("", __FUNCTION__, "dev_status_remark=[{0}]", tpssm18["DEV_STATUS_REMARK"].ToString());
			////Log::Trace("", __FUNCTION__, "stop_flag=[{0}]", tpssm18["STOP_FLAG"].ToString());
			////Log::Trace("", __FUNCTION__, "area_id=[{0}]", tpssm18["AREA_ID"].ToDecimal());

			tpssm18.TrimOrBlank();
			sqlstr = "tpssm18.Insert()";
			tpssm18.Insert();
			//tpssm18.MergeTo(in_pssm18.Tables[0], false);

			/*////Log::Trace("", __FUNCTION__, "开始调用f_pssm_paimp2_snd=[{0}]", tpssm18["AREA_ID"].ToDecimal());
			int ret = 0;
			ret = f_pssm_paimp2_snd(&in_pssm18, bcls_ret, conn);
			if (ret < 0)
			{
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
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
