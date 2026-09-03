/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2013-06-26
Description: LOT信息确定
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
//#include "tpshra1.h"




//程序用头文件
//int f_cm_200010_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //向L4送LOT确定状态
//int f_heat_l2_99_send(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//向L2发炉次确定电文
int f_pshp_dhcr_del_sm(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);
int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入

/*<remark>=========================================================
/// <summary>
///
/// </summary>
/// <param name=" "> </param>
/// <returns>下发的切割计划信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm02_lot_confm)

//-EP_SYSTEM_HEAD_END
int f_pssm02_lot_confm(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int fetchRowCount = 0;
	int ret;
	CString sqlstr = "";

	//2019zqq add
	CString sqlstr1 = "";
	CString v_hot_charge_flag = "";
	int v_count1 = 0;
	int v_row_count_h1 = 0;
	int v_row_count_h2 = 0;

	//CTPSHRA1 tpshra1(conn);
	CModel tpssm01("TPSSM01");
	CModel tpssm03("TPSSM03");
	CModel tpssm02("TPSSM02");
	CModel tpssm99("TPSSM99");


	EIClass bcls_dhcr_del_h1;
	bcls_dhcr_del_h1.Tables[0].set_TableName("PSHR");
	bcls_dhcr_del_h1.Tables[0].Columns.Add(DT_STRING, "EVENT_ID");
	bcls_dhcr_del_h1.Tables[0].Columns.Add(DT_STRING, "PREC_SLAB_NO");

	EIClass bcls_dhcr_del_h2;
	bcls_dhcr_del_h2.Tables[0].set_TableName("PSHR");
	bcls_dhcr_del_h2.Tables[0].Columns.Add(DT_STRING, "EVENT_ID");
	bcls_dhcr_del_h2.Tables[0].Columns.Add(DT_STRING, "PREC_SLAB_NO");

	//------------------------------------
	//炉次确定发送至L2
	//EIClass inSENDl2;
	//inSENDl2.Tables[0].set_TableName("PSSM");//调用切断计划明细
	//inSENDl2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	//inSENDl2.Tables[0].Columns.Add(DT_STRING, "PONO");
	//inSENDl2.Tables[0].Columns.Add(DT_STRING, "SLAB_NO");
	//inSENDl2.Tables[0].Columns.Add(DT_STRING, "ACTION_CODE");
	//inSENDl2.Tables[0].Rows.Add();

	//------------------------------------
	//炉次确定发送至L2
	EIClass inBsendl2;
	inBsendl2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inBsendl2.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	//inBsendl2.Tables[0].Columns.Add(DT_STRING, "HEAT_CONFM_FLAG");
	inBsendl2.Tables[0].Columns.Add(DT_STRING, "PONO");
	inBsendl2.Tables[0].Columns.Add(DT_STRING, "ST_NO");
	inBsendl2.Tables[0].Rows.Add();

	//------------------------------------
	//炉次确定记履历
	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);

	/* 业务变量 */
	int v_count = 0;
	int v_slab_num = 0;
	CString v_cast_lot_no = "";
	CString date_time = "";
	CString v_factory_div = "";
	CString v_dhcr_del_flag = "0";
	CString v_slab_no = "";
	CString v_slab_dest = "";

	/* 实体类定义 */

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);

	//调用电文接口
	EIClass inBlock;

	try
	{
		//--------------------------------
		//定义函数调用输入块结构
		inBlock.Tables[0].set_TableName("200010");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "LOT_CONFM_FLAG");
		inBlock.Tables[0].Columns.Add(DT_STRING, "SLAB_NUM");
		inBlock.Tables[0].Rows.Add();  //只生成一行

		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss").Substring(0,8);
		v_cast_lot_no = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO"].ToString();
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		Log::Info("", __FUNCTION__, "pssm02_lot_confm>v_cast_lot_no = [{0}]", v_cast_lot_no);
		Log::Info("", __FUNCTION__, "pssm02_lot_confm>v_factory_div = [{0}]", v_factory_div);
		Log::Info("", __FUNCTION__, "pssm02_lot_confm>date_time = [{0}]", date_time);

		//--------------------------------
		//条件校验1-不能有炉次正在生产
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = " SELECT COUNT(1)  "
				"    FROM TPSSM01 "
				"   WHERE CAST_LOT_NO = @CAST_LOT_NO "
				"     AND PONO_STATUS > 16 "
				"     AND PONO_STATUS < 91 ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
		v_count = cmd_inq.ExecuteScalar().ToInt32();

		if (v_count != 0)
		{
			CFormattable arguments[] = { v_cast_lot_no }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "浇注批号[{0}]下还有PONO未炉次确定，请联系炼钢调度。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-----------------------------------------------
		//条件校验2-1780热装有未入库的坯子不能进行炉次确定
		//取热装标记 去向
		sqlstr1 = "  select max(HOT_CHARGE_FLAG),MAX(SLAB_DEST) from tpssm01 where cast_lot_no=@v_cast_lot_no   ";
		cmd_tpssm03_inq.SetCommandText(sqlstr1);
		cmd_tpssm03_inq.Parameters.Set("v_cast_lot_no", v_cast_lot_no);
		cmd_tpssm03_inq.ExecuteReader();
		if (cmd_tpssm03_inq.Read())
		{
			v_hot_charge_flag = cmd_tpssm03_inq.GetString(1);
			v_slab_dest = cmd_tpssm03_inq.GetString(2);
		}
		cmd_tpssm03_inq.Close();

		Log::Trace("", __FUNCTION__, "==v_hot_charge_flag==[{0}],v_slab_dest=[{1}]", v_hot_charge_flag, v_slab_dest);

		if (v_hot_charge_flag.Trim() == "2" && v_slab_dest=="14")
		{
			//条件校验2-有未入库的坯子不能进行炉次确定
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr =
					" select count(*) from Tmmsm01									"
					" where in_flag = '0'											"
					" and pono in(select pono from tpssm01 where cast_lot_no =@v_cast_lot_no)  "
					" and hot_charge_flag<>'0'  "
					" and MAT_LINE_TYPE='SM'  "
					;
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_cast_lot_no", v_cast_lot_no);
			v_count = cmd_inq.ExecuteScalar().ToInt32();
			if (v_count != 0)
			{
				CFormattable arguments[] = { v_cast_lot_no }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "浇注批号[{0}]下还有坯子在辊道上，不允许进行LOT完成！", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
		
		}

		//-----------------------------------------------
		//获取浇次号下面的铸坯产出支数，校验切割信息有没有接收成功
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT COUNT(1) "
				"   FROM TPSSM03 "
				"  WHERE FACTORY_DIV = @FACTORY_DIV "
				"    AND CAST_LOT_NO = @CAST_LOT_NO "
				"    AND SLAB_PROD_FLAG = '1' "
				);
			break;
		}
		cmd_tpssm03_inq.SetCommandText(sqlstr);
		cmd_tpssm03_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
		cmd_tpssm03_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
		v_slab_num = cmd_tpssm03_inq.ExecuteScalar().ToInt32();
	

		//-----------------------------------------------
		//LOT确定信息发送L2
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT * "
				"   FROM TPSSM01 "
				"  WHERE FACTORY_DIV = @FACTORY_DIV "
				"    AND CAST_LOT_NO = @CAST_LOT_NO "
				//"    AND SLAB_PROD_FLAG!='1'  "//未产出的坯[0/9]
				);
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
		cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{

			cmd_inq.Fetch(tpssm01);

			inBsendl2.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			//inBsendl2.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11.SM_PLAN_NO;
			//inBsendl2.Tables[0].Rows[0]["HEAT_CONFM_FLAG"] = "1";
			inBsendl2.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
			inBsendl2.Tables[0].Rows[0]["ST_NO"] = tpssm01["ST_NO"];
			//--------------------------
			//炉次确定下发L2
			//ret = f_heat_l2_99_send(&inBsendl2, bcls_ret, conn);
			if (ret != 0)
			{
				//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//--------------------------
			//记炉次确定状态 至履历表
			tpssm99.CopyFrom(tpssm01);
			tpssm99["EVENT_ID"] = "Z1";
			tpssm99["VALID_FLAG"] = "1";
			tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

			ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

		}
		cmd_inq.Close();


		//-----------------------------------------------
		//发送LOT确定电文发送L4
		inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
		inBlock.Tables[0].Rows[0]["CAST_LOT_NO"] = v_cast_lot_no;
		if (v_dhcr_del_flag == "0")
		{
			inBlock.Tables[0].Rows[0]["LOT_CONFM_FLAG"] = "1";
		}
		else
		{
			inBlock.Tables[0].Rows[0]["LOT_CONFM_FLAG"] = "D";
		}
		inBlock.Tables[0].Rows[0]["SLAB_NUM"] = v_slab_num;

		//ret = f_cm_200010_snd(&inBlock, bcls_ret, conn);
		if (ret != 0) //调用不成功
		{
			//Log::Trace("", __FUNCTION__,  "f_ps200021_snd()发送炼钢lot状态出错:[{0}]", (const char*) s.msg);
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-----------------------------------------------
		//更新计划表状态
		if (v_dhcr_del_flag == "0")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " UPDATE TPSSM01  "
					"	SET PONO_STATUS = 91, "
					"       SLAB_REQ = '1' "
					"   WHERE CAST_LOT_NO = @CAST_LOT_NO "
					"     AND PONO_STATUS <= 16 ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();
			
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " INSERT INTO TPSSM40  "
					"	SELECT * FROM TPSSM10 T "
					"   WHERE T.CAST_LOT_NO = @CAST_LOT_NO "
					"     AND T.PONO_STATUS <= 16 ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " DELETE FROM TPSSM10 T "
					"   WHERE T.CAST_LOT_NO = @CAST_LOT_NO "
					"     AND T.PONO_STATUS <= 16 ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " UPDATE TPSSM02  "
					"     SET LOT_CONFM_FLAG = '1', "
					"         LOT_STATUS = '9', "
					"         LOT_CONFM_DATE=@date_time "
					"   WHERE CAST_LOT_NO = @CAST_LOT_NO ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.Parameters.Set("date_time", date_time);//取LOT确定日期
			cmd_inq.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " UPDATE TPSSM03  "
					"     SET SLAB_PROD_FLAG = '9' "
					"   WHERE CAST_LOT_NO = @CAST_LOT_NO "
					"     AND SLAB_PROD_FLAG != '1' ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();
		}
		else
		{
			Log::Info("", __FUNCTION__, "===v_cast_lot_no = [{0}]", v_cast_lot_no);
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " DELETE FROM TPSSM01  "
					"   WHERE CAST_LOT_NO = @CAST_LOT_NO "
					"     AND PONO_STATUS <= 16 ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " DELETE FROM TPSSM10 T "
					"   WHERE T.CAST_LOT_NO = @CAST_LOT_NO "
					"     AND T.PONO_STATUS <= 16 ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " DELETE FROM TPSSM02  "
					"   WHERE CAST_LOT_NO = @CAST_LOT_NO ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = " DELETE FROM TPSSM03  "
					"   WHERE CAST_LOT_NO = @CAST_LOT_NO "
					"     AND SLAB_PROD_FLAG != '1' ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_LOT_NO", v_cast_lot_no);
			cmd_inq.ExecuteNonQuery();
		}


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	return doFlag;
}
