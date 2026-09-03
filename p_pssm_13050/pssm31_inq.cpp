#include "stdafx.h"

using namespace BM2;
using namespace BM2::Data;
using namespace BM2::Data::DbClient;

// Service 入口
BM2F_ENTERACE(pssm31_inq)

/*<remark>=========================================================
/// <summary>
/// 运转状况查询
/// <para>
/// 1.读取前台传入参数；
/// 2.查询运转状况信息；
/// 3.返回查询结果。
/// </para>
/// </summary>
/// <param name="MAIN_BACKLOG_CODE">主工序代码</param>
/// <returns>查询运转状况实时信息查询。</returns>
===========================================================</remark>*/

int f_pssm31_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	//1.自定义变量
	int doFlag = 0;
	CString c_sql = "";
	int fetchRowCount;

	//2.定义表的实体对象	
	
	//3.定义DbCommand对象
	CDbCommand tpssmh1_cmd(conn);
	CDbCommand tpssmh2_cmd(conn);
	CDbCommand tpssmh3_cmd(conn);
	CDbCommand tpssmh4_cmd(conn);
	CDbCommand tpssmh5_cmd(conn);
	
	try
	{
		//获取输入参数

		//查询运转状况

		//查询设备状态配置表
		c_sql = "SELECT *"
				" FROM TPSSMH1 "
				;

		tpssmh1_cmd.SetCommandText(c_sql);
		tpssmh1_cmd.ExecuteQuery(bcls_ret->Tables[0]);
		bcls_ret->Tables[0].set_TableName("MAIN");

		bcls_ret->Tables.Add();
		c_sql = "SELECT *"
			" FROM TPSSMH2 "
			;

		tpssmh2_cmd.SetCommandText(c_sql);
		tpssmh2_cmd.ExecuteQuery(bcls_ret->Tables[1]);
		bcls_ret->Tables[1].set_TableName("DEV");

		bcls_ret->Tables.Add();
		c_sql = "SELECT *"
			" FROM TPSSMH3 "
			;

		tpssmh3_cmd.SetCommandText(c_sql);
		tpssmh3_cmd.ExecuteQuery(bcls_ret->Tables[2]);
		bcls_ret->Tables[2].set_TableName("ROUTE");

		bcls_ret->Tables.Add();
		c_sql = "SELECT *"
			" FROM TPSSMH4 "
			;

		tpssmh4_cmd.SetCommandText(c_sql);
		tpssmh4_cmd.ExecuteQuery(bcls_ret->Tables[3]);
		bcls_ret->Tables[3].set_TableName("LOC");

		bcls_ret->Tables.Add();
		c_sql = "SELECT *"
			" FROM TPSSMH5 "
			;

		tpssmh5_cmd.SetCommandText(c_sql);
		tpssmh5_cmd.ExecuteQuery(bcls_ret->Tables[4]);
		bcls_ret->Tables[4].set_TableName("POINT");

	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//4.关闭DbCommand对象
	tpssmh1_cmd.Close();
	tpssmh2_cmd.Close();
	tpssmh3_cmd.Close();
	tpssmh4_cmd.Close();
	tpssmh5_cmd.Close();


	return doFlag;
}