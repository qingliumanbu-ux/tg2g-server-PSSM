/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-26
Description:	根据传入的炼钢厂别区分，制造命令号、切割结束标记来修改钢种管理表中切割结束标记。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/*<remark>=========================================================
/// <summary>
/// 炼钢计划炉次钢种管理更新切割结束标记
/// <para>处理内容：更新 TPSSM11表内容，切割结束标记      </para>
/// <para>数据库表：TPSSM11(出钢计划主表管理)                  </para>
/// <para>主调用函数：被 物料模块 调用。           </para>
/// </summary>
/// <param name="sm_unit_no">厂别区分         </param>
/// <param name="pono">制造命令号         </param>
/// <param name="flag">切割结束标记     </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm11_cut_rcv(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i,dummy;


	CModel tpssm11("TPSSM11");
	CModel tpssm41("TPSSM41");
	CDbCommand cmd_inq(conn);
	CString sqlstr;


	try
	{
		blkseq = bcls_rec->Tables.IndexOf("PSSM11");
		if (blkseq < 0) 
		{
			strcpy(s.msg,_RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "传入的参数中没有\"PSSM11\"块。");
			throw CApplicationException(-1,s.msg,log.Location);
		} 

		/* 对输入信息循环处理 */
		rows=bcls_rec->Tables["PSSM11"].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{
			/* 取得单行传入信息 */
			tpssm11["FACTORY_DIV"]=bcls_rec->Tables["PSSM11"].Rows[i]["FACTORY_DIV"].ToString();
			tpssm11["HEAT_NO"]=bcls_rec->Tables["PSSM11"].Rows[i]["HEAT_NO"].ToString();
			tpssm11["CUT_FIN_FLAG"]=bcls_rec->Tables["PSSM11"].Rows[i]["CUT_FIN_FLAG"].ToString();

			////Log::Trace("", __FUNCTION__, "tpssm11["FACTORY_DIV"] =[{0}]", tpssm11["FACTORY_DIV"].ToString());

			//检查传入的HEAT_NO是否存在
			dummy=tpssm11.QueryCount("FACTORY_DIV,HEAT_NO");
			if (dummy == 0)
			{
				//CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				//CMessageFormat::Format(s.msg, _RES("PSSMS0000088")/*制造命令号[{0}]在出钢计划中不存在。*/, arguments, 1); //格式化字符串
				//throw CApplicationException(-1,s.msg,log.Location);
				tpssm41["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm41["HEAT_NO"] = tpssm11["HEAT_NO"];
				tpssm41["CUT_FIN_FLAG"] = tpssm11["CUT_FIN_FLAG"];
				tpssm41.Update("CUT_FIN_FLAG", "FACTORY_DIV,HEAT_NO");
			}
			tpssm11.Update("CUT_FIN_FLAG","FACTORY_DIV,HEAT_NO");
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
	return doFlag;
}
