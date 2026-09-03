/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-5
Version:1.0
Description:  模拟运转的事项信号查询 
Update: 2015-04-07 lijie 更新数据表
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



int f_pssm51f2_inq_sign(EIClass *bcls_rec,EIClass *bcls_ret, CDbConnection * conn);

/*<remark>=========================================================
/// <summary>
/// 模拟运转的事项信号查询
/// <para>根据选择的PONO, 查询对应各工序的运转信号           </para>
/// <para>数据库表：tpssms1(炼钢事项信号定义表)              </para>
/// <para>主调用函数：前台PSSM51画面F2(查询)调用。           </para>
/// </summary>
/// <param name="factory_div">炼钢主工序代码    </param>
/// <param name="pono">制造命令          </param>
/// <returns>事项信号定义信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm51f2_inq_sign)

//-EP_SYSTEM_HEAD_END
int f_pssm51f2_inq_sign(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int    area_id_min=0;                        /* 最小炼钢区域标识 */
	int    area_id_max=0;                        /* 最大炼钢区域标识 */
	CDecimal    bof_charge_no = 0;                  /* 转炉CHARGE编号:  为生成精炼重数用 */

	CModel tpssms1("TPSSMS1");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CDbCommand cmd_tpssms1_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{

		bcls_ret->Tables[0].set_TableName("TSIGNAL");
		bcls_ret->Tables[0].Columns.Add(tpssms1);
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "AREA_ID");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "SRP_SEQ"); //精炼重数
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO"); 
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "HEAT_NO");

		//获得输入参数
		tpssms1["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm11["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString();
		//tpssm12["AREA_ID"] = bcls_rec->Tables[0].Rows[0]["AREA_ID"].ToDecimal();
		//tpssm12["DEV_CODE"] = bcls_rec->Tables[0].Rows[0]["DEV_CODE"].ToString();
		
		Log::Info("", __FUNCTION__, "FACTORY_DIV  =[{0}]", tpssms1["FACTORY_DIV"].ToString());
		Log::Info("", __FUNCTION__, "PONO  =[{0}]", tpssm11["PONO"].ToString());


		if (tpssm12["AREA_ID"].ToDecimal() == 0)
		{
			area_id_min = 0;
			area_id_max = 10;
		}
		else
		{
			area_id_min = tpssm12["AREA_ID"].ToDecimal().ToInt32();
			area_id_max = tpssm12["AREA_ID"].ToDecimal().ToInt32();
		}

		//查询下达计划的子工序
		
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default:
			sqlstr = " SELECT * FROM "
					 " ( "
					 " SELECT DISTINCT T2.DEV_CODE, T2.AREA_ID, T2.CHARGE_NO,T2.SM_PLAN_NO FROM TPSSM12 T2 ,TPSSM11 T1 "
					 " WHERE T2.FACTORY_DIV = @tpssms1.FACTORY_DIV "
					 "   AND T2.SM_PLAN_NO = T1.SM_PLAN_NO "
					 "   AND T1.PONO       = @tpssm11.PONO ";
			if(tpssm12["DEV_CODE"].ToString().Trim() != "" ) sqlstr += " AND DEV_CODE = @tpssm12.DEV_CODE ";

			sqlstr += "   AND AREA_ID BETWEEN @area_id_min AND @area_id_max "
					 " ) "
					 " ORDER BY CHARGE_NO ASC ";
			break;
		}

		cmd_tpssm12_inq.SetCommandText(sqlstr);
		cmd_tpssm12_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm11.PONO",tpssm11["PONO"].ToString());
		cmd_tpssm12_inq.Parameters.Set("tpssm12.DEV_CODE",tpssm12["DEV_CODE"].ToString());
		cmd_tpssm12_inq.Parameters.Set("area_id_min",area_id_min);
		cmd_tpssm12_inq.Parameters.Set("area_id_max",area_id_max);
		cmd_tpssm12_inq.ExecuteReader();
		while(cmd_tpssm12_inq.Read())
		{
			tpssm12["DEV_CODE"] = cmd_tpssm12_inq.GetString(1);
			tpssm12["AREA_ID"] = cmd_tpssm12_inq .GetDecimal(2);
			tpssm12["CHARGE_NO"] = cmd_tpssm12_inq.GetDecimal(3);
			tpssm12["SM_PLAN_NO"] = cmd_tpssm12_inq.GetString(4);

			Log::Info("", __FUNCTION__, "tpssm12.DEV_CODE  =[{0}]",tpssm12["DEV_CODE"].ToString());


			if(tpssm12["AREA_ID"].ToDecimal() == 2) //脱磷工序
			{				
				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT * FROM TPSSMS1 "
							 " WHERE FACTORY_DIV	= @tpssms1.FACTORY_DIV "
							 "   AND DEV_CODE		= @tpssm12.DEV_CODE "
							 "   AND SUB_PROC_SEQ	> 0 "
							 "   AND RUN_SIGNAL BETWEEN '200' AND '300' "
							 " ORDER BY RUN_SIGNAL ASC ";
					break;
				}

				cmd_tpssms1_inq.SetCommandText(sqlstr);
				cmd_tpssms1_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
				cmd_tpssms1_inq.Parameters.Set("tpssm12.DEV_CODE",tpssm12["DEV_CODE"].ToString());
				cmd_tpssms1_inq.ExecuteReader();
				while(cmd_tpssms1_inq.Read())
				{
					cmd_tpssms1_inq.Fetch(tpssms1);
					CDataRow &row = bcls_ret->Tables[0].Rows.Add();
					row.Merge(tpssms1);
					row["AREA_ID"] =  tpssm12["AREA_ID"];
					row["PONO"] = tpssm11["PONO"];
				}
				cmd_tpssms1_inq.Close();
				
			}

			else if(tpssm12["AREA_ID"].ToDecimal() == 3) //转炉工序
			{				
				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT * FROM TPSSMS1 "
							 " WHERE FACTORY_DIV	= @tpssms1.FACTORY_DIV "
							 "   AND DEV_CODE		= @tpssm12.DEV_CODE "
							 "   AND SUB_PROC_SEQ	> 0 "
							 "   AND RUN_SIGNAL BETWEEN '300' AND '400' "
							 " ORDER BY RUN_SIGNAL ASC ";
					break;
				}
				cmd_tpssms1_inq.SetCommandText(sqlstr);
				cmd_tpssms1_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
				cmd_tpssms1_inq.Parameters.Set("tpssm12.DEV_CODE",tpssm12["DEV_CODE"].ToString());
				cmd_tpssms1_inq.ExecuteReader();
				while(cmd_tpssms1_inq.Read())
				{
					cmd_tpssms1_inq.Fetch(tpssms1);
					CDataRow &row = bcls_ret->Tables[0].Rows.Add();
					row.Merge(tpssms1);
					row["AREA_ID"] =  tpssm12["AREA_ID"];
					row["PONO"] = tpssm11["PONO"];
				}
				cmd_tpssms1_inq.Close();
				bof_charge_no = tpssm12["CHARGE_NO"];
			}

			else
			{				
		
				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:


					sqlstr = " SELECT * FROM TPSSMS1 "
							 " WHERE FACTORY_DIV	= @tpssms1.FACTORY_DIV "
							 "   AND DEV_CODE		= @tpssm12.DEV_CODE "
							 "   AND SUB_PROC_SEQ	> 0 "
							 " ORDER BY RUN_SIGNAL ASC ";
					break;
				}

				cmd_tpssms1_inq.SetCommandText(sqlstr);
				cmd_tpssms1_inq.Parameters.Set("tpssms1.FACTORY_DIV", tpssms1["FACTORY_DIV"].ToString());
				cmd_tpssms1_inq.Parameters.Set("tpssm12.DEV_CODE"         , tpssm12["DEV_CODE"].ToString());
				cmd_tpssms1_inq.ExecuteReader();
				while(cmd_tpssms1_inq.Read())
				{
					cmd_tpssms1_inq.Fetch(tpssms1);
					CDataRow &row = bcls_ret->Tables[0].Rows.Add();
					row.Merge(tpssms1);
					row["AREA_ID"] =  tpssm12["AREA_ID"];
					row["PONO"] = tpssm11["PONO"];
					if (tpssm12["AREA_ID"].ToDecimal() == 4) //精炼工序
					{					
						//ADD BY xiangping 
						////Log::Info("", __FUNCTION__, "bof_charge_no111  =[{0}]",bof_charge_no );
							////Log::Info("", __FUNCTION__, "tpssms1["FACTORY_DIV"] =[{0}]",tpssms1["FACTORY_DIV"].ToString());
									////Log::Info("", __FUNCTION__, "tpssm12.SM_PLAN_NO  =[{0}]",tpssm12.SM_PLAN_NO );
						if (bof_charge_no == 0)
						{

							switch(conn->DatabaseKind)
							{
							case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
							case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
							case DB_KIND_MSSQL:         // MS SQL Server数据库
							case DB_KIND_ORACLE:        // Oracle 数据库
							default:
							
								sqlstr = " SELECT CHARGE_NO FROM TPSSM12 "
												" WHERE FACTORY_DIV = @tpssms1.FACTORY_DIV "
												"   AND SM_PLAN_NO = @tpssm12.SM_PLAN_NO "
												"   AND AREA_ID    = 3";
								break;
							}

							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.Parameters.Set("tpssms1.FACTORY_DIV",tpssms1["FACTORY_DIV"].ToString());
							cmd_inq.Parameters.Set("tpssm12.SM_PLAN_NO",tpssm12["SM_PLAN_NO"].ToString());
							cmd_inq.ExecuteReader();

							if(cmd_inq.Read())
							{
								bof_charge_no = cmd_inq.GetDecimal(1);
							}
							cmd_inq.Close();
						

						}
						////Log::Info("", __FUNCTION__, "bof_charge_no  =[{0}]",bof_charge_no );

						row["SRP_SEQ"] =tpssm12["CHARGE_NO"].ToDecimal() - bof_charge_no;
					}
				}
				cmd_tpssms1_inq.Close();		
			}
		}
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
	cmd_tpssms1_inq.Close();
	cmd_tpssm12_inq.Close();

	return doFlag;
}
