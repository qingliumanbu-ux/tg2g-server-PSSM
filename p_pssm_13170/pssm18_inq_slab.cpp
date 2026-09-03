/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2014-06-25
Description: 制造命令号对应的板坯查询
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
// New Include
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



// service入口
BM2F_ENTERACE(pssm18_inq_slab)
//-EP_SYSTEM_HEAD_END                                                  
int f_pssm18_inq_slab(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{


	CTracer log(__FUNCTION__); 	//系统日志类定义

	/* 程序内部变量 */
	int doFlag = 0;
	int blkNum = 0;

	/* 业务变量 */
	CString	datetime("");
	CString v_table_type = "";
	CString lslab_no = "";
	CString strand_no = "";
	CDecimal    prod_density = 7.85;
	CString cs_a000_xs = "";
	CString cs_a001_xs = "";
	/* 数据库SQL操作字符串 */
	CString sqlstr;

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	

	CModel tpssm03("TPSSM03");
	CModel tpssm11("TPSSM11");
	CDataTable tb_tpssm03("TPSSM03");

	EIClass TPSSM03_SOURCE;
	TPSSM03_SOURCE.Tables[0].Columns.Add(tpssm03);
	TPSSM03_SOURCE.Tables[0].Columns.Add(DT_DECIMAL, "ORDER_THICK");
	TPSSM03_SOURCE.Tables[0].Columns.Add(DT_DECIMAL, "ORDER_WIDTH");
	TPSSM03_SOURCE.Tables[0].Columns.Add(DT_DECIMAL, "ORDER_LEN");
	TPSSM03_SOURCE.Tables[0].Columns.Add(DT_DECIMAL, "ORDER_MAX_LEN");
	TPSSM03_SOURCE.Tables[0].Columns.Add(DT_DECIMAL, "ORDER_MIN_LEN");

	bcls_ret->Tables.Add("TPSSM03");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "PONO");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "SLAB_NO");//预定材料号
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "STRAND_NO");//流号
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_THICK");//材料厚度
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_WIDTH");//材料宽度
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_LEN");//材料目标长度
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_MAX_LEN");//材料最大长度
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_MIN_LEN");//材料最小长度
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_WT");//材料重量（t）
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "ORDER_NO");//合同号
	//bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "SLAB_DEST");//材料去向
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "SG_SIGN");//牌号
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "SLAB_SEQ_2");//
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "SLAB_SIZE");//
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "ORDER_THICK");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "ORDER_WIDTH");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "ORDER_LEN");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "ORDER_MAX_LEN");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_DECIMAL, "ORDER_MIN_LEN");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "WORK_SPECIAL_REQ");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "A000_XS");
	bcls_ret->Tables["TPSSM03"].Columns.Add(DT_STRING, "A001_XS");

	try
	{


		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
			tpssm03["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();

		Log::Info("", __FUNCTION__, "PONO=[{0}]", tpssm03["PONO"].ToString());

		sqlstr = " SELECT  "
			"  (SELECT ATTRI_NUM_ATFLV||'-'||ATTRI_NUM_ATFLB  FROM TQMOM02 WHERE ATTRI_ITEM ='A000' AND ORDER_NO =A.ORDER_NO)  AS A000_XS,  "
			"  (SELECT ATTRI_NUM_ATFLV||'-'||ATTRI_NUM_ATFLB  FROM TQMOM02 WHERE ATTRI_ITEM ='A001' AND ORDER_NO =A.ORDER_NO)  AS A001_XS,  "
			" A.*,B.ORDER_THICK,B.ORDER_WIDTH,B.ORDER_LEN,B.ORDER_MAX_LEN,B.ORDER_MIN_LEN   "
		
			" FROM TPSSM03 A LEFT JOIN TQMOM01 B ON A.ORDER_NO = B.ORDER_NO WHERE A.PONO = @pono ORDER BY A.PONO,A.LSLAB_NO,A.SLAB_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("pono", tpssm03["PONO"].ToString());
		cmd_inq.ExecuteQuery(TPSSM03_SOURCE.Tables[0]);
		cmd_inq.Close();

		if (TPSSM03_SOURCE.Tables[0].Rows.get_Count() > 0)
		{
			for (int i = 0; i < TPSSM03_SOURCE.Tables[0].Rows.get_Count(); i++)
			{
				tpssm03.Reset();
				tpssm03.MergeFrom(TPSSM03_SOURCE.Tables[0].Rows[i]);
				tpssm11["PONO"] = tpssm03["PONO"].ToString();
				tpssm11.Query("PONO");
				if (lslab_no != tpssm03["LSLAB_NO"].ToString())
				{
					CDataRow & row_03 = bcls_ret->Tables["TPSSM03"].Rows.Add();

					//Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", tpssm03["LSLAB_NO"].ToString());

					if (tpssm11["CC_MACH_NO"].ToString() == "0")
					{
						strand_no = "Z";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "1")
					{
						strand_no = "A";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "2")
					{
						strand_no = "B";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "1")
					{
						strand_no = "C";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "3" && tpssm03["STRAND_NO"].ToString() == "2")
					{
						strand_no = "D";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "1")
					{
						strand_no = "E";
					}
					else if (tpssm11["CC_MACH_NO"].ToString() == "4" && tpssm03["STRAND_NO"].ToString() == "2")
					{
						strand_no = "F";
					}
					row_03["A000_XS"] = TPSSM03_SOURCE.Tables[0].Rows[i]["A000_XS"].ToString();
					row_03["A001_XS"] = TPSSM03_SOURCE.Tables[0].Rows[i]["A001_XS"].ToString();
					row_03["PONO"] = tpssm03["PONO"].ToString();
					row_03["SLAB_NO"] = tpssm03["LSLAB_NO"].ToString();//预定材料号
					row_03["STRAND_NO"] = strand_no;//流号
					row_03["SLAB_THICK"] = tpssm03["SLAB_THICK"].ToDecimal();//材料厚度
					row_03["SLAB_WIDTH"] = tpssm03["SLAB_WIDTH"].ToDecimal();//材料宽度
					row_03["WORK_SPECIAL_REQ"] = tpssm03["WORK_SPECIAL_REQ"].ToString();
					//Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", tpssm03["LSLAB_NO"].ToString());
					row_03["ORDER_THICK"] = TPSSM03_SOURCE.Tables[0].Rows[i]["ORDER_THICK"].ToDecimal();
					row_03["ORDER_WIDTH"] = TPSSM03_SOURCE.Tables[0].Rows[i]["ORDER_WIDTH"].ToDecimal();
					row_03["ORDER_LEN"] = TPSSM03_SOURCE.Tables[0].Rows[i]["ORDER_LEN"].ToDecimal();
					row_03["ORDER_MAX_LEN"] = TPSSM03_SOURCE.Tables[0].Rows[i]["ORDER_MAX_LEN"].ToDecimal();
					row_03["ORDER_MIN_LEN"] = TPSSM03_SOURCE.Tables[0].Rows[i]["ORDER_MIN_LEN"].ToDecimal();
					//Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", tpssm03["LSLAB_NO"].ToString());

					if (tpssm03["LSLAB_NO_LENGTH"].ToDecimal() != 0)
					{
						row_03["SLAB_LEN"] = tpssm03["LSLAB_NO_LENGTH"].ToDecimal();//材料目标长度
						row_03["SLAB_MAX_LEN"] = tpssm03["LSLAB_NO_LENGTH_MAX"].ToDecimal();//材料最大长度
						row_03["SLAB_MIN_LEN"] = tpssm03["LSLAB_NO_LENGTH_MIN"].ToDecimal();//材料最小长度
						//row_03["SLAB_WT"] = row_03["SLAB_LEN"].ToDecimal() * row_03["SLAB_THICK"].ToDecimal() * row_03["SLAB_WIDTH"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
						row_03["SLAB_WT"] = tpssm03["LSLAB_NO_WT"].ToDecimal();

						sqlstr = " SELECT sum(SLAB_SEQ_2) FROM TPSSM03 WHERE LSLAB_NO = @LSLAB_NO ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("LSLAB_NO", tpssm03["LSLAB_NO"].ToString());
						row_03["SLAB_SEQ_2"] = cmd_inq.ExecuteScalar();
						cmd_inq.Close();
						//row_03["SLAB_SEQ_2"] = tpssm03["SLAB_SEQ_2"].ToDecimal();
					}
					else
					{
						row_03["SLAB_LEN"] = tpssm03["SLAB_LEN"].ToDecimal();//材料目标长度
						row_03["SLAB_MAX_LEN"] = tpssm03["SLAB_MAX_LEN"].ToDecimal();//材料最大长度
						row_03["SLAB_MIN_LEN"] = tpssm03["SLAB_MIN_LEN"].ToDecimal();//材料最小长度
						//row_03["SLAB_WT"] = row_03["SLAB_LEN"].ToDecimal() * row_03["SLAB_THICK"].ToDecimal() * row_03["SLAB_WIDTH"].ToDecimal() * prod_density / 1000 / 1000 / 1000;
						row_03["SLAB_WT"] = tpssm03["SLAB_WT"].ToDecimal();
						row_03["SLAB_SEQ_2"] = tpssm03["SLAB_SEQ_2"].ToDecimal();
					}
					row_03["SLAB_SIZE"] = row_03["SLAB_THICK"].ToString() + "*" + row_03["SLAB_WIDTH"].ToString() + "*" + row_03["SLAB_LEN"].ToString();
					row_03["ORDER_NO"] = tpssm03["ORDER_NO"].ToString();//合同号
					//row_03["SLAB_DEST"];//材料去向
					row_03["SG_SIGN"] = tpssm03["SG_SIGN"].ToString();//牌号
					//row_03["FACTORY_NEXT"];//下游工厂
					lslab_no = TPSSM03_SOURCE.Tables[0].Rows[i]["LSLAB_NO"].ToString();
				}
			}
		}

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
	cmd_inq.Close();
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}




