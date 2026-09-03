/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-12-26
Description:	 炼钢计划运转信号处理函数。
Update: 2015-04-07 lijie 更新数据表
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


//INSERT INTO TPSSM33(factory_div, STATION_ID, STATION_No, station_name, area_id)
//SELECT factory_div, STATION_ID, STATION_NO, station_name, area_id
//FROM tpssmd1



/*<remark>=========================================================
/// <summary>
///  炼钢计划运转信号处理函数
/// <para>处理内容：在炼钢计划运行监控表中记入相应的跟踪信息</para>
/// <para>数据库表：TPSSM33(炼钢计划运行监控表) </para>
/// <para>主调用函数：被f_pssm_run_proc调用。           </para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="pono">制造命令          </param>
/// <param name="proc_time">处理时刻         </param>
/// <param name="run_signal">运转信号    </param>
/// <param name="proc_no">处理号     </param>
/// <param name="area_id">区域号    </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm33_run_upd_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int fetchRowCount;
	int i;
	int ret;

	CString	create_time;            /* 记录创建时刻 */
	int		dummy;
	CString	proc_no;				/* 处理号 */
	CString	proc_time;				/* 处理时刻 */
	int     area_id;                    /* 炼钢区域标识 */
	bool ishave = false;

	CModel tpssmd1("TPSSMD1");
	CModel tpssms1("TPSSMS1");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm33("TPSSM33");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_exe(conn);
	CString sqlstr;
	try
	{

		create_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		//获得输入参数
		tpssms1["FACTORY_DIV"]=bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		proc_no=bcls_rec->Tables[0].Rows[0]["PROC_NO"].ToString();
		proc_time=bcls_rec->Tables[0].Rows[0]["PROC_TIME"].ToString();
		tpssm11["PONO"]=bcls_rec->Tables[1].Rows[0]["PONO"].ToString();
		tpssms1["RUN_SIGNAL"]=bcls_rec->Tables[2].Rows[0]["RUN_SIGNAL"].ToString();
		tpssmd1["AREA_ID"]=bcls_rec->Tables[2].Rows[0]["AREA_ID"].ToDecimal();


		dummy=tpssms1.QueryCount("FACTORY_DIV,RUN_SIGNAL");
		if (dummy == 0)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000149")/*运转信号[{0}]不存在，请联系维护人员。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		tpssms1.Query("FACTORY_DIV,RUN_SIGNAL");
		
		tpssmd1["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
		tpssmd1["DEV_CODE"]=tpssms1["DEV_CODE"];
		dummy = tpssmd1.QueryCount("FACTORY_DIV,DEV_CODE,AREA_ID");
		if (dummy == 0)
		{
			CFormattable arguments[] = { tpssms1["RUN_SIGNAL"].ToString(), tpssms1["DEV_CODE"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000150")/*运转信号[{0}]对应的设备[{1}]不存在，请联系维护人员。*/, arguments, 2); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
		//2)根据PONO读取原计划表中记录内容
		tpssm11["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
		ishave = tpssm11.Query("FACTORY_DIV,PONO");
		if (ishave == false)
		{
			CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000088")/*制造命令号[{0}]在出钢计划中不存在。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}
		
		tpssm12["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
		tpssm12["SM_PLAN_NO"]=tpssm11["SM_PLAN_NO"];
		tpssm12["DEV_CODE"]=tpssmd1["DEV_CODE"];

		dummy = tpssm12.QueryCount("FACTORY_DIV,SM_PLAN_NO,DEV_CODE");
		if (dummy > 0)//有该子工序计划信息时处理
		{
			//原程序无效，去除
		}

		//查询原监控表中有无记录
		dummy = 0;
		tpssm33["PONO"]=tpssm11["PONO"];
		dummy=tpssm33.QueryCount("PONO");
		if (dummy == 0)//空，说明PONO计划未进入生产；第一条记录应从HM(脱硫站)写入
		{
			tpssm33["HEAT_NO"] = tpssm11["HEAT_NO"];
			tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];

			//读取计划的出钢结束时刻
			tpssm12["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"]=tpssm11["SM_PLAN_NO"];
			tpssm12["AREA_ID"]=3;
			tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");

			tpssm33["TAP_END_TIME"]=tpssm12["END_TIME_REAL"];
		}
		else if (dummy == 1 )
		{
			tpssm33.Query("PONO");
			//读取计划的出钢结束时刻
			tpssm12["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"]=tpssm11["SM_PLAN_NO"];
			tpssm12["AREA_ID"]=3;
			tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm33["TAP_END_TIME"]=tpssm12["END_TIME_REAL"];
		}
		else if (dummy > 1)
		{
			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
				sqlstr="SELECT MAX(VIEW_POS) FROM TPSSM33 WHERE PONO=@tpssm33.PONO";
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tpssm33.PONO",tpssm33["PONO"].ToString());
			tpssm33["VIEW_POS"]=cmd_inq.ExecuteScalar();

			tpssm33.Query("PONO,VIEW_POS");
		}
		if (tpssmd1["STATION_ID"].ToString().Trim() == "X")
		{
			tpssmd1["STATION_ID"] = "E";
		}
		else if (tpssmd1["STATION_ID"].ToString().Trim() == "Y")
		{
			tpssmd1["STATION_ID"] = "B";
		}
		//更新作业计划监控表的部分字段内容(接口表送入数据)
		tpssm33["STATION_ID"] = tpssmd1["STATION_ID"];		//PK1
		tpssm33["STATION_NO"] = tpssmd1["STATION_NO"];		//PK2
		tpssm33["PONO"] = tpssm11["PONO"];				//PI号
		tpssm33["CURR_PROC_NO"] = proc_no;					//当前处理号
		//tpssm33["RUN_STATUS"] = tpssms1["RUN_SIGNAL"];		//运转信号
		tpssm33["RUN_STATUS"] = tpssm11["RUN_STATUS"];	//运转状态
		tpssm33["START_TIME"] = proc_time;				//处理时刻
		tpssm33["ST_NO"] = tpssm11["ST_NO"];
		tpssm33["RUN_SIGNAL"] = tpssms1["RUN_SIGNAL"];
		tpssm33["SM_PLAN_NOL2"] = tpssm11["SM_PLAN_NOL2"];
		if (tpssms1["START_OR_END"].ToDecimal() == 2)
		{
			tpssm33["START_TIME_REAL"] = proc_time;
		}
		else if (tpssms1["START_OR_END"].ToDecimal() == 3)
		{
			tpssm33["END_TIME_REAL"] = proc_time;
		}
		//取HEAT_NO
		if (tpssmd1["AREA_ID"].ToDecimal() == 3)
		{
			tpssm33["HEAT_NO"] = proc_no;
			tpssm11["HEAT_NO"] = proc_no;
			////Log::Trace("", __FUNCTION__, "heat_no=[{0}]",(const char*)tpssm33["HEAT_NO"].ToString());
		}
		if (tpssmd1["STATION_ID"].ToString() == "E" || tpssmd1["STATION_ID"].ToString() == "B")
		{
			tpssm33["HEAT_NO"] = proc_no;
			////Log::Trace("", __FUNCTION__, "heat_no=[{0}]",(const char*)tpssm33["HEAT_NO"].ToString());
		}
		Log::Trace("", __FUNCTION__, "f_pssm33_run_upd_n>pono=[{0}], station=[{1}{2}], RUN_SIGNAL=[{3}]",
			(const char*)tpssm33["PONO"].ToString(),(const char*)tpssm33["STATION_ID"].ToString(),(const char*)tpssm33["STATION_NO"].ToString(),tpssm33["RUN_SIGNAL"].ToString());
		//清空原PONO的内容
		if (dummy >= 1)
		{
			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
				sqlstr="UPDATE TPSSM33 SET HEAT_NO=' ',PONO=' ',ST_NO=' ',RUN_STATUS=0,CAST_NO=' ',CAST_DIV_NO=0,CAST_PONO_SUM=0,";
				sqlstr += CString("CURR_PROC_NO=' ',START_TIME=' ',TAP_END_TIME=' ',POUR_START_TIME=' ',EVENT_ID =' ' , STATUS_NAME = ' ' ,SM_PLAN_NOL2 = ' ',START_TIME_REAL= ' ',END_TIME_REAL= ' '  WHERE PONO=@tpssm33.PONO");
				break;
			}
			cmd_exe.SetCommandText(sqlstr);
			cmd_exe.Parameters.Set("tpssm33.PONO",tpssm33["PONO"].ToString());
			cmd_exe.ExecuteNonQuery();
		}

		tpssm33["CAST_NO"]=tpssm11["CAST_NO"];
		tpssm33["CAST_DIV_NO"]=tpssm11["CAST_DIV_NO"];
		tpssm33["CAST_PONO_SUM"]=tpssm11["CAST_PONO_SUM"];
		tpssm33["POUR_START_TIME"] = tpssm11["CC_REQ_TIME"];

		tpssm33["EVENT_ID"] = tpssms1["EVENT_ID"];
		tpssm33["STATUS_NAME"] = tpssms1["STATUS_NAME"];
		//20130423 HYF 需要正确的显示浇铸开始时刻
		if(tpssmd1["AREA_ID"].ToDecimal() == 5)
		{
			//读取计划的实绩开浇时刻
			tpssm12["FACTORY_DIV"]=tpssms1["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"]=tpssm11["SM_PLAN_NO"];
			tpssm12["AREA_ID"]=5;
			tpssm12.Query("FACTORY_DIV,SM_PLAN_NO,AREA_ID");
			tpssm33["POUR_START_TIME"] = tpssm12["START_TIME_REAL"];
		}
		Log::Trace("", __FUNCTION__, "11[{0}]22[{1}]333[{2}]", tpssms1["START_OR_END"].ToDecimal(), proc_time, tpssm33["START_TIME_REAL"].ToString());
		tpssm33.Update("HEAT_NO,SM_PLAN_NOL2,START_TIME_REAL,END_TIME_REAL,PONO,ST_NO,RUN_STATUS,CAST_NO,CAST_DIV_NO,CAST_PONO_SUM,CURR_PROC_NO,START_TIME,TAP_END_TIME,POUR_START_TIME,STATUS_NAME,EVENT_ID,RUN_SIGNAL"
			, "STATION_ID,STATION_NO");
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
