/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-07-08
Description: 制造命令更换铸机
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件






int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 1.将制造命令下的炉次向前进一位
/// 2.在新的铸机号末尾添加制造命令
/// <para>
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数： 前台PSSM01画面F6 移动调用     </para>
/// </summary>
/// <param name="pono">制造命令   </param>
/// <returns>顺序调整后的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm10cllf6_change)
//-EP_SYSTEM_HEAD_END
int f_pssm10cllf6_change(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows, j, k, m;
	int ret = 0;

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息。
	CString sqlstr = "";
	CString plan_date = "";
	CString CAST_LOT_NO = "";
	int CAST_LOT_DIV_NO = 0;
	int v_lack_per = 0;
	int cc_seq = 0;
	CString v_restrand_flg = "";
	CString cast_no_name[100];
	CString castname = "";
	CString pono = "";
	CString factory_div = "";
	CString cc_mach_no = "";
	int slab_width = 0;
	int  check_flag = 0;

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CModel tpssmd9("TPSSMD9");

	EIClass inBlock3; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssmd9_inq(conn);
	CDbCommand cmd_tpssm01_upd(conn);
	CDbCommand cmd_tpssm01_upd2(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);

	try
	{
		//调用炼钢履历跟踪
		inBlock3.Tables[0].set_TableName("TRACE");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		//获取前台传入参数
		factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		cc_mach_no = bcls_rec->Tables[1].Rows[0]["CC_MACH_NO"].ToString().TrimOrBlank();

		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Trace("", __FUNCTION__, " rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().TrimOrBlank();
			Log::Info("", __FUNCTION__, "FACTORY_DIV = [{0}]", factory_div);
			Log::Info("", __FUNCTION__, "CC_MACH_NO = [{0}]", cc_mach_no);
			Log::Info("", __FUNCTION__, "PONO = [{0}]", pono);

			//找到tpssm03的SLAB_THICK,SLAB_WIDTH,BILLET_TYPE
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString("SELECT DISTINCT SLAB_THICK,SLAB_WIDTH,BILLET_TYPE FROM TPSSM03 WHERE FACTORY_DIV = @FACTORY_DIV "
					" AND PONO = @PONO ");
				break;
			}

			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("FACTORY_DIV", factory_div);
			cmd_tpssm03_inq.Parameters.Set("PONO", pono);
			cmd_tpssm03_inq.ExecuteReader();

			while (cmd_tpssm03_inq.Read())
			{
				ret = 0;
				tpssmd9["FACTORY_DIV"] = factory_div;
				tpssmd9["CC_MACH_NO"] = cc_mach_no;
				tpssmd9["CAST_THICK"] = cmd_tpssm03_inq.GetDecimal(1);
				slab_width = cmd_tpssm03_inq.GetInt32(2);
				tpssmd9["BILLET_TYPE"] = cmd_tpssm03_inq.GetString(3);

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = CString("SELECT COUNT(*) FROM TPSSMD9 WHERE FACTORY_DIV = @FACTORY_DIV "
						" AND CC_MACH_NO = @CC_MACH_NO "
						" AND CAST_THICK = @CAST_THICK "
						" AND CAST_WIDTH_MIN <= @slab_width "
						" AND CAST_WIDTH_MAX >= @slab_width "
						" AND BILLET_TYPE = @BILLET_TYPE ");
					break;
				}
				cmd_tpssmd9_inq.SetCommandText(sqlstr);
				cmd_tpssmd9_inq.Parameters.Set("FACTORY_DIV", tpssmd9["FACTORY_DIV"].ToString());
				cmd_tpssmd9_inq.Parameters.Set("CC_MACH_NO", tpssmd9["CC_MACH_NO"].ToString());
				cmd_tpssmd9_inq.Parameters.Set("CAST_THICK", tpssmd9["CAST_THICK"].ToDecimal());
				cmd_tpssmd9_inq.Parameters.Set("slab_width", slab_width);
				cmd_tpssmd9_inq.Parameters.Set("BILLET_TYPE", tpssmd9["BILLET_TYPE"].ToString());
				cmd_tpssmd9_inq.ExecuteReader();
				if (cmd_tpssmd9_inq.Read())
				{
					ret = cmd_tpssmd9_inq.GetInt16(1);
				}
				else ret = 0;
				cmd_tpssmd9_inq.Close();

				if (ret == 0) check_flag = 1;
			}

			Log::Trace("", __FUNCTION__, " check_flag = [{0}]", check_flag);
			if (check_flag == 1)
			{
				CFormattable arguments[] = { pono };
				CMessageFormat::Format(s.msg, "更改的铸机不生产[{0}]该类型的板坯。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			cmd_tpssm03_inq.Close();
		}


		if (check_flag == 0)
		{
			//获得输入参数
			rows = bcls_rec->Tables[0].Rows.get_Count();
			Log::Trace("", __FUNCTION__, " rows = [{0}]", rows);
			for (i = 0; i < rows; i++)
			{
				// 获取前台传入参数
				tpssm01["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"];
				tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];
				//tpssm01["CC_MACH_NO"] = cc_mach_no;

				tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"];
				tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];
				//tpssm10["CC_MACH_NO"] = cc_mach_no;

				//打印输入参数
				Log::Info("", __FUNCTION__, "PONO=[{0}]", tpssm01["PONO"].ToString());
				Log::Info("", __FUNCTION__, "FACTORY_DIV=[{0}]", tpssm01["FACTORY_DIV"].ToString());
				Log::Trace("", __FUNCTION__, " 更换铸机号CC_MACH_NO = [{0}]", cc_mach_no);

				tpssm01.Query("PONO,FACTORY_DIV");
				tpssm01.TrimOrBlank();
				Log::Trace("", __FUNCTION__, " 当前铸机号CC_MACH_NO = [{0}]", tpssm01["CC_MACH_NO"].ToString());


				tpssm10.Query("PONO,FACTORY_DIV");
				tpssm10.TrimOrBlank();

				//Log::Trace("", __FUNCTION__, "tpssm01["PONO_STATUS"] =[{0}]", tpssm01["PONO_STATUS"].ToDecimal());
				//Log::Trace("", __FUNCTION__, "tpssm10["PONO_STATUS"] =[{0}]", tpssm10["PONO_STATUS"].ToDecimal());

				//校验状态
				if (tpssm01["PONO_STATUS"].ToDecimal() != 16)
				{
					strcpy(s.msg, "tpssm01该制造命令状态不为命令接收状态，不允许更换铸机");/*该制造命令状态不为编制计划状态，不允许排序。*/
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (tpssm10["PONO_STATUS"].ToDecimal() != 16)
				{
					strcpy(s.msg, "tpssm10该制造命令状态不为命令接收状态，不允许更换铸机");/*该制造命令状态不为编制计划状态，不允许排序。*/
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//修改后续炉次的顺序号(TPSSM10)
				if (tpssm10["CC_SEQ"].ToDecimal() > 0)
				{
					//修改后续炉次的顺序号
					sqlstr = " UPDATE TPSSM10 "
						" SET CC_SEQ            = CC_SEQ - 1 "
						" WHERE FACTORY_DIV = TRIM(@tpssm01.FACTORY_DIV) "
						"   AND CC_MACH_NO        = TRIM(@tpssm10.CC_MACH_NO) "
						"   AND CC_SEQ			   > @tpssm10.CC_SEQ "
						"   AND CC_SEQ            < 900 ";//900以后的都是封锁的计划

					cmd_tpssm10_upd.SetCommandText(sqlstr);
					cmd_tpssm10_upd.Parameters.Set("tpssm01.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
					cmd_tpssm10_upd.ExecuteNonQuery();
					cmd_tpssm10_upd.Close();

				}

				//修改后续炉次的顺序号(TPSSM01)
				if (tpssm01["CC_SEQ"].ToDecimal() > 0)
				{
					//修改后续炉次的顺序号
					sqlstr = " UPDATE TPSSM01 "
						" SET CC_SEQ            = CC_SEQ - 1 "
						" WHERE FACTORY_DIV = TRIM(@tpssm01.FACTORY_DIV) "
						"   AND CC_MACH_NO        = TRIM(@tpssm01.CC_MACH_NO) "
						"   AND CC_SEQ			   > @tpssm01.CC_SEQ "
						"   AND CC_SEQ            < 900 ";//900以后的都是封锁的计划

					cmd_tpssm10_upd.SetCommandText(sqlstr);
					cmd_tpssm10_upd.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm01.CC_SEQ", tpssm01["CC_SEQ"].ToDecimal());
					cmd_tpssm10_upd.ExecuteNonQuery();
					cmd_tpssm10_upd.Close();

				}

				//查询更换铸机的当前最大连铸顺序号
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = CString(" SELECT MAX(CC_SEQ) FROM TPSSM01 "
						" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
						" AND    CC_MACH_NO  = @tpssm01.CC_MACH_NO "
						" AND    PONO_STATUS > 13 AND PONO_STATUS < 83"
						" AND    CC_SEQ != 999 ");
					break;
				}

				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm10_inq.Parameters.Set("tpssm01.CC_MACH_NO", cc_mach_no);
				cmd_tpssm10_inq.ExecuteReader();

				if (cmd_tpssm10_inq.Read())
				{
					tpssm10["CC_SEQ"] = cmd_tpssm10_inq.GetInt16(1);
				}
				else
				{
					tpssm10["CC_SEQ"] = 0;
				}

				cmd_tpssm10_inq.Close();
				Log::Trace("", __FUNCTION__, "更换铸机的 CC_SEQmax=[{0}]", tpssm10["CC_SEQ"].ToDecimal());


				//更新修改者，修改时间
				tpssm01["REC_REVISOR"] = CString(s.userid); //构造函数初始化
				tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tpssm01["CC_SEQ"] = tpssm10["CC_SEQ"].ToDecimal() + 1;
				tpssm01["CC_MACH_NO"] = cc_mach_no;

				tpssm10["REC_REVISOR"] = CString(s.userid); //构造函数初始化
				tpssm10["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tpssm10["CC_SEQ"] = tpssm01["CC_SEQ"];
				tpssm10["CC_MACH_NO"] = cc_mach_no;

				v_update = "REC_REVISOR,REC_REVISE_TIME,CC_MACH_NO,CC_SEQ";//修改字段信息。
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				if (tpssm01.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm01 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (tpssm10.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update tpssm10 failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//炼钢履历跟踪
				CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
				row3["EVENT_ID"] = "A3"; //计划调整
				row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row3["PONO"] = tpssm01["PONO"];
			}
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
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	/*cmd_tpssm01_inq.Close();*/

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}

