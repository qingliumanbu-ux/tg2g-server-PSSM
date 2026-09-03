/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-6-28
Description:	 混浇大分类
                 
Update: 
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 调宽标记
/// <para>数据库表： </para>
/// <para>主调用函数: f_pssm11_mix_upd </para>
/// </summary>
/// <param name="i=0">前一炉       </param>
/// <param name="i=1">当前炉       </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm_mix_flag(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	CString intermix_flag_pono;
	CString v_pono, v_pono_pre;
	
	CModel tpssm10("TPSSM10");
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{
		create_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		i = 1;
		//获取传入参数
		v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().TrimOrBlank();         //当前炉pono
		v_pono_pre = bcls_rec->Tables[0].Rows[i-1]["PONO"].ToString().TrimOrBlank(); //上一炉pono

		/* 中包第一炉设置插铁板标记 */
		if (bcls_rec->Tables[0].Rows[i]["TD_CHG_FLG"].ToDecimal() == 1)
		{
			////混浇分类
			//doFlag = f_pssm_mix_type(bcls_rec, bcls_ret);
			//if (doFlag != 0)
			//{
			//	sprintf(s.msg, "调用混浇分类代码判断函数 出错！");
			//	EDLog(1, 1, "f_pssm_mix_type:%s", s.msg);
			//	doFlag = 0;
			//	//goto l_logicerror;
			//}

			//doFlag = 0;
		}
		else
		{
			//浇次第一炉
			if (bcls_rec->Tables[0].Rows[i-1]["TD_CHG_FLG"].ToDecimal() == 1)
			{
				tpssm10["INTERMIX_TYPE"] = "A";
				tpssm10["INTERMIX_FLAG_PONO"] = "0";
			}
			else
			{
				//非浇次第一炉有上一炉信息校验
				if (strcmp(v_pono_pre, " ") == 0)
				{
					sprintf(s.msg, "传入参数pono_pre[%s]为空。", (const char*)v_pono_pre);
					//strcpy(s.msg, "连铸机号不能为空");
					Log::Info("", __FUNCTION__, "传入参数pono_pre[{0}]为空", v_pono_pre);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//出钢记号相同
				if (strcmp(bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString(), bcls_rec->Tables[0].Rows[i - 1]["ST_NO"].ToString()) == 0)
				{
					tpssm10["INTERMIX_TYPE"] = "0";
					tpssm10["INTERMIX_FLAG_PONO"] = "0";
					tpssm10["INTERMIX_TYPE_ROUTE"] = "000";
				}
				else
				{
					//允许混浇
					if (strcmp(bcls_ret->Tables[0].Rows[i]["CAN_MIX_GROUP"].ToString(), bcls_ret->Tables[0].Rows[i - 1]["CAN_MIX_GROUP"].ToString()) == 0)
					{
						tpssm10["INTERMIX_TYPE"] = "1";
						tpssm10["INTERMIX_FLAG_PONO"] = "1";
						tpssm10["INTERMIX_TYPE_ROUTE"] = "100";
					}//允许混浇但需人工确认
					else if (strcmp(bcls_ret->Tables[0].Rows[i]["WORRY_MIX_GROUP"].ToString(), bcls_ret->Tables[0].Rows[i - 1]["WORRY_MIX_GROUP"].ToString()) == 0)
					{
						tpssm10["INTERMIX_TYPE"] = "2";
						tpssm10["INTERMIX_FLAG_PONO"] = "2";
						tpssm10["INTERMIX_TYPE_ROUTE"] = "200";
					}
					//else
					//{
					//	//-----------禁止混浇---------------------
					//	if ((0 == strcmp(bcls_ret->Tables[0].Rows[i]["ARCHIVE_FLAG"].ToString(), "1") || 0 == strcmp(bcls_ret->Tables[0].Rows[i - 1]["ARCHIVE_FLAG"].ToString(), "1"))
					//		|| 2 < fabs(bcls_ret->Tables[0].Rows[i]["ARCHIVE_FLAG"].ToDecimal().ToDouble() - bcls_ret->Tables[0].Rows[i - 1]["ARCHIVE_FLAG"].ToDecimal().ToDouble()))//禁止混浇
					//	{
					//		intermix_type_pono = "4";
					//	}
					//	else//强制混浇
					//	{
					//		intermix_type_pono = "0";
					//	}
					//}

					//----------------混浇分类------------
					//doFlag = f_pssm_mix_type(bcls_rec, bcls_ret);
					if (doFlag != 0)
					{
						sprintf(s.msg, "调用混浇分类代码判断函数 出错！");
						EDLog(1, 1, "f_pssm_mix_type:%s", s.msg);
						doFlag = 0;
						//goto l_logicerror;
					}

				}
			}
		}

		tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"];

		tpssm10.Update("INTERMIX_TYPE_ROUTE", "PONO");

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

