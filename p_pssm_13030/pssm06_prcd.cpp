/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2022
作者: 涂献计
日期: 2022-04-06
功能: 炼钢模铸预计划调整确定炉次收回
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


#if defined _SYS_MMS
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

/*<remark >=========================================================
/// <summary >
/// 模铸预计划调整确定炉次收回
/// <para >
/// 1.根据传入的CAST - LOT号、修改制造命令信息。
/// 2.更新条件：CAST - LOT号；
/// 3.更新字段：画面输入炼钢区分，查询出制造命令号，制造命令状态 = "4"；
/// </para >
/// <para > 数据库表：TPSSMIC00(制造命令炉次表)</para >
/// <para > 主调用函数：前台pssm06画面F5(炉次收回)调用。   </para >
/// </summary >
/// <param name = "CAST_LOT_NO" > CAST_LOT号</param >
/// <returns > 模铸预计划调整确定信息</returns >
/// <returns > 成功：0</returns >
/// <returns > 失败：-1</returns >
=========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssm06_prcd);

int f_pssm06_prcd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/
	int doFlag = 0;		//返回值
	int logFlag = 1;
	CString cast_lot_no;
	int pono_status;
	CString pono;
	CString slab_dest;
	CString cc_mach_no;
	CString unit_code;
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息
	CDecimal v_cc_seq = 0;
	int ret;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CDbCommand comd(conn);

	CDbCommand cmd_tpssm01_inq(conn);

	//调用生产接口
	EIClass inBlock;
	EIClass inBlock1;
	EIClass inBlock3; //调用炼钢履历跟踪
	EIClass outBlock;

	CString datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{
		//声明调用电文的接口参数
		inBlock1.Tables[0].set_TableName("PONOSEND");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "MARKS1");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");

		//记录总条数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		//循环处理
		for (int i = 0; i < count; i++)
		{
			pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
			// 取得单行传入信息 
			tpssm01.Reset();
			tpssm01["PONO"] = pono;
			tpssm01.Query("PONO");

			//HYF 20130401 之前是校验>15的状态，16是PES命令接收状态，18才是排入计划
			if (tpssm01["PONO_STATUS"].ToDecimal() >= 18)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSCMS0000055")/*制造命令号[{0}]已排入出钢计划。*/, arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//如果是热轧去向，不允许炉次收回
			if (tpssm01["SLAB_DEST"].ToString() == "11")
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]为热轧去向，不能炉次收回操作!", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (tpssm01["RESTRAND_FLG"].ToString() == "1")
			{
				v_cc_seq = tpssm01["CC_SEQ"];
				//更新修改者，修改时间
				tpssm01["REC_REVISOR"] = CString(s.userid);
				tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

				tpssm01["CC_SEQ"] = v_cc_seq + 1;

				v_update = "REC_REVISOR, REC_REVISE_TIME, RESTRAND_FLG";
				v_condi = "FACTORY_DIV, CC_MACH_NO, CC_SEQ"; //查询条件

				if (tpssm01.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tpssm01["CC_SEQ"] = v_cc_seq;
			}

			//根据去向调用不同的函数
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

				sqlstr = CString(" UPDATE TPSSM01 "
					" SET CC_SEQ = CC_SEQ - 1 "
					" WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
					" AND CC_MACH_NO = @tpssm01.CC_MACH_NO "
					" AND CC_SEQ > @tpssm01.CC_SEQ "
					" AND CC_SEQ < 900 ");
				break;
			}
			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
			cmd_tpssm01_inq.Parameters.Set("tpssm01.CC_SEQ", tpssm01["CC_SEQ"].ToDecimal());
			cmd_tpssm01_inq.ExecuteNonQuery();

			//更新修改者，修改时间
			tpssm01["REC_REVISOR"] = CString(s.userid);
			tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

			tpssm01["ADJUST_FLAG"] = " ";
			tpssm01["PONO_STATUS"] = 11; //命令接收
			tpssm01["CC_SEQ"] = 0;
			tpssm01["PLAN_DATE"] = " ";
			tpssm01["CC_MACH_NO"] = " ";
			tpssm01["SHIFT_NO"] = " ";
			tpssm01["SHIFT_GROUP"] = " ";

			v_update = "REC_REVISOR,REC_REVISE_TIME,ADJUST_FLAG,PONO_STATUS,CC_SEQ,PLAN_DATE,CC_MACH_NO,SHIFT_NO,SHIFT_GROUP";
			v_condi = "FACTORY_DIV,PONO"; //查询条件

			if (tpssm01.Update(v_update, v_condi) != true)
			{
				strcpy(s.msg, "Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

#ifdef _SYS_MMS
			//调用计划下发电文
			inBlock1.Tables[0].Rows.Clear();
			inBlock1.Tables[0].Rows.Add();
			inBlock1.Tables[0].Rows[0]["MARKS1"] = 3;
			inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
			//增加主工序代码 HYF 20130422
			inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];

			/*ret = f_cm_002021_snd(&inBlock1, &outBlock, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}*/
#endif

#ifdef _SYS_MES

#endif

			//炼钢履历跟踪
			CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
			row3["EVENT_ID"] = "C2"; //PONO收回
			row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			row3["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			row3["PONO"] = tpssm01["PONO"];
			row3["PONO_STATUS"] = tpssm01["PONO_STATUS"];

		}
		CFormattable arguments[] = { count };
		CMessageFormat::Format(s.msg, "炉次收回了[{0}]条记录", arguments, 1);


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
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
		strncpy(s.msg, (const char *)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}


