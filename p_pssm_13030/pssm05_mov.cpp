/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-03-19
功能: 炼钢连铸预计划调整确定序号移动
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
/// 炼钢连铸预计划调整确定序号移动 
/// <para > 
/// 将一段序号的记录移动到另一序号以后，重排序；
/// </para > 
/// <para > 数据库表：TPSSM01(炼钢连铸制造命令炉次表) </para > 
/// <para > 主调用函数：前台PSSM05画面F6(功能操作-移动)调用。 </para > 
/// <param name = "cc_seq" > 序号起始 </param > 
/// <param name = "cc_seq_end" > 序号结束位置 </param > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */

// Service 入口
BM2F_ENTERACE(pssm05_mov)

int f_pssm05_mov(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;

	CString  sqlstr("");
	int count = 0;
	int inq_count = 0;
	int cc_seq = 0;
	int cc_seq_new = 0;
	CString pono = "";

	try
	{
	CModel tpssm01("TPSSM01");
		CString	datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");//取系统时间       
		CString upd_cc_seq = "UPDATE TPSSM01"
							" SET CC_SEQ = @cc_seq,"
							" REC_REVISOR = @rec_revisor,"
							" REC_REVISE_TIME = @rec_revise_time"
							" WHERE PONO = @pono";
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		for( int i = 0; i < rowCount;  i++)
		{
			pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();
			cc_seq_new++;
			CDbCommand cmd_upd(conn);
			cmd_upd.SetCommandText(upd_cc_seq);
			cmd_upd.Parameters.Set("cc_seq",cc_seq_new);
			cmd_upd.Parameters.Set("pono",pono);
			cmd_upd.Parameters.Set("rec_revisor",s.userid);
			cmd_upd.Parameters.Set("rec_revise_time",datetime);
			cmd_upd.ExecuteReader();

		}

		/*设置系统返回消息，国际化信息*/
		strcpy(s.sysmsg,  _RES("GCRSS0000002"));//处理成功。  
		//_RES("PSSMS0000200")/*移动成功。*/

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { "TPSBW101B", ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000009")/*修改记录失败，表[{0}]，sqlcode = [{1}]。请联系系统维护人员。*/, arguments, 2);
		CString str = "DB error:" + sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch(const CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//返回-1时事务将回滚，返回为0是事务将提交
	bcls_ret->SetSYS(s);
	return(doFlag);
}
