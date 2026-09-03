/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Date:     2015-2-10
Version:  3.1.0
Description: PES系统接受对MMS系统下发的制造命令进行接受
Update：
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"






#ifdef _SYS_PES
int f_cm_200007_snd(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn); //接收应答
#endif

int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //写炼钢调整履历表

#ifdef  _LINE_BW
//int f_psbw_roll_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //删除棒线轧制计划
#endif

#ifdef  _LINE_HP
int f_pshp_dhcr_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

/*<remark>=========================================================
/// <summary>
/// 获取材料合同信息
/// <para>
///    功能叙述段落
///  注：本函数只在MMS层用，输入/输出参数都以bcls_rec下的[MMSMSM]块传递
/// </para>
/// </summary>
/// <param name="param1">参数1  </param>
/// <param name="param2">参数2  </param>
/// <returns>返回参数：调用是否成功</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE_TELE(cm_002021_rcv)

int f_cm_002021_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int    doFlag = 0;
	int    fetchRowCount = 0;
	int ret = 0;
	int rownum = 0;

	CString   datetime = " ";
	int    blkseq = 0;
	EIClass inBlock1;  //调用发送反馈应答的函数
	EIClass inBlock2;  //调用删除棒线轧制计划函数用 f_psbw_roll_del()
	CDecimal    cc_seq = 0;                         /* 连铸顺序号 */
	CDecimal    dummy = 0;
	CDecimal    v_proc_div = 0;   //操作类别：1-新增；3-PONO删除
	CString   v_cast_lot_no = "";
	CString   v_handle_div = "";
	CString   v_table_name = "";
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CModel tpssm99("TPSSM99");//履历
	CDbCommand cmd_tpssm01_inq1(conn);
	CDbCommand cmd_tpssm03_sql(conn);
	CDbCommand cmd_tpssm03_upd(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CString sqlstr;

	EIClass in_pssm99trace;//调用履历函数

	try
	{

		//计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);
		//取系统日期、时间
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		tpssm01["REC_CREATE_TIME"] = datetime;

		//责任者
		tpssm01["REC_CREATOR"] = "XCOM";
		tpssm02["REC_CREATOR"] = "XCOM";
		tpssm02["REC_CREATE_TIME"] = tpssm01["REC_CREATE_TIME"];

		Log::Trace("", __FUNCTION__, "s.userid={0}", s.userid);
		Log::Trace("", __FUNCTION__, "s.username={0}", s.username);
		Log::Trace("", __FUNCTION__, "s.formname={0}", s.formname);


		//1)设置调用应答函数的输入参数:
		blkseq = 1;  //第一块
		inBlock1.Tables[blkseq - 1].set_TableName("X200007");
		inBlock1.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock1.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[blkseq - 1].Columns.Add(DT_STRING, "ACK_CODE");
		inBlock1.Tables[blkseq - 1].Rows.Add();  //单记录应答

		inBlock1.AddBlock();//发送给铁区的
		inBlock1.Tables[1].set_TableName("TQ");
		inBlock1.Tables[1].Columns.Add(DT_STRING, "OPER_FLAG");
		inBlock1.Tables[1].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock1.Tables[1].Columns.Add(DT_STRING, "PONO");
		inBlock1.Tables[1].Columns.Add(DT_DECIMAL, "PLAN_TAP_WT");
		inBlock1.Tables[1].Columns.Add(DT_STRING, "PLAN_DATE");
		inBlock1.Tables[1].Rows.Add();

		v_table_name = "PSHP"; //调用函数 ==f_pshp_dhcr_del  使用。
		if (!bcls_rec->Tables.Contains(v_table_name))
		{
			bcls_rec->Tables.Add(v_table_name);          //PSHP-接收的TABLE。
		}
		if (!bcls_rec->Tables[v_table_name].Columns.Contains("PREC_SLAB_NO"))
		{
			bcls_rec->Tables[v_table_name].Columns.Add(DT_STRING, "PREC_SLAB_NO"); //预定板坯号。
		}

		if (!bcls_rec->Tables[v_table_name].Columns.Contains("EVENT_ID"))
		{
			bcls_rec->Tables[v_table_name].Columns.Add(DT_STRING, "EVENT_ID"); //事件号 = PM
		}

		if (!bcls_rec->Tables[v_table_name].Columns.Contains("FUNC_ID"))
		{
			bcls_rec->Tables[v_table_name].Columns.Add(DT_STRING, "FUNC_ID"); //当前函数。
		}

		//2)设置删除棒线轧制计划函数f_psbw_roll_del()的输入参数:
		blkseq = 1;  //第一块
		inBlock2.Tables[blkseq - 1].set_TableName("PSBW");
		inBlock2.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");

		//--------------------------------------------------------
		//获取电文内容，单记录（每PONO）
		tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		tpssm01.Print();
		tpssm02.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		v_proc_div = bcls_rec->Tables[0].Rows[0]["PROC_DIV"];
		Log::Trace("", __FUNCTION__, "proc_div=[{0}]", v_proc_div.ToInt32());


		if (v_proc_div == 1)  //1-新增
		{
			//------------------------------
			//炉次命令赋值

			v_handle_div = "I";//给铁区的操作区分标志
			tpssm01.TrimOrBlank();

			if (tpssm01.QueryCount("FACTORY_DIV,PONO") > 0)
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() };
				CMessageFormat::Format(s.msg, _RES("该PONO已经存在")/*该PONO已经存在。*/, arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			sqlstr = "tpssm01.Insert()";
			tpssm01.Insert();

			//------------------------------
			//LOT信息赋值
			tpssm02["LOT_STATUS"] = 3;//命令下达

			//新增LOT前先做删除。防止碎片LOT????(LOT收回产生)
			sqlstr = "tpssm02.Delete()";
			tpssm02.Delete("FACTORY_DIV,CAST_LOT_NO");

			tpssm02.TrimOrBlank();
			sqlstr = "tpssm02.Insert()";
			tpssm02.Insert();


			//------------------------------
			//写入连铸计划表（TPSSM10）
			//取当前出钢计划中最大的浇注顺序号和PONO
			cc_seq = 0;
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句。SQL 语句标准化修改:去掉NVL。 xuwen 2013-4-10
				sqlstr = CString(
					" SELECT MAX(cc_seq) FROM TPSSM10 "
					"  WHERE FACTORY_DIV = @tpssm01.FACTORY_DIV "
					"    AND CC_MACH_NO = @tpssm01.CC_MACH_NO "
					//"    AND PONO_STATUS >= 16 "
					"    AND CC_SEQ     < 900 "   //900以后是挂起的炉次
					);
				break;
			}
			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("tpssm01.FACTORY_DIV", tpssm01["FACTORY_DIV"].ToString());
			cmd_tpssm10_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
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
			Log::Trace("", __FUNCTION__, "当前出钢计划中最大的浇注顺序号cc_seq = [{0}]", cc_seq.ToInt32());


			//如果要新增的PONO已存在，那么删除以前的（再排计划时），重新接受.
			//如果要新增的PONO已编入出钢计划，此处的状态校验，在MMS层校验
			//删除该PONO后，不用考虑cc_seq 跳号，只要顺序排列出就行
			tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];  /* 炼钢单元号 -PK*/
			tpssm10["PONO"] = tpssm01["PONO"];
			sqlstr = "tpssm10.Delete()";
			tpssm10.Delete("FACTORY_DIV,PONO");


			//新增计划
			tpssm10.CopyFrom(tpssm01);

			tpssm10["PONO_STATUS"] = 16;                    /* 制造命令状态 */
			if (cc_seq == 0)
			{
				cc_seq = 1;
			}
			else
			{
				cc_seq = cc_seq + 1;
			}
			tpssm10["CC_SEQ"] = cc_seq;                /* 连铸顺序号 */

			tpssm10["TD_CHG_FLG"] = 0;                     /* 中间包更换标志 */
			//zxl20160822方坯T默认不要
			tpssm10["BILLET_TYPE"] = bcls_rec->Tables[0].Rows[0]["BILLET_TYPE"];
			if (tpssm10["BILLET_TYPE"].ToString() == "3")
			{
				tpssm10["RESTRAND_FLG"] = " ";
			}

			if (tpssm01["RESTRAND_FLG"].ToString().Trim() == "")//连浇
			{
				tpssm10["CC_PREP_TIME"] = 4;  // 炉间准备时间
			}
			else
			{
				tpssm10["CC_PREP_TIME"] = 100;  // 炉间准备时间
			}

			tpssm10["POUR_TIME"] = 40; //浇注时间
			tpssm10["SG_SIGN"] = tpssm01["REMARK"];//zxl20160817备注赋值给牌号字段
			tpssm10.TrimOrBlank();
			sqlstr = "tpssm10.Insert()";
			tpssm10.Insert();

			//dclian---add---2015-11-16------

			tpssm99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
			tpssm99["PONO"] = tpssm10["PONO"];
			tpssm99["EVENT_ID"] = "A1";
			tpssm99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
			tpssm99["VALID_FLAG"] = "1";

			tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
			//Log::Trace("", __FUNCTION__, "接收MMS下发的预计划，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
			//Log::Trace("", __FUNCTION__, "记录履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());

			//dclian---add---2015-11-16------

#ifdef _SYS_PES
			Log::Trace("", __FUNCTION__, "发送应答电文赋值");

			inBlock1.Tables["X200007"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			inBlock1.Tables["X200007"].Rows[0]["PONO"] = tpssm01["PONO"];
			inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "2";  // 2计划接收正常   
#endif      		
		}
		else if (v_proc_div == 3) //3-炉次删除
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句。
				sqlstr = CString(
					" SELECT FACTORY_DIV FROM TPSSM01  "
					" WHERE PONO = @tpssm01.PONO "

					);
				break;
			}
			cmd_tpssm01_inq1.SetCommandText(sqlstr);
			cmd_tpssm01_inq1.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm01_inq1.ExecuteReader();
			if (cmd_tpssm01_inq1.Read())
			{
				tpssm01["FACTORY_DIV"] = cmd_tpssm01_inq1.GetString(1);
			}
			cmd_tpssm01_inq1.Close();
			/* 特别注释：2014-12-03，xuwen修改，PONO删除逻辑
			1、判断当前PONO状态，如已编入出钢计划(18)，报错退出
			2、对于TPSSM10表，是开浇第一炉(带“T”)，T传递给下一炉，包括准备时间
			3、对于TPSSM01表，删除PONO及对应铸坯（对于浮动铸坯命令，不删除）
			*/
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["PONO"] =[{0}]", tpssm01["PONO"].ToString());
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["FACTORY_DIV"] =[{0}]", tpssm01["FACTORY_DIV"].ToString());
			//dclian---add---2015-11-16------
			tpssm99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm99["PONO"] = tpssm01["PONO"];
			tpssm99["EVENT_ID"] = "A3";
			tpssm99["VALID_FLAG"] = "1";
			tpssm99["PONO_STATUS"] = tpssm01["PONO_STATUS"];
			//正应答
			inBlock1.Tables["X200007"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			inBlock1.Tables["X200007"].Rows[0]["PONO"] = tpssm01["PONO"];
			inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "3";  // PONO删除成功

			//--------------------------
			//1、浇铸计划表处理
			//读取要删除PONO信息
			tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm10["PONO"] = tpssm01["PONO"];
			sqlstr = "tpssm10.Query()";
			bool has10 = tpssm10.Query();

			if (has10 == true) //有记录
			{
				//1）状态校验
				if (tpssm10["PONO_STATUS"].ToDecimal() >= 18) //编入出钢计划或生产，不能删除，给负应答
				{

					tpssm99["VALID_FLAG"] = "0";

					tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

					inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "4";  // PONO删除失败

					tpabort(0);
					tpbegin(0, 0);
					Log::Trace("", __FUNCTION__, "★★★★★记录LOT收回失败履历★★★★★");
					//记录履历
					ret = 0;
					ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

#ifdef _SYS_PES
					Log::Trace("", __FUNCTION__, "★★★★★发送应答电文开始★★★★★");
					//发送命令接收应答至MMS--
					ret = f_cm_200007_snd(&inBlock1, bcls_ret, conn);
					if (ret != 0)
					{
						//发送应答电文失败，不应该影响接收炉次制造命令电文
					}
					Log::Trace("", __FUNCTION__, "★★★★★发送应答f_cm_200007_snd★★★★★");

#endif
					tpcommit(0);
					tpbegin(0, 0);
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "要删除炉次[{0}]已排入出钢计划，不能删除。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//dclian---add---2015-11-16------
		/*		Log::Trace("", __FUNCTION__, "接收MMS下发的LOT收回，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());*/

#ifdef  _LINE_HP
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = "SELECT SLAB_NO "
						"  FROM TPSSM03 "
						" WHERE SLAB_PROD_FLAG	!= '1' "
						"   AND PONO	= @tpssm01.PONO ";
					break;
				}
				cmd_tpssm03_sql.SetCommandText(sqlstr);
				cmd_tpssm03_sql.Parameters.Clear();
				cmd_tpssm03_sql.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
				cmd_tpssm03_sql.ExecuteReader();
				while (cmd_tpssm03_sql.Read())
				{
					bcls_rec->Tables["PSHP"].Rows.Add();
					bcls_rec->Tables["PSHP"].Rows[rownum]["PREC_SLAB_NO"] = cmd_tpssm03_sql.GetString(1);//虚拟材料号。
					bcls_rec->Tables["PSHP"].Rows[rownum]["EVENT_ID"] = "PM"; //PSHP，要求PM写死，即可。
					bcls_rec->Tables["PSHP"].Rows[rownum]["FUNC_ID"] = "cm_h3h421_rcv"; //当前函数。
					rownum++;
				}
				cmd_tpssm03_sql.Close();

				int v_pshp_num = bcls_rec->Tables["PSHP"].Rows.get_Count();
				if (v_pshp_num >= 1)
				{//若有 数据，才调用
					doFlag = f_pshp_dhcr_del(bcls_rec, bcls_ret, conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
#endif

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


				//删除PONO
				sqlstr = "tpssm10.Delete()";
				tpssm10.Delete();

				Log::Trace("", __FUNCTION__, "删除tpssm10");


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

				//dclian---add---2015-11-16------

				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

				Log::Trace("", __FUNCTION__, "回收LOT,传入in_pssm99trace=[{0}]", in_pssm99trace.Tables[0].Rows[0]["EVENT_ID"].ToString());
				//dclian---add---2015-11-16------

			}//if  TPSSM10 有记录

			//--------------------------
			//2、炉次命令表（TPSSM01）处理
			//读取要删除PONO信息

			sqlstr = "tpssm01.Query()";
			dummy = tpssm01.QueryCount("FACTORY_DIV,PONO");

			//删除命令炉次表
			sqlstr = "tpssm01.Delete()";
			tpssm01.Delete();

			Log::Trace("", __FUNCTION__, "删除tpssm01");
			//--------------------------
			//3、删除LOT表(TPSSM02)
			if (dummy>0) //TPSSM01有记录
			{
				//查询炉次表中指定LOT下的PONO是否还有, 没有就将LOT删除
				sqlstr = "tpssm01.QueryCount()";
				dummy = tpssm01.QueryCount("FACTORY_DIV,CAST_LOT_NO");

				if (dummy <= 0) //没有炉次，LOT删除
				{
					tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"].ToString().Trim();
					tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"].ToString().Trim();
					sqlstr = "tpssm02.Delete()";
					tpssm02.Delete("FACTORY_DIV,CAST_LOT_NO");
				}
				Log::Trace("", __FUNCTION__, "删除tpssm02");
			}

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

			Log::Trace("", __FUNCTION__, "删除tpssm02");

#ifdef _LINE_BW
			////HYF 20140502调用轧钢函数，删除棒线轧制计划
			//CDataRow &row2 = inBlock2.Tables["PSBW"].Rows.Add();
			//row2["PONO"] = tpssm10["PONO"];
			////
			//ret = f_psbw_roll_del(&inBlock2, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
#endif


		}//if 4-炉次收回，按指定PONO删除
		else if (v_proc_div == 4)
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:         // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:  // 所有数据库适用，通用SQL语句。
				sqlstr = CString(
					" SELECT FACTORY_DIV FROM TPSSM01  "
					" WHERE PONO = @tpssm01.PONO "

					);
				break;
			}
			cmd_tpssm01_inq1.SetCommandText(sqlstr);
			cmd_tpssm01_inq1.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm01_inq1.ExecuteReader();
			if (cmd_tpssm01_inq1.Read())
			{
				tpssm01["FACTORY_DIV"] = cmd_tpssm01_inq1.GetString(1);
			}
			cmd_tpssm01_inq1.Close();
			/* 特别注释：2014-12-03，xuwen修改，PONO删除逻辑
			1、判断当前PONO状态，如已编入出钢计划(18)，报错退出
			2、对于TPSSM10表，是开浇第一炉(带“T”)，T传递给下一炉，包括准备时间
			3、对于TPSSM01表，删除PONO及对应铸坯（对于浮动铸坯命令，不删除）
			*/
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["PONO"] =[{0}]", tpssm01["PONO"].ToString());
			//Log::Trace("", __FUNCTION__, "炉次收回tpssm01["FACTORY_DIV"] =[{0}]", tpssm01["FACTORY_DIV"].ToString());
			//dclian---add---2015-11-16------
			tpssm99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm99["PONO"] = tpssm01["PONO"];
			tpssm99["EVENT_ID"] = "A3";
			tpssm99["VALID_FLAG"] = "1";
			tpssm99["PONO_STATUS"] = tpssm01["PONO_STATUS"];
			//正应答
			inBlock1.Tables["X200007"].Rows[0]["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			inBlock1.Tables["X200007"].Rows[0]["PONO"] = tpssm01["PONO"];
			inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "0";  // LOT收回正常

			//--------------------------
			//1、浇铸计划表处理
			//读取要删除PONO信息
			tpssm10["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			tpssm10["PONO"] = tpssm01["PONO"];
			sqlstr = "tpssm10.Query()";
			bool has10 = tpssm10.Query();

			if (has10 == true) //有记录
			{
				//1）状态校验
				if (tpssm10["PONO_STATUS"].ToDecimal() >= 18) //编入出钢计划或生产，不能删除，给负应答
				{

					tpssm99["VALID_FLAG"] = "0";

					tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

					inBlock1.Tables["X200007"].Rows[0]["ACK_CODE"] = "1";  // 收回失败1

					tpabort(0);
					tpbegin(0, 0);
					Log::Trace("", __FUNCTION__, "★★★★★记录LOT收回失败履历★★★★★");
					//记录履历
					ret = 0;
					ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

#ifdef _SYS_PES
					Log::Trace("", __FUNCTION__, "★★★★★发送应答电文开始★★★★★");
					//发送命令接收应答至MMS--
					ret = f_cm_200007_snd(&inBlock1, bcls_ret, conn);
					if (ret != 0)
					{
						//发送应答电文失败，不应该影响接收炉次制造命令电文
					}
					Log::Trace("", __FUNCTION__, "★★★★★发送应答f_cm_200007_snd★★★★★");
#endif

					tpcommit(0);
					tpbegin(0, 0);
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "要删除炉次[{0}]已排入出钢计划，不能删除。", arguments, 1);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//dclian---add---2015-11-16------
			/*	Log::Trace("", __FUNCTION__, "接收MMS下发的LOT收回，事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());*/

#ifdef  _LINE_HP
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default:
					sqlstr = "SELECT SLAB_NO "
						"  FROM TPSSM03 "
						" WHERE SLAB_PROD_FLAG	!= '1' "
						"   AND PONO	= @tpssm01.PONO ";
					break;
				}
				cmd_tpssm03_sql.SetCommandText(sqlstr);
				cmd_tpssm03_sql.Parameters.Clear();
				cmd_tpssm03_sql.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
				cmd_tpssm03_sql.ExecuteReader();
				while (cmd_tpssm03_sql.Read())
				{
					bcls_rec->Tables["PSHP"].Rows.Add();
					bcls_rec->Tables["PSHP"].Rows[rownum]["PREC_SLAB_NO"] = cmd_tpssm03_sql.GetString(1);//虚拟材料号。
					bcls_rec->Tables["PSHP"].Rows[rownum]["EVENT_ID"] = "PM"; //PSHP，要求PM写死，即可。
					bcls_rec->Tables["PSHP"].Rows[rownum]["FUNC_ID"] = "cm_h3h421_rcv"; //当前函数。
					rownum++;
				}
				cmd_tpssm03_sql.Close();

				int v_pshp_num = bcls_rec->Tables["PSHP"].Rows.get_Count();
				if (v_pshp_num >= 1)
				{//若有 数据，才调用
					doFlag = f_pshp_dhcr_del(bcls_rec, bcls_ret, conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
#endif

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


				//删除PONO
				sqlstr = "tpssm10.Delete()";
				tpssm10.Delete();

				Log::Trace("", __FUNCTION__, "删除tpssm10");


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

				//dclian---add---2015-11-16------

				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);

				Log::Trace("", __FUNCTION__, "回收LOT,传入in_pssm99trace=[{0}]", in_pssm99trace.Tables[0].Rows[0]["EVENT_ID"].ToString());
				//dclian---add---2015-11-16------

			}//if  TPSSM10 有记录

			//--------------------------
			//2、炉次命令表（TPSSM01）处理
			//读取要删除PONO信息

			sqlstr = "tpssm01.Query()";
			dummy = tpssm01.QueryCount("FACTORY_DIV,PONO");

			//删除命令炉次表
			sqlstr = "tpssm01.Delete()";
			tpssm01.Delete();

			Log::Trace("", __FUNCTION__, "删除tpssm01");
			//--------------------------
			//3、删除LOT表(TPSSM02)
			if (dummy>0) //TPSSM01有记录
			{
				//查询炉次表中指定LOT下的PONO是否还有, 没有就将LOT删除
				sqlstr = "tpssm01.QueryCount()";
				dummy = tpssm01.QueryCount("FACTORY_DIV,CAST_LOT_NO");

				if (dummy <= 0) //没有炉次，LOT删除
				{
					tpssm02["FACTORY_DIV"] = tpssm01["FACTORY_DIV"].ToString().Trim();
					tpssm02["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"].ToString().Trim();
					sqlstr = "tpssm02.Delete()";
					tpssm02.Delete("FACTORY_DIV,CAST_LOT_NO");
				}
				Log::Trace("", __FUNCTION__, "删除tpssm02");
			}

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

			Log::Trace("", __FUNCTION__, "删除tpssm02");

#ifdef _LINE_BW
			////HYF 20140502调用轧钢函数，删除棒线轧制计划
			//CDataRow &row2 = inBlock2.Tables["PSBW"].Rows.Add();
			//row2["PONO"] = tpssm10["PONO"];
			////
			//ret = f_psbw_roll_del(&inBlock2, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
#endif


		}//if 4-炉次收回，按指定PONO删除
		
#ifdef _SYS_PES
		Log::Trace("", __FUNCTION__, "最后发送正应答至MMS=[{0}]", inBlock1.Tables[0].Rows.get_Count());
		//发送命令接收应答至MMS--
		ret = f_cm_200007_snd(&inBlock1, bcls_ret, conn);
		if (ret != 0)
		{
			//发送应答电文失败，不应该影响接收炉次制造命令电文
		}
		Log::Trace("", __FUNCTION__, "最终记录成功履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
#endif

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
	cmd_tpssm03_upd.Close();
	cmd_tpssm01_inq.Close();
	cmd_tpssm10_inq.Close();
	return doFlag;

}
