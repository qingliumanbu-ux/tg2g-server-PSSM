/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-05-22
功能: 制造命令日备注信息表维护_查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
///制造命令日备注信息表_查询
/// <para > 
/// 查询制造命令日备注信息表记录；
/// </para > 
/// <para > 数据库表：TPSSM24(制造命令日备注信息表)      </para > 
/// <para > 主调用函数：前台PSSM24画面F2(查询)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
// Service 入口
BM2F_ENTERACE(pssm24_inq)

///////////////////////////////////////////////////
//存在主键的表， 执行单记录查询
///////////////////////////////////////////////////
int f_pssm24_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;

	CString  sqlstr("");
	CString  sql_1;
	sql_1 = "";
	CString factory_div;
	CString plan_date;
	//CDbCommand cmd_inq(conn);
	/*定义业务用变量*/
	try
	{
		// 定义表的实体对象
	CModel tpssm24_inf("TPSSM24");

		//分页查询用参数
		int  nStart = (int)bcls_rec->Tables[0].Rows[0]["START"];//获取查询起始值
		int  nPageSize = (int)bcls_rec->Tables[0].Rows[0]["PAGE_SIZE"];//获取页面值
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		plan_date = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();
		int  RECORD_TOTAL = 0;
		//判断前台传入的factory_div,plan_date是否为空    修改时间：2017-12-20
		if (factory_div.Trim() != "")
			sql_1 = sql_1+" and FACTORY_DIV=@factory_div ";
		if (plan_date.Trim()!= "")
			sql_1 = sql_1 + " and PLAN_DATE = @plan_date ";

		////Log::Info("", __FUNCTION__, "factory_div = [{0}], plan_date = [{1}]", factory_div, plan_date);
		////Log::Info("", __FUNCTION__, "nStart =[{0}], nPageSize = [{1}]", nStart, nPageSize);

		//定义数据库操作命令对象comm_inq执行sql语句，sql字符串用""包括，可以分行，但每行前后务必留出一个空格。
		CString sqlstr = " SELECT * "
						 " FROM TPSSM24"
			             " WHERE 1=1  ";	

		CString sql_count = " SELECT "
			                " COUNT(1) "
							" FROM TPSSM24"
			                " WHERE 1=1  ";		
		sqlstr = sqlstr + sql_1;
		sql_count = sql_count + sql_1;
		CString sql_order = " ORDER BY SEQ_NO ASC ";
		//获取记录数
		CDbCommand cmd(sql_count, conn);
		cmd.Parameters.Set("factory_div", factory_div);
		cmd.Parameters.Set("plan_date", plan_date);
		CDecimal rc = cmd.ExecuteScalar();
		bcls_ret->ExtendedProperties.Add("RECORD_TOTAL",rc.ToString());

		//查询出结果
		CDbCommand comm_inq(sqlstr + sql_order,conn);
		comm_inq.Parameters.Set("factory_div", factory_div);
		comm_inq.Parameters.Set("plan_date", plan_date);
		comm_inq.ExecuteQuery(bcls_ret->Tables[0],nStart,nPageSize);

		//如果前台要求通过表名进行关联，需要进行设置对应块的表名信息
		bcls_ret->Tables[0].set_TableName("TPSSM24");
		strcpy(s.msg, _RES("PMOMS0000311")/*查询成功*/); 

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
