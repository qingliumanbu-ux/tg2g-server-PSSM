/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-09-10 17:13:56
Description: 炼钢日出钢能力-查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */
//int f_pssm21_capa_cal(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//炼钢日出钢能力计算


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 炼钢日出钢能力查询
/// <para>
/// 1.根据factory_div,plan_date等条件进行炼钢日出钢能力查询。
/// </para>
/// <para>数据库表：TPSSM21(炼钢日出钢能力表)          </para>
/// <para>主调用函数：前台PSSM21画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 炼钢日出钢能力表</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm22f2_inq)

int f_pssm22f2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	int i = 0;
	CString sqlstr = "";
	CDecimal rating_charge = 0; //额定炉数 
	CDecimal dev_capacity = 0; //限制炉数
	CDecimal plan_charte = 0; //计划炉数

	CModel tpssm21("TPSSM21");
	CModel tpssmd1("TPSSMD1");

	EIClass inBlock;
	EIClass outBlock;

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssmd1_inq(conn);  //与DB 建立连接
	CDbCommand cmd_tpssm21_inq(conn);  //与DB 建立连接


	try
	{
		//设定返回参数表
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别区分
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_CODE");//工序设备
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "RATING_CHARGE");//额定炉数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "DEV_CAPACITY");//限制炉数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "PLAN_CHARGE");//计划炉数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PLAN_DATE");//计划日期

		//定义输入块的列
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别区分
		inBlock.Tables[0].Columns.Add(DT_STRING, "STATION_ID");//设备类型
		inBlock.Tables[0].Columns.Add(DT_STRING, "STATION_NO");//设备站号
		inBlock.Tables[0].Columns.Add(DT_STRING, "PLAN_DATE");//计划日期

		//获取传入参数
		tpssmd1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm21["PLAN_DATE"] = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();

		//打印输入参数
		////Log::Info("", __FUNCTION__, "pssm21_inq>factory_div = [{0}]", tpssmd1["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "pssm21_inq>plan_date = [{0}]", tpssm21["PLAN_DATE"].ToString());	

		//---------------------------------------------------------
		//预计划只关心BOF, 精炼的工序能力, 对连铸关心每台铸机能力, 因此将分别统计
		//统计BOF, 精炼工序

		sqlstr = " SELECT DISTINCT STATION_ID FROM TPSSMD1 WHERE AREA_ID IN('3','4') "
				 	"AND FACTORY_DIV = @FACTORY_DIV ";

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
			else if (tpssmd1["STATION_ID"].ToString() == "E")
			{
				tpssmd1["STATION_NAME"] = "EAF";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "R")
			{
				tpssmd1["STATION_NAME"] = "RH";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "L")
			{
				tpssmd1["STATION_NAME"] = "LF";
			}
			else if (tpssmd1["STATION_ID"].ToString() == "V")
			{
				tpssmd1["STATION_NAME"] = "VD";
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
			//ret = f_pssm21_capa_cal(&inBlock, &outBlock, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			rating_charge = outBlock.Tables[0].Rows[0]["rat_capa"]; //设备额定炉数
			dev_capacity = outBlock.Tables[0].Rows[0]["cal_capa"]; //设备限制炉数

			////Log::Info("", __FUNCTION__, "-----取计划炉数-----");
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT  NVL(MIN(PLAN_CHARGE),0) PLAN_CHARGE  \
						FROM TPSSM21 \
						WHERE PLAN_DATE = @PLAN_DATE \
						AND DEV_CODE = @DEV_CODE \
						AND FACTORY_DIV = @FACTORY_DIV ";
				break;
			}
			cmd_tpssm21_inq.SetCommandText(sqlstr);
			cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
			cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
			cmd_tpssm21_inq.Parameters.Set("DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			cmd_tpssm21_inq.ExecuteReader();
			if (cmd_tpssm21_inq.Read())
			{
				plan_charte = cmd_tpssm21_inq.GetInt32(1);
				////Log::Info("", __FUNCTION__, "计划炉数plan_charte = [{0}]", plan_charte);
			}
			cmd_tpssm21_inq.Close();

			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"]; //厂别区分
			bcls_ret->Tables[0].Rows[i]["DEV_CODE"] = tpssmd1["STATION_NAME"]; //工序设备
			bcls_ret->Tables[0].Rows[i]["RATING_CHARGE"] = rating_charge; //额定炉数
			bcls_ret->Tables[0].Rows[i]["DEV_CAPACITY"] = dev_capacity; //限制炉数
			bcls_ret->Tables[0].Rows[i]["PLAN_CHARGE"] = plan_charte; //计划炉数
			bcls_ret->Tables[0].Rows[i]["PLAN_DATE"] = tpssm21["PLAN_DATE"]; //计划日期

			i++;
			////Log::Info("", __FUNCTION__, "i = [{0}]", i);

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
			sqlstr = " SELECT * FROM TPSSMD1 WHERE AREA_ID = 5 \
					 AND FACTORY_DIV = @FACTORY_DIV \
					 ORDER  BY STATION_NO ASC ";
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

			//2)根据指定的工序和计划日期, 计算工序能力
			inBlock.Tables[0].Rows.Add();
			inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
			inBlock.Tables[0].Rows[0]["STATION_ID"] = tpssmd1["STATION_ID"];//工序ID
			inBlock.Tables[0].Rows[0]["STATION_NO"] = tpssmd1["STATION_NO"];//工序NO
			inBlock.Tables[0].Rows[0]["PLAN_DATE"] = tpssm21["PLAN_DATE"];//计划日期

			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_ID"] = [{0}]", tpssmd1["STATION_ID"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_NO"] = [{0}]", tpssmd1["STATION_NO"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["DEV_CODE"] = [{0}]", tpssmd1["DEV_CODE"].ToString());
			////Log::Info("", __FUNCTION__, "tpssmd1["STATION_NAME"] = [{0}]", tpssmd1["STATION_NAME"].ToString());

			//调用设备能力计算函数
			//ret = f_pssm21_capa_cal(&inBlock, &outBlock, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			rating_charge = outBlock.Tables[0].Rows[0]["rat_capa"]; //额定炉数
			dev_capacity = outBlock.Tables[0].Rows[0]["cal_capa"]; //限制炉数

			////Log::Info("", __FUNCTION__, "-----取计划炉数-----");
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT NVL(MIN(PLAN_CHARGE),0) PLAN_CHARGE \
						FROM TPSSM21 \
						WHERE PLAN_DATE = @PLAN_DATE \
						AND DEV_CODE = @DEV_CODE \
						AND FACTORY_DIV = @FACTORY_DIV ";
				break;
			}
			cmd_tpssm21_inq.SetCommandText(sqlstr);
			cmd_tpssm21_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
			cmd_tpssm21_inq.Parameters.Set("PLAN_DATE", tpssm21["PLAN_DATE"].ToString());
			cmd_tpssm21_inq.Parameters.Set("DEV_CODE", tpssmd1["DEV_CODE"].ToString());
			cmd_tpssm21_inq.ExecuteReader();
			if (cmd_tpssm21_inq.Read())
			{
				plan_charte = cmd_tpssm21_inq.GetInt32(1);
				////Log::Info("", __FUNCTION__, "计划炉数plan_charte = [{0}]", plan_charte);
			}
			cmd_tpssm21_inq.Close();

			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"]; //厂别区分
			bcls_ret->Tables[0].Rows[i]["DEV_CODE"] = tpssmd1["STATION_NAME"]; //工序设备
			bcls_ret->Tables[0].Rows[i]["RATING_CHARGE"] = rating_charge; //额定炉数
			bcls_ret->Tables[0].Rows[i]["DEV_CAPACITY"] = dev_capacity; //限制炉数
			bcls_ret->Tables[0].Rows[i]["PLAN_CHARGE"] = plan_charte; //计划炉数
			bcls_ret->Tables[0].Rows[i]["PLAN_DATE"] = tpssm21["PLAN_DATE"]; //计划日期

			i++;
			////Log::Info("", __FUNCTION__, "i = [{0}]", i);

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
