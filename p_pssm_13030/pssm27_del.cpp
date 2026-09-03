/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-07-19
功能: 删除交接部出钢记号转换表
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm17.h"
#include "tpssmerrm.h"

/*<remark >========================================================= 
/// <summary > 
/// 修改交接部出钢记号转换表
/// <para > 
/// 1.根据传入的交接部主出钢记号、出钢记号1删除数据库中对应的信息；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F5(删除)调用。   </para > 
/// </summary > 
/// <param name = "ST_NO" > 主出钢记号  </param > 
/// <param name = "ST_NO1" > 出钢记号1  </param > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm27_del)

int f_pssm27_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	/*实体类定义*/

	CTPSSM17 tpssm17(conn);
	CTPSSM17 tpssm17_mirror(conn);
	CTPSSMERRM tpssmerrm(conn);
	/******业务处理开始******/

	try
	{
		// 传入块中第一个表的行数
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		for( int i = 0; i < rowCount;  i++)
		{
			//将对象字段重置为默认值
			tpssm17.Reset();
			tpssmerrm.Reset();
			// 获取前台传入参数
			tpssm17.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			// 执行删除,失败抛出异常
			sqlstr = CString(" tpssm17.Delete()");
			//如果记录对应的标记为I，可以直接删除（还没有发送至L3）
			tpssm17.SEND_FLAG = "I";
			tpssm17.Delete("ST_NO,ST_NO1,SEND_FLAG");
			
            Log::Trace("", __FUNCTION__, "***********************************************分割线 [{0}]*******************************************************", s.datetime);


			tpssm17.REC_REVISE_TIME = s.datetime;
			tpssm17.REC_REVISOR = s.userid;
			tpssm17.SEND_FLAG = "D";
			tpssm17.Update("REC_REVISE_TIME,REC_REVISOR,SEND_FLAG","ST_NO,ST_NO1");
            //add by tuxianji 20140228 TPSSM17删除留档备查
			
			tpssmerrm.REC_CREATOR = s.userid;
			tpssmerrm.REC_CREATE_TIME = s.datetime;
			tpssmerrm.REC_REVISE_TIME = tpssmerrm.REC_CREATE_TIME;
			tpssmerrm.REC_REVISOR = s.svc_name;
			tpssmerrm.ERROR_EVENT_DATE = CString(s.datetime).Substring(0,8);
			tpssmerrm.ERROR_TIME =s.datetime;
			tpssmerrm.ERROR_PROGRAM = "pssm17_del";
			tpssmerrm.ERROR_SQL_CODE = 0;
			tpssmerrm.ERROR_STATUS = "DELETE";
			Log::Trace("", __FUNCTION__, "ERROR_STATUS = [{0}]", tpssmerrm.ERROR_STATUS);
			tpssmerrm.EVENT_TABLE = "TPSSM17";
			tpssmerrm.ERROR_REASON = "主出钢记号:"+tpssm17.ST_NO+"副出钢记号："+tpssm17.ST_NO1+"发送标记："+tpssm17.SEND_FLAG+"修改者："+tpssm17.REC_REVISOR+"修改时间："+tpssm17.REC_REVISE_TIME;
			//写处理履历
			tpssmerrm.Insert();
            Log::Trace("", __FUNCTION__, "ERROR_REASON = [{0}]", tpssmerrm.ERROR_REASON);

			tpssm17_mirror.CopyFrom(tpssm17);


			tpssm17_mirror.ST_NO = tpssm17.ST_NO1;
			tpssm17_mirror.ST_NO1 = tpssm17.ST_NO;
			tpssm17_mirror.SEND_FLAG = "I";

			tpssm17_mirror.Delete("ST_NO,ST_NO1,SEND_FLAG");
			tpssm17_mirror.SEND_FLAG = "D";
			tpssm17_mirror.Update("REC_REVISE_TIME,REC_REVISOR,SEND_FLAG","ST_NO,ST_NO1");
		     Log::Trace("", __FUNCTION__, "END-- SEND_FLAG= [{0}]", tpssm17_mirror.SEND_FLAG);


		}//for


		//设置系统返回消息，国际化信息
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000020")/*删除信息失败。*/, arguments, 1);
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