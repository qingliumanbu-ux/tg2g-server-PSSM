/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-06-26
Description: PONO收回
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



#if defined _SYS_MES

#endif

int f_pmom_status_upd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#if defined _SYS_MMS
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 制造命令回收
/// <para>
/// 1.校验是否未排入出钢计划
/// 2.将PONO置为命令接受状态
/// 3.调用PM函数
/// 4.下达PES命令删除电文
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)         </para>
/// <para>主调用函数：前台PSSM01画面F8 PONO收回调用 </para>
/// </summary>
/// <param name="pono">制造命令   </param>
/// <returns>炉次制造命令</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm06_lrcd)
//-EP_SYSTEM_HEAD_END
int f_pssm06_lrcd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret;
	CString temp_cast_lot_no = "";

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息
	CString sqlstr;
	CDecimal    cc_seq = 0;                         /* 连铸顺序号 */
	CDecimal    dummy = 0;
	CString   v_cast_lot_no = "";
	CString v_restrand_flg = ""; //重引锭标记
	int v_lack_per = 0;
	int v_cast_lot_sum = 0;
	CString factory_div = "";
	CString pono = "";
	CString cc_mach_no = "";

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");

#if defined _SYS_MES
	CModel tpssm10("TPSSM10");
#endif
	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_sql(conn); //与DB 建立连接。
	CDbCommand cmd_del(conn); //与DB 建立连接。
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_sql_count(conn);
	//调用生产接口
	EIClass inBlock;
	EIClass inBlock3; //调用炼钢履历跟踪
	EIClass outBlock;

	//调用电文接口
	EIClass inBlock1;

	try
	{
		//声明调用生产的接口参数
		inBlock.Tables[0].set_TableName("PSSMCHS");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");

		//声明调用电文的接口参数
		inBlock1.Tables[0].set_TableName("PONOSEND");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "MARKS1");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");

		//调用炼钢履历跟踪
		inBlock3.Tables[0].set_TableName("TRACE");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		/*获取前台输入数据*/
		factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString();/*炼钢区分*/
		cc_mach_no = bcls_rec->Tables[1].Rows[0]["CC_MACH_NO"].ToString();//连铸机号

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, "rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//打印传入参数
			////Log::Trace("", __FUNCTION__, "pssm01f8_rcd>tpssm01["FACTORY_DIV"] = [{0}]", tpssm01["FACTORY_DIV"].ToString());
			////Log::Trace("", __FUNCTION__, "pssm01f8_rcd>tpssm01["PONO"] = [{0}]", tpssm01["PONO"].ToString());
			////Log::Trace("", __FUNCTION__, "pssm01f8_rcd>tpssm01["CAST_LOT_NO"] = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
			////Log::Trace("", __FUNCTION__, "pssm01f8_rcd>tpssm01["PLAN_DATE"] = [{0}]", tpssm01["PLAN_DATE"].ToString());

			tpssm01["FACTORY_DIV"] = factory_div;
			tpssm01["CC_MACH_NO"] = cc_mach_no;

			if (tpssm01["CAST_LOT_NO"].ToString() == temp_cast_lot_no)//同CAST-LOT避免重复下达
			{
				continue;
			}

			////Log::Trace("", __FUNCTION__, "tpssm01["PONO_STATUS"] = [{0}]", tpssm01["PONO_STATUS"].ToDecimal());
			//HYF 20130401 之前是校验>15的状态，16是PES命令接收状态，18才是排入计划
			if (tpssm01["PONO_STATUS"].ToDecimal() >= 18)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSCMS0000055")/*制造命令号[{0}]已排入出钢计划。*/, arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if ((tpssm01["SLAB_DEST"].ToString() == " ") && (tpssm01["HOT_CHARGE_FLAG"].ToString() == "1" || tpssm01["HOT_CHARGE_FLAG"].ToString() == "2"))
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]只能删除，不能收回。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//sqlstr = "SELECT CAST_LOT_SUM FROM TPSSM02 WHERE CAST_LOT_NO = @tpssm01.CAST_LOT_NO ";

			//cmd_sql_count.SetCommandText(sqlstr);
			//cmd_sql_count.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			//cmd_sql_count.ExecuteReader();

			//if (cmd_sql_count.Read())
			//{
			//	tpssm01["CAST_LOT_SUM"] = cmd_sql_count.GetInt32(1);
			//}
			//cmd_sql_count.Close();

			v_cast_lot_sum = 0;
			sqlstr = "SELECT COUNT(1) FROM TPSSM01 WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV AND CAST_LOT_NO = @tpssm01.CAST_LOT_NO AND PONO_STATUS >= 18 ";

			cmd_sql_count.SetCommandText(sqlstr);
			cmd_sql_count.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_sql_count.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_sql_count.ExecuteReader();

			if (cmd_sql_count.Read())
			{
				v_cast_lot_sum = cmd_sql_count.GetInt32(1);
			}
			cmd_sql_count.Close();

			if (v_cast_lot_sum > 0)
			{
				CFormattable arguments[] = { tpssm01["CAST_LOT_NO"].ToString(), v_cast_lot_sum }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "浇次[{0}]内已有{1}炉排入计划，不能LOT收回。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT * FROM TPSSM01 "
					" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
					" AND    CAST_LOT_NO = @tpssm01.CAST_LOT_NO "
					" AND    PONO_STATUS = 16 "
					" ORDER  BY PLAN_DATE,CC_SEQ   ");
				break;
			}

			////Log::Trace("", __FUNCTION__, "tpssm01["PONO"] = [{0}]", tpssm01["PONO"].ToString());

			cmd_sql.SetCommandText(sqlstr);// 设置执行的SQL语句
			cmd_sql.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_sql.Parameters.Set("tpssm01.CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_sql.ExecuteReader();

			while (cmd_sql.Read())
			{
				cmd_sql.Fetch(tpssm01);
				tpssm01.TrimOrBlank();

				if (tpssm01["RES_CODE"].ToString() == "R")
				{
					CFormattable arguments[] = { tpssm01["CAST_LOT_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "再排浇次号[{0}]不允许收回，请确认", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//更新修改者，修改时间
				tpssm01["REC_REVISOR"] = CString(s.userid);
				tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

				tpssm01["PONO_STATUS"] = 11; //命令接收
				tpssm01["CC_SEQ"] = 0;
				tpssm01["PLAN_DATE"] = " ";
				tpssm01["CC_MACH_NO"] = " ";
				tpssm01["SHIFT_NO"] = " ";
				tpssm01["SHIFT_GROUP"] = " ";

				v_update = "REC_REVISOR,REC_REVISE_TIME,PONO_STATUS,CC_SEQ,PLAN_DATE,CC_MACH_NO,SHIFT_NO,SHIFT_GROUP";
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				if (tpssm01.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}


				//根据去向调用不同的函数
				//switch(conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//case DB_KIND_ORACLE:	    // Oracle 数据库
				//default:

				//	sqlstr = CString(" SELECT MAT_DESTION "
				//		" FROM TPSSM01 "
				//		" WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
				//		" AND PONO = @tpssm01.PONO");
				//	break;
				//}
				//cmd_tpssm01_inq.SetCommandText(sqlstr);
				//cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				//cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
				//cmd_tpssm01_inq.ExecuteReader();

				//if(cmd_tpssm01_inq.Read())
				//{
				//	tpssm01.MAT_DESTION = cmd_tpssm01_inq.GetString(1).TrimOrBlank();
				//}
				//cmd_tpssm01_inq.Close();

				//////Log::Trace("", __FUNCTION__, "tpssm01.MAT_DESTION	= [{0}]", tpssm01.MAT_DESTION);				

				//调用生产函数
				inBlock.Tables[0].Rows.Clear();
				inBlock.Tables[0].Rows.Add();
				inBlock.Tables[0].Rows[0]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				inBlock.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];

				doFlag = f_pmom_status_upd2(&inBlock, &outBlock, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

#ifdef _SYS_MMS
				//调用计划下发电文
				inBlock1.Tables[0].Rows.Clear();
				inBlock1.Tables[0].Rows.Add();
				inBlock1.Tables[0].Rows[0]["MARKS1"] = 3;
				inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
				//增加主工序代码 HYF 20130422
				inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];

				ret = f_cm_002021_snd(&inBlock1, &outBlock, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
#endif

#ifdef _SYS_MES
				//判断该PONO是否存在
				dummy = 0;
				tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm10["PONO"] = tpssm01["PONO"];
				dummy = tpssm10.QueryCount("FACTORY_DIV,PONO");
				if (dummy <= 0)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令号[{0}]不存在", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				////Log::Trace("", __FUNCTION__, "dummy=[{0}]", dummy.ToInt32());
				//查询指定PONO信息

				tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				tpssm10["PONO"] = tpssm01["PONO"];
				tpssm10.Query();
				tpssm10.TrimOrBlank();

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = " SELECT CAST_LOT_NO FROM TPSSM10 "
						"  WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
						"    AND PONO		 = @tpssm01.PONO ";
					break;
				}

				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm10_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
				cmd_tpssm10_inq.ExecuteReader();
				if (cmd_tpssm10_inq.Read())
				{
					v_cast_lot_no = cmd_tpssm10_inq.GetString(1).TrimOrBlank();
				}
				cmd_tpssm10_inq.Close();


				//判断炉次状态
				if (tpssm10["PONO_STATUS"].ToDecimal() > 16)
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令号[{0}]已排入出钢计划，不能删除", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//如果被删除的炉次有"T"标记, 将该标记传给下一炉
				if (tpssm10["RESTRAND_FLG"].ToString() == "T")
				{

					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:
						sqlstr = " UPDATE TPSSM10 "
							"   SET RESTRAND_FLG = 'T' "
							" WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
							"   AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
							"   AND CC_SEQ      = @tpssm10.CC_SEQ + 1 ";
						break;
					}
					cmd_tpssm10_upd.SetCommandText(sqlstr);
					cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
					cmd_tpssm10_upd.ExecuteNonQuery();
				}

				////Log::Trace("", __FUNCTION__, "删除命令一览调整表");

				sqlstr = "tpssm10.Delete(FACTORY_DIV,PONO)";
				tpssm10.Delete("FACTORY_DIV,PONO");
				////Log::Trace("", __FUNCTION__, "后序的炉次浇注顺序号向上移");


				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " UPDATE TPSSM10 "
						"   SET CC_SEQ = CC_SEQ - 1 "
						" WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
						"  AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
						"  AND CC_SEQ		> @tpssm10.CC_SEQ "
						"  AND CC_SEQ		< 900 ";
					break;
				}
				cmd_tpssm10_upd.SetCommandText(sqlstr);
				cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
				cmd_tpssm10_upd.ExecuteNonQuery();

				////Log::Trace("", __FUNCTION__, "★★★★★ 炉次删除完后，统计该pono的cast_lot_no在10表内的所有数量，修改该10表该cast_lot_no下所有pono的cast_lot_sum ★★★★★");

				/**********2009-2-5 11:18 start*************/
				////Log::Trace("", __FUNCTION__, "tpssm01["FACTORY_DIV"] =[{0}]", tpssm01["FACTORY_DIV"].ToString());
				////Log::Trace("", __FUNCTION__, "v_cast_lot_no=[{0}]", v_cast_lot_no);


				tpssm10["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssm10["CAST_LOT_NO"] = v_cast_lot_no;
				dummy = tpssm10.QueryCount("FACTORY_DIV,CAST_LOT_NO");
				tpssm10["CAST_LOT_SUM"] = dummy;
				tpssm10["CAST_LOT_NO"] = v_cast_lot_no;
				tpssm10.Update("FACTORY_DIV,CAST_LOT_NO");
#endif

				//炼钢履历跟踪
				CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
				row3["EVENT_ID"] = "C2"; //LOT收回
				row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row3["PONO"] = tpssm01["PONO"];
				row3["PONO_STATUS"] = tpssm01["PONO_STATUS"];

			}//while			
			cmd_sql.Close();

			temp_cast_lot_no = tpssm01["CAST_LOT_NO"];

			//刷新当日内的LOT顺序号
			//借用LACK_PER为计划内浇次顺序号
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT DISTINCT T.CAST_LOT_NO, T.RESTRAND_FLG, T.FACTORY_DIV "
					" FROM TPSSM01 T "
					"  WHERE T.FACTORY_DIV	= @tpssm01.FACTORY_DIV "
					"    AND T.PLAN_DATE	= @tpssm01.PLAN_DATE "
					"    AND T.CC_MACH_NO	= @tpssm01.CC_MACH_NO ";
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
				v_restrand_flg		= cmd_tpssm02_inq.GetString(2);
				tpssm02["FACTORY_DIV"] = cmd_tpssm02_inq.GetString(3);

				if (v_restrand_flg == "T")
				{
					v_lack_per++;
				}

				tpssm02["LACK_PER"] = v_lack_per;
				tpssm02.Update("LACK_PER", "CAST_LOT_NO, FACTORY_DIV");
			}
			cmd_tpssm02_inq.Close();
		}

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock3, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}
	cmd_tpssm01_inq.Close();

	return doFlag;

}
