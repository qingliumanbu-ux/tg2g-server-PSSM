/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-05-22
功能: 制造命令日备注信息表维护_新增
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
///制造命令日备注信息表_新增
/// <para > 
/// 1.根据传入的区分代码，制造命令日备注信息表记录；
/// </para > 
/// <para > 数据库表：TPSSM24(制造命令日备注信息表)      </para > 
/// <para > 主调用函数：前台PSSM24画面F3(新增)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
BM2F_ENTERACE(pssm24_ins)

int f_pssm24_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值
	int seq_no = 0;
	int seq_no_tmp = 0;
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString factory_div = "";
	CString plandate;
	//定义变量--取系统当前时间
	CString datetimeNow = CDateTime::Now().ToString("yyyyMMdd");
	
	////Log::Trace("", "", "datetimeNow{0}", datetimeNow);

	try
	{	
		/*实体类定义*/
		//CTPSSM24 tpssm24_inf(conn);
		//修改方式，采用CModel
		CModel tpssm24_inf = CModel("TPSSM24");

		CDbCommand cmd_inq(conn);
		CDbCommand cmd_max(conn);
		//factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString();
		//plandate = bcls_rec->Tables[1].Rows[0]["PLAN_DATE"].ToString(); 
		//plandate不从前台获取
		plandate = datetimeNow;
		int count = bcls_rec->Tables[0].Rows.get_Count();
		//for(int i = 0;i < count;i++)
		//{

		//	//重置表结构变量

		//	//tpssm24_inf.Reset();

		//	// 取得单行传入信息 
		//	tpssm24_inf.MergeFrom(bcls_rec->Tables[0].Rows[i]);
		//	sqlstr = " SELECT "
		//		" MAX(SEQ_NO) "
		//		" FROM TPSSM24 "
		//		// " WHERE FACTORY_DIV = @factory_div "      
		//		// " AND PLAN_DATE = @plan_date "; 表中主键为plan_date和seq_no，如果加上factory_div可能造成主键冲突
		//		"WHERE PLAN_DATE = @plan_date  ";
		//	cmd_max.SetCommandText(sqlstr);
		//	//cmd_max.Parameters.Set("factory_div", factory_div);
		//	cmd_max.Parameters.Set("plan_date",plandate);
		//	cmd_max.ExecuteReader();
		//	if(cmd_max.Read())
		//	{
		//		seq_no = cmd_max.GetInt32(1);
		//		seq_no_tmp = seq_no + 1;
		//		cmd_max.Close();
		//	}
		//	else
		//	{
		//		seq_no_tmp = seq_no = 1;
		//		cmd_max.Close();
		//	}
		//	/*tpssm24_inf.SEQ_NO = seq_no_tmp;
		//	tpssm24_inf.FACTORY_DIV = factory_div;
		//	tpssm24_inf.PLAN_DATE = plandate;
		//	tpssm24_inf.REC_CREATOR = s.userid;
		//	tpssm24_inf.REC_CREATE_TIME = datetimeNow;
		//	tpssm24_inf.REC_REVISE_TIME = tpssm24_inf.REC_CREATE_TIME;
		//	tpssm24_inf.REC_REVISOR = s.svc_name;*/ 
		//	
		//	//新增制造命令日备注记录
		//	sqlstr = "INSERT TPSSM24";
		//	tpssm24_inf.Insert();
		//}
		sqlstr = " SELECT "
			" MAX(SEQ_NO) "
			" FROM TPSSM24 "
			// " WHERE FACTORY_DIV = @factory_div "      
			// " AND PLAN_DATE = @plan_date "; 表中主键为plan_date和seq_no，如果加上factory_div可能造成主键冲突
			"WHERE PLAN_DATE = @plan_date  ";
		cmd_max.SetCommandText(sqlstr);
		//cmd_max.Parameters.Set("factory_div", factory_div);
		cmd_max.Parameters.Set("plan_date", plandate);
		cmd_max.ExecuteReader();
		if (cmd_max.Read())
		{
			seq_no = cmd_max.GetInt32(1);
			seq_no_tmp = seq_no + 1;
			cmd_max.Close();
		}
		else
		{
			seq_no_tmp = seq_no = 1;
			cmd_max.Close();
		}

		////Log::Trace("", "", "seq_no_tmp:{0}", seq_no_tmp);

		//采用CModel方法
		if (bcls_rec->AtBlkName("ADD") > 0)
		{
			for (int i = 0; i < bcls_rec->Tables["ADD"].Rows.get_Count(); i++)
			{
				tpssm24_inf.Reset();
				bcls_rec->Tables["ADD"].Rows[i]["seq_no"] = seq_no_tmp;
				bcls_rec->Tables["ADD"].Rows[i]["plan_date"] = datetimeNow;
				tpssm24_inf.MergeFrom(bcls_rec->Tables["ADD"].Rows[i]);
				seq_no_tmp = seq_no_tmp + 1;//seq_no是主键，为了避免主键冲突，在前台不提供该列值的输入，根据后台得到
				tpssm24_inf.Insert();
			}
			////Log::Trace("", "", "新增了{0}条记录", bcls_rec->Tables["ADD"].Rows.get_Count());
		}

		CFormattable arguments[] = {count};
		CMessageFormat::Format(s.msg,_RES("PMOMS0000339")/*共新增了[{0}]条记录！*/, arguments, 1);	
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