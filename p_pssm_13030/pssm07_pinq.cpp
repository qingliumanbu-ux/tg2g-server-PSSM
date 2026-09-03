/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 涂献计
日期: 2012-04-011
功能: 制造命令下的炉次命令查询查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


int f_pssm_show_send_charge_des(CString HeatSendFlag,CString HeatChargeFlag,CString & ShowString);

/*<remark >========================================================= 
/// <summary > 
/// 连铸预计划调整确定查询
/// <para > 
/// 1.根据传入的炼钢区分、连铸机号查询制造命令信息和连铸制造命令铸机流信息你。
/// 2.查询条件：炼钢区分、连铸机号；
/// 3.排序方式：申请日期升序,浇铸批号升序,分割号升序；
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表) / TPSSM03(板坯命令表) </para > 
/// <para > 主调用函数：前台pssm07画面F2(查询)调用。   </para > 
/// </summary > 
/// <param name = "WHOLE_BACKLOG_CODE" > 全程工序代码  </param > 
/// <returns > 连铸预计划调整确定信息</returns > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssm07_pinq);

int f_pssm07_pinq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	int logFlag = 1;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString sql_where = "";
	CString sql_count = "";
	/*定义业务用变量*/
	CString temp_cast_lot = "";
	CString factory_div;
	CString cc_mach_no;
	CString plan_date1;
	CString plan_date2;
	CString steel_app_date1;
	CString steel_app_date2;
	CString strBsqf;
	CString strBsqf_1;
	CString strBsqf_2;
	CString strBsqf_3;
	CString strCast_lot_vol;
	CString slab_width_1;
	CString slab_width_2;
	CString cc_div;
	CString cast_sum;
	CString check_flag;
	CString cast_div_no;
	CString strFlame_clean_1;
	CString strTemp_whole_backlog;
	CDecimal min_slab_width;
	CDecimal max_slab_width;
	CDecimal slab_thick_1;
	CDecimal slab_thick_2;


	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm03("TPSSM03");

	/******业务处理开始******/
	try 
	{
		//分页查询用参数
		int  nStart = (int)bcls_rec->Tables[0].Rows[0]["START"];//获取查询起始值
		int  nPageSize = (int)bcls_rec->Tables[0].Rows[0]["PAGE_SIZE"];//获取页面值
		int  RECORD_TOTAL = 0;

		/*获取前台输入数据*/
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();/*炼钢区分*/
		cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();//连铸机号
		plan_date1 = bcls_rec->Tables[0].Rows[0]["PLAN_DATE1"].ToString();
		plan_date2 = bcls_rec->Tables[0].Rows[0]["PLAN_DATE2"].ToString();
		steel_app_date1 = bcls_rec->Tables[0].Rows[0]["STEEL_APP_DATE1"].ToString();
		steel_app_date2 = bcls_rec->Tables[0].Rows[0]["STEEL_APP_DATE2"].ToString();
        check_flag=bcls_rec->Tables[0].Rows[0]["CHECK_FLAG"].ToString();

		Log::Info("", __FUNCTION__, "factory_div = [{0}]",factory_div);
		Log::Info("", __FUNCTION__, "START = [{0}]", nStart);
		Log::Info("", __FUNCTION__, "PAGE_SIZE = [{0}]", nPageSize);
		Log::Info("", __FUNCTION__, "CHECK_FLAG = [{0}]", check_flag);


		if (bcls_rec->Tables[0].Columns.Contains("CAST_LOT_NO"))
		{
			tpssm01["CAST_LOT_NO"] = bcls_rec->Tables[0].Rows[0]["CAST_LOT_NO"].ToString();
		}
		if (bcls_rec->Tables[0].Columns.Contains("PONO"))
		{
			tpssm01["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO"].ToString();
		}

		if (plan_date1.Trim()!="")
		{
			plan_date1 = plan_date1.Substring(0, 8);
		}
		if (plan_date2.Trim() != "")
		{
			plan_date2 = plan_date2.Substring(0, 8);
		}
		if (steel_app_date1.Trim() != "")
		{
			steel_app_date1 = steel_app_date1.Substring(0, 8);
		}
		if (steel_app_date2.Trim() != "")
		{
			steel_app_date2 = steel_app_date2.Substring(0, 8);
		}

		////Log::Debug("", __FUNCTION__, "IN:factory_div = [{0}]", factory_div);
		////Log::Debug(logFlag,1,"IN:cc_mach_no = [{0}]",cc_mach_no);	
		////Log::Debug(logFlag,1,"IN:plan_date1 = [{0}]",plan_date1);
		////Log::Debug(logFlag,1,"IN:plan_date2 = [{0}]",plan_date2);
		////Log::Debug(logFlag,1,"IN:check_flag = [{0}]",check_flag);
		////Log::Debug(logFlag,1,"IN:steel_app_date1 = [{0}]",steel_app_date1);
		////Log::Debug(logFlag,1,"IN:steel_app_date2 = [{0}]",steel_app_date2);
		////Log::Debug(logFlag,1,"IN:tpssm01["CAST_LOT_NO"] = [{0}]",tpssm01["CAST_LOT_NO"].ToString());
		////Log::Debug(logFlag,1,"IN:tpssm01["PONO"] = [{0}]",tpssm01["PONO"].ToString());



        if(check_flag == "1")
		{
		sqlstr = " SELECT T1.* "
			     " FROM  TPSSM01 T1 "
			     " WHERE 1 = 1  ";
		sql_count = " SELECT "
			        " COUNT(PONO) "
					" FROM TPSSM01 "
					" WHERE 1=1  ";
		}
		else
		{
			sqlstr = " SELECT T1.* "
			     " FROM  HPSSM01 T1 "
			     " WHERE 1 = 1  ";
			sql_count = " SELECT "
			        " COUNT(PONO) "
					" FROM HPSSM01 "
					" WHERE 1=1  ";
		}
		
		if (factory_div.GetLength() > 0)
		{
			sql_where += " AND FACTORY_DIV = @factory_div";
		}
		if(cc_mach_no.GetLength() > 0)
		{
			sql_where += " AND CC_MACH_NO = @cc_mach_no";
		}
		if(plan_date1.GetLength() > 0)
		{
			sql_where += " AND PLAN_DATE >= @plan_date1";
		}
		if ( plan_date2.GetLength() > 0)
		{
			sql_where += " AND PLAN_DATE <= @plan_date2";
		}
		if(steel_app_date1.GetLength() > 0)
		{
			sql_where += " AND STEEL_APP_DATE >= @steel_app_date1";
		}
		if (steel_app_date2.GetLength() > 0)
		{
			sql_where += " AND STEEL_APP_DATE <= @steel_app_date2";
		}
		if (tpssm01["CAST_LOT_NO"].ToString().GetLength() > 0)
		{
			sql_where += " AND CAST_LOT_NO = @cast_lot_no";
		}
		if (tpssm01["PONO"].ToString().GetLength() > 0)
		{
			sql_where += " AND PONO = @pono";
		}
		sql_count += sql_where;
		////Log::Debug("", __FUNCTION__ ,"开始进行总数查询 ");
		////Log::Debug("", __FUNCTION__ ,"count_sql = [{0}]", sql_count);

		Log::Info("", __FUNCTION__, "plan_date1 = [{0}]", plan_date1);
		Log::Info("", __FUNCTION__, "plan_date2 = [{0}]", plan_date2);
		Log::Info("", __FUNCTION__, "steel_app_date1 = [{0}]", steel_app_date1);
		Log::Info("", __FUNCTION__, "steel_app_date2 = [{0}]", steel_app_date2);


		CDbCommand cmd_count(sql_count,conn);
		cmd_count.Parameters.Set("factory_div", factory_div);
		cmd_count.Parameters.Set("cc_mach_no", cc_mach_no);
		cmd_count.Parameters.Set("plan_date1", plan_date1);
		cmd_count.Parameters.Set("plan_date2", plan_date2);
		cmd_count.Parameters.Set("steel_app_date1", steel_app_date1);
		cmd_count.Parameters.Set("steel_app_date2", steel_app_date2);
		cmd_count.Parameters.Set("cast_lot_no", tpssm01["CAST_LOT_NO"].ToString());
		cmd_count.Parameters.Set("pono", tpssm01["PONO"].ToString());
		CDecimal rc = cmd_count.ExecuteScalar();
		bcls_ret->ExtendedProperties.Add("RECORD_TOTAL",rc.ToString());
		////Log::Debug("", __FUNCTION__ ,"总数为[{0}]", rc.ToString());

		////Log::Debug("", __FUNCTION__ ,"开始进行明细查询 ");
		sqlstr += sql_where + " ORDER BY PLAN_DATE,CC_MACH_NO,CC_SEQ,PONO";
		/*给查询SQL赋条件值*/
		CDbCommand cmd_sql(sqlstr,conn);
		cmd_sql.Parameters.Set("factory_div", factory_div);
		cmd_sql.Parameters.Set("cc_mach_no", cc_mach_no);
		cmd_sql.Parameters.Set("plan_date1", plan_date1);
		cmd_sql.Parameters.Set("plan_date2", plan_date2);
		cmd_sql.Parameters.Set("steel_app_date1", steel_app_date1);
		cmd_sql.Parameters.Set("steel_app_date2", steel_app_date2);
		cmd_sql.Parameters.Set("cast_lot_no", tpssm01["CAST_LOT_NO"].ToString());
		cmd_sql.Parameters.Set("pono", tpssm01["PONO"].ToString());
		////Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr );
		/*查询满足条件的计划信息*/
		cmd_sql.ExecuteQuery(bcls_ret->Tables[0],nStart,nPageSize);	
		
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"CC_DIV");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_THICK");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_WIDTH");

		for(int i = 0;i < bcls_ret->Tables[0].Rows.get_Count();i++)
		{
			cast_sum = bcls_ret->Tables[0].Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
			cast_div_no = bcls_ret->Tables[0].Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/
			strCast_lot_vol = bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/
			////Log::Debug("", __FUNCTION__ ,"strCast_lot_vol = [{0}]",strCast_lot_vol );
			/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
			cc_div = cast_sum + " - " + cast_div_no;

			bcls_ret->Tables[0].Rows[i]["CC_DIV"] = cc_div;
			
			/*根据查询到的浇铸批号查询铸机流中的厚度和宽度*/
			if (strCast_lot_vol != temp_cast_lot)
			{
				sqlstr = " SELECT "
					     " MAX(SLAB_THICK),"
						 " MAX(SLAB_WIDTH),"
						 " MIN(SLAB_WIDTH) "
						 " FROM TPSSM03 "
						 " WHERE CAST_LOT_NO = @cast_lot_vol"  ;

				CDbCommand cmd_inq(sqlstr,conn);
				cmd_inq.Parameters.Set("cast_lot_vol" ,strCast_lot_vol);
				cmd_inq.ExecuteReader();
				if(cmd_inq.Read())
				{
					slab_thick_1 = cmd_inq.GetInt32(1);
					max_slab_width = cmd_inq.GetInt32(2);
					min_slab_width = cmd_inq.GetInt32(3);
				}
				slab_width_1 = min_slab_width.ToString() + " - " + max_slab_width.ToString();
				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH"] = slab_width_1;
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK"] = slab_thick_1.ToString();
				cmd_inq.Close();
				temp_cast_lot = strCast_lot_vol;
			}
			else
			{
				/*流宽、流厚、CAST - LOT号置空*/
				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK"] = "";
				bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"] = "";
			}
		}

		bcls_ret->Tables[0].set_TableName("TPSSM01");

		/*返回处理信息*/
		if(bcls_ret->Tables[0].Rows.get_Count() == 0)
		{
			strcpy(s.msg,_RES("GCRSS0000013")/*没有满足条件的记录。*/);
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

	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}


