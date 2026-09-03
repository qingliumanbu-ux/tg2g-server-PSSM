/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Date:     2015-07-16
Version:  3.1.0
Description:  PES系统接受对MMS系统下发的板坯制造命令进行接收
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



#include "epex.h"

// service入口
BM2F_ENTERACE_TELE(cm_002134_rcv)

int f_cm_002134_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int    doFlag = 0;
	CString   datetime = "";
	int    dummy = 0;
	int    blkseq = 0;

	CDbCommand cmd_inq(conn);
	CString sqlstr = "";
	CString xml_info = "";
	CDecimal resume_seq_no = 0;

	try
	{
		sqlstr = "  SELECT DA_SCHEDULE_UPDATE_SEQ.nextval FROM dual  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			resume_seq_no = cmd_inq.GetDecimal(1);
		}

		xml_info = bcls_rec->Tables["broner"].Rows[0]["xml_info"].ToString();
		sqlstr = " INSERT INTO CX_INT_MSCC_RECEIVE_XML (ID,MESSAGE_NO,STATUS,XML_DATA,PRIORITY,T_CREATED,T_MODIFIED,REMARK,MQ_MSG_ID,DESTINATION,PPREAD) "
			" VALUES(@resume_seq_no, @message_no, @status, @xml_info, @priority, SYSDATE, SYSDATE, @remark, @mq_msg_id, @dest, @ppread) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("resume_seq_no", resume_seq_no);
		cmd_inq.Parameters.Set("message_no", 9001);
		cmd_inq.Parameters.Set("status", 0);
		cmd_inq.Parameters.Set("xml_info", xml_info);
		cmd_inq.Parameters.Set("priority", 1);
		cmd_inq.Parameters.Set("remark", "NIL");
		cmd_inq.Parameters.Set("mq_msg_id", "NIL");
		cmd_inq.Parameters.Set("dest", "MES");
		cmd_inq.Parameters.Set("ppread", 0);
		cmd_inq.ExecuteNonQuery();

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
