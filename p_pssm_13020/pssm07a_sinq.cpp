/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-04-11
功能: 制造命令下的板坯命令查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"



/*<remark >=========================================================
/// <summary >
/// 制造命令下的板坯命令查询
/// <para >
/// 1.根据传入的制造命令号、连铸机号查询板坯命令信息。
/// 2.查询条件：制造命令号；
/// 3.排序方式：制造命令号升序；
/// </para >
/// <para > 数据库表：TPSSM03(板坯命令表) </para >
/// <para > 主调用函数：前台pssm07画面双击事件调用。   </para >
/// </summary >
/// <param name = "PONO" > 制造命令号  </param >
/// <returns > 制造命令下的板坯命令信息</returns >
/// <returns > 成功：0</returns >
/// <returns > 失败：-1</returns >
=========================================================== </remark > */

/******service入口******/

BM2F_ENTERACE(pssm07a_sinq);

int f_pssm07a_sinq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/
	int doFlag = 0;		//返回值
	int logFlag = 1;

	CString check_flag;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString  sql_cont = "";

	/*定义业务用变量*/

	CString pono = "";
	int  total = 0;

	/*实体类定义*/
	CModel tpssm03("TPSSM03");

	/******业务处理开始******/
	try
	{
		//分页查询用参数
		int  nStart = (int)bcls_rec->Tables[0].Rows[0]["START"];//获取查询起始值
		int  nPageSize = (int)bcls_rec->Tables[0].Rows[0]["PAGE_SIZE"];//获取页面值
		int  RECORD_TOTAL = 0;

		/*获取前台输入数据*/
		pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToString();/*炼钢区分*/
		check_flag = bcls_rec->Tables[0].Rows[0]["CHECK_FLAG"].ToString();

		/*建立查询数据SQL语句*/
		if (check_flag == "1")
		{
			sqlstr = " SELECT "
				" TPSSM03.* "
				" FROM   TPSSM03 "
				" WHERE  PONO = @pono "
				" ORDER BY PONO,SLAB_NO ASC ";
			sql_cont = " SELECT "
				" COUNT(*)  "
				" FROM  TPSSM03 "
				" WHERE  PONO = @pono ";
		}
		else
		{
			sqlstr = " SELECT "
				" HPSSM03.* "
				" FROM   HPSSM03 "
				" WHERE  PONO = @pono "
				" ORDER BY PONO,SLAB_NO ASC ";

			sql_cont = " SELECT "
				" COUNT(*)  "
				" FROM  HPSSM03 "
				" WHERE  PONO = @pono ";
		}

		CDbCommand cmd_count(sql_cont, conn);
		cmd_count.Parameters.Set("pono", pono);
		CDecimal rc = cmd_count.ExecuteScalar();
		bcls_ret->ExtendedProperties.Add("RECORD_TOTAL", rc.ToString());

		/*给查询SQL赋条件值*/
		CDbCommand cmd_sql(sqlstr, conn);
		cmd_sql.Parameters.Set("pono", pono);
		cmd_sql.ExecuteQuery(bcls_ret->Tables[0], nStart, nPageSize);
		bcls_ret->Tables[0].set_TableName("TPSSM03");

		/*返回处理信息*/
		if (bcls_ret->Tables[0].Rows.get_Count() == 0)
		{
			strcpy(s.msg, _RES("GCRSS0000013")/*没有满足条件的记录。*/);
		}
		else
		{
			CFormattable arguments[] = { rc.ToString() };
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004")/*查询到[{0}]条记录。*/, arguments, 1);
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char *)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}





