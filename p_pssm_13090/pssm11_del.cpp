/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:1.0
Description: 出钢计划删除
Update：   2014-11-27  xuwen  表结构优化
**************************************************/
#include "stdafx.h"






#if defined _SYS_PES
//向L4发送计划状态
//int f_ps200021_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn);
#endif

////出钢计划各炉次开浇时刻计算
//int f_pssm11_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
////工序作业时刻计算
//int f_pssm12_time_calc(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
//精炼顺序计算
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
////浇铸顺联动调整
//int f_pssm10_upd_by_cast(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

//调用TPS模型
int f_pssm_call_tps(CString main_backlog_code, int mode, int bof_cc_matching, CDbConnection * conn);//

/*<remark>=========================================================
/// <summary>
/// 出钢计划删除
/// <para>删除选择的出钢计划，修改各工序计划内容及CAST号。          </para>
/// <para>删除条件：没有进入生产, 即PONO状态<20。                   </para>
/// 1.根据各铸机选择的PONO，删除出钢计划;
/// 2.删除条件:没有进入生产, 即PONO状态 <20
/// 3.修改CAST:1)带T的传下一炉; 2)CAST内序号减1
/// 4.整体浇铸时刻修改
/// <para>数据库表：TPSSM11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：前台PSSM11画面F5(计划删除)调用。              </para>
/// </summary>
/// <param name="PONO">制造命令号                    </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2F_ENTERACE(pssm11_del)

int f_pssm11_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序用变量 */
	int doFlag = 0;
	int ret = 0;
	int fetchRowCount;
	int i, blkseq, rows, blkseq1;
	CString date_time = "";

	int  dummy;
	CDecimal sm_plan_no = 0;
	CString  pono="";
	CString cast_no="";               /* 计算CAST号用 */
	int cast_div_no = 0;          /* CAST分割号 */
	CString v_sm_unit_no = "";	//炼钢单元号
	int     bof_cc_matching = 0;     /*炉机匹配标记: 0-不指定,模型推荐; 1-指定,模型按要求计算*/
	int		mode = 0;      //调用模型功能:  0-模型;   1-匹配;

	CString sqlstr = "";

	EIClass inBlock1;
	EIClass inBlock2;  
	EIClass inBlock3;

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_upd(conn);


	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");


		// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm10("TPSSM10");
	CModel tpssm01("TPSSM01");


		inBlock1.Tables[0].set_TableName("POUR");
		inBlock1.Tables[0].Columns.Add(DT_STRING,"SM_UNIT_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING,"CC_MACH_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING,"CC_REQ_TIME");
		inBlock1.Tables[0].Rows.Add();

		inBlock2.Tables[0].set_TableName("X200021");
		inBlock2.Tables[0].Columns.Add(DT_STRING,"SM_UNIT_NO");
		inBlock2.Tables[0].Columns.Add(DT_STRING,"PONO");
		inBlock2.Tables[0].Columns.Add(DT_STRING,"PONO_STATUS");
		inBlock2.Tables[0].Rows.Add();

		//f_pssm12_sequ_calc()用，传入炼钢单元号，单记录方式
		inBlock3.Tables[0].set_TableName("PLAN");

		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		////inBlock3.Tables[0].Rows.Add();

		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();

		/* 获得输入参数 */
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (i = 0; i < rows; i++) 
		{
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();
			tpssm11["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[i]["SM_PLAN_NO"].ToString();

			////Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}]", tpssm11["SM_PLAN_NO"].ToString());

			tpssm11["PLAN_EDIT_FLAG"] = "D";
			sqlstr = "tpssm11.Update(PLAN_EDIT_FLAG)";
			tpssm11.Update("PLAN_EDIT_FLAG", "FACTORY_DIV, SM_PLAN_NO");

		}//删除炉次置标记


		//采用倒序删除，会比较好
		sqlstr = CString(
			" SELECT * FROM TPSSM11 "
			"  WHERE PLAN_EDIT_FLAG = 'D' "//D-删除标记
			"    AND FACTORY_DIV = @tpssm11.FACTORY_DIV  "//D-删除标记
			" ORDER BY CAST_NO DESC, CAST_DIV_NO DESC"  //必须倒排序，有校验
			);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())  //CAST内有下一炉
		{

			//读取原计划信息
			//1)计划主表
			cmd_inq.Fetch(tpssm11);
			//sqlstr = "tpssm11.Query()";
			//bool has11 = tpssm11.Query("SM_PLAN_NO");
			//if (has11 == false)
			//{
			//	CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString().ToString() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "出钢计划[{0}]不存在，不用删除。", arguments, 1);
			//	throw CApplicationException(-1, s.msg, log.Location);

			//}
			//tpssm11.TrimOrBlank();

			//2)计划子表，如果是开浇第一炉（带T标志的），读取炉次条件信息
			bool has12 = false;
			if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T")
			{
				tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["AREA_ID"] = 5;  //连铸工序信息
				tpssm12["SUB_CHARGE_NO"] = 0;
				sqlstr = "tpssm12.Query()";
				has12 = tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, AREA_ID, SUB_CHARGE_NO");  //必须保证为一条连铸工序信息
			}


			////Log::Trace("", __FUNCTION__, "PONO=[{0}]", tpssm11["PONO"].ToString());

			if (tpssm11["PONO_STATUS"].ToDecimal().ToInt32() >= 20) //20-进入生产
			{				
				//strcpy(s.msg,_RES("PSSMS0000181"))/*出钢计划进入生产，不能删除*/;
				CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "出钢计划号[{0}]进入生产，不能删除。", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//删除计划主表
			sqlstr = "tpssm11.Delete()";
			tpssm11.Delete("FACTORY_DIV, SM_PLAN_NO");


			//删除计划子表
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			sqlstr = "tpssm12.Delete()";
			tpssm12.Delete("FACTORY_DIV, SM_PLAN_NO");


			//修改连铸命令表中的PONO状态
			tpssm10["PONO_STATUS"] = 16;
			tpssm10["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm10["PONO"] = tpssm11["PONO"];

			////Log::Trace("", __FUNCTION__, "PONO=[{0}], PONO_STATUS=[{1}]", tpssm10["PONO"].ToString(), tpssm10["PONO_STATUS"].ToDecimal());
			sqlstr = "tpssm10.Update()";
			tpssm10.Update("PONO_STATUS", "FACTORY_DIV, PONO");


			tpssm01["PONO_STATUS"] =16;
			tpssm01["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm01["PONO"] = tpssm11["PONO"];
			sqlstr = "tpssm01.Update()";
			tpssm01.Update("PONO_STATUS", "FACTORY_DIV, PONO");


			//CAST 内的赋号修改
			//当开浇第一炉被删掉时，T标记往下一炉带
			if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "T")
			{
				//对"T"标记往下一炉带的处理，会带来其他问题。改为提示报错. 2015-2-2 xuwen 修改
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" SELECT SM_PLAN_NO, PONO, CAST_NO FROM TPSSM11 "
						" WHERE FACTORY_DIV = @factory_div "
						"	AND CC_MACH_NO = @cc_mach_no "
						"	AND CAST_NO    = @cast_no "
						"	AND CAST_DIV_NO > @cast_div_no "
						" ORDER BY CAST_DIV_NO ASC "
						);
					break;
				}
				cmd_tpssm11_inq.SetCommandText(sqlstr);
				cmd_tpssm11_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm11_inq.Parameters.Set("cc_mach_no", tpssm11["CC_MACH_NO"].ToString());
				cmd_tpssm11_inq.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
				cmd_tpssm11_inq.Parameters.Set("cast_div_no", tpssm11["CAST_DIV_NO"].ToDecimal());
				cmd_tpssm11_inq.ExecuteReader();
				if (cmd_tpssm11_inq.Read())  //CAST内有下一炉
				{
					sm_plan_no = cmd_tpssm11_inq.GetDecimal(1);
					pono       = cmd_tpssm11_inq.GetString(2).Trim();
					tpssm11["CAST_NO"] = cmd_tpssm11_inq.GetString(3).Trim();
					////Log::Trace("", __FUNCTION__, "CAST内下一炉：pono=[{0}], sm_plan_no=[{1}]", pono, sm_plan_no);

					CFormattable arguments[] = { tpssm11["SM_PLAN_NO"].ToString(), tpssm11["CAST_NO"].ToString(), sm_plan_no.ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "炉次[计划号:{0}]为CAST[{1}]内第1炉,此CAST还有其他炉次[计划号:{2}]，首炉不能删除。", arguments, 3);
					throw CApplicationException(-1, s.msg, log.Location);

					////出钢计划主表修改
					//tpssm11["SM_PLAN_NO"] = sm_plan_no;
					//tpssm11.RESTRAND_FLAG = "T";
					//tpssm11["CC_REQ_TIME"] = tpssm11["CC_REQ_TIME"];
					//sqlstr = "tpssm11.Update(RESTRAND_FLAG)";
					//tpssm11.Update("RESTRAND_FLAG,CC_REQ_TIME", "SM_PLAN_NO");

					////出钢计划子表修改，主要是炉次条件信息写入
					//if (has12 == true) //读取到记录，则写入
					//{
					//	tpssm12["SM_PLAN_NO"] = sm_plan_no;
					//	tpssm12["AREA_ID"] = 5;  //连铸工序信息
					//	tpssm12["SUB_CHARGE_NO"] = 0;
					//	sqlstr = "tpssm12.Update(RESTRAND_FLAG)";
					//	tpssm12.Update(
					//		"PREP_TIME,"       //准备时间      重开浇的炉间准备时间
					//		"PRE_PROC_TIME",   //前处理时间
					//		"SM_PLAN_NO,AREA_ID,SUB_CHARGE_NO");
					//}


					////浇铸顺序表修改，对于CC_SEQ修改，单独的函数中计算
					//tpssm10["PONO"] = pono;
					//tpssm10.RESTRAND_FLAG = "T";
					//if (tpssm11["CC_REQ_TIME"].ToString().Trim() == "")
					//{
					//	//tpssm10["CC_REQ_TIME"] = " ";    不清空，保留原预测时刻
					//	tpssm10["CC_REQ_TIME_FLAG"] = " ";  //CC要求时刻指定标记
					//}
					//else
					//{
					//	tpssm10["CC_REQ_TIME"] = tpssm11["CC_REQ_TIME"];
					//	tpssm10["CC_REQ_TIME_FLAG"] = "1";  //CC要求时刻指定标记
					//}
					//sqlstr = "tpssm10.Update(RESTRAND_FLAG)";
					//tpssm10.Update(
					//	"RESTRAND_FLAG,"
					//	"CC_REQ_TIME,"
					//	"CC_REQ_TIME_FLAG",
					//	"PONO");

				}
				else
				{
				}
				cmd_tpssm11_inq.Close();

			}// if RESTRAND_FLAG=="T"

			//2.CAST内序号减1(浇次内顺序号)
			//1)修改计划表
			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" UPDATE TPSSM11 "
						"    SET CAST_DIV_NO = CAST_DIV_NO - 1 "
						"  WHERE FACTORY_DIV  = @factory_div "
						"	 AND CC_MACH_NO  = @cc_mach_no "
						"	 AND CAST_NO     = @cast_no "
						"	 AND CAST_DIV_NO > @cast_div_no "
						);
				break;
			}
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_upd.Parameters.Set("cc_mach_no", tpssm11["CC_MACH_NO"].ToString());
			cmd_upd.Parameters.Set("cast_no", tpssm11["CAST_NO"].ToString());
			cmd_upd.Parameters.Set("cast_div_no", tpssm11["CAST_DIV_NO"].ToDecimal());
			cmd_upd.ExecuteNonQuery();

		}//while
		cmd_inq.Close();


		//--------------------------------------
		//2.各炉次开浇时刻重新计算
		//ret = f_pssm11_pour_time(&inBlock3, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//--------------------------------------
		//重新计算工序作业时刻，这个有点麻烦，浇次受影响的炉次都要传入。
		//3.根据传入的炉次，依据CAST的时刻，推算各工序的作业时刻（有模型计算，可省略本计算）
		//ret = f_pssm12_time_calc(&inBlock3, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//----------------------------------------------------------
		//模型计算
		//mode=0 正常编制   mode=1 时间优化 mode=2 局部编制  mode=3 转炉编制  mode=4 非转炉编制 mode=5 模铸编制
		//mode = bcls_rec->Tables[1].Rows[0]["MODE"];
		mode = 1;
		bof_cc_matching = 1;
		//暂不调用模型，通过画面的优化按钮计算
		//ret = f_pssm_call_tps(v_sm_unit_no, mode, bof_cc_matching, conn);
		if (ret < 0)
		{
			//strcpy(s.msg, "模型调用出错.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		inBlock3.Tables[0].Rows.Add();
		inBlock3.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

		//----------------------------------------------------------
		//4.重新计算各工序流水号（预定炉号）
		ret = f_pssm12_sequ_calc_n(&inBlock3, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------------
		//5.浇铸顺联动调整(一定在计算CAST后调用)，不做联动
		//ret = f_pssm10_upd_by_cast(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------------
		//6.重新计算计划号（某炉次删除后，计划号空出，不必重新计算，通过优化计算）
		//ret = f_pssm11_planno(&inBlock1, bcls_ret);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}



		////----------------------------------------------------------
		////修改出钢计划应答表的计划编辑标记（整体计划状态）
		//sqlstr = CString(
		//	" UPDATE TPSSM23 SET "
		//	" PLAN_EDIT_FLAG = '2' " //2-删除计划；
		//	",PLAN_EDIT_TIME = @date_time " //计划编辑时刻
		//	" WHERE FACTORY_DIV = @factory_div "
		//	);
		//cmd_upd.SetCommandText(sqlstr);
		//cmd_upd.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
		//cmd_upd.Parameters.Set("date_time", date_time);
		//cmd_upd.ExecuteNonQuery();

	}
	catch(CDbException& ex)  //用于捕获数据库操作异常
	{

		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)
	{
		s.flag = -1;
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}

