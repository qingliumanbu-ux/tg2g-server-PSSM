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

//int f_cm_pxqs05_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);
//int f_cm_pxqs06_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);
//int f_cm_pxyx01_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);
//int f_cm_pxcx01_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

// service入口
BM2F_ENTERACE(pssm14f3_proc)

int f_pssm14f3_proc(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	/* ***** 自定义变量 ***** */
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	CString cast_number = "";
	CString strand_no = "";
	int		TotalRecordCount = 0  ;
	int		seq = 0  ;
	int		ret = 0  ;

		

	//系统的分页类信息。
	CPageInfo pageInfo; 
 
	CModel tpssm03("TPSSM03");
	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	EIClass iblk_qs_hp;
	EIClass iblk_qs_hr;
	EIClass iblk_yx;

	try
	{
		//获取传入参数
		tpssm11["CAST_NO"] = bcls_rec->Tables[1].Rows[0]["CAST_NO"].ToString();
		tpssm11["CAST_DIV_NO"] = 1;
		tpssm11["STEEL_RETURN_CODE"] = " ";
		tpssm11.Query("CAST_NO,CAST_DIV_NO,STEEL_RETURN_CODE");
		if (tpssm11["RUN_STATUS"].ToString().Trim() == "")
		{
			tpssm41["CAST_NO"] = tpssm11["CAST_NO"];
			tpssm41["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"];
			tpssm41["STEEL_RETURN_CODE"] = tpssm11["STEEL_RETURN_CODE"];
			tpssm41.Query("CAST_NO,CAST_DIV_NO,STEEL_RETURN_CODE");
			tpssm11["RUN_STATUS"] = tpssm41["RUN_STATUS"];
			tpssm11["FACTORY_DIV"] = tpssm41["FACTORY_DIV"];
		}
		//if (tpssm11["RUN_STATUS"].ToString() >= "52" || tpssm11["RUN_STATUS"].ToString().Trim() == "")
		//{
		//	CFormattable arguments[] = { tpssm11["CAST_NO"].ToString(), s.msg }; // 定义参数列表的数组
		//	CMessageFormat::Format(s.msg, "浇次号[{0}]首炉已开浇，无法调整铸流", arguments, 1);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm03.MergeFrom(bcls_rec->Tables[0].Rows[i]);
				if (strand_no != bcls_rec->Tables[0].Rows[i]["STRAND_NO"].ToString()) 
					seq = 1;
				else 
					seq = seq + 1;

			//tpssm03.SEQUENCE_NUMBER = seq;
			//tpssm03.STATUS_FLAG = "0";
			//tpssm03.Update("SEQUENCE_NUMBER, STATUS_FLAG", "SLAB_NO");

			strand_no = bcls_rec->Tables[0].Rows[i]["STRAND_NO"].ToString();

			if (tpssm03.QueryCount("SLAB_NO,STRAND_NO") == 0)
			{
				tpssm03.Update("STRAND_NO", "SLAB_NO");
			}
		}

		if (tpssm11["RUN_STATUS"].ToString().Trim() >= "36")
		{
			Log::Trace("", __FUNCTION__, "重发电文");

			if (tpssm11["FACTORY_DIV"].ToString() != "A32")
			{
				CString sqlstr_hp =
					" SELECT '1' FLG, A.CAST_NO, A.CC_MACH_NO, A.PONO"
					" FROM TPSSM11 A"
					" JOIN TPSSM01 B ON A.PONO = B.PONO AND ((B.SLAB_DEST BETWEEN '21' AND '25') OR (B.SLAB_DEST = '90' AND B.LINE_TYPE = 'HP'))"
					" WHERE A.CAST_NO = @CAST_NO"
					" ORDER BY A.CAST_DIV_NO"
					;

				CString sqlstr_hr =
					" SELECT '1' FLG, A.CAST_NO, A.CC_MACH_NO, B.CAST_LOT_NO"
					" FROM TPSSM11 A"
					" JOIN TPSSM01 B ON A.PONO = B.PONO AND (B.SLAB_DEST < '21' OR B.SLAB_DEST > '25') AND NOT (B.SLAB_DEST = '90' AND B.LINE_TYPE = 'HP')"
					" WHERE A.CAST_NO = @CAST_NO"
					" GROUP BY A.CAST_NO, A.CC_MACH_NO, B.CAST_LOT_NO"
					" ORDER BY B.CAST_LOT_NO"
					;

				cmd_inq1.SetCommandText(sqlstr_hp);
				cmd_inq1.Parameters.Set("CAST_NO", tpssm11["CAST_NO"].ToString());
				cmd_inq1.ExecuteQuery(iblk_qs_hp.Tables[0]);
				cmd_inq1.Close();

				cmd_inq1.SetCommandText(sqlstr_hr);
				cmd_inq1.Parameters.Set("CAST_NO", tpssm11["CAST_NO"].ToString());
				cmd_inq1.ExecuteQuery(iblk_qs_hr.Tables[0]);
				cmd_inq1.Close();

				//ret = f_cm_pxqs05_snd(&iblk_qs_hp, bcls_ret, conn);
				//if (ret < 0)
				//	throw CApplicationException(-1, s.msg, log.Location);

				//ret = f_cm_pxqs06_snd(&iblk_qs_hr, bcls_ret, conn);
				//if (ret < 0)
				//	throw CApplicationException(-1, s.msg, log.Location);
			}
			else
			{
				CString sqlstr_yx =
					" SELECT '1' FLG, A.CC_MACH_NO, B.CAST_LOT_NO"
					" FROM TPSSM11 A"
					" JOIN TPSSM10 B ON A.PONO = B.PONO"
					" WHERE A.CAST_NO = @CAST_NO"
					" GROUP BY A.CAST_NO, A.CC_MACH_NO, B.CAST_LOT_NO"
					" ORDER BY B.CAST_LOT_NO"
					;
				cmd_inq.SetCommandText(sqlstr_yx);
				cmd_inq.Parameters.Set("CAST_NO", tpssm11["CAST_NO"].ToString());
				cmd_inq.ExecuteQuery(iblk_yx.Tables[0]);
				cmd_inq.Close();

				//ret = f_cm_pxyx01_snd(&iblk_yx, bcls_ret, conn);
				//if (ret < 0)
				//	throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		if (tpssm11["RUN_STATUS"].ToString().Trim() >= "30")
		{
			EIClass iblk_cc;
			CString oper_flg = "1";
			if (tpssm11["RUN_STATUS"].ToString() < "52" && (tpssm11["FACTORY_DIV"].ToString() == "A34" || tpssm11["FACTORY_DIV"].ToString() == "A35")) oper_flg = "5";
			if (tpssm11["RUN_STATUS"].ToString() < "52" && tpssm11["FACTORY_DIV"].ToString() == "A31") oper_flg = "5";
			CString sqlstr_cc =
				" SELECT '" + oper_flg + "' FLG, A.CAST_NO, A.CC_MACH_NO, A.CAST_NUMBER, B.CAST_LOT_NO,"
				" (LISTAGG(B.CAST_LOT_DIV_NO,',') WITHIN GROUP (ORDER BY B.CAST_LOT_DIV_NO)) CAST_LOT_DIV_NO"
				" FROM TPSSM11 A"
				" LEFT JOIN TPSSM10 B ON A.PONO = B.PONO"
				" WHERE A.CAST_NO = @CAST_NO"
				" GROUP BY A.CAST_NO, A.CC_MACH_NO, A.CAST_NUMBER, B.CAST_LOT_NO"
				" ORDER BY A.CAST_NO, A.CC_MACH_NO, A.CAST_NUMBER, B.CAST_LOT_NO"
				;
			cmd_inq.SetCommandText(sqlstr_cc);
			cmd_inq.Parameters.Set("CAST_NO", tpssm11["CAST_NO"].ToString());
			cmd_inq.ExecuteQuery(iblk_cc.Tables[0]);
			cmd_inq.Close();

			if (tpssm11["RUN_STATUS"].ToString() < "52" && (tpssm11["FACTORY_DIV"].ToString() == "A34" || tpssm11["FACTORY_DIV"].ToString() == "A35"))
			{
				int count = iblk_cc.Tables[0].Rows.get_Count();
				for (int i = 0; i < count; i++)
				{
					CDataRow& row = iblk_cc.Tables[0].Rows.Add();
					row.Merge(iblk_cc.Tables[0].Rows[i]);
					row["FLG"] = "1";
				}
			}
			if (tpssm11["RUN_STATUS"].ToString() < "52" && tpssm11["FACTORY_DIV"].ToString() == "A31")
			{
				int count = iblk_cc.Tables[0].Rows.get_Count();
				for (int i = 0; i < count; i++)
				{
					CDataRow& row = iblk_cc.Tables[0].Rows.Add();
					row.Merge(iblk_cc.Tables[0].Rows[i]);
					row["FLG"] = "1";
				}
			}

			//ret = f_cm_pxcx01_snd(&iblk_cc, bcls_ret, conn);
			//if (ret < 0)
			//	throw CApplicationException(-1, s.msg, log.Location);
		}
		//else
		//{
		//	Log::Trace("", __FUNCTION__, "不发电文");
		//}


		//for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		//{
		//	tpssm03.MergeFrom(bcls_rec->Tables[0].Rows[i]);
		//	if (cast_number != bcls_rec->Tables[0].Rows[i]["CAST_NUMBER"].ToString()) seq = 1;
		//	if (strand_no != bcls_rec->Tables[0].Rows[i]["STRAND_NO"].ToString()) seq = 1;
		//	else seq = seq + 1;
		//	tpssm03.SEQUENCE_NUMBER = seq;
		//	tpssm03.STATUS_FLAG = "0";
		//	tpssm03.Update("SEQUENCE_NUMBER, STATUS_FLAG", "SLAB_NO");

		//	cast_number = bcls_rec->Tables[0].Rows[i]["CAST_NUMBER"].ToString();
		//	strand_no = bcls_rec->Tables[0].Rows[i]["STRAND_NO"].ToString();
		//	if (tpssm03.QueryCount("SLAB_NO,STRAND_NO") == 0)
		//	{
		//		tpssm03.Update("STRAND_NO", "SLAB_NO");
		//	}
		//}
		
		cmd_inq.Close();
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
