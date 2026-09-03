/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-16
Version:  3.1.0
Description: 出钢计划编制条件查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




/*<remark>=========================================================
/// <summary>
/// 出钢计划编制条件查询
/// <para>查询连铸炉次条件和连铸浇铸信息。                 </para>
/// <para>数据库表：TPSSM26/10                             </para>
/// <para>主调用函数：PSSM11P编制对话框画面调用。          </para>
/// </summary>
/// <param name="sm_unit_no"> 炼钢厂别代码      </param>
/// <returns>连铸炉次条件和连铸浇铸信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11add_inq2)

int f_pssm11add_inq2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);	//程序用变量

	int doFlag = 0;

	/* 业务变量 */
	CString factory_div = "";
	CString cc_mach_no = "";
	int count = 0;
	int count1 = 0;

	CString sqlstr = "";

	CDbCommand cmd_tpssm10_inq(conn);

	// 定义表的实体对象
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");


	try
	{

		//---------------------------------------------------
		//设定返回数据表
		//1)连铸浇铸信息
		bcls_ret->Tables[0].set_TableName("TPSSM10_CC");
		bcls_ret->Tables["TPSSM10_CC"].Columns.Add(tpssm10);
		bcls_ret->Tables["TPSSM10_CC"].Columns.Add(DT_STRING, "CAST_SHOW");  //CAST号，显示用
		bcls_ret->Tables["TPSSM10_CC"].Columns.Add(DT_STRING, "LOT_SHOW");   //浇次号，显示用
		bcls_ret->Tables["TPSSM10_CC"].Columns.Add(DT_STRING, "SLAB_SPEC");  //铸坯规格(厚宽长)

		CDataRow *prow = 0;

		//---------------------------------------------------
		//获得输入参数
		//CString flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString().Trim();
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().Trim();
		//////Log::Trace("", __FUNCTION__, "flag1[{0}]", flag);

		//---------------------------------------------------
		//查询连铸浇铸信息，查询未编入计划的炉次
		sqlstr = CString(
			" SELECT * FROM TPSSM10 "
			"  WHERE FACTORY_DIV = @factory_div " //16-命令接收
			"    AND CC_MACH_NO = DECODE(@cc_mach_no, '0', CC_MACH_NO, @cc_mach_no) "
			"    AND PONO_STATUS = 16 "
			" ORDER BY CC_MACH_NO ASC, CC_SEQ ASC "
			);
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.Parameters.Set("factory_div", factory_div);
		cmd_tpssm10_inq.Parameters.Set("cc_mach_no", cc_mach_no);
		cmd_tpssm10_inq.ExecuteReader();
		while (cmd_tpssm10_inq.Read())
		{
			cmd_tpssm10_inq.Fetch(tpssm10);

			////Log::Trace("", __FUNCTION__, "PONO=[{0}], CC_MACH_NO=[{1}]。", tpssm10["PONO"].ToString(), tpssm10["CC_MACH_NO"].ToString());

			//判断指定PONO是否正确
			if (tpssm10["CC_MACH_NO"].ToString().Trim() == "") //不能为空，报错
			{
				CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令[{0}]的连铸机号为空，数据错误，请选择或输入后操作。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			CDataRow & row = bcls_ret->Tables["TPSSM10_CC"].Rows.Add();
			row.Merge(tpssm10);

			int i = bcls_ret->Tables["TPSSM10_CC"].Rows.get_Count() - 1;
			prow = &(bcls_ret->Tables["TPSSM10_CC"].Rows[i]);

			//有指定要求或带T标记的
			if (tpssm10["CC_REQ_TIME_FLAG"].ToString().Trim() == "1")   //|| tpssm10.RESTRAND_FLAG.Trim() == "T"
			{
				(*prow)["CC_REQ_TIME"] = tpssm10["CC_REQ_TIME"];
			}
			else
			{
				(*prow)["CC_REQ_TIME"] = "";
			}

			(*prow)["LOT_SHOW"] = tpssm10["CAST_LOT_NO"].ToString().Trim() + "-" + tpssm10["CAST_LOT_DIV_NO"].ToDecimal().ToString();

			//去除小数
			tpssm10["SLAB_THICK"] = tpssm10["SLAB_THICK"].ToDecimal().ToInt32();
			tpssm10["SLAB_WIDTH"] = tpssm10["SLAB_WIDTH"].ToDecimal().ToInt32();
			tpssm10["SLAB_LEN"] = tpssm10["SLAB_LEN"].ToDecimal().ToInt32();
			(*prow)["SLAB_SPEC"] = tpssm10["SLAB_THICK"].ToDecimal().ToString() + "*" + tpssm10["SLAB_WIDTH"].ToDecimal().ToString() + "*" + tpssm10["SLAB_LEN"].ToDecimal().ToString();

		}
		cmd_tpssm10_inq.Close();


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

	cmd_tpssm10_inq.Close();

	return doFlag;

}
