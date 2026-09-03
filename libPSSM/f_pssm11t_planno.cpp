/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   lijie
Date:     2011-12-12
Version:  3.1.0
Description: 出钢计划的计划号生成
Update：   2014-11-13  xuwen  炉次条件合并，表结构优化后程序调整
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件




/*<remark>=========================================================
/// <summary>
/// 出钢计划的计划号生成，核心技术是解决跳号问题
/// <para>数据库表：TPSSM11/12/27            </para>
/// <para>主调用函数：
///      pssm11_add (出钢计划编入调用)
///      pssm11_opti(出钢计划优化重计算)
///      pssm11_del (出钢计划删除，计划号空出。调用封掉，通过优化计算)
///      pssm18_save(甘特图出钢计划编制保存)
/// </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/
int f_pssm11t_planno(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq = 0;
	int plan_id = 0;
	CDecimal dummy = 0;
	CString sm_plan_no = "";
	CString sm_plan_no_used = "";  //整型已使用计划顺序号
	CDecimal l_sm_plan_no = 0;       //整型计划顺序号
	CDecimal l_sm_plan_no_used = 0;  //整型已使用计划顺序号
	int  unused_flag = 0;          //未使用标记: 1-未使用
	EIClass inBlock;
	EIClass outBlock;
	CString datatime = "";

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm27("TPSSM27");
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tpssm11_upd(conn);
	CDbCommand cmd_tpssm12_upd(conn);
	CDbCommand cmd_tpssm27_inq(conn);
	CString sqlstr;
	try
	{

		datatime = CDateTime::Now().ToString("yyyyMMddHHmmss");


		//-----------------------------------------------------
		// 获得输入参数
		//读取编入计划的炼钢单元号，单记录方式
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in pssm18_save().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		//tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[0]);


		//查询已使用的计划顺序号
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT MAX(SM_PLAN_NO) FROM TPSSM27 "
				"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
				"    AND USE_STATUS        = '1' "
				);
			break;
		}

		cmd_tpssm27_inq.SetCommandText(sqlstr);
		cmd_tpssm27_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString().TrimOrBlank());
		cmd_tpssm27_inq.ExecuteReader();
		if (cmd_tpssm27_inq.Read())
		{
			sm_plan_no_used = cmd_tpssm27_inq.GetString(1).TrimOrBlank();
		}
		else //没查询到记录
		{
			sm_plan_no_used = " ";
		}
		cmd_tpssm27_inq.Close();

		//如果记录的已使用的计划顺序号为空, 顺序号计数从1开始
		if (sm_plan_no_used == " ")
		{
			l_sm_plan_no_used = 0;
		}
		else
		{
			l_sm_plan_no_used = atol(sm_plan_no_used);
		}


		//查询未使用的计划顺序号，解决跳号问题
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT NVL(MIN(SM_PLAN_NO),' ') FROM TPSSM27 "
				"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
				"    AND USE_STATUS       <> '1' "
				);
			break;
		}

		cmd_tpssm27_inq.SetCommandText(sqlstr);
		cmd_tpssm27_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm27_inq.ExecuteReader();
		if (cmd_tpssm27_inq.Read())
		{
			tpssm27["SM_PLAN_NO"] = cmd_tpssm27_inq.GetString(1).TrimOrBlank();
		}
		else
		{
			tpssm27["SM_PLAN_NO"] = " ";
		}
		cmd_tpssm27_inq.Close();


		//得到当前计划顺序号
		if (tpssm27["SM_PLAN_NO"].ToString()[0] == ' ')
		{
			l_sm_plan_no = l_sm_plan_no_used;
			unused_flag = 0; //无未使用的计划顺序号
		}
		else
		{
			l_sm_plan_no = atol(tpssm27["SM_PLAN_NO"].ToString()) - 1;
			unused_flag = 1; //有未使用的计划顺序号
		}

		////Log::Info("", __FUNCTION__, "l_sm_plan_no = [{0}], l_sm_plan_no_used =[{1}]", l_sm_plan_no, l_sm_plan_no_used);

		//查询转炉工序的计划, 按照处理开始时刻排序		
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:         // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:  // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT PONO, A FROM "
				" (  SELECT MIN(START_TIME) A, PONO FROM "
				"    ( "
				"      SELECT DECODE( TRIM(START_TIME_REAL), NULL, START_TIME, START_TIME_REAL) START_TIME, PONO FROM TPSSM12 "
				"       WHERE FACTORY_DIV = TRIM(@tpssm11.FACTORY_DIV) "
				"         AND AREA_ID = 3  "
				"    ) "
				"    GROUP BY PONO "
				" ) "
				" ORDER BY A ASC "
				);  //此逻辑比较复杂，可以用如下简单的

			//根据计划主表的开始时刻排序计算，不分铸机
			sqlstr = CString(
				" SELECT PONO_STATUS, SM_PLAN_NO, PONO "
				"   FROM TPSSM11 "
				"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
				" ORDER BY STEEL_START_TIME ASC "
				);
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString().TrimOrBlank());
		cmd_tpssm12_inq.ExecuteReader();
		while (cmd_tpssm12_inq.Read())
		{
			//tpssm11["PONO"] = cmd_tpssm12_inq.GetString(1);
			//switch(conn->DatabaseKind)
			//{
			//case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			//case DB_KIND_MSSQL:         // MS SQL Server数据库
			//case DB_KIND_ORACLE:        // Oracle 数据库
			//default:  // 所有数据库适用，通用SQL语句
			//	sqlstr = CString(
			//		" SELECT PONO_STATUS, SM_PLAN_NO FROM TPSSM11 "
			//		"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
			//		"    AND PONO       = @tpssm11.PONO "
			//		);
			//	break;
			//}
			//cmd_tpssm11_inq.SetCommandText(sqlstr);
			//cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString().TrimOrBlank() );
			//cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO"             , tpssm11["PONO"].ToString().TrimOrBlank() );
			//cmd_tpssm11_inq.ExecuteReader();
			//if(cmd_tpssm11_inq.Read())
			//{
			//	tpssm11["PONO_STATUS"] = cmd_tpssm11_inq.GetDecimal(1);
			//	tpssm11.SM_PLAN_NO  = cmd_tpssm11_inq.GetString(2).TrimOrBlank();
			//}
			//cmd_tpssm11_inq.Close();

			tpssm11["PONO_STATUS"] = cmd_tpssm12_inq.GetDecimal(1);
			tpssm11["SM_PLAN_NO"] = cmd_tpssm12_inq.GetString(2).TrimOrBlank();
			tpssm11["PONO"] = cmd_tpssm12_inq.GetString(3).TrimOrBlank();
			////Log::Trace("", __FUNCTION__, "pono_status=[{0}], sm_plan_no=[{1}]", tpssm11["PONO_STATUS"].ToDecimal(), tpssm11["SM_PLAN_NO"].ToString());


			//有计划号的, 即非临时计划号的, 不再计算计划号
			if (tpssm11["SM_PLAN_NO"].ToString()[0] != 'P')
			{
				continue;
			}

			//生成计划号
			for (;;) //新增记录的计划号计算
			{
				if (unused_flag == 1)//有未使用的计划顺序号
				{
					do
					{
						l_sm_plan_no = (l_sm_plan_no.ToInt32() + 1) % 10000000000;  //限制为10位整数
						tpssm27["SM_PLAN_NO"] = l_sm_plan_no.ToString();
						////Log::Info("", __FUNCTION__, "l_sm_plan_no = [{0}], tpssm27["SM_PLAN_NO"] =[{1}]", l_sm_plan_no, tpssm27["SM_PLAN_NO"].ToString());

						dummy = 0;
						//查找是否记录了未使用的计划顺序号

						switch (conn->DatabaseKind)
						{
						case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
						case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
						case DB_KIND_MSSQL:	        // MS SQL Server数据库
						case DB_KIND_ORACLE:        // Oracle 数据库
						default:
							sqlstr = " SELECT COUNT(1) FROM TPSSM27 "
								"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
								"    AND SM_PLAN_NO = @tpssm27.SM_PLAN_NO "
								"    AND USE_STATUS <> '1' ";
							break;
						}

						cmd_tpssm27_inq.SetCommandText(sqlstr);
						cmd_tpssm27_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString().TrimOrBlank());
						cmd_tpssm27_inq.Parameters.Set("tpssm27.SM_PLAN_NO", tpssm27["SM_PLAN_NO"].ToString().TrimOrBlank());
						dummy = cmd_tpssm27_inq.ExecuteScalar();
					} 
					while (dummy == 0 && (l_sm_plan_no <= l_sm_plan_no_used)); //没有找到,并且比最大的使用的计划号小

					if (l_sm_plan_no > l_sm_plan_no_used)
					{
						unused_flag = 0;
					}

				}
				else
				{
					l_sm_plan_no = (l_sm_plan_no.ToInt32() + 1) % 10000000000;  //限制为10位整数
					tpssm27["SM_PLAN_NO"] = l_sm_plan_no.ToString();
				}

				//查询新赋计划号是否在出钢计划中使用
				dummy = 0;
				sqlstr = CString(
					" SELECT COUNT(1) FROM TPSSM11  "
					" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"	AND SM_PLAN_NO = @tpssm27.SM_PLAN_NO "
						);
				cmd_tpssm11_inq.SetCommandText(sqlstr);
				cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString().TrimOrBlank());
				cmd_tpssm11_inq.Parameters.Set("tpssm27.SM_PLAN_NO", tpssm27["SM_PLAN_NO"].ToString() );
				dummy = cmd_tpssm11_inq.ExecuteScalar();

				if (dummy == 0) break;

			}//for
				
			////Log::Trace("", __FUNCTION__, "pono=[{0}], sm_plan_no=[{1}], new_plan_no=[{2}]", tpssm11["PONO"].ToString(), tpssm11["SM_PLAN_NO"].ToString(), tpssm27["SM_PLAN_NO"].ToString());


			//新计划号与原计划不同，则修改
			if (tpssm11["SM_PLAN_NO"].ToString() != tpssm27["SM_PLAN_NO"].ToString())
			{
				//方案二：原计划号改为临时计划号，以保证新计划号写入。  2015-10-06 增加
				plan_id++;
				CString tmp_plan_no = CString::Format("P%.5d", plan_id);
				sqlstr = CString(
					" UPDATE TPSSM11 "
					"    SET SM_PLAN_NO = @tmp_plan_no"
					"  WHERE SM_PLAN_NO = @sm_plan_no "
					);
				cmd_tpssm11_upd.SetCommandText(sqlstr);
				cmd_tpssm11_upd.Parameters.Set("tmp_plan_no", tmp_plan_no);
				cmd_tpssm11_upd.Parameters.Set("sm_plan_no", tpssm27["SM_PLAN_NO"].ToString());
				cmd_tpssm11_upd.ExecuteNonQuery();

				sqlstr = CString(
					" UPDATE TPSSM12 "
					"    SET SM_PLAN_NO = @tmp_plan_no"
					"  WHERE SM_PLAN_NO = @sm_plan_no "
					);
				cmd_tpssm12_upd.SetCommandText(sqlstr);
				cmd_tpssm12_upd.Parameters.Set("tmp_plan_no", tmp_plan_no);
				cmd_tpssm12_upd.Parameters.Set("sm_plan_no", tpssm27["SM_PLAN_NO"].ToString());
				cmd_tpssm12_upd.ExecuteNonQuery();


				//-----------------------------------------
				//新计划号写入，修改tpssm11/12
				sqlstr = " UPDATE TPSSM12 \
						SET SM_PLAN_NO = @tpssm27.SM_PLAN_NO \
						WHERE SM_PLAN_NO      = @tpssm11.SM_PLAN_NO ";

				cmd_tpssm12_upd.SetCommandText(sqlstr);
				cmd_tpssm12_upd.Parameters.Set("tpssm27.SM_PLAN_NO", tpssm27["SM_PLAN_NO"].ToString());
				cmd_tpssm12_upd.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
				cmd_tpssm12_upd.ExecuteNonQuery();

				tpssm11["SM_PLAN_NO"] = tpssm27["SM_PLAN_NO"];
				sqlstr = "tpssm11.Update(SM_PLAN_NO)";
				tpssm11.Update("SM_PLAN_NO", "FACTORY_DIV,PONO");
			}

		}//while
		cmd_tpssm12_inq.Close();

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
