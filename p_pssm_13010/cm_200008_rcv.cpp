/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-07-22
Description: MMS系统接收PES上传的钢种变更
**************************************************************************************************************/

/***** C++ 的标准头文件部分 *****/ 
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/ 
#include "epex.h"



/* ***** 静态函数申明 ***** */
int f_mmsm0055_proc(EIClass *bcls_rec,EIClass *bcls_ret,CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// MMS系统接收PES上传的钢种变更
/// <para>
/// 1.读取传入参数
/// 2.修改炼钢计划状态
/// </para>
/// </summary>
/// <param name="FACTORY_DIV">主工序代码</param>
/// <param name="PONO_OLD">旧制造命令号</param>
/// <param name="PONO_NEW">新制造命令号</param>
/// <returns>炉次制造命令。</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE_TELE(cm_200008_rcv)


int f_cm_200008_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/*程序用变量*/
	int doFlag = 0;
	int ret = 0;

	/*业务变量*/
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString v_factory_div;
	CString v_pono_old;
	CString v_pono_new;
	CString v_pono_status_old;
	CString v_pono_status_new;
	CString v_st_no_old;
	CString v_st_no_new;
	CString sqlstr;
	CDecimal v_count = 0;
	
	EIClass inBlock99; //调用炼钢履历跟踪

    /*实体类定义*/
 	CDbCommand cmd_inq(conn); //与DB 建立连接。
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm01_inq(conn); //与DB 建立连接。

	/*数据库操作类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	
	try
	{
		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码				
	
		if (bcls_rec->Tables[0].Columns.Contains("PONO_OLD"))
			v_pono_old= bcls_rec->Tables[0].Rows[0]["PONO_OLD"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("PONO_NEW"))
			v_pono_new = bcls_rec->Tables[0].Rows[0]["PONO_NEW"].ToString().TrimOrBlank().ToUpper();


		////Log::Info("", __FUNCTION__,  "FACTORY_DIV=[{0}], PONO_OLD=[{1}], PONO_NEW=[{2}]", v_factory_div, v_pono_old, v_pono_new);

		//新旧制造命令都不为空时才处理
		if (v_pono_new != "" && v_pono_old != "")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT PONO_STATUS,ST_NO FROM TPSSM01 "
					" WHERE PONO  = @pono_old ");
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("pono_old", v_pono_old);
			cmd_inq.ExecuteReader();

			if (cmd_inq.Read())
			{
				v_pono_status_old = cmd_inq.GetString(1);
				v_st_no_old = cmd_inq.GetString(2);
			}

			cmd_inq.Close();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = CString(" SELECT PONO_STATUS,ST_NO FROM TPSSM01 "
					" WHERE PONO  = @pono_new ");
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("pono_new", v_pono_new);
			cmd_inq.ExecuteReader();

			if (cmd_inq.Read())
			{
				v_pono_status_new = cmd_inq.GetString(1);
				v_st_no_new = cmd_inq.GetString(2);
			}

			cmd_inq.Close();


			sqlstr = CString(
				" UPDATE  TPSSM01"
				" SET 	  PONO_STATUS   = @v_pono_status_new"
				" WHERE   PONO          = @pono_old"
				);

			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("v_pono_status_new", v_pono_status_new);   //设置修改数据项
			cmd_upd.Parameters.Set("pono_old", v_pono_old);     //设置条件数据项
			cmd_upd.ExecuteNonQuery();	              //执行修改


			sqlstr = CString(
				" UPDATE  TPSSM01"
				" SET 	  PONO_STATUS   = @v_pono_status_old"
				" WHERE   PONO          = @pono_new "
				);

			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("v_pono_status_old", v_pono_status_old);   //设置修改数据项
			cmd_upd.Parameters.Set("pono_new", v_pono_new);     //设置条件数据项
			cmd_upd.ExecuteNonQuery();

			//--------------------------------------------------------------------------------
			//修改制造命令LOT表(TPSSM02)中状态信息

			//出
			tpssm01["FACTORY_DIV"] = v_factory_div;
			tpssm01["PONO"] = v_pono_old;
			sqlstr = "tpssm01.Query()";
			tpssm01.Query("PONO,FACTORY_DIV");

			sqlstr = "select count(*) from tpssm01 where pono_status >= 18 and cast_lot_no = @cast_lot_no ";

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("cast_lot_no", tpssm01["CAST_LOT_NO"].ToString());
			v_count = cmd_tpssm01_inq.ExecuteScalar();

			if (v_count == 0)
			{
				tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

				tpssm02["LOT_STATUS"] = 3;
				tpssm02["REC_REVISOR"] = s.userid;
				tpssm02["REC_REVISE_TIME"] = dateNow;
				sqlstr = "tpssm02.Update(LOT_STATUS)";
				tpssm02.Update(
					"LOT_STATUS,"
					"REC_REVISOR,REC_REVISE_TIME",
					"FACTORY_DIV,CAST_LOT_NO"); //主键
			}


			tpssm01["FACTORY_DIV"] = v_factory_div;
			tpssm01["PONO"] = v_pono_new;
			sqlstr = "tpssm01.Query()";
			tpssm01.Query("PONO,FACTORY_DIV");

			tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

			tpssm02["LOT_STATUS"] = 4;
			tpssm02["REC_REVISOR"] = s.userid;
			tpssm02["REC_REVISE_TIME"] = dateNow;
			sqlstr = "tpssm02.Update(LOT_STATUS)";
			tpssm02.Update(
				"LOT_STATUS,"
				"REC_REVISOR,REC_REVISE_TIME",
				"FACTORY_DIV,CAST_LOT_NO"); //主键

			if (!bcls_rec->Tables.Contains("MMSM0055"))
			{
				bcls_rec->Tables.Add("MMSM0055");

			}

			if (!bcls_rec->Tables["MMSM0055"].Columns.Contains("PONO_OUT"))
			{
				bcls_rec->Tables["MMSM0055"].Columns.Add(DT_STRING, "PONO_OUT");
			}

			if (!bcls_rec->Tables["MMSM0055"].Columns.Contains("ST_NO_OUT"))
			{
				bcls_rec->Tables["MMSM0055"].Columns.Add(DT_STRING, "ST_NO_OUT");
			}

			if (!bcls_rec->Tables["MMSM0055"].Columns.Contains("PONO_IN"))
			{
				bcls_rec->Tables["MMSM0055"].Columns.Add(DT_STRING, "PONO_IN");
			}

			if (!bcls_rec->Tables["MMSM0055"].Columns.Contains("ST_NO_IN"))
			{
				bcls_rec->Tables["MMSM0055"].Columns.Add(DT_STRING, "ST_NO_IN");
			}

			bcls_rec->Tables["MMSM0055"].Rows.Add();


			//错误，传入的应该是钢种，而不是制造命令状态 HYF 20130402
			bcls_rec->Tables["MMSM0055"].Rows[0]["PONO_OUT"] = v_pono_old;
			bcls_rec->Tables["MMSM0055"].Rows[0]["ST_NO_OUT"] = v_st_no_old;
			bcls_rec->Tables["MMSM0055"].Rows[0]["PONO_IN"] = v_pono_new;
			bcls_rec->Tables["MMSM0055"].Rows[0]["ST_NO_IN"] = v_st_no_new;
			//暂时注释---等MM好了再回复		  
			doFlag = f_mmsm0055_proc(bcls_rec, bcls_ret,conn);

			if(doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//炼钢履历跟踪
			CDataRow &row91 = inBlock99.Tables["TRACE"].Rows.Add();
			row91["EVENT_ID"] = "H1"; //钢种变更
			row91["FACTORY_DIV"] = v_factory_div;
			row91["CAST_LOT_NO"] = "";
			row91["PONO"] = v_pono_old;
			row91["PONO_STATUS"] = "";

			CDataRow &row92 = inBlock99.Tables["TRACE"].Rows.Add();
			row92["EVENT_ID"] = "H1"; //钢种变更
			row92["FACTORY_DIV"] = v_factory_div;
			row92["CAST_LOT_NO"] = "";
			row92["PONO"] = v_pono_new;
			row92["PONO_STATUS"] = "";

			//-----------------------------------------------
			//炼钢履历跟踪
			ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
 
	 }
	 catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
