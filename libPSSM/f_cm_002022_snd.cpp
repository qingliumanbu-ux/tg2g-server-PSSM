/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    向萍
Version:   1.0
Date:      2015-05-22
Description: LOT信息下发电文
**************************************************************************************************************/
/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/


#include "epex.h"

/*<remark>=========================================================
/// <summary>
/// 发送LOT信息
/// <para>
/// 1.读取传入参数；
/// 2.组织LOT数据，发送LOT信息。
/// </para>
/// </summary>
/// <param name="PONO">制造命令号</param>
/// <returns>发送板坯制造命令。</returns>
===========================================================</remark>*/

BM2_FUNCTION_EXPORT
int f_cm_002022_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	//程序用变量
	int doFlag = 0;

	CString sqlstr("");
	CString	lpsz_tc_no("");
	EIClass inBlock;

	// 创建电文处理对象
	EPEX epex(&s, conn);

	/* 实体类定义 */
	CModel tpssm03("TPSSM03");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm03_inq(conn);

	try
	{
		/****** 获取输入参数 ***** */
		tpssm03.MergeFrom(bcls_rec->Tables["SLABSND"].Rows[0]);

		/* ***** 打印输入参数 ***** */
		////Log::Trace("", __FUNCTION__, "pono = [{0}]", tpssm03["PONO"].ToString());
		////Log::Trace("", __FUNCTION__, "FACTORY_DIV = [{0}]", tpssm03["FACTORY_DIV"].ToString());

		/* ***** 检查输入参数 ***** */
		if (tpssm03["PONO"].ToString() == " ")
		{
			strcpy(s.sysmsg, "pono is null.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		lpsz_tc_no = "002022";

		//	//读取炉次信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = CString(" SELECT * FROM TPSSM03 WHERE PONO = @PONO ");
			break;
		}
		cmd_tpssm03_inq.SetCommandText(sqlstr);
		cmd_tpssm03_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString());
		cmd_tpssm03_inq.ExecuteReader();
		while (cmd_tpssm03_inq.Read())
		{
			cmd_tpssm03_inq.Fetch(tpssm03);
			tpssm03.TrimOrBlank();

			////Log::Trace("", __FUNCTION__, "tpssm03["SLAB_NO"] = [{0}]", tpssm03["SLAB_NO"].ToString());

			//电文初始化
			////Log::Info("", __FUNCTION__, "电文初始化。");
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				sprintf(s.sysmsg, "电文002022初始化出错！");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			////Log::Info("", __FUNCTION__, "电文初始化成功。");

			//拼电文数据
			if (epex.SetValue(0, tpssm03) < 0)
			{
				sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			////Log::Info("", __FUNCTION__, "电文数据拼接成功。");

			/*发送电文 */
			if (epex.SendTele() < 0)
			{
				sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Info("", __FUNCTION__, "电文发送成功。");

			/* 释放 */
			epex.Uninitialize();
			////Log::Info("", __FUNCTION__, "电文释放成功。");
		}
		cmd_tpssm03_inq.Close();
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
