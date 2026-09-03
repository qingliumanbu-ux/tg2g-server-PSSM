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
int f_cm_002023_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	//程序用变量
	int doFlag = 0;
	int v_count = 0;
	int v_mod = 0;
	int i = 0;
	int ret = 0;
	CString order_no = "";

	CString sqlstr("");
	CString	lpsz_tc_no("");
	EIClass inBlock;
	EIClass temp;
	EIClass temp2;

	// 创建电文处理对象
	EPEX epex(&s, conn);

	/* 实体类定义 */
	CModel tpssm03("TPSSM03");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm03_inq2(conn);

	try
	{
		/****** 获取输入参数 ***** */
		tpssm03.MergeFrom(bcls_rec->Tables["SLABSND"].Rows[0]);

		/* ***** 打印输入参数 ***** */
		////Log::Trace("", __FUNCTION__, "f_cm_002023_snd>PONO = [{0}]", tpssm03["PONO"].ToString());
		////Log::Trace("", __FUNCTION__, "f_cm_002023_snd>FACTORY_DIV = [{0}]", tpssm03["FACTORY_DIV"].ToString());

		/* ***** 检查输入参数 ***** */
		if (tpssm03["PONO"].ToString() == " ")
		{
			strcpy(s.sysmsg, "pono is null.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		lpsz_tc_no = "002023"; //方坯命令

		////电文初始化
		//////Log::Info("", __FUNCTION__, "电文初始化。");
		//if (epex.Initialize(lpsz_tc_no) < 0)
		//{
		//	sprintf(s.sysmsg, "电文初始化出错！");
		//	throw CApplicationException(-1, s.msg, s.svc_name);
		//}
		//////Log::Info("", __FUNCTION__, "电文初始化成功。");

		//读取炉次信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = " SELECT DISTINCT ORDER_NO FROM TPSSM03 WHERE PONO = @PONO ";

			break;
		}
		cmd_tpssm03_inq.SetCommandText(sqlstr);
		cmd_tpssm03_inq.Parameters.Set("PONO", tpssm03["PONO"].ToString());
		cmd_tpssm03_inq.ExecuteQuery(temp.Tables[0]);

		ret = temp.Tables[0].Rows.get_Count();
		////Log::Info("", __FUNCTION__, "PONO = [{0}]合同数量 = [{1}]", tpssm03["PONO"].ToString(), ret);
		for (int i = 0; i < ret; i++)
		{
			//初始化	
			if (epex.Initialize(lpsz_tc_no) < 0)
			{
				sprintf(s.msg, "初始化电文失败，原因[%s]", epex.GetMsg());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Info("", __FUNCTION__, "初始化完毕");

			tpssm03["ORDER_NO"] = temp.Tables[0].Rows[i]["order_no"].ToString();

			sqlstr = " SELECT * FROM TPSSM03 WHERE PONO = @PONO AND ORDER_NO = @ORDER_NO ";

			cmd_tpssm03_inq2.SetCommandText(sqlstr);
			cmd_tpssm03_inq2.Parameters.Set("PONO", tpssm03["PONO"].ToString());
			cmd_tpssm03_inq2.Parameters.Set("ORDER_NO", tpssm03["ORDER_NO"].ToString());
			cmd_tpssm03_inq2.ExecuteQuery(temp2.Tables[0]);

			for (int j = 1; j <= temp2.Tables[0].Rows.get_Count(); j++)
			{
				//////Log::Info("", __FUNCTION__, "j = [{0}]", j);
				tpssm03.MergeFrom(temp2.Tables[0].Rows[0]);
				tpssm03.TrimOrBlank();

				//拼电文数据
				if (epex.SetValue(0, tpssm03) < 0)
				{
					sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				tpssm03["SLAB_NO"] = temp2.Tables[0].Rows[j - 1]["SLAB_NO"];
				tpssm03["SLAB_SEQ_1"] = temp2.Tables[0].Rows[j - 1]["SLAB_SEQ_1"];
				tpssm03["SLAB_SEQ_2"] = temp2.Tables[0].Rows[j - 1]["SLAB_SEQ_2"];

				//板坯号
				if (epex.SetValue("slab_no", v_count, tpssm03["SLAB_NO"].ToString()) < 0)
				{
					////Log::Debug("", __FUNCTION__, "SetValue 板坯号:{0}", epex.GetMsg());
					sprintf(s.msg, epex.GetMsg());
					sprintf(s.sysmsg, epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//炉流内板坯顺序号
				if (epex.SetValue("slab_seq_1", v_count, tpssm03["SLAB_SEQ_1"].ToDecimal()) < 0)
				{
					////Log::Debug("", __FUNCTION__, "SetValue 炉流内板坯顺序号:{0}", epex.GetMsg());
					sprintf(s.msg, epex.GetMsg());
					sprintf(s.sysmsg, epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//流内板坯顺序号
				if (epex.SetValue("slab_seq_2", v_count, tpssm03["SLAB_SEQ_2"].ToDecimal()) < 0)
				{
					////Log::Debug("", __FUNCTION__, "SetValue 流内板坯顺序号:{0}", epex.GetMsg());
					sprintf(s.msg, epex.GetMsg());
					sprintf(s.sysmsg, epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//////Log::Info("", __FUNCTION__, "电文数据拼接成功。");

				v_count++;
				v_mod = j % 90;

				if (v_mod == 0)
				{
					if (j < temp2.Tables[0].Rows.get_Count())
					{
						//发送
						if (epex.SendTele() < 0)
						{
							sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
							throw CApplicationException(-1, s.msg, log.Location);
						}
						////Log::Info("", __FUNCTION__, "发送完毕");

						// 释放
						epex.Uninitialize();
						////Log::Info("", __FUNCTION__, "释放完毕");

						// 初始化	
						if (epex.Initialize(lpsz_tc_no) < 0)
						{
							sprintf(s.msg, "再次初始化电文失败，原因[%s]", epex.GetMsg());
							throw CApplicationException(-1, s.msg, log.Location);
						}
						////Log::Info("", __FUNCTION__, "再次初始化完毕");
					}
					v_count = 0;
				}
			}
			cmd_tpssm03_inq2.Close();

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

			v_count = 0;

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
