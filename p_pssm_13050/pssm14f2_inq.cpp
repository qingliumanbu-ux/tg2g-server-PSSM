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
BM2F_ENTERACE(pssm14f2_inq)

int f_pssm14f2_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int		TotalRecordCount = 0  ;

		

	//系统的分页类信息。
	CPageInfo pageInfo; 
 
	CModel tpssm11("TPSSM11");
	CModel tpssm03("TPSSM03");

	CDbCommand cmd_inq(conn);

	try
	{
		
		//获取传入参数
		tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		//打印输入参数
		Log::Info("", __FUNCTION__, "==FACTORY_DIV= [{0}]", tpssm11["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "==CC_MACH_NO = [{0}]", tpssm11["CC_MACH_NO"].ToString());


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:
		case DB_KIND_DB2_ORACLE:
		case DB_KIND_MSSQL:
		case DB_KIND_ORACLE:
		default:
			//浇次信息查询SQL语句
			sqlstr_temp = "";
			sqlstr_temp_order = "";
			sqlstr =
				" SELECT A.CC_MACH_NO, A.CAST_NO, A.CAST_PONO_SUM, B.CAST_LOT_NO, MAX(B.SLAB_DEST) SLAB_DEST,"
				/*" CASE WHEN"
				" (MAX(B.SLAB_DEST) >= '21' AND MAX(B.SLAB_DEST) <= '25')"
				" OR (MAX(B.SLAB_DEST) = '90' AND MAX(B.LINE_TYPE) = 'HP')"
				" THEN '厚板模式' ELSE '薄板模式' END AS PLAN_TYPE,"*/
				" DECODE(MAX(C.CC_MACH_NO), NULL, ' ', '浇注中') CUR_POUR,"
				" COUNT(A.SM_PLAN_NO) CAST_PONO_SUM, MAX(B1.LOT_NUM) LOT_NUM,"
				" (LISTAGG(DISTINCT A.ST_NO,'*') WITHIN GROUP (ORDER BY A.ST_NO)) ST_NO,"
				" (LISTAGG(DISTINCT B.SG_SIGN,'*') WITHIN GROUP (ORDER BY B.SG_SIGN)) SG_SIGN,"
				" A.CC_MACH_NO"
				" FROM TPSSM11 A"
				" LEFT JOIN TPSSM26 C ON A.CAST_NO = C.CAST_NO AND A.CAST_DIV_NO = C.CAST_DIV_NO AND A.RUN_STATUS = '52'"
				" LEFT JOIN TPSSM01 B ON A.PONO = B.PONO"
				" LEFT JOIN"
				" (SELECT CAST_LOT_NO, CC_MACH_NO, COUNT(PONO) LOT_NUM FROM TPSSM10 GROUP BY CC_MACH_NO, CAST_LOT_NO"
				" )B1 ON B.CAST_LOT_NO = B1.CAST_LOT_NO AND B1.CC_MACH_NO = A.CC_MACH_NO"
				" WHERE 1 = 1"
				" AND A.RUN_STATUS < '53'"
				" AND A.CAST_NO IN (SELECT CAST_NO FROM TPSSM11 WHERE  PONO_STATUS >= 20)"
				;
			if (tpssm11["FACTORY_DIV"].ToString().Trim() != "")
				sqlstr_temp += " AND A.FACTORY_DIV = @tpssm11.FACTORY_DIV";
			if (tpssm11["CC_MACH_NO"].ToString().Trim() != "")
				sqlstr_temp += " AND A.CC_MACH_NO = @tpssm11.CC_MACH_NO";
					
			sqlstr_temp +=" GROUP BY A.CC_MACH_NO, A.CAST_NO, A.CAST_PONO_SUM, B.CAST_LOT_NO";
			sqlstr_temp_order += " ORDER BY A.CC_MACH_NO, A.CAST_NO, A.CAST_PONO_SUM, B.CAST_LOT_NO";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
			break;
		}

		Log::Info("", __FUNCTION__, "==浇次信息查询 begin...");
		Log::Info("", __FUNCTION__, "===sqlstr=== [{0}]", sqlstr);
		Log::Info("", __FUNCTION__, "===sqlstr_count=== [{0}]", sqlstr_count);

		cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());

		//cmd_inq.SetCommandText(sqlstr_count);
		//TotalRecordCount = cmd_inq.ExecuteScalar().ToInt32();

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);



		cmd_inq.Close();

		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

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
