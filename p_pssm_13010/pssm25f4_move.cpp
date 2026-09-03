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


/*<remark>=========================================================
/// <summary>
/// 1.取计划下发的最大CC_SEQ,将所有编入计划的CC_SEQ重置
/// 2.必须是计划编制状态的Pono才可以调整顺序
/// <para>
/// </para>
/// <para>数据库表：tpssm05(炼钢制造命令表)          </para>
/// <para>主调用函数： 前台PSSM01画面F6 移动调用     </para>
/// </summary>
/// <param name="pono">制造命令   </param>
/// <returns>顺序调整后的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm25f4_move)
//-EP_SYSTEM_HEAD_END
int f_pssm25f4_move(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret = 0;

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息。
	CString sqlstr = "";
	int v_mat_seq_no = 0;

	/* 实体类定义 */
	CModel tpssm05("TPSSM05");

	EIClass inBlock3; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm05_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);

	try
	{
		//获取前台传入参数
		tpssm05["FACTORY_DIV"] = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		tpssm05["PLAN_BACKLOG_CODE"] = bcls_rec->Tables[1].Rows[0]["PLAN_BACKLOG_CODE"].ToString().TrimOrBlank();
		tpssm05["PLAN_NO"] = bcls_rec->Tables[1].Rows[0]["PLAN_NO"].ToString().TrimOrBlank();//取出第一行生产日期

		////Log::Info("", __FUNCTION__, "tpssm05["FACTORY_DIV"] = [{0}]", tpssm05["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "tpssm05["PLAN_BACKLOG_CODE"] = [{0}]", tpssm05["PLAN_BACKLOG_CODE"].ToString());
		////Log::Info("", __FUNCTION__, "tpssm05["PLAN_NO"] = [{0}]", tpssm05["PLAN_NO"].ToString());

		//获取当前工序已释放的计划的最大计划执行顺序号,
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库			
		default: // 所有数据库适用，通用SQL语句
			sqlstr = " SELECT NVL(MAX(MAT_SEQ_NO),0)  FROM TPSSM05 "
				" WHERE FACTORY_DIV = @tpssm05.FACTORY_DIV "
				"   AND PLAN_BACKLOG_CODE = @tpssm05.PLAN_BACKLOG_CODE "
				"   AND PLAN_NO = @tpssm05.PLAN_NO "
				"   AND PLAN_STATUS >= '06' ";
			break;
		}
		cmd_tpssm05_inq.SetCommandText(sqlstr);
		cmd_tpssm05_inq.Parameters.Set("tpssm05.FACTORY_DIV", tpssm05["FACTORY_DIV"].ToString());
		cmd_tpssm05_inq.Parameters.Set("tpssm05.PLAN_BACKLOG_CODE", tpssm05["PLAN_BACKLOG_CODE"].ToString());
		cmd_tpssm05_inq.Parameters.Set("tpssm05.PLAN_NO", tpssm05["PLAN_NO"].ToString());
		cmd_tpssm05_inq.ExecuteReader();

		if (cmd_tpssm05_inq.Read())
		{
			v_mat_seq_no = cmd_tpssm05_inq.GetInt32(1);
		}
		cmd_tpssm05_inq.Close();

		////Log::Trace("", __FUNCTION__, " cc_seq = [{0}]", v_mat_seq_no);

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, " rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm05["PLAN_NO"] = bcls_rec->Tables[0].Rows[i]["PLAN_NO"];
			tpssm05["IN_MAT_NO"] = bcls_rec->Tables[0].Rows[i]["IN_MAT_NO"];
			tpssm05["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];

			//打印输入参数
			////Log::Info("", __FUNCTION__, "tpssm05["IN_MAT_NO"] =[{0}]", tpssm05["IN_MAT_NO"].ToString());
			////Log::Info("", __FUNCTION__, "tpssm05["FACTORY_DIV"] =[{0}]", tpssm05["FACTORY_DIV"].ToString());

			sqlstr = "tpssm05.Query()";
			tpssm05.Query("PONO,FACTORY_DIV");
			tpssm05.TrimOrBlank();

			////Log::Trace("", __FUNCTION__, "tpssm05["PLAN_STATUS"] =[{0}]", tpssm05["PLAN_STATUS"].ToString());

			if (tpssm05["PLAN_STATUS"].ToString() != "04")
			{
				strcpy(s.msg, _RES("PSCMS0000061")/*该制造命令状态不为编制计划状态，不允许排序。*/);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//更新修改者，修改时间
			tpssm05["REC_REVISOR"] = CString(s.userid);
			tpssm05["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

			tpssm05["MAT_SEQ_NO"] = ++v_mat_seq_no;
			////Log::Trace("", __FUNCTION__, " tpssm05["MAT_SEQ_NO"] = [{0}]", tpssm05["MAT_SEQ_NO"].ToDecimal());

			v_update = "REC_REVISOR,REC_REVISE_TIME,MAT_SEQ_NO";//修改字段信息。
			v_condi = "FACTORY_DIV,IN_MAT_NO,PLAN_NO"; //查询条件

			if (tpssm05.Update(v_update, v_condi) != true)
			{
				strcpy(s.msg, "Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
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

	/*cmd_tpssm05_inq.Close();*/

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;

}
