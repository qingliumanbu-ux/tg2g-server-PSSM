/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-02-08
功能: 计划日制造命令信息释放
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
/// 将计划日制造命令信息释放
/// <para > 
/// 1.根据传入的CAST - LOT号，进行制造命令释放。
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表) </para > 
/// <para > 主调用函数：前台PSSM04画面F5(释放)调用。   </para > 
/// </summary > 
/// <param name = "CAST_LOT_NO" > 浇铸批号  </param > 
/// <returns > 日制造命令信息</returns > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssm04_rel);

int f_pssm04_rel(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	int logFlag = 1;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	CString strCast_lot_vol;

	//定义变量--取系统当前时间
	CString datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	/*实体类定义*/
	CModel tpssm01("TPSSM01");

	/******业务处理开始******/
	try 
	{
		/*循环处理*/
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count();i++)
		{  
			tpssm01.Reset();
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			/*获取前台输入数据*/ 
			strCast_lot_vol = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO"];/*浇铸批号*/
			if (strCast_lot_vol != " ")
			{
				/*设置封锁信息*/
				tpssm01["HOLD_MAKER"] = s.userid;
				tpssm01["HOLD_TIME"] = datetimeNow;
				/*建立查询数据SQL语句*/
				sqlstr = " UPDATE  TPSSM01" 
					     " SET   PONO_STATUS = 11  , "
					     "       HOLD_MAKER = @hold_planner, "
					     "       HOLD_TIME = @hold_time"
					     " WHERE PONO_STATUS = 12 AND   "
					     "       CAST_LOT_NO = @cast_lot_no";

				/*给修改SQL赋条件值*/
				CDbCommand cmd_sql(sqlstr,conn);
				cmd_sql.Parameters.Set("hold_planner", tpssm01["HOLD_MAKER"].ToString());
				cmd_sql.Parameters.Set("hold_time", tpssm01["HOLD_TIME"].ToString());
				cmd_sql.Parameters.Set("cast_lot_no", strCast_lot_vol);
				cmd_sql.ExecuteNonQuery();
			}
		}

		/*返回处理信息*/
		strcpy(s.msg,_RES("GCRSS0000002")/*处理成功。*/);	
		//strcpy(s.msg,_RES("PSSMS0000197")/*释放成功。*/);
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


