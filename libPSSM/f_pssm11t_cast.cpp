/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:  3.1
Description: 出钢计划浇次计算
Update：   2014-11-10  xuwen  炉次条件合并，子工序计划
**************************************************/
#include "stdafx.h"

//程序用头文件




//int f_pssm11_cast_create(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn);   //新增炉次生成CAST号
int f_pssm11t_cast_recalc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划炉次CAST号全部重算（按TPSSM10表浇注顺）

//int f_pssm12_pour_time(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);   //新增炉次浇铸周期（浇注准备、浇注时间）设置
//int f_pssm11_pour_time(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn);   //所有未浇注完炉次浇注时刻计算（包到、开浇、浇完、包离）
int f_pssm11t_get_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //获取开浇时刻，浇注时刻计算（包到、浇完、包离）

/*<remark>=========================================================
/// <summary>
/// 出钢计划浇次计算
/// <para>1.根据当前出钢计划的内容，计算CAST号；</para>
/// <para>2.根据计划炉重和铸坯规格，计算浇铸时间；</para>
/// <para>3.计算各炉次的开浇时刻。</para>
/// <para>4.新增炉次的开浇时刻，可指定或根据当前出钢计划浇次的内容决定
///  1)如果该连铸下有计划, 以当前计划推算第一炉的开浇时刻； 
///  2)如果该连铸下无计划, 以指定时刻推算第一炉的开浇时刻；
/// </para>
/// <para>数据库表：tpssm11(炼钢出钢计划主表)</para>
/// <para>主调用函数：pssm11_add(出钢计划编入调用)</para>
/// </summary>
/// <param name="PONO">制造命令号，新增入出钢计划的PONO</param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm11t_cast(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int blkseq, rows, i=0;
	int ret=0;

	CString v_pono = "";	//输入PONO
	CString v_restrand_flag = "";   // 输入连连铸标记
	CString v_cc_req_time = "";	//输入开浇时刻


	CString sqlstr;
	CModel tpssm11("TPSSM11");
	CModel tpssm10("TPSSM10");

	try
	{


		//-----------------------------------------------------------------------
		//获得输入参数
		//读取编入计划的指定开浇时刻.读取编入计划的PONO
		blkseq = bcls_rec->Tables.IndexOf("PONO");
		if (blkseq < 0)
		{
			strcpy(s.msg, "传入编制计划的命令数据块[PONO]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PONO] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		//for (i = 0; i < rows; i++)
		//{
		//	v_pono = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString().Trim();
		//	v_restrand_flag = bcls_rec->Tables[blkseq].Rows[i]["RESTRAND_FLG"].ToString().Trim();
		//	v_cc_req_time = bcls_rec->Tables[blkseq].Rows[i]["CC_REQ_TIME"].ToString().Trim(); //输入开浇时刻

		//	tpssm10["PONO"] = v_pono;
		//	tpssm11["PONO"] = v_pono;

		//	////Log::Info("", __FUNCTION__, "pono=[{0}], restrand_flag=[{1}], cc_req_time=[{2}]", v_pono, v_restrand_flag, v_cc_req_time);

		//	//前台时间控件为空时，得到的值是 00010101000000。 一个很严重的bug
		//	if (v_cc_req_time == "00010101000000")
		//	{
		//		v_cc_req_time = "";
		//	}

		//	//炉次设定为开浇炉，并有时刻时，写入记录
		//	if ( v_restrand_flag == "T" )
		//	{

		//		//指定开浇时刻
		//		if (v_cc_req_time == "")  //指定时刻为空时，取消指定标记
		//		{
		//			tpssm10["CC_REQ_TIME"] = " ";
		//			tpssm10["CC_REQ_TIME_FLAG"] = " ";  //CC要求时刻指定标记
		//			tpssm11["CC_REQ_TIME"] = " ";
		//		}
		//		else
		//		{
		//			tpssm10["CC_REQ_TIME"] = v_cc_req_time;
		//			tpssm10["CC_REQ_TIME_FLAG"] = "1";  //CC要求时刻指定标记
		//			tpssm11["CC_REQ_TIME"] = v_cc_req_time;
		//		}

		//	}
		//	else //取消开浇指定
		//	{
		//		tpssm10["CC_REQ_TIME"] = " ";
		//		tpssm10["CC_REQ_TIME_FLAG"] = " ";  //CC要求时刻指定标记
		//		tpssm11["CC_REQ_TIME"] = " ";
		//	}

		//	//在 f_pssm11_ins_pono 中操作，此处注释 xuwen 2015-4-9
		//	////修改计划表
		//	//sqlstr = "tpssm11.Update()";
		//	//tpssm11.Update("CC_REQ_TIME", "PONO");

		//	////同步修改浇铸计划表
		//	//sqlstr = "tpssm10.Update()";
		//	//tpssm10.Update("CC_REQ_TIME,CC_REQ_TIME_FLAG", "PONO");
		//	
		//}//for 


		//-----------------------------------------------------------------------
		//1.CAST号生成
		//ret = f_pssm11_cast_create(bcls_rec, bcls_ret, conn);
		//ret = f_pssm18_cast_create(bcls_rec, &outBlock, conn);  //不能用此函数，它是用开浇时间来计算CAST号，此时时刻没计算。2014-11-12 xuwen注释
		//出钢计划炉次CAST号全部重算(不需要参数)
		ret = f_pssm11t_cast_recalc(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-----------------------------------------------------------------------
		//2.浇注时刻计算
		// 1)浇铸时间设置, 输入参数:新增PONO
		//ret = f_pssm12_pour_time(bcls_rec, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		// 2)浇铸时刻计算, 无输入参数, 全部计算
		//ret = f_pssm11_pour_time(bcls_rec, bcls_ret, conn);  //浇铸时刻计算
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//获取开浇时刻，浇注时刻计算（包到、浇完、包离）
		ret = f_pssm11t_get_pour_time(bcls_rec, bcls_ret, conn);
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

	return doFlag;
}
