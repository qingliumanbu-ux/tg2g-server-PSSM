/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-03-01 17:13:56
Description: 炉次确认画面-实绩信息查询
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/




/* ***** 静态函数申明 ***** */


/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 炉次确认画面-实绩信息查询
/// <para>
/// 1.根据pono进行查询。
///
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)          </para>
/// <para>主调用函数：前台PSSM91画面F2(查询)按钮         </para>
/// </summary>
/// <param name="">                      </param>
/// <param name="">                    </param>
/// <returns> 制造命令表 </returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm91f2_inq2)

int f_pssm91f2_inq2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int i = 0;
	int count = 0;
	int cut_num = 0; //切断数
	int prcut_num = 0;//实绩切断数
	CString sqlstr = "";
	CString dev_code = "";
	CDecimal area_id = 0;
	CString tablename = "";
	CString proc_no = "";
	CString pract_rcv_flag = "";
	CString history_flag = "";
	CModel tpssm11("TPSSM11");
	CModel tpssm03("TPSSM03");
	CModel tpssm41("TPSSM41");
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tmmsm_inq(conn);

	try
	{
		//设置返回块
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO"); //制造命令号
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DEV_CODE"); //设备代码 
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PROC_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRACT"); //实绩
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CUT_NUM"); //切断数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PRCUT_NUM"); //实绩切断数

		//--------------------------------
		//获取传入参数
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"];
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		history_flag = bcls_rec->Tables[1].Rows[0]["HISTORY_FLAG"].ToString();
		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "pssm91f2_inq2>PONO =[{0}]", tpssm11["PONO"].ToString());
		Log::Info("", __FUNCTION__, "pssm91f2_inq2>FACTORY_DIV =[{0}]", tpssm11["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "history_flag =[{0}]", history_flag);
		if (history_flag == "false" || history_flag == "0")
		{
			sqlstr = "tpssm11.Query()";
			if (!tpssm11.Query("PONO,FACTORY_DIV"))
			{
				strcpy(s.msg, "select tpssm11 failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//查询设备代码
			sqlstr = "select dev_code,area_id,proc_no,pract_rcv_flag from tpssm12 where sm_plan_no = @sm_plan_no and factory_div = @factory_div ";

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			while (cmd_tpssm12_inq.Read())
			{
				dev_code = cmd_tpssm12_inq.GetString(1);
				area_id = cmd_tpssm12_inq.GetDecimal(2);
				proc_no = cmd_tpssm12_inq.GetString(3);
				pract_rcv_flag = cmd_tpssm12_inq.GetString(4);

				Log::Info("", __FUNCTION__, "dev_code = [{0}] ,area_id = [{1}]", dev_code, area_id);

				count = atol((const char*)pract_rcv_flag.Trim());

				//查询是否有实绩
				if (area_id == 2) //BOF
				{
					//tablename = "tmmsm21";

					//sqlstr = "select count(*) from " + tablename + " where proc_no = @proc_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("proc_no", proc_no);
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 3) //BOF
				{
					//if (dev_code.Substring(0,1) == "E")
					//{
					//	tablename = "tmmsm20";
					//}
					//else
					//{
					//	tablename = "tmmsm21";
					//}
					//
					//sqlstr = "select count(*) from " + tablename + " where heat_no = @heat_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm11["HEAT_NO"].ToString());
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 4 && dev_code.Substring(0, 1) == "L") // LF
				{
					//tablename = "tmmsm24";

					//sqlstr = "select count(*) from " + tablename + " where proc_no = @proc_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("proc_no", proc_no);
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 4 && dev_code.Substring(0, 1) == "R") //RH
				{
					//tablename = "tmmsm23";

					//sqlstr = "select count(*) from " + tablename + " where proc_no = @proc_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("proc_no", proc_no);
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 5) //CC
				{
					//if (dev_code.Substring(0, 1) == "C")
					//{
					//	tablename = "tmmsm31";
					//}
					//else
					//{
					//	tablename = "tmmsm41";
					//}

					//sqlstr = "select count(*) from " + tablename + " where heat_no = @heat_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm11["HEAT_NO"].ToString());
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();

					//连铸时查询切断数，实绩切断数
					tpssm03["PONO"] = tpssm11["PONO"];
					tpssm03["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
					cut_num = tpssm03.QueryCount("PONO,FACTORY_DIV");

					Log::Info("", __FUNCTION__, "cut_num =[{0}]", cut_num);

					sqlstr = "select count(*) from tmmsm33 where heat_no = @heat_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm11["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						prcut_num = cmd_tmmsm_inq.GetInt32(1);

						Log::Info("", __FUNCTION__, "prcut_num =[{0}]", prcut_num);
					}
					cmd_tmmsm_inq.Close();
				}

				//返回块值
				bcls_ret->Tables[0].Rows.Add();
				bcls_ret->Tables[0].Rows[i]["PONO"] = tpssm11["PONO"];
				bcls_ret->Tables[0].Rows[i]["DEV_CODE"] = dev_code;
				bcls_ret->Tables[0].Rows[i]["PROC_NO"] = proc_no;
				bcls_ret->Tables[0].Rows[i]["PRACT"] = count;
				bcls_ret->Tables[0].Rows[i]["CUT_NUM"] = cut_num;
				bcls_ret->Tables[0].Rows[i]["PRCUT_NUM"] = prcut_num;

				i++;

			}
			cmd_tpssm12_inq.Close();
		}
		else
		{
			tpssm41["PONO"] = tpssm11["PONO"].ToString();
			tpssm41["FACTORY_DIV"] = tpssm11["FACTORY_DIV"].ToString();
			sqlstr = "tpssm11.Query()";
			if (!tpssm41.Query("PONO,FACTORY_DIV"))
			{
				strcpy(s.msg, "select tpssm41 failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//查询设备代码
			sqlstr = "select dev_code,area_id,proc_no,pract_rcv_flag from tpssm42 where sm_plan_no = @sm_plan_no and factory_div = @factory_div ";

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm41["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm41["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.ExecuteReader();
			while (cmd_tpssm12_inq.Read())
			{
				dev_code = cmd_tpssm12_inq.GetString(1);
				area_id = cmd_tpssm12_inq.GetDecimal(2);
				proc_no = cmd_tpssm12_inq.GetString(3);
				pract_rcv_flag = cmd_tpssm12_inq.GetString(4);

				Log::Info("", __FUNCTION__, "dev_code = [{0}] ,area_id = [{1}]", dev_code, area_id);

				count = atol((const char*)pract_rcv_flag.Trim());

				//查询是否有实绩
				if (area_id == 2) //BOF
				{
					//tablename = "tmmsm21";

					//sqlstr = "select count(*) from " + tablename + " where proc_no = @proc_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("proc_no", proc_no);
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 3) //BOF
				{
					//if (dev_code.Substring(0,1) == "E")
					//{
					//	tablename = "tmmsm20";
					//}
					//else
					//{
					//	tablename = "tmmsm21";
					//}
					//
					//sqlstr = "select count(*) from " + tablename + " where heat_no = @heat_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm11["HEAT_NO"].ToString());
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 4 && dev_code.Substring(0, 1) == "L") // LF
				{
					//tablename = "tmmsm24";

					//sqlstr = "select count(*) from " + tablename + " where proc_no = @proc_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("proc_no", proc_no);
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 4 && dev_code.Substring(0, 1) == "R") //RH
				{
					//tablename = "tmmsm23";

					//sqlstr = "select count(*) from " + tablename + " where proc_no = @proc_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("proc_no", proc_no);
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();
				}
				else if (area_id == 5) //CC
				{
					//if (dev_code.Substring(0, 1) == "C")
					//{
					//	tablename = "tmmsm31";
					//}
					//else
					//{
					//	tablename = "tmmsm41";
					//}

					//sqlstr = "select count(*) from " + tablename + " where heat_no = @heat_no ";

					//cmd_tmmsm_inq.SetCommandText(sqlstr);
					//cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm11["HEAT_NO"].ToString());
					//cmd_tmmsm_inq.ExecuteReader();
					//if (cmd_tmmsm_inq.Read())
					//{
					//	count = cmd_tmmsm_inq.GetInt32(1);
					//}
					//cmd_tmmsm_inq.Close();

					//连铸时查询切断数，实绩切断数
					tpssm03["PONO"] = tpssm41["PONO"];
					tpssm03["FACTORY_DIV"] = tpssm41["FACTORY_DIV"];
					cut_num = tpssm03.QueryCount("PONO,FACTORY_DIV");

					Log::Info("", __FUNCTION__, "cut_num =[{0}]", cut_num);

					sqlstr = "select count(*) from tmmsm33 where heat_no = @heat_no ";

					cmd_tmmsm_inq.SetCommandText(sqlstr);
					cmd_tmmsm_inq.Parameters.Set("heat_no", tpssm41["HEAT_NO"].ToString());
					cmd_tmmsm_inq.ExecuteReader();
					if (cmd_tmmsm_inq.Read())
					{
						prcut_num = cmd_tmmsm_inq.GetInt32(1);

						Log::Info("", __FUNCTION__, "prcut_num =[{0}]", prcut_num);
					}
					cmd_tmmsm_inq.Close();
				}

				//返回块值
				bcls_ret->Tables[0].Rows.Add();
				bcls_ret->Tables[0].Rows[i]["PONO"] = tpssm41["PONO"];
				bcls_ret->Tables[0].Rows[i]["DEV_CODE"] = dev_code;
				bcls_ret->Tables[0].Rows[i]["PROC_NO"] = proc_no;
				bcls_ret->Tables[0].Rows[i]["PRACT"] = count;
				bcls_ret->Tables[0].Rows[i]["CUT_NUM"] = cut_num;
				bcls_ret->Tables[0].Rows[i]["PRCUT_NUM"] = prcut_num;

				i++;

			}
			cmd_tpssm12_inq.Close();
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
