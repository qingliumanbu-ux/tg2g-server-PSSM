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
BM2F_ENTERACE(pssm03_order_inq)

int f_pssm03_order_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{

	CTracer log(__FUNCTION__);
	CString   cs_pono = "",cs_cast_lot_no="";
	CString   station = "";
	CString   station_name = "";                /* 工位(设备)名称 */
	int doFlag = 0;
	CDecimal order_thick_from = 0, order_thick_to=0;
	//设置返回块参数

	

	CString sqlstr = "";
	CDbCommand comm_inq(conn);
	// 定义表的实体对象
	CModel tpssmd1("TPSSMD1");

	try
	{
		// 获取前台传入参数
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
		{
			cs_pono = bcls_rec->Tables[0].Rows[0]["PONO"];
		}
		if (bcls_rec->Tables[0].Columns.Contains("CAST_LOT_NO"))
		{
			cs_cast_lot_no = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO"];
		}
		if (bcls_rec->Tables[0].Columns.Contains("ORDER_THICK_FROM"))
		{
			order_thick_from = bcls_rec->Tables[0].Rows[0]["ORDER_THICK_FROM"].ToDecimal();
		}
		if (bcls_rec->Tables[0].Columns.Contains("ORDER_THICK_TO"))
		{
			order_thick_to = bcls_rec->Tables[0].Rows[0]["ORDER_THICK_TO"].ToDecimal();
		}
		////Log::Info("", __FUNCTION__, "sm_unit_no=[{0}]", sm_unit_no);

		/* 查询炼钢设备代码*/

		sqlstr = " SELECT T.*, T1.APN_DESC, T1.ORDER_CUST_CNAME, T1.CONSIGN_CUST_CNAME, t1.ORDER_WIDTH, t1.ORDER_THICK, t2.CC_MACH_NO, t2.ST_NO  FROM TPSSM03 t LEFT JOIN TQMOM01 T1 ON T1.ORDER_NO = T.ORDER_NO  LEFT JOIN TPSSM10 T2 ON T.PONO = T2.PONO WHERE T.PONO IN(SELECT PONO FROM TPSSM10) ";
		if (cs_pono.Trim() != "")
		{
			sqlstr += " AND T.PONO			like '%'|| @cs_pono||'%'";
		}
		if (cs_cast_lot_no.Trim() != "")
		{
			sqlstr += " AND T.CAST_LOT_NO		= @cs_cast_lot_no";
		}
		if (order_thick_from !=0)
		{
			sqlstr += " AND T1.ORDER_THICK >= " + order_thick_from.ToString();
		}
		if (order_thick_to != 0)
		{
			sqlstr += " AND T1.ORDER_THICK  <= " + order_thick_to.ToString();
		}
		Log::Trace("", __FUNCTION__, "query records. [{0}]", sqlstr);
		comm_inq.SetCommandText(sqlstr);
		comm_inq.Parameters.Set("cs_pono", cs_pono);
		comm_inq.Parameters.Set("cs_cast_lot_no", cs_cast_lot_no);
		comm_inq.ExecuteQuery(bcls_ret->Tables[0]);

		
		comm_inq.Close();

		

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

