/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    SUNHAO
Version:   3.1.0
Date:      2023-5-17
Description: 出钢计划连铸顺序号重排更新
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"




int f_pssm_castlot_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;


	/* 实体类定义 */
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");

	CModel tpssm10("TPSSM10");
	CModel tpssm01("TPSSM01");


	CString sqlstr;

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_seq_inq(conn);
	CDbCommand cmd_upd(conn);

	CString station_no = "";
	CString pono = "", factory_div = "";
	CString CAST_LOT_NO2 = "";
	int cc_seq_cur = 0;
	try
	{
		CAST_LOT_NO2 = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO2"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "CAST_LOT_NO2=[{0}]", CAST_LOT_NO2);

		/*tpssm10.Reset();
		sqlstr = " SELECT CAST_LOT_NO2,CAST_LOT_DIV_NO2,PONO FROM tpssm10 WHERE CAST_LOT_NO2 IN ( " + CAST_LOT_NO2 + " ) ORDER BY CAST_LOT_NO2,CAST_LOT_DIV_NO2 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssm10["PONO"] = cmd_inq.GetString(3);
			if (tpssm10["CAST_LOT_NO2"].ToString() != cmd_inq.GetString(1))
			{
				tpssm10["CAST_LOT_NO2"] = cmd_inq.GetString(1);
				i = 0;
			}
			tpssm10["CAST_LOT_DIV_NO2"].ToDecimal() = ++i;
			if (tpssm10["CAST_LOT_DIV_NO2"].ToDecimal() != cmd_inq.GetDecimal(2))
			{
				tpssm10.Update("CAST_LOT_DIV_NO2", "PONO");
			}
			if (tpssm10["CAST_LOT_DIV_NO2"].ToDecimal() == 1)
			{
				tpssm10["RESTRAND_FLG"] = "T";
				tpssm10.Update("RESTRAND_FLG", "PONO");
			}

		}
		cmd_inq.Close();*/

		tpssm10.Reset();
		sqlstr = " SELECT max(CAST_LOT_DIV_NO2),CAST_LOT_NO2 FROM tpssm10 WHERE CAST_LOT_NO2 IN ( " + CAST_LOT_NO2 + " ) GROUP BY CAST_LOT_NO2 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssm10["CAST_LOT_NO2"] = cmd_inq.GetString(2);
			tpssm10["CAST_LOT_SUM2"] = cmd_inq.GetDecimal(1);
			tpssm10.Update("CAST_LOT_SUM2", "CAST_LOT_NO2");

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
