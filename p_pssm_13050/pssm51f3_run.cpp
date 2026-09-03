/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-16
Version:1.0
Description: 出钢计划运转处理
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件





//int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
int f_pssm_run_proc_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm51f3_run(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn);
//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 出钢计划运转处理
/// <para>根据选择的PONO, 将传入的设备运转信号进行处理，更新计划状态。</para>
/// <para>数据库表：TPSSM13/14(炼钢出钢计划跟踪表)                  </para>
/// <para>主调用函数：前台PSSM51画面F3(发送)调用。                  </para>
/// </summary>
/// <param name="pono">制造命令          </param>
/// <param name="proc_no">设备处理号     </param>
/// <param name="proc_time">处理时间     </param>
/// <param name="signal">信号代码        </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm51f3_run)
//-EP_SYSTEM_HEAD_END
int f_pssm51f3_run(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret = 0;
	CString   pono="";                          /* 制造命令号 */
	CString   proc_time="";                    /* 处理时刻 */
	CString   proc_no="";                      /* 处理号 */
	CString   simul_flag = "1";              /* 模拟标记: 1- 模拟*/

	CModel tpssms1("TPSSMS1");
	CString sqlstr;
	CModel tpssm99("TPSSM99");//履历
	CModel tpssm11("TPSSM11");//计划主表

	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);
	try
	{
		//取得传入信息
		proc_time = bcls_rec->Tables[0].Rows[0]["PROC_TIME"];
		proc_no = bcls_rec->Tables[0].Rows[0]["PROC_NO"];
		//simul_flag = bcls_rec->Tables[0].Rows[0]["simul_flag"]; //为履历记录用
		pono = bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
		tpssms1.MergeFrom(bcls_rec->Tables[2].Rows[0]);
		if ( !bcls_rec->Tables[0].Columns.Contains("SIMUL_FLAG"))
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "SIMUL_FLAG");
		if (!bcls_rec->Tables[2].Columns.Contains("CHARGE_NO_2"))
			bcls_rec->Tables[2].Columns.Add(DT_STRING, "CHARGE_NO_2");
		bcls_rec->Tables[0].Rows[0]["SIMUL_FLAG"] = 	simul_flag; //为履历记录用
		proc_no=proc_no.TrimOrBlank();
		pono=pono.TrimOrBlank();

		////dclian---add---2015-11-16------
		//////Log::Trace("", __FUNCTION__, "信号模拟，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
		//tpssm11["PONO"] = pono;
		//tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		//tpssm11.Query("FACTORY_DIV,PONO");
		//tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		//tpssm99["PONO"] = tpssm11["PONO"];
		//tpssm99["EVENT_ID"] = "16";
		//tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		//tpssm99["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
		//tpssm99["SIMUL_FLAG"] = simul_flag;
		//tpssm99["PROC_NO"] = proc_no;
		//tpssm99["EVENT_DATETIME"] = proc_time;//模拟时间
		//
		//////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

		
		//判断状态是否正确，即状态不可逆
		ret = f_pssm_run_proc_n(bcls_rec, bcls_ret,conn);
		if (ret < 0)
		{
			//tpssm99["VALID_FLAG"] = "0";
			//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
			//////Log::Trace("", __FUNCTION__, "记录失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
			//
			//tpabort(0);
			//tpbegin(0, 0);
			////记录编入计划成功的履历
			//ret = 0;
			//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			//tpcommit(0);
			//tpbegin(0, 0);

			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//tpssm99["VALID_FLAG"] = "1";
		//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		//////Log::Trace("", __FUNCTION__, "记录成功履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		////记录编入计划成功的履历
		//ret = 0;
		//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}
