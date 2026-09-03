/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    zhp
Version:    3.1.0
Date:     2016-11-11
Description: 出钢计划调整浇次查询。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"





/*<remark>=========================================================
/// <summary>
/// 出钢计划调整浇次查询
/// <para>出钢计划进行浇次调整时，查询出钢计划表中的连铸信息。      </para>
/// <para>浇次计划根据cast_no, cast_div_no升序排序。                </para>
/// <para>并按不同流，查询板坯宽度信息。                            </para>
/// <para>数据库表：TPSSM11/03(炼钢出钢计划跟踪表)                  </para>
/// <para>主调用函数：前台FormPSSM11CastDlg画面查询按钮调用。       </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <param name="cc_mach_no">连铸机号                </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_inq_cast)

int f_pssm11_inq_cast(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	CString dev_code = "";
	CString dev_name = "";
	CString factory_div = "";
	CDecimal area_id = 0;
	CString cc_mach_no = "";
	CString v_cast_no_show = "";
	int k = 0;

	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn); 
	CDbCommand cmd_tpssm03_inq(conn);

	try
	{

		//设置返回块参数
		bcls_ret->Tables[0].set_TableName("CAST");  //与Client端dS_PSSM11ProcNo.TPSSM25表保持一致
		bcls_ret->Tables[0].Columns.Add(tpssm11);
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CAST_NO_SHOW");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_THICK");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH_1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SLAB_WIDTH_2");

		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().Trim();

		k = 1;
		sqlstr = CString(
			//"SELECT A.*, B.* "
			//"  FROM TPSSM11 A,TPSSM10 B "
			//"  WHERE A.FACTORY_DIV = B.FACTORY_DIV "
			//"	 AND A.PONO = B.PONO  "
			//"	 AND A.FACTORY_DIV = @FACTORY_DIV "
			//"	 AND A.CC_MACH_NO = @CC_MACH_NO  "
			//"	 AND A.RUN_STATUS < '52' "
			//" ORDER BY A.CAST_NO ASC, A.CAST_DIV_NO ASC "
			"SELECT * "
			"  FROM TPSSM11 "
			"  WHERE FACTORY_DIV = @FACTORY_DIV "
			"	 AND CC_MACH_NO = @CC_MACH_NO  "
			"	 AND RUN_STATUS < '52' "
			" ORDER BY CAST_NO ASC, CAST_DIV_NO ASC "
			);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("FACTORY_DIV", factory_div);
		cmd_inq.Parameters.Set("CC_MACH_NO", cc_mach_no);

		////Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);

		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssm11.Reset();
			//k = cmd_inq.Fetch(tpssm11, k);
			//k = cmd_inq.Fetch(tpssm10, k);
			cmd_inq.Fetch(tpssm11);

			CDataRow& row = bcls_ret->Tables["CAST"].Rows.Add();
			row.Merge(tpssm11);

			v_cast_no_show = CString::Format("%s-%d-%d", (const char*)tpssm11["CAST_NO"].ToString(), tpssm11["CAST_PONO_SUM"].ToDecimal().ToInt32(), tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32());
			row["CAST_NO_SHOW"] = v_cast_no_show;

			//------------------------------------------
			//查询出钢计划子表, 将炉次、时间等内容写入相应的列中
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"SELECT STRAND_NO, MIN(SLAB_WIDTH), MIN(SLAB_THICK) FROM TPSSM03 "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND PONO = @pono "
					" GROUP BY STRAND_NO "
					);
				break;
			}

			cmd_tpssm03_inq.SetCommandText(sqlstr);
			////Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm03_inq.Parameters.Set("pono", tpssm11["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();

			while (cmd_tpssm03_inq.Read())
			{
				tpssm03["STRAND_NO"] = cmd_tpssm03_inq.GetString(1);
				tpssm03["SLAB_WIDTH"] = cmd_tpssm03_inq.GetDecimal(2);
				tpssm03["SLAB_THICK"] = cmd_tpssm03_inq.GetDecimal(3);

				if (tpssm03["STRAND_NO"].ToString() == "1")
				{
					//1流的
					row["SLAB_WIDTH_1"] = tpssm03["SLAB_WIDTH"];
				}
				else if (tpssm03["STRAND_NO"].ToString() == "2")
				{
					//2流的
					row["SLAB_WIDTH_2"] = tpssm03["SLAB_WIDTH"];
				}
				row["SLAB_THICK"] = tpssm03["SLAB_THICK"];
			}
			cmd_tpssm03_inq.Close();
			
		}//while 结束
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
