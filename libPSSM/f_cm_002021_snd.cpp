/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    向萍
Version:   1.0
Date:      2015-05-22
Description: 炉次制造命令下发电文
**************************************************************************************************************/
/***** C++ 的标准头文件部分 *****/ 
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/ 




#include "epex.h"

int f_cm_002022_snd(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 发送炉次制造命令
/// <para>
/// 1.读取传入参数；
/// 2.组织炉次数据，发送炉次信息；
/// 3.组织铸坯数据，发送铸坯信息。
/// </para>
/// </summary>
/// <param name="OP_FLG">操作区分</param>
/// <param name="PONO">制造命令号</param>
/// <param name="CAST_LOT_NO">CAST_LOT号</param>
/// <returns>发送炉次制造命令。</returns>
===========================================================</remark>*/

BM2_FUNCTION_EXPORT
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkNum = 0;

	/* 业务变量 */
	CString sqlstr(""); 
	CString	lpsz_tc_no("");
	CString v_proc_div = "";
	EIClass inBlock;

	// 创建电文处理对象
	EPEX epex(&s, conn);

	/* 实体类定义 */
	
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);

	/* 数据库操作类定义 */	

	try
	{
		/* ***** 获取输入参数 ***** */
		tpssm01.MergeFrom(bcls_rec->Tables["PONOSEND"].Rows[0]);

		if(bcls_rec->Tables["PONOSEND"].Columns.Contains("MARKS1"))    //1:新增;2:LOT删除;3:PONO删除
		v_proc_div = bcls_rec->Tables["PONOSEND"].Rows[0]["MARKS1"].ToString().Trim();
		
		////Log::Trace("", __FUNCTION__, "f_cm_002021_snd>v_proc_div = [{0}]", v_proc_div);
		////Log::Trace("", __FUNCTION__, "f_cm_002021_snd>tpssm01["PONO"] = [{0}]", tpssm01["PONO"].ToString());
		////Log::Trace("", __FUNCTION__, "f_cm_002021_snd>tpssm01["CAST_LOT_NO"] = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
		////Log::Trace("", __FUNCTION__, "f_cm_002021_snd>tpssm01["FACTORY_DIV"] = [{0}]", tpssm01["FACTORY_DIV"].ToString());

		/* 检查输入参数合法性 */
		if(v_proc_div == "0")
		{
			strcpy(s.sysmsg, "操作区分不能为空!");
			throw CApplicationException(-1, s.msg,  log.Location );	
		} 
	
		if(tpssm01["PONO"].ToString() == " " && v_proc_div!= "2")
		{
			strcpy(s.sysmsg, "pono不能为空!");
			throw CApplicationException(-1, s.msg,  log.Location );
		}
	    
		if(tpssm01["CAST_LOT_NO"].ToString() == " " && v_proc_div == "2")
		{
			strcpy(s.sysmsg, "cast_lot_no is null.");
			throw CApplicationException(-1, s.msg,  log.Location );
		}	

		/* 查询pono */
		sqlstr = "tpssm01.Query()";
		ret = tpssm01.QueryCount("FACTORY_DIV,PONO");
		////Log::Trace("", __FUNCTION__, "tpssm01.QueryCount = [{0}]", ret);
		if (ret == 1)
		{
			tpssm01.Query("FACTORY_DIV,PONO");
			tpssm01.TrimOrBlank();
		}
	
		//	//读取炉次信息
		//switch(conn->DatabaseKind)
		//{
		//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//	case DB_KIND_ORACLE:	    // Oracle 数据库
		//	default:
		//		sqlstr = CString(" SELECT * FROM TPSSM01 "
		//					   	 " WHERE PONO = @pono ");
		//		break;
		//}
		//cmd_tpssm01_inq.SetCommandText( sqlstr );
		//cmd_tpssm01_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
		//cmd_tpssm01_inq.ExecuteReader();
		//if (cmd_tpssm01_inq.Read())
		//{
		//	
		//	cmd_tpssm01_inq.Fetch(tpssm01);
		//}
		tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
		tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

		sqlstr = "tpssm02.Query()";
		ret = tpssm02.QueryCount("FACTORY_DIV,CAST_LOT_NO");
		////Log::Trace("", __FUNCTION__, "tpssm02.QueryCount = [{0}]", ret);
		if (ret == 1)
		{
			tpssm02.Query("FACTORY_DIV,CAST_LOT_NO");
			tpssm02.TrimOrBlank();
		}
			
		lpsz_tc_no = "002021";

		if (epex.Initialize(lpsz_tc_no) < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}
	

		if (epex.SetValue(0, tpssm01) < 0)
		{
			sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		////Log::Trace("", __FUNCTION__,  "tpssm02["CAST_LOT_NO"] = [{0}]", tpssm02["CAST_LOT_NO"].ToString());
		////Log::Trace("", __FUNCTION__,  "tpssm02["ST_NO"] = [{0}]", tpssm02["ST_NO"].ToString());
		////Log::Trace("", __FUNCTION__,  "tpssm02["FACTORY_DIV"] = [{0}]", tpssm02["FACTORY_DIV"].ToString());

		if (epex.SetValue(0, tpssm02) < 0)
		{
			sprintf(s.msg, "发送电文失败，原因[%s]", epex.GetMsg());
			throw CApplicationException(-1, s.msg, s.svc_name);
		}
	
		////Log::Trace("", __FUNCTION__, "PROC_DIV = [{0}]", v_proc_div);
		if(epex.SetValue("PROC_DIV", 0, v_proc_div) < 0)
		{
			////Log::Debug("", __FUNCTION__, "SetValue PROC_DIV:{0}", epex.GetMsg());
			sprintf(s.msg,epex.GetMsg()) ;
			sprintf(s.sysmsg, epex.GetMsg()); 
			throw CApplicationException(-1, s.msg, log.Location); 
		}

		////Log::Trace("", __FUNCTION__, "发送H3H421电文开始");
		/*发送电文 */
		if (epex.SendTele() < 0)
		{
			strcpy(s.msg, "电文发送失败。");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////Log::Trace("", __FUNCTION__, "发送H3H421电文结束");

		/* 释放 */
		epex.Uninitialize();


		//发送铸坯命令
		if(v_proc_div == "1")
		{					
			if (!bcls_rec->Tables.Contains("SLABSND"))
			{
				bcls_rec->Tables.Add("SLABSND");
			}
			if (!bcls_rec->Tables["SLABSND"].Columns.Contains("FACTORY_DIV"))
			{
				bcls_rec->Tables["SLABSND"].Columns.Add(DT_STRING, "FACTORY_DIV");
			}
			if (!bcls_rec->Tables["SLABSND"].Columns.Contains("PONO"))
			{
				bcls_rec->Tables["SLABSND"].Columns.Add(DT_STRING, "PONO");
			}

			bcls_rec->Tables["SLABSND"].Rows.Add();
			bcls_rec->Tables["SLABSND"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			bcls_rec->Tables["SLABSND"].Rows[0]["PONO"] = tpssm01["PONO"];

			doFlag = f_cm_002022_snd(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
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

	cmd_tpssm01_inq.Close();
	

	return doFlag;
}


