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
/// 1.取计划下发的最大CC_SEQ,将所有编入计划的CC_SEQ重置 
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
BM2F_ENTERACE(pssm01f6_move)
//-EP_SYSTEM_HEAD_END
int f_pssm01f6_move(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret = 0;

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
    CString v_condi  = "";  //过滤的字段信息。
	CString sqlstr = "";
	CString plan_date = "";
	int cc_seq = 0;
	int v_lack_per = 0;
	CString v_restrand_flg = "";
	
	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");

	EIClass inBlock3; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);

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

		if (bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString() != "")
		{
			plan_date = bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString().Substring(0, 8);//取出第一行生产日期
		}


		Log::Info("", __FUNCTION__, "FACTORY_DIV = [{0}]", tpssm01["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "CC_MACH_NO = [{0}]", tpssm01["CC_MACH_NO"].ToString());
		Log::Info("", __FUNCTION__, "plan_date = [{0}]", plan_date);

		//借用LACK_PER为计划内浇次顺序号
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT MAX(LACK_PER) FROM TPSSM02 "
				"  WHERE FACTORY_DIV	= @tpssm01.FACTORY_DIV "
				"    AND CAST_LOT_NO	IN (SELECT CAST_LOT_NO "
				"							 FROM TPSSM01 "
				"							 WHERE PLAN_DATE = @tpssm01.PLAN_DATE "
				"							   AND CC_MACH_NO = @tpssm01.CC_MACH_NO) ";
			break;
		}

		cmd_tpssm02_inq.SetCommandText(sqlstr);
		cmd_tpssm02_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm02_inq.Parameters.Set("tpssm01.PLAN_DATE", plan_date);
		cmd_tpssm02_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
		cmd_tpssm02_inq.ExecuteReader();
		if (cmd_tpssm02_inq.Read())
		{
			v_lack_per = cmd_tpssm02_inq.GetInt32(1);
		}
		else
		{
			v_lack_per = 0;
		}
		cmd_tpssm02_inq.Close();

		//找到计划下发的CC_SEQ最大值
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
			sqlstr = CString(" SELECT MAX(CC_SEQ) FROM TPSSM01 "
							 " WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
							 " AND    CC_MACH_NO  = @tpssm01.CC_MACH_NO "
							 " AND    PLAN_DATE =  @tpssm01.PLAN_DATE "
							 " AND    PONO_STATUS > 13 ");
			break;
		}

		cmd_tpssm01_inq.SetCommandText( sqlstr );
		cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("tpssm01.PLAN_DATE", plan_date);
		cmd_tpssm01_inq.ExecuteReader();

		if ( cmd_tpssm01_inq.Read() )
		{
			tpssm01["CC_SEQ"] = cmd_tpssm01_inq.GetInt16(1);
		}
		else
		{
			tpssm01["CC_SEQ"] = 0 ; 		
		}
		
	    cmd_tpssm01_inq.Close();

		cc_seq =tpssm01["CC_SEQ"].ToDecimal().ToInt32();
		////Log::Trace("", __FUNCTION__, " cc_seq = [{0}]", cc_seq);

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, " rows = [{0}]", rows);
		for (i = 0; i < rows;  i++ )
		{
			// 获取前台传入参数
			tpssm01["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"];
			tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];
			
			//打印输入参数
			Log::Info("", __FUNCTION__, "PONO =[{0}]", tpssm01["PONO"].ToString());
			Log::Info("", __FUNCTION__, "FACTORY_DIV =[{0}]", tpssm01["FACTORY_DIV"].ToString());

			sqlstr = "tpssm01.Query()";
			tpssm01.Query("PONO,FACTORY_DIV");
			tpssm01.TrimOrBlank();

			////Log::Info("", __FUNCTION__,  "tpssm01["PLAN_DATE"] =[{0}]", tpssm01["PLAN_DATE"].ToString());

			if(tpssm01["PLAN_DATE"].ToString() != plan_date)
			{
				 //strcpy(s.msg, _RES("PSCMS0000062")/*相同生产日期的制造命令才允许移动*/);
				strcpy(s.msg, "相同生产日期的制造命令才允许移动");
				throw CApplicationException(-1, s.msg, log.Location);	
			}

			////Log::Trace("", __FUNCTION__,  "tpssm01["PONO_STATUS"] =[{0}]", tpssm01.PONO_STATUS );

			if(tpssm01["PONO_STATUS"].ToDecimal() != 13)
			{
			    //strcpy(s.msg, _RES("PSCMS0000061")/*该制造命令状态不为编制计划状态，不允许排序。*/);
				strcpy(s.msg, "该制造命令状态不为编制计划状态，不允许排序。");
				throw CApplicationException(-1, s.msg, log.Location);	
			}

			//更新修改者，修改时间
			tpssm01["REC_REVISOR"]     = CString(s.userid);
			tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
            
			tpssm01["CC_SEQ"] = ++cc_seq;
			////Log::Trace("", __FUNCTION__, " tpssm01["CC_SEQ"] = [{0}]", tpssm01["CC_SEQ"].ToDecimal());

			v_update = "REC_REVISOR,REC_REVISE_TIME,CC_SEQ";//修改字段信息。
			v_condi = "FACTORY_DIV,PONO"; //查询条件

			if(tpssm01.Update(v_update,v_condi)  != true)
			{
				strcpy(s.msg,"Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);	
			}

			//炼钢履历跟踪
			CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
			row3["EVENT_ID"] = "A3"; //计划调整
			row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			row3["PONO"] = tpssm01["PONO"];
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

			if (v_restrand_flg == "T")
			{
				v_lack_per++;
			}

			tpssm02["LACK_PER"] = v_lack_per;
			tpssm02.Update("LACK_PER", "CAST_LOT_NO");
		}
		cmd_tpssm02_inq.Close();

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock3, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
	}

	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = {  ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg)-1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	/*cmd_tpssm01_inq.Close();*/

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}

