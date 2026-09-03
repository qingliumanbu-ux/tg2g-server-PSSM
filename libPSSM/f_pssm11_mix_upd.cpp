/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-6-27
Description:	 根据混交基准表(TPSSM46)判断混浇标志
                 混浇标记：1、允许混浇（L4）；2、混浇需人工确定（L4）；0、强制混浇（不满足1、2、4的）；4、禁止混浇（禁止混浇标记 + 硬度组规则）
Update: 
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "math.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 调宽标记
/// <para>数据库表：TQMTS13混浇分组，TQMTS14混浇确认表 </para>
/// <para>主调用函数。</para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="CC_MACH_NO">连铸机号     </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm11_mix_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	CString intermix_type_pono;
	
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
			
			sqlstr = CString(
				" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, nvl(E.ST_NO_TO,A.ST_NO) CAN_MIX_GROUP,nvl(F.IDX_NO,A.ST_NO) WORRY_MIX_GROUP "
				" ,G.ARCHIVE_FLAG,G.HARDNESS_GROUP,G.HARD_GROUP "
				" from TPSSM10 A "
				" LEFT JOIN TQMTS13 E ON E.ST_NO_FROM = A.ST_NO "
				" LEFT JOIN TQMTS14 F ON F.IDX_NO_01 = E.ST_NO_TO "
				" LEFT JOIN TQMTS0X G ON G.ST_NO = A.PONO "
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
		for (i = 1; i < rows; i++ )
		{
			/* 同出钢记号或者换包 */
			if ((strcmp(bcls_ret->Tables[0].Rows[i]["ST_NO"].ToString(), bcls_ret->Tables[0].Rows[i - 1]["ST_NO"].ToString()) == 0)
				|| bcls_ret->Tables[0].Rows[i]["TD_CHG_FLG"].ToDecimal() == 1)
			{
				intermix_type_pono = " ";
			}
			else
			{
				intermix_type_pono = 1;
				/* 允许混浇 */
				if (strcmp(bcls_ret->Tables[0].Rows[i]["CAN_MIX_GROUP"].ToString(), bcls_ret->Tables[0].Rows[i - 1]["CAN_MIX_GROUP"].ToString())==0)
				{
					intermix_type_pono = "1";
				}
				else if (strcmp(bcls_ret->Tables[0].Rows[i]["WORRY_MIX_GROUP"].ToString(), bcls_ret->Tables[0].Rows[i - 1]["WORRY_MIX_GROUP"].ToString()) == 0)
				{
					intermix_type_pono = "2";
				}
				else
				{
					//-----------禁止混浇---------------------
					if ((0 == strcmp(bcls_ret->Tables[0].Rows[i]["ARCHIVE_FLAG"].ToString(), "1") || 0 == strcmp(bcls_ret->Tables[0].Rows[i-1]["ARCHIVE_FLAG"].ToString(), "1")) 
						|| 2 < fabs(bcls_ret->Tables[0].Rows[i]["ARCHIVE_FLAG"].ToDecimal().ToDouble() - bcls_ret->Tables[0].Rows[i - 1]["ARCHIVE_FLAG"].ToDecimal().ToDouble()))//禁止混浇
					{
						intermix_type_pono = "4";
					}
					else//强制混浇
					{
						intermix_type_pono = "0";
					}
				}
			}

			tpssm10["PONO"] = bcls_ret->Tables[0].Rows[i]["PONO"];
			tpssm10["INTERMIX_FLAG_PONO"] = intermix_type_pono;

			tpssm10.Update("INTERMIX_FLAG_PONO", "PONO");

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

