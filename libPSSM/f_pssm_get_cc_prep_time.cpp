/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    xuwen
Version:   3.1.0
Date:      2014-12-12
Description: 获取连铸炉次间准备时间
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/*<remark>=========================================================
/// <summary>
/// 获取连铸炉次间准备时间（单记录）
/// <para>处理内容：根据连铸标准时间(TPSSMC1)，计算炉次间准备时间     </para>
/// <para>数据库表：TPSSMC3(浇铸顺序表)                  </para>
/// <para>主调用函数：pssm12a_upd 调用。               </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_get_cc_prep_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;

	bool hasc1 = false;

	CModel tpssmd9("TPSSMD9");
	CModel tpssm10("TPSSM10");

	CString sqlstr;
	CDbCommand cmd_upd(conn);


	try
	{

		//-------------------------------------------------------
		//定义返回参数
		blkseq = bcls_ret->Tables.IndexOf("PREP_TIME");
		if (blkseq < 0)
		{
			bcls_ret->Tables.Add("PREP_TIME");
			bcls_ret->Tables["PREP_TIME"].Columns.Add(DT_DECIMAL, "CC_PREP_TIME");//炉次间准备时间
			bcls_ret->Tables["PREP_TIME"].Rows.Add();  //单记录返回
		}


		//--------------------------------------------------------------
		//获得输入参数
		//1. 读取指定的默认设备，存储在dev_code数组中
		blkseq = bcls_rec->Tables.IndexOf("PONO_CAST");
		if (blkseq < 0)
		{
			strcpy(s.msg, "传入炉次信息数据块[PONO_CAST]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PONO_CAST] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			//tpssmd1.DEV_CODE = bcls_rec->Tables[blkseq].Rows[i]["DEV_CODE"];
			tpssm10.MergeFrom(bcls_rec->Tables[blkseq].Rows[i]);

			////Log::Info("", __FUNCTION__, "CC_MACH_NO=[{0}], RESTRAND_FLAG=[{1}], TD_CHG_FLAG=[{2}]",
			//	tpssm10["CC_MACH_NO"].ToString(), tpssm10["RESTRAND_FLG"].ToString(), tpssm10["TD_CHG_FLG"].ToDecimal());

			//读取连铸设备参数表（TPSSMC1），对准备时间做修正
			if (tpssmd9["CC_MACH_NO"].ToString() != tpssm10["CC_MACH_NO"].ToString().Trim()) //没有读取过
			{
				tpssmd9["CC_MACH_NO"] = tpssm10["CC_MACH_NO"].ToString().Trim();

				sqlstr = "tpssmc1.Query()";
				hasc1 = tpssmd9.Query("CC_MACH_NO");
			}


			//设置炉间准备时间，必须与函数 f_pssm10_pour_time, pssm10_strd 中的计算一致
			if (hasc1 == true)
			{
				//重开浇炉次
				if (tpssm10["RESTRAND_FLG"].ToString() == "T")//CAST间, CAST的第一炉
				{
					tpssm10["CC_PREP_TIME"] = tpssmd9["TT_TD_DELAY_CAST"].ToDecimal() + tpssmd9["TT_PREP_W0_CAST"].ToDecimal();    //CCCAST间TD延长时间 + CAST间准备时间(调幅无)
				}
				else  //CAST内
				{

					if (tpssm10["TD_CHG_FLG"].ToDecimal() == 1) //更换中包
					{
						//CC第二炉次直前准备时间 + max(CC第二炉次TD交换时间, 第二炉次直前准备时间) 
						tpssm10["CC_PREP_TIME"] =
							((tpssmd9["TT_TDEX_2CH"].ToDecimal() > tpssmd9["TT_PREP_2CH"].ToDecimal()) ? tpssmd9["TT_TDEX_2CH"].ToDecimal(): tpssmd9["TT_PREP_2CH"].ToDecimal());
					}
					else
					{
						//第二炉次准备时间 + CC第二炉次直前准备时间
						tpssm10["CC_PREP_TIME"] = tpssmd9["TT_PREP_2CH"]; // + tpssmc1.TT_PREP_LAST_2CH
						//////Log::Trace("", __FUNCTION__, "CAST内不换中包, pono=[{0}] prep_time=[{1}]", tpssm10["PONO"].ToString(), tpssm10["CC_PREP_TIME"].ToDecimal());
					}

				}//if

			}
			else //若没找到对应连铸机配置，取默认值
			{
				//重开浇炉次
				if (tpssm10["RESTRAND_FLG"].ToString() == "T")//CAST间, CAST的第一炉
				{
					tpssm10["CC_PREP_TIME"] = 60;
				}
				else  //CAST内
				{
					tpssm10["CC_PREP_TIME"] = 4;
				}
			}


			//返回数据赋值
			bcls_ret->Tables["PREP_TIME"].Rows[0]["CC_PREP_TIME"] = tpssm10["CC_PREP_TIME"];

		}//for



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
