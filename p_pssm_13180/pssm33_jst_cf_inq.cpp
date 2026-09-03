/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2014-3-5
Version:1.0
Description: 出钢计划甘特图查询
Update：
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "CUtils.h"
//程序用头文件
/*<remark>=========================================================
/// <summary>
/// 甘特图计划信息查询
/// <para>主要数据：主计划及工序计划，设备信息及状态，浇铸信息,传搁时间信息。</para>
/// <para>数据库表：tpssm11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：PSSM18画面查询(甘特图)调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>出钢计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm33_jst_cf_inq)

int f_pssm33_jst_cf_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int blkseq = 0;
	int ret = 0;
	int fetchRowCount = 0;

	CString v_factory_div = "";	//炼钢单元号
	CString base_time = "";
	CString cast_lot_no_and_div = "";
	CDecimal  pour_time1 = 0;               /* 连铸机浇注时间*/
	CDecimal  pour_time = 0;               /* 连铸机浇注时间*/
	CString dev_move_start = "";
	CString dev_move_end = "";
	CString dev_code = "";
	CString query_type = "";
	CString v_slab_dest = "";
	CDecimal prod_density = 0; //板坯密度
	int k = 0;
	int mode = 1;
	CString lslab_no = "";
	CString strand_no = "";
	CDecimal seq = 0;
	CString pono = "";
	CString show_flag = "";
	CString cs_dev_code = "";
	CModel tpssm33("TPSSM33");
	//CModel tep0002("TEP0002");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);
	CString sqlstr;
	CString sqlstr_cf;
	EIClass tb_tpssm11;
	int  i = 0;
	CDataTable tb_tpssm03("TPSSM03");
	CDataTable tb_tpssmd3("TPSSMD3_TEST");
	CDataTable tb_tep0002("TEP0002");

	try
	{
		//cs_dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"].ToString().Trim();
		sqlstr = "  SELECT * FROM  TPSSM33 T1  ";
		cmd_inq.SetCommandText(sqlstr);
		/*cmd_inq.Parameters.Set("station_id", cs_dev_code.Substring(0, 1));
		cmd_inq.Parameters.Set("station_no", cs_dev_code.Substring(1, 1));
		cmd_inq.Parameters.Set("dev_code", cs_dev_code);*/
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm33);
			sqlstr_cf = " SELECT ELM_NAME,ELM_ACT FROM TQMTS25  WHERE ST_SAMPLE_NO =(SELECT MAX(ST_SAMPLE_NO) FROM TQMTS24 WHERE HEAT_NO =@heat_no AND	WHOLE_BACKLOG_CODE=@whole_backlog_code )  ";
			Log::Trace("", __FUNCTION__, "RUN_SEQ=[{0}]", sqlstr_cf);
			cmd_inq2.SetCommandText(sqlstr_cf);
			cmd_inq2.Parameters.Set("whole_backlog_code", tpssm33["STATION_ID"].ToString());
			cmd_inq2.Parameters.Set("pono", tpssm33["PONO"].ToString());
			cmd_inq2.Parameters.Set("heat_no", tpssm33["HEAT_NO"].ToString());

			//分页获取
			cmd_inq2.SetCommandText(sqlstr_cf);
			bcls_ret->Tables.Add();
			cmd_inq2.ExecuteQuery(bcls_ret->Tables[i]);
			bcls_ret->Tables[i].set_TableName(tpssm33["STATION_ID"].ToString() + tpssm33["STATION_NO"].ToString());
			i = i + 1;
			cmd_inq2.Close();
		}
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
