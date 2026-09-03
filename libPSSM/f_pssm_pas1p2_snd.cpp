/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dclian
Version:    1.0
Date:     2016-1-05
Description:	 发送铸坯命令至L2。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

#include "tpssmd1.h"
#include "tpssm01.h"
#include "tpssm11.h"
#include "tpssm12.h"
#include "tpssm03.h"


/*<remark>=========================================================
/// <summary>
/// 发送出钢计划至L2
///<para>1.读取传入的计划号、厂别区分</para>
/// <para>2.拼接电文后发送L2。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：保存下发调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="sm_plan_no">计划号          </param>
/// <param name="heat_no">熔炼号          </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_pas1p2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int fetchRowCount1 = 0;

	CString sqlstr = "";
	int send_flag = 0;

	CString lpsz_tc_no = " ";
	int tele_num = 1;

	CDecimal d_total_tel_num = 0;
	CDecimal v_count = 0;
	CDecimal plan_charge_num = 0;
	CString v_bof_no = " ";
	CString v_sg_sign = " ";
	CString v_ladle_level = " ";
	CString factory_div = " ";//分区标志
	CString oper_flag = " ";//操作区分标志

	CDecimal max_num = 25; //坯子电文循环数

	EPEX epex(&s, conn, 1);

	CTPSSMD1 tpssmd1(conn);
	CTPSSM01 tpssm01(conn);
	CTPSSM11 tpssm11(conn);
	CTPSSM12 tpssm12(conn);
	CTPSSM03 tpssm03(conn);

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_inq(conn);
	try
	{
		Log::Trace("", __FUNCTION__, "传入行=[{0}]", bcls_rec->Tables["SLAB"].Rows.get_Count());
		/*for (int i = 0; i < bcls_rec->Tables["SLAB"].Rows.get_Count(); i++)
		{*/
			factory_div = bcls_rec->Tables["SLAB"].Rows[0]["FACTORY_DIV"].ToString().Trim();
			tpssm11.PONO = bcls_rec->Tables["SLAB"].Rows[0]["PONO"].ToString().Trim();
			oper_flag = bcls_rec->Tables["SLAB"].Rows[0]["OPER_FLAG"].ToString().Trim();

			Log::Trace("", __FUNCTION__, "传入factory_div=[{0}]", factory_div);
			Log::Trace("", __FUNCTION__, "传入tpssm11.PONO=[{0}]", tpssm11.PONO);
			Log::Trace("", __FUNCTION__, "传入oper_flag=[{0}]", oper_flag);

			if (factory_div.Trim() == "1")
			{
				lpsz_tc_no = "PAS1P2";/*一区赋电文号*/
			}
			if (factory_div.Trim() == "2")
			{
				lpsz_tc_no = "PAS2P2";/*二区赋电文号*/
			}
			if (factory_div.Trim() == "3")
			{
				lpsz_tc_no = "PAS3P2";/*三区赋电文号*/
			}
			sqlstr = "SELECT a.*  \
					 				 		 FROM tpssm11  a  \
											 						 WHERE a.factory_div = @factory_div \
																	 							 AND  PONO = @tpssm11.PONO ";
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("factory_div", factory_div);
			cmd_tpssm11_inq.Parameters.Set("tpssm11.PONO", tpssm11.PONO);
			cmd_tpssm11_inq.ExecuteReader();
			while (cmd_tpssm11_inq.Read())
			{
				cmd_tpssm11_inq.Fetch(tpssm11);

			}
			cmd_tpssm11_inq.Close();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = " SELECT COUNT(1) FROM TPSSM03 \
						 					 WHERE  FACTORY_DIV = @factory_div \
											 					 					AND  PONO = @tpssm11.PONO \
																										and  SLAB_PROD_FLAG <> '1' ";
				break;
			}
			cmd_tpssm03_inq.SetCommandText(sqlstr);
			cmd_tpssm03_inq.Parameters.Set("factory_div", factory_div);
			cmd_tpssm03_inq.Parameters.Set("tpssm11.PONO", tpssm11.PONO);
			v_count = cmd_tpssm03_inq.ExecuteScalar();
			cmd_tpssm03_inq.Close();
			Log::Trace("", __FUNCTION__, "PONO=[{0}]当前铸坯行数v_count[{1}]", tpssm11.PONO,v_count.ToInt32());
			if (v_count != 0)
			{
				//有计划
				plan_charge_num = 0;
				d_total_tel_num = 0;

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT COUNT(1) FROM TPSSM03 \
							 						 WHERE  FACTORY_DIV = @factory_div \
													 						 AND  PONO = @tpssm11.PONO \
																			 						 and  SLAB_PROD_FLAG <> '1' ";
					break;
				}
				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm03_inq.Parameters.Set("tpssm11.PONO", tpssm11.PONO);
				plan_charge_num = cmd_tpssm03_inq.ExecuteScalar();
				cmd_tpssm03_inq.Close();

				Log::Trace("", __FUNCTION__, "plan_charge_num = [{0}]", plan_charge_num.ToInt32());

				//电文数
				sqlstr = " SELECT CEIL(@plan_charge_num/@max_num) FROM dual";

				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("plan_charge_num", plan_charge_num);
				cmd_tpssm03_inq.Parameters.Set("max_num", max_num);
				d_total_tel_num = cmd_tpssm03_inq.ExecuteScalar();
				cmd_tpssm03_inq.Close();
				Log::Trace("", __FUNCTION__, "d_total_tel_num = [{0}]", d_total_tel_num.ToInt32());

				fetchRowCount1 == 0;

				sqlstr = "SELECT a.*  \
						 						 FROM tpssm03 a  \
												 						 WHERE a.factory_div = @factory_div \
																		 						 AND  PONO = @tpssm11.PONO \
																								 						 and  SLAB_PROD_FLAG <> '1' ";
				cmd_tpssm03_inq.SetCommandText(sqlstr);
				cmd_tpssm03_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm03_inq.Parameters.Set("tpssm11.PONO", tpssm11.PONO);
				cmd_tpssm03_inq.ExecuteReader();
				while (cmd_tpssm03_inq.Read())
				{
					cmd_tpssm03_inq.Fetch(tpssm03);//把数据都压在头文件里面

					sqlstr = " SELECT MOD(@fetchRowCount1, @max_num) FROM dual";

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("fetchRowCount1", fetchRowCount1);
					cmd_inq.Parameters.Set("max_num", max_num);
					v_count = cmd_inq.ExecuteScalar();
					cmd_inq.Close();
					fetchRowCount1 = v_count.ToInt32();
					Log::Trace("", __FUNCTION__, "fetchRowCount1 = [{0}]", fetchRowCount1);

					Log::Trace("", __FUNCTION__, "发送铸坯命令，电文号lpsz_tc_no = [{0}]", lpsz_tc_no);

					if (fetchRowCount1 == 0)
					{
						//初始化
						if (epex.Initialize(lpsz_tc_no) < 0)
						{
							Log::Trace("", __FUNCTION__, "初始化出错.");
							Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					//操作标记
					if (epex.SetValue("oper_flag", 0, oper_flag.Trim()) < 0)
					{
						Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
						Log::Trace("", __FUNCTION__, "初始化oper_flag出错.");
						throw CApplicationException(-1, s.msg, log.Location);
					}
					if (epex.SetValue("cc_mach_no", 0, tpssm11.CC_MACH_NO) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误cc_mach_no= [{0}]", tpssm11.CC_MACH_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					if (epex.SetValue("pono", 0, tpssm11.PONO) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误pono = [{0}]", tpssm11.PONO);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					if (epex.SetValue("cast_lot_no", fetchRowCount1, tpssm03.CAST_LOT_NO) < 0)
					{
						Log::Trace("", __FUNCTION__, "cast_lot_no = [{0}]赋值失败", tpssm03.CAST_LOT_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}
					if (epex.SetValue("st_no", 0, tpssm11.ST_NO) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误st_no = [{0}]", tpssm11.ST_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tpssm01.PONO = tpssm11.PONO.Trim();
					tpssm01.FACTORY_DIV = tpssm11.FACTORY_DIV.Trim();
					if (tpssm01.Query("PONO,FACTORY_DIV"))
					{
						if (epex.SetValue("cast_lot_sum", 0, tpssm01.CAST_LOT_SUM) < 0)
						{
							Log::Trace("", __FUNCTION__, "压值错误cast_lot_sum = [{0}]", tpssm01.CAST_LOT_SUM);
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					

					Log::Trace("", __FUNCTION__, "铸坯号slab_no = [{0}]", tpssm03.SLAB_NO);

					if (epex.SetValue("slab_no", fetchRowCount1, tpssm03.SLAB_NO) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误furnace_no= [{0}]", tpssm03.SLAB_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}



					if (epex.SetValue("slab_num", fetchRowCount1, tpssm03.SLAB_NUM) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误cast_no= [{0}]", tpssm11.CAST_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					if (epex.SetValue("slab_width", fetchRowCount1, tpssm03.SLAB_WIDTH) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误SLAB_WIDTH= [{0}]", tpssm03.SLAB_WIDTH);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					if (epex.SetValue("slab_thick", fetchRowCount1, tpssm03.SLAB_THICK) < 0)
					{
						Log::Trace("", __FUNCTION__, "压值错误cast_div_no= [{0}]", tpssm11.CAST_DIV_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//
					if (epex.SetValue("slab_max_len", fetchRowCount1, tpssm03.SLAB_MAX_LEN) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//最小长度
					if (epex.SetValue("slab_min_len", fetchRowCount1, tpssm03.SLAB_MIN_LEN) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//长度
					if (epex.SetValue("slab_len", fetchRowCount1, tpssm03.SLAB_LEN) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//目标取向
					if (epex.SetValue("slab_dest", fetchRowCount1, tpssm03.SLAB_DEST) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					//热装标记
					if (epex.SetValue("hot_charge_flag", fetchRowCount1, tpssm03.HOT_CHARGE_FLAG) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//热送标记
					if (epex.SetValue("hot_send_flag", fetchRowCount1, tpssm03.HOT_SEND_FLAG) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					if (fetchRowCount1 == (max_num - 1))
					{
						//发送电文
						if (epex.SendTele() < 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}

						// 释放
						epex.Uninitialize();
						Log::Trace("", __FUNCTION__, "下发铸坯命令成功= [{0}]", tele_num);

						send_flag = 1;
						tele_num++;
					}

					fetchRowCount1 = fetchRowCount1 + 1;
					send_flag = 0;

					Log::Trace("", __FUNCTION__, "+1后的 fetchRowCount1== [{0}]", fetchRowCount1);
					Log::Trace("", __FUNCTION__, "tpssm11.pono=[{0}]的压电文结束", tpssm11.PONO);

				}
				if (send_flag == 0 && fetchRowCount1 != max_num)
				{
					Log::Trace("", __FUNCTION__, "计划不满循环数 发送电文");

					/*发送电文*/
					if (epex.SendTele() < 0)
					{
						//EDLog(1, 1, "发送出钢计划电文 = [%s]",(const char*) epex.GetMsg());
						Log::Trace("", __FUNCTION__, "发送出钢计划电文=[{0}]", epex.GetMsg());
						sprintf(s.msg, _RES("GCRSS0000032")/*电文发送失败。*/);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					// 释放
					epex.Uninitialize();
					Log::Trace("", __FUNCTION__, "last send");

				}
				cmd_tpssm03_inq.Close();

			}
			else
			{
				Log::Trace("", __FUNCTION__, "CCCCCCC 000000 send!");

			}
		/*}*/
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

	return doFlag;

}

