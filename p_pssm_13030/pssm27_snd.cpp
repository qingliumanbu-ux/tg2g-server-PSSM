/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-07-20
功能: 交接部出钢记号转换表下发
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
int f_pssm2500_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
int f_pssm1500_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);

/*<remark >========================================================= 
/// <summary > 
/// 交接部出钢记号转换表下发
/// <para > 
/// 下发交接部出钢记号转换信息；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F12(下达)调用。   </para > 
/// </summary > 
/// <returns > 下发成功失败信息</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm27_snd)

int f_pssm27_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	CString ST_NO = "";
	/*实体类定义*/

	/******业务处理开始******/

	try
	{


		bcls_rec->Tables.Add("PSSM27_SND");

		//定义查询
		sqlstr = " SELECT "
			     " DISTINCT(ST_NO) "
				 " FROM TPSSM17 "
				 " WHERE SEND_FLAG IN ('I','D','U') "
				 " ORDER BY ST_NO";

		//查询出结果
		CDbCommand comm_inq(sqlstr,conn);
		comm_inq.ExecuteQuery(bcls_rec->Tables["PSSM27_SND"]);
		if (bcls_rec->Tables["PSSM27_SND"].Rows.get_Count() <= 0)
		{
			sprintf(s.msg,"没有可以下发的记录。");
		}
		else
		{
			doFlag = f_pssm2500_snd(bcls_rec,bcls_ret,conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1,s.msg,log.Location);
			}
			doFlag = f_pssm1500_snd(bcls_rec,bcls_ret,conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1,s.msg,log.Location);
			}

			sqlstr = " UPDATE TPSSM17 "
				     " SET SEND_FLAG = 'S' "
					 " ,TC_TRANS_TIME = @tc_trans_time "
					 " WHERE  SEND_FLAG IN ('I','U')";
			CDbCommand comm_upd (sqlstr,conn);
			comm_upd.Parameters.Set("tc_trans_time",CDateTime::Now().ToString("yyyyMMddHHmmss"));
			comm_upd.ExecuteNonQuery();

			sqlstr = "DELETE FROM TPSSM17 WHERE SEND_FLAG = 'D'";
			CDbCommand comm_del (sqlstr,conn);
			comm_del.ExecuteNonQuery();
			strcpy(s.msg, "电文发送成功。");
		}


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
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

	return(doFlag);
}