/*=========================================================================
//程序名称:     pssm11_inq
//隶属子系统:   PSSM
//产品名称:     BSM1
//创建人员:     JHZHAO
//创建时间:     2012-10-29 17:13:56
//修改人员:
//修改日期:
//-----------------------------------------------------------------------
//功能描述:		炼钢设备查询
//数据库表:     TPSSMD1(炼钢设备表);
//主调用函数:   前台PSSM11DevDlg(设备及工序调整)画面调用
//需调用函数:
//=========================================================================*/
#include "stdafx.h"




//#include "tpssmc1.h"


/*<remark>=========================================================
/// <summary>
/// 出钢计划路径及设备调整查询
/// <para>出钢计划路径及设备调整查询,为对话框画面初始化。       </para>
/// <para>1.根据传入的主工序代码，查询所有炼钢设备。            </para>
/// <para>2.查询选择的计划内容,包括主计划和工序计划内容。       </para>
/// <para>数据库表：TPSSMD1/11/12(炼钢出钢计划表)               </para>
/// <para>主调用函数：前台PSSM11DevDlg(设备及工序调整),FormPSSM11SeqDlg(顺序调整)画面调用。</para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_dev_inq)


int f_pssm11_dev_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int blknum;

	CString colname = "";

	CString pono = "";
	int first_srf; //精炼工序的第一个charge_no
	int early_flag = 0;   //早到时间超标: 0-范围内; 1-超出
	CString start_time = "";
	CDecimal st_no_flag = 0;
	CDecimal l_ca_main_min = 0;
	CDecimal l_ca_main_max = 0;
	CString  factory_div = "";

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmt1("TPSSMT1");
	CModel tpssmd1("TPSSMD1");
	//CTPSSMC1 tpssmc1(conn);

	CString sqlstr = "";
	CString sqlorder = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	try
	{

		//------------------------------------------
		//设置返回块参数
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("DEV");
		bcls_ret->Tables[blknum].Columns.Add(tpssmd1);   //从实体对象创建架构

		blknum = 2; //第2块
		bcls_ret->Tables[blknum].set_TableName("PONO");
		bcls_ret->Tables[blknum].Columns.Add(tpssm11);   //从实体对象创建架构

		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "S_PTN");
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "AR_NO");

		blknum = 3; //第3块
		bcls_ret->Tables[blknum].set_TableName("CHARGE");
		bcls_ret->Tables[blknum].Columns.Add(tpssm12);   //从实体对象创建架构

		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "PROD_FLAG");   //进入生产标识：0-未生产; 1-已生产
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "S_PROD_FLAG");   //进入生产标识：0-未生产; 1-已生产
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "AR_PROD_FLAG");   //进入生产标识：0-未生产; 1-已生产


		//--------------------------------------------------------------
		//获得输入参数
		pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		////Log::Info("", __FUNCTION__, "pono = [{0}]", pono);
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", factory_div);


		//------------------------------------------
		// 出钢计划查询
		sqlstr = CString(
			"SELECT * FROM TPSSMD1 "
			" WHERE FACTORY_DIV = @factory_div " //83-
			"   ORDER BY AREA_ID, STATION_ID, STATION_NO ASC "
			);
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteReader();

		fetchRowCount = 0;
		blknum = 0; //第1块

		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);

			CDataRow& row = bcls_ret->Tables["DEV"].Rows.Add();
			row.Merge(tpssmd1);
		}
		cmd_inq.Close();

		//读取炉次信息
		tpssm11["FACTORY_DIV"] = factory_div;
		tpssm11["PONO"] = pono;

		if (tpssm11.Query("FACTORY_DIV, PONO") == true)
		{
			CDataRow& row = bcls_ret->Tables["PONO"].Rows.Add();
			row.Merge(tpssm11);
		}

		//读取炉次信息
		tpssm11["FACTORY_DIV"] = factory_div;
		tpssm11["PONO"] = pono;
		
		sqlstr = "SELECT * FROM TPSSM12 "
			" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
			" AND SM_PLAN_NO = @tpssm11.SM_PLAN_NO"
			" ORDER BY CC_MACH_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm12);

			CDataRow& row = bcls_ret->Tables["PROD_FLAG"].Rows.Add();
			row.Merge(tpssm12);

			if (tpssm12["PROC_NO"].ToString().Trim() == "")
			{
				row["PROD_FLAG"] = 0;
			}
			else
			{
				row["PROD_FLAG"] = 1;
			}
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
