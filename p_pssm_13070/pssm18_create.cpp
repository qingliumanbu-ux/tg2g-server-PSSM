/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-13
Version:1.0
Description: 时间推移入口和出口处理
Update：2015-03-23 lijie修改
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
int f_pssm_call_tps_n(CString main_backlog_code, int mode, CDbConnection * conn);//
int f_pssm_call_tps_n2(CString factory_div, int mode, EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//int f_pssm21_create(EIClass *bcls_rec,EIClass *bcls_ret,CDbConnection * conn);
int f_pssm11_planno_n(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm12_sequ_calc_n(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn);  //根据作业时刻，赋流水号
//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
/*<remark>=========================================================
/// <summary>
/// 使用TPS2.0,主要用于计划编制完成后的生成模型优化的入口文件和对结果的处理
/// <para>1生成出钢计划in文件</para>
/// <para>2生成设备in文件</para>
/// <para>3生成传搁时间in文件</para>
/// <para>4对模型返回的结果进一步处理，保存到数据库</para>
/// <para>数据库表：tpssm11/12/18/d1/d4/d5</para>
/// <para>主调用函数：TPS2.0模型优化调用</para>
/// </summary>
/// <param name=></param>
/// <returns>模型计算结果</returns>
===========================================================</remark>*/
//-此节代码请勿更改
// service入口
BM2F_ENTERACE(pssm18_create)
//-EP_SYSTEM_HEAD_END
int f_pssm18_create(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);	//程序用变量
	int doFlag = 0;
	int ret=0;
	
	CDecimal dummy =0;			     /*编入出钢计划数量*/
	int		mode = 0;      //调用优化功能:  0-;   1-时间调整;
	//CDecimal flag;
	
	CModel tpssm11("TPSSM11");
	CModel tpssm99("TPSSM99");

	CString sqlstr;
	CString v_factory_div = " ";//分区标志
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm11_upd(conn);


	EIClass inblk;  //调用重新计算熔炼号的函数
	EIClass inBlock;
	EIClass outBlock;
	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);

	inblk.Tables[0].set_TableName("PLAN");  //
	inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	inblk.Tables[0].Columns.Add(DT_STRING, "SPECIAL_FLAG");
	inblk.Tables[0].Rows.Add();

	try
	{
		//获得输入参数
		v_factory_div = bcls_rec->Tables["PLAN"].Rows[0]["FACTORY_DIV"];
		mode = bcls_rec->Tables["PLAN"].Rows[0]["MODE"];
		inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		//inblk.Tables["PLAN"].Rows[0]["SPECIAL_FLAG"] = "1";
		//flag = bcls_rec->Tables["MODEL_INFO"].Rows[0]["STYLE"].ToDecimal();
		//mode=1 时间调整优化
		
		Log::Info("", __FUNCTION__, "FACTORY_DIV =[{0}]", (const char*)v_factory_div);
		if (mode != 2)
		{
			sqlstr = "SELECT a.*  \
					 				 	 FROM tpssm11 a\
										 					 WHERE  a.run_status        < 52  AND   factory_div =@factory_div";
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("factory_div", v_factory_div);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				cmd_tpssm11_inq.Fetch(tpssm11);//把数据都压在头文件里面
				tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm99["PONO"] = tpssm11["PONO"];
				tpssm99["EVENT_ID"] = "K2";
				tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
				tpssm99["VALID_FLAG"] = "1";//暂时默认为成功
				tpssm99["EVENT_DATETIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");//当前系统时间
				/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
			}
			cmd_tpssm11_inq.Close();
		}
		//--------------------------------------------------
		//时间推移计算
		if (mode == 1 || mode == 0)
		{
			ret = f_pssm_call_tps_n(v_factory_div, mode, conn);
			if (ret < 0)
			{
				for (int i = 0; i < in_pssm99trace.Tables[0].Rows.get_Count(); i++)
				{
					in_pssm99trace.Tables[0].Rows[i]["VALID_FLAG"] = "0";//记录失败履历
				}
				tpabort(0);
				tpbegin(0, 0);
				//记录失败的履历
				ret = 0;
				ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpcommit(0);
				tpbegin(0, 0);
				strcpy(s.msg, "时间优化推移出错.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		if (mode == 2)
		{
			ret = f_pssm_call_tps_n2(v_factory_div, mode, bcls_rec, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			inblk.Tables["PLAN"].Rows[0]["SPECIAL_FLAG"] = "1";
		}
		////Log::Trace("", __FUNCTION__, "时间优化成功--履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		//优化调整的履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//考虑到推荐设备的问题，所以计划号处理号都需要重新计算		
		////Log::Info("",__FUNCTION__,"重新计算计划号");
		ret=0;
		ret = f_pssm11_planno_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{  
			////Log::Info("",__FUNCTION__, "f_pssm11_planno()调用出错。");
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////Log::Info("",__FUNCTION__,"重新计算处理号");		
		ret = 0;
		ret = f_pssm12_sequ_calc_n(&inblk, bcls_ret, conn);
		if (ret < 0)
		{  
			////Log::Info("",__FUNCTION__, "f_pssm12_sequ_calc_n()调用出错。");
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
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
