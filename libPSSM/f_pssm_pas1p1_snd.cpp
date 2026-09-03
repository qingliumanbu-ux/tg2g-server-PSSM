/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   dclian
Version:    1.0
Date:     2016-1-05
Description:	 发送出钢计划至L2。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "epex.h"

#include "tpssmd1.h"
#include "tpssm01.h"s
#include "tpssm10.h"
#include "tpssm11.h"
#include "tpssm12.h"
#include "tpssm99.h"

int f_pssm_pas1p2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//铸坯命令发送
int f_cm_pam1p2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//铸坯命令发送
int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入
/*<remark>=========================================================
/// <summary>
/// 发送出钢计划至L2
///<para>1.读取传入的计划号、制造命令号、熔炼号</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：保存下发调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="sm_plan_no">计划号          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="heat_no">熔炼号          </param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm_pas1p1_snd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;
	int fetchRowCount1 = 0;

	CString sqlstr="";
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
	CDecimal max_num = 15; //电文循环数

	EPEX epex(&s, conn, 1);

	CTPSSMD1 tpssmd1(conn);
	CTPSSM01 tpssm01(conn);
	CTPSSM10 tpssm10(conn);
	CTPSSM11 tpssm11(conn);
	CTPSSM12 tpssm12(conn);
	CTPSSM99 tpssm99(conn);

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_inq(conn);
	EIClass inblk;        //调用函数用
	EIClass in_pssm99trace;//调用履历函数
	
	try
	{
		factory_div = bcls_rec->Tables["PAS"].Rows[0]["FACTORY_DIV"].ToString();
		oper_flag = bcls_rec->Tables["PAS"].Rows[0]["OPER_FLAG"].ToString();
		Log::Trace("", __FUNCTION__, "传入factory_div=[{0}]", factory_div);
		Log::Trace("", __FUNCTION__, "传入OPER_FLAG=[{0}]", oper_flag);
		//1、铸坯命令下发
		inblk.Tables[0].set_TableName("SLAB");    //铸坯命令下发
		inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");   //炼钢单元号
		inblk.Tables[0].Columns.Add(DT_STRING, "PONO");    //制造命令号
		inblk.Tables[0].Columns.Add(DT_STRING, "OPER_FLAG");    //操作区分标志
		inblk.Tables[0].Rows.Add();//只需要传入一行
		//2、制造命令状态反馈到MMS
		inblk.Tables.Add("X200009");
		inblk.Tables["X200009"].Columns.Add(DT_STRING, "PONO");   //制造命令号
		inblk.Tables["X200009"].Columns.Add(DT_DECIMAL, "PONO_STATUS");    //制造命令状态

		//3、计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);
		if (factory_div.Trim()=="1")
		{
			lpsz_tc_no = "PAS1P1";/*一区赋电文号*/
		}
		if (factory_div.Trim() == "2")
		{
			lpsz_tc_no = "PAS2P1";/*二区赋电文号*/
		}
		if (factory_div.Trim() == "3")
		{
			lpsz_tc_no = "PAS3P1";/*三区赋电文号*/
		}

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT COUNT(1) FROM TPSSM11 \
					 WHERE  FACTORY_DIV = @factory_div \
					 AND  RUN_STATUS        < 52  " ;
			break;
		}
		if (oper_flag.Trim()=="D")
		{
			sqlstr = sqlstr + " AND  PLAN_EDIT_FLAG = 'D' ";
		}
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("factory_div", factory_div);
		v_count = cmd_tpssm11_inq.ExecuteScalar();
		cmd_tpssm11_inq.Close();
		Log::Trace("", __FUNCTION__, "转炉区sqlstr[{0}]", sqlstr);
		Log::Trace("",__FUNCTION__,"转炉区当前计划数v_count[{0}]",v_count.ToInt32());
		if(v_count != 0)
		{  	
			//有计划
			plan_charge_num = 0;		
			d_total_tel_num = 0;

			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = " SELECT COUNT(1) FROM TPSSM11 \
						 WHERE  FACTORY_DIV = @factory_div \
						 AND  RUN_STATUS        < 52" ;
				break;
			}
			if (oper_flag.Trim() == "D")
			{
				sqlstr = sqlstr + " AND  PLAN_EDIT_FLAG = 'D' ";
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("factory_div", factory_div);
			plan_charge_num = cmd_tpssm11_inq.ExecuteScalar();//总炉数
			cmd_tpssm11_inq.Close();

			Log::Trace("", __FUNCTION__, "plan_charge_num = [{0}]", plan_charge_num.ToInt32());

			//电文数
			sqlstr = " SELECT CEIL(@plan_charge_num/@max_num) FROM dual";

			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("plan_charge_num",plan_charge_num);
			cmd_tpssm11_inq.Parameters.Set("max_num",max_num);
			d_total_tel_num = cmd_tpssm11_inq.ExecuteScalar();//总电文数
			cmd_tpssm11_inq.Close();
			Log::Trace("", __FUNCTION__, "d_total_tel_num = [{0}]", d_total_tel_num.ToInt32());

			fetchRowCount1 == 0;

			sqlstr = "SELECT a.*  \
						 FROM tpssm11 a, tpssm12 b \
						 WHERE a.factory_div = @factory_div \
						 AND a.sm_plan_no = b.sm_plan_no \
						 AND b.area_id = 3 \
						 AND a.run_status        < 52 ";
			if (oper_flag.Trim() == "D")
			{
				sqlstr = sqlstr + " AND  PLAN_EDIT_FLAG = 'D' ";
			}
			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("factory_div", factory_div);
			cmd_tpssm11_inq.ExecuteReader();
			while(cmd_tpssm11_inq.Read())
			{
				cmd_tpssm11_inq.Fetch(tpssm11);//把数据都压在头文件里面

				sqlstr = " SELECT MOD(@fetchRowCount1, @max_num) FROM dual";

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("fetchRowCount1",fetchRowCount1);
				cmd_inq.Parameters.Set("max_num",max_num);
				v_count = cmd_inq.ExecuteScalar();
				cmd_inq.Close();
				fetchRowCount1 = v_count.ToInt32();
				Log::Trace("", __FUNCTION__, "第fetchRowCount1 = [{0}]炉", fetchRowCount1);

				if (fetchRowCount1 == 0)
				{
					//初始化
					if (epex.Initialize(lpsz_tc_no) < 0)
					{
						Log::Trace("",__FUNCTION__,"初始化出错.");
						Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]",epex.GetMsg());
						throw CApplicationException(-1,s.msg,log.Location);
					}
				}
				//操作区分
				if (epex.SetValue("oper_flag", 0, oper_flag.Trim()) < 0)
				{
					Log::Info("", __FUNCTION__, "epex.GetMsg()=[{0}]", epex.GetMsg());
					Log::Trace("", __FUNCTION__, "初始化oper_flag出错.");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				//总炉数
				if(epex.SetValue("plan_lot_num", 0, plan_charge_num) < 0)
				{
					Log::Trace("",__FUNCTION__,"初始化plan_lot_num出错.");
					throw CApplicationException(-1, s.msg, log.Location);				
				}
				//第几条电文
				if(epex.SetValue("travel_tc_no", 0, tele_num) < 0)
				{
					Log::Trace("",__FUNCTION__,"初始化travel_tc_no出错.");
					throw CApplicationException(-1, s.msg, log.Location);				
				}
				//电文总数
				if(epex.SetValue("travel_tc_num", 0, d_total_tel_num) < 0)
				{
					Log::Trace("",__FUNCTION__,"初始化travel_tc_num出错.");
					strcpy(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
					strcpy(s.sysmsg,"电文hmjl02拼接失败");
					throw CApplicationException(-1, s.msg, log.Location);				
				}
				//本电文中炉数
				if(epex.SetValue("this_tc_lot_num", 0, fetchRowCount1 + 1) < 0)
				{
					Log::Trace("",__FUNCTION__,"初始化this_tc_lot_num出错.");
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				if(epex.SetValue("sm_plan_no", fetchRowCount1,tpssm11.SM_PLAN_NO) < 0)
				{
					Log::Trace("", __FUNCTION__, "sm_plan_no = [{0}]", tpssm11.SM_PLAN_NO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				if(epex.SetValue("pono", fetchRowCount1,tpssm11.PONO) < 0)
				{
					Log::Trace("", __FUNCTION__, "pono = [{0}]", tpssm11.PONO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}
				//查询浇铸批号：主要因柳钢中板二切那边要求降低余材率，坯子按照浇铸批号为切割单元，尤其是两炉之间的坯子，只要在这个浇次内就可以看成成才。
				//2016-3-24日add
				tpssm10.PONO = tpssm11.PONO;
				tpssm10.FACTORY_DIV = tpssm11.FACTORY_DIV;
				
				if (tpssm10.Query("PONO,FACTORY_DIV"))
				{
					if (epex.SetValue("cast_lot_no", fetchRowCount1, tpssm10.CAST_LOT_NO) < 0)
					{
						Log::Trace("", __FUNCTION__, "cast_lot_no = [{0}]赋值失败", tpssm10.CAST_LOT_NO);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}

				

				if(epex.SetValue("heat_no", fetchRowCount1,tpssm11.HEAT_NO) < 0)
				{
					Log::Trace("", __FUNCTION__, "heat_no = [{0}]", tpssm11.HEAT_NO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				if(epex.SetValue("st_no", fetchRowCount1,tpssm11.ST_NO) < 0)
				{
					Log::Trace("", __FUNCTION__, "st_no = [{0}]", tpssm11.ST_NO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}			
				//热送热装标记，牌号
				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
					sqlstr = " SELECT  HOT_SEND_FLAG,HOT_CHARGE_FLAG  FROM TPSSM01 \
							 WHERE FACTORY_DIV = @factory_div \
							 AND PONO = @pono  ";
					break;
				}
				CDbCommand cmd_tpssm01_inq(sqlstr, conn);
				cmd_tpssm01_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm01_inq.Parameters.Set("pono", tpssm11.PONO);
				cmd_tpssm01_inq.ExecuteReader();
				if ( cmd_tpssm01_inq.Read() )
				{
					tpssm01.HOT_SEND_FLAG = cmd_tpssm01_inq.GetString(1);
					tpssm01.HOT_CHARGE_FLAG = cmd_tpssm01_inq.GetString(2);
					v_sg_sign = cmd_tpssm01_inq.GetString(1);
				}
				cmd_tpssm01_inq.Close();
				//if (epex.SetValue("hot_send_flag", fetchRowCount1, tpssm01.HOT_SEND_FLAG) < 0)
				//{
				//	Log::Debug("", __FUNCTION__, "SetValue hot_send_flag:{0}", epex.GetMsg());
				//	Log::Trace("", __FUNCTION__, "hot_send_flag= [{0}]", tpssm01.HOT_SEND_FLAG);
				//    throw CApplicationException(-1, s.msg, log.Location);
				//}
				//if (epex.SetValue("hot_charge_flag", fetchRowCount1, tpssm01.HOT_CHARGE_FLAG) < 0)
				//{
				//	Log::Trace("", __FUNCTION__, "hot_charge_flag= [{0}]", tpssm01.HOT_CHARGE_FLAG);
				//	throw CApplicationException(-1, s.msg, log.Location);
				//}

				/*if(epex.SetValue("sg_sign", fetchRowCount1, v_sg_sign.SubstringNE(0,50)) < 0)
				{
					Log::Trace("", __FUNCTION__, "sg_sign = [{0}]", v_sg_sign.SubstringNE(0,50));
					throw CApplicationException(-1, s.msg, log.Location);				
				}*/

				if(epex.SetValue("refine_route_code", fetchRowCount1,tpssm11.REFINE_ROUTE_CODE) < 0)
				{
					Log::Trace("", __FUNCTION__, "初始化 refine_route_code 出错");
					throw CApplicationException(-1, s.msg, log.Location);				
				}				

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT  DEV_CODE  FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 3 \
							 AND  SM_PLAN_NO              = @tpssm11.SM_PLAN_NO  ";
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.DEV_CODE = cmd_tpssm12_inq.GetString(1);
				}
				cmd_tpssm12_inq.Close();
				Log::Trace("", __FUNCTION__, "炉座号tpssm12.DEV_CODE = [{0}]", tpssm12.DEV_CODE);

				if (epex.SetValue("furnace_no", fetchRowCount1, tpssm12.DEV_CODE) < 0)
				{
					Log::Trace("", __FUNCTION__, "furnace_no= [{0}]", tpssm12.DEV_CODE);
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				if(epex.SetValue("cc_mach_no", fetchRowCount1,tpssm11.CC_MACH_NO) < 0)
				{
					Log::Trace("", __FUNCTION__, "cc_mach_no= [{0}]", tpssm11.CC_MACH_NO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}			

				if(epex.SetValue("cast_no", fetchRowCount1, tpssm11.CAST_NO) < 0)
				{
					Log::Trace("", __FUNCTION__, "cast_no= [{0}]", tpssm11.CAST_NO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}
				
				/*if(epex.SetValue("cast_pono_sum", fetchRowCount1, tpssm11.CAST_PONO_SUM) < 0)
				{
					Log::Trace("", __FUNCTION__, "cast_pono_sum= [{0}]", tpssm11.CAST_PONO_SUM);
					throw CApplicationException(-1, s.msg, log.Location);				
				}*/

				if(epex.SetValue("cast_div_no", fetchRowCount1, tpssm11.CAST_DIV_NO) < 0)
				{
					Log::Trace("", __FUNCTION__, "cast_div_no= [{0}]", tpssm11.CAST_DIV_NO);
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				//中包快换标志
				if(epex.SetValue("td_chg_flg", fetchRowCount1, tpssm11.TD_CHG_FLG) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				////吹氩指示
				//if(epex.SetValue("ar_flag", fetchRowCount1, " ") < 0)
				//{
				//	throw CApplicationException(-1, s.msg, log.Location);				
				//}

				//装料开始时刻
				if(epex.SetValue("bof_start_time", fetchRowCount1, " ") < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT START_TIME, END_TIME FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 3 \
							 AND  SM_PLAN_NO              = @tpssm11.SM_PLAN_NO " ;
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.START_TIME = cmd_tpssm12_inq.GetString(1);
					tpssm12.END_TIME   = cmd_tpssm12_inq.GetString(2);
				}
				cmd_tpssm12_inq.Close();

				//开吹时刻
				if(epex.SetValue("blow_start_time", fetchRowCount1, tpssm12.START_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}	

				//出钢终时刻
				if(epex.SetValue("bof_end_time", fetchRowCount1, tpssm12.END_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}	

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT START_TIME FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 4 \
							 AND  CHARGE_NO         = 2 \
							 AND  SM_PLAN_NO        = @tpssm11.SM_PLAN_NO " ;
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.START_TIME = cmd_tpssm12_inq.GetString(1);
				}
				else
				{
					tpssm12.START_TIME = " ";
				}
				cmd_tpssm12_inq.Close();

				//精炼1开始时刻
				if(epex.SetValue("refine_start_time_1", fetchRowCount1,tpssm12.START_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT START_TIME FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 4 \
							 AND  CHARGE_NO         = 3 \
							 AND  SM_PLAN_NO              = @tpssm11.SM_PLAN_NO " ;
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.START_TIME = cmd_tpssm12_inq.GetString(1);
				}
				else
				{
					tpssm12.START_TIME = " ";
				}
				cmd_tpssm12_inq.Close();

				//精炼2开始时刻
				if(epex.SetValue("refine_start_time_2", fetchRowCount1,tpssm12.START_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT START_TIME FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 4 \
							 AND  CHARGE_NO         = 4 \
							 AND  SM_PLAN_NO              = @tpssm11.SM_PLAN_NO " ;
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.START_TIME = cmd_tpssm12_inq.GetString(1);
				}
				else
				{
					tpssm12.START_TIME = " ";
				}
				cmd_tpssm12_inq.Close();

				//精炼3开始时刻
				if(epex.SetValue("refine_start_time_3", fetchRowCount1,tpssm12.START_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT START_TIME FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 4 \
							 AND  CHARGE_NO         = 5 \
							 AND  SM_PLAN_NO              = @tpssm11.SM_PLAN_NO " ;
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.START_TIME = cmd_tpssm12_inq.GetString(1);
				}
				else
				{
					tpssm12.START_TIME = " ";
				}
				cmd_tpssm12_inq.Close();

				//精炼4开始时刻
				if(epex.SetValue("refine_start_time_4", fetchRowCount1,tpssm12.START_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				switch(conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:

					sqlstr = " SELECT LADLE_ARRIVE_TIME,START_TIME,END_TIME FROM TPSSM12 \
							 WHERE  FACTORY_DIV = @factory_div \
							 AND  AREA_ID           = 5 \
							 AND  SM_PLAN_NO              = @tpssm11.SM_PLAN_NO " ;
					break;
				}
				cmd_tpssm12_inq.SetCommandText(sqlstr);
				cmd_tpssm12_inq.Parameters.Set("factory_div", factory_div);
				cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11.SM_PLAN_NO);
				cmd_tpssm12_inq.ExecuteReader();
				if(cmd_tpssm12_inq.Read())
				{
					tpssm12.LADLE_ARRIVE_TIME = cmd_tpssm12_inq.GetString(1);
					tpssm12.START_TIME = cmd_tpssm12_inq.GetString(2);
					tpssm12.END_TIME = cmd_tpssm12_inq.GetString(3);
				}
				else
				{
					tpssm12.START_TIME = " ";
				}
				cmd_tpssm12_inq.Close();

				//CC要求时刻/钢包到达时刻
				if(epex.SetValue("cc_ladle_arr_time", fetchRowCount1,tpssm12.LADLE_ARRIVE_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				//钢包浇注始时刻
				if(epex.SetValue("pour_time_begin", fetchRowCount1,tpssm12.START_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				//钢包浇注终时刻
				if(epex.SetValue("pour_time_end", fetchRowCount1,tpssm12.END_TIME) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);				
				}

				////计划钢水量
				//if(epex.SetValue("plan_tap_wt", fetchRowCount1,tpssm11.PLAN_TAP_WT) < 0)
				//{
				//	throw CApplicationException(-1, s.msg, log.Location);				
				//}
				//钢包镇静（早到）时间---暂时给0
				if (epex.SetValue("ladle_kill_time_pre_arr", fetchRowCount1, 0) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
				////钢包号
				//if(epex.SetValue("ladle_no", fetchRowCount1,tpssm11.LADLE_NO) < 0)
				//{
				//	throw CApplicationException(-1, s.msg, log.Location);				
				//}
				
				////钢包等级
				//sqlstr = " SELECT LADLE_LEVEL FROM TTMSM01 \
				//		 WHERE  LADLE_NO = @tpssm11.LADLE_NO " ;
				//cmd_tpssm12_inq.SetCommandText(sqlstr);
				//cmd_tpssm12_inq.Parameters.Set("tpssm11.LADLE_NO",tpssm11.LADLE_NO);
				//cmd_tpssm12_inq.ExecuteReader();
				//if(cmd_tpssm12_inq.Read())
				//{
				//	v_ladle_level = cmd_tpssm12_inq.GetString(1);
				//}
				//else
				//{
				//	v_ladle_level = " ";
				//}
				//cmd_tpssm12_inq.Close();
				//if(epex.SetValue("ladle_level", fetchRowCount1,v_ladle_level) < 0)
				//{
				//	throw CApplicationException(-1, s.msg, log.Location);				
				//}

				if (fetchRowCount1 == (max_num - 1))
				{
					//发送电文
					if(epex.SendTele() < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);				
					}

					// 释放
					epex.Uninitialize();
					Log::Trace("", __FUNCTION__, "计划成功= [{0}]", tele_num);

					send_flag = 1;
					tele_num ++;
				}
				//给MMS上传状态压值
				inblk.Tables["X200009"].Rows.Add();
				inblk.Tables["X200009"].Rows[fetchRowCount1]["PONO"] = tpssm11.PONO.Trim();
				inblk.Tables["X200009"].Rows[fetchRowCount1]["PONO_STATUS"] = tpssm11.PONO_STATUS;

				

				Log::Trace("", __FUNCTION__, "+1后的 fetchRowCount1== [{0}]", fetchRowCount1);
				Log::Trace("", __FUNCTION__, "tpssm11.pono=[{0}]的压电文结束", tpssm11.PONO);
				
				//inblk.Tables["SLAB"].Rows.Add();
				inblk.Tables["SLAB"].Rows[0]["FACTORY_DIV"] = tpssm11.FACTORY_DIV.Trim();
				inblk.Tables["SLAB"].Rows[0]["PONO"] = tpssm11.PONO.Trim();
				inblk.Tables["SLAB"].Rows[0]["OPER_FLAG"] = oper_flag.Trim();
				
				//下发L2铸坯命令
				Log::Trace("", __FUNCTION__, "调用函数f_pssm_pas1p2_snd开始=[{0}]", inblk.Tables["SLAB"].Rows.get_Count());
				ret = f_pssm_pas1p2_snd(&inblk, bcls_ret, conn);
				if (ret < 0)
				{
					for (int i = 0; i < in_pssm99trace.Tables[0].Rows.get_Count(); i++)
					{
						in_pssm99trace.Tables[0].Rows[i]["VALID_FLAG"] = "0";
						Log::Trace("", __FUNCTION__, "记录失败履历传入块第[{0}]行", i);
					}
					//记录下发计划失败的履历
					ret = 0;
					ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
					if (ret < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					throw CApplicationException(-1, s.msg, log.Location);
				}

				fetchRowCount1 = fetchRowCount1 + 1;
				send_flag = 0;

				//写计划履历表
				tpssm99.EVENT_ID = "M1";//铸坯命令下发L2
				tpssm99.FACTORY_DIV = factory_div;
				tpssm99.PONO = tpssm11.PONO;
				tpssm99.PONO_STATUS = tpssm11.PONO_STATUS;
				tpssm99.VALID_FLAG = "1";//先定义都是成功下发
				
				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
			}
			if (send_flag == 0 && fetchRowCount1 !=max_num )
			{
				Log::Trace("", __FUNCTION__, "计划不满循环数 发送电文");

				/*发送电文*/
				if(epex.SendTele() < 0)
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
			cmd_tpssm11_inq.Close();

		}
		else
		{
			//无计划
			//初始化
			Log::Trace("",__FUNCTION__,"无计划初始化");
			//if (epex.Initialize(lpsz_tc_no) < 0)
			//{
			//	Log::Trace("",__FUNCTION__,"初始化出错.");
			//	throw CApplicationException(-1,s.msg,log.Location);
			//}
			////操作区分
			//if (epex.SetValue("oper_flag", 0, oper_flag.Trim()) < 0)
			//{
			//	Log::Trace("", __FUNCTION__, "初始化oper_flag出错.");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			////总炉数
			//if (epex.SetValue("plan_lot_num", 0, plan_charge_num) < 0)
			//{
			//	Log::Trace("", __FUNCTION__, "初始化plan_lot_num出错.");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			////第几条电文
			//if (epex.SetValue("travel_tc_no", 0, tele_num) < 0)
			//{
			//	Log::Trace("", __FUNCTION__, "初始化travel_tc_no出错.");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			////电文总数
			//if (epex.SetValue("travel_tc_num", 0, d_total_tel_num) < 0)
			//{
			//	Log::Trace("", __FUNCTION__, "初始化travel_tc_num出错.");
			//	strcpy(s.msg, _RES("GCRSS0000015")/*系统出现异常，电文拼接出错，请联系系统维护人员。*/);
			//	strcpy(s.sysmsg, "电文hmjl02拼接失败");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			////本电文中炉数
			//if (epex.SetValue("this_tc_lot_num", 0, fetchRowCount1 + 1) < 0)
			//{
			//	Log::Trace("", __FUNCTION__, "初始化this_tc_lot_num出错.");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
			//if(epex.SendTele() < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);				
			//}

			//// 释放
			//epex.Uninitialize();
			Log::Trace("",__FUNCTION__,"CCCCCCC 000000 send!");

		}

		
		
		////传MMS制造命令状态
		//Log::Trace("", __FUNCTION__, "调用函数f_cm_pam1p2_snd开始=[{0}]",inblk.Tables["X200009"].Rows.get_Count());
		//ret = f_cm_pam1p2_snd(&inblk, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
		//记录下发计划成功的履历
		Log::Trace("", __FUNCTION__, "调用函数f_pssm99_trace记录履历");
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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

