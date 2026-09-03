/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   chejs
Version:    1.0
Date:     2015-09-10 17:13:56
Description: 炼钢日出钢能力（PSSM21）-预计划编制后的规程校验
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/
#include "tpssm01.h"
#include "tpssm21.h"
#include "tpssm23.h"

/* ***** 静态函数申明 ***** */
int f_pssm21_chk_capa(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//炼钢连铸机作业时间(出钢能力)校验


//-----------------------------------------------------------------------
//功能描述:		预计划编制后的规程校验
//数据库表:     TPSSM01
//表中文名:     炼钢连铸制造命令炉次表
//主调用函数:   前台 PSSM21画面(制造命令编制)调用
//需调用函数:   f_pssm01_chk_capa
//-----------------------------------------------------------------------
//函数功能:     预计划编制后的规程校验
//传入参数:     
//传出参数:     
//处理流程:     
//1.根据前台传入连铸机号, 生产日期进行查询
//=========================================================================*/

// service入口
BM2F_ENTERACE(pssm21_chk)

int f_pssm21_chk(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int ret = 0;
	int i = 0;
	int rows = 0;
	int chk_flag = 0;  //chk_flag=0: 没有违规; chk_flag=1:有违规;
	int num_wk = 0; //规程校验不合的记录数量
	CString date_Now = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString blk_name = ""; //块名

	CString sqlstr = "";

	CTPSSM01 tpssm01(conn);
	CTPSSM21 tpssm21(conn);
	CTPSSM23 tpssm23(conn);

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm23_inq(conn);  //与DB 建立连接。

	try
	{
		//设定返回块的参数
		bcls_ret->Tables[0].set_TableName("NUM_WK");  //返回块
		bcls_ret->Tables[0].Columns.Add(DT_INT32, "num_wk"); //规程校验不合的记录数量

		//----------------------------------------------------------------------
		//获取传入参数
		tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		/* ***** 打印输入参数 ***** */
		Log::Info("", __FUNCTION__, "pssm21_chk>FACTORY_DIV = [{0}]", tpssm01.FACTORY_DIV);
		Log::Info("", __FUNCTION__, "pssm21_chk>CC_MACH_NO = [{0}]", tpssm01.CC_MACH_NO);
		Log::Info("", __FUNCTION__, "pssm21_chk>PLAN_DATE = [{0}]", tpssm01.PLAN_DATE);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " DELETE FROM TPSSM23 "
				"WHERE FACTORY_DIV = @FACTORY_DIV "
				"AND PLAN_DATE = @PLAN_DATE ";
			break;
		}
		cmd_tpssm23_inq.SetCommandText(sqlstr);
		cmd_tpssm23_inq.Parameters.Set("FACTORY_DIV", tpssm01.FACTORY_DIV);
		cmd_tpssm23_inq.Parameters.Set("PLAN_DATE", tpssm01.PLAN_DATE);
		cmd_tpssm23_inq.ExecuteNonQuery();
		cmd_tpssm23_inq.Close();

		tpssm23.SEQ_NO = 0; //序号

		//----------------------------------------------------------------------
		//4.日计划炉数的有效作业能力校验
		blk_name = "capa"; //定义返回的块名
		ret = f_pssm21_chk_capa(bcls_rec, bcls_ret,conn);
		if (ret < 0)//函数调用出错
		{
			Log::Info("", __FUNCTION__, "调用函数f_pssm21_chk_capa出错");

			bcls_ret->GetSYS(&s);
			
			tpssm23.FACTORY_DIV = tpssm01.FACTORY_DIV;
			tpssm23.PLAN_DATE = tpssm01.PLAN_DATE;
			tpssm23.ERR_CODE = " ";
			tpssm23.ERR_DESC = s.msg;
			tpssm23.SEQ_NO = tpssm23.SEQ_NO + 1;

			tpssm23.REC_CREATE_TIME = date_Now;
			tpssm23.REC_CREATOR = s.userid;
			tpssm23.REC_REVISE_TIME = date_Now;
			tpssm23.REC_REVISOR = s.userid;
			tpssm23.COMPANY_CODE = s.company_code;
			tpssm23.COMPANY_NAME = s.company_name;

			tpssm23.Print();
			
			sqlstr = "tpssm23.Insert(capa)";
			tpssm23.Insert();
		}
		else
		{
			Log::Info("", __FUNCTION__, "成功调用f_pssm21_chk_capa函数");

			/*if (blkseq > 0)
			{
				
			}*/
			//rows = bcls_ret->GetRow(blkseq);
			rows = bcls_ret->Tables[0].Rows.get_Count();
			Log::Info("", __FUNCTION__, "rows = [{0}]", rows);
			for (i = 0; i < rows; i++)
			{
				chk_flag = bcls_ret->Tables[0].Rows[i]["chk_flag"]; //违规标记
				CString station_name = bcls_ret->Tables[0].Rows[i]["station_name"]; //工序设备名称
				CString limit_capa = bcls_ret->Tables[0].Rows[i]["limit_capa"]; //限制炉数
				CString plan_charge = bcls_ret->Tables[0].Rows[i]["plan_charge"]; //编入计划的炉数

				Log::Info("", __FUNCTION__, "chk_flag = [{0}]", chk_flag);
				Log::Info("", __FUNCTION__, "station_name = [{0}]", station_name);
				Log::Info("", __FUNCTION__, "limit_capa = [{0}]", limit_capa);
				Log::Info("", __FUNCTION__, "plan_charge = [{0}]", plan_charge);
				Log::Info("", __FUNCTION__, "------------------------------------");

				if (chk_flag == 1)
				{
					tpssm23.ERR_CODE = "4";//4-日计划炉数大于日作业能力
					tpssm23.ERR_DESC = CString::Format("工序[%s]的计划炉数[%d]超出当日的生产能力[%d]炉", station_name, plan_charge, limit_capa);
					tpssm23.SEQ_NO = tpssm23.SEQ_NO + 1;

					tpssm23.FACTORY_DIV = tpssm01.FACTORY_DIV;
					tpssm23.PLAN_DATE = tpssm01.PLAN_DATE;

					tpssm23.REC_CREATE_TIME = date_Now;
					tpssm23.REC_CREATOR = s.userid;
					tpssm23.REC_REVISE_TIME = date_Now;
					tpssm23.REC_REVISOR = s.userid;
					tpssm23.COMPANY_CODE = s.company_code;
					tpssm23.COMPANY_NAME = s.company_name;

					sqlstr = "tpssm23.Insert(4)";
					tpssm23.Insert();

				}
			}
		}

		//----------------------------------------------------------------------
		//统计校验的结果记录数量，供前台画面对话框用
		tpssm23.FACTORY_DIV = tpssm01.FACTORY_DIV;
		tpssm23.PLAN_DATE = tpssm01.PLAN_DATE;
		num_wk = tpssm23.QueryCount("FACTORY_DIV,PLAN_DATE");
		Log::Info("", __FUNCTION__, "num_wk = [{0}]", num_wk);

		bcls_ret->Tables[0].Rows[0]["num_wk"] = num_wk;
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
