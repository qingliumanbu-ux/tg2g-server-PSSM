/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    SUNHAO
Version:   3.1.0
Date:      2023-5-17
Description: 出钢计划连铸顺序号重排更新
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"




int f_pssm_seq_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	int cc_seq_cur = 0;
	try
	{

		//①获取当前分区铸机清单
		sqlstr = "SELECT STATION_NO FROM TPSSMD1 WHERE STATION_ID = 'C' ORDER BY STATION_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			station_no = cmd_inq.GetString(1);//获取当前铸机号
			//Log::Info("", __FUNCTION__, "station_no=[{0}]", station_no);
			//②根据当前铸机号，获取当前铸机连铸顺序信息
			sqlstr = " SELECT T1.FACTORY_DIV,T1.PONO,T1.CC_SEQ "
				"	FROM TPSSM10 T1 LEFT JOIN TPSSM11 T2 ON T1.PONO = T2.PONO "
				"	WHERE 1 = 1 "
				"   AND T1.CC_MACH_NO = '" + station_no + "'"
				"	AND T1.PONO_STATUS < 83 "
				"	AND T1.PONO_STATUS > 14 "
				"	AND CC_SEQ != 999 "
				"	AND CC_SEQ != 0 "
				"	ORDER BY T2.CAST_NO ASC, T2.CAST_DIV_NO ASC, T1.CC_SEQ ASC, "
				"	T1.CAST_LOT_NO, T1.PLAN_DATE, T1.CAST_LOT_DIV_NO ASC ";
			cmd_seq_inq.SetCommandText(sqlstr);
			cmd_seq_inq.ExecuteReader();
			int seq_no = 1;
			while (cmd_seq_inq.Read())
			{
				factory_div = cmd_seq_inq.GetString(1);
				pono = cmd_seq_inq.GetString(2);
				cc_seq_cur = cmd_seq_inq.GetInt16(3);
				//Log::Info("", __FUNCTION__, "seq_no=[{0}], factory_div=[{1}],pono=[{2}]cc_seq_cur[{3}]", seq_no, factory_div, pono, cc_seq_cur);
				//③若当前顺序号与重排后不一致，则更新当前PONO顺序号
				if (cc_seq_cur != seq_no)
				{
					//Log::Info("", __FUNCTION__, "执行更新");
					tpssm10["PONO"] = pono;
					tpssm10["FACTORY_DIV"] = factory_div;
					tpssm10["CC_SEQ"] = seq_no;
					tpssm10.Update("CC_SEQ", "PONO,FACTORY_DIV");

					tpssm01["PONO"] = pono;
					tpssm01["CC_SEQ"] = seq_no;
					tpssm01.Update("CC_SEQ", "PONO");
				}
				seq_no++;//连铸顺序号累加
			}
			cmd_seq_inq.Close();

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
