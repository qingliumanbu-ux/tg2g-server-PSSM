/*=========================================================================
//程序名称:		pssm23_inq
//隶属子系统:	PSSM
//产品名称:		
//创建人员:     chejs
//创建时间:     2015-9-24 15:04:48
//修改人员:
//修改日期:
//-----------------------------------------------------------------------
//功能描述:		规程校验结果查询
//数据库表:     TPSSM23
//表中文名:     炼钢作业计划规程出错表
//主调用函数:   前台PSSM23画面F2(查询)调用
//需调用函数:
//-----------------------------------------------------------------------
//函数功能:     
//传入参数:
//传出参数:
//处理流程:
//1.根据前台传入生产日期范围进行查询
//=========================================================================*/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 制造命令查询
/// <para>
/// 1.根据pono,cc_mach_no等条件进行制造命令查询。
///
/// </para>
/// <para>数据库表：tpssm22(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM01画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm23_inq)

int f_pssm23_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	CString sqlstr = "";
	CString factory_div = "";
	CString plan_date_begin = ""; //计划日期开始
	CString plan_date_end = ""; //计划日期结束

	CModel tpssm23("TPSSM23");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm23_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tsi00c0_inq(conn);  //与DB 建立连接。

	try
	{
		//获取传入参数
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		plan_date_begin = bcls_rec->Tables[0].Rows[0]["PLAN_DATE_BEGIN"].ToString();
		plan_date_end = bcls_rec->Tables[0].Rows[0]["PLAN_DATE_END"].ToString();

		//打印输入参数
		////Log::Info("", __FUNCTION__, "pssm23_inq>factory_div = [{0}]", factory_div);
		////Log::Info("", __FUNCTION__, "pssm23_inq>plan_date_begin = [{0}]", plan_date_begin);
		////Log::Info("", __FUNCTION__, "pssm23_inq>plan_date_end = [{0}]", plan_date_end);

		//查询TPSSM23炼钢作业计划规程出错表
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = "SELECT * FROM TPSSM23 "
				"WHERE FACTORY_DIV LIKE @FACTORY_DIV || '%'"
				"AND PLAN_DATE >= @PLAN_DATE_BEGIN  "
				"AND PLAN_DATE <= @PLAN_DATE_END "
				"ORDER BY PLAN_DATE ASC, SEQ_NO ASC ";
			break;
		}
		cmd_tpssm23_inq.SetCommandText(sqlstr);
		cmd_tpssm23_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_tpssm23_inq.Parameters.Set("PLAN_DATE_BEGIN", plan_date_begin);
		cmd_tpssm23_inq.Parameters.Set("PLAN_DATE_END", plan_date_end);
		ret = cmd_tpssm23_inq.ExecuteQuery(bcls_ret->Tables[0]);
		////Log::Info("", __FUNCTION__, "ret = [{0}]", ret);
		cmd_tpssm23_inq.Close();

		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ERR_EXPLAIN");

		for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			CString err_code = bcls_ret->Tables[0].Rows[i]["ERR_CODE"].ToString();
			//根据违规代码,查询代码说明
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = "SELECT NVL(UNRULL_FLAG_MC, ' ') "
					"FROM TSI00C0 "
					"WHERE UNRULL_FLAG_ID = @UNRULL_FLAG_ID ";
				break;
			}
			cmd_tsi00c0_inq.SetCommandText(sqlstr);
			cmd_tsi00c0_inq.Parameters.Set("UNRULL_FLAG_ID", err_code);
			cmd_tsi00c0_inq.ExecuteReader();
			if (cmd_tsi00c0_inq.Read())
			{
				CString err_explain = cmd_tsi00c0_inq.GetString(1);
				bcls_ret->Tables[0].Rows[i]["ERR_EXPLAIN"] = err_explain;
			}
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
