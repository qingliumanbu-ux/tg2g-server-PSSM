/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:魏晨祥
Date:2023-06-17
Version:1.0
Description: 新增铸余计划
**************************************************/
// C 的标准头文件部分  

#include "stdafx.h"


int f_pssm11c_cra(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm11c_ins(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
/*<remark>=========================================================
/// <summary>
/// <para> 新增；</para>
/// <para>数据库表：TPSSM11C/02/03/10               </para>
/// <para>主调用函数：TPSSM11C画面F3新增调用。        </para>
/// </summary>
/// <param name="main_backlog_code">炼钢主工序代码    </param>
/// <returns>铸余计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11c_ins)

int f_pssm11c_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	int ret = 0;
	CString	create_time = "";            /* 记录创建时刻 */
	CString ret_flag = "";

	CDbCommand cmd_tpssm10_inq(conn);

	CModel tpssm01("TPSSM01");

	EIClass outBlock;
	try
	{

		ret = f_pssm11c_cra(bcls_rec, &outBlock, conn);
		if (ret == -1)
		{
			EDLog(1, 1, "调用f_pssm11c_cra()生成铸余计划出错:[%s]", s.msg);
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
	cmd_tpssm10_inq.Close();

	return doFlag;
}
