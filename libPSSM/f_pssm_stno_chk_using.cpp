/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-17
Version:1.0
Description: 检查指定的出钢记号是否被计划使用
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

/*<remark>=========================================================
/// <summary>
/// 检查指定的出钢记号是否被计划使用次计算
/// <para>1.检查所接收的出钢记号在连铸预计划表里是否存在；</para>
/// <para>2.检查所接收的出钢记号在出钢计划表里是否存在；</para>
/// </para>
/// <para>数据库表：tpssm01/11</para>
/// <para>主调用函数：质量调用</para>
/// </summary>
/// <param name="ST_NO">内部钢种</param>
/// <param name="check_flag">检查标记</param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm_stno_chk_using(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;
	CString check_flag=" ";  //检查类别: 1-连铸预计划; 2-出钢计划;

	CDecimal dummy =0;

	CModel tpssm01("TPSSM01");
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm01_inq(conn);
	CString sqlstr;

	try
	{
		//bcls_ret->Tables[0].set_TableName("STNO");
		//bcls_ret->Tables[0].Columns.Add(DT_STRING,"USING_CODE"); //0-不在计划中;

		//获得输入参数
		blkseq = 1;
		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[blkseq-1].Rows[0]["FACTORY_DIV"];
		if(bcls_rec->Tables[blkseq-1].Columns.Contains("CHECK_FLAG"))
		{
			check_flag = bcls_rec->Tables[blkseq-1].Rows[0]["CHECK_FLAG"]; //1-连铸预计划; 2-出钢计划;
		}
		////Log::Info("", __FUNCTION__, "tpssm01["FACTORY_DIV"] =[{0}]",(const char*)tpssm01["FACTORY_DIV"].ToString());

		/* 对输入信息循环处理 */
		blkseq = 2;
		rows = bcls_rec->Tables[blkseq-1].Rows.get_Count();
		for (i = 1; i <= rows; i++ )
		{
			tpssm01["ST_NO"] = bcls_rec->Tables[blkseq-1].Rows[i-1]["ST_IDX_NO"];

			////Log::Trace("", __FUNCTION__, "tpssm01["ST_NO"] =[{0}]",(const char*)tpssm01["ST_NO"].ToString());
			//if ( check_flag[0] == '1' )
			//{
			//查询所接收的出钢记号在连铸预计划表里是否存在
			dummy  = 0;
			// EXEC SQL
			//SELECT COUNT(*) INTO :num
			//				  FROM TPSSM01  //计划编制主表
			//				 WHERE FACTORY_DIV = trim(:tpssm11.FACTORY_DIV) /*主工序代码*/
			//				   AND ST_NO             = trim(:tpssm11.ST_NO)
			//				   AND pono_status       >= 13  //13-计划收池
			//				   AND pono_status       <= 90  //91-炉次确定
			//				   ;
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:

				/*sqlstr = "SELECT COUNT(*) FROM TPSSM01   \
						 WHERE FACTORY_DIV  = TRIM(@factory_div)  \
						 AND ST_NO              = TRIM(@st_no) \
						 AND PONO_STATUS        > 16   \
						 AND PONO_STATUS        <= 90  ";*/
			

				sqlstr = "SELECT COUNT(*) FROM TPSSM01"
						" WHERE  ST_NO    = @st_no "
						" AND PONO_STATUS > 16  "
						" AND PONO_STATUS <= 90  ";

			
				if(tpssm01["FACTORY_DIV"].ToString().Trim() != "")
				{
					sqlstr	+= " AND FACTORY_DIV	= @factory_div"; 
				}
				break;
			}

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("factory_div",tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq.Parameters.Set("st_no",tpssm01["ST_NO"].ToString());
			dummy = cmd_tpssm01_inq.ExecuteScalar();
			if (dummy > 0)
			{
				CFormattable arguments[] = { tpssm01["ST_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000026")/*出钢记号[{0}]已排入出钢计划。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}


			//}
			//else if (check_flag[0] == '2' )
			//{
			//查询所接收的出钢记号在出钢计划表里是否存在
			//dummy = 0;
			// EXEC SQL
			//SELECT COUNT(*) INTO :num
			//				  FROM tpssm11  //计划编制主表
			//				 WHERE FACTORY_DIV = trim(:tpssm11.FACTORY_DIV) /*主工序代码*/
			//				   AND ST_NO             = trim(:tpssm11.ST_NO);

			//	dummy = tpssm11.QueryCount("FACTORY_DIV,ST_NO");
			//	if (dummy > 0)
			//	{
			//		CFormattable arguments[] = { tpssm11.ST_NO }; // 定义参数列表的数组
			//		CMessageFormat::Format(s.msg, _RES("PSSMS0000026")/*出钢记号[{0}]已排入出钢计划。*/, arguments, 1); //格式化字符串
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			//}
		}

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}
