/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-03-04
Version:1.0
Description: 新增制造命令
**************************************************/
// C 的标准头文件部分  

#include "stdafx.h"





//#include "tpssmd9.h"
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);
int f_pssm10f5_del(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表
/*<remark>=========================================================
/// <summary>
///  删除后备制造命令
/// <para> 删除后备制造命令；</para>
/// <para>数据库表：TPSSM01/02/03/10               </para>
/// <para>主调用函数：TPSSM09画面F4删除调用。        </para>
/// </summary>
/// <param name="main_backlog_code">炼钢主工序代码    </param>
/// <returns>制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm10f5_del)

int f_pssm10f5_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{ 
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	CString sqlstr = "";
	CDecimal dummy = 0, cc_seq = 0;
	int i = 0, ret = 0;

	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_tpssm03_upd(conn);
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CModel tpssm40("TPSSM40");
	CModel tpssm99("TPSSM99");

	EIClass outBlock;
	EIClass inBlock;
	EIClass in_pssm99trace;  //调用履历函数

	inBlock.Tables[0].set_TableName("X200009");
	inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");
	inBlock.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
	inBlock.Tables[0].Columns.Add(DT_STRING, "SM_PLAN_NO");
	inBlock.Tables[0].Rows.Add();  //只生成一行

	in_pssm99trace.Tables[0].set_TableName("TRACE");
	in_pssm99trace.Tables[0].Clone(tpssm99);

	try
	{
		/* 对输入信息循环处理 */
		for (i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			/* PONO删除逻辑
			1、判断当前PONO状态，如已编入出钢计划(18)，报错退出
			2、对于TPSSM10表，是开浇第一炉(带“T”)，T传递给下一炉，包括准备时间
			3、对于TPSSM01表，删除PONO及对应铸坯（对于浮动铸坯命令，不删除）
			*/
			tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"];

		/*	Log::Trace("", __FUNCTION__, "tpssm10["PONO"] =[{0}]", tpssm10["PONO"].ToString());*/
			tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];

			//--------------------------
			//1、浇铸计划表处理

			sqlstr = "tpssm10.Query()";
			bool has10 = tpssm10.Query();
			Log::Trace("", __FUNCTION__, "has10=[{0}]", has10);

			if (has10 == true) //有记录
			{
				//1）状态校验
				if (tpssm10["PONO_STATUS"].ToDecimal() >= 18) //编入出钢计划或生产，不能删除
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "要删除炉次[{0}]已排入出钢计划，不能删除。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tpssm01["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssm01["PONO"] = tpssm10["PONO"];

				if (tpssm01.Query("FACTORY_DIV, PONO"))
				{
					//西王项目后备新增用，借用字段判断是否属于后备新增PONO
					//if (tpssm01["CC_PLAN_MODE"].ToString() != "B")
					//{
					//	CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					//	CMessageFormat::Format(s.msg, "制造命令号[{0}]不是后备新增的计划，不能删除。", arguments, 1);
					//	throw CApplicationException(-1, s.msg, log.Location);
					//}
				}

				//2）"T"标记传递给下一炉
				if (tpssm10["RESTRAND_FLG"].ToString().Trim() != "")
				{

					Log::Trace("", __FUNCTION__, "cc_mach_no =[{0}]", tpssm10["CC_MACH_NO"].ToString());
					//读取下一炉的cc_seq（考虑跳号可能性。重号就没办法了）
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:         // MS SQL Server数据库
					case DB_KIND_ORACLE:        // Oracle 数据库
					default:  // 所有数据库适用，通用SQL语句。
						sqlstr = CString(
							" SELECT MIN(cc_seq) FROM TPSSM10 "
							"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
							"    AND CC_MACH_NO = @tpssm10.CC_MACH_NO "
							"    AND CC_SEQ     > @tpssm10.CC_SEQ "
							);
						break;
					}
					cmd_tpssm10_inq.SetCommandText(sqlstr);
					cmd_tpssm10_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
					cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
					cmd_tpssm10_inq.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
					cmd_tpssm10_inq.ExecuteReader();
					if (cmd_tpssm10_inq.Read())
					{
						cc_seq = cmd_tpssm10_inq.GetDecimal(1);
					}
					else
					{
						cc_seq = 0;
					}

					if (cc_seq > 0)//有下一炉
					{
						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:         // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default:  // 所有数据库适用，通用SQL语句。
							sqlstr = CString(
								" UPDATE TPSSM10 "
								"    SET RESTRAND_FLG = 'T', "
								"        CC_PREP_TIME = @cc_prep_time "  //炉间准备时间
								"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
								"    AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
								"    AND CC_SEQ     = @cc_seq "
								"    AND RESTRAND_FLG <> 'T' "  //下一炉如果有T标记，不做修改
								);
							break;
						}
						cmd_tpssm10_upd.SetCommandText(sqlstr);
						cmd_tpssm10_upd.Parameters.Set("cc_prep_time", tpssm10["CC_PREP_TIME"].ToDecimal());
						cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString().Trim());
						cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString().Trim());
						cmd_tpssm10_upd.Parameters.Set("cc_seq", cc_seq);
						cmd_tpssm10_upd.ExecuteNonQuery();


					}//if (cc_seq > 0) 有下一炉

				}//if 带T

				tpssm40.CopyFrom(tpssm10);
				tpssm40["PONO_STATUS"] = 91;
				tpssm40.Insert();
				//删除PONO
				sqlstr = "tpssm10.Delete()";
				tpssm10.Delete();


				//后序的炉次浇注顺序号向上移
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = CString(
						" UPDATE TPSSM10 "
						"   SET CC_SEQ = CC_SEQ - 1 "
						"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
						"    AND CC_MACH_NO	= @tpssm10.CC_MACH_NO "
						"    AND CC_SEQ     > @tpssm10.CC_SEQ "
						"    AND CC_SEQ	    < 900 "
						);
					break;
				}
				cmd_tpssm10_upd.SetCommandText(sqlstr);
				cmd_tpssm10_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_MACH_NO", tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssm10_upd.Parameters.Set("tpssm10.CC_SEQ", tpssm10["CC_SEQ"].ToDecimal());
				cmd_tpssm10_upd.ExecuteNonQuery();

			}//if  TPSSM10 有记录

			Log::Trace("", __FUNCTION__, "111");
			//--------------------------
			//2、炉次命令表（TPSSM01）处理
			//读取要删除PONO信息
			tpssm01["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
			tpssm01["PONO"] = tpssm10["PONO"];
			sqlstr = "tpssm01.Query()";
			dummy = tpssm01.QueryCount("FACTORY_DIV,PONO");

			Log::Trace("", __FUNCTION__, "222");
			//删除命令炉次表
			sqlstr = "tpssm01.Delete()";
			tpssm01.Delete();

			//--------------------------
			//2、LOT命令表（TPSSM02）处理
			//读取要删除LOT信息
			tpssm02["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
			tpssm02["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
			sqlstr = "tpssm02.Query()";

			//删除LOT命令表
			sqlstr = "tpssm02.Delete()";
			tpssm02.Delete();

			Log::Trace("", __FUNCTION__, "333");
			//--------------------------
			//4、删除板坯表(TPSSM03)
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = CString(
					" DELETE TPSSM03 "
					"  WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV "
					"    AND PONO	= @tpssm10.PONO "
					);
				break;
			}
			cmd_tpssm03_upd.SetCommandText(sqlstr);
			cmd_tpssm03_upd.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
			cmd_tpssm03_upd.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
			cmd_tpssm03_upd.ExecuteNonQuery();

			if (tpssm10["PONO"][0] == '9')
			{
				Log::Trace("", __FUNCTION__, "PONO =[{0}]为后备命令，不发送炉次关闭电文", tpssm10["PONO"].ToString());
			}
			else
			{
				//-----------------------------------------------
				//发送炼钢PONO状态
				inBlock.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				inBlock.Tables[0].Rows[0]["PONO"] = tpssm10["PONO"];
				inBlock.Tables[0].Rows[0]["PONO_STATUS"] = 91;

				ret = f_cm_200009_snd(&inBlock, bcls_ret, conn);
				if (ret != 0) //调用不成功
				{
					//////Log::Trace("", __FUNCTION__,  "f_ps200009_snd()发送炼钢PONO状态出错:[{0}]", (const char*) s.msg);
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			//写计划履历表
			tpssm99["EVENT_ID"] = "1D";
			tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
			tpssm99["PONO"] = tpssm10["PONO"];
			tpssm99["VALID_FLAG"] = "1";//操作成功
			tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

			//记录编入计划成功的履历
			ret = 0;
			ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
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
	cmd_tpssm10_inq.Close();

	return doFlag;
}
