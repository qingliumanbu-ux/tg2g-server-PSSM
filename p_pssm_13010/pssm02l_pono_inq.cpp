/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   周平
Version:    1.0
Date:     2015-11-7 17:13:56
Description: 炼钢计划LOT信息查询
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
/// <para>主调用函数：前台PSSM01画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm02l_pono_inq)

int f_pssm02l_pono_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	CString sqlstr = "";
	int fetchRowCount = 0;
	int i = 0;
	int slab_sum_l = 0;

	CString heat_no = "";	
	CString v_lack_per = ""; //顺序号
	CString function_id = "PSSM02L_PONO";
	CString ischeck = "";

	//系统的分页类信息。
	CPageInfo pageInfo;

	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tqmts0x("TQMTS0X");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm03_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm02_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_tpssm11_inq(conn);  //与DB 建立连接。

	try
	{
		//--------------------------------
		//获取传入参数
		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm01["CAST_LOT_NO"] = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO"];
		v_lack_per = bcls_rec->Tables[0].Rows[0]["LACK_PER"];
		tpssm01["PLAN_DATE"] = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"];
		tpssm01["CC_MACH_NO"] = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"];		
		ischeck = bcls_rec->Tables[0].Rows[0]["IS_CHECK"];

		/* ***** 打印输入参数 ***** */
		////Log::Info("", __FUNCTION__, "pssm02l_pono_inq>FACTORY_DIV = [{0}]", tpssm01["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "pssm02l_pono_inq>CAST_LOT_NO = [{0}]", tpssm01["CAST_LOT_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pssm02l_pono_inq>v_lack_per = [{0}]", v_lack_per);
		////Log::Info("", __FUNCTION__, "pssm02l_pono_inq>CC_MACH_NO = [{0}]", tpssm01["CC_MACH_NO"].ToString());
		////Log::Info("", __FUNCTION__, "pssm02l_pono_inq>PLAN_DATE = [{0}]", tpssm01["PLAN_DATE"].ToString());
		////Log::Info("", __FUNCTION__, "pssm02l_lot_inq>ischeck = [{0}]", ischeck);

		i = v_lack_per.Find("-");
		v_lack_per = v_lack_per.Substring(0, i);
		////Log::Info("", __FUNCTION__, "v_lack_per = [{0}]", v_lack_per);

		//如果前台传了FUNCTION_ID，后台直接调用就可以
		if (bcls_rec->Tables[0].Columns.IndexOf("function_id") < 0)
		{
			bcls_rec->Tables[0].Columns.Add(DT_STRING, "FUNCTION_ID");
			bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"] = function_id;
		}
		else
		{
			function_id = bcls_rec->Tables[0].Rows[0]["FUNCTION_ID"];
		}
		////Log::Trace("", __FUNCTION__, "===function_id =  [{0}]", function_id);

		if (bcls_ret->Tables.get_Count() < 1)
		{
			//在bcls_ret 中增加一个块，放查询结果
			bcls_ret->Tables.Add();
		}
		f_edsetcustominfo(bcls_rec, bcls_ret);	

		if (tpssm01["PLAN_DATE"].ToString().Trim() == "")
		{
			//取计划日期
			sqlstr = " SELECT DISTINCT PLAN_DATE FROM TPSSM01 WHERE CAST_LOT_NO = @CAST_LOT_NO AND FACTORY_DIV = @FACTORY_DIV";
			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm01_inq.ExecuteReader();
			if (cmd_tpssm01_inq.Read())
			{
				tpssm01["PLAN_DATE"] = cmd_tpssm01_inq.GetString(1);
			}
			cmd_tpssm01_inq.Close();
		}		

		sqlstr = "SELECT * "
			" FROM TPSSM01 "
			" WHERE PLAN_DATE = @PLAN_DATE "
			" AND CC_MACH_NO = @CC_MACH_NO "
			" AND FACTORY_DIV = @FACTORY_DIV ";
		if (ischeck == "1")
		{
			sqlstr += " AND CAST_LOT_NO IN ( SELECT CAST_LOT_NO  FROM TPSSM02  WHERE LACK_PER = @LACK_PER  AND LOT_STATUS >= 8 ) ";
		}
		else
		{
			sqlstr += " AND CAST_LOT_NO IN ( SELECT CAST_LOT_NO  FROM TPSSM02  WHERE LACK_PER = @LACK_PER  AND LOT_STATUS < 8 ) ";
		}
		sqlstr += " ORDER BY CC_SEQ, CAST_LOT_NO ,CAST_LOT_DIV_NO ASC ";

		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.Parameters.Set("PLAN_DATE", tpssm01["PLAN_DATE"].ToString());
		cmd_tpssm01_inq.Parameters.Set("CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
		cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
		cmd_tpssm01_inq.Parameters.Set("LACK_PER", v_lack_per);	

		cmd_tpssm01_inq.ExecuteReader();
		while (cmd_tpssm01_inq.Read())
		{
			
			cmd_tpssm01_inq.Fetch(tpssm01);
			tpssm01.TrimOrBlank();

			tpssm01.MergeTo(bcls_ret->Tables[0]);

			slab_sum_l = 0;
			//查询炉次未产出支数
			if (tpssm01["CC_MACH_NO"].ToString() == "1" || tpssm01["CC_MACH_NO"].ToString() == "4") //板坯
			{
				sqlstr = "select nvl(count(distinct lslab_no),0) "
					" from tpssm03 t "
					" where t.factory_div = @factory_div "
					"   and t.pono = @pono "
					" and t.slab_prod_flag <> '1' "
					" group by lslab_no ";
			}
			else //方坯
			{
				sqlstr = "select nvl(count(lslab_no),0) "
					" from tpssm03 t "
					" where t.factory_div = @factory_div "
					"   and t.pono = @pono "
					"   and t.slab_prod_flag <> '1' "
					" group by lslab_no ";
			}			

			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();
			if (cmd_tpssm03_inq.Read())
			{
				slab_sum_l = cmd_tpssm03_inq.GetInt32(1);
			}
			cmd_tpssm03_inq.Close();

			bcls_ret->Tables[0].Rows[fetchRowCount]["SLAB_SUM_L"] = slab_sum_l;

			fetchRowCount++;
		}
		cmd_tpssm01_inq.Close();
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
