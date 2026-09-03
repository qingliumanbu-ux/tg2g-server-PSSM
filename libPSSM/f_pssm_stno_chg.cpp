/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2014
Author:    xuwen
Version:   3.1.0
Date:      2014-12-29
Description: 钢种变更
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tpssm01.h"
#include "tpssm10.h"
#include "tpssm11.h"
#include "tpssm12.h"
#include "tpssm13.h"
//#include "tpssm14.h"
#include "tpssm31.h"
#include "tpssm32.h"
#include "tpssmd1.h"
#include "tpssmd4.h"
#include "tpssmd5.h"

//计划模块处理
int f_pssm10_stno_chg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //钢种变更-连铸浇铸顺调整
int f_pssm13_stno_chg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //钢种变更-出钢计划调整
int f_pssm31_stno_chg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //钢种变更-炉次钢种表调整

//实绩模块处理
int f_mmsm_stno_chg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //处理钢种对换或者钢种变更

//电文函数
int f_pssm_kbkz67_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//钢种变更至制造管理系统
int f_pssm_kbba22_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//钢种变更至转炉L2系统
int f_pssm_kbra22_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//钢种变更至精炼L2系统
int f_pssm_kbma22_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//钢种变更至炼钢物流L2系统


/*<remark>=========================================================
/// <summary>
/// 钢种变更
/// <para>前提条件是炉次未开浇。钢种变更分转炉区域和各精炼区域的变更。
///钢种变更后，精炼路径沿用原计划的路径，炉次的成分重新判定，重新进行炉次品质判定。
/// </para>
/// <para>1.检查参数：                                             </para>
/// <para>1）传入的制造命令号不为空；                              </para>
/// <para>2）校验是否是同一台连铸机的钢种变更；                    </para>
/// <para>3）已经开浇的制造命令不能钢种变更；                      </para>
/// <para>校验为正常时,继续以下的处理;否则,只输出警报,操作无效。   </para>
/// <para>2.上述替换制造命令号输入时,作以下处理.                   </para>
/// <para>a.制造命令置换处理                                       </para>
/// <para>确认用以置换的制造命令号对应的制造命令在制造命令表（TPSSM10）中存在,且对出钢计划画面作制造命令置换。
///否则,不作置换操作.但是,生产时刻、热送要求时刻、浇次号及浇次分割号不置换。
///但浇次号,浇次分割号在变更前置空.
/// </para>
/// <para>b.出钢计划置换处理                                          </para>
/// <para>用未列入出钢计划的由操作员输入制造命令号指定的制造命令替代出钢计划中的被替代制造命令。
///从出钢计划外制造命令号文件中删除操作员输入的制造命令号,追加被替换的制造命令号。
/// </para>
/// <para>c.作业实绩替换处理                                          </para>
/// <para>当钢种替换变更输入时,MES对已收到的作业实绩中,把实绩文件中的制造命令号和
///出钢记号变更为替换制造命令号和替换出钢记号,其它不变；
/// </para>
/// <para>d. 生产管理模块的材料状态处理：                             </para>
/// <para>将钢种变更的两个制造命令下的材料状态进行重新计算；          </para>
/// <para>d.向L2发送钢种变更通知                                      </para>
/// <para>向已送出作业实绩的的L2系统送出钢种变更通知                  </para>
/// <para>e.炉次的成分重新判定，重新进行炉次品质判定。                </para>
/// <para>数据库表：TPSSM31/11/13(炉次钢种管理表、出钢计划表、跟踪表) </para>
/// <para>主调用函数：前台PSSM11画面(F8 钢种变更) pssm11_stno_chg(service)调用 </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>无</returns>
===========================================================</remark>*/

int f_pssm_stno_chg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i;


	EIClass inBlock; //电文
	EIClass inBlock2; //MM函数用

	CDecimal sm_plan_no_out = 0;
	CString  pono_out= "";
	CString  pono_in = "";

	CString sqlstr;
	CTPSSM10 tpssm10_out(conn);
	CTPSSM10 tpssm10_in(conn);	//变更目标计划
	CTPSSM13 tpssm13_out(conn); //被变更出的计划
	CTPSSM13 tpssm13_in(conn);  //被变更入的计划
	CDbCommand cmd_inq(conn);


	try
	{
		//---------------------------------------------------------------------------
		//1.电文函数数据块信息结构定义(电文结构字段)
		inBlock.Tables[0].set_TableName("STNO_CHG");
		inBlock.Tables[0].Columns.Add(DT_DECIMAL, "SM_PLAN_NO1");   //计划号1
		inBlock.Tables[0].Columns.Add(DT_DECIMAL, "SM_PLAN_NO2");   //计划号2
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO1");         //PONO1（变更前PONO）
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO2");         //PONO1（变更后PONO）

		//2.MM函数用数据块信息结构定义
		inBlock2.Tables[0].set_TableName("STNO_CHG");
		inBlock2.Tables[0].Columns.Add(DT_DECIMAL, "SM_PLAN_NO1");   //计划号1
		inBlock2.Tables[0].Columns.Add(DT_DECIMAL, "SM_PLAN_NO2");   //计划号2
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO1");         //PONO1（变更前PONO）
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO2");         //PONO1（变更后PONO）
		inBlock2.Tables[0].Columns.Add(DT_STRING, "ST_NO1");        //ST_NO1（变更前ST_NO）
		inBlock2.Tables[0].Columns.Add(DT_STRING, "ST_NO2");        //ST_NO2（变更后ST_NO）


		//--------------------------------------------------------------
		//获得输入参数
		blkseq = bcls_rec->Tables.IndexOf("STNO_CHG"); //钢种变更 数据块
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到钢种变更数据块[STNO_CHG]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [STNO_CHG] NOT EXIST in f_pssm_stno_chg().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		if (rows < 0)
		{
			strcpy(s.msg, "传入数据块[STNO_CHG]没有数据，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [STNO_CHG] HAVE NOTHING .");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sm_plan_no_out = bcls_rec->Tables["STNO_CHG"].Rows[0]["SM_PLAN_NO_OUT"].ToDecimal().ToInt32();
		pono_out = bcls_rec->Tables["STNO_CHG"].Rows[0]["PONO_OUT"].ToString().Trim();
		pono_in  = bcls_rec->Tables["STNO_CHG"].Rows[0]["PONO_IN"].ToString().Trim();

		Log::Info("", __FUNCTION__, "sm_plan_no_out=[{0}], outPONO=[{1}], inPONO=[{2}]", sm_plan_no_out, pono_out, pono_in);

		//增加钢种变更条件判断
		//2015-10-15 改为变更入的制造命令是否在下达计划表（TPSSM13）中存在. 否则有2个计划对应一个PONO
		//如果计划未下达，则不允许进行变更。  
		//tpssm26.CC_MACH_NO = out_tpssm13.CC_MACH_NO;
		//tpssm26.Query("SM_UNIT_NO,CC_MACH_NO");
		//if (tpssm26.EDIT_FLAG != 5)
		//{
		//	strcpy(s.msg, _RES("PSSMS0000198"))/*出钢计划未下达，不能做钢水交换*/;
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//1).检查替换入的PONO是否在下达计划表（TPSSM13）中存在。 2015-10-15 增加
		tpssm13_in.PONO = pono_in;
		if (tpssm13_in.QueryCount("PONO") > 0)
		{
			CFormattable arguments[] = { pono_in }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令[{0}]还存在于下达的出钢计划中（计划未下达），不能做钢种变更。请先下达计划，再做变更！", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//2).检查替换出的PONO状态
		// 以下达计划表(TPSSM13)内容为主, 生产中的计划必然存在于该表中
		tpssm13_out.SM_PLAN_NO = sm_plan_no_out;
		//tpssm13_out.PONO = pono_out;
		sqlstr = "tpssm13_out.Query(PONO)";
		bool has13_out = tpssm13_out.Query("SM_PLAN_NO");
		if (has13_out == false)
		{
			CFormattable arguments[] = { tpssm13_out.SM_PLAN_NO.ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "计划号[{0}]在下达的出钢计划中不存在，不能做钢种变更。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//获取变更入PONO信息
		tpssm10_in.PONO = pono_in;
		sqlstr = "tpssm10_in.Query(PONO)";
		bool has10_in = tpssm10_in.Query("PONO");

		//钢种变更PONO内容对换: 
		//1) 浇注顺(tpssm10),
		//2) 计划表(TPSSM11/12), 下达表(TPSSM13/14), 
		//3) 炉次钢种表(TPSSM31/32)

		//-------------------------------------------
		//钢种变更-连铸浇铸顺调整
		ret = f_pssm10_stno_chg(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-------------------------------------------
		//钢种变更-出钢计划调整
		ret = f_pssm13_stno_chg(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-------------------------------------------
		//钢种变更-炉次钢种表调整
		ret = f_pssm31_stno_chg(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//---------------------------------------------------------------------------
		//MM模块函数数据块赋值
		CDataRow &row = inBlock2.Tables["STNO_CHG"].Rows.Add();

		row["SM_PLAN_NO1"] = tpssm13_out.SM_PLAN_NO;
		row["SM_PLAN_NO2"] = 0;  //一定赋0，表示钢种变更
		row["PONO1"] = pono_out;
		row["PONO2"] = pono_in;
		row["ST_NO1"] = tpssm13_out.ST_NO;
		row["ST_NO2"] = tpssm10_in.ST_NO;

		//实绩模块处理
		ret = f_mmsm_stno_chg(&inBlock2, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//---------------------------------------------------------------------------
		//电文用数据块赋值
		inBlock.Tables[0].Rows.Add();
		inBlock.Tables[0].Rows[0]["SM_PLAN_NO1"] = tpssm13_out.SM_PLAN_NO;
		inBlock.Tables[0].Rows[0]["SM_PLAN_NO2"] = 0;  //一定赋0，表示钢种变更
		inBlock.Tables[0].Rows[0]["PONO1"] = pono_out;
		inBlock.Tables[0].Rows[0]["PONO2"] = pono_in;

		////另一版本
		//inBlock2.Tables[0].set_TableName("STNO_CHG");
		//inBlock2.Tables[0].Columns.Add(DT_DECIMAL, "SM_PLAN_NO");   //计划号1
		//inBlock2.Tables[0].Columns.Add(DT_STRING, "BEFORE_PONO");        //变更前PONO
		//inBlock2.Tables[0].Columns.Add(DT_STRING, "AFTER_PONO");         //变更后PONO
		//inBlock2.Tables[0].Columns.Add(DT_STRING, "CHANGE_DEV_CODE");   //计划号2

		////数据块赋值
		//inBlock2.Tables[0].Rows.Add();
		//inBlock2.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm13_out.SM_PLAN_NO;
		//inBlock2.Tables[0].Rows[0]["BEFORE_PONO"] = pono_out;
		//inBlock2.Tables[0].Rows[0]["AFTER_PONO"] = pono_in;
		//inBlock2.Tables[0].Rows[0]["CHANGE_DEV_CODE"] = "";


		//-------------------------------------------
		//1)钢种变更至制造管理系统
		ret = f_pssm_kbkz67_snd(&inBlock, bcls_ret, conn);
		ret = 0;
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//2)钢种变更至转炉L2系统
		ret = f_pssm_kbba22_snd(&inBlock, bcls_ret, conn);
		ret = 0;
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//3)钢种变更至精炼L2系统
		ret = f_pssm_kbra22_snd(&inBlock, bcls_ret, conn);
		ret = 0;
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//4)钢种变更至炼钢物流L2系统
		ret = f_pssm_kbma22_snd(&inBlock, bcls_ret, conn);
		ret = 0;
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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

	s.flag = doFlag;
	return doFlag;
}
