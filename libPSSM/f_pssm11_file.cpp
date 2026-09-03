/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:weihuamei
Date:2015-05-14
Version:1.0
Description: 出钢计划归档
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



//#include "tpssm42.h"






/*<remark>=========================================================
/// <summary>
/// 出钢计划归档
/// <para>将出钢计划归入历史表。  </para>
/// <para>数据库表：TPSSM11/12/41/42(出钢计划主表/出钢计划子表/出钢计划历史主表/出钢计划历史子表) </para>///
/// </summary>
/// <param name="factory_div">炼钢主工序代码    </param>
/// <param name="pono">制造命令                       </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/

#if defined _SYS_PES
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //向L4送炉次确定状态 
int f_plan_delete_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表
int f_pssm11_file_force(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //出钢计划归档

BM2_FUNCTION_EXPORT
int f_pssm11_file(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret = 0;
	CString   datetime;
	CString cs_factory_div = "";
	CString cs_sm_plan_no = "";
	CString cs_pono = "";
	CModel tpssm41("TPSSM41");
	CModel tpssm42("TPSSM41");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm13("TPSSM13");//wcy比较表删除
	CModel tpssm14("TPSSM14");
	CModel tpssm10("TPSSM10");
	CModel tpssm40("TPSSM40");
	CModel tpssm01("TPSSM01");
	CModel tpssm35("TPSSM35");
	CModel tpssm99("TPSSM99");
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_tpssm40_ins(conn);
	CDbCommand cmd_tpssm41_ins(conn);
	CDbCommand cmd_tpssm42_ins(conn);
	CString sqlstr;

	EIClass inBlock;  //向MMS发送电文用
	EIClass inBlock2;  //强制归档用
	EIClass in_pssm99trace;  //调用履历函数

	try
	{
		//--------------------------------
		//定义函数调用输入块结构
		//1.增加向MMS发送炉次状态电文,单记录
		inBlock.Tables[0].set_TableName("X200009");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
		//inBlock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO_IN");
		inBlock.Tables[0].Rows.Add();  //只生成一行


		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//5、计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);

		/*for(int i=0;i<bcls_rec->Tables[0].Rows.get_Count();i++)
		{*/
		cs_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		cs_sm_plan_no = bcls_rec->Tables[0].Rows[0]["SM_PLAN_NO"].ToString().Trim();
		cs_pono = bcls_rec->Tables[0].Rows[0]["PONO"].ToString().Trim();

		////Log::Trace("", __FUNCTION__, "cs_sm_plan_no=[{0}]", cs_sm_plan_no);

		tpssm01["REC_REVISOR"] = s.userid;
		tpssm01["REC_REVISE_TIME"] = datetime;
		tpssm01["PONO_STATUS"] = 91;
		//tpssm01.HEAT_CONFM_TIME = datetime;
		tpssm01["FACTORY_DIV"] = cs_factory_div;
		tpssm01["PONO"] = cs_pono;
		tpssm01.Update("REC_REVISOR,REC_REVISE_TIME,PONO_STATUS", "FACTORY_DIV,PONO");

		tpssm11["FACTORY_DIV"] = cs_factory_div;
		tpssm11["SM_PLAN_NO"] = cs_sm_plan_no;
		tpssm11["PONO"] = cs_pono;


		////Log::Trace("", __FUNCTION__, "tpssm11["SM_PLAN_NO"] =[{0}]", tpssm11["SM_PLAN_NO"].ToString());
		////Log::Trace("", __FUNCTION__, " factory_div=[{0}], pono=[{1}],sm_plan_no=[{2}]", tpssm11["FACTORY_DIV"].ToString(), tpssm11["PONO"].ToString(), tpssm11["SM_PLAN_NO"].ToString());

		tpssm41["FACTORY_DIV"] = cs_factory_div;
		tpssm41["SM_PLAN_NO"] = cs_sm_plan_no;
		tpssm41["PONO"] = cs_pono;

		//检查该计划是否在历史表存在，不存在说明未归档
		if (tpssm41.QueryCount("FACTORY_DIV,PONO") <= 0)
		{

			//未生产的不归档
			tpssm11.Query("FACTORY_DIV,PONO");

#if defined _SYS_PES
			inBlock.Tables[0].Rows[0]["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			//inBlock.Tables[0].Rows[0]["SM_PLAN_NO_IN"] = tpssm11["SM_PLAN_NO"];
			ret = f_plan_delete_snd2(&inBlock, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
#endif
			if (tpssm11["PONO_STATUS"].ToDecimal() >= 83 && tpssm11["RUN_STATUS"].ToString() >= "53")
			{
				//更新制造命令状态
				tpssm10["REC_REVISOR"] = s.userid;
				tpssm10["REC_REVISE_TIME"] = datetime;
				tpssm10["PONO_STATUS"] = 91;
				//tpssm10.HEAT_CONFM_TIME = datetime;
				tpssm10["FACTORY_DIV"] = cs_factory_div;
				tpssm10["PONO"] = cs_pono;
				tpssm10.Update("REC_REVISOR,REC_REVISE_TIME,PONO_STATUS", "FACTORY_DIV,PONO");

				tpssm11["REC_REVISOR"] = s.userid;
				tpssm11["REC_REVISE_TIME"] = datetime;
				tpssm11["PONO_STATUS"] = 91;
				tpssm11["HEAT_CONFM_TIME"] = datetime;
				tpssm11.Update("REC_REVISOR,REC_REVISE_TIME,PONO_STATUS,HEAT_CONFM_TIME", "FACTORY_DIV,PONO");



				//更新查询浇注顺序，如果浇注结束信号没有导致浇注顺没有调整，在炉次确定或强制时调整
				tpssm10["FACTORY_DIV"] = cs_factory_div;
				tpssm10["PONO"] = cs_pono;
				tpssm10.Query("FACTORY_DIV,PONO");
				////Log::Trace("", __FUNCTION__, "tpssm10["CC_SEQ"] =[{0}]", tpssm10["CC_SEQ"].ToDecimal());
				if (tpssm10["CC_SEQ"].ToDecimal() > 0)
				{
					tpssm10["CC_SEQ"] = 0;
					tpssm10.Update("CC_SEQ", "FACTORY_DIV,PONO");
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
						sqlstr = " UPDATE TPSSM10 \
							 		SET  CC_SEQ           = CC_SEQ - 1 \
								 WHERE FACTORY_DIV = TRIM(@tpssm10.FACTORY_DIV) \
									AND CC_MACH_NO   = TRIM(@tpssm10.CC_MACH_NO) \
									AND CC_SEQ       > @tpssm10.CC_SEQ \
																																																				AND CC_SEQ       < 900  ";//挂起的 cc_seq 不改 
						break;
					}
					cmd_tpssm10_upd.SetCommandText(sqlstr);
					////Log::Trace("", __FUNCTION__, "111sqlstr=[{0}]", sqlstr);
					cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
					cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
					cmd_tpssm10_upd.ExecuteNonQuery();
					cmd_tpssm10_upd.Close();

				}

				//计划主表归档操作
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" INSERT INTO TPSSM41 ( "
						" SELECT * FROM TPSSM11 "
						" WHERE FACTORY_DIV = @cs_factory_div "
						" AND SM_PLAN_NO   = @cs_sm_plan_no )"
						);
					break;
				}
				cmd_tpssm41_ins.SetCommandText(sqlstr);
				cmd_tpssm41_ins.Parameters.Set("cs_factory_div", cs_factory_div);
				cmd_tpssm41_ins.Parameters.Set("cs_sm_plan_no", cs_sm_plan_no);

				cmd_tpssm41_ins.ExecuteNonQuery();
				cmd_tpssm41_ins.Close();
				//计划子表归档操作
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" INSERT INTO TPSSM42 ( "
						" SELECT * FROM TPSSM12 "
						" WHERE FACTORY_DIV = @cs_factory_div "
						" AND SM_PLAN_NO   = @cs_sm_plan_no ) "
						);
					break;
				}
				cmd_tpssm42_ins.SetCommandText(sqlstr);

				cmd_tpssm42_ins.Parameters.Set("cs_factory_div", cs_factory_div);
				cmd_tpssm42_ins.Parameters.Set("cs_sm_plan_no", cs_sm_plan_no);
				cmd_tpssm42_ins.ExecuteNonQuery();
				cmd_tpssm42_ins.Close();

				//制造命令归档
				//计划子表归档操作
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = CString(
						" INSERT INTO TPSSM40 ( "
						" SELECT * FROM TPSSM10 "
						" WHERE FACTORY_DIV = @cs_factory_div "
						" AND PONO         = @cs_pono ) "
						);
					break;
				}
				cmd_tpssm40_ins.SetCommandText(sqlstr);
				cmd_tpssm40_ins.Parameters.Set("cs_factory_div", cs_factory_div);
				cmd_tpssm40_ins.Parameters.Set("cs_pono", cs_pono);
				cmd_tpssm40_ins.ExecuteNonQuery();
				cmd_tpssm40_ins.Close();

				tpssm11["FACTORY_DIV"] = cs_factory_div;
				tpssm11["SM_PLAN_NO"] = cs_sm_plan_no;
				tpssm13["FACTORY_DIV"] = cs_factory_div;
				tpssm13["SM_PLAN_NO"] = cs_sm_plan_no;

				//删除在线计划					
				tpssm11.Delete("FACTORY_DIV,SM_PLAN_NO");
				tpssm13.Delete("FACTORY_DIV,SM_PLAN_NO");

				tpssm12["FACTORY_DIV"] = cs_factory_div;
				tpssm12["SM_PLAN_NO"] = cs_sm_plan_no;
				tpssm14["FACTORY_DIV"] = cs_factory_div;
				tpssm14["SM_PLAN_NO"] = cs_sm_plan_no;

				tpssm12.Delete("FACTORY_DIV,SM_PLAN_NO");
				tpssm14.Delete("FACTORY_DIV,SM_PLAN_NO");

				tpssm10["FACTORY_DIV"] = cs_factory_div;
				tpssm10["PONO"] = cs_pono;
				tpssm10.Delete("FACTORY_DIV,PONO");

			}
			else
			{
				strcpy(s.msg, "该炉次还未开浇结束，请用强制确定!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		//}	
		////Log::Trace("", __FUNCTION__, "计划归档成功！");


		

#ifdef _SYS_PES    
		//-----------------------------------------------
		//发送炼钢PONO状态
		inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		inBlock.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
		inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 91;

		ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);
		if (ret != 0) //调用不成功
		{
			//////Log::Trace("", __FUNCTION__,  "f_ps200009_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}
#endif

		//写计划履历表
		tpssm99["EVENT_ID"] = "Z1";
		tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = tpssm11["PONO"];
		tpssm99["HEAT_NO"] = tpssm11["HEAT_NO"];
		tpssm99["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
		tpssm99["VALID_FLAG"] = "1";//操作成功
		/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

		////Log::Trace("", __FUNCTION__, "记录成功履历，传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		//记录编入计划成功的履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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
	cmd_tpssm41_ins.Close();
	cmd_tpssm42_ins.Close();
	return doFlag;

}
