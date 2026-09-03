/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    SUNHAO
Version:   3.1.0
Date:      2023-5-17
Description: 出钢计划连铸顺序号重排更新
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"




int f_pssm_castlot_upd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;


	/* 实体类定义 */
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm_cast("TPSSM_CAST");

	CModel tpssm10("TPSSM10");


	CString sqlstr;

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_seq_inq(conn);
	CDbCommand cmd_upd(conn);
	int i_count = 0;
	CString dateNow14 = "";
	CString station_no = "";
	CString pono = "", factory_div = "";
	CString CAST_LOT_NO2 = "";
	CString cast_lot_string = "";
	CDecimal count = 0;
	CDecimal count2 = 0;
	CDecimal cast_head = 80;
	try
	{
		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");
		sqlstr = "   SELECT A.CAST_NO,B.CAST_LOT_NO2,B.CAST_LOT_NO  FROM TPSSM11 A,TPSSM10 B WHERE A.PONO = B.PONO AND A.CAST_DIV_NO = 1 AND A.PONO_STATUS <= 83 ORDER BY A.CAST_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssm_cast.Reset();
			tpssm_cast["CAST_NO"] = cmd_inq.GetString(1);
			tpssm_cast["CAST_LOT_NO2"] = cmd_inq.GetString(3);

			


			while (cast_lot_string.Find(tpssm_cast["CAST_LOT_NO2"].ToString().Trim()) >= 0)
			{
				for (i_count = 0; i_count <= 9; i_count++)
				{
					tpssm_cast["CAST_LOT_NO2"] = (cast_head + i_count).ToString() + tpssm_cast["CAST_LOT_NO2"].ToString().Substring(2);
					tpssm10["CAST_LOT_NO2"] = tpssm_cast["CAST_LOT_NO2"];
					if (tpssm10.QueryCount("CAST_LOT_NO2") == 0 && cast_lot_string.Find(tpssm_cast["CAST_LOT_NO2"].ToString().Trim()) < 0)
					{
						break;
					}
				}
				Log::Trace("", __FUNCTION__, "  [{0}]", i_count);
				if (i_count >= 9)
				{
					break;
				}
			}

			count = tpssm_cast.QueryCount("CAST_NO");
			if (count == 0)
			{
				tpssm_cast["REC_CREATE_TIME"] = dateNow14;
				tpssm_cast["REC_CREATOR"] = s.userid;
				tpssm_cast.Insert();
			}
			else
			{
				tpssm_cast["REC_REVISE_TIME"] = dateNow14;
				tpssm_cast["REC_REVISOR"] = s.userid;
				tpssm_cast.Update("CAST_LOT_NO2,REC_REVISE_TIME,REC_REVISOR", "CAST_NO");
			}
			if (cast_lot_string.Trim() == "")
			{
				cast_lot_string = tpssm_cast["CAST_LOT_NO2"].ToString();
			}
			else
			{
				cast_lot_string = cast_lot_string + "," + tpssm_cast["CAST_LOT_NO2"].ToString();
			}
			//Log::Trace("", __FUNCTION__, "  [{0}]->[{1}]", tpssm_cast["CAST_LOT_NO2"].ToString(), cast_lot_string);
		}
		cmd_inq.Close();
		// Log::Trace("", __FUNCTION__, "  [{0}]->[{1}]", tpssm_cast["CAST_LOT_NO2"].ToString(), cast_lot_string);
		sqlstr = " UPDATE TPSSM10 A SET CAST_LOT_DIV_NO2 = ( "
			"	SELECT CAST_DIV_NO FROM TPSSM11 "
			"	WHERE PONO = A.PONO AND PONO_STATUS < 83 ) "
			" WHERE EXISTS ( "
			"	SELECT 1 FROM TPSSM11 WHERE PONO = A.PONO AND PONO_STATUS < 83 ) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " UPDATE TPSSM10 A SET CAST_LOT_NO2 = ( "
			" SELECT B.CAST_LOT_NO2 FROM TPSSM_CAST B, TPSSM11 C"
			"	WHERE A.PONO = C.PONO AND B.CAST_NO  = C.CAST_NO AND C.PONO_STATUS < 83 ) "
			" WHERE EXISTS ( "
			"	SELECT 1 FROM TPSSM_CAST B, TPSSM11 C WHERE A.PONO = C.PONO AND B.CAST_NO  = C.CAST_NO AND C.PONO_STATUS < 83 ) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
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
