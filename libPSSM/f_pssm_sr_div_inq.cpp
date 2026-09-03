/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-12-27
Description:	 炼钢计划中精炼路径转换为精炼区分函数。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


//程序用头文件



//#include "tpssm31.h"
/*<remark>=========================================================
/// <summary>
/// 炼钢计划中精炼路径转换为精炼区分函数
/// <para>根据传入的厂别区分、制造命令号。</para>
/// <para>1.读取该制造命令号的精炼路径。</para>
/// <para>2.将精炼路径转化为精炼区分。 </para>
/// <para>数据库表：tpssm13(出钢计划跟踪主表) tpssm31(炉次钢种管理主表)，tpssmd1(设备配置表)          </para>
/// <para>主调用函数：由质量的炉次品质异常判定查询调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono">制造命令号          </param>
/// <returns>精炼路径信息</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm_sr_div_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int fetchRowCount;
	int blkseq, num;
	int i, len;

	CString	 date_time;            /* 记录创建时刻 */
	int    dummy;
	CString	 refine_route_code="";            /* 精炼路径 */
	CString	 refine_div_code="";              /* 精炼区分 */
	CModel tpssmd1("TPSSMD1");
	CModel tpssm11("TPSSM11");
	//CTPSSM31 tpssm31(conn);
	CDbCommand cmd_inq(conn);
	CString sqlstr;


	try
	{
		date_time=CDateTime::Now().ToString("yyyyMMddHHmmss");
		//设定返回参数表
		if (bcls_ret->Tables[0].Columns.IndexOf("REFINE_DIV_CODE")<=0)
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING,"REFINE_DIV_CODE");//精炼区分
		}
		//获得输入参数
		tpssm11["FACTORY_DIV"]=bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm11["PONO"]=bcls_rec->Tables[0].Rows[0]["PONO"].ToString();

		//1.检查输入的PONO是否为存在
		dummy=tpssm11.QueryCount("FACTORY_DIV,PONO");
		if (dummy > 0) //出钢计划中有
		{
			tpssm11.Query("FACTORY_DIV,PONO");
			refine_route_code=tpssm11["REFINE_ROUTE_CODE"];
		}
		//else  //出钢计划中没有, 查找炉次管理表
		//{
		//	tpssm31.FACTORY_DIV=tpssm13.FACTORY_DIV;
		//	tpssm31.PONO=tpssm13.PONO;
		//	tpssm31.Query("FACTORY_DIV,PONO");
		//	refine_route_code=tpssm31.REFINE_ROUTE_CODE;

		//}
		refine_route_code=refine_route_code.TrimOrBlank();

		//2.检查输入的精炼路径是否是空,如果空则退出校验
		if (refine_route_code.Compare(" ") != 0)
		{
			tpssmd1["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

			//3.循环处理
			len=refine_route_code.GetLength();
			for (i=0;  i<len;  i++)
			{
				//HYF 20130427 SubstringNE
				tpssmd1["DEV_CODE"]=refine_route_code.SubstringNE(i,1);
				tpssmd1["AREA_ID"]=4;
				dummy=tpssmd1.QueryCount("FACTORY_DIV,DEV_CODE,AREA_ID");
				if (dummy == 0)
				{
					CFormattable arguments[] = { tpssmd1["DEV_CODE"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000136")/*设备[{0}]未配置，请联系维护人员。*/, arguments, 1); //格式化字符串
					throw CApplicationException(-1,s.msg,log.Location);
				}
				
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
				tpssmd1["STATION_ID"]=tpssmd1["STATION_ID"].ToString().TrimOrBlank();
				refine_div_code=refine_div_code +tpssmd1["STATION_ID"].ToString();
			}	
			//返回转换后的精炼区分
			bcls_ret->Tables[0].Rows[0]["REFINE_DIV_CODE"]=refine_div_code;
		}
		else
		{
			bcls_ret->Tables[0].Rows[0]["REFINE_DIV_CODE"]=" ";
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
