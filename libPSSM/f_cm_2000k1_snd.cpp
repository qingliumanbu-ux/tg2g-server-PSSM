/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhengqiangqiang
Version:    1.0
Date:     2018-7-27
Description：自动热送开始
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




#include "epex.h"

/*<remark>=========================================================
/// <summary>
/// 发送计划状态信息至MMS
/// <para>1.读取传入的厂别区分、制造命令号、制造命令状态</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：由计划编制，计划删除调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="pono_status">制造命令状态          </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_cm_2000k1_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;
	int v_slab_num = 0;

	CString lpsz_tc_no = " ";
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");


	EPEX epex(&s, conn);

	CString sqlstr;

	/*实体类定义*/
	CModel tpssm11("TPSSM11");
	CModel tpssm10("TPSSM10");
	CModel tpssm03("TPSSM03");
	CModel tpsbws1("TPSBWS1");

	CDbCommand cmd_inq(conn);

	try
	{
		/* ***** 获取输入参数 ***** */
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString();

		/*if (bcls_rec->Tables[0].Columns.Contains("HEAT_NO"))
		{
		tpssm11["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString();
		}*/


		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "factory_div=[{0}]", tpssm11["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "pono=[{0}]", tpssm11["PONO"].ToString());


		/* ***** 检查输入参数合法性 ***** */
		if (tpssm11.Query("PONO,FACTORY_DIV") == false)
		{
			CFormattable arguments[] = { tpssm11["PONO"].ToString(), tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "查不到制造命令[{0}],熔炼号[{1}]，请检查数据。", arguments, 2); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}


		/* ***** 程序处理 ***** */
		if (tpssm11["RUN_STATUS"].ToString().Trim() == "52") //开浇
		{
			tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm10["PONO"] = tpssm11["PONO"];

			if (tpssm10.Query("PONO,FACTORY_DIV") == false)
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString(), tpssm11["HEAT_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "查不到制造命令[{0}],熔炼号[{1}]，请检查数据。", arguments, 2); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (tpssm10["HOT_CHARGE_FLAG"].ToString().Trim() == "2" || tpssm10["HOT_CHARGE_FLAG"].ToString().Trim() == "5")
			{
				tpsbws1["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpsbws1["PONO"] = tpssm11["PONO"];

				tpsbws1.Delete("PONO");
				tpsbws1.Delete("HEAT_NO");
				tpsbws1.CopyFrom(tpssm10);
				tpsbws1.CopyFrom(tpssm11);

				//tpsbws1["MAT_NUM"] = tpssm10.SLAB_SUM;//理论支数
				tpssm03["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm03["PONO"] = tpssm11["PONO"];
				int dummy = tpssm03.QueryCount("FACTORY_DIV,PONO");

				Log::Info("", __FUNCTION__, "begin....理论支数=[{0}]", dummy);

				tpsbws1["MAT_NUM"] = dummy;//理论支数
				tpsbws1["REC_CREATOR"] = s.userid;
				tpsbws1["REC_CREATE_TIME"] = dateNow;
				tpsbws1["REC_REVISE_TIME"] = "";
				tpsbws1["REC_REVISOR"] = "";
				//tpsbws1["LADLE_ARRIVE_TIME"] = tpssm10["HOT_CHARGE_FLAG"];
				tpsbws1["HOT_CHARGE_FLAG"] = tpssm10["HOT_CHARGE_FLAG"];

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库

				default: // 所有数据库适用，通用SQL语句
					sqlstr = " select distinct c.ingot_code, c.prec_roll_plan_no AS PLAN_NO_CON, substr(c.prec_roll_plan_no, 1, 4) plan_backlog_code, c.SLAB_WT  "
						" FROM tpssm03 c																												    "
						" WHERE PONO = @pono																										"
						" and prec_roll_plan_no<>' '																									   "
						" fetch first 1 rows only																										   ";
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("pono", tpssm11["PONO"].ToString());
				cmd_inq.ExecuteReader();
				Log::Trace("", "", "sql={0}", sqlstr);
				if (cmd_inq.Read())
				{

					tpsbws1["INGOT_CODE"] = cmd_inq.GetString(1).Trim();
					tpsbws1["PLAN_NO_CON"] = cmd_inq.GetString(2).Trim();
					//tpsbws1["PLAN_BACKLOG_CODE"] = cmd_inq.GetString(3).Trim();
					tpsbws1["SLAB_WT"] = cmd_inq.GetDecimal(4);
				}
				cmd_inq.Close();

				Log::Info("", __FUNCTION__, "测试用---");
				//tpsbws1.Print();


				tpsbws1.Insert();

				tpsbws1["COMPANY_CODE"] = "S";
				tpsbws1.Update("COMPANY_CODE", "HEAT_NO");

				//----开始发送电文
				lpsz_tc_no = "2000K1";

				if (epex.Initialize(lpsz_tc_no) < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				if (epex.SetValue(0, tpsbws1) < 0)
				{
					sprintf(s.msg, epex.GetMsg());
					sprintf(s.sysmsg, epex.GetMsg());
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//发送电文
				if (epex.SendTele() < 0)
				{
					Log::Debug("", __FUNCTION__, "发送电文失败:{0}", epex.GetMsg());
					strcpy(s.msg, _RES("GCRSS0000032")/*电文发送失败。*/);
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				epex.Uninitialize();
			}

		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
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
