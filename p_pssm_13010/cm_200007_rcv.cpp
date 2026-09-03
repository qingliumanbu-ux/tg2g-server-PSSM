/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-07-22
Description: MMS系统接受PES的命令应答
**************************************************************************************************************/

/***** C++ 的标准头文件部分 *****/ 
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/ 

  

  

#include "epex.h" 
/* ***** 静态函数申明 ***** */
#if defined _SYS_MMS && defined _LINE_HP
int f_pmouhp_send_pes(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif
/*<remark>=========================================================
/// <summary>
/// MMS系统接受PES的命令应答
/// <para>
/// 1.读取传入参数
/// 2.调用厚板生产函数
/// </para>
/// </summary>
/// <param name="FACTORY_DIV">主工序代码</param>
/// <param name="PONO">制造命令号</param>
/// <param name="ACK_CODE">应答</param>
/// <returns>炉次制造命令。</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE_TELE(cm_200007_rcv)


int f_cm_200007_rcv(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	/*程序用变量*/
	int doFlag = 0;

	/*业务变量*/
  CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString v_mat_destion;
	CString v_pono;
	CString v_ack_code;
	CString v_factory_div;
	CString sqlstr;
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息。
    /*实体类定义*/
	CDbCommand cmd_inq(conn); //与DB 建立连接。
	CDbCommand cmd_sql(conn);
	CDbCommand cmd_tpssm01_inq(conn);

	/*数据库操作类定义*/
	
	CModel tpssm03("TPSSM03");
	CModel tpssm01("TPSSM01");
	int blkNum = 0;
	int ret = 0;

	//调用厚板发送四大命令
	blkNum = bcls_rec->Tables.IndexOf("SEND");
	if (blkNum < 0)
	{
		bcls_rec->Tables.Add("SEND");
		bcls_rec->Tables["SEND"].Columns.Add(DT_STRING, "MAT_DESIGN");
		bcls_rec->Tables["SEND"].Columns.Add(DT_STRING, "PONO");
		bcls_rec->Tables["SEND"].Columns.Add(DT_STRING, "REPEAT_SEND");
	}
	
	try
	{
				
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
			v_pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().TrimOrBlank().ToUpper();
		
		if (bcls_rec->Tables[0].Columns.Contains("ACK_CODE"))
			v_ack_code =  bcls_rec->Tables[0].Rows[0]["ACK_CODE"].ToString().TrimOrBlank().ToUpper();
		
		if (bcls_rec->Tables[0].Columns.Contains("FACTORY_DIV"))
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().TrimOrBlank();
		
		////Log::Info("", __FUNCTION__, "接收应答PONO=[{0}], ACK_CODE=[{1}],FACTORY_DIV=[{2}]", v_pono, v_ack_code, v_factory_div);
		if (v_ack_code == "0")//炉次收回成功
		{

		}
		else if (v_ack_code == "1")
		{
			//炉次收回失败
			strcpy(s.msg, "炉次收回 failed.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		else if (v_ack_code == "2")//计划下发接收成功
		{
			tpssm01["PONO"] = v_pono;
			
			sqlstr = " SELECT * FROM TPSSM01 WHERE PONO = @tpssm01.PONO ";

			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm01_inq.ExecuteReader();

			if (cmd_tpssm01_inq.Read())
			{
				cmd_tpssm01_inq.Fetch(tpssm01);
#if defined _SYS_MMS && defined _LINE_HP
				if(bcls_rec->Tables["SEND"].Rows.get_Count() <= 0)
						bcls_rec->Tables["SEND"].Rows.Add();

				bcls_rec->Tables["SEND"].Rows[0]["MAT_DESIGN"] = "C";
				bcls_rec->Tables["SEND"].Rows[0]["PONO"] = tpssm01["PONO"];

				////Log::Info("", __FUNCTION__, "----f_pmouhp_send_pes START ----");
				doFlag = f_pmouhp_send_pes(bcls_rec, bcls_ret, conn);
				////Log::Info("", __FUNCTION__, "----f_pmouhp_send_pes END ----");

				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
#endif
			}
			cmd_tpssm01_inq.Close();

			//更新修改者，修改时间

			//tpssm01["REC_REVISOR"] = CString(s.userid);
			tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tpssm01["PONO"] = v_pono;
			tpssm01["FACTORY_DIV"] = v_factory_div;
			tpssm01["PONO_STATUS"] = 15;
			tpssm01["RES_CODE"] = v_ack_code;

			v_update = "REC_REVISE_TIME, PONO_STATUS, RES_CODE";//修改字段信息。
			v_condi = "FACTORY_DIV,PONO"; //查询条件
			if (tpssm01.Update(v_update, v_condi) != true)
			{
				strcpy(s.msg, "Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			strcpy(s.msg, "计划下发接收成功.");
			//throw CApplicationException(-1, s.msg, log.Location);
		}
		else if (v_ack_code == "3")
		{
			
		}
		else if (v_ack_code == "4")
		{
			
		}
		else
		{
			//炉次收回失败
			strcpy(s.msg, "炉次收回 failed.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
	 }

	 catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
