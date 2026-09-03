
/* 程序对应表名    : TSISM38/TPSSM22
程序对应表中文名: 日出钢能力限定表/设备预定休止计划表
生成日期        : 2006-6-13 10:47:50
生成人          : 许雯
//=========================================
// 炼钢日出钢能力计算。后台 pssm21_inq, f_pssm01_chk_capa 调用
// 计算方法: 设备能力 * (1 - 设备休止时间/24 )
//-----------------------------------------
//1.查询炼钢设备能力限定表(TPSSM38)的静态参数
//2.根据设备代码, 查询指定日期下的设备休止时间
//3.工序设备限制炉数的计算
//=========================================

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



// 函数入口
BM2_FUNCTION_EXPORT
int f_pssm21_capa_cal(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	CString sqlstr = "";
	CDecimal rating_charge = 0; /* 额定工序能力 */
	CDecimal dev_capacity = 0; /* 设备限制炉数 */
	CDecimal plan_charge = 0; /* 计划炉数 */
	CDecimal stop_time = 0; /*计划休止时间, 时间长度*/
	CString plan_date = ""; //计划日期
	CString date_b = ""; /*指定计划日期的开始时刻*/
	CString date_e = ""; /*指定计划日期的结束时刻*/

	CModel tpssmdb("TPSSMDB");
	CModel tpssmd1("TPSSMD1");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm22_inq(conn);  //与DB 建立连接
	CDbCommand cmd_tpssmdb_inq(conn);  //与DB 建立连接


	try
	{
		//设定返回参数表
		bcls_ret->Tables[0].Clear();
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "rat_capa");//设备额定炉数
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "cal_capa");//设备限制炉数(加入休止因素计算后)
		bcls_ret->Tables[0].Rows.Add();

		/* 获得输入参数 */
		tpssmd1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();//厂别区分
		tpssmd1["STATION_ID"] = bcls_rec->Tables[0].Rows[0]["STATION_ID"].ToString();//工序ID
		tpssmd1["STATION_NO"] = bcls_rec->Tables[0].Rows[0]["STATION_NO"].ToString();//工序NO
		plan_date = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期

		//打印输入参数
		////Log::Info("", __FUNCTION__, "f_pssm21_capa_cal>FACTORY_DIV = [{0}]", tpssmd1["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "f_pssm21_capa_cal>STATION_ID = [{0}]", tpssmd1["STATION_ID"].ToString());
		////Log::Info("", __FUNCTION__, "f_pssm21_capa_cal>STATION_NO = [{0}]", tpssmd1["STATION_NO"].ToString());
		////Log::Info("", __FUNCTION__, "f_pssm21_capa_cal>PLAN_DATE = [{0}]", plan_date);

		//设定一天的开始时刻和结束时刻
		date_b = plan_date + "000000";
		date_e = plan_date + "235959";

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT PLAN_CHARGE  \
					FROM TPSSMDB \
					WHERE FACTORY_DIV = @FACTORY_DIV \
					AND STATION_ID = @STATION_ID \
					AND STATION_NO = @STATION_NO ";
			break;
		}
		cmd_tpssmdb_inq.SetCommandText(sqlstr);
		cmd_tpssmdb_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
		cmd_tpssmdb_inq.Parameters.Set("STATION_ID", tpssmd1["STATION_ID"].ToString());
		cmd_tpssmdb_inq.Parameters.Set("STATION_NO", tpssmd1["STATION_NO"].ToString());
		cmd_tpssmdb_inq.ExecuteReader();
		if (cmd_tpssmdb_inq.Read())
		{
			plan_charge = cmd_tpssmdb_inq.GetInt32(1);
			////Log::Info("", __FUNCTION__, "tpssmdb["PLAN_CHARGE"] = [{0}]", plan_charge);

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT nvl(sum((se-sb)*24*60), 0) stop_time \
					   FROM ( \
					   SELECT decode(sign(b - db), -1, db, b) sb, \
					   decode(sign(e - de), -1, e, de) se \
					   FROM ( \
					   SELECT  to_date(stoppage_time_start, 'YYYYMMDDhh24miss') b, \
					   to_date(stoppage_time_end, 'YYYYMMDDhh24miss') e, \
					   to_date(@date_b, 'YYYYMMDDhh24miss') db, \
					   to_date(@date_e, 'YYYYMMDDhh24miss') de \
					   FROM TPSSM22 \
					   WHERE FACTORY_DIV = @FACTORY_DIV \
					   AND DEV_CODE = @STATION_ID || @STATION_NO \
					   ) WHERE(db > b and db < e) or (db < b and de > e) or (de > b and de < e) ) ";
				break;
			}
			cmd_tpssm22_inq.SetCommandText(sqlstr);
			cmd_tpssm22_inq.Parameters.Set("date_b", date_b);
			cmd_tpssm22_inq.Parameters.Set("date_e", date_e);
			cmd_tpssm22_inq.Parameters.Set("FACTORY_DIV", tpssmd1["FACTORY_DIV"].ToString());
			cmd_tpssm22_inq.Parameters.Set("STATION_ID", tpssmd1["STATION_ID"].ToString());
			cmd_tpssm22_inq.Parameters.Set("STATION_NO", tpssmd1["STATION_NO"].ToString());
			cmd_tpssm22_inq.ExecuteReader();
			if (cmd_tpssm22_inq.Read())
			{
				stop_time = cmd_tpssm22_inq.GetInt32(1);
				////Log::Info("", __FUNCTION__, "stop_time = [{0}]", stop_time);

				//根据休止时间，重新计算该日下的设备能力
				rating_charge = rating_charge + plan_charge;
				rating_charge = rating_charge.ToInt32();
				dev_capacity = dev_capacity + ((1 - stop_time / (24 * 60)) * plan_charge);
				dev_capacity = dev_capacity.ToInt32();

				////Log::Info("", __FUNCTION__, "额定炉数rating_charge = [{0}]", rating_charge);
				////Log::Info("", __FUNCTION__, "限制炉数dev_capacity = [{0}]", dev_capacity);
			}
			cmd_tpssm22_inq.Close();
		}
		cmd_tpssmdb_inq.Close();

		bcls_ret->Tables[0].Rows[0]["rat_capa"] = rating_charge;//额定炉数
		bcls_ret->Tables[0].Rows[0]["cal_capa"] = dev_capacity;//限制炉数

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
	cmd_tpssmdb_inq.Close();
	return doFlag;

}
