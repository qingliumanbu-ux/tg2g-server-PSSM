/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-26
Description:	 写入实绩接收标记。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


//程序用头文件




/*<remark>=========================================================
/// <summary>
/// 写入实绩接收标记
/// <para>处理内容：将实绩接收标记写入数据库  </para>
/// <para>数据库表：TPSSM11/12(炉次钢种管理主表/子表) </para>
/// <para>主调用函数：被物料模块调用。           </para>
/// </summary>
/// <param name="sm_unit_no">厂别区分         </param>
/// <param name="heat_no">炉号          </param>
/// <param name="station_id">工位代码         </param>
/// <param name="station_no">工位号    </param>
/// <param name="proc_no">处理号     </param>
/// <param name="flag">接收标志    </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm12_mm_rcv(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i, blkseq;
	CString v_flag="";
	int dummy = 0;
	CModel tpssmd1("TPSSMD1");
	CModel tpssm12("TPSSM12");
	CModel tpssm11("TPSSM11");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	try
	{
		blkseq = bcls_rec->Tables.IndexOf("PSSM12");
		if (blkseq <= 0) 
		{
			strcpy(s.msg,_RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "传入的参数中没有\"PSSM12\"块。");
			throw CApplicationException(-1,s.msg,log.Location);
		} 

		//获得输入参数
		tpssm12["FACTORY_DIV"]=bcls_rec->Tables["PSSM12"].Rows[0]["FACTORY_DIV"].ToString();
		tpssm12["HEAT_NO"]=bcls_rec->Tables["PSSM12"].Rows[0]["HEAT_NO"].ToString();
		tpssmd1["STATION_ID"]=bcls_rec->Tables["PSSM12"].Rows[0]["STATION_ID"].ToString();
		tpssmd1["STATION_NO"]=bcls_rec->Tables["PSSM12"].Rows[0]["STATION_NO"].ToString();
		tpssm12["PROC_NO"]=bcls_rec->Tables["PSSM12"].Rows[0]["PROC_NO"].ToString();
		tpssm12["PRACT_RCV_FLAG"]=bcls_rec->Tables["PSSM12"].Rows[0]["PRACT_RCV_FLAG"].ToString();
		/////////////
		tpssm12["SM_PLAN_NO"] = bcls_rec->Tables["PSSM12"].Rows[0]["SM_PLAN_NO"].ToString();

		Log::Trace("", __FUNCTION__, "f_pssm12_mm_rcv> sm_unit_no=[{0}], heat_no=[{1}], station=[{2}{3}], proc_no=[{4}], flag=[{5}]", 
			(const  char*)tpssm12["FACTORY_DIV"].ToString(), (const  char*)tpssm12["HEAT_NO"].ToString(), (const  char*)tpssmd1["STATION_ID"].ToString(), (const  char*)tpssmd1["STATION_NO"].ToString(), (const  char*)tpssm12["PROC_NO"].ToString(), (const  char*)tpssm12["PRACT_RCV_FLAG"].ToString());

			Log::Trace("", __FUNCTION__, "tpssm12.PRACT_RCV_FLAG000000=[{0}]]", tpssm12["PRACT_RCV_FLAG"].ToString());



		tpssmd1["FACTORY_DIV"]=tpssm12["FACTORY_DIV"];
		dummy=tpssmd1.QueryCount("FACTORY_DIV,STATION_ID,STATION_NO");
		if (dummy <= 0)
		{
			CFormattable arguments[] = { tpssmd1["STATION_ID"].ToString(), tpssmd1["STATION_NO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000160")/*工位标志[{0}]和工位号[{1}]未配置，请联系维护人员。*/, arguments, 2); //格式化字符串
			throw CApplicationException(-1,s.msg,log.Location);
		}

		//Log::Trace("", __FUNCTION__, "tpssm12.PRACT_RCV_FLAG11111111=[{0}]]", tpssm12["PRACT_RCV_FLAG"].ToString());


		tpssmd1.Query("FACTORY_DIV,STATION_ID,STATION_NO");
		tpssm12["DEV_CODE"] = tpssmd1["DEV_CODE"];
		//Log::Trace("", __FUNCTION__,  "dev_code=[{0}][{1}]",(const char*)tpssm12["DEV_CODE"].ToString(),(const char*)tpssmd1["DEV_CODE"].ToString());
		//Log::Trace("", __FUNCTION__,  "area_id =[{0}]",tpssmd1["AREA_ID"].ToDecimal().ToInt32());

			//Log::Trace("", __FUNCTION__, "tpssm12.PRACT_RCV_FLAG22222=[{0}]", tpssm12["PRACT_RCV_FLAG"].ToString());

		//转炉和连铸区域不校验处理号
		if(tpssmd1["AREA_ID"].ToDecimal() ==3 || tpssmd1["AREA_ID"].ToDecimal() ==5 )
		{

			//Log::Trace("", __FUNCTION__, "tpssm12.PRACT_RCV_FLAG3333333=[{0}]", tpssm12["PRACT_RCV_FLAG"].ToString());
			if (tpssm12["PRACT_RCV_FLAG"].ToString().Trim()  == '1')
			{
				tpssm12["AREA_ID"]=tpssmd1["AREA_ID"];
				tpssm12.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO,AREA_ID");
				
				tpssm12["PRACT_RCV_FLAG"]=" ";
				dummy=tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
				//if (tpssm12["AREA_ID"].ToDecimal() ==5 || dummy==0)
				//{
				//	tpssm11["FACTORY_DIV"] =tpssm12["FACTORY_DIV"];
				//	tpssm11["HEAT_NO"] =tpssm12["HEAT_NO"];
				//	tpssm11["PRACT_RCV_FLAG"] ="1";
				//	tpssm11.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO");
				//}

				if (dummy==0)
				{
					tpssm11["FACTORY_DIV"]=tpssm12["FACTORY_DIV"];
					tpssm11["HEAT_NO"]=tpssm12["HEAT_NO"];
					tpssm11["PRACT_RCV_FLAG"]="1";
					tpssm11.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO");
				}
			}
			else
			{				
				tpssm12["AREA_ID"]=tpssmd1["AREA_ID"];
				tpssm12["PRACT_RCV_FLAG"]=" ";
				tpssm12.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO,AREA_ID");

				tpssm11["FACTORY_DIV"]=tpssm12["FACTORY_DIV"];
				tpssm11["HEAT_NO"]=tpssm12["HEAT_NO"];
				tpssm11["PRACT_RCV_FLAG"]=tpssm12["PRACT_RCV_FLAG"];
				tpssm11.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO");
			}
		}
		else if (tpssmd1["AREA_ID"].ToDecimal() == 4)//精炼区域
		{
			//查询所接收的炉次实绩是否存在
			dummy = 0;
			dummy=tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,DEV_CODE,PROC_NO");
			if (dummy <= 0)
			{
				CFormattable arguments[] = { tpssm12["HEAT_NO"].ToString(), tpssm12["DEV_CODE"].ToString(), tpssm12["PROC_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000161")/*制造命令号[{0}]下设备[{1}]的处理号[{2}]不存在，请联系维护人员。*/, arguments, 3); //格式化字符串
				//throw CApplicationException(-1,s.msg,log.Location);
			}
			//if(tpssm12["PRACT_RCV_FLAG"].ToString()[0]=='1')
			if (tpssm12["PRACT_RCV_FLAG"].ToString().Trim() == "1")
			{
				//对非精炼区，不按处理号收实绩标识;  
				//实绩新增时
				tpssm12["AREA_ID"]=tpssmd1["AREA_ID"];
				tpssm12.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO,DEV_CODE,PROC_NO");
				//判断是否所有的工序的实绩都接收到
				tpssm12["PRACT_RCV_FLAG"]=" ";
				dummy=tpssm12.QueryCount("FACTORY_DIV,HEAT_NO,PRACT_RCV_FLAG");
				//if (dummy == 0 || tpssm12["AREA_ID"].ToDecimal() == 5) //全收到实绩的工序, 或收到连铸实绩 
				//{
				//	//修改实绩接收标记
				//	tpssm11["FACTORY_DIV"] =tpssm12["FACTORY_DIV"];
				//	tpssm11["HEAT_NO"] =tpssm12["HEAT_NO"];
				//	tpssm11["PRACT_RCV_FLAG"] ="1";
				//	tpssm11.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO");
				//}

				if (dummy == 0)
				{
					tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssm11["HEAT_NO"] = tpssm12["HEAT_NO"];
					tpssm11["PRACT_RCV_FLAG"] = "1";
					tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,HEAT_NO");
				}
			}
			else
			{	  
				//对非精炼区，不按处理号收实绩标识
				//实绩删除时	
				tpssm12["AREA_ID"]=tpssmd1["AREA_ID"];
				tpssm12["PRACT_RCV_FLAG"]=" ";
				tpssm12.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO,AREA_ID,PROC_NO");

				tpssm11["FACTORY_DIV"]=tpssm12["FACTORY_DIV"];
				tpssm11["HEAT_NO"]=tpssm12["HEAT_NO"];
				tpssm11["PRACT_RCV_FLAG"]=tpssm12["PRACT_RCV_FLAG"];
				tpssm11.Update("PRACT_RCV_FLAG","FACTORY_DIV,HEAT_NO");
			}	
		}
		else if (tpssmd1["AREA_ID"].ToDecimal() == 2)
		{
			tpssm12["AREA_ID"] = tpssmd1["AREA_ID"];
			dummy = 0;
			dummy = tpssm12.QueryCount("FACTORY_DIV,AREA_ID,DEV_CODE,SM_PLAN_NO");
			if (dummy <= 0)
			{
				CFormattable arguments[] = { tpssm12["HEAT_NO"].ToString(), tpssm12["DEV_CODE"].ToString(), tpssm12["PROC_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000161")/*制造命令号[{0}]下设备[{1}]的处理号[{2}]不存在，请联系维护人员。*/, arguments, 3); //格式化字符串
				//throw CApplicationException(-1,s.msg,log.Location);
			}

			if (dummy > 0)
			{
				if (tpssm12["PRACT_RCV_FLAG"].ToString().Trim() == "1")
				{
					sqlstr = " UPDATE TPSSM12 SET PRACT_RCV_FLAG = '1' WHERE DEV_CODE = @DEV_CODE AND AREA_ID = 2 AND SM_PLAN_NO = @SM_PLAN_NO AND PRACT_RCV_FLAG = ' ' AND CHARGE_NO = "
						" ( SELECT MIN(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE = @DEV_CODE AND AREA_ID = 2 AND SM_PLAN_NO = @SM_PLAN_NO AND PRACT_RCV_FLAG = ' ') ";
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cmd_inq.Parameters.Set("SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();

					tpssm12["PRACT_RCV_FLAG"] = " ";
					dummy = tpssm12.QueryCount("FACTORY_DIV,SM_PLAN_NO,PRACT_RCV_FLAG");
					if (dummy == 0)
					{
						tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
						tpssm11["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
						tpssm11["PRACT_RCV_FLAG"] = "1";
						tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,SM_PLAN_NO");
					}
				}

				else
				{
					sqlstr = " UPDATE TPSSM12 SET PRACT_RCV_FLAG = ' ' WHERE DEV_CODE = @DEV_CODE AND AREA_ID = 2 AND SM_PLAN_NO = @SM_PLAN_NO AND PRACT_RCV_FLAG = '1' AND CHARGE_NO = "
						" ( SELECT MAX(CHARGE_NO) FROM TPSSM12 WHERE DEV_CODE = @DEV_CODE AND AREA_ID = 2 AND SM_PLAN_NO = @SM_PLAN_NO AND PRACT_RCV_FLAG = '1') ";
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cmd_inq.Parameters.Set("SM_PLAN_NO", tpssm12["SM_PLAN_NO"].ToString());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();

					tpssm11["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssm11["SM_PLAN_NO"] = tpssm12["SM_PLAN_NO"];
					tpssm11["PRACT_RCV_FLAG"] = tpssm12["PRACT_RCV_FLAG"];
					tpssm11.Update("PRACT_RCV_FLAG", "FACTORY_DIV,SM_PLAN_NO");
				}
			}

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
