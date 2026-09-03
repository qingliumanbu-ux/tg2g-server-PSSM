/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-07-08
Description: 制造命令移动
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 1.取计划下发的最大CAST_LOT_DIV_NO,将所有编入计划的CAST_LOT_DIV_NO重置
/// 2.必须是计划编制状态的Pono才可以调整顺序
/// <para>
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数： 前台PSSM01画面F6 移动调用     </para>
/// </summary>
/// <param name="pono">制造命令   </param>
/// <returns>顺序调整后的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm10cllcf3_move)
//-EP_SYSTEM_HEAD_END
int f_pssm10cllcf3_move(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm10("TPSSM10");

	EIClass inBlock3; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm01_upd(conn);
	CDbCommand cmd_tpssm01_upd2(conn);
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
		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		tpssm01["CC_MACH_NO"] = bcls_rec->Tables[1].Rows[0]["CC_MACH_NO"].ToString().TrimOrBlank();

		//找到计划下发的CC_SEQ最大值
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = CString(
				" SELECT MAX(CC_SEQ) FROM TPSSM01 "
				" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
				" AND    CC_MACH_NO  = @tpssm01.CC_MACH_NO "
				" AND    PONO_STATUS >= 18 AND PONO_STATUS < 83"
				" AND    CC_SEQ != 999 "
			);
			break;
		}

		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
		cmd_tpssm01_inq.ExecuteReader();

		if (cmd_tpssm01_inq.Read())
		{
			tpssm01["CC_SEQ"] = cmd_tpssm01_inq.GetInt16(1);
		}
		else
		{
			tpssm01["CC_SEQ"] = 0;
		}
		//tpssm01["CC_SEQ"] = 0;
		cmd_tpssm01_inq.Close();

		cc_seq = tpssm01["CC_SEQ"].ToDecimal().ToInt32();
		Log::Trace("", __FUNCTION__, " cc_seq = [{0}]", cc_seq);

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Trace("", __FUNCTION__, " rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm01["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
			tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();

			tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
			tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();

			sqlstr = "tpssm01.Query()";
			tpssm01.Query("PONO,FACTORY_DIV");
			tpssm01.TrimOrBlank();

			tpssm10.Query("PONO,FACTORY_DIV");
			tpssm10.TrimOrBlank();

			Log::Info("", __FUNCTION__, "PONO=[{0}]PONO_STATUS[{1}]", tpssm10["PONO"].ToString(), tpssm10["PONO_STATUS"].ToDecimal());

			if (tpssm10["PONO_STATUS"].ToDecimal() != 16)
			{
				continue;
				//strcpy(s.msg, "tpssm10该制造命令状态不为命令接收状态，不允许排序");/*该制造命令状态不为编制计划状态，不允许排序。*/
				//throw CApplicationException(-1, s.msg, log.Location);
			}

			if (tpssm10["PONO_STATUS"].ToDecimal() == 15) //命令挂起
			{
				tpssm01["CC_SEQ"] = 999;
				tpssm10["CC_SEQ"] = 999;
			}
			else if (tpssm10["PONO_STATUS"].ToDecimal() == 83)  //浇铸终了
			{
				tpssm01["CC_SEQ"] = 0;
				tpssm10["CC_SEQ"] = 0;
			}
			else if (tpssm10["PONO_STATUS"].ToDecimal() == 91)  //炉次确定
			{
				tpssm01["CC_SEQ"] = 0;
				tpssm10["CC_SEQ"] = 0;
			}
			else
			{
				cc_seq++;
				if (cc_seq > 999) cc_seq = 999;
				tpssm01["CC_SEQ"] = cc_seq; //按照前台排序更改tpssm01表中的cc_seq顺序号
				tpssm10["CC_SEQ"] = tpssm01["CC_SEQ"];
			}

			//更新修改者，修改时间
			tpssm01["REC_REVISOR"] = CString(s.userid); //构造函数初始化
			tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tpssm10["REC_REVISOR"] = CString(s.userid); //构造函数初始化
			tpssm10["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");


			/*Log::Trace("", __FUNCTION__, " tpssm01["CC_SEQ"] = [{0}]", tpssm01["CC_SEQ"].ToDecimal());*/

			/*if (tpssm01["CC_SEQ"].ToDecimal() == 1)
			{
				tpssm01["RESTRAND_FLG"] = "T";
				tpssm10["RESTRAND_FLG"] = "T";
			}
			else
			{
				tpssm01["RESTRAND_FLG"] = " ";
				tpssm10["RESTRAND_FLG"] = " ";
			}*/

			v_update = "REC_REVISOR,REC_REVISE_TIME,CC_SEQ,RESTRAND_FLG";//修改字段信息。
			v_condi = "FACTORY_DIV,PONO"; //查询条件
			tpssm01.Update(v_update, v_condi);
			tpssm10.Update(v_update, v_condi);
			//if (tpssm01.Update(v_update, v_condi) != true)
			//{
			//	strcpy(s.msg, "Update 01 failed.");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			//if (tpssm10.Update(v_update, v_condi) != true)
			//{
			//strcpy(s.msg, "Update 10 failed.");
			//throw CApplicationException(-1, s.msg, log.Location);
			//}

			/*sqlstr = " UPDATE TPSSM10 "
				"  SET  CC_SEQ = @tpssm10.CC_SEQ, "
				"  REC_REVISOR = @tpssm10.REC_REVISOR, "
				"  REC_REVISE_TIME = @tpssm10.REC_REVISE_TIME "
				" WHERE PONO = @tpssm10.PONO "
				"   AND FACTORY_DIV = @tpssm10.FACTORY_DIV ";

			cmd_tpssm10_upd.SetCommandText(sqlstr);
			cmd_tpssm10_upd.Parameters.Clear();
			cmd_tpssm10_upd.Parameters.Set("tpssm10.REC_REVISOR", tpssm10["REC_REVISOR"].ToString());
			cmd_tpssm10_upd.Parameters.Set("tpssm10.REC_REVISE_TIME", tpssm10["REC_REVISE_TIME"].ToString());
			cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
			cmd_tpssm10_upd.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
			cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
			cmd_tpssm10_upd.ExecuteNonQuery();
			cmd_tpssm10_upd.Close();*/

			//炼钢履历跟踪
			CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
			row3["EVENT_ID"] = "1U"; //铸顺调整
			row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			row3["PONO"] = tpssm01["PONO"];
		}

		//有必要吗？看看日后要不要去掉; noted by YYP 2021/1/14
		sqlstr = " UPDATE TPSSM10 "
			"  SET  CC_SEQ = 0 "
			"  WHERE PONO_STATUS IN ( 83,91 ) ";

		cmd_tpssm10_upd.SetCommandText(sqlstr);
		cmd_tpssm10_upd.ExecuteNonQuery();
		cmd_tpssm10_upd.Close();

		sqlstr = " UPDATE TPSSM01 "
			"  SET  CC_SEQ = 0 "
			"  WHERE PONO_STATUS IN ( 83,91 ) ";

		cmd_tpssm01_upd.SetCommandText(sqlstr);
		cmd_tpssm01_upd.ExecuteNonQuery();
		cmd_tpssm01_upd.Close();

		//-----------------------------------------------
		//炼钢履历跟踪
		//ret = f_pssm99_trace(&inBlock3, bcls_ret, conn);
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

