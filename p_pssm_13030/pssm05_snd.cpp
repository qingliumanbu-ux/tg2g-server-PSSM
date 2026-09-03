/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2013-06-26
Description: 下发炼钢计划
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



#if defined _SYS_MES

#endif

#if defined _SYS_MMS
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif
int f_psbw1108_send(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//棒线DHCR计划

int f_pmouhp_send_pes(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

int f_mmsm_slab_dest(const CString& slab_plan_dest, CString& slab_dest_code, CDbConnection * conn);


/*<remark>=========================================================
/// <summary>
/// 炼钢计划下达
/// <para>
/// 1.查询LOT内的PONO, 将相应的PONO状态置计划下达
/// 2.调用炼钢连铸制造命令电文发送和调用DHCR电文发送
/// </para>
/// <para>数据库表：TPSSM01(炼钢制造命令表)         </para>
/// <para>主调用函数：前台PSSM01画面F3 计划编制调用 </para>
/// </summary>
/// <param name="pono">制造命令号    </param>
/// <returns>下发的炉次制造命令信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm05_snd)

//-EP_SYSTEM_HEAD_END
int f_pssm05_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int fetchRowCount = 0;
	int ret;
	CString strCast_lot_vol = "";
	CString temp_cast_lot_no = "";
	CString v_plan_status = "";
	CString v_plan_no = "";
	CString v_proc_div = "";

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息。
	CString sqlstr;
	CDecimal    cc_seq = 0;                         /* 连铸顺序号 */
	CDecimal    dummy = 0;
	int blkNum;

	CString   v_slab_dest_code = "";

	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");

#if defined _SYS_MES
	CModel tpssm10("TPSSM10");
#endif

	/* 数据库操作类定义 */
	CDbCommand cmd_upd(conn); //与DB 建立连接。
	CDbCommand cmd_sql(conn); //与DB 建立连接。
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpshpa1_inq(conn);
	CDbCommand cmd_tpshra1_inq(conn);

	//调用电文接口
	EIClass inBlock;
	EIClass inBlock1;
	EIClass outBlock;


	try
	{

		//调用棒线计划的接口参数声明
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");

		//调用厚板发送四大命令


		blkNum = bcls_rec->Tables.IndexOf("SEND");
		if (blkNum < 0)
		{
			bcls_rec->Tables.Add("SEND");
			bcls_rec->Tables["SEND"].Columns.Add(DT_STRING, "MAT_DESIGN");
			bcls_rec->Tables["SEND"].Columns.Add(DT_STRING, "PONO");

		}

		//向PES发送命令
		blkNum = bcls_rec->Tables.IndexOf("PONOSEND");
		if (blkNum < 0)
		{
			bcls_rec->Tables.Add("PONOSEND");
			bcls_rec->Tables["PONOSEND"].Columns.Add(DT_STRING, "MARKS1");
			bcls_rec->Tables["PONOSEND"].Columns.Add(DT_STRING, "FACTORY_DIV");
			bcls_rec->Tables["PONOSEND"].Columns.Add(DT_STRING, "PONO");
			bcls_rec->Tables["PONOSEND"].Columns.Add(DT_STRING, "CAST_LOT_NO");


		}

		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"].ToString();
		
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Info("", __FUNCTION__, "rowCount = [{0}]", rowCount);

		for (int i = 0; i < rowCount; i++)
		{
			strCast_lot_vol = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO"].ToString();

			////Log::Info("", __FUNCTION__, "strCast_lot_vol = [{0}]", strCast_lot_vol);
			////Log::Info("", __FUNCTION__, "FACTORY_DIV = [{0}]", tpssm01["FACTORY_DIV"].ToString());

			if (strCast_lot_vol == temp_cast_lot_no)//同CAST-LOT避免重复下达
			{
				continue;
			}

			////Log::Info("", __FUNCTION__, "strCast_lot_vol = [{0}]", strCast_lot_vol);

			tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm02["LOT_STATUS"] = 3;
			tpssm02.Update("LOT_STATUS", "CAST_LOT_NO, FACTORY_DIV");

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT * FROM TPSSM01 "
					" WHERE  FACTORY_DIV = @tpssm01.FACTORY_DIV "
					" AND    CAST_LOT_NO = @tpssm01.CAST_LOT_NO "
					" AND    PONO_STATUS = 13 "
					" ORDER  BY PLAN_DATE,CC_SEQ   ");
				break;
			}

			cmd_sql.SetCommandText(sqlstr);// 设置执行的SQL语句
			cmd_sql.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_sql.Parameters.Set("tpssm01.CAST_LOT_NO", strCast_lot_vol);
			cmd_sql.ExecuteReader();

			while (cmd_sql.Read())
			{
				cmd_sql.Fetch(tpssm01);

				fetchRowCount++;

				////Log::Trace("", __FUNCTION__, "★★★★★查询板坯去向代码★★★★★");
				f_mmsm_slab_dest(tpssm01["SLAB_DEST"].ToString(), v_slab_dest_code, conn);

				////Log::Info("", __FUNCTION__, "tpssm01["CAST_LOT_NO"] = [{0}], tpssm01["PONO"] = [{1}], tpssm01.HOT_CHARGE_FLAG[{2}]", tpssm01["CAST_LOT_NO"].ToString(), tpssm01["PONO"].ToString(), tpssm01["HOT_CHARGE_FLAG"].ToString());
				if ((fetchRowCount == 1)
					&& ((tpssm01["HOT_CHARGE_FLAG"].ToString() == "1") || (tpssm01["HOT_CHARGE_FLAG"].ToString() == "2"))
					&& (v_slab_dest_code.Trim() == "HP"))
				{
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:
						sqlstr = CString(" SELECT PLAN_STATUS, PLAN_NO FROM TPSHPA1 "
							" WHERE  CAST_LOT_NO = @tpssm01.CAST_LOT_NO ");
						break;
					}
					cmd_tpshpa1_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
					cmd_tpshpa1_inq.Parameters.Set("tpssm01.CAST_LOT_NO", strCast_lot_vol);
					cmd_tpshpa1_inq.ExecuteReader();

					if (cmd_tpshpa1_inq.Read())
					{
						v_plan_status = cmd_tpshpa1_inq.GetString(1);
						v_plan_no = cmd_tpshpa1_inq.GetString(2);
					}
					cmd_tpshpa1_inq.Close();

					if ((v_plan_status < "04") && (tpssm01["HOT_CHARGE_FLAG"].ToString() == "2"))
					{
						CFormattable arguments[] = { strCast_lot_vol, v_plan_no }; // 定义参数列表的数组
						CMessageFormat::Format(s.msg, "LOT[{0}]是对应的DHCR轧制计划[{1}]还未确定。请联系轧制计划员", arguments, 2); //格式化字符串
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}

				//更新修改者，修改时间
				tpssm01["REC_REVISOR"] = CString(s.userid);
				tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

#ifdef _SYS_MMS 
				tpssm01["PONO_STATUS"] = 14;
#endif

#ifdef _SYS_MES
				tpssm01["PONO_STATUS"] = 15;
#endif

				v_update = "REC_REVISOR,REC_REVISE_TIME,PONO_STATUS";//修改字段信息。
				v_condi = "FACTORY_DIV,PONO"; //查询条件

				if (tpssm01.Update(v_update, v_condi) != true)
				{
					strcpy(s.msg, "Update failed.");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//调用计划下发电文
				bcls_rec->Tables["PONOSEND"].Rows.Clear();

				if (bcls_rec->Tables["PONOSEND"].Rows.get_Count() <= 0)
					bcls_rec->Tables["PONOSEND"].Rows.Add();
				bcls_rec->Tables["PONOSEND"].Rows[0]["MARKS1"] = 1;
				bcls_rec->Tables["PONOSEND"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
				bcls_rec->Tables["PONOSEND"].Rows[0]["PONO"] = tpssm01["PONO"];
				bcls_rec->Tables["PONOSEND"].Rows[0]["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];

				////Log::Trace("", __FUNCTION__, "★★★★★发送制造命令信息start★★★★★");
#ifdef _SYS_MMS 
				ret = f_cm_002021_snd(bcls_rec, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
#endif
				////Log::Trace("", __FUNCTION__, "★★★★★发送制造命令信息end★★★★★");

#ifdef _SYS_MES
				//使用MES时，执行以下内容
				//新增10表
				////Log::Info("", __FUNCTION__, "FACTORY_DIV = [{0}]", tpssm01["FACTORY_DIV"].ToString());
				////Log::Info("", __FUNCTION__, "CC_MACH_NO = [{0}]", tpssm01["CC_MACH_NO"].ToString());
				//---------------------1---------------
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					//SQL 语句标准化修改:去掉NVL。 xuwen 2013-4-10
					sqlstr = " SELECT NVL(MAX(CC_SEQ), 0) FROM TPSSM10 "
						"  WHERE FACTORY_DIV	= @tpssm01.FACTORY_DIV "
						"    AND CC_MACH_NO		= @tpssm01.CC_MACH_NO "
						"    AND PONO_STATUS    >= 16 "
						"    AND CC_SEQ         < 900 ";
					break;
				}

				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm10_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
				//cc_seq = cmd_tpssm10_inq.ExecuteScalar();
				cmd_tpssm10_inq.ExecuteReader();
				if (cmd_tpssm10_inq.Read())
				{
					cc_seq = cmd_tpssm10_inq.GetDecimal(1);
				}
				else
				{
					cc_seq = 0;
				}
				cmd_tpssm10_inq.Close();
				////Log::Trace("", __FUNCTION__, "当前出钢计划中最大的浇注顺序号cc_seq = [{0}]", cc_seq.ToInt32());


				////Log::Trace("", __FUNCTION__, "★★★★★查询一览调整表中是否存在该计划★★★★★");
				//查询一览调整表中是否存在该计划
				dummy = 0;
				// EXEC SQL SELECT COUNT(*) INTO :dummy
				/*FROM tpssm10
				WHERE FACTORY_DIV = trim(:tpssm01["FACTORY_DIV"].ToString())
				AND PONO				= trim(:tpssm01["PONO"].ToString())
				and pono_status<=16;*/
				sqlstr = " SELECT COUNT(*) FROM TPSSM10 "
					"   WHERE FACTORY_DIV	= @tpssm01.FACTORY_DIV "
					"    AND PONO			= @tpssm01.PONO "
					"    AND PONO_STATUS	<= 16";
				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
				cmd_tpssm10_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
				dummy = cmd_tpssm10_inq.ExecuteScalar();
				////Log::Trace("", __FUNCTION__, "dummy = [{0}]", dummy);
				if (dummy > 0)//存在的必然是再排计划
				{
					//如果以前有，那么删除以前的，重新接受
					// EXEC SQL DELETE FROM tpssm10	WHERE PONO = :tpssm01["PONO"];
					tpssm10["PONO"] = tpssm01["PONO"];
					tpssm10.Delete("PONO");
				}

				//新增计划
				tpssm10["REC_CREATE_TIME"] = tpssm01["REC_CREATE_TIME"];  /* 创建时间*/
				tpssm10["REC_CREATOR"] = tpssm01["REC_CREATOR"];  /* 创建人*/
				tpssm10["REC_REVISE_TIME"] = tpssm01["REC_CREATE_TIME"];  /* 创建时间*/
				tpssm10["REC_REVISOR"] = tpssm01["REC_CREATOR"];  /* 创建人*/
				tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];  /* 主工序代码 -PK*/
				tpssm10["PONO"] = tpssm01["PONO"];    /* 制造命令号 -PK*/
				tpssm10["PONO_STATUS"] = 16;                    /* 制造命令状态 */
				////Log::Trace("", __FUNCTION__, "当前出钢计划中最大的浇注顺序号cc_seq = [{0}]", cc_seq.ToInt32());
				if (cc_seq == 0)
				{
					cc_seq = 1;
				}
				else
				{
					cc_seq = cc_seq + 1;
				}
				//由于使用了MMS上的CCSEQ，因此10表不用再计算CCSEQ了 HYF 0228
				//tpssm10["CC_SEQ"]		= cc_seq;                /* 连铸顺序号 */
				tpssm10["CC_SEQ"] = tpssm01["CC_SEQ"];                /* 连铸顺序号 */
				tpssm10["ST_NO"] = tpssm01["ST_NO"];         /* 出钢记号 */
				tpssm10["SG_SIGN"] = tpssm01["SG_SIGN"];       /* 牌号（钢级） */
				tpssm10["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];   /* CAST_LOT号 */
				tpssm10["CAST_LOT_DIV_NO"] = tpssm01["CAST_LOT_DIV_NO"];          /* CAST_LOT分割号 */
				tpssm10["CAST_LOT_SUM"] = tpssm01["CAST_LOT_SUM"];             /* CAST_LOT内炉数 */
				tpssm10["CC_MACH_NO"] = tpssm01["CC_MACH_NO"];    /* 连铸机号 */
				tpssm10["RESTRAND_FLG"] = tpssm01["RESTRAND_FLG"];             /* 重引锭标志 */
				tpssm10["BACKLOG_EA"] = tpssm01["BACKLOG_EA"];    /* 钢区工序途径 */
				tpssm10["REFINE_DIV"] = tpssm01["REFINE_DIV"]; /* 精炼路径代码 */
				tpssm10["SMELT_MODE"] = tpssm01["SMELT_MODE"];               /* 冶炼模式 */
				tpssm10["PLAN_TAP_WT"] = tpssm01["PLAN_TAP_WT"];              /* 计划出钢重量 */
				tpssm10["PLAN_DATE"] = tpssm01["PLAN_DATE"];     /* 计划日期 */
				tpssm10["CC_REQ_TIME"] = tpssm01["CC_REQ_TIME"];   /* CC要求时刻设定 */
				tpssm10["HOT_SEND_FLAG"] = tpssm01["HOT_SEND_FLAG"]; /* 热送标记 */
				tpssm10["HOT_CHARGE_FLAG"] = tpssm01["HOT_CHARGE_FLAG"];       /* 热装标记 */
				tpssm10["TD_CHG_FLG"] = 0;                     /* 中间包更换标志 */
				tpssm10["SHIFT_NO"] = tpssm01["SHIFT_NO"];      /* 班次号 */
				// EXEC SQL INSERT INTO tpssm10 VALUES (:tpssm10); /*炼钢连铸计划在线调整表*/
				tpssm10.TrimOrBlank();
				tpssm10.Insert();

#endif
				////Log::Trace("", __FUNCTION__, "★★★★★查询板坯去向代码★★★★★");
				f_mmsm_slab_dest(tpssm01["SLAB_DEST"].ToString(), v_slab_dest_code, conn);

#ifdef _LINE_BW

				//2030515 HYF 判断去向来调用这个函数，不然会报错
				if (v_slab_dest_code.Trim() == "BW")
					//if(tpssm01.MAT_DESTION.Trim()=="13")
				{
					//调用棒线DHCR计划函数
					inBlock1.Tables[0].Rows.Clear();
					inBlock1.Tables[0].Rows.Add();
					inBlock1.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
					inBlock1.Tables[0].Rows[0]["PONO"] = tpssm01["PONO"];
					inBlock1.Tables[0].Rows[0]["CC_MACH_NO"] = tpssm01["CC_MACH_NO"];

					//ret = f_psbw1108_send(&inBlock1, &outBlock, conn);
					ret = 0;
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
#endif

				//					//发送四大命令
				//			#ifdef _LINE_HP
				//			
				//			////Log::Trace("", __FUNCTION__, "★★★★★发送四大命令start★★★★★");
				//		/*	if(tpssm01.MAT_DESTION.Trim()=="10")*/
				//		 if(v_slab_dest_code.Trim()=="HP")
				//			{
				//				bcls_rec->Tables["SEND"].Rows.Clear();
				//
				//				if(bcls_rec->Tables["SEND"].Rows.get_Count() <= 0)
				//					   bcls_rec->Tables["SEND"].Rows.Add();
				//					bcls_rec->Tables["SEND"].Rows[0]["MAT_DESIGN"] = "C";
				//					bcls_rec->Tables["SEND"].Rows[0]["PONO"]   = tpssm01["PONO"]; 
				//
				//				doFlag = f_pmouhp_send_pes(bcls_rec, bcls_ret,conn);
				//
				//				if(ret < 0)
				//				{
				//					throw CApplicationException(-1, s.msg, log.Location); 
				//				}
				//			}
				//			
				//			////Log::Trace("", __FUNCTION__, "★★★★★发送四大命令end★★★★★");
				//			#endif
				//
				//
			}//while
			cmd_sql.Close();

			temp_cast_lot_no = strCast_lot_vol;
		}


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;
	}

	return doFlag;
}
