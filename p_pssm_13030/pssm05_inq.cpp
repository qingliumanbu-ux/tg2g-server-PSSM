/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-03-02
功能: 炼钢计划日制造命令编制信息查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


int f_pssm_show_send_charge_des(CString HeatSendFlag,CString HeatChargeFlag,CString & ShowString);

/*<remark >========================================================= 
/// <summary > 
/// 日制造命令编制信息查询
/// <para > 
/// 1.根据传入的炼钢区分、连铸机号、生产日期查询制造命令信息。
/// 2.查询条件：炼钢区分；
/// 3.排序方式：申请日期升序,浇铸批号升序,分割号升序；
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表) / TPSSM03(板坯命令表) </para > 
/// <para > 主调用函数：前台pssm05画面F2(查询)调用。   </para > 
/// </summary > 
/// <param name = "WHOLE_BACKLOG_CODE" > 全程工序代码  </param > 
/// <returns > 日制造命令信息</returns > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssm05_inq);

int f_pssm05_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	int logFlag = 1;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_temp = "";
	CString sqlstr_temp_order = "";
	int TotalRecordCount = 0;

	/*定义业务用变量*/
	CString temp_cast_lot = "";
	CString strFactory_div;
	CString strCc_mach_no;
	CString strBsqf_1;
	CString strBsqf_2;
	CString strBsqf_3;
	CString strBsqf;
	CString strCast_lot_vol;
	CString slab_width_1;
	CString slab_width_2;
	CString strN_ccc;
	CString strN_ccc_1;
	CString strN_ccc_2;
	CString strFlame_clean_1;
	CString strTemp_whole_backlog;
	CDecimal min_slab_width;
	CDecimal max_slab_width;
	CDecimal min_side_press;
	CDecimal max_side_press;
	CDecimal slab_thick_1;
	CDecimal slab_thick_2;
	CString st_no = "";
    CString powder_code = "";
	CString v_pono = "";
	CString v_hot_charge_method = "";
	CString side_press = "";
	CString slab_dest = "";

	//系统的分页类信息。
	CPageInfo pageInfo;
	int strand_num = 0;

	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssm03("TPSSM03");

	CDbCommand cmd_tpssm01_inq(conn);  //与DB 建立连接。

	/******业务处理开始******/
	try 
	{
		try
		{
			//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 1;
			pageInfo.PageSize = 1000;
		}

		/*获取前台输入数据*/
		strFactory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();/*炼钢区分*/
		tpssm01["PLAN_DATE"] = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期
		strCc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();//连铸机号
		////Log::Debug("", __FUNCTION__, "IN:FACTORY_DIV = [{0}]", strFactory_div);
		////Log::Debug("", __FUNCTION__, "IN:PLAN_DATE = [{0}]", tpssm01["PLAN_DATE"].ToString());
		////Log::Debug("", __FUNCTION__, "IN:CC_MACH_NO = [{0}]", strCc_mach_no);

		/*检查连铸机号选择正确否*/
		if (strCc_mach_no.Trim() == "")
		{
			strcpy(s.msg,_RES("PSSMS0000046")/*请输入连铸机号。*/);
			throw CApplicationException(-1,s.msg,log.Location);
		}

		/*建立查询数据SQL语句*/
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr_count = " SELECT COUNT(1) "
				"   FROM TPSSM01 "
				"  WHERE  PONO_STATUS >= 12  AND PONO_STATUS <= 15 "
				;
			sqlstr = " SELECT * "
				"   FROM TPSSM01 "
				"  WHERE  PONO_STATUS >= 12  AND PONO_STATUS <= 15 "
				;

			if (tpssm01["PLAN_DATE"].ToString().Trim() != "")
			{
				sqlstr_temp += " AND PLAN_DATE = @plan_date ";
			}

			if (strFactory_div.Trim() != "")
			{
				sqlstr_temp += " AND FACTORY_DIV = @factory_div ";
			}

			if (strCc_mach_no.Trim() != "")
			{
				sqlstr_temp += " AND CC_MACH_NO = @cc_mach_no ";
			}

			sqlstr_temp_order += " ORDER BY CC_SEQ ASC ";

			sqlstr_count = sqlstr_count + sqlstr_temp;
			sqlstr = sqlstr + sqlstr_temp + sqlstr_temp_order;
			break;
		}

		/*给查询SQL赋条件值*/
		CDbCommand cmd_tpssm01_inq(sqlstr, conn);

		cmd_tpssm01_inq.Parameters.Set("factory_div", strFactory_div);
		cmd_tpssm01_inq.Parameters.Set("plan_date", tpssm01["PLAN_DATE"].ToString());
		cmd_tpssm01_inq.Parameters.Set("cc_mach_no", strCc_mach_no);

		////Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr);
		/*查询满足条件的计划信息*/
		cmd_tpssm01_inq.SetCommandText(sqlstr_count);
		TotalRecordCount = cmd_tpssm01_inq.ExecuteScalar().ToInt32();
		//分页获取
		cmd_tpssm01_inq.SetCommandText(sqlstr);
		cmd_tpssm01_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_tpssm01_inq.Close();

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"HOT_SENDCHARGE_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"N_CCC");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"FLAME_CLEAN_1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_WIDTH");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_THICK");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_WIDTH2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_THICK2");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"CAST_LOT_NO1");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"POWDER_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"LEN_FLAG");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SIDE_PRESS");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"CC_STANDARD");

		for(int i = 0;i < bcls_ret->Tables[0].Rows.get_Count();i++)
		{
			strFactory_div = bcls_ret->Tables[0].Rows[i]["FACTORY_DIV"];/*全程工序代码*/
			st_no = bcls_ret->Tables[0].Rows[i]["ST_NO"].ToString(); /*出钢记号*/
			slab_dest = bcls_ret->Tables[0].Rows[i]["SLAB_DEST"].ToString();/*去向*/
			
   //         //保护渣代码查询

   //         bcls_ret->Tables[0].Rows[i]["POWDER_CODE"] = powder_code;
			//////Log::Debug("", __FUNCTION__ ,"powder_code = [{0}]",powder_code);

			strN_ccc_1 = bcls_ret->Tables[0].Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
			strN_ccc_2 = bcls_ret->Tables[0].Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/

			strCast_lot_vol = bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"];/*CAST_LOT号*/

			/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
			strN_ccc = strN_ccc_1 + " - " + strN_ccc_2;

			bcls_ret->Tables[0].Rows[i]["N_CCC"] = strN_ccc;
			/*CAST - LOT号*/
			bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"] = strCast_lot_vol;

			//限时热装钢种查询20130905update
			//cmd_sm28.SetCommandText(sel_sm28);
			//cmd_sm28.Parameters.Set("st_no",st_no);
			//cmd_sm28.ExecuteReader();
			//if(cmd_sm28.Read())
			//{
			//	v_hot_charge_method = cmd_sm28.GetString(1);
			//	bcls_ret->Tables[0].Rows[i]["HOT_SENDCHARGE_FLAG"] = v_hot_charge_method;
			//}
			//else
			//{
			//	v_hot_charge_method = "";
			//	f_pssm_show_send_charge_des(strBsqf_1,strBsqf_2,strBsqf);
			//	bcls_ret->Tables[0].Rows[i]["HOT_SENDCHARGE_FLAG"] = strBsqf;
			//}
			//cmd_sm28.Close();

			/*根据查询到的浇铸批号查询铸机流中的厚度和宽度*/
			if (strCast_lot_vol != temp_cast_lot)
			{
				sqlstr = " SELECT MAX(SLAB_THICK), "
					     " MAX(SLAB_WIDTH), "
					     " MIN(SLAB_WIDTH), "
						 " MAX(STRAND_NUM) "
					     " FROM TPSSM03 "
					     " WHERE CAST_LOT_NO = @cast_lot_vol "
						 "   AND FACTORY_DIV = @factory_div ";

				sqlstr += "  AND  STRAND_NO = '1' ";

				CDbCommand cmd_inq(sqlstr, conn); 
				cmd_inq.Parameters.Set("cast_lot_vol", strCast_lot_vol);
				cmd_inq.Parameters.Set("factory_div", strFactory_div);
				cmd_inq.ExecuteReader();

				if(cmd_inq.Read())
				{
					slab_thick_1 = cmd_inq.GetInt32(1);
					max_slab_width = cmd_inq.GetInt32(2);
					min_slab_width = cmd_inq.GetInt32(3);
					strand_num = cmd_inq.GetInt32(4);
				}
				cmd_inq.Close();

				slab_width_1 = max_slab_width.ToString() + " - " + min_slab_width.ToString();

				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH"] = slab_width_1;
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK"] = slab_thick_1.ToString();

				if (strand_num == 2)
				{
					sqlstr = " SELECT  MAX(SLAB_THICK),"
						     " MAX(SLAB_WIDTH),"
							 " MIN(SLAB_WIDTH) "
							 " FROM TPSSM03 "
							 " WHERE CAST_LOT_NO = @cast_lot_vol"
							 "   AND FACTORY_DIV = @factory_div ";

					sqlstr += "  AND STRAND_NO = '2'";

					CDbCommand cmd_inq1(sqlstr, conn);
					cmd_inq1.Parameters.Set("cast_lot_vol", strCast_lot_vol);
					cmd_inq1.Parameters.Set("factory_div", strFactory_div);

					cmd_inq1.ExecuteReader();

					if(cmd_inq1.Read())
					{
						slab_thick_2 = cmd_inq1.GetInt32(1);
						max_slab_width = cmd_inq1.GetInt32(2);
						min_slab_width = cmd_inq1.GetInt32(3);
					}

					slab_width_2 = max_slab_width.ToString() + " - " + min_slab_width.ToString();

					bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH2"] = slab_width_2;
					bcls_ret->Tables[0].Rows[i]["SLAB_THICK2"] = slab_thick_2.ToString();

					cmd_inq1.Close();
				}

				temp_cast_lot = strCast_lot_vol;
			}
			else
			{
				/*流宽、流厚、CAST - LOT号置空*/
				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH2"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK2"] = "";
				bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"] = "";
			}

			/*制造命令号*/
			v_pono = bcls_ret->Tables[0].Rows[i]["PONO"];
			CString sql_len = " SELECT MIN(SLAB_MAX_LEN - SLAB_MIN_LEN) "
						      " FROM TPSSM03 "
						      " WHERE PONO = @pono "
							  "	  AND FACTORY_DIV = @factory_div ";
		    sqlstr = sql_len;
			CDbCommand cmd_len(sql_len, conn);
			cmd_len.Parameters.Set("factory_div", strFactory_div);
			cmd_len.Parameters.Set("pono", v_pono);
			cmd_len.ExecuteReader();

			if(cmd_len.Read())
			{
				if(cmd_len.GetDecimal(1) >  0  && cmd_len.GetDecimal(1) <  0.3)//长度之差小于0.3显示"1"
				{
					bcls_ret->Tables[0].Rows[i]["LEN_FLAG"] = "1";
				}
				else
				{
					////Log::Trace("", __FUNCTION__ ,"len_flag2 = [{0}]",cmd_len.GetDecimal(1));
					bcls_ret->Tables[0].Rows[i]["LEN_FLAG"] = "";
				}
			}
			cmd_len.Close();
		}

		bcls_ret->Tables[0].set_TableName("TPSSM01");

		//返回分页总数量信息 
		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = TotalRecordCount;

		/*返回处理信息*/
		if(bcls_ret->Tables[0].Rows.get_Count() == 0)
		{
			strcpy(s.msg,_RES("GCRSS0000013")/*没有满足条件的记录。*/);
		}
		else
		{			
			CFormattable arguments[] = {bcls_ret->Tables[0].Rows.get_Count()}; 
			CMessageFormat::Format(s.msg, _RES("GCRSS0000004")/*查询到[{0}]条记录。*/,	arguments, 1); 			
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


