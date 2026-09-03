/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-07-27
功能: 日浇铸顺计划一览表查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm01.h"
#include "tpssm03.h"

/*<remark >========================================================= 
/// <summary > 
/// 日浇铸顺计划一览表查询
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
/// <returns > 失败：-1</returns > =========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssmlgr1_inq);

int f_pssmlgr1_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	int logFlag = 1;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	CString temp_cast_lot = "";
	CString strUnit_code;
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
	CDecimal slab_thick_1;
	CDecimal slab_thick_2;
	CString plan_date;

	/*实体类定义*/
	CTPSSM01 tpssm01(conn);
	CTPSSM03 tpssm03(conn);

	/******业务处理开始******/
	try 
	{
		//  /*分页查询用参数*/
		//   int  nStart = (int)bcls_rec->Tables[0].Rows[0]["START"];//获取查询起始值
		//   int  nPageSize = (int)bcls_rec->Tables[0].Rows[0]["PAGE_SIZE"];//获取页面值
		//   int  RECORD_TOTAL = 0;

		/*获取前台输入数据*/
		plan_date = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期
		strCc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();//连铸机号

		/*建立查询数据SQL语句*/
		sqlstr = " SELECT "
			     " TPSSM01.* , "
				 " SUBSTR(FLAME_CLEAN,1,1) as JQZS "
				 " FROM  TPSSM01 "
				 " WHERE  PONO_STATUS > 1  AND PONO_STATUS <= 7 "
				 " AND CC_MACH_NO = @cc_mach_no "
				 " AND PLAN_DATE >= @plan_date "
				 " ORDER BY CC_SEQ ASC ";

		/*给查询SQL赋条件值*/
		CDbCommand cmd_sql(sqlstr,conn);
		cmd_sql.Parameters.Set("plan_date", plan_date);
		cmd_sql.Parameters.Set("cc_mach_no", strCc_mach_no);

		/*查询满足条件的计划信息*/
		cmd_sql.ExecuteQuery(bcls_ret->Tables[0]);	
		
		bcls_ret->Tables[0].Columns.Add(DT_INT32,"SMELT_MODE");//作为序号使用

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"HOT_SENDCHARGE_FLAG");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"N_CCC");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"FLAME_CLEAN_1");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_WIDTH1");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_THICK1");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_WIDTH2");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SLAB_THICK2");

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"CAST_LOT_NO1");

		for(int i = 0;i < bcls_ret->Tables[0].Rows.get_Count();i++)
		{   
			bcls_ret->Tables[0].Rows[0]["SEND_PLANNER"] = s.userid;
			bcls_ret->Tables[0].Rows[i]["SMELT_MODE"] = i;//序号
			bcls_ret->Tables[0].Rows[0]["REC_CREATE_TIME"] = plan_date;
			strTemp_whole_backlog = bcls_ret->Tables[0].Rows[i]["WHOLE_BACKLOG_CODE"];/*全程工序代码*/
			strN_ccc_1 = bcls_ret->Tables[0].Rows[i]["CAST_LOT_SUM"];/*CAST_LOT内炉数*/
			strN_ccc_2 = bcls_ret->Tables[0].Rows[i]["CAST_LOT_DIV_NO"];/*CAST_LOT分割号*/

			strFlame_clean_1 = bcls_ret->Tables[0].Rows[i]["JQZS"];/*机清指示*/

			strBsqf_1 = bcls_ret->Tables[0].Rows[i]["HEAT_HOT_SEND_FLAG"];/*整炉热送标志*/
			strBsqf_2 = bcls_ret->Tables[0].Rows[i]["HEAT_HOT_CHARGE_FLAG"];/*整炉热装标志*/
			strBsqf_3 = bcls_ret->Tables[0].Rows[i]["HOT_CHARGE_FLAG"];/*合同热装标志*/

			strCast_lot_vol = bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/



			/*连连铸 = CAST_LOT内炉数 + CAST_LOT分割号*/
			strN_ccc = strN_ccc_1 + " - " + strN_ccc_2;

			bcls_ret->Tables[0].Rows[i]["N_CCC"] = strN_ccc;

			/*CAST - LOT号*/
			bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO1"] = strCast_lot_vol;
			bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"] = strCast_lot_vol;

			/*机清区分*/
			bcls_ret->Tables[0].Rows[i]["FLAME_CLEAN_1"] = strFlame_clean_1;

			if (strBsqf_2 == "3" || strBsqf_1 == "3") //整炉混装或整炉混热送
			{
				strBsqf = "CCR3";
			}

			if (strBsqf_2 == "2" && strBsqf_1 == "1")//整炉不热装且整炉热送
			{
				strBsqf = "CCR1";
			}

			if (strBsqf_2 == "2" && strBsqf_1 == "2")//整炉不热装且整炉不热送
			{
				strBsqf = "CCR2";
			}

			if (strBsqf_2 == "1" && strBsqf_3 == "1")//整炉热装保温坑热装
			{
				strBsqf = "HCR";
			}

			if (strBsqf_2 == "1" && strBsqf_3 == "2")//整炉热装直接热装
			{
				strBsqf = "DHCR";
			}

			if (strBsqf_2 == "1" && strBsqf_3 == "3")//整炉热装直接轧制
			{
				strBsqf = "HDR";
			}

			bcls_ret->Tables[0].Rows[i]["HOT_SENDCHARGE_FLAG"] = strBsqf;

			/*根据查询到的浇铸批号查询铸机流中的厚度和宽度*/
			if (strCast_lot_vol != temp_cast_lot)
			{
				sqlstr = " SELECT "
					     " MAX(NOM_SLAB_THICK),"
						 " MAX(NOM_SLAB_WIDTH),"
						 " MIN(NOM_SLAB_WIDTH) "
						 " FROM TPSSM03 "
						 " WHERE CAST_LOT_NO = @cast_lot_vol"  ;

				sqlstr += "  AND  STRAND_NO = '1' ";

				CDbCommand cmd_inq(sqlstr,conn);

				cmd_inq.Parameters.Set("cast_lot_vol" ,strCast_lot_vol);

				cmd_inq.ExecuteReader();

				if(cmd_inq.Read())
				{
					slab_thick_1 = cmd_inq.GetInt32(1);
					max_slab_width = cmd_inq.GetInt32(2);
					min_slab_width = cmd_inq.GetInt32(3);
				}

				slab_width_1 = max_slab_width.ToString() + " - " + min_slab_width.ToString();

				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH1"] = slab_width_1;

				bcls_ret->Tables[0].Rows[i]["SLAB_THICK1"] = slab_thick_1.ToString();

				cmd_inq.Close();

				if ( strTemp_whole_backlog == "A1" ||  strTemp_whole_backlog == "A2" ||
					strTemp_whole_backlog == "A5")
				{
					sqlstr = " SELECT "
						     " MAX(NOM_SLAB_THICK),"
							 " MAX(NOM_SLAB_WIDTH)," 
							 " MIN(NOM_SLAB_WIDTH) "
							 " FROM TPSSM03 "
							 " WHERE CAST_LOT_NO = @cast_lot_vol"  ;

					sqlstr += "  AND STRAND_NO = '2'";

					CDbCommand cmd_inq1(sqlstr,conn);

					cmd_inq1.Parameters.Set("cast_lot_vol" ,strCast_lot_vol);

                    Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr);

					cmd_inq1.ExecuteReader();

					if(cmd_inq1.Read())
					{
						slab_thick_2 = cmd_inq1.GetInt32(1);
						max_slab_width = cmd_inq1.GetInt32(2);
						min_slab_width = cmd_inq1.GetInt32(3);
                        Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr);

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
				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH1"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK1"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_WIDTH2"] = "";
				bcls_ret->Tables[0].Rows[i]["SLAB_THICK2"] = "";
				bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"] = "";
			}
		}

		bcls_ret->Tables[0].set_TableName("TPSSM01");

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


