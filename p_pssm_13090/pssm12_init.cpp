/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-21
Description: 出钢计划炉次条件画面初始化查询
**************************************************/
#include "stdafx.h"




/*<remark>=========================================================
/// <summary>
/// 出钢计划炉次条件画面初始化查询
/// <para>1.根据传入的炼钢单元号，查询当前设备信息。</para>
/// <para>数据库表：TPSSMD1(炼钢设备配置表)                    </para>
/// <para>主调用函数：前台PSSM12P画面加载时调用。              </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>连铸机号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12_init)


int f_pssm12_init(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int blknum;

	/* 业务变量 */
	CString  carry_div = "";
	CString  v_factory_div = "";

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
		ptable->Columns.Add(DT_STRING, "DEV_NO");  //连铸机号
		ptable->Columns.Add(DT_STRING, "DEV_CODE");
		ptable->Columns.Add(DT_STRING, "DEV_DESC");

		CDataTable &table_ds = bcls_ret->Tables.Add(*ptable);     //按模板表结构，克隆新表
		table_ds.set_TableName("DS_DEV");  //脱P, 定义表名, 与Client端dsPSSM12P.DS_DEV表名一致

		CDataTable &table_dp = bcls_ret->Tables.Add(*ptable);     //按模板表结构，克隆新表
		table_dp.set_TableName("DP_DEV");  //脱P, 定义表名

		CDataTable &table_sm = bcls_ret->Tables.Add(*ptable);     //按模板表结构，克隆新表
		table_sm.set_TableName("SM_DEV");  //脱C/炼钢, 定义表名

		CDataTable &table_sr = bcls_ret->Tables.Add(*ptable);     //按模板表结构，克隆新表
		table_sr.set_TableName("SR_DEV");  //精炼, 定义表名

		CDataTable &table_cc = bcls_ret->Tables.Add(*ptable);     //按模板表结构，克隆新表
		table_cc.set_TableName("CC_DEV");  //连铸, 定义表名



		//---------------------------------------------------
		//获得输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		////Log::Info("", __FUNCTION__, "v_factory_div=[{0}]", v_factory_div);


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
				" WHERE FACTORY_DIV = @factory_div"
				" ORDER BY AREA_ID ASC, DEV_CODE ASC "
				);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);
			tpssmd1.TrimOrBlank();

			//根据对应区域，新增相应块的记录
			CDataRow *pRow = NULL;
			if (tpssmd1["AREA_ID"].ToDecimal().ToInt32() == 1) //脱S
			{
				CDataRow &row1 = bcls_ret->Tables["DS_DEV"].Rows.Add();
				pRow = &row1;
			}
			else if (tpssmd1["AREA_ID"].ToDecimal().ToInt32() == 2) //脱磷
			{
				CDataRow &row2 = bcls_ret->Tables["DP_DEV"].Rows.Add();
				pRow = &row2;
			}
			else if (tpssmd1["AREA_ID"].ToDecimal().ToInt32() == 3) //转炉区
			{
				CDataRow &row3 = bcls_ret->Tables["SM_DEV"].Rows.Add();
				pRow = &row3;
			}
			else if (tpssmd1["AREA_ID"].ToDecimal().ToInt32() == 4) //精炼区域
			{
				CDataRow &row4 = bcls_ret->Tables["SR_DEV"].Rows.Add();
				pRow = &row4;
			}
			else if (tpssmd1["AREA_ID"].ToDecimal().ToInt32() == 5) //浇铸
			{
				CDataRow &row5 = bcls_ret->Tables["CC_DEV"].Rows.Add();
				pRow = &row5;
			}

			(*pRow)["DEV_NO"] = tpssmd1["DEV_CODE"].ToString().SubstringNE(1).Trim();
			(*pRow)["DEV_CODE"] = tpssmd1["DEV_CODE"].ToString().Trim();
			(*pRow)["DEV_DESC"] = tpssmd1["STATION_NAME"].ToString().Trim();

		}
		cmd_inq.Close();


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
