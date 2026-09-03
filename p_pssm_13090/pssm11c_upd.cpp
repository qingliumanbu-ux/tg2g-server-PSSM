/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      魏晨祥
Version:     1.0
Date:        2023-06-16
Description: 铸余计划修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件，请包含在""中


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
///铸余计划修改
/// <returns>  </returns>
===========================================================</remark>*/
int f_pssm11c_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);

// service入口
BM2F_ENTERACE(pssm11c_upd)


int f_pssm11c_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	//程序用变量
	int doFlag = 0;
	int i = 0;
	int fetchRowCount = 0;

	CString sqlstr = "";

	/* 实体类定义 */
	CModel tpssm11c("TPSSM11C");

	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);

	try
	{
		/* 对输入信息循环处理 */
		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			/* 取得单行传入信息 */
			tpssm11c.Reset();
			tpssm11c.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			Log::Trace("", "", "tpssm11c_del IN:---heat_no[{0}]", (const char*)tpssm11c["SM_PLAN_NO"].ToString());

			//校验传入参数
			if (tpssm11c["SM_PLAN_NO"].ToString().TrimOrBlank() == " ")
			{
				strcpy(s.msg, "修改的计划号不可为空！");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (tpssm11c["PONO_STATUS"].ToString() == "99")
			{
				strcpy(s.msg, "铸余计划已经结束，不可修改！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm11c.Update("TPD_NO,IRON_LADLE_NO,PONO_STATUS","SM_PLAN_NO");
		}

		//sprintf(s.msg, _RES("QM00S0004339")/*删除完毕。*/);
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
