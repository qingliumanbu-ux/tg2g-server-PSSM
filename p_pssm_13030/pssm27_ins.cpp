/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-07-19
功能: 新增交接部出钢记号转换表
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm17.h"

/*<remark >========================================================= 
/// <summary > 
/// 新增交接部出钢记号转换表
/// <para > 
/// 1.根据传入的交接部出钢记号转换信息写入数据库；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F3(新增)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm27_ins)

int f_pssm27_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值


	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString sel_max = " SELECT NVL(MAX(SEQ_NO),0) + 1 FROM TPSSM17 "
					  " WHERE ST_NO = @st_no ";
;

	/*业务变量*/

	/*实体类定义*/
	CTPSSM17 tpssm17(conn);
	CTPSSM17 tpssm17_mirror(conn);

	CDbCommand cmd_seq(conn);

	/*获取当前日期*/
	CString dateNow = CDateTime::Now().ToString("yyyyMMdd");	//取系统日期
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");//取系统时间  

	try
	{
		// 传入块中第一个表的行数
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		for(int i = 0;  i < rowCount;  i++)
		{
			//将对象字段重置为默认值
			tpssm17.Reset();

			// 获取前台传入参数
			tpssm17.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			int con = tpssm17.QueryCount("ST_NO,ST_NO1");
			if(con > 0)
			{
				sprintf(s.msg, "主出钢记号[%s],第一出钢记号[%s]数据已存在,请重新输入",(const char * ) tpssm17.ST_NO,(const char * )tpssm17.ST_NO1  );
				s.flag = -1;                 
				throw CApplicationException(-1,s.msg,log.Location);
			}
			tpssm17.SEND_FLAG = "I";
			tpssm17.REC_CREATE_TIME = s.datetime;
			tpssm17.REC_CREATOR = s.userid;
			tpssm17.REC_REVISE_TIME = tpssm17.REC_CREATE_TIME;
			tpssm17.REC_REVISOR = s.svc_name;

			//sel_max = "SELECT NVL(MAX(SEQ_NO),0) + 1 FROM TPSSM17 "
			//	      " WHERE ST_NO = @st_no ";
			cmd_seq.SetCommandText(sel_max);
			cmd_seq.Parameters.Set("st_no", tpssm17.ST_NO);
			cmd_seq.ExecuteReader();
			if(cmd_seq.Read()) 
			{
				tpssm17.SEQ_NO = cmd_seq.GetDecimal(1) ; //将数据获取到实体对象中
			}
			cmd_seq.Close();
			// 执行新增,失败抛出异常
			sqlstr = CString("tpssm17.Insert()");
			Log::Trace("",__FUNCTION__,"st_no = [{0}],st_no1 = [{1}]",tpssm17.ST_NO,tpssm17.ST_NO1);

			tpssm17.Insert();

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
				sprintf(s.msg, "tpssm17.DECI_FLAG[%s],不在设定的范围[1-5]之间，请重新输入",(const char * )tpssm17.DECI_FLAG);
				s.flag = -1;                 
				throw CApplicationException(-1,s.msg,log.Location);
				break;
			}
			if (tpssm17_mirror.QueryCount("ST_NO,ST_NO1") > 0)
			{
				tpssm17_mirror.REC_REVISOR = s.userid;
				tpssm17_mirror.REC_REVISE_TIME = s.datetime;
				tpssm17_mirror.SEND_FLAG = "U";
				tpssm17_mirror.Update("ST_NO2,ST_NO3,DECI_FLAG,SEND_FLAG,REC_REVISOR,REC_REVISE_TIME","ST_NO,ST_NO1");
			}
			else
			{
				Log::Trace("",__FUNCTION__,"st_no111 = [{0}],st_no222 = [{1}]",tpssm17_mirror.ST_NO,tpssm17_mirror.ST_NO1);
				cmd_seq.SetCommandText(sel_max);
				cmd_seq.Parameters.Set("st_no",tpssm17_mirror.ST_NO);
				cmd_seq.ExecuteReader();
				if(cmd_seq.Read()) 
				{
					tpssm17_mirror.SEQ_NO = cmd_seq.GetDecimal(1) ; //将数据获取到实体对象中
				}
				cmd_seq.Close();
				tpssm17_mirror.Insert();
			}
		}

		//设置系统返回消息，国际化信息
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  

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

	return doFlag;
}