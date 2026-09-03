/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-09-10 17:13:56
Description: 炼钢日出钢能力（PSSM22）-炼钢设备预定休止计划删除
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/


/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 炼钢设备预定休止计划删除
/// <para>数据库表：tpssm22(炼钢设备预定休止计划表)          </para>
/// <para>前台 PSSM21 画面(出钢能力)调用         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 炼钢设备预定休止计划表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm22f11_del)

int f_pssm22f11_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";

	CModel tpssm22("TPSSM22");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm22_inq(conn);  //与DB 建立连接。

	try
	{
		//--------------------------------
		//获取传入参数
		tpssm22.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		/* ***** 打印输入参数 ***** */

		int count = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < count; i++)
		{
			tpssm22.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " DELETE FROM TPSSM22 \
					WHERE FACTORY_DIV = @FACTORY_DIV \
					AND SEQ_NO = @SEQ_NO ";
				break;
			}
			cmd_tpssm22_inq.SetCommandText(sqlstr);
			cmd_tpssm22_inq.Parameters.Set("FACTORY_DIV", tpssm22["FACTORY_DIV"].ToString());
			cmd_tpssm22_inq.Parameters.Set("SEQ_NO", tpssm22["SEQ_NO"].ToDecimal());
			cmd_tpssm22_inq.ExecuteNonQuery();
			cmd_tpssm22_inq.Close();
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
