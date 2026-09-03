/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2023-06-8 10:13:56
Description: 钢水返送信息新增
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/
#include "tpssm35.h"

/* ***** 静态函数申明 ***** */
int f_mmsm_gyhl(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
// service入口
BM2F_ENTERACE(pssmxh35_del)
int f_pssmxh35_del(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	int fetchRowCount;
	int i;
	CDecimal affectRow = 0;
	CString	datetimeNow; /* 记录创建时间 */

	CString v_factory_div = "LG1";

	CString sqlstr = "";
	CString str = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	EIClass bcls_rec_xh;
	bcls_rec_xh.Tables[0].set_TableName("XH");
	bcls_rec_xh.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	bcls_rec_xh.Tables[0].Rows.Add();

	//系统的分页类信息。
	//CPageInfo pageInfo;

	CModel tpssm35("TPSSM35");

	CDbCommand cmd(conn);

	EIClass inblkmmsm;

	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "PROC_FLAG");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "HEAT_NO");
	inblkmmsm.Tables[0].Columns.Add(DT_STRING, "MB_HEAT_NO");
	inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "WTS");
	inblkmmsm.Tables[0].Columns.Add(DT_DECIMAL, "FLAG");
	inblkmmsm.Tables[0].Rows.Add();  //只生成一行

	try
	{
		datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm35.Reset();
			tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			Log::Trace("", "", "FACTORY_DIV[{0}]", v_factory_div);
			Log::Trace("", "", "PONO[{0}]", (const char*)tpssm35["PONO"].ToString());
			Log::Trace("", "", "RET_PONO[{0}]", (const char*)tpssm35["RET_PONO"].ToString());
			Log::Trace("", "", "HEAT_NO[{0}]", (const char*)tpssm35["HEAT_NO"].ToString());
			Log::Trace("", "", "RET_HEAT_NO[{0}]", (const char*)tpssm35["RET_HEAT_NO"].ToString());

			/*inblkmmsm.Tables[0].Rows[0]["PROC_FLAG"] = "D";
			inblkmmsm.Tables[0].Rows[0]["DEV_CODE"] = tpssm35["DEV_CODE"];
			inblkmmsm.Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"];
			inblkmmsm.Tables[0].Rows[0]["MB_HEAT_NO"] = tpssm35["RET_HEAT_NO"].ToString().TrimOrBlank();
			inblkmmsm.Tables[0].Rows[0]["WTS"] = tpssm35["RETURN_MLSL"];
			inblkmmsm.Tables[0].Rows[0]["FLAG"] = "1";
			ret = f_mmsmgglwt_proc(&inblkmmsm, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}*/

			tpssm35.Delete("FACTORY_DIV,HEAT_NO,RET_HEAT_NO");//根据主键删除
			bcls_rec_xh.Tables[0].Rows[0]["HEAT_NO"] = tpssm35["HEAT_NO"].ToString();
			ret = f_mmsm_gyhl(&bcls_rec_xh, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}



	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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