/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   JHZHAO
Version:    1.0
Date:     2012-1-6
Description:修改月计划信息
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件


/*<remark>=========================================================
/// <summary>

/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm28_catch)


int f_pssm28_catch(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;

	CString factory_div = "";
	CString catch_div = "";   //0-查询  1-编制  2-下达  9-判定
	CString catch_result = "1";   //0-不可编辑  1-可编辑
	CDbCommand cmd_inq(conn);
	CString userid = "";
	int adm_count = 0;
	CString mac_adr = "";
	// 定义表的实体对象
	CModel tpssm26("TPSSM26");
	CString sqlstr = "";

	try
	{
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Trace(" ", __FUNCTION__, "rows =[{0}]", rows);
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		catch_div = bcls_rec->Tables[0].Rows[0]["CATCH_DIV"].ToString();
		Log::Trace(" ", __FUNCTION__, "factory_div =[{0}],catch_div={1}", factory_div, catch_div);
		tpssm26["FACTORY_DIV"] = factory_div;
		userid = s.userid;
		mac_adr = s.fore_mac;
		sqlstr = "select * from tpssm26 where FACTORY_DIV=@factory_div ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm26);
		}
		cmd_inq.Close();

		bcls_ret->Tables[0].Columns.Add(tpssm26);
		if (!bcls_ret->Tables[0].Columns.Contains("CATCH_RESULT"))bcls_ret->Tables[0].Columns.Add(DT_STRING, "CATCH_RESULT");

		bcls_ret->Tables[0].Rows.Clear();
		if (catch_div == "0")
		{

			sqlstr = "update tpssm26 set ARCHIVE_FLAG = @tpssm26.ARCHIVE_FLAG"
				",COMPANY_CODE = @tpssm26.COMPANY_CODE"
				" where FACTORY_DIV=@factory_div ";


			if (tpssm26["REC_REVISOR"].ToString() == userid)//同一个人
			{
				tpssm26["REC_REVISOR"] = userid;
				tpssm26["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tpssm26["ARCHIVE_FLAG"] = "2";
				tpssm26["COMPANY_CODE"] = s.fore_mac;
				cmd_inq.SetCommandText(sqlstr);
				Log::Info("", __FUNCTION__, "update sqlstr=[{0}]", sqlstr);
				cmd_inq.Parameters.Set("tpssm26.REC_REVISOR", tpssm26["REC_REVISOR"].ToString());
				cmd_inq.Parameters.Set("tpssm26.REC_REVISE_TIME", tpssm26["REC_REVISE_TIME"].ToString());
				cmd_inq.Parameters.Set("tpssm26.ARCHIVE_FLAG", tpssm26["ARCHIVE_FLAG"].ToString());
				cmd_inq.Parameters.Set("tpssm26.COMPANY_CODE", tpssm26["COMPANY_CODE"].ToString());
				cmd_inq.ExecuteNonQuery();
				catch_result = "1";
			}
			else
			{
				catch_result = "0";
			}
		}
		else
		{
			sqlstr = "update tpssm26 set REC_REVISOR = @tpssm26.REC_REVISOR "
				",REC_REVISE_TIME = @tpssm26.REC_REVISE_TIME"
				",ARCHIVE_FLAG = @tpssm26.ARCHIVE_FLAG"
				",COMPANY_CODE = @tpssm26.COMPANY_CODE"
				" where FACTORY_DIV=@factory_div ";
			if (tpssm26["ARCHIVE_FLAG"].ToString().Trim() == "1"
				&&tpssm26["REC_REVISOR"].ToString().Trim() != userid.Trim())
			{
				catch_result = "0";
			}
			else
			{
				catch_result = "1";

				if (catch_div == "1")
				{
					tpssm26.MergeFrom(bcls_rec->Tables[0].Rows[0]);
					tpssm26["REC_REVISOR"] = userid;
					tpssm26["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
					tpssm26["ARCHIVE_FLAG"] = "1";
					tpssm26["COMPANY_CODE"] = s.fore_mac;
					//tpssm26.Update("REC_REVISOR,REC_REVISE_TIME,ARCHIVE_FLAG,COMPANY_CODE", "FACTORY_DIV");
				}
				else if (catch_div == "2")
				{
					tpssm26.MergeFrom(bcls_rec->Tables[0].Rows[0]);
					tpssm26["REC_REVISOR"] = userid;
					tpssm26["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
					tpssm26["ARCHIVE_FLAG"] = "2";
					tpssm26["COMPANY_CODE"] = mac_adr;
					//tpssm26.Update("REC_REVISOR,REC_REVISE_TIME,ARCHIVE_FLAG,COMPANY_CODE", "FACTORY_DIV");
				}

				cmd_inq.SetCommandText(sqlstr);
				Log::Info("", __FUNCTION__, "update sqlstr=[{0}]", sqlstr);
				cmd_inq.Parameters.Set("tpssm26.REC_REVISOR", tpssm26["REC_REVISOR"].ToString());
				cmd_inq.Parameters.Set("tpssm26.REC_REVISE_TIME", tpssm26["REC_REVISE_TIME"].ToString());
				cmd_inq.Parameters.Set("tpssm26.ARCHIVE_FLAG", tpssm26["ARCHIVE_FLAG"].ToString());
				cmd_inq.Parameters.Set("tpssm26.COMPANY_CODE", tpssm26["COMPANY_CODE"].ToString());
				cmd_inq.ExecuteNonQuery();
			}



		}
		tpssm26.MergeTo(bcls_ret->Tables[0], true);


		adm_count = 0;
		sqlstr = "select COUNT(1) from tep0002 where code_class = 'PSAZ2N' AND CODE_DESC_1_CONTENT LIKE @USER_ADM ";
		cmd_inq.SetCommandText(sqlstr);
		//Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
		cmd_inq.Parameters.Set("USER_ADM", userid);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			adm_count = cmd_inq.GetInt32(1);
		}
		cmd_inq.Close();
		Log::Info("", __FUNCTION__, "adm_count=[{0}]", adm_count);
		if (adm_count >= 0)
		{
			catch_result = "1";
		}
		CDataRow & row = bcls_ret->Tables[0].Rows[0];
		row["CATCH_RESULT"] = catch_result;
	}//try结束
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

