/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   xuwen
Date:     2014-11-27
Version:  3.1.0
Description: 出钢计划删除（甘特图）
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




//#include "tpssm26.h"


#if defined _SYS_PES

BM2_FUNCTION_IMPORT
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入

/*<remark>=========================================================
/// <summary>
/// 出钢计划删除（甘特图）
/// <para> 根据计划编制标志，删除为“D”的炉次     </para>
/// <para> 1.检查计划的状态,是否已进入生产, 已进入的则报错退出；</para>
/// <para> 2.删除该PONO，并同步更新TPSSM01、TPSSM10表；</para>
/// <para> 3.判断该炉次的CAST的是否还有炉次：</para>
/// <para>   1) 没有则将后续的CAST号向前移；</para>
/// <para>   2) 有则重新统计CAST内炉总数。</para>
/// <para>数据库表：tpssm11/12(炼钢出钢计划主表)</para>
/// <para>前台画面PSSM18的删除按钮和pssm11_save</para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码</param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm11_del_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	/* 程序用变量 */
	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i;

	EIClass inBlock;  //向MMS发送电文用
	EIClass in_pssm99trace;  //调用履历函数

	//业务用变量
	CString dateNow14 = "";
	CString v_factory_div = "";	//炼钢单元号
	CString v_pono = " ";

	CString special_flag = "";
	
	CModel tpssm01("TPSSM01");
	CModel tpssm10("TPSSM10");
	CModel tpssm17("TPSSM17");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssm99("TPSSM99");
	CModel tpssm12z("TPSSM12Z");
	//CTPSSM16 tpssm16(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm15_inq(conn);
	CDbCommand cmd(conn);
	CString sqlstr;

	try
	{
		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);

		//--------------------------------
		//定义函数调用输入块结构
		//1.增加向MMS发送炉次状态电文,单记录
		inBlock.Tables[0].set_TableName("X200009");
		inBlock.Tables[0].Columns.Add(DT_STRING,  "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING,  "PONO");
		inBlock.Tables[0].Columns.Add(DT_STRING,  "PONO_STATUS");
		inBlock.Tables[0].Rows.Add();  //只生成一行


		//-----------------------------------------------------
		// 读取输入信息，单记录
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm11_del_heat_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		if (bcls_rec->Tables[blkseq].Columns.Contains("SPECIAL_FLAG"))
		{
			special_flag = bcls_rec->Tables[blkseq].Rows[0]["SPECIAL_FLAG"].ToString();
		}

		////Log::Info(" ", __FUNCTION__, "factory_div =[{0}]", v_factory_div);


		tpssm10["REC_REVISOR"] = CString(s.userid);
		tpssm10["REC_REVISE_TIME"] = dateNow14;
		tpssm01["REC_REVISOR"] = CString(s.userid);
		tpssm01["REC_REVISE_TIME"] = dateNow14;
		tpssm10["FACTORY_DIV"] = v_factory_div;
		tpssm01["FACTORY_DIV"] = v_factory_div;


		if (special_flag != "1")
		{

			//查询计划编制标志为“D”的炉次，做相应数据处理
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM TPSSM11 "
					"  WHERE FACTORY_DIV     = @v_factory_div "
					"    AND PLAN_EDIT_FLAG = 'D' "
					);
				break;
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				cmd_tpssm11_inq.Fetch(tpssm11);
				//tpssm11.TrimOrBlank();
				////Log::Info(" ", __FUNCTION__, "tpssm11["PONO"] ={0}",tpssm11["PONO"].ToString());

				//1.判断状态是否允许删除
				if (tpssm11["PONO_STATUS"].ToDecimal() >= 20) //已进入生产
				{
					CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000063")/*制造命令号[{0}]已生产, 不能删除。*/, arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//1)修改制造命令炉次表(tpssm10)中状态信息
				////Log::Info(" ", __FUNCTION__, "PONO =[{0}]", tpssm11["PONO"].ToString());
				tpssm10["FACTORY_DIV"] = v_factory_div;
				tpssm10["PONO"] = tpssm11["PONO"];
				tpssm10["PONO_STATUS"] = 16;
				tpssm10["REC_REVISOR"] = CString(s.userid);
				tpssm10["REC_REVISE_TIME"] = dateNow14;
				sqlstr = "tpssm10.Update(PONO_STATUS)";
				tpssm10.Update(
					"PONO_STATUS,"
					"REC_REVISOR,REC_REVISE_TIME",
					"FACTORY_DIV,PONO"); //主键


				//2) 修改制造命令炉次表(tpssm01)中状态信息
				tpssm01["PONO_STATUS"] = 15;
				tpssm01["PONO"] = tpssm10["PONO"];
				tpssm01["FACTORY_DIV"] = v_factory_div;
				sqlstr = "tpssm01.Update(PONO_STATUS)";
				tpssm01.Update(
					"PONO_STATUS,"
					"REC_REVISOR,REC_REVISE_TIME",
					"FACTORY_DIV,PONO"); //主键


#ifdef _SYS_PES    
				//-----------------------------------------------
				//发送炼钢PONO状态
				inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				inBlock.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
				inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 15;

				//ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);  //wcy 暂时取消
				if (ret != 0) //调用不成功
				{
					//////Log::Trace("", __FUNCTION__,  "f_ps200009_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
#endif
				tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm99["EVENT_ID"] = "B2";
				tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
				tpssm99["PONO"] = tpssm11["PONO"];
				tpssm99["VALID_FLAG"] = "1";//操作成功
				/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
				in_pssm99trace.Tables[0].Rows.Clear();
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
			}//while 


			//删除计划编制标志为“D”的炉次，做相应数据处理
			//1)删除计划子表
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" DELETE FROM TPSSM12 "
					"  WHERE FACTORY_DIV     = @v_factory_div "
					"    AND SM_PLAN_NO in ( "
					"      SELECT SM_PLAN_NO FROM TPSSM11 "
					"       WHERE FACTORY_DIV = @v_factory_div "
					"         AND PLAN_EDIT_FLAG = 'D' ) "
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("v_factory_div", v_factory_div);
			cmd.ExecuteNonQuery();

			//1)删除中频炉对应关系表
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = " update TPSSM12Z set SM_PLAN_NO=' ',PONO=' ',ST_NO=' ',HEAT_NO=' ',SM_PLAN_NOL2=' ' where SM_PLAN_NO in ( "
					"      SELECT SM_PLAN_NO FROM TPSSM11 "
					"       WHERE FACTORY_DIV = @v_factory_div "
					"         AND PLAN_EDIT_FLAG = 'D' ) "
					;
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("v_factory_div", v_factory_div);
			cmd.ExecuteNonQuery();


			//2)删除计划主表
			tpssm11["FACTORY_DIV"] = v_factory_div;
			tpssm11["PLAN_EDIT_FLAG"] = "D";
			sqlstr = "tpssm11.Delete()";
			tpssm11.Delete("FACTORY_DIV,PLAN_EDIT_FLAG");
		}
		else//wcy 日平衡逻辑
		{
			//查询计划编制标志为“D”的炉次，做相应数据处理
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT * FROM TPSSM15 "
					"  WHERE FACTORY_DIV     = @v_factory_div "
					"    AND PLAN_EDIT_FLAG = 'D' "
					);
				break;
			}
			cmd_tpssm15_inq.SetCommandText(sqlstr);
			cmd_tpssm15_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_tpssm15_inq.ExecuteReader();
			while (cmd_tpssm15_inq.Read())
			{
				cmd_tpssm15_inq.Fetch(tpssm15);
				//tpssm15.TrimOrBlank();
				////Log::Info(" ", __FUNCTION__, "tpssm15["PONO"] ={0}",tpssm15["PONO"].ToString());

				//1.判断状态是否允许删除
				if (tpssm15["PONO_STATUS"].ToDecimal() >= 20) //已进入生产
				{
					CFormattable arguments[] = { tpssm15["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000063")/*制造命令号[{0}]已生产, 不能删除。*/, arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//1)修改制造命令炉次表(tpssm17)中状态信息
				//Log::Info(" ", __FUNCTION__, "PONO =[{0}]", tpssm15["PONO"].ToString());
				tpssm17["FACTORY_DIV"] = v_factory_div;
				tpssm17["PONO"] = tpssm15["PONO"];
				tpssm17["PONO_STATUS"] = 16;
				tpssm17["REC_REVISOR"] = CString(s.userid);
				tpssm17["REC_REVISE_TIME"] = dateNow14;
				sqlstr = "tpssm17.Update(PONO_STATUS)";
				tpssm17.Update(
					"PONO_STATUS,"
					"REC_REVISOR,REC_REVISE_TIME",
					"FACTORY_DIV,PONO"); //主键


				//2) 修改制造命令炉次表(tpssm01)中状态信息
				//tpssm01["PONO_STATUS"] = 15;
				//tpssm01["PONO"] = tpssm17["PONO"];
				//tpssm01["FACTORY_DIV"] = v_factory_div;
				//sqlstr = "tpssm01.Update(PONO_STATUS)";
				//tpssm01.Update(
				//	"PONO_STATUS,"
				//	"REC_REVISOR,REC_REVISE_TIME",
				//	"FACTORY_DIV,PONO"); //主键


#ifdef _SYS_PES    
				//-----------------------------------------------
				//发送炼钢PONO状态
				//inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm15["FACTORY_DIV"];
				//inBlock.Tables[0].Rows[0]["PONO"] = tpssm15["PONO"];
				//inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 15;

				//ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);  //wcy 暂时取消
				//if (ret != 0) //调用不成功
				//{
					//////Log::Trace("", __FUNCTION__,  "f_ps200009_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					//throw CApplicationException(-1, s.msg, log.Location);
				//}
#endif
				//tpssm99["FACTORY_DIV"] = tpssm15["FACTORY_DIV"];
				//tpssm99["EVENT_ID"] = "B2";
				//tpssm99["PONO_STATUS"] = tpssm15["PONO_STATUS"];
				//tpssm99["PONO"] = tpssm15["PONO"];
				//tpssm99["VALID_FLAG"] = "1";//操作成功
				/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
				//in_pssm99trace.Tables[0].Rows.Clear();
				//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
				////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

				////Log::Trace("", __FUNCTION__, "记录成功履历，传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
				//记录编入计划成功的履历
				//ret = 0;
				//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				//if (ret < 0)
				//{
					//throw CApplicationException(-1, s.msg, log.Location);
				//}
			}//while 


			//删除计划编制标志为“D”的炉次，做相应数据处理
			//1)删除计划子表
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" DELETE FROM TPSSM16 "
					"  WHERE FACTORY_DIV     = @v_factory_div "
					"    AND SM_PLAN_NO in ( "
					"      SELECT SM_PLAN_NO FROM TPSSM15 "
					"       WHERE FACTORY_DIV = @v_factory_div "
					"         AND PLAN_EDIT_FLAG = 'D' ) "
					);
				break;
			}
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("v_factory_div", v_factory_div);
			cmd.ExecuteNonQuery();


			//2)删除计划主表
			tpssm15["FACTORY_DIV"] = v_factory_div;
			tpssm15["PLAN_EDIT_FLAG"] = "D";
			sqlstr = "tpssm15.Delete()";
			tpssm15.Delete("FACTORY_DIV,PLAN_EDIT_FLAG");
		}


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
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
