/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-26
Version:  3.1.0
Description: 甘特图出钢计划编制保存
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
int f_pssm_castlot_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
/*<remark>=========================================================
/// <summary>
/// 甘特图出钢计划编制保存
/// <para>根据Client甘特图输入的数据，修改出钢计划。  </para>
/// <para>
///   1.出钢计划信息写入:主计划与工序计划
///   2.删除排除的炉次
///   3.CAST号计算
///   4.计划号计算
///   5.处理号计算
/// <para>数据库表：TPSSM11/12                   </para>
/// <para>主调用函数：PSSM18画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>制造命令号</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_pssm10)

int f_pssm18_pssm10(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i = 0;

	/* 业务变量 */
	CString v_factory_div = "";	//炼钢单元号
	CString v_pono = "";
	CString v_cc_req_time = "";  //开浇时刻
	CString v_tpd_start_time = "";  //倒罐开始时刻
	CDecimal v_td_chg_flg = 0, v_smelt_mode = 0;
	CDecimal charge_no = 0;      //工序charge号
	CDecimal area_id = 0;
	CString datetime = "";
	CString routebagkey = "";
	CString routelist = "";
	CString backlog_ea = "";
	CString cc_mach_no = "";
	CString restrand_flg = "";
	CString refine_div = "";
	CString cast_lot_no2 = "";
	CDecimal cast_lot_div_no2 = 0;

	EIClass inblock;
	EIClass outblock;
	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);
	CDbCommand cmd_tpssm18_del(conn);
	CDbCommand cmd_tpssm11_inq(conn);

	CDataTable tb_tpssm11("TPSSM11");
	
	inblock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO2");  //炼钢单元号
	inblock.Tables[0].Rows.Add();

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		CModel tpssm10("TPSSM10");
		CModel tpssm11("TPSSM11");
		CModel tpssm12("TPSSM12");
		CModel tpssm12_ds("TPSSM12");//脱硫
		CModel tpssm12_sd("TPSSM12");//预溶液
		CModel tpssm12_dp("TPSSM12");//转炉脱磷
		CModel tpssm12_bof("TPSSM12");//转炉/电炉工序
		CModel tpssm12_sr("TPSSM12");//精炼
		CModel tpssm12_cc("TPSSM12");//连铸
		CModel tpssm18("TPSSM18");//设备状态
		CModel tpssm99("TPSSM99");
		CModel tpssmd1("TPSSMD1");
		CModel tpssmd6("TPSSMD6");
		//--------------------------------
		/* ***** 获取输入参数 ***** */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rows; i++)
		{
			v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();
			/* ***** 检查输入参数合法性 ***** */
			if (v_pono.GetLength() <= 0)
			{
				doFlag = -11;
				//sprintf(s.msg, "收到的PONO号[%s]长度有误！",(const char*)tpssm11["PONO"].ToString());
				CFormattable arguments[] = { v_pono }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm10["PONO"] = v_pono;
			tpssm10["FACTORY_DIV"] = "LG1";
			tpssm10.Query("PONO,FACTORY_DIV");

			if (bcls_rec->Tables[0].Columns.Contains("ROUTEBAGKEY"))
			{
				routebagkey = bcls_rec->Tables[0].Rows[i]["ROUTEBAGKEY"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("ROUTELIST"))
			{
				routelist = bcls_rec->Tables[0].Rows[i]["ROUTELIST"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("CC_MACH_NO"))
			{
				cc_mach_no = bcls_rec->Tables[0].Rows[i]["CC_MACH_NO"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("RESTRAND_FLG"))
			{
				restrand_flg = bcls_rec->Tables[0].Rows[i]["RESTRAND_FLG"].ToString().Trim();
				if (restrand_flg == "0")
				{
					restrand_flg = " ";
				}
				else if (restrand_flg == "1")
				{
					restrand_flg = "T";
				}
			}
			if (bcls_rec->Tables[0].Columns.Contains("REFINE_DIV"))
			{
				refine_div = bcls_rec->Tables[0].Rows[i]["REFINE_DIV"].ToString().Trim();
			}
			if (bcls_rec->Tables[0].Columns.Contains("CAST_LOT_NO2"))
			{
				cast_lot_no2 = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO2"].ToString().Trim();
				Log::Trace("", __FUNCTION__, "cast_lot_no2=[{0}]", cast_lot_no2);
			}
			if (bcls_rec->Tables[0].Columns.Contains("CAST_LOT_DIV_NO2"))
			{
				cast_lot_div_no2 = bcls_rec->Tables[0].Rows[i]["CAST_LOT_DIV_NO2"].ToDecimal();
				Log::Trace("", __FUNCTION__, "cast_lot_div_no2=[{0}]", cast_lot_div_no2);
			}
			
			//-------------------------------------------------------------------
			//
			tpssm10["ROUTELIST"] = routelist;
			tpssm10["CC_MACH_NO"] = cc_mach_no;
			tpssm10["RESTRAND_FLG"] = restrand_flg;
			tpssm10["REFINE_DIV"] = refine_div;
			//tpssm10["CAST_LOT_NO2"] = cast_lot_no2;
			//tpssm10["CAST_LOT_DIV_NO2"] = cast_lot_div_no2;
			tpssm10.Update("ROUTELIST,CC_MACH_NO,RESTRAND_FLG,REFINE_DIV", "PONO,FACTORY_DIV");//,CAST_LOT_NO2,CAST_LOT_DIV_NO2

		}

		//rows = bcls_rec->Tables["CAST"].Rows.get_Count();
		//cast_lot_no2 = "";
		//for (int i = 0; i < rows; i++)
		//{
		//	if (cast_lot_no2.Trim() == "")
		//	{
		//		cast_lot_no2 = bcls_rec->Tables["CAST"].Rows[i]["CAST_LOT_NO2"].ToString().Trim();
		//	}
		//	else cast_lot_no2 = cast_lot_no2 + "," + bcls_rec->Tables["CAST"].Rows[i]["CAST_LOT_NO2"].ToString().Trim();
		//}
		//Log::Info("", __FUNCTION__, "cast_lot_no2=[{0}]", cast_lot_no2);
		//if (cast_lot_no2.Trim() != "")
		//{
		//	inblock.Tables[0].Rows[0]["CAST_LOT_NO2"] = cast_lot_no2;
		//	//ret = f_pssm_castlot_upd(&inblock, bcls_ret, conn);
		//	if (ret < 0)
		//	{
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}

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
