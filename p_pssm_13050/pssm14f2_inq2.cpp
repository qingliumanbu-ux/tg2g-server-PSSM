/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-03-01 17:13:56  
Description: 制造命令查询
**************************************************/

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
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM09画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm14f2_inq2)

int f_pssm14f2_inq2(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int		TotalRecordCount = 0  ;
	CDecimal strand_num = 0  ;

		

	//系统的分页类信息。
	CPageInfo pageInfo; 
 
	CModel tpssm01("TPSSM01");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm03("TPSSM03");

	CDbCommand cmd_inq(conn);

	try
	{
		string t_oddslab = "t_oddslab";
		string t_evenslab = "t_evenslab";
		if (bcls_ret->Tables.Contains(t_oddslab))
			bcls_ret->Tables.Remove(t_oddslab);
		if (bcls_ret->Tables.Contains(t_evenslab))
			bcls_ret->Tables.Remove(t_evenslab);

		bcls_ret->Tables.Add(t_oddslab);
		bcls_ret->Tables.Add(t_evenslab);
		for (int i = 0; i < 10; i++)
		{
			bcls_ret->Tables[t_oddslab].Columns.Add(DT_STRING);
			bcls_ret->Tables[t_evenslab].Columns.Add(DT_STRING);
		}

		//打印输入参数
		tpssm11["CAST_PONO_SUM"] = bcls_rec->Tables[0].Rows[0]["CAST_PONO_SUM"].ToDecimal();
		tpssm11["CAST_NO"] = bcls_rec->Tables[0].Rows[0]["CAST_NO"].ToString();
		tpssm01["CAST_LOT_NO"] = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO"].ToString();
		tpssm11["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();
		
		Log::Info("", __FUNCTION__, "CAST_PONO_SUM[{0}]CAST_NO[{1}]CAST_LOT_NO[{2}]", tpssm11["CAST_PONO_SUM"].ToDecimal(), tpssm11["CAST_NO"].ToString(), tpssm01["CAST_LOT_NO"].ToString());
		
		//获取铸机流数
		sqlstr = " SELECT MAX(STRAND_NUM) STRAND_NUM FROM TPSSMD9 WHERE CC_MACH_NO = @CC_MACH_NO GROUP BY CC_MACH_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			strand_num = cmd_inq.GetDecimal(1);
		}
		cmd_inq.Close();
		Log::Info("", __FUNCTION__, "strand_num[{0}]", strand_num);

		//获取计划去向
		sqlstr = " SELECT * FROM TPSSM01 WHERE CAST_LOT_NO = @CAST_LOT_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm01);
		}
		cmd_inq.Close();
		Log::Info("", __FUNCTION__, "SLAB_DEST[{0}]", tpssm01["SLAB_DEST"].ToString());

		if (strand_num > 2)//方坯
		{
			sqlstr =
				" SELECT A.*, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO||'-'||B.CAST_DIV_NO) CAST_NO, B.CAST_PONO_SUM, C.CAST_LOT_NO, C.ST_NO, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO) CAST_NO_1, B.CAST_DIV_NO"
				" FROM TPSSM03 A"
				" LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
				" JOIN TPSSM01 C ON A.PONO = C.PONO AND C.CC_MACH_NO = @CC_MACH_NO"
				" WHERE 1=1"
				" AND C.CAST_LOT_NO = @CAST_LOT_NO"
				" ORDER BY A.STRAND_NO, A.STRAND_NUM, A.SLAB_SEQ_2"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_PONO_SUM", tpssm11["CAST_PONO_SUM"].ToDecimal());
			cmd_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_inq.Parameters.Set("CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
			cmd_inq.ExecuteQuery(bcls_ret->Tables[t_oddslab]);
			cmd_inq.Close();
		}
		else
		{
			if (tpssm01["FACTORY_DIV"].ToString() == "C31")
			{
				sqlstr =
					" SELECT A.*, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO||'-'||B.CAST_DIV_NO) CAST_NO, B.CAST_PONO_SUM, C.CAST_LOT_NO, C.ST_NO, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO) CAST_NO_1, B.CAST_DIV_NO"
					" FROM TPSSM03 A"
					" JOIN TPSSM11 B ON A.PONO = B.PONO"
					" LEFT JOIN TPSSM01 C ON A.PONO = C.PONO"
					" WHERE B.CAST_PONO_SUM = @CAST_PONO_SUM"
					" AND A.STRAND_NO = @STRAND_NO"
					" ORDER BY A.STRAND_NO, A.STRAND_NUM, A.SLAB_SEQ_2"
					;
			}
			else
			{
				if ((tpssm01["SLAB_DEST"].ToString() >= "21" && tpssm01["SLAB_DEST"].ToString() <= "25")
					|| (tpssm01["SLAB_DEST"].ToString() == "90" && tpssm01["LINE_TYPE"].ToString() == "HP")
					)//厚板按炉
				{
					sqlstr =
						" SELECT A.*, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO||'-'||B.CAST_DIV_NO) CAST_NO, B.CAST_PONO_SUM, C.CAST_LOT_NO, C.ST_NO, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO) CAST_NO_1, B.CAST_DIV_NO"
						" FROM TPSSM03 A"
						" JOIN TPSSM11 B ON A.PONO = B.PONO"
						" JOIN TPSSM01 C ON A.PONO = C.PONO"
						" WHERE 1=1"
						" AND B.CAST_PONO_SUM = @CAST_PONO_SUM"
						" AND A.STRAND_NO = @STRAND_NO"
						" AND STEEL_RETURN_CODE = ' '"
						" ORDER BY B.CAST_NO, B.CAST_DIV_NO, B.PONO, A.STRAND_NUM, A.SLAB_SEQ_1"

						;
				}
				else
				{
					sqlstr =
						" SELECT A.*, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO||'-'||B.CAST_DIV_NO) CAST_NO, B.CAST_PONO_SUM, C.CAST_LOT_NO, C.ST_NO, DECODE(B.CAST_NO, NULL, '未编制', B.CAST_NO) CAST_NO_1, B.CAST_DIV_NO"
						" FROM TPSSM03 A"
						" LEFT JOIN TPSSM11 B ON A.PONO = B.PONO"
						" JOIN TPSSM01 C ON A.PONO = C.PONO AND C.CC_MACH_NO = @CC_MACH_NO"
						" WHERE 1=1"
						" AND C.CAST_LOT_NO = @CAST_LOT_NO"
						" AND A.STRAND_NO = @STRAND_NO"
						" ORDER BY A.STRAND_NUM, A.SLAB_SEQ_2"
						;
				}
			}

			Log::Info("", __FUNCTION__, "===sqlstr=== [{0}]", sqlstr);

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("CAST_PONO_SUM", tpssm11["CAST_PONO_SUM"].ToDecimal());
			cmd_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_inq.Parameters.Set("CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
			cmd_inq.Parameters.Set("STRAND_NO", "1");
			cmd_inq.ExecuteQuery(bcls_ret->Tables[t_oddslab]);
			cmd_inq.Parameters.Set("STRAND_NO", "2");
			cmd_inq.ExecuteQuery(bcls_ret->Tables[t_evenslab]);
			cmd_inq.Close();
		}
		

	 }
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		//__AG_DB_EXCEPTION_;		//使用EAppDef.h中宏定义 
			s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
