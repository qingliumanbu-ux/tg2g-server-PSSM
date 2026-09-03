/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2012-01-16
Version:1.0
Description: 返送画面熔炼号和返送位置查询
Update: 2015-03-19 lijie  数据表调整
**************************************************/
//框架公用头文件，勿删 
#include "stdafx.h"

//程序用头文件



//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 返送画面熔炼号和返送位置查询
/// <para>查询熔炼号和返送位置。                            </para>
/// <para>数据库表：tpssm11/D1                    </para>
/// <para>主调用函数：PSSM18R画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>熔炼号和返送位置</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18r_rt_inq)
//-EP_SYSTEM_HEAD_END
int f_pssm18r_rt_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0; 
	CModel tpssm11("TPSSM11");
	CModel tpssmd1("TPSSMD1");
	CString sqlstr = "";
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	try
	{
		//获得输入参数
		tpssmd1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm11["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NO"];
		//设定返回参数表
		//1)熔炼号
		bcls_ret->Tables[0].set_TableName("HTNO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"HEAT_NO");

		//2)返送位置信息
		bcls_ret->Tables.Add();
		bcls_ret->Tables[1].set_TableName("POSITION");
		bcls_ret->Tables[1].Columns.Add(DT_STRING,"STATION_NAME");
		bcls_ret->Tables[1].Columns.Add(DT_STRING,"DEV_CODE");	

		//---------------查询熔炼号---------------------------
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT HEAT_NO FROM TPSSM11 \
					    WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV \
					      AND RUN_STATUS       >= 31 \
					      AND RUN_STATUS        < 91 \
					      AND HEAT_NO = @HEAT_NO \
					      AND STEEL_RETURN_CODE <> '1' ";
			break;
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("tpssmd1.FACTORY_DIV",tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("HEAT_NO", tpssm11["HEAT_NO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		while(cmd_tpssm11_inq.Read())
		{
			//tpssm11["HEAT_NO"] = cmd_tpssm11_inq.GetString(1);	
			CDataRow & row = bcls_ret->Tables[0].Rows.Add();
			row["HEAT_NO"] = cmd_tpssm11_inq.GetString(1);
		}
		cmd_tpssm11_inq.Close();

		//----------读取设备信息-----------------------
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:


			sqlstr = " SELECT DEV_CODE,STATION_NAME FROM TPSSMD1 \
						WHERE FACTORY_DIV = @tpssmd1.FACTORY_DIV \
						  AND AREA_ID           = 3 \
						ORDER BY AREA_ID, DEV_CODE ";
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("tpssmd1.FACTORY_DIV",tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while(cmd_tpssmd1_inq.Read())
		{
			//tpssm11["HEAT_NO"] = cmd_tpssm11_inq.GetString(1);	
			CDataRow & row = bcls_ret->Tables[1].Rows.Add();
			row["DEV_CODE"] = cmd_tpssmd1_inq.GetString(1);
			row["STATION_NAME"] = cmd_tpssmd1_inq.GetString(2);			
		}
		cmd_tpssmd1_inq.Close();
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
	cmd_tpssm11_inq.Close();	
	cmd_tpssmd1_inq.Close();
	return doFlag;

}
