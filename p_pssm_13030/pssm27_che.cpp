/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2014-02-27
功能: 交接部出钢记号转换表检查
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm17.h"
#include "tpssmerrm.h"

/*<remark >========================================================= 
/// <summary > 
/// 交接部出钢记号转换表检查
/// <para > 
/// 查询交接部出钢记号转换表；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F9(检查)调用。   </para > 
/// </summary > 
/// <param name = "ST_NO" > 主出钢记号  </param > 
/// <returns > 检查返回信息</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm27_che)

int f_pssm27_che(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/******业务处理开始******/

	try
	{
		// 定义表的实体对象
		CTPSSM17 tpssm17(conn);
		CTPSSMERRM tpssmerrm(conn);

		//定义数据库操作命令对象comm_inq执行sql语句，sql字符串用""包括，可以分行，但每行前后务必留出一个空格。
		sqlstr = " SELECT  * FROM TPSSM17 WHERE ST_NO||ST_NO1 NOT IN(select ST_NO1||ST_NO from TPSSM17) ";

		//获取记录数
		CDbCommand cmd(sqlstr, conn);
		cmd.ExecuteReader();
		while(cmd.Read())
		{
			tpssm17.Reset();
			cmd.Fetch(tpssm17);
			if(tpssm17.ST_NO!=""&&tpssm17.ST_NO1!="")
			{
				tpssmerrm.REC_CREATOR = s.userid;
				tpssmerrm.REC_CREATE_TIME = s.datetime;
				tpssmerrm.REC_REVISE_TIME = tpssmerrm.REC_CREATE_TIME;
				tpssmerrm.REC_REVISOR = s.svc_name;
				tpssmerrm.ERROR_EVENT_DATE = CString(s.datetime).Substring(0,8);
				tpssmerrm.ERROR_TIME = s.datetime ; 
				tpssmerrm.ERROR_PROGRAM = "pssm17_che";
				tpssmerrm.ERROR_SQL_CODE = 0;
				tpssmerrm.ERROR_STATUS = "SELECT";
				tpssmerrm.EVENT_TABLE = "TPSSM17";
				tpssmerrm.ERROR_REASON = "TPSSM17表主出钢记号："+tpssm17.ST_NO+"副出钢记号："+ tpssm17.ST_NO1+"无匹配异常出错";

				//写出错处理履历
		        tpssmerrm.Insert();
			}

		}
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  
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