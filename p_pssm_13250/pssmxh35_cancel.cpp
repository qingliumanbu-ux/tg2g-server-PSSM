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
// service入口
BM2F_ENTERACE(pssmxh35_cancel)
int f_pssmxh35_cancel(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
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
	CModel tpssm11("TPSSM11");
	CModel tpssm11_ret("TPSSM11_RET");
	CModel tpssm41("TPSSM41");
	CModel tpssm13("TPSSM13");
	CModel tpssm12("TPSSM12");
	CModel tpssm10("TPSSM10");
	CModel tpssm01("TPSSM01");
	CDbCommand cmd(conn);


	try
	{
		datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm35.Reset();
			tpssm35.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			if (tpssm35["HEAT_NO"].ToString().Trim() == "")
			{
				strcpy(s.msg, "炉号为空无法回炉取消！");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (tpssm35["RET_HEAT_NO"].ToString().Trim() != "")
			{
				strcpy(s.msg, "回退炉号需要为空方能回炉取消！");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm35.Query("HEAT_NO");
			tpssm11["FACTORY_DIV"] = v_factory_div;
			tpssm11["HEAT_NO"] = tpssm35["HEAT_NO"];
			if (tpssm11.Query("HEAT_NO,FACTORY_DIV"))
			{
				tpssm10["PONO"] = tpssm11["PONO"];
				tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm10["PONO_STATUS"] = 15;
				tpssm01["PONO"] = tpssm11["PONO"];
				tpssm01["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm01["PONO_STATUS"] = 15;
				if (tpssm01.QueryCount("PONO,FACTORY_DIV")==1)
				{
					tpssm01.Update("PONO_STATUS", "PONO,FACTORY_DIV");
				}
				if (tpssm10.QueryCount("PONO,FACTORY_DIV") == 1)
				{
					tpssm10.Update("PONO_STATUS", "PONO,FACTORY_DIV");
				}
				tpssm11_ret["SM_PLAN_NO"] =tpssm11["SM_PLAN_NO"];
				tpssm11_ret["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm11_ret.Query("SM_PLAN_NO,FACTORY_DIV");
				tpssm11.Delete("SM_PLAN_NO,FACTORY_DIV");
				tpssm11.CopyFrom(tpssm11_ret);
				tpssm11.Insert();
				tpssm01.Reset();
				tpssm10.Reset();
				tpssm10["PONO"] = tpssm11_ret["PONO"];
				tpssm10["FACTORY_DIV"] = tpssm11_ret["FACTORY_DIV"];
				tpssm10["PONO_STATUS"] = 15;
				tpssm01["PONO"] = tpssm11_ret["PONO"];
				tpssm01["FACTORY_DIV"] = tpssm11_ret["FACTORY_DIV"];
				tpssm01["PONO_STATUS"] = 15;
				if (tpssm01.QueryCount("PONO,FACTORY_DIV") == 1)
				{
					tpssm01.Update("PONO_STATUS", "PONO,FACTORY_DIV");
				}
				if (tpssm10.QueryCount("PONO,FACTORY_DIV") == 1)
				{
					tpssm10.Update("PONO_STATUS", "PONO,FACTORY_DIV");
				}
				tpssm11_ret.Delete("SM_PLAN_NO,FACTORY_DIV");
				tpssm35.Delete("HEAT_NO");

				tpssm12["FACTORY_DIV"] = tpssm35["FACTORY_DIV"];
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["CHARGE_NO"] = tpssm11["CURR_WP_NO"];
				sqlstr = "tpssm12.Query()";
				tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == tpssm12["END_TIME"].ToString().Trim())
				{
					tpssm12["END_TIME_REAL"] = " ";

					sqlstr = "tpssm12.Update()";
					tpssm12.Update("END_TIME_REAL", "FACTORY_DIV,SM_PLAN_NO,CHARGE_NO");
				}
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