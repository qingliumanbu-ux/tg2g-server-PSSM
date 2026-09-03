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
#include "tpssm01.h"
#include "tpssm02.h"

/* ***** 静态函数申明 ***** */
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪
int f_mm0011(CString SeqName, CDecimal SeqLen, CString &SeqNo, CDbConnection * conn);
int f_get_cast_no(EIClass * bcls_rec, CDbConnection * conn); //生成cast_no

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
BM2F_ENTERACE(pssm01cf5_restrd)

int f_pssm01cf5_restrd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
	CString cc_mach_no_before = ""; //用于比较cc_mach_no是否相同的字段(前一个)
	CString cc_mach_no_after = ""; //用于比较cc_mach_no是否相同的字段(后一个)
	CString v_cast_no_seq = "";//用于拼接cast_no的浇次顺序号（7位）
	CString v_cast_no = ""; //拼接完成的cast_no号
	CString v_cast_lot_no_before = ""; //用于比较cast_lot_no是否相同的字段(前一个)
	CString v_cast_lot_no_after = "";//用于比较cast_lot_no是否相同的字段(后一个)
	CDecimal v_cast_lot_div_no;
	CString datetime = "";//得到系统当前时间

	CString v_seq_name = ""; //序号名称
	CDecimal v_seq_length;//序号长度
	CString v_seq_type = ""; //序号类型


	CString billet_type = ""; //钢坯类型 [PSA6]
	CString prev_billet_type = ""; //上一炉次的钢坯类型
	CString	v_restrand_flg = "";
	int	v_lack_per = 0;

	CTPSSM01 tpssm01(conn);
	CTPSSM02 tpssm02(conn);

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
		Log::Trace("", __FUNCTION__, "Rows is {0}", rows);

		/*单行处理和多行处理共同包含的功能*/
		for (int i = 0; i < rows; i++)  //遍历前台传入的每一行数据 
		{
			//----------------------------------------------------------------------
			//获取传入参数
			tpssm01.FACTORY_DIV = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();
			tpssm01.PONO = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();

			/* ***** 打印输入参数 ***** */
			Log::Info("", __FUNCTION__, "pssm01cf5_restrd>FACTORY_DIV = [{0}]", tpssm01.FACTORY_DIV);
			Log::Info("", __FUNCTION__, "pssm01cf5_restrd>PONO = [{0}]", tpssm01.PONO);

			//----------------------------------------------------------------------
			//有效性判断
			ret = tpssm01.QueryCount("FACTORY_DIV,PONO");
			if (ret == 0)
			{
				CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令[{0}]不存在。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			sqlstr = "tpssm01.Query()";
			tpssm01.Query("FACTORY_DIV,PONO");
			tpssm01.TrimOrBlank();

			/*2018-06-20号注释*/
			//if (tpssm01.PONO_STATUS != 13)
			//{
			//	CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "制造命令[{0}]不在收池状态，不能重引锭。", arguments, 1); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			Log::Info("", __FUNCTION__, "tpssm01.RESTRAND_FLG = [{0}]", tpssm01.RESTRAND_FLG);

			if ("" == bcls_rec->Tables[0].Rows[0]["RESTRAND_FLG"].ToString().Trim()) //第一行RESTRAND_FLG为空，则重引锭
			{
				Log::Info("", __FUNCTION__, "Here is 1");
				if (0 == i)  //将第一行重引锭标志置为T
				{
					//置重引锭标志
					tpssm01.RESTRAND_FLG = "T";

					tpssm01.REC_REVISE_TIME = date_Now;
					tpssm01.REC_REVISOR = s.userid;

					v_update = "REC_REVISE_TIME,REC_REVISOR,RESTRAND_FLG";
					v_condi = "FACTORY_DIV,PONO"; //查询条件

					sqlstr = "tpssm01.Update()";
					if (tpssm01.Update(v_update, v_condi) < 0)
					{
						strcpy(s.msg, "Update failed.");
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}
			else  //第一行RESTRAND_FLG不为空，则重引锭取消
			{
				//1.本日首炉不能取消"T"标志
				if (tpssm01.CC_SEQ == 1)
				{
					CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令[{0}]是本日第1炉，不能取消重引锭标志。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//2.钢坯类型检验, 方坯类型的LOT和板坯类型的LOT不能合并连浇

				tpssm02.FACTORY_DIV = tpssm01.FACTORY_DIV;
				tpssm02.CAST_LOT_NO = tpssm01.CAST_LOT_NO;

				ret = tpssm02.QueryCount("FACTORY_DIV,CAST_LOT_NO");
				if (ret == 0)
				{
					CFormattable arguments[] = { tpssm02.CAST_LOT_NO }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "浇铸批号[{0}]不存在。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				sqlstr = "tpssm02.Query()";
				tpssm02.Query("FACTORY_DIV,CAST_LOT_NO");
				tpssm02.TrimOrBlank();

				billet_type = tpssm02.BILLET_TYPE;
				Log::Info("", __FUNCTION__, "billet_type = [{0}]", billet_type);

				Log::Info("", __FUNCTION__, "tpssm01.PLAN_DATE = [{0}]", tpssm01.PLAN_DATE);
				Log::Info("", __FUNCTION__, "tpssm01.CC_MACH_NO = [{0}]", tpssm01.CC_MACH_NO);
				Log::Info("", __FUNCTION__, "tpssm01.CC_SEQ = [{0}]", tpssm01.CC_SEQ);

				/*2018-06-20号注释*/
				//switch (conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:				// MS SQL Server数据库
				//case DB_KIND_ORACLE:	        // Oracle 数据库
				//default:
				//	sqlstr = "SELECT BILLET_TYPE "
				//		"FROM TPSSM02 "
				//		"WHERE FACTORY_DIV = @FACTORY_DIV "
				//		"AND CAST_LOT_NO = ( "
				//		"SELECT CAST_LOT_NO "
				//		"FROM TPSSM01 "
				//		"WHERE FACTORY_DIV = @FACTORY_DIV "
				//		"AND PLAN_DATE = @PLAN_DATE "
				//		"AND CC_MACH_NO = @CC_MACH_NO "
				//		"AND CC_SEQ = @CC_SEQ - 1) ";
				//	break;
				//}
				//cmd_tpssm02_inq.SetCommandText(sqlstr);
				//cmd_tpssm02_inq.Parameters.Set("FACTORY_DIV", tpssm01.FACTORY_DIV);
				//cmd_tpssm02_inq.Parameters.Set("PLAN_DATE", tpssm01.PLAN_DATE);
				//cmd_tpssm02_inq.Parameters.Set("CC_MACH_NO", tpssm01.CC_MACH_NO);
				//cmd_tpssm02_inq.Parameters.Set("CC_SEQ", tpssm01.CC_SEQ);
				//cmd_tpssm02_inq.ExecuteReader();
				//if (cmd_tpssm02_inq.Read())
				//{
				//	prev_billet_type = cmd_tpssm02_inq.GetString(1);
				//	Log::Info("", __FUNCTION__, "prev_billet_type = [{0}]", prev_billet_type);
				//}
				//cmd_tpssm02_inq.Close();


				/*以下为原代码中就注释掉的*/
				////连铸的钢坯类型不一致
				//if (strcmp(billet_type, prev_billet_type) < 0)
				//{
				//	CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "制造命令[{0}]浇注钢坯类型与前一炉次的不一致，不能连浇。", arguments, 1); //格式化字符串
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

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
				//cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01.FACTORY_DIV);
				//cmd_tpssm01_inq.Parameters.Set("PLAN_DATE", tpssm01.PLAN_DATE);
				//cmd_tpssm01_inq.Parameters.Set("CC_MACH_NO", tpssm01.CC_MACH_NO);
				//cmd_tpssm01_inq.Parameters.Set("CC_SEQ", tpssm01.CC_SEQ);
				//cmd_tpssm01_inq.ExecuteReader();
				//if (cmd_tpssm01_inq.Read())
				//{
				//	prev_st_no = cmd_tpssm01_inq.GetString(1);
				//	prev_mat_destion = cmd_tpssm01_inq.GetString(2);
				//}
				//cmd_tpssm01_inq.Close();

				//if(tpssm01.MAT_DESTION != prev_mat_destion )//去向不一致
				//{
				//	CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "制造命令[{0}]去向与前一炉次的不一致，不能连浇。", arguments, 1); //格式化字符串
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}
				//if (prev_mat_destion == "12" || prev_mat_destion == "18" || tpssm01.ST_NO != prev_st_no)//厚板向但出钢记号不一致
				//{
				//	CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
				//	CMessageFormat::Format(s.msg, "制造命令[%s]的出钢记号与前一炉次的不一致，不能连浇。", arguments, 1); //格式化字符串
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				//已置重引锭标志,则做标志取消(只对第一行处理)

				if (0 == i)
				{
					tpssm01.RESTRAND_FLG = " ";

					tpssm01.REC_REVISE_TIME = date_Now;
					tpssm01.REC_REVISOR = s.userid;

					v_update = "REC_REVISE_TIME,REC_REVISOR,RESTRAND_FLG";
					v_condi = "FACTORY_DIV,PONO"; //查询条件

					sqlstr = "tpssm01.Update()";
					if (tpssm01.Update(v_update, v_condi) < 0)
					{
						strcpy(s.msg, "Update failed.");
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				/*2018-06-20号注释*/
				////炼钢履历跟踪
				//CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
				//row99["EVENT_ID"] = "1T"; //重引锭
				//row99["FACTORY_DIV"] = tpssm01.FACTORY_DIV;
				//row99["CAST_LOT_NO"] = tpssm01.CAST_LOT_NO;
				//row99["PONO"] = tpssm01.PONO;
				//row99["PONO_STATUS"] = tpssm01.PONO_STATUS;
			}

			/*2018-06-20号注释*/
			////刷新当日内的LOT顺序号
			////借用LACK_PER为计划内浇次顺序号
			//switch (conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:	        // MS SQL Server数据库
			//case DB_KIND_ORACLE:	        // Oracle 数据库
			//default:
			//	sqlstr = " SELECT T.CAST_LOT_NO, T.RESTRAND_FLG, T.PLAN_DATE, T.CC_MACH_NO, T.CC_SEQ "
			//		" FROM TPSSM01 T "
			//		"  WHERE FACTORY_DIV	= @tpssm01.FACTORY_DIV "
			//		"    AND PLAN_DATE	= @tpssm01.PLAN_DATE "
			//		"    AND CC_MACH_NO	= @tpssm01.CC_MACH_NO "
			//		" ORDER BY T.PLAN_DATE, T.CC_MACH_NO, T.CC_SEQ, T.CAST_LOT_NO ASC ";
			//	break;
			//}

			//cmd_tpssm02_inq.SetCommandText(sqlstr);
			//cmd_tpssm02_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01.FACTORY_DIV);
			//cmd_tpssm02_inq.Parameters.Set("tpssm01.PLAN_DATE", tpssm01.PLAN_DATE);
			//cmd_tpssm02_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01.CC_MACH_NO);
			////cc_seq = cmd_tpssm02_inq.ExecuteScalar();
			//cmd_tpssm02_inq.ExecuteReader();

			//v_lack_per = 0;
			//while (cmd_tpssm02_inq.Read())
			//{
			//	tpssm02.CAST_LOT_NO = cmd_tpssm02_inq.GetString(1);
			//	v_restrand_flg = cmd_tpssm02_inq.GetString(2);

			//	if (v_restrand_flg == "T")
			//	{
			//		v_lack_per++;
			//	}

			//	tpssm02.LACK_PER = v_lack_per;
			//	tpssm02.Update("LACK_PER", "CAST_LOT_NO");
			//}
			//cmd_tpssm02_inq.Close();
		}//for end


		/*多行的独有功能*/
		if (rows > 1)
		{

			tpssm01.PONO = bcls_rec->Tables[0].Rows[0]["PONO"].ToString();
			sqlstr = "tpssm01.Query()";
			tpssm01.Query("PONO");
			tpssm01.TrimOrBlank();

			if ("" == tpssm01.CAST_NO.Trim()) //当cast_no为空，则进行重引锭操作
			{
#pragma region 判断CC_MACH_NO是否相同 
				for (int i = 0; i < rows; i++)  //遍历前台传入的每一行数据 
				{
					cc_mach_no_before = cc_mach_no_after;

					tpssm01.PONO = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
					sqlstr = "tpssm01.Query()";
					tpssm01.Query("PONO");
					tpssm01.TrimOrBlank();

					cc_mach_no_after = tpssm01.CC_MACH_NO;
					if (i > 0)
					{
						if (cc_mach_no_before != cc_mach_no_after)
						{
							CFormattable arguments[] = { tpssm01.PONO }; // 定义参数列表的数组
							CMessageFormat::Format(s.msg, "制造命令[{0}]的CC_MACH_NO号与其它PONO号不同。", arguments, 1); //格式化字符串
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
				}
#pragma endregion	

#pragma region 判断第一行的cast_lot_div_no是否为最小
				for (int i = 0; i < rows; i++)
				{
					tpssm01.PONO = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
					sqlstr = "tpssm01.Query()";
					tpssm01.Query("PONO");
					tpssm01.TrimOrBlank();

					//取第一行的cast_lot_div_no
					if (0 == i)
					{
						v_cast_lot_div_no = tpssm01.CAST_LOT_DIV_NO;

					}
					if (0 != i) //第一行不比较
					{

						if (v_cast_lot_div_no < tpssm01.CAST_LOT_DIV_NO)
						{
							continue;
						}
						else if (v_cast_lot_div_no == tpssm01.CAST_LOT_DIV_NO)
						{
							strcpy(s.sysmsg, "第一行CAST_LOT_DIV_NO有相同的值");
							throw CApplicationException(-1, s.sysmsg, log.Location);
						}
						else
						{
							strcpy(s.sysmsg, "第一行CAST_LOT_DIV_NO不为最小值");
							throw CApplicationException(-1, s.sysmsg, log.Location);
						}
					}
				}
#pragma endregion

#pragma region 调用生成cast_no函数
				doFlag = f_get_cast_no(bcls_rec, conn); //调用函数生成cast_no,并置入每一个pono号中，更新于表
				if (doFlag < 0)
				{
					strcpy(s.sysmsg, "调用f_get_cast_no函数出错");
					throw CApplicationException(-1, s.sysmsg, log.Location);
				}
#pragma endregion	

			}
			else  //当cast_no不为空，则进行重引锭取消操作
			{
#pragma region 将tpssm01表中的cast_no置位空

				for (int i = 0; i < rows; i++)
				{
					tpssm01.REC_REVISE_TIME = date_Now;
					tpssm01.REC_REVISOR = s.userid;
					tpssm01.CAST_NO = " ";
					tpssm01.PONO = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();

					v_update = "REC_REVISE_TIME,REC_REVISOR,CAST_NO"; //更新字段（ "CAST_NO"）
					v_condi = "PONO";  //查询条件字段(PONO号)

					sqlstr = "tpssm01.Update()";
					if (tpssm01.Update(v_update, v_condi) < 0)
					{
						strcpy(s.msg, "Update TPSSM01 failed.");
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
#pragma endregion

#pragma region 将tpssm02表中的cast_no和cc_seq置位空 
				for (int i = 0; i < rows; i++)
				{
					v_cast_lot_no_before = v_cast_lot_no_after;
					tpssm01.PONO = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
					sqlstr = "tpssm01.Query()";
					tpssm01.Query("PONO");
					tpssm01.TrimOrBlank();

					tpssm02.CAST_LOT_NO = tpssm01.CAST_LOT_NO;
					v_cast_lot_no_after = tpssm02.CAST_LOT_NO;

					if (v_cast_lot_no_before == v_cast_lot_no_after)
					{
						continue;
					}

					tpssm02.REC_REVISE_TIME = date_Now;
					tpssm02.REC_REVISOR = s.userid;
					tpssm02.CAST_NO = " ";
					tpssm02.CC_SEQ = 0;
					v_update = "REC_REVISE_TIME,REC_REVISOR,CAST_NO, CC_SEQ"; //更新字段（ "CAST_NO","CC_SEQ"）
					v_condi = "CAST_LOT_NO";  //查询条件字段("CAST_LOT_NO")

					sqlstr = "tpssm02.Update()";
					if (tpssm02.Update(v_update, v_condi) < 0)
					{
						strcpy(s.msg, "Update TPSSM02 failed.");
						throw CApplicationException(-1, s.msg, log.Location);
					}

				}
#pragma endregion
			}

		}

		/*2018-06-20号注释*/
		//-----------------------------------------------
		////炼钢履历跟踪
		//ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

	}

	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员*/, arguments, 1);
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


