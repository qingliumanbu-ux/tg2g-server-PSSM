/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2015-10-17
Description: 炉次条件开浇时刻设置
Modify:    2015-11-23 只对未开浇第1炉设置开浇时刻 
**************************************************/
#include "stdafx.h"

#include "tpssm11.h"
#include "tpssm10.h"


/*<remark>=========================================================
/// <summary>
/// 出钢计划炉次条件开浇时刻设置
/// <para>修改内容包括: 开浇时刻调整</para>
/// <para>对非精炼工序, 调整的内容是设备工位号的更新。          </para>
/// <para>对精炼工序，存在增减精炼工序，调整的内容包括：
///钢区工艺途径、精炼路径、设备工位号的更新。                   </para>
/// <para>数据库表：TPSSMD1/11/12(炼钢出钢计划表)               </para>
/// <para>主调用函数：前台PSSM12P(炉次条件)画面F3(修改)调用。</para>
/// </summary>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12a_cctime)


int f_pssm12a_cctime(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i=0;

	CString date_time = "";
	CString date_time1 = ""; //当前时刻提前控制量

	/* 业务变量 */
	CDecimal v_sm_plan_no = 0;	//炼钢计划号
	CString  v_pour_time = "";	//输入开浇时刻
	CString  v_pono = "";
	int     bof_cc_matching = 0;     /*炉机匹配标记: 0-不指定,模型推荐; 1-指定,模型按要求计算*/
	int		mode = 0;      //调用模型功能:  0-模型;   1-匹配;

	EIClass inBlock;

	// 定义表的实体对象
	CTPSSM11 tpssm11(conn);
	CTPSSM10 tpssm10(conn);

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);



	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------------------
		//定义函数调用信息块
		blkseq = 0;  //第一块 "PONO"块，f_pssm12_time_calc()函数用
		inBlock.Tables[blkseq].set_TableName("PONO");
		//inBlock.Tables[blkseq].Columns.Add(tpssm11);   //从实体对象创建架构
		inBlock.Tables[blkseq].Columns.Add(DT_STRING, "PONO");//制造命令号


		//---------------------------------------------------
		//获得输入参数
		//循环读取浇铸信息，多记录
		blkseq = bcls_rec->Tables.IndexOf("POURTIME");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有开浇时刻设定信息数据块[POURTIME]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [POURTIME] NOT EXIST in pssm12_upd().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			v_sm_plan_no = bcls_rec->Tables[blkseq].Rows[i]["SM_PLAN_NO"].ToDecimal().ToInt32();
			v_pour_time  = bcls_rec->Tables[blkseq].Rows[i]["CC_REQ_TIME"].ToString().Trim();

			Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}], CC_REQ_TIME=[{1}]", v_sm_plan_no, v_pour_time);


			//校验
			//1.获取计划状态
			sqlstr = CString(
				" SELECT PONO, PONO_STATUS, CC_MACH_NO, CAST_NO, CAST_DIV_NO"
				" FROM TPSSM11 WHERE SM_PLAN_NO = @sm_plan_no"
				);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("sm_plan_no", v_sm_plan_no);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tpssm11.PONO = cmd_inq.GetString(1).Trim();
				tpssm11.PONO_STATUS = cmd_inq.GetDecimal(2).ToInt32();
				tpssm11.CC_MACH_NO = cmd_inq.GetString(3).Trim();
				tpssm11.CAST_NO = cmd_inq.GetString(4).Trim();
				tpssm11.CAST_DIV_NO = cmd_inq.GetDecimal(5).ToInt32();
			}
			else
			{
				tpssm11.PONO_STATUS = 0;
				tpssm11.CC_MACH_NO = "";
				tpssm11.CAST_NO = "";
				tpssm11.CAST_DIV_NO = 0;
			}
			cmd_inq.Close();

			if (tpssm11.PONO_STATUS > 81)
			{
				CFormattable arguments[] = { v_sm_plan_no.ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]已开浇，不能修改开浇时刻。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//2.只对未开浇第1炉设置时刻  2015-11-23 增加
			//连铸机号不为空进行计算
			if (tpssm11.CC_MACH_NO == "")
			{
				CFormattable arguments[] = { v_sm_plan_no.ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "计划[{0}]的连铸机为空，请联系系统维护人员。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//CAST第一炉不做校验
			if (tpssm11.CAST_DIV_NO > 1)
			{
				CDecimal plan_no = 0;
				CString cast_no = "";
				//未开浇第1炉计划号
				sqlstr = CString(
					"SELECT SM_PLAN_NO, CAST_NO FROM TPSSM11 "
					" WHERE CC_MACH_NO = @cc_mach_no "
					" AND PONO_STATUS < 82 "  //82-开浇
					" ORDER BY CAST_NO ASC, CAST_DIV_NO ASC"
					);
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("cc_mach_no", tpssm11.CC_MACH_NO);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					plan_no = cmd_inq.GetDecimal(1).ToInt32();
					cast_no = cmd_inq.GetString(2).Trim();
				}
				else
				{
					plan_no = 0;
					cast_no = "";
				}
				cmd_inq.Close();

				Log::Trace("", __FUNCTION__, "未开浇第1炉计划号:[{0}]", plan_no);

				//设定的开浇时刻不是未开浇第1炉，报错
				if (plan_no != v_sm_plan_no)
				{
					CFormattable arguments[] = { v_sm_plan_no.ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "计划[{0}]不是未开浇第1炉，不能做开浇时刻修改。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//不允许后续CAST的开浇时刻设置，因为会有跨天的设置问题，只能在浇铸计划中做。
				////指定计划的CAST是当前浇铸的CAST
				//if (tpssm11.CAST_NO == cast_no)
				//{
				//}
				//else //不同的只能指定T标记（开浇）炉
				//{
				//	if (tpssm11.CAST_DIV_NO > 1)
				//	{
				//		CFormattable arguments[] = { v_sm_plan_no.ToString() }; // 定义参数列表的数组
				//		CMessageFormat::Format(s.msg, "计划[{0}]不是CAST[{1}]内第1炉，不能做开浇时刻设定。", arguments, 1); //格式化字符串
				//		throw CApplicationException(-1, s.msg, log.Location);
				//	}
				//}
			}


			tpssm11.SM_PLAN_NO = v_sm_plan_no;

			//---------------------------------------------
			//如果输入的时刻为空，则清除指定开浇时刻
			if (v_pour_time == "")
			{
				//修改计划表
				tpssm11.CC_REQ_TIME = "";

				sqlstr = "tpssm11.Update()-1";
				tpssm11.Update("CC_REQ_TIME", "SM_PLAN_NO");

				//修改浇铸计划表（CC要求时刻不用清空）
				tpssm10.PONO = tpssm11.PONO;
				tpssm10.CC_REQ_TIME_FLAG = ""; //取消CC要求时刻指定标记
				sqlstr = "tpssm10.Update()-1";
				tpssm10.Update("CC_REQ_TIME_FLAG", "PONO");

			}
			else
			{

				//根据输入的时刻，计算14位目的时刻
				date_time1 = CDateTime::Now().AddMinutes(-20).ToString("yyyyMMddHHmmss");  //将当前时间减30分钟，避免当前炉次指定时刻错认为是跨天设置
				CString curr_time = date_time.SubstringNE(8, 4);
				CString curr_time1 = date_time1.SubstringNE(8, 4);

				//如果过0点设置(输入的时刻比当前时刻晚)，认为是跨天设置
				if (date_time.SubstringNE(0, 8) == date_time1.SubstringNE(0, 8)) //提前量与当前时刻是同一天，则不存在跨天设置问题
				{
					//输入的时刻比当前时刻晚(过0点设置)，认为是跨天设置
					if (v_pour_time < curr_time1)
					{
						CDateTime tmp_time = CDateTime::Parse(date_time.SubstringNE(0, 8) + v_pour_time + "00");
						tpssm11.CC_REQ_TIME = tmp_time.AddDays(1).ToString("yyyyMMddHHmmss");

					}
					else
					{
						tpssm11.CC_REQ_TIME = date_time.SubstringNE(0, 8) + v_pour_time + "00";
					}

				}
				else //反向跨天设置（不合理，但必须保证当前炉次时刻不跨天）
				{
					//输入的时刻比当前时刻早(过24点设置)，认为是反向跨天设置
					if (v_pour_time.SubstringNE(0, 2) == "23") //设置在晚上23到24点之间时
					{
						//CDateTime tmp_time = CDateTime::Parse(date_time.SubstringNE(0, 8) + v_pour_time + "00");
						//tpssm11.CC_REQ_TIME = tmp_time.AddDays(-1).ToString("yyyyMMddHHmmss");
						tpssm11.CC_REQ_TIME = date_time1.SubstringNE(0, 8) + v_pour_time + "00";
					}
					else
					{
						tpssm11.CC_REQ_TIME = date_time.SubstringNE(0, 8) + v_pour_time + "00";
					}
				}
				Log::Trace("", __FUNCTION__, "计算后开浇时刻： CC_REQ_TIME=[{1}]", v_sm_plan_no, tpssm11.CC_REQ_TIME);

				//修改计划表
				sqlstr = "tpssm11.Update()-2";
				tpssm11.Update("CC_REQ_TIME", "SM_PLAN_NO");

				//修改浇铸计划表
				tpssm10.PONO = tpssm11.PONO;
				tpssm10.CC_REQ_TIME = tpssm11.CC_REQ_TIME;
				tpssm10.CC_REQ_TIME_FLAG = "1"; //设置CC要求时刻指定标记
				sqlstr = "tpssm10.Update()-2";
				tpssm10.Update("CC_REQ_TIME,CC_REQ_TIME_FLAG", "PONO");

			}

		}//for 传入参数




	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = -1;
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
