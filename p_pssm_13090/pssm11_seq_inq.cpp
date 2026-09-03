/*=========================================================================
//程序名称:     pssm11_seq_inq
//隶属子系统:   PSSM
//产品名称:     BM2MES
//创建人员:     ZHP
//创建时间:     2016/11/16 9:52:35
//修改人员:
//修改日期:
//=========================================================================*/
#include "stdafx.h"



/*<remark>=========================================================
/// <summary>
/// 工序顺序调整查询
/// <para>根据指定的设备，按生产次序查询该设备下的计划。            </para>
/// <para>根据计划状态，分别查询已生产的工序计划和未生产的工序计划。</para>
/// <para>数据库表：TPSSM13/14(炼钢出钢计划跟踪表)                  </para>
/// <para>主调用函数：前台FormPSSM11SeqDlg画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <param name="dev_code">炼钢设备代码              </param>
/// <returns>指定设备下的出钢计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_seq_inq)


int f_pssm11_seq_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	int doFlag = 0;
	int k = 0;
	
	CString  factory_div = "";
	CString dev_code = "";

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	
	try
	{
		//--------------------------------------------------------------
		//获得输入参数
		dev_code = bcls_rec->Tables[0].Rows[0]["DEV_CODE"].ToString().Trim();
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		//查询工序已生产的计划
		//bcls_ret->Tables.Add();
		bcls_ret->Tables[0].set_TableName("PROD");
		bcls_ret->Tables[0].Columns.Add(tpssm11);
		bcls_ret->Tables[0].Columns.Add(tpssm12);

		sqlstr = CString(
			//"SELECT a.*, b.* FROM TPSSM11 a LEFT JOIN tpssmt1 b ON( a.SM_PLAN_NO = b.SM_PLAN_NO ) "
			"SELECT A.*,B.* FROM TPSSM12 A, TPSSM11 B "
			" WHERE A.FACTORY_DIV = B.FACTORY_DIV  "
			"	AND A.SM_PLAN_NO = B.SM_PLAN_NO "
			"	AND A.FACTORY_DIV = @factory_div " 
			"   AND A.DEV_CODE = @dev_code "
			"   AND A.PROC_NO <> ' ' "
			"   AND A.SM_PLAN_NO IN ( "
			"		SELECT SM_PLAN_NO FROM TPSSM11 "
			"		WHERE  FACTORY_DIV	= @factory_div "
			"		  AND  PONO_STATUS	< 91) "
			"   ORDER BY A.PROC_NO ASC  "
			);
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.Parameters.Set("dev_code", dev_code);
		cmd_inq.ExecuteReader();
		
		while (cmd_inq.Read())
		{
			k = 1;
			k = cmd_inq.Fetch(tpssm12, k);
			k = cmd_inq.Fetch(tpssm11, k);


			CDataRow& row = bcls_ret->Tables[0].Rows.Add();
			row.Merge(tpssm12);
			row.Merge(tpssm11);
		}
		cmd_inq.Close();

		//查询工序未生产的计划
		bcls_ret->Tables.Add();
		bcls_ret->Tables[1].set_TableName("PLAN");
		bcls_ret->Tables[1].Columns.Add(tpssm11);
		bcls_ret->Tables[1].Columns.Add(tpssm12);

		sqlstr = CString(
			//"SELECT a.*, b.* FROM TPSSM11 a LEFT JOIN tpssmt1 b ON( a.SM_PLAN_NO = b.SM_PLAN_NO ) "
			"SELECT A.*,B.* FROM TPSSM12 A, TPSSM11 B "
			" WHERE A.FACTORY_DIV = B.FACTORY_DIV  "
			"	AND A.SM_PLAN_NO = B.SM_PLAN_NO "
			"	AND A.FACTORY_DIV = @factory_div "
			"   AND A.DEV_CODE = @dev_code "
			"   AND A.PROC_NO = ' ' "
			"   AND A.SM_PLAN_NO IN ( "
			"		SELECT SM_PLAN_NO FROM TPSSM11 "
			"		WHERE  FACTORY_DIV	= @factory_div "
			"		  AND  PONO_STATUS	< 83) "
			"   ORDER BY A.PRE_PROC_NO ASC  "
			);
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.Parameters.Set("dev_code", dev_code);
		cmd_inq.ExecuteReader();
		
		while (cmd_inq.Read())
		{
			k = 1;
			k = cmd_inq.Fetch(tpssm12, k);
			k = cmd_inq.Fetch(tpssm11, k);

			CDataRow& row = bcls_ret->Tables[1].Rows.Add();
			row.Merge(tpssm12);
			row.Merge(tpssm11);
		}
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
