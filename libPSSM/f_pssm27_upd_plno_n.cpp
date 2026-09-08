/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-17
Version:1.0
Description: 出钢计划计划号记录表
Update: 2015-04-07 lijie 更新数据表
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




BM2_FUNCTION_EXPORT
 int f_pssm27_upd_plno_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i, rows, blkseq;
	long l_sm_plan_no=0;       //整型计划顺序号
	long l_sm_plan_no_used=0;  //整型已使用计划顺序号
	long kk=0;
	//int  unused_flag;          //未使用标记: 1-未使用
	CString	datetime="";            /* 记录创建时刻 */
	CDecimal	dummy=0;
	CString    sm_plan_no_used="";     /* 当前已使用的计划顺序号 */

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm27("TPSSM27");
	CDbCommand cmd_tpssm27_inq(conn);
	CDbCommand cmd_tpssm27_del(conn);
	CString sqlstr = CString("");


	try
	{
		datetime=CDateTime::Now().ToString("yyyyMMddHHmmss");

		blkseq = bcls_rec->Tables.IndexOf("PLAN_NO");
		if (blkseq < 0)
		{
			strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		//对输入信息循环处理
		for (i = 1;  i <= rows;  i++ )
		{
			//取得单行传入信息
			//tpssm27.MergeFrom(bcls_rec->Tables[blkseq-1].Rows[i-1]);
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i-1]["FACTORY_DIV"];
			tpssm11["PONO"] = bcls_rec->Tables[blkseq].Rows[i-1]["PONO"];
			tpssm27["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];

			//读取当前PONO的信息			
			sqlstr = "tpssm11.Query(PONO)";
			tpssm11.Query("FACTORY_DIV,PONO");

			tpssm11["SM_PLAN_NO"]=tpssm11["SM_PLAN_NO"].ToString().TrimOrBlank();
			tpssm27["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];

			////Log::Info("", __FUNCTION__, "pono=[{0}], sm_plan_no=[{1}], pono_status=[{2}]",
			//	(const char*)tpssm11["PONO"].ToString(),(const char*)tpssm11["SM_PLAN_NO"].ToString(),tpssm11["PONO_STATUS"].ToDecimal().ToInt32());

			//判断PONO状态, 非BOF区域不做任何处理
			if (tpssm11["PONO_STATUS"].ToDecimal() < 20 || tpssm11["PONO_STATUS"].ToDecimal() > 83) //40改为82, 因为BOF信号不来就会漏掉 
			{
				continue;
			}

			//读取当前作业计划号计录表数据
			//查询已使用的计划顺序号			
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT MAX(SM_PLAN_NO) FROM TPSSM27 "
						 "  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
						 "    AND USE_STATUS        = '1' ";
				break;
			}

			cmd_tpssm27_inq.SetCommandText(sqlstr);
			cmd_tpssm27_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm27_inq.ExecuteReader();
			if(cmd_tpssm27_inq.Read())
			{
				sm_plan_no_used = cmd_tpssm27_inq.GetString(1);
			}
			else
			{
				sm_plan_no_used = " ";
			}
			cmd_tpssm27_inq.Close();
			sm_plan_no_used=sm_plan_no_used.TrimOrBlank();
			
			////Log::Info("", __FUNCTION__, "已使用的计划顺序号=[{0}]", sm_plan_no_used);
			//判断记录已使用的计划顺序号是否为空, 
			if (sm_plan_no_used[0] == ' ')
			{
				//如果记录已使用的计划顺序号为空, 则将当前PONO的作业计划号写入记录表
				tpssm27["USE_STATUS"] = "1";
				
				// DM8 适配 CHANGE-388（HR-003 已确认 2026-09-08）：删除无计划号的 TPSSM27 记录，SM_PLAN_NO 为 NULL、空串、纯空格三种情况一起删除。
				// 改写原因：原写法 TRIM(SM_PLAN_NO) IS NULL 依赖"TRIM 后空串视为 NULL"的兼容行为；DM8 不同兼容模式下空串与 NULL 的处理不一致，可能漏删。
				// 改写口径：COALESCE(LENGTH(TRIM(SM_PLAN_NO)), 0) = 0 不做空串字面量比较，任何兼容配置下三种情况都命中；分厂条件与绑定参数保持不变。
				// 原方案A（完整保留）：
				// sqlstr = "DELETE FROM TPSSM27 "
				// " WHERE FACTORY_DIV = @tpssm27.FACTORY_DIV "
				// "   AND TRIM(SM_PLAN_NO)is NULL ";
				// 方案B（DM8 SQL）：
				sqlstr = "DELETE FROM TPSSM27 "
						 " WHERE FACTORY_DIV = @tpssm27.FACTORY_DIV "
						 "   AND COALESCE(LENGTH(TRIM(SM_PLAN_NO)), 0) = 0 ";
				cmd_tpssm27_del.SetCommandText(sqlstr);
				cmd_tpssm27_del.Parameters.Set("tpssm27.FACTORY_DIV",tpssm27["FACTORY_DIV"].ToString());
				cmd_tpssm27_del.ExecuteNonQuery();

				sqlstr = "tpssm27.Insert() -1 ";
				tpssm27.Insert();

			}
			else
			{
				l_sm_plan_no_used = atol(sm_plan_no_used);
				l_sm_plan_no      = atol(tpssm27["SM_PLAN_NO"].ToString());

				//查询当前PONO的计划号是否存在
				dummy = 0;
				
				tpssm27["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm27["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				dummy = tpssm27.QueryCount("FACTORY_DIV,SM_PLAN_NO");
				if (dummy == 0)  //不存在
				{
					//与已有的计划号比较, 取较大的
					if (l_sm_plan_no_used < l_sm_plan_no) //( 已有的 < 当前的 )
					{
						//修改已使用的计划号	
						tpssm27["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
						tpssm27["USE_STATUS"] = "1";
						sqlstr = "tpssm27.Update(SM_PLAN_NO)";
						tpssm27.Update("SM_PLAN_NO","FACTORY_DIV,USE_STATUS");

						//记录跳跃的计划号
						if ( (l_sm_plan_no - l_sm_plan_no_used) > 10 ) // (当前的 - 已有的) > 1 时, 计划号有跳跃
						{
							//记录跳跃的计划号, 表示未被使用
							for (kk = l_sm_plan_no_used+10 ;  kk < l_sm_plan_no ;  kk = kk + 10)//wcy 计划号按10递增 个位作为分罐号处理 限制为7位整数 6位流水
							{
								//sprintf(tpssm27["SM_PLAN_NO"].ToString(), "%ld", kk);
								tpssm27["SM_PLAN_NO"] = tpssm27["SM_PLAN_NO"].ToString().Format("%ld", kk);
								tpssm27["USE_STATUS"] = " ";
							
								sqlstr = "tpssm27.Insert() -2 ";
								tpssm27.Insert();
							}
						}
					}
				}
				else  //存在
				{
					//读取使用标记
					
					switch(conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = " SELECT USE_STATUS FROM TPSSM27 "
								 "  WHERE FACTORY_DIV	= @tpssm11.FACTORY_DIV "
								 "    AND SM_PLAN_NO	= @tpssm11.SM_PLAN_NO ";
						break;
					}

					cmd_tpssm27_inq.SetCommandText(sqlstr);
					cmd_tpssm27_inq.Parameters.Set("tpssm11.FACTORY_DIV",tpssm11["FACTORY_DIV"].ToString());
					cmd_tpssm27_inq.Parameters.Set("tpssm11.SM_PLAN_NO",tpssm11["SM_PLAN_NO"].ToString());
					cmd_tpssm27_inq.ExecuteReader();
					if(cmd_tpssm27_inq.Read())
					{
						tpssm27["USE_STATUS"] = cmd_tpssm27_inq.GetString(1);
					}
					cmd_tpssm27_inq.Close();
					tpssm27["USE_STATUS"]=tpssm27["USE_STATUS"].ToString().TrimOrBlank();

					//判断使用标记
					if (tpssm27["USE_STATUS"].ToString()[0] == '1') //使用中
					{
						//不做任何操作
					}
					else //未使用
					{
						//与已使用的计划号比较, 取较大的
						if (l_sm_plan_no_used < l_sm_plan_no) //( 已有的 < 当前的 )
						{
							//该分支一般不会进入, 即被记录且大于已使用的计划号
							//删除已有的
							
							tpssm27["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
							tpssm27["SM_PLAN_NO"] = sm_plan_no_used.TrimOrBlank();
							sqlstr = "tpssm27.Delete(SM_PLAN_NO) -1 ";
							tpssm27.Delete("FACTORY_DIV,SM_PLAN_NO");

							//记录最大的(当前的)
							
							tpssm27["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
							tpssm27["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
							tpssm27["USE_STATUS"] = "1";
							tpssm27.Update("USE_STATUS","FACTORY_DIV,SM_PLAN_NO");
						}
						else if (l_sm_plan_no_used > l_sm_plan_no) //( 已有的 < 当前的 )
						{
							//删除当前的
							
							tpssm27["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
							//HYF 0228修改 tpssm27["SM_PLAN_NO"] = sm_plan_no_used.TrimOrBlank(); 这里删除错误了
							tpssm27["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
							sqlstr = "tpssm27.Delete(SM_PLAN_NO) -2 ";
							tpssm27.Delete("FACTORY_DIV,SM_PLAN_NO");
						}
						//else
						//{  //l_sm_plan_no_used == l_sm_plan_no 的情况，必定是tpssm27["USE_STATUS"].ToString()[0] == '1'
						//}
					}
				}//是否存在

			}//修改已使用的
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

	cmd_tpssm27_inq.Close();
	return doFlag;
}

