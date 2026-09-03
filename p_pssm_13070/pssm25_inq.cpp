/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-13
Version:1.0
Description: 模拟运转当前处理号生成查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件







int f_pssm25_inq(EIClass *bcls_rec,EIClass *bcls_ret,CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 模拟运转当前处理号生成查询
/// <para>炼钢运转事象信号的当前处理号查询并生成。                       </para>
/// <para>处理号代码定义: 工序号(1位) + 炉座号(1位) + 年末(1位) + 顺序号(5位)。</para>
/// <para>返回的信息：                                                    </para>
/// <para>对未开始处理的工序, 将顺序号+1；                                </para>
/// <para>对已开始处理的工序, 取其当前处理号；                            </para>
/// <para>1.根据选择输入的station_id, station_no查询当前的该工为的处理号。</para>
/// <para>2.将查询出的处理号的流水号加1处理,                              </para>
/// <para>3.判断该 PONO 的状态,对当前工序的已生产的,返回原处理号,否则返回加1处理的处理号。</para>
/// <para>数据库表：tpssm11(炼钢计划表)              </para>
/// <para>主调用函数：前台PSSM51画面处理号查询调用。         </para>
/// </summary>
/// <param name="factory_div">炼钢主工序代码    </param>
/// <param name="pono">制造命令           </param>
/// <param name="dev_code">运行设备代码   </param>
/// <param name="area_id">区域代码        </param>
/// <param name="srp_seq">第几重精炼      </param>
/// <returns>设备当前处理号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm25_inq)

//-EP_SYSTEM_HEAD_END
int f_pssm25_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	//	long stream_no; //计算流水号用
	//	int  srp_num = 0;   //记录第几重精炼用
	CDecimal dummy =0;
	CString   pono="";                   /* 制造命令号 */
	//	int    pono_status;               /* 制造命令状态 */
	CString   station_id="";             /* 炼钢工位代码 */
	CString   station_no="";             /* 炼钢工位号 */
	CString   datetime="";               /* 记录创建时间 */
	int    srp_seq=0;                   /* 精炼重数 */
	CString   ladle_arrive_time="";
	CString   ladle_leave_time="";
	CString   start_time="";
	CString   end_time="";
	int    min_charge = 0;
	bool ishave = false;

	CModel tpssmd1("TPSSMD1");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm25("TPSSM25");
	CModel tpssms1("TPSSMS1");
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm25_inq(conn);

	CString sqlstr = "";

	try
	{

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName("PROC");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CURR_PROC_NO");  //返回计算后的当前处理号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CURR_PROD_TIME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CURR_PLAN_TIME");

		datetime=CDateTime::Now().ToString("yyyyMMddHHmmss");
		CDataRow &row = bcls_ret->Tables[0].Rows.Add();
		row["CURR_PROD_TIME"] =  datetime;

		//获得输入参数
		tpssm25["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		pono                      = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().TrimOrBlank();
		tpssm12["DEV_CODE"]          = bcls_rec->Tables[0].Rows[0]["DEV_CODE"];
		tpssm12["AREA_ID"]           = bcls_rec->Tables[0].Rows[0]["AREA_ID"];
		tpssms1["START_OR_END"]      = bcls_rec->Tables[0].Rows[0]["START_OR_END"];
		srp_seq                   = bcls_rec->Tables[0].Rows[0]["SRP_SEQ"];

		//检查PONO是否为空
		if (pono.Compare(" ") == 0)
		{
			return 0;
		}

		//查询该 dev_code 的信息
		tpssmd1["FACTORY_DIV"] = tpssm25["FACTORY_DIV"];
		tpssmd1["DEV_CODE"]          = tpssm12["DEV_CODE"];
		tpssmd1["AREA_ID"]           = tpssm12["AREA_ID"];
		sqlstr = "tpssmd1.Query()";
		tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

		tpssmd1.TrimOrBlank();
		station_id = tpssmd1["STATION_ID"];
		station_no = tpssmd1["STATION_NO"];

		//查询炼钢作业计划工位运行信息表(tpssm25)中信息
		dummy = 0;

		tpssm25["STATION_ID"] = station_id;
		tpssm25["STATION_NO"] = station_no;
		sqlstr = "tpssm25.QueryCount()";
		dummy = tpssm25.QueryCount("FACTORY_DIV,STATION_ID,STATION_NO");

		if (dummy <= 0)//不存在该工序设备的, 新增该记录, 并初始化该设备处理号=" "
		{
			tpssm25["REC_CREATOR"] = s.userid;
			tpssm25["REC_CREATE_TIME"] = datetime;
			tpssm25["STATION_ID"] = station_id;
			tpssm25["STATION_NO"] = station_no;
			tpssm25["CURR_PROC_NO"] = " ";

			// EXEC SQL INSERT INTO tpssm25 VALUES (:tpssm25);
			sqlstr = "tpssm25.Insert()";
			tpssm25.Insert();

		}
		else
		{			
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT CURR_PROC_NO FROM TPSSM25 "
					" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
					"   AND STATION_ID        = @station_id "
					"   AND STATION_NO        = @station_no ";
				break;
			}

			cmd_tpssm25_inq.SetCommandText(sqlstr);
			cmd_tpssm25_inq.Parameters.Set("tpssm25.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString().TrimOrBlank() );
			cmd_tpssm25_inq.Parameters.Set("station_id"               , station_id);
			cmd_tpssm25_inq.Parameters.Set("station_no"               , station_no);
			cmd_tpssm25_inq.ExecuteReader();
			if(cmd_tpssm25_inq.Read())
			{
				tpssm25["CURR_PROC_NO"] = cmd_tpssm25_inq.GetString(1);
				tpssm25["CURR_PROC_NO"]=tpssm25["CURR_PROC_NO"].ToString().TrimOrBlank();
			}
			cmd_tpssm25_inq.Close();
		}

		//查询指定PONO的信息
		dummy = 0;

		tpssm11["FACTORY_DIV"] = tpssm25["FACTORY_DIV"];
		tpssm11["PONO"] = pono;
		sqlstr = "tpssm11.QueryCount()";
		ishave = tpssm11.Query("FACTORY_DIV,PONO");

		if (ishave == false)
		{
			CFormattable arguments[] = { pono }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000088")/*制造命令号[{0}]在出钢计划中不存在。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-------------------------------------------------------
		//查询当前PONO下的某工序的处理号
		dummy = 0;

		tpssm12["FACTORY_DIV"] = tpssm25["FACTORY_DIV"];
		tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		sqlstr = "tpssm12.QueryCount() - 1";
		dummy = tpssm12.QueryCount("FACTORY_DIV,SM_PLAN_NO,DEV_CODE");

		if (dummy == 0)
		{
			CFormattable arguments[] = { pono, tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000025")/*制造命令号[{0}]不在设备[{1}]上生产。*/, arguments, 2); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-------------------------------------------------------
		//查询当前PONO下的某工序的CHARGE_NO号
		if (tpssm12["AREA_ID"].ToDecimal() == 4)
		{			
			////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}], pono=[{1}]", tpssm25["FACTORY_DIV"].ToString(), pono);
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT MIN(CHARGE_NO) "
					"   FROM TPSSM12 "
					"  WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
					"    AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
					"    AND AREA_ID           = 4 ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm25.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				tpssm12["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(1) - 1 + srp_seq;
			}
			else
			{
				tpssm12["CHARGE_NO"] = srp_seq - 1;
			}
			cmd_tpssm12_inq.Close();

			////Log::Trace("", __FUNCTION__, "tpssm12["CHARGE_NO"] =[{0}]", tpssm12["CHARGE_NO"].ToDecimal());
		}
		else
		{
			////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}], pono=[{1}], DEV_CODE=[{2}], AREA_ID=[{3}]",
				//tpssm25["FACTORY_DIV"].ToString(), pono, tpssm12["DEV_CODE"].ToString(), tpssm12["AREA_ID"].ToDecimal());

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT MIN(CHARGE_NO) FROM TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
					"   AND DEV_CODE          = @tpssm12.DEV_CODE "
					"   AND AREA_ID           = @tpssm12.AREA_ID ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm25.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO"  , tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE"         , tpssm12["DEV_CODE"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID"          , tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				tpssm12["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(1);
			}
			else
			{
				tpssm12["CHARGE_NO"] = 0;
			}
			cmd_tpssm12_inq.Close();
			////Log::Trace("", __FUNCTION__, "tpssm1211.CHARGE_NO=[{0}]", tpssm12["CHARGE_NO"].ToDecimal());
		}

		dummy = 0;

		tpssm12["FACTORY_DIV"] = tpssm25["FACTORY_DIV"];
		tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		sqlstr = "tpssm12.QueryCount() - 2";
		dummy = tpssm12.QueryCount("FACTORY_DIV,SM_PLAN_NO,DEV_CODE,CHARGE_NO");
		if (dummy == 0)
		{
			CFormattable arguments[] = { pono, tpssm12["DEV_CODE"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000025")/*制造命令号[{0}]不在设备[{1}]上生产。*/, arguments, 2); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//1)根据传入的station_id, station_no得到dev_code
		//2)通过dev_code查到该设备在该PONO的最近的处理号
		dummy = 0;

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = " SELECT COUNT(*) FROM TPSSM12 "
				" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
				"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
				"   AND DEV_CODE          = @tpssm12.DEV_CODE "
				"   AND CHARGE_NO         = @tpssm12.CHARGE_NO "
				"   AND TRIM(PROC_NO) is not NULL ";
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("tpssm25.FACTORY_DIV",tpssm25["FACTORY_DIV"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO" , tpssm12["SM_PLAN_NO"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE" , tpssm12["DEV_CODE"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm12.CHARGE_NO", tpssm12["CHARGE_NO"].ToDecimal());
		dummy = cmd_tpssm12_inq.ExecuteScalar();

		cmd_tpssm12_inq.Close();
		if (dummy != 0)
		{
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT PROC_NO FROM TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
					"   AND DEV_CODE          = @tpssm12.DEV_CODE "
					"   AND CHARGE_NO         = @tpssm12.CHARGE_NO "
					"   AND TRIM(PROC_NO) is not NULL ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm25.FACTORY_DIV",tpssm25["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO" , tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE" , tpssm12["DEV_CODE"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.CHARGE_NO", tpssm12["CHARGE_NO"].ToDecimal());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				tpssm12["PROC_NO"] = cmd_tpssm12_inq.GetString(1);
			}
			cmd_tpssm12_inq.Close();
			tpssm25["CURR_PROC_NO"] = tpssm12["PROC_NO"];

		}
		else//处理号都为空, 表明PONO还没有进入该工序
		{
			tpssm12["PROC_NO"] = " ";
			// 处理号代码定义: 工序号(1位) + 炉座号(1位) + 年末(1位) + 顺序号(5位)
			//if (tpssm25["CURR_PROC_NO"].ToString().Compare(" ") == 0)//跨年度时,系统自动将该字段清空
			//{
			//	sprintf(tpssm25["CURR_PROC_NO"].ToString(), "%c%c%c%.5d", station_id[0], station_no[0], date_time[3], 1 );
			//}
			//else
			//{
			//	if (tpssm25["CURR_PROC_NO"].ToString()[2] != date_time[3])//跨年度
			//	{//跨年度, 从1计数
			//		stream_no = 1;
			//	}
			//	else
			//	{
			//		stream_no = (atol(&tpssm25["CURR_PROC_NO"].ToString()[3]) + 1)%100000; //转炉从第3位开始为流水号; 2009-5-8 xuwen 修改,去掉厂区号(1位)
			//	}
			//	sprintf(tpssm25["CURR_PROC_NO"].ToString(), "%c%c%c%.5d", station_id[0], station_no[0], date_time[3], stream_no);
			//}

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT PRE_PROC_NO FROM TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
					"   AND DEV_CODE          = @tpssm12.DEV_CODE "
					"   AND CHARGE_NO         = @tpssm12.CHARGE_NO ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm25.FACTORY_DIV",tpssm25["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE" , tpssm12["DEV_CODE"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.CHARGE_NO", tpssm12["CHARGE_NO"].ToDecimal());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				tpssm12["PROC_NO"] = cmd_tpssm12_inq.GetString(1);
			}
			cmd_tpssm12_inq.Close();
			tpssm25["CURR_PROC_NO"] = tpssm12["PROC_NO"];
		}

		//////Log::Trace("", __FUNCTION__, "curr_proc_no=[%s]",(const char*)tpssm25["CURR_PROC_NO"].ToString());
		row["CURR_PROC_NO"] =  tpssm25["CURR_PROC_NO"];
		//查询当前计划时间
		if(tpssm12["AREA_ID"].ToDecimal() != 4)
		{			
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT DECODE(ARRIVE_REAL_TIME,' ',LADLE_ARRIVE_TIME,ARRIVE_REAL_TIME), \
						 DECODE(LEAVE_REAL_TIME,' ',LADLE_LEAVE_TIME,LEAVE_REAL_TIME), \
						 DECODE(START_TIME_REAL,' ',START_TIME,START_TIME_REAL), \
						 DECODE(END_TIME_REAL,' ',END_TIME,END_TIME_REAL)  \
						 FROM TPSSM12 \
						 WHERE SM_PLAN_NO = @tpssm12.SM_PLAN_NO  \
						 AND DEV_CODE = @tpssm12.DEV_CODE \
						 AND AREA_ID  = @tpssm12.AREA_ID ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO",tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE",tpssm12["DEV_CODE"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID",tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				ladle_arrive_time = cmd_tpssm12_inq.GetString(1);
				ladle_leave_time  = cmd_tpssm12_inq.GetString(2);
				start_time        = cmd_tpssm12_inq.GetString(3);
				end_time          = cmd_tpssm12_inq.GetString(4);
			}
			cmd_tpssm12_inq.Close();

			if(tpssms1["START_OR_END"].ToDecimal() == 1)
			{
				row["CURR_PLAN_TIME"] =    ladle_arrive_time;
			}
			else if(tpssms1["START_OR_END"].ToDecimal() == 2)
			{
				row["CURR_PLAN_TIME"] =    start_time;
			}
			else if(tpssms1["START_OR_END"].ToDecimal() == 3)
			{
				row["CURR_PLAN_TIME"] =    end_time;
			}
			else
			{
				row["CURR_PLAN_TIME"] =    ladle_leave_time;
			}
		}
		else
		{			
			////Log::Trace("", __FUNCTION__,"取MIN CHARGE");
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT MIN(CHARGE_NO) FROM TPSSM12 "
					" WHERE FACTORY_DIV = @tpssm25.FACTORY_DIV "
					"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
					"   AND AREA_ID = 4 ";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);			
			cmd_tpssm12_inq.Parameters.Set("tpssm25.FACTORY_DIV", tpssm25["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				min_charge = cmd_tpssm12_inq.GetDecimal(1).ToInt32();
			}
			else
			{
				min_charge = 0;
			}
			cmd_tpssm12_inq.Close();

			////Log::Trace("", __FUNCTION__,"取时间");

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				sqlstr = " SELECT DECODE(ARRIVE_REAL_TIME,' ',LADLE_ARRIVE_TIME,ARRIVE_REAL_TIME), \
						 DECODE(LEAVE_REAL_TIME,' ',LADLE_LEAVE_TIME,LEAVE_REAL_TIME), \
						 DECODE(START_TIME_REAL,' ',START_TIME,START_TIME_REAL), \
						 DECODE(END_TIME_REAL,' ',END_TIME,END_TIME_REAL) \
						 FROM TPSSM12 \
						 WHERE SM_PLAN_NO = @tpssm12.SM_PLAN_NO  \
						 AND DEV_CODE  = @tpssm12.DEV_CODE \
						 AND AREA_ID   = @tpssm12.AREA_ID \
						 AND CHARGE_NO = @srp_seq + @min_charge - 1";
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);			
			cmd_tpssm12_inq.Parameters.Set("tpssm12.SM_PLAN_NO",tpssm12["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE",tpssm12["DEV_CODE"].ToString());
			cmd_tpssm12_inq.Parameters.Set("tpssm12.AREA_ID",tpssm12["AREA_ID"].ToDecimal());
			cmd_tpssm12_inq.Parameters.Set("srp_seq",srp_seq);
			cmd_tpssm12_inq.Parameters.Set("min_charge",min_charge);
			cmd_tpssm12_inq.ExecuteReader();
			if(cmd_tpssm12_inq.Read())
			{
				ladle_arrive_time = cmd_tpssm12_inq.GetString(1);
				ladle_leave_time  = cmd_tpssm12_inq.GetString(2);
				start_time        = cmd_tpssm12_inq.GetString(3);
				end_time          = cmd_tpssm12_inq.GetString(4);
			}
			cmd_tpssm12_inq.Close();

			if(tpssms1["START_OR_END"].ToDecimal() == 1)
			{
				row["CURR_PLAN_TIME"] =    ladle_arrive_time;
			}
			else if(tpssms1["START_OR_END"].ToDecimal() == 2)
			{
				row["CURR_PLAN_TIME"] =    start_time;
			}
			else if(tpssms1["START_OR_END"].ToDecimal() == 3)
			{
				row["CURR_PLAN_TIME"] =    end_time;
			}
			else
			{
				row["CURR_PLAN_TIME"] =    ladle_leave_time;
			}
		}


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
	cmd_tpssm25_inq.Close();
	cmd_tpssm12_inq.Close();
	return doFlag;

}
