/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-10-20
功能: 炼钢计划连铸铸开浇时间设定及推算
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"



/*<remark>=========================================================
/// <summary>
/// 连铸制造命令开浇时间设定及推算
/// <para>
/// 1.根据传入的制造命令号、开浇时刻、生产日期计算当日的后续制造命令号开浇时刻。
/// </para>
/// <para>数据库表：TPSSM01(炼钢连铸制造命令炉次表)</para>
/// <para>主调用函数：前台pssm05画面F10(设定时间)调用。   </para>
/// <para>调用函数：f_pssm_compute_time(浇铸时间计算)。   </para>
/// </summary>
/// <param name="ST_NO">主出钢记号  </param>
/// <returns>成功：0</returns>
/// <returns>失败：-1</returns>
===========================================================</remark>*/
/******service入口******/
BM2F_ENTERACE(pssm05_set);


int f_pssm05_set(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */	
	int doFlag = 0;		//返回值
	int logFlag = 1;
	int return_flag = 0;
	int n = 2;//流数

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	CString plan_date = "";
	CString pono = "";
	CString cc_req_time = "";
	CString strCast_lot_no = "";
	CString strPono = "";
	CString strSt_no = "";
	CString strPrev_st_no = "";
	CString strTemp_whole_backlog;
	CString v_factory_div = "";
	CString v_plan_date = "";
	CString v_cc_mach_no = "";
	CDecimal v_cc_seq = 0;
	CDecimal slab_num = 0;
	CDecimal sum_slab_width = 0;
	CDecimal slab_width = 0;
	CDecimal value_md = 0;
	CDecimal value_lz = 0;
	CDecimal value_dj = 0;
	CDecimal v_seq_no = 0;
	CDecimal cast_speed_max = 0;
	CDecimal v_cast_prep_time = 0;
	CDateTime cc_req_time_max;
    CDateTime cc_req_time_nom;
    CDateTime cc_req_times;
	CDateTime cc_req_times_prev;
	CDateTime v_plan_date_prev;
	CDecimal t_element_wk = 0;
	CDecimal pour_times_prev = 0;
	CDecimal pour_times = 0;
	CDecimal slab_thick = 0;
	double pour_time;
	double pour_time_prev;
	CString v_cc_req_time = "";
	CString pono_prev = "";
	CString shift_no = "";
	CString shift_group = "";
	CString v_prev_plan_date = "";

	int sqlcode = 0;
	CString event_status = "";    //事件属性
	CString event_table = "";     //事件表
	CString event_key = "";       //事件关键字

	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm20("TPSSM20");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_code_inq(conn);  //与DB 建立连接。
	CDbCommand cmd_upd(conn);  //与DB 建立连接。

	/****** 业务处理开始 ******/
	try 
	{
		/*获取前台输入数据*/
		plan_date = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期
		pono =  bcls_rec->Tables[0].Rows[0]["PONO"].ToString();
		cc_req_time = bcls_rec->Tables[0].Rows[0]["CC_REQ_TIME"].ToString();
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		////Log::Trace("", __FUNCTION__ ,"pono = [{0}] cc_req_time =[{1}]",pono,cc_req_time);


		//首先选取出传入制造命令号对应的序号
		tpssm01["PONO"] = pono;
		tpssm01["FACTORY_DIV"] = v_factory_div;
		if (tpssm01.Query("PONO, FACTORY_DIV") == false)
		{
			CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令[{0}]不存在。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpssm01["PONO_STATUS"].ToDecimal() != 12) //13-收池
		{
			CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令[{0}]已不在收池状态，不能设置开浇时间。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (tpssm01["RESTRAND_FLG"].ToString() != "T") //1-重开机
		{
			CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "制造命令[{0}]不是LOT中第1炉，不能置重开机标志。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////根据开始时刻(start_time)计算班别、班次		 
		//bcls_ret->SetColVal("SHIFTBLOCK", "target_date", 1, tpssm01.cc_req_time);

		//doFlag = f_epep_get_shift_group("DEFUAL", cc_req_time, tpssm01["SHIFT_NO"].ToString(), tpssm01["SHIFT_GROUP"].ToString(), conn);
		if (doFlag != 0)
		{
			CFormattable arguments[] = { cc_req_time }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, "CC开始时间[{0}]调用函数f_epep_get_shift_group计算班组班别出错。", arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////Log::Trace("", __FUNCTION__ ,"tpssm01["CC_SEQ"] = [{0}]",tpssm01["CC_SEQ"].ToDecimal());
        v_plan_date = tpssm01["PLAN_DATE"];
        v_cc_mach_no = tpssm01["CC_MACH_NO"];
		v_cc_seq = tpssm01["CC_SEQ"];
		v_factory_div = tpssm01["FACTORY_DIV"];

		/*建立查询数据SQL语句*/
		sqlstr = " UPDATE TPSSM01 "
				 " SET  CC_REQ_TIME_FLAG = '1', "
				 " CC_REQ_TIME	= @cc_req_time, "
				 " SHIFT_NO		= @shift_no, "
				 " SHIFT_GROUP	= @shift_group, "
				 " REC_REVISE_TIME	= @rec_revise_time, "
				 " REC_REVISOR		= @rec_revisor "
				 " WHERE PONO = @pono "
				 "   AND FACTORY_DIV = @factory_div";

		/*给查询SQL赋条件值*/
		CDbCommand cmd_upd(sqlstr,conn);
		cmd_upd.Parameters.Set("pono", pono);
		cmd_upd.Parameters.Set("factory_div", v_factory_div);
		cmd_upd.Parameters.Set("cc_req_time", cc_req_time);
		cmd_upd.Parameters.Set("shift_no", tpssm01["SHIFT_NO"].ToString());
		cmd_upd.Parameters.Set("shift_group", tpssm01["SHIFT_GROUP"].ToString());
		cmd_upd.Parameters.Set("rec_revise_time", tpssm01["REC_REVISE_TIME"].ToString());
		cmd_upd.Parameters.Set("rec_revisor", tpssm01["REC_REVISOR"].ToString());

		////Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr);
		cmd_upd.ExecuteNonQuery();
		
		sqlstr = "SELECT * "
			     "  FROM TPSSM20  "
				 " WHERE FACTORY_DIV = @factory_div ";
		CDbCommand cmd_code_inq(sqlstr, conn);
		cmd_code_inq.Parameters.Set("factory_div", v_factory_div);
		cmd_code_inq.ExecuteReader();

		if (cmd_code_inq.Read())
		{
			cmd_inq.Fetch(tpssm20);

			if (tpssm20["ITEM_CODE"].ToString() == "LZ")
			{
				value_lz = tpssm20["ITEM_VALUE_N"];
			}
			else if (tpssm20["ITEM_CODE"].ToString() == "MD")
			{
				value_md = tpssm20["ITEM_VALUE_N"];
			}
			else if (tpssm20["ITEM_CODE"].ToString() == "DJ")
			{
				value_dj = tpssm20["ITEM_VALUE_N"];
			}
		}
		cmd_code_inq.Close();

		/*推算本日计划的开浇时刻*/
		/*建立查询数据SQL语句*/
		sqlstr = " SELECT PONO,ST_NO,CAST_LOT_NO,CC_REQ_TIME_FLAG,CC_REQ_TIME, "
				 " POUR_TIME,SLAB_PLACE_CODE,JOINT_CAST_LOT_NO "	
			     " FROM  TPSSM01 "
				 " WHERE  PONO_STATUS >= 12  "
				 " AND PLAN_DATE = @plan_date "
				 " AND FACTORY_DIV = @factory_div "
				 " AND CC_MACH_NO = @cc_mach_no"
				 " AND CC_SEQ  >= @cc_seq "
                 " ORDER BY CC_SEQ ASC,"
				 " CAST_LOT_NO ASC,"
				 " CAST_LOT_DIV_NO ASC";
		
		/*给查询SQL赋条件值*/
		CDbCommand cmd_inq(sqlstr,conn);
		cmd_inq.Parameters.Set("factory_div", v_factory_div);
		cmd_inq.Parameters.Set("plan_date", v_plan_date);
		cmd_inq.Parameters.Set("cc_mach_no",v_cc_mach_no);
		cmd_inq.Parameters.Set("cc_seq",v_cc_seq);
		cmd_inq.ExecuteReader();
		int fetchRowCount = 0;
		while (cmd_inq.Read())
		{
			tpssm01.Reset();
			tpssm01["PONO"] = cmd_inq.GetString(1);
			tpssm01["ST_NO"] = cmd_inq.GetString(2);
			tpssm01["CAST_LOT_NO"] = cmd_inq.GetString(3);
			tpssm01["CC_REQ_TIME_FLAG"] = cmd_inq.GetString(4);
			tpssm01["CC_REQ_TIME"] = cmd_inq.GetString(5).Trim();
			tpssm01["POUR_TIME"] = cmd_inq.GetInt32(6);
			tpssm01["SLAB_PLACE_CODE"] = cmd_inq.GetString(7).Trim();
			tpssm01["JOINT_CAST_LOT_NO"] = cmd_inq.GetString(8).Trim();

			if (tpssm01["CC_REQ_TIME_FLAG"].ToString() == "1")//指定开浇时刻
			{
				cc_req_times_prev = CDateTime::Parse(tpssm01["CC_REQ_TIME"].ToString());
				pour_times_prev = tpssm01["POUR_TIME"];
				continue;
			}
			else
			{
				if (tpssm01["CC_SEQ"].ToDecimal() == 1)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "本日首炉制造命令[{0}]没有指定开浇时间。", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				
				//对重开机的,再加上开机准备时间
				if (tpssm01["RESTRAND_FLG"].ToString() == 1)
				{	
					//v_cast_prep_time = "";
					//根据连铸机号查询CAST准备时间
					pour_times_prev = pour_times_prev + v_cast_prep_time;
				}
			}

			cc_req_times = cc_req_times_prev.AddMinutes(pour_time_prev);
			v_cc_req_time = cc_req_times.ToString("yyyyMMddHHmmss");

			//doFlag = f_epep_get_shift_group("DEFUAL", v_cc_req_time, tpssm01["SHIFT_NO"].ToString(), tpssm01["SHIFT_GROUP"].ToString(), conn);
			//if (doFlag != 0)
			//{
			//	CFormattable arguments[] = { cc_req_time }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "CC开始时间[{0}]调用函数f_epep_get_shift_group计算班组班别出错。", arguments, 1); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			sqlstr = " UPDATE TPSSM01 "
				"  SET  CC_REQ_TIME = @cc_req_time "
				"       ,SHIFT_NO = @shift_no "
				"       ,SHIFT_GROUP = @shift_group "
				" WHERE PONO = @pono ";
			CDbCommand cmd_upd(sqlstr, conn);
			cmd_upd.Parameters.Set("pono", tpssm01["PONO"].ToString());
			cmd_upd.Parameters.Set("cc_req_time", v_cc_req_time);
			cmd_upd.Parameters.Set("shift_no", tpssm01["SHIFT_NO"].ToString());
			cmd_upd.Parameters.Set("shift_group", tpssm01["SHIFT_GROUP"].ToString());
			cmd_upd.ExecuteNonQuery();

			////Log::Trace("", __FUNCTION__, "cc_req_times111 = [{0}]", cc_req_times.ToString("yyyyMMddHHmmss"));
			////Log::Trace("", __FUNCTION__, "pour_times111 = [{0}]", pour_times);
			
			cc_req_times_prev = cc_req_times;
			pour_times_prev = tpssm01["POUR_TIME"];
			pono_prev = tpssm01["PONO"];
			fetchRowCount++;

		}
		cmd_inq.Close();
	}	

	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
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
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}


