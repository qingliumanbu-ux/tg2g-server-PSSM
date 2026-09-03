/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2011-12-22
Description: 炼钢计划撤销
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


int f_pmom_status_upd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 撤销炼钢计划
/// <para>
/// 1.PONO状态修改
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)         </para>
/// <para>主调用函数： </para>
/// </summary>
/// <param name="FACTORY_DIV">炼钢厂别代码    </param>
/// <returns>连铸机信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm05_retu)

int f_pssm05_retu(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret;

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息
	CString sqlstr;

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm01_inq_1(conn);

	EIClass inBlock; //调用生产接口
	EIClass inBlock3; //调用炼钢履历跟踪
	EIClass outBlock;

	try
	{
		//调用生产的接口参数
		inBlock.Tables[0].set_TableName("PSSMCHS");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");

		//调用炼钢履历跟踪
		inBlock3.Tables[0].set_TableName("TRACE");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//打印输入参数
			////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}],CAST_LOT_NO=[{1}]", tpssm01["FACTORY_DIV"].ToString(), tpssm01["CAST_LOT_NO"].ToString());

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT * FROM TPSSM01  WHERE FACTORY_DIV = @FACTORY_DIV AND CAST_LOT_NO = @cast_lot_no ";
				break;
			}
			cmd_tpssm01_inq_1.SetCommandText(sqlstr);
			cmd_tpssm01_inq_1.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq_1.Parameters.Set("cast_lot_no", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm01_inq_1.ExecuteReader();

			while (cmd_tpssm01_inq_1.Read())
			{
				cmd_tpssm01_inq_1.Fetch(tpssm01);
				tpssm01.TrimOrBlank();

				if ((tpssm01["PONO_STATUS"].ToDecimal() != 12) && (tpssm01["PONO_STATUS"].ToDecimal() != 13)) //计划编制
				{
					continue;
				}

				if (tpssm01["RES_CODE"].ToString() == "R")
				{
					CFormattable arguments[] = { tpssm01["CAST_LOT_NO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "再排浇次号[{0}]不允许撤销，请确认", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tpssm01["CC_MACH_NO"] = " ";
				tpssm01["PLAN_DATE"] = " ";
				tpssm01["SHIFT_NO"] = " ";
				tpssm01["SHIFT_GROUP"] = " ";
				tpssm01["REC_REVISOR"] = CString(s.userid);
				tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tpssm01["CC_SEQ"] = 0;
				tpssm01["PONO_STATUS"] = 11; //命令接收

				v_update = "REC_REVISOR,REC_REVISE_TIME,CC_MACH_NO,PONO_STATUS,PLAN_DATE,SHIFT_NO,SHIFT_GROUP,CC_SEQ";//修改字段信息。
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				if (tpssm01.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				////根据去向调用不同的函数
				//switch(conn->DatabaseKind)
				//{
				//	case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//	case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				//	case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//	case DB_KIND_ORACLE:	    // Oracle 数据库
				//	default:
				//	sqlstr = CString(" SELECT MAT_DESTION "
				//					 " FROM TPSSM01 "
				//					 " WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
				//					 " AND   PONO		 = @tpssm01.PONO ");
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

				//声明调用生产的接口参数
				inBlock.Tables[0].Rows.Clear();
				inBlock.Tables[0].Rows.Add();
				inBlock.Tables[0].Rows[0]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				inBlock.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];

				doFlag = f_pmom_status_upd2(&inBlock, &outBlock, conn);

				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				//炼钢履历跟踪
				CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
				row3["EVENT_ID"] = "A2"; //计划撤销
				row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
				row3["PONO"] = tpssm01["PONO"];
				row3["PONO_STATUS"] = tpssm01["PONO_STATUS"];

			}
			cmd_tpssm01_inq_1.Close();

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

	//返回-1时事务将回滚，返回为0是事务将提交

	return doFlag;
}
