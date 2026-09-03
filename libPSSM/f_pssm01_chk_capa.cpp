/* 程序对应表名    : TPSSM21
程序对应表中文名: 炼钢作业计划日出钢能力统计表
生成日期        : 2015-9-22
生成人          : chejs
//=========================================
// 炼钢连铸机作业时间(出钢能力)校验。后台 pssm01f9_chk 调用
// 校验内容: 在一台设备上，编入的炉数是否小于该设备上的限制炉数
//-----------------------------------------
//1.根据系统配置的工序和连铸设备查询
//2.设备限制炉数的计算
//3.查询计划编入炉数
//=========================================
*/
//=========================================

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */
int f_pssm21_capa_cal(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢日出钢能力计算

// 函数入口
BM2_FUNCTION_EXPORT
int f_pssm01_chk_capa(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	int i = 0;
	CString sqlstr = "";
	CDecimal dev_capacity = 0; //设备限制炉数

	/* 使用的表结构变量 */
	CModel tpssm21("TPSSM21");//炼钢作业计划日出钢能力统计表
	CModel tpssmd1("TPSSMD1");//炼钢作业计划设备代码表

	EIClass inBlock;
	EIClass outBlock;

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm21_inq(conn);  //与DB 建立连接
	CDbCommand cmd_tpssmd1_inq(conn);  //与DB 建立连接

	try
	{
		//定义输入块的列
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别区分
		inBlock.Tables[0].Columns.Add(DT_STRING, "STATION_ID");//设备类型
		inBlock.Tables[0].Columns.Add(DT_STRING, "STATION_NO");//设备站号
		inBlock.Tables[0].Columns.Add(DT_STRING, "PLAN_DATE");//计划日期

		//设定返回参数表
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "chk_flag"); //检查标记
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "station_name");	//工序名称
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "limit_capa"); //限制能力
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "plan_charge"); //计划炉数

		/* 获得输入参数 */
		tpssmd1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();//厂别区分
		tpssm21["PLAN_DATE"] = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期

		//打印输入参数
		////Log::Info("", __FUNCTION__, "f_pssm01_chk_capa>FACTORY_DIV = [{0}]", tpssmd1["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "f_pssm01_chk_capa>PLAN_DATE = [{0}]", tpssm21["PLAN_DATE"].ToString());

		//1.根据系统配置的工序和连铸设备查询
		//预计划只关心BOF, 精炼的工序能力, 对连铸关心每台铸机能力, 因此将分别统计
		//统计BOF, 精炼工序

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT DISTINCT STATION_ID FROM TPSSMD1 WHERE AREA_ID IN('3','4') "
				"AND FACTORY_DIV = @FACTORY_DIV ";
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while (cmd_tpssmd1_inq.Read())
		{
			tpssmd1["STATION_ID"] = cmd_tpssmd1_inq.GetString(1);
			tpssmd1["STATION_NO"] = "0"; //不关心这个
			tpssmd1["DEV_CODE"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

			//1)根据station_id查询工序名称
			if (tpssmd1["STATION_ID"].ToString() == "B")
			{
				tpssmd1["STATION_NAME"] = "BOF";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "R")
			{
				tpssmd1["STATION_NAME"] = "RH";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "L")
			{
				tpssmd1["STATION_NAME"] = "LF";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "A")
			{
				tpssmd1["STATION_NAME"] = "AR";
			}

			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_ID"] = [{0}]", tpssmd1["STATION_ID"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_NO"] = [{0}]", tpssmd1["STATION_NO"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["DEV_CODE"] = [{0}]", tpssmd1["DEV_CODE"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_NAME"] = [{0}]", tpssmd1["STATION_NAME"].ToString());

			//2)根据指定的工序和计划日期, 计算工序能力
			inBlock.Tables[0].Rows.Add();
			inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
			inBlock.Tables[0].Rows[0]["STATION_ID"] = tpssmd1["STATION_ID"];//工序ID
			inBlock.Tables[0].Rows[0]["STATION_NO"] = tpssmd1["STATION_NO"];//工序NO
			inBlock.Tables[0].Rows[0]["PLAN_DATE"] = tpssm21["PLAN_DATE"];//计划日期

			//调用设备能力计算函数
			ret = f_pssm21_capa_cal(&inBlock, &outBlock, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			dev_capacity = outBlock.Tables[0].Rows[0]["cal_capa"]; //设备限制炉数

			//3)根据指定的计划日期和工序, 查询编入计划的能力
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT NVL(min(PLAN_CHARGE), 0) PLAN_CHARGE "
					"FROM TPSSM21 "
					"WHERE PLAN_DATE = @PLAN_DATE "
					"AND DEV_CODE = @DEV_CODE "
					"AND FACTORY_DIV = @FACTORY_DIV ";
				break;
			}
			cmd_tpssm21_inq.SetCommandText(sqlstr);
			cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
			cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
			cmd_tpssm21_inq.Parameters.Set("DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			cmd_tpssm21_inq.ExecuteReader();
			if (cmd_tpssm21_inq.Read())
			{
				tpssm21["PLAN_CHARGE"] = cmd_tpssm21_inq.GetInt32(1);
				////Log::Info("", __FUNCTION__, "tpssm21["PLAN_CHARGE"] = [{0}]", tpssm21["PLAN_CHARGE"].ToDecimal());
			}
			cmd_tpssm21_inq.Close();

			bcls_ret->Tables[0].Rows.Add(); //注意要新增
			if (tpssm21["PLAN_CHARGE"].ToDecimal() > dev_capacity) //计划炉数>限制炉数
			{
				bcls_ret->Tables[0].Rows[i]["chk_flag"] = "1"; //违规
			}
			else
			{
				bcls_ret->Tables[0].Rows[i]["chk_flag"] = "0"; //不违规
			}
			bcls_ret->Tables[0].Rows[i]["station_name"] = tpssmd1["STATION_NAME"]; //工序设备名称
			bcls_ret->Tables[0].Rows[i]["limit_capa"] = dev_capacity; //设备限制炉数
			bcls_ret->Tables[0].Rows[i]["plan_charge"] = tpssm21["PLAN_CHARGE"]; //计划编入炉数

			i++;

		}
		cmd_tpssmd1_inq.Close();

		//---------------------------------------------------------
		//统计连铸机

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT * FROM TPSSMD1 WHERE AREA_ID = 5 "
				"AND FACTORY_DIV = @FACTORY_DIV "
				"ORDER  BY STATION_NO ASC ";
			break;
		}
		cmd_tpssmd1_inq.SetCommandText(sqlstr);
		cmd_tpssmd1_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssmd1_inq.ExecuteReader();
		while (cmd_tpssmd1_inq.Read())
		{
			tpssmd1.Reset();
			cmd_tpssmd1_inq.Fetch(tpssmd1);
			tpssmd1.TrimOrBlank();

			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_ID"] = [{0}]", tpssmd1["STATION_ID"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_NO"] = [{0}]", tpssmd1["STATION_NO"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["DEV_CODE"] = [{0}]", tpssmd1["DEV_CODE"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_NAME"] = [{0}]", tpssmd1["STATION_NAME"].ToString());

			//1)根据指定的工序和计划日期, 计算工序能力
			inBlock.Tables[0].Rows.Add();
			inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
			inBlock.Tables[0].Rows[0]["STATION_ID"] = tpssmd1["STATION_ID"];//工序ID
			inBlock.Tables[0].Rows[0]["STATION_NO"] = tpssmd1["STATION_NO"];//工序NO
			inBlock.Tables[0].Rows[0]["PLAN_DATE"] = tpssm21["PLAN_DATE"];//计划日期

			//调用设备能力计算函数
			ret = f_pssm21_capa_cal(&inBlock, &outBlock, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			dev_capacity = outBlock.Tables[0].Rows[0]["cal_capa"]; //设备限制炉数

			//2)根据指定的计划日期和工序, 查询已编入计划的炉数
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT NVL(min(PLAN_CHARGE), 0) PLAN_CHARGE "
					"FROM TPSSM21 "
					"WHERE PLAN_DATE = @PLAN_DATE "
					"AND DEV_CODE = @DEV_CODE "
					"AND FACTORY_DIV = @FACTORY_DIV ";
				break;
			}
			cmd_tpssm21_inq.SetCommandText(sqlstr);
			cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
			cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
			cmd_tpssm21_inq.Parameters.Set("DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			cmd_tpssm21_inq.ExecuteReader();
			if (cmd_tpssm21_inq.Read())
			{
				tpssm21["PLAN_CHARGE"] = cmd_tpssm21_inq.GetInt32(1);
				////Log::Info("", __FUNCTION__, "tpssm21["PLAN_CHARGE"] = [{0}]", tpssm21["PLAN_CHARGE"].ToDecimal());
			}
			cmd_tpssm21_inq.Close();

			bcls_ret->Tables[0].Rows.Add(); //注意要新增
			if (tpssm21["PLAN_CHARGE"].ToDecimal() > dev_capacity) //计划炉数>限制炉数
			{
				bcls_ret->Tables[0].Rows[i]["chk_flag"] = "1"; //违规
			}
			else
			{
				bcls_ret->Tables[0].Rows[i]["chk_flag"] = "0"; //不违规
			}
			bcls_ret->Tables[0].Rows[i]["station_name"] = tpssmd1["STATION_NAME"]; //工序设备名称
			bcls_ret->Tables[0].Rows[i]["limit_capa"] = dev_capacity; //设备限制炉数
			bcls_ret->Tables[0].Rows[i]["plan_charge"] = tpssm21["PLAN_CHARGE"]; //计划编入炉数

			i++;

		}
		cmd_tpssmd1_inq.Close();

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
	return doFlag;

}
