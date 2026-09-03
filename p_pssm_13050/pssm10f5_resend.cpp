/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2025-01-14
Version:1.0
Description: 新增制造命令
**************************************************/
// C 的标准头文件部分  

#include "stdafx.h"

#include "epex.h"



//#include "tpssmd9.h"
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);
int f_pssm10f5_resend(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表
/*<remark>=========================================================
/// <summary>
///  删除后备制造命令
/// <para> 删除后备制造命令；</para>
/// <para>数据库表：TPSSM01/02/03/10               </para>
/// <para>主调用函数：TPSSM09画面F4删除调用。        </para>
/// </summary>
/// <param name="main_backlog_code">炼钢主工序代码    </param>
/// <returns>制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm10f5_resend)

int f_pssm10f5_resend(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CDecimal dummy = 0, cc_seq = 0;
	int i = 0, ret = 0;
	CString lpsz_tc_no = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	EPEX epex(&s, conn);
	
	CModel tpssm10("TPSSM10");
	CModel tpssm40("TPSSM40");
	CModel tpssm99("TPSSM99");
	EIClass outBlock;
	EIClass in_pssm99trace;  //调用履历函数

	
	in_pssm99trace.Tables[0].set_TableName("TRACE");
	in_pssm99trace.Tables[0].Clone(tpssm99);

	try
	{
		/* 对输入信息循环处理 */
		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm10.Reset();
			tpssm10.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			
			if (tpssm10["PONO_STATUS"].ToDecimal() == 91) //编入出钢计划或生产，不能删除
			{
				//-----------------------------------------------
				//发送炼钢PONO状态
				/*inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				inBlock.Tables[0].Rows[0]["PONO"] = tpssm10["PONO"];
				inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 91;*/

				/* *****	发送电文开始 ***** */
				lpsz_tc_no = "210011";/*赋电文号*/

				/*初始化*/
				if (epex.Initialize(lpsz_tc_no) < 0)
				{
					strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
					sprintf(s.sysmsg, "电文210011初始化出错！");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				/*拼电文数据*/
				if (epex.SetValue("tpssm01", "pre_heat_no", 0, tpssm10["PONO"].ToString()) < 0)
				{
					sprintf(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					sprintf(s.sysmsg, "电文210011拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				/*发送电文*/
				if (epex.SendTele() < 0)
				{
					sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//释放电文
				epex.Uninitialize();
				//ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);

				//if (ret != 0) //调用不成功
				//{
				//	//////Log::Trace("", __FUNCTION__,  "f_ps200009_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
				//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				//写计划履历表
				tpssm99["EVENT_ID"] = "Z3";
				tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssm99["PONO"] = tpssm10["PONO"];
				tpssm99["VALID_FLAG"] = "1";//操作成功
				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

				//记录编入计划成功的履历
				ret = 0;
				ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
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
