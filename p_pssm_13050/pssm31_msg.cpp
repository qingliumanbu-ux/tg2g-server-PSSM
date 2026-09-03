#include "stdafx.h"

using namespace BM2;
using namespace BM2::Data;
using namespace BM2::Data::DbClient;

// Service 入口
BM2F_ENTERACE(pssm31_msg)

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
/// <returns>查询运转状况实时信息。</returns>
===========================================================</remark>*/

int f_pssm31_msg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	//1.自定义变量
	int doFlag = 0;
	CString c_sql = "";
	CString factory_div = "";
	

	//2.定义表的实体对象	

	//3.定义DbCommand对象
	CDbCommand tpssmha_cmd(conn);
	
	try
	{
		//获取输入参数
		//factory_div = bcls_rec->Tables[0].Rows[0]["MAIN_BACKLOG_CODE"].ToString().Trim();
		if ("" == factory_div)
		{
			factory_div = "A10";
		}

		//查询运转状况
		c_sql = "SELECT *"
				" FROM TPSSMHA "
				//" WHERE COMPANY_CODE = @company_code"
				//" AND MAIN_BACKLOG_CODE = @factory_div"
				;

		tpssmha_cmd.SetCommandText(c_sql);
		//tpssm31_cmd.Parameters.Set("company_code", s.company_code);
		//tpssm31_cmd.Parameters.Set("factory_div", factory_div);
		tpssmha_cmd.ExecuteQuery(bcls_ret->Tables[0]);

		bcls_ret->Tables[0].set_TableName("PSSM31");
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
	tpssmha_cmd.Close();


	return doFlag;
}