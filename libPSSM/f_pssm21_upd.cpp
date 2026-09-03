/* 程序对应表名    : TPSSM21
// 程序对应表中文名: 炼钢日出钢能力统计表
// 生成日期        : 2013-9-15
// 生成人          : chejs
//============================================
// 日出钢能力统计。后台 pssm01f3_inc (编入计划)调用
//--------------------------------------------
// 根据指定的生产日期，将收池表中的所有制造命令所经过的工序，
// 进行纵向统计
//1.日出钢能力统计表中计划数清0
//2.循环读取收池的命令
//3.统计连铸机的计划数量
//4.分别统计转炉、精炼各工序的计划数量
//--------------------------------------------
//============================================
*/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/





// 函数入口
BM2_FUNCTION_EXPORT
int f_pssm21_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	int count = 0;
	CString date_Now = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString sqlstr = "";

	CModel tpssm21("TPSSM21");//炼钢作业计划日出钢能力统计表
	CModel tpssm01("TPSSM01");//炼钢连铸炉次制造命令表
	CModel tpssm03("TPSSM03");//炼钢连铸制造命令板坯表
	CModel tpssmd1("TPSSMD1");//炼钢作业计划设备代码表

	EIClass inBlock;

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接
	CDbCommand cmd_tpssm21_inq(conn);  //与DB 建立连接
	CDbCommand cmd_tpssm38_inq(conn);  //与DB 建立连接
	CDbCommand cmd_tpssmd1_inq(conn);  //与DB 建立连接


	try
	{
		/* 获得输入参数 */
		tpssm21["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();//厂别区分
		tpssm21["PLAN_DATE"] = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期

		//打印输入参数
		////Log::Info("", __FUNCTION__, "FACTORY_DIV = [{0}]", tpssm21["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "PLAN_DATE = [{0}]", tpssm21["PLAN_DATE"].ToString());

		//---------------------------------------------------------------------------
		//1.日出钢能力统计表中计划数清0
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = "UPDATE TPSSM21 "
				"SET PLAN_CHARGE = 0 "
				"WHERE FACTORY_DIV = @FACTORY_DIV "
				"AND PLAN_DATE = @PLAN_DATE ";
			break;
		}
		cmd_tpssm21_inq.SetCommandText(sqlstr);
		cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssm21["FACTORY_DIV"].ToString());
		cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
		ret = cmd_tpssm21_inq.ExecuteNonQuery();
		if (ret >= 0)
		{
			////Log::Info("", __FUNCTION__, "update tpssm21 success.");
		}
		cmd_tpssm21_inq.Close();

		//---------------------------------------------------------------------------
		//2.循环读取收池的炉次命令, 不分连铸机
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT * FROM TPSSM01 "
				" WHERE FACTORY_DIV = @FACTORY_DIV "
				" AND	PLAN_DATE = @PLAN_DATE "
				" AND	PONO_STATUS >= 13 "
				" ORDER BY CC_MACH_NO ASC, CC_SEQ ASC ";
				break;
		}
		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm21["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
		////Log::Debug("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_tpssm01_inq.ExecuteReader();
		while (cmd_tpssm01_inq.Read())
		{
			tpssm01.Reset();
			cmd_tpssm01_inq.Fetch(tpssm01);
			tpssm01.TrimOrBlank();

			////Log::Info("", __FUNCTION__, "tpssm01["CC_MACH_NO"] = [{0}]", tpssm01["CC_MACH_NO"].ToString());			

			//--------------------------------------------------------------------
			//1.根据连铸机号，统计不同连铸机计划数量

			tpssmd1["FACTORY_DIV"] = tpssm21["FACTORY_DIV"];
			tpssmd1["STATION_NO"] = tpssm01["CC_MACH_NO"];
			tpssmd1["AREA_ID"] = 5;
			count = tpssmd1.QueryCount("FACTORY_DIV,STATION_NO,AREA_ID");
			if (count != 1)
			{
				strcpy(s.msg, "设备代码维护有误，请联系系统维护人员");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssmd1.Query("FACTORY_DIV,STATION_NO,AREA_ID");
			tpssmd1.TrimOrBlank();

			tpssm21["DEV_CODE"] = tpssmd1["DEV_CODE"];
			////Log::Info("", __FUNCTION__, "tpssm21["DEV_CODE"] = [{0}]", tpssm21["DEV_CODE"].ToString());

			count = tpssm21.QueryCount("FACTORY_DIV,PLAN_DATE,DEV_CODE");
			////Log::Info("", __FUNCTION__, "CC_count = [{0}]", count);

			if (count == 0)
			{
				tpssm21["PLAN_CHARGE"] = 1; //计划炉数

				tpssm21["REC_CREATE_TIME"] = date_Now;
				tpssm21["REC_CREATOR"] = s.userid;
				tpssm21["REC_REVISE_TIME"] = date_Now;
				tpssm21["REC_REVISOR"] = s.userid;
				tpssm21["COMPANY_CODE"] = s.company_code;
				tpssm21["COMPANY_NAME"] = s.company_name;

				sqlstr = "tpssm21.insert()";
				tpssm21.Insert();
			}
			else if (count == 1)
			{
				sqlstr = "tpssm21.Query(FACTORY_DIV,PLAN_DATE,DEV_CODE)";
				tpssm21.Query("FACTORY_DIV,PLAN_DATE,DEV_CODE");

				tpssm21["PLAN_CHARGE"] = tpssm21["PLAN_CHARGE"].ToDecimal() + 1;
				tpssm21["REC_REVISE_TIME"] = date_Now;
				tpssm21["REC_REVISOR"] = s.userid;

				sqlstr = "tpssm21.Update(PLAN_CHARGE,REC_REVISE_TIME,REC_REVISOR)";
				tpssm21.Update("PLAN_CHARGE,REC_REVISE_TIME,REC_REVISOR", "FACTORY_DIV,PLAN_DATE,DEV_CODE");

			}


			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:				// MS SQL Server数据库
			//case DB_KIND_ORACLE:	        // Oracle 数据库
			//default:
			//	sqlstr = " SELECT COUNT(*) "
			//			"FROM TPSSM21 "
			//			"WHERE FACTORY_DIV = @FACTORY_DIV "
			//			"AND PLAN_DATE = @PLAN_DATE "
			//			"AND DEV_CODE = @DEV_CODE ";
			//	break;
			//}
			//cmd_tpssm21_inq.SetCommandText(sqlstr);
			//cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssm21["FACTORY_DIV"].ToString());
			//cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
			//cmd_tpssm21_inq.Parameters.Set("DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			//cmd_tpssm21_inq.ExecuteReader();
			//if (cmd_tpssm21_inq.Read())
			//{
			//	count = cmd_tpssm21_inq.GetInt32(1);
			//	////Log::Info("", __FUNCTION__, "CC_count = [{0}]", count);

			//	if (count <= 0) //无记录, 新增
			//	{
			//		tpssm21["PLAN_CHARGE"] = 1; //计划炉数
			//		tpssm21["DEV_CODE"] = tpssmd1["DEV_CODE"];//工序设备

			//		////Log::Info("", __FUNCTION__, "tpssm21["PLAN_CHARGE"] = [{0}]", tpssm21["PLAN_CHARGE"].ToDecimal());
			//		////Log::Info("", __FUNCTION__, "tpssm21["FACTORY_DIV"] = [{0}]", tpssm21["FACTORY_DIV"].ToString());
			//		////Log::Info("", __FUNCTION__, "tpssm21["DEV_CODE"] = [{0}]", tpssm21["DEV_CODE"].ToString());
			//		////Log::Info("", __FUNCTION__, "tpssm21["PLAN_DATE"] = [{0}]", tpssm21["PLAN_DATE"].ToString());

			//		tpssm21["REC_CREATE_TIME"] = date_Now;
			//		tpssm21["REC_CREATOR"] = s.userid;
			//		tpssm21["REC_REVISE_TIME"] = date_Now;
			//		tpssm21["REC_REVISOR"] = s.userid;
			//		tpssm21["COMPANY_CODE"] = s.company_code;
			//		tpssm21["COMPANY_NAME"] = s.company_name;
			//		
			//		sqlstr = "tpssm21.insert()";
			//		tpssm21.Insert();
			//	}
			//	else
			//	{
			//		//修改当前计划数
			//		switch (conn->DatabaseKind)
			//		{
			//		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			//		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			//		case DB_KIND_MSSQL:				// MS SQL Server数据库
			//		case DB_KIND_ORACLE:	        // Oracle 数据库
			//		default:
			//			sqlstr = "UPDATE TPSSM21 "
			//					"SET PLAN_CHARGE = PLAN_CHARGE + 1, "
			//					"REC_REVISOR = @REC_REVISOR, "
			//					"REC_REVISE_TIME = @REC_REVISE_TIME "
			//					"WHERE FACTORY_DIV = @FACTORY_DIV "
			//					"AND PLAN_DATE = @PLAN_DATE "
			//					"AND DEV_CODE = @DEV_CODE ";
			//			break;
			//		}
			//		cmd_tpssm21_inq.SetCommandText(sqlstr);
			//		cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssm21["FACTORY_DIV"].ToString());
			//		cmd_tpssm21_inq.Parameters.Set("REC_REVISOR", s.userid);
			//		cmd_tpssm21_inq.Parameters.Set("REC_REVISE_TIME", date_Now);
			//		cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
			//		cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssmd1["DEV_CODE"].ToString());
			//		cmd_tpssm21_inq.ExecuteNonQuery();
			//		cmd_tpssm21_inq.Close();

			//	}

			//}
			//cmd_tpssm21_inq.Close();

			//--------------------------------------------------------------------
			//2.根据钢区工艺途径码，分别统计转炉、精炼各工序的计划数量

			////Log::Info("", __FUNCTION__, "tpssm01["BACKLOG_EA"] = [{0}]", tpssm01["BACKLOG_EA"].ToString());
			int backlog_length = strlen(tpssm01["BACKLOG_EA"].ToString());
			for (int i = 0; i < backlog_length - 1; i++)
			{
				tpssm21["DEV_CODE"] = tpssm01["BACKLOG_EA"].ToString().SubstringNE(i, 1) + "0";
				////Log::Info("", __FUNCTION__, "tpssm21["DEV_CODE"] = [{0}]", tpssm21["DEV_CODE"].ToString());

				count = tpssm21.QueryCount("FACTORY_DIV,PLAN_DATE,DEV_CODE");
				////Log::Info("", __FUNCTION__, "00_count = [{0}]", count);

				if (count == 0)
				{
					tpssm21["PLAN_CHARGE"] = 1; //计划炉数

					tpssm21["REC_CREATE_TIME"] = date_Now;
					tpssm21["REC_CREATOR"] = s.userid;
					tpssm21["REC_REVISE_TIME"] = date_Now;
					tpssm21["REC_REVISOR"] = s.userid;
					tpssm21["COMPANY_CODE"] = s.company_code;
					tpssm21["COMPANY_NAME"] = s.company_name;

					sqlstr = "tpssm21.insert()";
					tpssm21.Insert();
				}
				else if (count == 1)
				{
					sqlstr = "tpssm21.Query(FACTORY_DIV,PLAN_DATE,DEV_CODE)";
					tpssm21.Query("FACTORY_DIV,PLAN_DATE,DEV_CODE");

					tpssm21["PLAN_CHARGE"] = tpssm21["PLAN_CHARGE"].ToDecimal() + 1;
					tpssm21["REC_REVISE_TIME"] = date_Now;
					tpssm21["REC_REVISOR"] = s.userid;

					sqlstr = "tpssm21.Update(PLAN_CHARGE,REC_REVISE_TIME,REC_REVISOR)";
					tpssm21.Update("PLAN_CHARGE,REC_REVISE_TIME,REC_REVISOR", "FACTORY_DIV,PLAN_DATE,DEV_CODE");

				}

				//switch (conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:				// MS SQL Server数据库
				//case DB_KIND_ORACLE:	        // Oracle 数据库
				//default:
				//	sqlstr = " SELECT COUNT(*) "
				//		"FROM TPSSM21 "
				//		"WHERE FACTORY_DIV = @FACTORY_DIV "
				//		"AND PLAN_DATE = @PLAN_DATE "
				//		"AND DEV_CODE = @DEV_CODE ";
				//	break;
				//}
				//cmd_tpssm21_inq.SetCommandText(sqlstr);
				//cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssm21["FACTORY_DIV"].ToString());
				//cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
				//cmd_tpssm21_inq.Parameters.Set("DEV_CODE", tpssm21["DEV_CODE"].ToString());
				//cmd_tpssm21_inq.ExecuteReader();
				//if (cmd_tpssm21_inq.Read())
				//{
				//	count = cmd_tpssm21_inq.GetInt32(1);
				//	////Log::Info("", __FUNCTION__, "00_count = [{0}]", count);
				//	if (count <= 0) //无记录, 新增
				//	{
				//		tpssm21["PLAN_CHARGE"] = 1;
				//		tpssm21["DEV_CODE"] = tpssm21["DEV_CODE"];

				//		tpssm21["REC_CREATE_TIME"] = date_Now;
				//		tpssm21["REC_CREATOR"] = s.userid;
				//		tpssm21["COMPANY_CODE"] = s.company_code;
				//		tpssm21["COMPANY_NAME"] = s.company_name;

				//		sqlstr = "tpssm21.insert()";
				//		tpssm21.Insert();
				//	}
				//	else
				//	{
				//		//修改当前计划数
				//		switch (conn->DatabaseKind)
				//		{
				//		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				//		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//		case DB_KIND_MSSQL:				// MS SQL Server数据库
				//		case DB_KIND_ORACLE:	        // Oracle 数据库
				//		default:
				//			sqlstr = "UPDATE TPSSM21 "
				//				"SET PLAN_CHARGE = PLAN_CHARGE + 1, "
				//				"REC_REVISOR = @REC_REVISOR, "
				//				"REC_REVISE_TIME = @REC_REVISE_TIME "
				//				"WHERE FACTORY_DIV = @FACTORY_DIV "
				//				"AND PLAN_DATE = @PLAN_DATE "
				//				"AND DEV_CODE = @DEV_CODE ";
				//			break;
				//		}
				//		cmd_tpssm21_inq.SetCommandText(sqlstr);
				//		cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssm21["FACTORY_DIV"].ToString());
				//		cmd_tpssm21_inq.Parameters.Set("REC_REVISOR", s.userid);
				//		cmd_tpssm21_inq.Parameters.Set("REC_REVISE_TIME", date_Now);
				//		cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
				//		cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["DEV_CODE"].ToString());
				//		cmd_tpssm21_inq.ExecuteNonQuery();
				//		cmd_tpssm21_inq.Close();
				//	}

				//}
				//cmd_tpssm21_inq.Close();
			}
		}
		cmd_tpssm01_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
	cmd_tpssm38_inq.Close();
	return doFlag;

}
