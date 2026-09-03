/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-27
Description:	 发送计划状态信息至MMS。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

#include "epex.h"

int f_plan_delete_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_plan_insert_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_plan_update_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 发送计划状态信息至MMS
/// <para>1.读取传入的厂别区分、制造命令号、制造命令状态</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：由计划编制，计划删除调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="pono_status">制造命令状态          </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_t8e2s1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;

	CString lpsz_tc_no = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	EPEX epex(&s, conn);

	CString sqlstr;
	EIClass inblk2;

	inblk2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	inblk2.Tables[0].Columns.Add(DT_STRING, "OPER_FLAG");
	inblk2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inblk2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO_IN");
	inblk2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO_AA");
	inblk2.Tables[0].Rows.Add();

	/*实体类定义*/
	//CModel tpssm01("TPSSM01");
	//CModel tpssm02("TPSSM02");
	//CModel tpssm11("TPSSM11");

	//CDbCommand cmd_tpssm01_inq(conn);

	try
	{
		inblk2.Tables[0].Rows[0]["FACTORY_DIV"] = bcls_rec->Tables["PLAN"].Rows[0]["FACTORY_DIV"].ToString();
		inblk2.Tables[0].Rows[0]["OPER_FLAG"] = bcls_rec->Tables["PLAN"].Rows[0]["OPER_FLAG"].ToString();
		inblk2.Tables[0].Rows[0]["SM_PLAN_NO"] = bcls_rec->Tables["PLAN"].Rows[0]["SM_PLAN_NO"].ToString();
		inblk2.Tables[0].Rows[0]["SM_PLAN_NO_IN"] = bcls_rec->Tables["PLAN"].Rows[0]["SM_PLAN_NO_IN"].ToString();
		inblk2.Tables[0].Rows[0]["SM_PLAN_NO_AA"] = bcls_rec->Tables["PLAN"].Rows[0]["SM_PLAN_NO_AA"].ToString();
		Log::Trace("", __FUNCTION__, "计划范围sm_plan_no={0}", bcls_rec->Tables["PLAN"].Rows[0]["SM_PLAN_NO_IN"].ToString());
		Log::Trace("", __FUNCTION__, "计划范围sm_plan_no={0}", bcls_rec->Tables["PLAN"].Rows[0]["SM_PLAN_NO_AA"].ToString());
		ret = f_plan_delete_snd(&inblk2, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		ret = f_plan_update_snd(&inblk2, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		ret = f_plan_insert_snd(&inblk2, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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
