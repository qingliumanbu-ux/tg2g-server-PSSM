/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-02-13
功能: 炼钢计划日制造命令信息_LOT删除
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"



#if defined _SYS_MES

#endif

/******定义调用的函数******/

/*<remark >========================================================= 
/// <summary > 
/// 炼钢计划日制造命令信息_LOT删除
/// <para > 
/// 1.根据传入的LOT号删除当前记录；
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表)     </para > 
/// <para > 数据库表：TPSSM02(制造命令LOT表)      </para > 
/// <para > 数据库表：TPSSM03(制造命令板坯表)     </para > 
/// <para > 数据库表：TPSSM04(制造命令铸机表)     </para > 
/// <para > 主调用函数：前台PSSM04画面F4(LOT删除)调用。   </para > 
/// <para > 调用厚板材料申请函数，进行材料申请状态信息修改(调用函数f_pmom_pssm_upd_hpstatus)。</para > 
/// <para > 调用薄板材料申请函数，进行材料申请删除(调用函数f_pmom_pssm_upd_hpstatus)。 </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
BM2F_ENTERACE(pssm04_ldel)

int f_pmom_pssm_del_pono(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //PONO删除调用材料申请功能。2014-1-28 xuwen 替换原有f_pmomhr_pssm_del_pono、f_pmomsm_pssm_del_pono、f_pmombw_pssm_del_pono函数
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪
#if defined _SYS_MMS
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

int f_pssm04_ldel(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值
	int ret = 0;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*业务变量*/
	CString strPono_vol = "";
	CString strCast_lot_no = "";
	CString strPlanner_id = "";
	CString strSys_id = "";
	CString strSlab_dest = "";
	CString strMain_backlog_code = "";
	CString temp_cast_lot_no = "";

	CString sche_flag = "";
	CDecimal strPono_status ;
	CDecimal j = 0 ;
	CDecimal cc_seq = 0;

	EIClass inBlock;//调用生产接口
	EIClass inBlock1; //调用电文接口
	EIClass inBlock3; //调用炼钢履历跟踪
	EIClass outBlock;

	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
#if defined _SYS_MES
	CModel tpssm10("TPSSM10");
#endif

	CDbCommand cmd_del(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);

	try
	{	
		bcls_ret->Tables[0].Rows.Clear();
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"I_CODE");  //‘1’
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"PONO");  //制造命令号

		//声明调用生产的接口参数
		inBlock.Tables[0].set_TableName("DELETEPONO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");

		//声明调用电文的接口参数
		inBlock1.Tables[0].set_TableName("PONOSEND");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "MARKS1");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");

		//调用炼钢履历跟踪
		inBlock3.Tables[0].set_TableName("TRACE");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		//循环处理
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm01.Reset();
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			
			/*获取前台输入数据*/ 
			strPono_vol = bcls_rec->Tables[0].Rows[i]["PONO"];
			////Log::Debug("", __FUNCTION__ ,"IN:pono = [{0}]",strPono_vol);
			if (strPono_vol == " ")
			{
				strcpy(s.msg,"传入制造命令号不可为空");
				throw CApplicationException(-1,s.msg,s.svc_name);
			}

			strCast_lot_no = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO"];
			////Log::Debug("", __FUNCTION__ ,"cast_lot_no = [{0}]",strCast_lot_no);

			if (strCast_lot_no == temp_cast_lot_no)//同CAST-LOT避免重复删除
			{
				continue;
			}

			if  (strCast_lot_no != "")
			{
				sqlstr = " SELECT "
					     " PONO_STATUS,"
						 " SLAB_DEST,"
						 " SCHE_FLAG,"
						 " FACTORY_DIV, "
						 " PONO, "
						 " CAST_LOT_NO "
					     " FROM TPSSM01 "
						 " WHERE  CAST_LOT_NO = @strCast_lot_no";

				CDbCommand cmd_inq(sqlstr,conn);

				cmd_inq.Parameters.Set("strCast_lot_no", strCast_lot_no);

				cmd_inq.ExecuteReader();

				while (cmd_inq.Read())
				{
					tpssm01["PONO_STATUS"] = cmd_inq.GetInt32(1);
					sche_flag = cmd_inq.GetString(3);
					tpssm01["FACTORY_DIV"] = cmd_inq.GetString(4);
					tpssm01["PONO"] = cmd_inq.GetString(5);
					tpssm01["CAST_LOT_NO"] = cmd_inq.GetString(6);
					////Log::Debug("", __FUNCTION__, "tpssm01["PONO_STATUS"] = [{0}]", tpssm01["PONO_STATUS"].ToDecimal());
					////Log::Debug("", __FUNCTION__ ,"sche_flag = [{0}]",sche_flag);

					if (tpssm01["PONO_STATUS"].ToDecimal() >= 18)
					{
						CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "制造命令号[{0}]已排入出钢计划。", arguments, 1);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					if (sche_flag == '*')
					{
						sprintf(s.msg,"制造命令有再排标记，无法进行删除。");
						throw CApplicationException(-1,s.msg,log.Location);
					}

					///*检查是否有轧制计划*/
					//if ((strMain_backlog_code == "A1" || strMain_backlog_code == "A2" )
					//	&& (strSlab_dest != "06" ))
					//{
					//	sqlstr = " SELECT "
					//			 " VALUE(COUNT(*) ,0)  "
					//			 " FROM   TPMOM00  "
					//			 " WHERE  CAST_LOT_NO = @cast_lot_no "
					//			 " AND    APP_STATUS LIKE  '_4%' ";
					//	CDbCommand cmd_inq_om(sqlstr,conn);
					//	cmd_inq_om.Parameters.Set("cast_lot_no" ,strCast_lot_no);
					//	int count_p = cmd_inq_om.ExecuteScalar().ToInt32();
					//	if  (count_p > 0 )
					//	{
					//		strcpy(s.msg,"请先删除轧制计划,LOT号[" + strCast_lot_no + "]");
					//		throw CApplicationException(-1,s.msg,log.Location);
					//	}
					//	cmd_inq_om.Close();
					//}

					//调用生产函数
					inBlock.Tables[0].Rows.Clear();
					inBlock.Tables[0].Rows.Add();
					inBlock.Tables[0].Rows[0]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
					inBlock.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];

					ret = f_pmom_pssm_del_pono(&inBlock, &outBlock, conn);

					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//删除TPSSM01
					tpssm01.Delete("FACTORY_DIV, PONO");

					//删除TPSSM03
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:

						sqlstr = CString(" DELETE FROM TPSSM03 "
							" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
							" AND    PONO = @tpssm01.PONO ");
						break;
					}

					cmd_del.SetCommandText(sqlstr);
					cmd_del.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());   //设置条件数据项
					cmd_del.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());   //设置条件数据项
					cmd_del.ExecuteNonQuery();	              //执行删除
					cmd_del.Close();

#ifdef _SYS_MES
					//读取要删除PONO信息
					tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					tpssm10["PONO"] = tpssm01["PONO"];
					sqlstr = "tpssm10.Query()";
					bool has10 = tpssm10.Query();

					if (has10 == true) //有记录
					{
						//2）"T"标记传递给下一炉
						if (tpssm10["RESTRAND_FLG"].ToString().Trim() != "")
						{

							////Log::Trace("", __FUNCTION__, "cc_mach_no =[{0}]", tpssm10["CC_MACH_NO"].ToString());
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


						//删除PONO
						sqlstr = "tpssm10.Delete()";
						tpssm10.Delete();

						////Log::Trace("", __FUNCTION__, "删除tpssm10");


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
					}
#endif

					//调用计划下发电文
					inBlock1.Tables[0].Rows.Clear();
					inBlock1.Tables[0].Rows.Add();
					inBlock1.Tables[0].Rows[0]["MARKS1"] = 3;
					inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
					//增加主工序代码 HYF 20130401
					inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];

#ifdef _SYS_MMS 
					ret = f_cm_002021_snd(bcls_rec, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
#endif

					//-----------------------------------------------
					//炼钢履历跟踪
					CDataRow &row3 = inBlock3.Tables["TRACE"].Rows.Add();
					row3["EVENT_ID"] = "C1"; //PONO删除
					row3["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					row3["PONO"] = tpssm01["PONO"];

					ret = f_pssm99_trace(&inBlock3, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				cmd_inq.Close();

				tpssm02["CAST_LOT_NO"] = temp_cast_lot_no;
				//删除TPSSM02
				tpssm02.Delete("FACTORY_DIV, CAST_LOT_NO");

				temp_cast_lot_no = strCast_lot_no;
			}
		}
		if  (j == 0 ) 
		{
			strcpy(s.msg,("删除LOT号不成功"));	
		}
		else
		{
			strcpy(s.msg,_RES("GCRSS0000002")/*处理成功。*/);
		}
	}

	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
