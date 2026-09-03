/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2014-10-29
Description:查询TPSSM17表中的连铸机信息。
**************************************************************************************************************/
#include "stdafx.h"

//程序用头文件
#include "tpssm17.h"
/*<remark>=========================================================
/// <summary>
/// 当前处理号查询
/// <para>数据库表：TPSSM17炼钢连铸浇注周期信息表</para>
/// <para>主调用函数：前台 PSSM11画面F2调用。   </para>
/// </summary>
/// <returns>当前处理号信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_mach)

int f_pssm11_mach(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;

	CTPSSM17 tpssm17(conn);

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);


	try
	{

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName(CString("TPSSM_CAST_INFO"));
		bcls_ret->Tables["TPSSM_CAST_INFO"].Columns.Add(tpssm17);

		sqlstr = "SELECT * FROM TPSSM17 WHERE 1= 1 "
			     " ORDER BY CC_MACH_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm17);
			//tpssm17.MergeTo(bcls_ret->Tables["TPSSM_CAST_INFO"], false);

			tpssm17.LD_STEEL_WT = (tpssm17.LD_STEEL_WT / 1000).Round(2);  //剩余钢水量 Kg转换为吨
			tpssm17.ODD_CAST_SPEED  = (tpssm17.ODD_CAST_SPEED / 100).Round(3);  //奇流铸造速度 cm/min(与L2一致)转换为 m/min	
			tpssm17.EVEN_CAST_SPEED = (tpssm17.EVEN_CAST_SPEED / 100).Round(3);  //偶流铸造速度 cm/min(与L2一致)转换为 m/min

			CDataRow& row = bcls_ret->Tables[0].Rows.Add();
			row.Merge(tpssm17);

		}//while 结束
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
