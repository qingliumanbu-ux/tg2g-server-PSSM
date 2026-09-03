/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2012-1-4
Description:	查询系统的转炉->连铸的工位。
**************************************************************************************************************/
#include "stdafx.h"


/*<remark>=========================================================
/// <summary>
/// 	炼钢作业计划设备代码表查询
/// <para>根据前台传入的MAIN_BACKLOG_CDOE,	对炼钢作业计划设备代码表进行查询。    </para>
/// <para>数据库表：tpssmd1(炼钢作业计划设备代码表)                 </para>
/// <para>主调用函数：前台FormPSSMD6画面中CreateTable()函数调用。 </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>转炉->连铸的工位</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssmd1_inq2)

int f_pssmd1_inq2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn) 
{

	CTracer log(__FUNCTION__);
	CString   sm_unit_no = "";  
	CString   station = "";  
	CString   station_name = "";                /* 工位(设备)名称 */
	int doFlag=0;

	//设置返回块参数

	bcls_ret->Tables[0].set_TableName("STATION");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION");
	bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_NAME");

	CString sqlstr = "";
	CDbCommand comm_inq(conn);
	// 定义表的实体对象
	CModel tpssmd1("TPSSMD1");

	try
	{
		// 获取前台传入参数
		if (bcls_rec->Tables[0].Columns.Contains("FACTORY_DIV"))
		{
			sm_unit_no = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		}
		
		////Log::Info("", __FUNCTION__, "sm_unit_no=[{0}]", sm_unit_no);

		/* 查询炼钢设备代码*/

		sqlstr = "SELECT * FROM TPSSMD1 "
			" WHERE FACTORY_DIV = @sm_unit_no "
			"   AND AREA_ID > 1 "
			"   AND AREA_ID <= 5 "
			" ORDER BY AREA_ID ASC, STATION_ID ASC, STATION_NO ASC ";

		comm_inq.SetCommandText(sqlstr);
		comm_inq.Parameters.Set("sm_unit_no", sm_unit_no);
		comm_inq.ExecuteReader();
		//循环从游标中取数据，压回前台
		int rowIndex = 0;
		while( comm_inq.Read() )
		{
			comm_inq.Fetch(tpssmd1);//把数据都压在头文件里面
			station = tpssmd1["STATION_ID"].ToString().Trim() + tpssmd1["STATION_NO"].ToString().Trim();
			bcls_ret->Tables[0].Rows.Add();
			//bcls_ret->SetColVal(1, fetchRowCount, "station", station);
			//bcls_ret->SetColVal(1, fetchRowCount, "station_name", tpssmd1.station_name);
			bcls_ret->Tables[0].Rows[rowIndex]["STATION"]      = station;
			bcls_ret->Tables[0].Rows[rowIndex]["STATION_NAME"] = tpssmd1["STATION_NAME"];
			rowIndex ++;
		}
		comm_inq.Close();

		////Log::Trace("", __FUNCTION__, "query records. [{0}]", bcls_ret->Tables[0].Rows.get_Count() );

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

