/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-09-10 17:13:56
Description: 炼钢计划编制（PSSM01）-执行新的浇铸顺重引锭工作
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */
int f_pssm99_trace_l4(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

//-----------------------------------------------------------------------
//功能描述:		执行新的浇铸顺重引锭工作:需修改此pono重引锭标志(=1)
//数据库表:     TPSSM01
//表中文名:     炼钢连铸制造命令炉次表
//主调用函数:   前台 PSSM01画面(制造命令编制)调用
//-----------------------------------------------------------------------
//1.取得pono, 必须是 LOT 中的第1炉
//2.修改TPSSM01表中重引锭
//3.去向不一致不能连浇
//4.厚板向出钢记号不一致不能连浇
//=========================================================================*/

// service入口
BM2F_ENTERACE(pssm01f10_restrd)

int f_pssm01f10_restrd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int rows = 0;
	int ret = 0;
	int dummy = 0;
	CString date_Now = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString sqlstr = "";
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息

	CString billet_type = ""; //钢坯类型 [PSA6]
	CString prev_billet_type = ""; //上一炉次的钢坯类型
	CString	v_restrand_flg = "";
	int	v_lack_per = 0;	

	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");

	EIClass inBlock99; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。

	try
	{
		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rows; i++)
		{
			//----------------------------------------------------------------------
			//获取传入参数
			tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();
			tpssm01["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();

			/* ***** 打印输入参数 ***** */
			////Log::Info("", __FUNCTION__, "pssm01f10_restrd>FACTORY_DIV = [{0}]", tpssm01["FACTORY_DIV"].ToString());
			////Log::Info("", __FUNCTION__, "pssm01f10_restrd>PONO = [{0}]", tpssm01["PONO"].ToString());

			//----------------------------------------------------------------------
			//有效性判断
			ret = tpssm01.QueryCount("FACTORY_DIV,PONO");
			if (ret == 0)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令[{0}]不存在。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			sqlstr = "tpssm01.Query()";
			tpssm01.Query("FACTORY_DIV,PONO");
			tpssm01.TrimOrBlank();

			if (tpssm01["PONO_STATUS"].ToDecimal() != 13)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令[{0}]不在收池状态，不能重引锭。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			////Log::Info("", __FUNCTION__, "tpssm01["RESTRAND_FLG"] = [{0}]", tpssm01["RESTRAND_FLG"].ToString());

			if (tpssm01["RESTRAND_FLG"].ToString().Trim() == "") //重引锭标志
			{
				if (tpssm01["CAST_LOT_DIV_NO"].ToDecimal() != 1)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令[{0}]不是LOT中第1炉，不能重引锭。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//置重引锭标志
				tpssm01["RESTRAND_FLG"] = "T";

				tpssm01["REC_REVISE_TIME"] = date_Now;
				tpssm01["REC_REVISOR"] = s.userid;

				v_update = "REC_REVISE_TIME,REC_REVISOR,RESTRAND_FLG";
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				sqlstr = "tpssm01.Update()";
				if (tpssm01.Update(v_update, v_condi) < 0)
				{
					strcpy(s.msg, "Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else
			{
				//1.本日首炉不能取消"T"标志
				if (tpssm01["CC_SEQ"].ToDecimal() == 1)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令[{0}]是本日第1炉，不能取消重引锭标志。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//2.钢坯类型检验, 方坯类型的LOT和板坯类型的LOT不能合并连浇

				tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

				ret = tpssm02.QueryCount("FACTORY_DIV,CAST_LOT_NO");
				if (ret == 0)
				{
					CFormattable arguments[] = { tpssm02["CAST_LOT_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "浇铸批号[{0}]不存在。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				sqlstr = "tpssm02.Query()";
				tpssm02.Query("FACTORY_DIV,CAST_LOT_NO");
				tpssm02.TrimOrBlank();

				billet_type = tpssm02["BILLET_TYPE"];
				////Log::Info("", __FUNCTION__, "billet_type = [{0}]", billet_type);

				////Log::Info("", __FUNCTION__, "tpssm01["PLAN_DATE"] = [{0}]", tpssm01["PLAN_DATE"].ToString());
				////Log::Info("", __FUNCTION__, "tpssm01["CC_MACH_NO"] = [{0}]", tpssm01["CC_MACH_NO"].ToString());
				////Log::Info("", __FUNCTION__, "tpssm01["CC_SEQ"] = [{0}]", tpssm01["CC_SEQ"].ToDecimal());

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:				// MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = "SELECT BILLET_TYPE "
						"FROM TPSSM02 "
						"WHERE FACTORY_DIV = @FACTORY_DIV "
						"AND CAST_LOT_NO = ( "
						"SELECT CAST_LOT_NO "
						"FROM TPSSM01 "
						"WHERE FACTORY_DIV = @FACTORY_DIV "
						"AND PLAN_DATE = @PLAN_DATE "
						"AND CC_MACH_NO = @CC_MACH_NO "
						"AND CC_SEQ = @CC_SEQ - 1) ";
					break;
				}
				cmd_tpssm02_inq.SetCommandText(sqlstr);
				cmd_tpssm02_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm02_inq.Parameters.Set("PLAN_DATE", tpssm01["PLAN_DATE"].ToString());
				cmd_tpssm02_inq.Parameters.Set("CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
				cmd_tpssm02_inq.Parameters.Set("CC_SEQ", tpssm01["CC_SEQ"].ToDecimal());
				cmd_tpssm02_inq.ExecuteReader();
				if (cmd_tpssm02_inq.Read())
				{
					prev_billet_type = cmd_tpssm02_inq.GetString(1);
					////Log::Info("", __FUNCTION__, "prev_billet_type = [{0}]", prev_billet_type);
				}
				cmd_tpssm02_inq.Close();

				//连铸的钢坯类型不一致
				if (strcmp(billet_type,prev_billet_type) < 0)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令[{0}]浇注钢坯类型与前一炉次的不一致，不能连浇。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//3.出钢记号检验, 去向不同不能合并，厚板向不同出钢记号不允许合并连浇
				//上一炉次的去向及出钢记号

				//CString prev_st_no = ""; //上一炉次的出钢记号
				//CString prev_mat_destion = ""; //上一炉次的去向

				//switch (conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:				// MS SQL Server数据库
				//case DB_KIND_ORACLE:	        // Oracle 数据库
				//default:
				//	sqlstr = "SELECT ST_NO, MAT_DESTION "
				//		   "FROM TPSSM01 "
				//		   "WHERE FACTORY_DIV = @FACTORY_DIV "
				//		   "AND PLAN_DATE = @PLAN_DATE "
				//		   "AND CC_MACH_NO = @CC_MACH_NO "
				//		   "AND CC_SEQ = @CC_SEQ - 1 ";
				//	break;
				//}
				//cmd_tpssm01_inq.SetCommandText(sqlstr);
				//cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				//cmd_tpssm01_inq.Parameters.Set("PLAN_DATE", tpssm01["PLAN_DATE"].ToString());
				//cmd_tpssm01_inq.Parameters.Set("CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
				//cmd_tpssm01_inq.Parameters.Set("CC_SEQ", tpssm01["CC_SEQ"].ToDecimal());
				//cmd_tpssm01_inq.ExecuteReader();
				//if (cmd_tpssm01_inq.Read())
				//{
				//	prev_st_no = cmd_tpssm01_inq.GetString(1);
				//	prev_mat_destion = cmd_tpssm01_inq.GetString(2);
				//}
				//cmd_tpssm01_inq.Close();

				//if(tpssm01.MAT_DESTION != prev_mat_destion )//去向不一致
				//{
				//	CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "制造命令[{0}]去向与前一炉次的不一致，不能连浇。", arguments, 1); //格式化字符串
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}
				//if (prev_mat_destion == "12" || prev_mat_destion == "18" || tpssm01["ST_NO"].ToString() != prev_st_no)//厚板向但出钢记号不一致
				//{
				//	CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "制造命令[%s]的出钢记号与前一炉次的不一致，不能连浇。", arguments, 1); //格式化字符串
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				//已置重引锭标志,则做标志取消
				tpssm01["RESTRAND_FLG"] = " ";

				tpssm01["REC_REVISE_TIME"] = date_Now;
				tpssm01["REC_REVISOR"] = s.userid;

				v_update = "REC_REVISE_TIME,REC_REVISOR,RESTRAND_FLG";
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				sqlstr = "tpssm01.Update()";
				if (tpssm01.Update(v_update, v_condi) < 0)
				{
					strcpy(s.msg, "Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//炼钢履历跟踪
				CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
				row99["EVENT_ID"] = "1T"; //重引锭
				row99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				row99["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row99["PONO"] = tpssm01["PONO"];
				row99["PONO_STATUS"] = tpssm01["PONO_STATUS"];
			}		
			
			//刷新当日内的LOT顺序号
			//借用LACK_PER为计划内浇次顺序号
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT T.CAST_LOT_NO, T.RESTRAND_FLG, T.PLAN_DATE, T.CC_MACH_NO, T.CC_SEQ "
					" FROM TPSSM01 T "
					"  WHERE FACTORY_DIV	= @tpssm01.FACTORY_DIV "
					"    AND PLAN_DATE	= @tpssm01.PLAN_DATE "
					"    AND CC_MACH_NO	= @tpssm01.CC_MACH_NO "
					" ORDER BY T.PLAN_DATE, T.CC_MACH_NO, T.CC_SEQ, T.CAST_LOT_NO ASC ";
				break;
			}
	
			cmd_tpssm02_inq.SetCommandText(sqlstr);
			cmd_tpssm02_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm02_inq.Parameters.Set("tpssm01.PLAN_DATE", tpssm01["PLAN_DATE"].ToString());
			cmd_tpssm02_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
			//cc_seq = cmd_tpssm02_inq.ExecuteScalar();
			cmd_tpssm02_inq.ExecuteReader();
			
			v_lack_per = 0;
			while (cmd_tpssm02_inq.Read())
			{
				tpssm02["CAST_LOT_NO"] = cmd_tpssm02_inq.GetString(1);
				v_restrand_flg = cmd_tpssm02_inq.GetString(2);
				
				if(v_restrand_flg == "T")
				{
					v_lack_per ++;
				}
				
				tpssm02["LACK_PER"] = v_lack_per;
				tpssm02.Update("LACK_PER","CAST_LOT_NO");
			}
			cmd_tpssm02_inq.Close();	
		}//for end

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace_l4(&inBlock99, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
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
