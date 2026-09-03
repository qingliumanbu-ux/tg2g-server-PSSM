/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-6-27
Description:	 调宽标记: ：不调宽，1：炉内，2：炉间，3炉内炉间
Update: 
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 调宽标记
/// <para>数据库表：TPSSM10铸顺，TPSSM03板坯计划表 </para>
/// <para>主调用函数。</para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="CC_MACH_NO">连铸机号     </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm_mix_wid(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	int adjust_width_mark;
	
	CModel tpssm10("TPSSM10");
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{
		create_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		//获取传入参数
		tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		tpssm10["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().TrimOrBlank();

		//找到计划下发的CC_SEQ最大值
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			
				/*sqlstr = CString(
					" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, B.*,C.* "
					" from TPSSM10 A "
					" LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX1, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN1, PONO "
					" from TPSSM03 where PONO in(select PONO from TPSSM10) AND STRAND_NO = '1' group by PONO) B ON A.PONO = B.PONO "
					" LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX2, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN2, PONO "
					" from TPSSM03 where PONO in(select PONO from TPSSM10) AND STRAND_NO = '2' group by PONO) C ON A.PONO = C.PONO "
					
					" where PONO_STATUS < '83' AND    CC_SEQ != 999 "
					"   and CC_MACH_NO = @CC_MACH_NO "
					" order by CASE WHEN PONO_STATUS>'16' THEN 0 ELSE 1000 END + CC_SEQ "
					);*/
			sqlstr = CString(
				" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, B.*,C.* "
				" from TPSSM10 A "
				" LEFT JOIN(select nvl(MAX(SLAB_WIDTH),0) SLAB_WIDTH_MAX1, nvl(MIN(SLAB_WIDTH),0) SLAB_WIDTH_MIN1, PONO,Max(FIRST_SLAB_WIDTH) FIRST_SLAB_WIDTH_ODD,Max(LAST_SLAB_WIDTH) LAST_SLAB_WIDTH_ODD "
				" from(select PONO, SLAB_WIDTH, FIRST_VALUE(SLAB_WIDTH) OVER(PARTITION BY PONO order by slab_no) FIRST_SLAB_WIDTH, LAST_VALUE(SLAB_WIDTH) OVER(PARTITION BY PONO  order by slab_no) LAST_SLAB_WIDTH FROM TPSSM03 where PONO in(select PONO from TPSSM10) AND STRAND_NO = '1') group by PONO) B ON A.PONO = B.PONO "
				" LEFT JOIN(select nvl(MAX(SLAB_WIDTH), 0) SLAB_WIDTH_MAX2, nvl(MIN(SLAB_WIDTH), 0) SLAB_WIDTH_MIN2, PONO, Max(FIRST_SLAB_WIDTH) FIRST_SLAB_WIDTH_EVEN, Max(LAST_SLAB_WIDTH) LAST_SLAB_WIDTH_EVEN "
				" from(select PONO, SLAB_WIDTH, FIRST_VALUE(SLAB_WIDTH) OVER(PARTITION BY PONO order by slab_no) FIRST_SLAB_WIDTH, LAST_VALUE(SLAB_WIDTH) OVER(PARTITION BY PONO  order by slab_no) LAST_SLAB_WIDTH FROM TPSSM03 where PONO in(select PONO from TPSSM10) AND STRAND_NO = '2') group by PONO) C ON A.PONO = C.PONO "

				" where PONO_STATUS < '83' AND    CC_SEQ != 999 "
				"   and CC_MACH_NO = @CC_MACH_NO "
				" order by CASE WHEN PONO_STATUS>'16' THEN 0 ELSE 1000 END + CC_SEQ "
				);

			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		/* 对信息循环处理 */
		rows = bcls_ret->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{
			/* 比较炉内宽度 */
			if (bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH_MAX1"].ToDecimal() == bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH_MIN1"].ToDecimal()
				&& bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH_MAX2"].ToDecimal() == bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH_MIN2"].ToDecimal())
			{
				adjust_width_mark = 0;
			}
			else
			{
				adjust_width_mark = 1;
			}

			/* 比较炉间宽度 */
			if (i > 0)
			{
				if (bcls_ret->Tables[0].Rows[i]["FIRST_SLAB_WIDTH_ODD"].ToDecimal() != bcls_ret->Tables[0].Rows[i - 1]["LAST_SLAB_WIDTH_ODD"].ToDecimal()
					|| bcls_ret->Tables[0].Rows[i]["FIRST_SLAB_WIDTH_EVEN"].ToDecimal() != bcls_ret->Tables[0].Rows[i - 1]["LAST_SLAB_WIDTH_EVEN"].ToDecimal())
				{
					adjust_width_mark += 2;
				}
			}			

			tpssm10["PONO"] = bcls_ret->Tables[0].Rows[i]["PONO"];
			if (adjust_width_mark > 0)
				tpssm10["ADJUST_WIDTH_MARK"] = adjust_width_mark;
			else
				tpssm10["ADJUST_WIDTH_MARK"] = " ";

			if (i > 0 || bcls_ret->Tables[0].Rows[i]["ADJUST_WIDTH_MARK"].ToString().Trim() == "")
			{
				tpssm10.Update("ADJUST_WIDTH_MARK", "PONO");
			}			

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

