/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:chejs
Date:2015-10-12
Version:1.0
Description: 转炉处理号修改
Update:
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




//int f_pssm12_sequ_calc_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划各工序作业顺序号生成（包括HEAT_NO）
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 转炉处理号修改
/// <para>甘特图转炉处理号修改。                            </para>
/// <para>数据库表：tpssm25                   </para>
/// <para>主调用函数：PSSM21O2画面调用。                </para>
/// </summary>
/// <param name="STATION_ID">处理工位标识          </param>
/// <param name="STATION_NO">处理工位号     </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_ul_upd)
//-EP_SYSTEM_HEAD_END
int f_pssm18_ul_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret = 0;
	CDecimal dummy = 0;

	CString sqlstr = "";
	CString dev_code = "" , factory_div = ""; //设备代码
	CString proc_no = ""; //处理号
	CString proc_no_old = ""; //原处理号
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString year = "";

	CModel tpssm11("TPSSM11");
	CModel tpssm25("TPSSM25");
	CModel tpssm99("TPSSM99");
	CDbCommand cmd_tpssm12_inq(conn);

	EIClass inblk;        //调用函数用
	EIClass in_pssm99trace;//调用履历函数
	//计划履历按一炉为单位
	in_pssm99trace.Tables[0].set_TableName("TRACE");

	try
	{
		//1、总体计划块
		inblk.Tables[0].set_TableName("PLAN");  //
		inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
		inblk.Tables[0].Rows.Add();

		

		//获取传入参数
		dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"];
		proc_no = bcls_rec->Tables[0].Rows[0]["PROC_NO"];
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		//打印传入参数
		////Log::Trace("", __FUNCTION__, "dev_code=[{0}],proc_no=[{1}],factory_div=[{2}]", dev_code, proc_no, factory_div);
		tpssm25["FACTORY_DIV"] = factory_div;
		tpssm25["STATION_ID"] = dev_code.SubstringNE(0, 1);
		tpssm25["STATION_NO"] = dev_code.SubstringNE(1, 1);
		////Log::Trace("", __FUNCTION__, "tpssm25["STATION_ID"] = [{0}],tpssm25["STATION_NO"] = [{1}]", tpssm25["STATION_ID"].ToString(), tpssm25["STATION_NO"].ToString());

		year = proc_no.SubstringNE(0, 2);
		////Log::Trace("", __FUNCTION__, "year = [{0}]", year);

		if (strlen(proc_no) != 9)
		{
			CFormattable arguments[] = { proc_no };	//定义参数列表的数组
			CMessageFormat::Format(s.msg, "熔炼号[{0}]位数错误，请检查！", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		ret = tpssm25.QueryCount("STATION_ID,STATION_NO,FACTORY_DIV");
		////Log::Trace("", __FUNCTION__, "查询25表记录数ret=[{0}]", ret);
		if (ret != 1)
		{
			CFormattable arguments[] = { dev_code };	//定义参数列表的数组
			CMessageFormat::Format(s.msg, "设备代码[{0}]炼钢计划处理号维护错误，请检查！", arguments, 1);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		tpssm25.Query("STATION_ID,STATION_NO,FACTORY_DIV");

		proc_no_old = tpssm25["CURR_PROC_NO"];
		if (proc_no_old.Compare(proc_no) == 0)
		{
			strcpy(s.msg, "处理号与原处理号相同，请重新输入处理号。");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		tpssm11["HEAT_NO"] = proc_no_old;
		tpssm11["FACTORY_DIV"] = factory_div;
		tpssm11.Query("HEAT_NO,FACTORY_DIV");



		//switch (conn->DatabaseKind)
		//{
		//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		//case DB_KIND_MSSQL:	        // MS SQL Server数据库
		//case DB_KIND_ORACLE:	        // Oracle 数据库
		//default:

		//	sqlstr = " SELECT COUNT(1) "
		//		" FROM TPSSM12 T "
		//		" WHERE T.DEV_CODE = @DEV_CODE "
		//		" AND T.PRE_PROC_NO > @CURR_PROC_NO ";
		//	break;
		//}

		//cmd_tpssm12_inq.SetCommandText(sqlstr);

		//cmd_tpssm12_inq.Parameters.Set("DEV_CODE", dev_code);
		//cmd_tpssm12_inq.Parameters.Set("CURR_PROC_NO", proc_no);

		//dummy = cmd_tpssm12_inq.ExecuteScalar();

		//if (dummy > 0)
		//{
		//	CFormattable arguments[] = { proc_no };	//定义参数列表的数组
		//	CMessageFormat::Format(s.msg, "熔炼号[{0}]后有炉次已经开始生产，不能执行。", arguments, 1);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		tpssm25["CURR_PROC_NO"] = proc_no;
		tpssm25["REC_REVISE_TIME"] = datetime;
		tpssm25["REC_REVISOR"] = s.userid;

		if (tpssm25.Update("CURR_PROC_NO,REC_REVISE_TIME,REC_REVISOR", "STATION_ID,STATION_NO,FACTORY_DIV") < 0)
		{
			strcpy(s.msg, "update failed。");
			throw CApplicationException(-1, s.msg, log.Location);
		}	

	

		//调用炼钢计划履历函数---记录之前的炉号和最新炉号,记录之前炉号对应的PONO信息
		tpssm99["FACTORY_DIV"] = factory_div;	
		tpssm99["PONO"] = tpssm11["PONO"];
		tpssm99["PONO_STATUS"] = tpssm99["PONO_STATUS"];
		tpssm99["EVENT_ID"] = "13";
		tpssm99["HEAT_NO"] = proc_no;//新的炉号
		tpssm99["HEAT_NO_OLD"] = proc_no_old;//老的炉号
		tpssm99["VALID_FLAG"] = "1";
		in_pssm99trace.Tables[0].Clone(tpssm99);
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

		////Log::Trace("", __FUNCTION__, "记录失败履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		//记录编入计划成功的履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
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
