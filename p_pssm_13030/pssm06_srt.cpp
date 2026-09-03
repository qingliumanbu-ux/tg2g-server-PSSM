/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2022
作者: 涂献计
日期: 2022-04-06
功能: 炼钢模铸预计划调整确定再排
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
/// 模铸预计划调整确定再排
/// <para > 
/// 1.根据传入的CAST - LOT号、修改制造命令信息。
/// 2.更新条件：CAST - LOT号；
/// 3.更新字段：制造命令状态,再排标记,生产日期；
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表)</para > 
/// <para > 主调用函数：前台pssm06画面F5(再排)调用。   </para > 
/// </summary > 
/// <param name = "CAST_LOT_NO" > CAST - LOT号  </param > 
/// <returns > 模铸预计划调整确定信息</returns > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssm06_srt);

int f_pssm06_srt(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	int logFlag = 1;
	int upd_count = 0;
	CString cast_lot_no;
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	/*实体类定义*/

	CString datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{	

		//记录总条数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		//循环处理
		for (int i = 0; i < count ; i++)
		{ 
			cast_lot_no = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO"].ToString();

			//根据传入信息查询表
			sqlstr = "UPDATE TPSSM01"; //用于捕获数据库操作异常情况
			CString sql_upd = " UPDATE TPSSM01 "
				              " SET PONO_STATUS = 11,"
							  " SCHE_FLAG = '*',"
				              " PLAN_DATE = ' ',"
							  " REC_REVISOR = @rec_revisor, "
							  " REC_REVISE_TIME = @rec_revise_time "
				              " WHERE  CAST_LOT_NO = @cast_lot_no "
							  " AND  PONO_STATUS = 14 ";
			CDbCommand cmmd(sql_upd, conn);
			cmmd.Parameters.Set("cast_lot_no",cast_lot_no);
			cmmd.Parameters.Set("rec_revisor",s.userid);
			cmmd.Parameters.Set("rec_revise_time",datetimeNow);
			upd_count = cmmd.ExecuteReader();

			if(upd_count < 0)   
			{
				strcpy(s.msg, "再排操作失败!");
				//_RES("PSSMS0000210")/*再排操作失败!*/
			}


		}	

		CFormattable arguments[] = {upd_count}; 
		CMessageFormat::Format(s.msg, _RES("PSSMS0000265")/*再排了[{0}]条记录。*/,	arguments, 1); 			

	}

	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
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
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}


