/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-07-19
功能: 修改交接部出钢记号转换表
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm17.h"

/*<remark >========================================================= 
/// <summary > 
/// 修改交接部出钢记号转换表
/// <para > 
/// 1.根据传入的交接部出钢记号转换信息修改数据库中对应信息；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F4(修改)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > =========================================================== </remark > */
// Service 入口
BM2F_ENTERACE(pssm27_upd)

int f_pssm27_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值


	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString sel_max = "SELECT NVL(MAX(SEQ_NO),0) + 1 FROM TPSSM17";

	/*业务变量*/

	/*实体类定义*/
	CTPSSM17 tpssm17(conn);
	CTPSSM17 tpssm17_old(conn);
	CTPSSM17 tpssm17_mirror(conn);

	CDbCommand cmd_upd(conn);
	CDbCommand cmd_seq(conn);

	/*获取当前日期*/
	CString dateNow = CDateTime::Now().ToString("yyyyMMdd");	//取系统日期
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");//取系统时间 

	try
	{

		// 传入块中第一个表的行数
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		for( int i = 0; i < rowCount;  i++)
		{
			//将对象字段重置为默认值
			tpssm17.Reset();

			// 获取前台传入参数
			tpssm17.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			if (bcls_rec->Tables.get_Count() >= 2)
			{
				tpssm17_old.MergeFrom(bcls_rec->Tables[1].Rows[i]);
			}

			tpssm17.REC_REVISE_TIME = s.datetime;
			tpssm17.REC_REVISOR = s.userid;

			if (tpssm17.SEND_FLAG == "D")
			{
				CFormattable arguments[] = { tpssm17.SEND_FLAG};
				CMessageFormat::Format(s.msg, "tpssm17.SEND_FLAG = [{0}],已删除，不能进行修改", arguments, 1);
				s.flag = -1;                 
				throw CApplicationException(-1,s.msg,log.Location);
			}
			if (tpssm17.SEND_FLAG != "I" )
			{
				tpssm17.SEND_FLAG = "U";
			}

			sqlstr = CString("tpssm17.Update()");
			int count = tpssm17.Update("REC_REVISOR,REC_REVISE_TIME,ST_NO2,ST_NO3,SEND_FLAG,DECI_FLAG","ST_NO,ST_NO1");
			if(count <= 0)       //封装的修改方法
			{
				sprintf(s.msg, "修改的记录 tpssm17.ST_NO[ % ],tpssm17.ST_NO1[ % s]在数据库表中不存在。",(const char * )tpssm17.ST_NO,(const char * )tpssm17.ST_NO1);
				s.flag = -1;                 
				throw CApplicationException(-1,s.msg,log.Location);
			}

			//开始操作原记录的镜像
			tpssm17_mirror.CopyFrom(tpssm17);
			tpssm17_mirror.ST_NO = tpssm17.ST_NO1;
			tpssm17_mirror.ST_NO1 = tpssm17.ST_NO;
			switch (tpssm17.DECI_FLAG[0])
			{
			case '1':
				break;
			case '2':
				tpssm17_mirror.DECI_FLAG = "3";
				break;
			case '3':
				tpssm17_mirror.DECI_FLAG = "2";
				break;
			case '4':
				break;
			case '5':
				tpssm17_mirror.ST_NO2 = tpssm17.ST_NO3;
				tpssm17_mirror.ST_NO3 = tpssm17.ST_NO2;
				break;
			default:
				sprintf(s.msg, "tpssm17.DECI_FLAG[ % ],不在设定的范围[1-5]之间，请重新输入",(const char * )tpssm17.DECI_FLAG);
				s.flag = -1;                 
				throw CApplicationException(-1,s.msg,log.Location);
				break;
			}
			if (tpssm17_mirror.QueryCount("ST_NO,ST_NO1") > 0)
			{
				tpssm17_mirror.REC_REVISOR = s.userid;
				tpssm17_mirror.REC_REVISE_TIME = s.datetime;
				if (tpssm17.SEND_FLAG != "I" )
				{
					tpssm17.SEND_FLAG = "U";
				}
				tpssm17_mirror.Update("ST_NO2,ST_NO3,DECI_FLAG,SEND_FLAG,REC_REVISOR,REC_REVISE_TIME","ST_NO,ST_NO1");
			}
			else
			{
				cmd_seq.SetCommandText(sel_max);
				cmd_seq.ExecuteReader();
				if(cmd_seq.Read()) 
				{
					tpssm17_mirror.SEQ_NO = cmd_seq.GetDecimal(1) ; //将数据获取到实体对象中
				}
				cmd_seq.Close();
				tpssm17_mirror.REC_CREATE_TIME = s.datetime;
				tpssm17_mirror.REC_CREATOR = s.userid;
				tpssm17_mirror.SEND_FLAG = "I";
				tpssm17_mirror.Insert();
			}

		}//for

		//设置系统返回消息，国际化信息
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { "TPSSM17", ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000009")/*修改记录失败，表[{0}]，sqlcode = [{1}]。请联系系统维护人员。*/, arguments, 2);
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