/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-6-20
Description:	 生成铸余计划。
Update: 
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 生成铸余计划
/// <para>数据库表：TPSSM11C(炼钢铸余计划表) </para>
/// <para>主调用函数：被铸余计划结束 主动调用生成下一条。</para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="dev_code">设备代码     </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm11c_cra(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	
	CModel tpssm11c("TPSSM11C");
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{
		create_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		/* 对输入信息循环处理 */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{
			/* 取得单行传入信息 */
			//tpssm11c.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tpssm11c["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];
			tpssm11c["STATION_NO"] = bcls_rec->Tables[0].Rows[i]["STATION_NO"];

			if (tpssm11c["FACTORY_DIV"][0] == ' ' || tpssm11c["STATION_NO"][0] == ' ')
			{
				sprintf(s.sysmsg, "设备号[STATION_NO]或者厂别[FACTORY_DIV] 不可为空.");
				strcpy(s.msg, "设备号[STATION_NO]或者厂别[FACTORY_DIV] 不可为空.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm11c["REC_CREATOR"] = s.userid;
			tpssm11c["REC_CREATE_TIME"] = create_time;
			//tpssm11c["REC_REVISOR"] = s.userid;

			////Log::Trace("", __FUNCTION__,  "f_pssm34_ins_n>dev_code=[{0}]",(const char*)tpssm34["DEV_CODE"].ToString());
			////Log::Trace("", __FUNCTION__, "tpssm34["PONO"] =[{0}] FACTORY_DIV =[{1}]", tpssm34["PONO"].ToString(), tpssm34["FACTORY_DIV"].ToString());

			//获取流水号
			CString maxseq = EPGetNextSeq("ZY_PLAN_NO", conn);
			CString sm_plan_no = CString::Format("ZY%06d", atoi(maxseq));

			
			////Log::Trace("", __FUNCTION__, "tpssm34["RUN_SEQ"] =[{0}]",tpssm34["RUN_SEQ"].ToDecimal());
			tpssm11c["SM_PLAN_NO"] = sm_plan_no;
			tpssm11c["PONO_STATUS"] = 0;			
			
			sqlstr = "tpssm11c.Insert()";
			tpssm11c.Insert();

		}


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

