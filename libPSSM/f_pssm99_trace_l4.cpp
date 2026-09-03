/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dongcuilian
Version:    1.0
Date:     2015-11-10
Description:	 写炼钢计划履历表。
Update:
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/*<remark>=========================================================
/// <summary>
/// 写炼钢调整履历表
/// <para>处理内容：写炼钢计划履历表履历表  </para>
/// <para>数据库表：TPSSM99 </para>
/// <para>主调用函数：被f_pssm11_ins_heat()、pssm18_chg()等调用。           </para>
/// </summary>
/// <param name="FACTORY_DIV">厂别区分         </param>
/// <param name="pono">制造命令          </param>
/// <param name="heat_no">熔炼号          </param>
/// <returns>无</returns>
===========================================================</remark>*/

BM2_FUNCTION_EXPORT
int f_pssm99_trace_l4(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	CString maxseq = "";
	CString newSeqNo = "";
	int blkseq = 0;

	CModel tpssm99("TPSSM99");
	CModel tpssm01("TPSSM01");
	CModel tep0002("TEP0002");//小代码描述
	CDbCommand cmd_inq(conn);
	CString sqlstr;


	try
	{
		create_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		blkseq = bcls_rec->Tables.IndexOf("TRACE");
		if (blkseq < 0)
		{
			sprintf(s.msg, "没有找到传入数据块[TRACE]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [TRACE] NOT EXIST ");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//tpssm36. = "A";
		/* 对输入信息循环处理 */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 1; i <= rows; i++)
		{
			/* 取得单行传入信息 */
			tpssm99.MergeFrom(bcls_rec->Tables[blkseq].Rows[i - 1]);
			if (tpssm99["PONO"].ToString().Trim() == "") //不能写入
			{
				CFormattable arguments[] = { tpssm99["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "要写入履历的炉次PONO[{0}]为空，不能记录履历。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//获取系统时间和操作人员			
			tpssm99["REC_CREATOR"] = s.userid;
			tpssm99["REC_CREATE_TIME"] = create_time;
			tpssm99["REC_REVISOR"] = s.userid;
			tpssm99["REC_REVISE_TIME"] = create_time;

			////Log::Trace("", __FUNCTION__, "tpssm99["PONO"] =[{0}] EVENT_ID =[{1}]", tpssm99["PONO"].ToString(), tpssm99["EVENT_ID"].ToString());

			tpssm01["PONO"] = tpssm99["PONO"].ToString().Trim();
			////Log::Trace("", __FUNCTION__, "尚未排入出钢计划从tpssm99["PONO_STATUS"] =[{0}]", tpssm99["PONO_STATUS"].ToDecimal());
			tpssm01["FACTORY_DIV"] = tpssm99["FACTORY_DIV"].ToString().Trim();
			tpssm01.Query("PONO,FACTORY_DIV");

			tpssm99["SM_PLAN_NO"] = " ";//初始化
			tpssm99["HEAT_NO"] = " ";
			tpssm99["BACKLOG_EA"] = tpssm01["BACKLOG_EA"];

			tpssm99["PONO_STATUS"] = tpssm01["PONO_STATUS"];//PONO状态
			tpssm99["SG_SIGN"] = tpssm01["SG_SIGN"];
			tpssm99["TD_CHG_FLG"] = tpssm01["TD_CHG_FLG"];//快换中包标志
			tpssm99["ST_NO"] = tpssm01["ST_NO"];
			tpssm99["RESTRAND_FLG"] = tpssm01["RESTRAND_FLG"];//重引锭标记
			tpssm99["CC_MACH_NO"] = tpssm01["CC_MACH_NO"];
			tpssm99["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			tpssm99["CAST_LOT_DIV_NO"] = tpssm01["CAST_LOT_DIV_NO"];

			//设置序列号
			maxseq = EPGetNextSeq("PSSM99_SEQ", conn);
			newSeqNo = newSeqNo.Format("%4s", (const char*)maxseq);
			tpssm99["PROD_SEQ_NO"] = create_time + newSeqNo;

			//读取事件描述
			tep0002["CODE"] = tpssm99["EVENT_ID"];
			tep0002["CODE_CLASS"] = "PSAX";//炼钢计划履历事件代码
			if (!tep0002.Query("CODE,CODE_CLASS"))
			{
				CFormattable arguments[] = { tep0002["CODE"].ToString() };
				CMessageFormat::Format(s.msg, "读取炼钢计划履历事件代码出错，请检查。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm99["EVENT_DESC"] = tep0002["CODE_DESC_1_CONTENT"].ToString().Trim();

			tpssm99.TrimOrBlank();
			/*tpssm99.Print();*/

			////Log::Trace("", __FUNCTION__, "tpssm99.Insert():PROD_SEQ_NO=[{0}], FACTORY_DIV=[{1}], SM_PLAN_NO=[{2}], PONO=[{3}], EVENT_ID=[{4}], EVENT_DESC=[{5}],INSERT_FALG = [{6}]", tpssm99["PROD_SEQ_NO"].ToString(), tpssm99["FACTORY_DIV"].ToString(), tpssm99["SM_PLAN_NO"].ToString(), tpssm99["PONO"].ToString(), tpssm99["EVENT_ID"].ToString(), tpssm99["EVENT_DESC"].ToString(), tep0002["CODE_DESC_2_CONTENT"].ToString());
			sqlstr = "tpssm99.Insert()";

			if (tep0002["CODE_DESC_2_CONTENT"].ToString() == "1")
			{
				tpssm99.Insert();
			}

		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		////Log::Trace("", __FUNCTION__, "tpssm99--ex.GetCode()=[{0}]", ex.GetCode());
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		////Log::Trace("", __FUNCTION__, "s.flag=[{0}],doFlag=[{1}]", s.flag, doFlag);
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

