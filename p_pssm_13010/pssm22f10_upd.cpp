/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-09-10 17:13:56
Description: 炼钢日出钢能力（PSSM21）-炼钢设备预定休止计划修改
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 炼钢设备预定休止计划修改
/// <para>数据库表：tpssm22(炼钢设备预定休止计划表)          </para>
/// <para>前台 PSSM21 画面(出钢能力)调用         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 炼钢设备预定休止计划表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm22f10_upd)

int f_pssm22f10_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	CString date_Now = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString sqlstr = "";

	CModel tpssm22("TPSSM22");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm22_inq(conn);  //与DB 建立连接。

	try
	{
		//--------------------------------
		//获取传入参数

		/* ***** 打印输入参数 ***** */

		int count = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < count; i++)
		{
			tpssm22.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//判断是否存在相同设备下的休止时间段没有重叠
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT COUNT(*) FROM TPSSM22 \
							WHERE FACTORY_DIV = @FACTORY_DIV \
							AND DEV_CODE = @DEV_CODE \
							AND(STOPPAGE_TIME_START < @STOPPAGE_TIME_START AND STOPPAGE_TIME_END > @STOPPAGE_TIME_START ) \
							OR(STOPPAGE_TIME_START < @STOPPAGE_TIME_END AND STOPPAGE_TIME_END   > @STOPPAGE_TIME_END ) ";
				break;
			}
			cmd_tpssm22_inq.SetCommandText(sqlstr);
			cmd_tpssm22_inq.Parameters.Set("FACTORY_DIV", tpssm22["FACTORY_DIV"].ToString());
			cmd_tpssm22_inq.Parameters.Set("DEV_CODE", tpssm22["DEV_CODE"].ToString());
			cmd_tpssm22_inq.Parameters.Set("STOPPAGE_TIME_START", tpssm22["STOPPAGE_TIME_START"].ToString());
			cmd_tpssm22_inq.Parameters.Set("STOPPAGE_TIME_END", tpssm22["STOPPAGE_TIME_END"].ToString());
			cmd_tpssm22_inq.ExecuteReader();
			if (cmd_tpssm22_inq.Read())
			{
				int count_22 = cmd_tpssm22_inq.GetInt32(1);
				////Log::Info("", __FUNCTION__, "count_22 = [{0}]", count_22);

				if (count_22 > 0)
				{
					CFormattable arguments[] = { tpssm22["SEQ_NO"].ToDecimal() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "新增的预定休止计划与[{0}]在休止时间上存在重叠。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			cmd_tpssm22_inq.Close();

			tpssm22["REC_REVISE_TIME"] = date_Now;
			tpssm22["REC_REVISOR"] = s.userid;

			sqlstr = "tpssm22.Update";
			tpssm22.Update("REC_REVISOR,REC_REVISE_TIME,DEV_CODE,STOPPAGE_TIME_START,STOPPAGE_TIME_END,STOPPAGE_CAUSE_DESC", "FACTORY_DIV,SEQ_NO");

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
