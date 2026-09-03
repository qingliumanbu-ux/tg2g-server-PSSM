/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-11-29
Description:	对炼钢作业计划设备传搁时间表进行修改。
**************************************************************************************************************/
#include "stdafx.h"



/*<remark>=========================================================
/// <summary>
/// 	炼钢作业计划设备传搁时间表修改
/// <para>读取传入的FACTORY_DIV,起始设备号，结束设备号，移动时间。    </para>
/// <para>判断输入的记录表中不存在,且设定的移行时间>0, 则新增，记录存在, 判断设定的移行时间是否>0,>0修改,<=0删除  </para>
/// <para>数据库表：tpssmd6(炼钢作业计划设备传搁时间表)                 </para>
/// <para>主调用函数：前台FormPSSMD6画面修改按钮调用。 </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <param name="dev_move_start">起始设备号     </param>
/// <param name="dev_move_end">结束设备号     </param>
/// <param name="move_time">移动时间     </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssmd6_upd)


int f_pssmd6_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag=0;
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString sqlstr="";
	// 定义表的实体对象
	CModel tpssmd6("TPSSMD6");

	try
	{
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows;  i++ )
		{
			// 获取前台传入参数
			tpssmd6.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			int count = tpssmd6.QueryCount("FACTORY_DIV, DEV_MOVE_START, DEV_MOVE_END");

			switch (count)
			{
			case 0: //没有记录
				if (tpssmd6["MOVE_TIME"].ToDecimal() > 0)
			    {
					tpssmd6["REC_CREATOR"]     = CString(s.userid);
					tpssmd6["REC_CREATE_TIME"] = datetime;
					tpssmd6.Insert();
				}
				break;
			case 1: //有记录
			    if (tpssmd6["MOVE_TIME"].ToDecimal() > 0)
			    {
					tpssmd6["REC_REVISOR"]     = CString(s.userid);
					tpssmd6["REC_REVISE_TIME"] = datetime;
					tpssmd6.Update("REC_REVISOR, REC_REVISE_TIME, MOVE_TIME", 
						"FACTORY_DIV, DEV_MOVE_START, DEV_MOVE_END");

			    }
				else
				{
					tpssmd6.Delete("FACTORY_DIV, DEV_MOVE_START, DEV_MOVE_END");
				}

				break;
			default: //有多余的记录
			    if (tpssmd6["MOVE_TIME"].ToDecimal() > 0)
			    {
					tpssmd6.Insert();
				}
				break;
			}
		}//for

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

