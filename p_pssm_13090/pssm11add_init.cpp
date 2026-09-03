/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-21
Description: 出钢计划设备信息画面初始化查询
**************************************************/
#include "stdafx.h"




/*<remark>=========================================================
/// <summary>
/// 出钢计划编制对话画面初始化查询
/// <para>1.根据传入的炼钢单元号，查询当前设备信息。</para>
/// <para>数据库表：TPSSMD1(炼钢设备配置表)                    </para>
/// <para>主调用函数：前台PSSM11Add画面加载时调用。              </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>连铸机号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11add_init)


int f_pssm11add_init(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blknum;
	int i = 0;

	/* 业务变量 */
	CString  carry_div = "";
	CString  factory_div = "";
	CString  v_station_id = "";
	CDecimal v_area_id = 0;

	EIClass  retBlk;   //返回块模板定义用

	CString sqlstr = "";

	CDbCommand cmd_inq(conn);

	try
	{
		// 定义表的实体对象
	CModel tpssmd1("TPSSMD1");

		//--------------------------------
		//设定返回查询记录信息结构
		blknum = 0; //第1块
		retBlk.Tables[blknum].set_TableName("DEV");
		CDataTable * ptable = &(retBlk.Tables[blknum]);  //定义表指针，用于模板复制
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别区分
		bcls_ret->Tables[0].Columns.Add(DT_BOOLEAN, "SELECTION");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_ID");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_CODE");

		//---------------------------------------------------
		//获得输入参数
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		////Log::Info("", __FUNCTION__, "factory_div=[{0}]", factory_div);

		//---------------------------------------------------
		//查询连铸浇铸信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT * FROM TPSSMD1 "
				" WHERE FACTORY_DIV = @factory_div "
				" ORDER BY AREA_ID ASC, DEV_CODE ASC "
				);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);
			tpssmd1.TrimOrBlank();

			////Log::Trace("", __FUNCTION__, "STATION_ID =[{0}]", tpssmd1["STATION_ID"].ToString());
			////Log::Trace("", __FUNCTION__, "STATION_NO =[{0}]", tpssmd1["STATION_NO"].ToString());

			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"]; //厂别区分
			bcls_ret->Tables[0].Rows[i]["STATION_ID"] = tpssmd1["STATION_ID"];
			bcls_ret->Tables[0].Rows[i]["STATION_NO"] = tpssmd1["STATION_NO"];
			bcls_ret->Tables[0].Rows[i]["DEV_CODE"] = tpssmd1["DEV_CODE"];

			if (tpssmd1["AREA_ID"].ToDecimal() == 5)
			{
				if (v_area_id == tpssmd1["AREA_ID"].ToDecimal())
				{
					bcls_ret->Tables[0].Rows[i]["SELECTION"] = false;
				}
				else
				{
					bcls_ret->Tables[0].Rows[i]["SELECTION"] = true;
				}
			}
			else
			{
				if (v_station_id == tpssmd1["STATION_ID"].ToString())
				{
					bcls_ret->Tables[0].Rows[i]["SELECTION"] = false;
				}
				else
				{
					bcls_ret->Tables[0].Rows[i]["SELECTION"] = true;
				}
			}
			
			i++;
			v_station_id = tpssmd1["STATION_ID"];
			v_area_id = tpssmd1["AREA_ID"];
		}
		cmd_inq.Close();

		bcls_ret->Tables.Add("TPSSM26");

		//读取TPSSM26，默认机组
		//查询连铸浇铸信息
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT * FROM TPSSM26 "
				" WHERE FACTORY_DIV = @factory_div "
				" ORDER BY CC_MACH_NO ASC "
				);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["TPSSM26"]);


	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = -1;
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
