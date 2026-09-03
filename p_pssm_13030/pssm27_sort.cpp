/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-10-26
功能: 交接部出钢记号转换表排序
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm17.h"

/*<remark >========================================================= 
/// <summary > 
/// 交接部出钢记号转换表排序
/// <para > 
/// 1.根据传入的交接部主出钢记号、出钢记号1对数据库中对应的信息进行排序；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F6(排序)调用。   </para > 
/// </summary > 
/// <param name = "ST_NO" > 主出钢记号  </param > 
/// <param name = "ST_NO1" > 出钢记号1  </param > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm27_sort)

int f_pssm27_sort(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
    CString st_no = "";
	CString v_st_no = "";
	int seq_no = 0;
	int v_seq_no = 0;
	CString datetimeNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	/*定义业务用变量*/
	/*实体类定义*/

	CTPSSM17 tpssm17(conn);
	CTPSSM17 tpssm17_mirror(conn);

	/******业务处理开始******/

	try
	{
		st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString();
		sqlstr = " SELECT * FROM TPSSM17 WHERE ST_NO=@st_no ORDER BY SEQ_NO ";
		CDbCommand cmd(sqlstr, conn);
		cmd.Parameters.Set("st_no",st_no);
		cmd.ExecuteQuery(bcls_ret->Tables[0]);
        sqlstr = " UPDATE TPSSM17 "
			     " SET SEQ_NO = @seq_no, "
			     " REC_REVISOR=@rec_revisor,"
				 " REC_REVISE_TIME=@rec_revise_time"
				 " WHERE ST_NO=@v_st_no "
				 " AND SEQ_NO=@v_seq_no";
		CDbCommand cmd_up(conn);
		for(int i = 0; i < bcls_ret->Tables[0].Rows.get_Count();i++)
		{
			seq_no =  i + 1;
			v_st_no = bcls_ret->Tables[0].Rows[i]["ST_NO"].ToString();
			v_seq_no =(int) bcls_ret->Tables[0].Rows[i]["SEQ_NO"];
			cmd_up.SetCommandText(sqlstr);
			cmd_up.Parameters.Set("seq_no",seq_no);
			cmd_up.Parameters.Set("rec_revisor",s.userid);
			cmd_up.Parameters.Set("rec_revise_time",datetimeNow);
			cmd_up.Parameters.Set("v_st_no",v_st_no);
			cmd_up.Parameters.Set("v_seq_no",v_seq_no);
			cmd_up.ExecuteNonQuery();
		}
 
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