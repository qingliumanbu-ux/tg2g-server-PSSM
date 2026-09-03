/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-03-08
功能: 炼钢计划开浇时间推算
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm01.h"
#include "tpssm03.h"
#include "tpssm17.h"

/*<remark >========================================================= 
/// <summary > 
/// 制造命令开浇时间推算
/// <para > 
/// 1.根据传入的炼钢区分、连铸机号、生产日期计算开浇时间。
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表) / TPSSM03(板坯命令表) </para > 
/// <para > 主调用函数：前台pssm05画面F5(开浇时间)调用。   </para > 
/// <para > 调用函数：f_pssm_compute_time(浇铸时间计算)。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > =========================================================== </remark > */
/******service入口******/
BM2F_ENTERACE(pssm05_time);

int f_pssm_compute_time(CString unit_code,CString plan_date,CString cc_mach_no,CDbConnection * conn);
int f_pssm05_time(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	int logFlag = 1;
	int return_flag = 0;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	CString temp_cast_lot = "";
	CString strUnit_code;
	CString strCc_mach_no;
	CString strCast_lot_no;
	CString strPlan_date;
	CString strPono;
	CString strSt_no;
	CString strPrev_st_no;
	CString strTemp_whole_backlog;
	CString v_plan_date;
	CString v_unit_code;
	CString v_cc_mach_no;
	CString pono;
	CString cc_req_time;
	CString pour_time;
	CString st_no;
	CString pour_strart_time_l3;
	CDecimal min_slab_width;
	CDecimal max_slab_width;
	CDecimal slab_thick_1;
	CDecimal slab_thick_2;


	/*实体类定义*/
	CTPSSM01 tpssm01(conn);
	CTPSSM17 tpssm17(conn);

	/******业务处理开始******/
	try 
	{
		/*获取前台输入数据*/
		strUnit_code = bcls_rec->Tables[0].Rows[0]["UNIT_CODE"].ToString();/*炼钢区分*/
		strPlan_date = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();//计划日期
		strCc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();//连铸机号
		Log::Debug("", __FUNCTION__ ,"IN:unit_code = [{0}]",strUnit_code);
		/*建立查询数据SQL语句*/
		sqlstr = "SELECT * "
				" FROM  TPSSM01 "
				" WHERE  PONO_STATUS IN(2,3)"
				" AND CAST_LOT_DIV_NO = 01 "
				" AND PLAN_DATE = @plan_date "
				" AND UNIT_CODE = @unit_code "
				" AND CC_MACH_NO = @cc_mach_no"
				" ORDER BY CC_SEQ ASC,CAST_LOT_NO ";

		/*给查询SQL赋条件值*/
		CDbCommand cmd_sql(sqlstr,conn);
		cmd_sql.Parameters.Set("unit_code", strUnit_code);
		cmd_sql.Parameters.Set("plan_date", strPlan_date);
		cmd_sql.Parameters.Set("cc_mach_no", strCc_mach_no);
		Log::Debug("", __FUNCTION__ ,"sqlstr = [{0}]",sqlstr);
		/*查询满足条件的计划信息*/
		cmd_sql.ExecuteQuery(bcls_ret->Tables[0]);	
		cmd_sql.Close();
		/*置板坯位置代码*/
		for(int i = 0;i < bcls_ret->Tables[0].Rows.get_Count();i++)
		{
			//strTemp_whole_backlog = bcls_ret->Tables[0].Rows[i]["WHOLE_BACKLOG_CODE"];/*全程工序代码*/
			strCast_lot_no = bcls_ret->Tables[0].Rows[i]["CAST_LOT_NO"];/*CAST - LOT号*/
			strPono = bcls_ret->Tables[0].Rows[i]["PONO"];/*PONO号*/
			strSt_no = bcls_ret->Tables[0].Rows[i]["ST_NO"];/*CAST_LOT分割号*/

			if (strCast_lot_no != "" && i == 0)
			{
				strPrev_st_no = strSt_no;
			}
			Log::Debug("", __FUNCTION__ ,"strSt_no = [{0}]",strSt_no);
			Log::Debug("", __FUNCTION__ ,"strPrev_st_no = [{0}]",strPrev_st_no);
			Log::Debug("", __FUNCTION__ ,"strCast_lot_no = [{0}]",strCast_lot_no);
			Log::Debug("", __FUNCTION__ ,"strPono = [{0}]",strPono);

			if (strCast_lot_no != "" && i > 0)
			{
				if (strSt_no == strPrev_st_no)
				{
					tpssm01.SLAB_PLACE_CODE = "";
				}
				else
				{
					Log::Debug("", __FUNCTION__ ,"strSt_no1 = [{0}]",strSt_no);
					Log::Debug("", __FUNCTION__ ,"strPrev_st_no1 = [{0}]",strPrev_st_no);
					tpssm17.Reset();
					sqlstr = " SELECT * "
						" FROM  TPSSM17 "
						" WHERE  ST_NO = @st_no "
						"  AND   ST_NO1 = @st_no1";
					CDbCommand cmd_inq(sqlstr,conn);
					cmd_inq.Parameters.Set("st_no", strSt_no);
					cmd_inq.Parameters.Set("st_no1", strPrev_st_no);
					cmd_inq.ExecuteReader();
					if(cmd_inq.Read())
					{
						cmd_inq.Fetch(tpssm17);
						Log::Debug("", __FUNCTION__ ,"DECI_FLAG = [{0}]",tpssm17.DECI_FLAG);
						Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE0 = [{0}]",tpssm01.SLAB_PLACE_CODE);
						if 	(tpssm17.DECI_FLAG == "1")
						{
							tpssm01.SLAB_PLACE_CODE = "A";
							Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE1 = [{0}]",tpssm01.SLAB_PLACE_CODE);
						}
						else if (tpssm17.DECI_FLAG == "2" ||tpssm17.DECI_FLAG == "3" )
						{
							tpssm01.SLAB_PLACE_CODE = "B";
						Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE2 = [{0}]",tpssm01.SLAB_PLACE_CODE);
						}					
						else if (tpssm17.DECI_FLAG == "4" && tpssm17.ST_NO2 == "YY000000" )
						{
							tpssm01.SLAB_PLACE_CODE = "D";
						Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE3 = [{0}]",tpssm01.SLAB_PLACE_CODE);
						}					
						else if (tpssm17.DECI_FLAG == "4" && (tpssm17.ST_NO2 == "AN1111C5"||
							tpssm17.ST_NO2 == "AN1122Z1" || tpssm17.ST_NO2 == "AN1122Z2" ||
							tpssm17.ST_NO2 == "AN1122Z3" || tpssm17.ST_NO2 == "AN2222Z1" || 
							tpssm17.ST_NO2 == "AN2233Z1" || tpssm17.ST_NO2 == "AN4444Z1" || 
							tpssm17.ST_NO2 == "AN6666Z1" || tpssm17.ST_NO2 == "AN7777Z1" || 
							tpssm17.ST_NO2 == "AN8888Z1" || tpssm17.ST_NO2 == "AP1119E5" || 
							tpssm17.ST_NO2 == "AQ1111E1" || tpssm17.ST_NO2 == "AQ2222H5" || 
							tpssm17.ST_NO2 == "AQ3333H5" || tpssm17.ST_NO2 == "AQ4444H5" || 
							tpssm17.ST_NO2 == "AQ5555H5" ))
						{
							tpssm01.SLAB_PLACE_CODE = "E";
							Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE4 = [{0}]",tpssm01.SLAB_PLACE_CODE);
						}
						else
						{
							tpssm01.SLAB_PLACE_CODE = "C";
							Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE5 = [{0}]",tpssm01.SLAB_PLACE_CODE);
						}
					Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE6 = [{0}]",tpssm01.SLAB_PLACE_CODE);
					}
					else
					{
						tpssm01.SLAB_PLACE_CODE = "F";
					}
					Log::Debug("", __FUNCTION__ ,"SLAB_PLACE_CODE7 = [{0}]",tpssm01.SLAB_PLACE_CODE);
				}
				sqlstr = " UPDATE  TPSSM01 "
					" SET  SLAB_PLACE_CODE = @slab_place_code "
					" WHERE  PONO = @pono ";
				CDbCommand cmd_upd(sqlstr,conn);
				cmd_upd.Parameters.Set("slab_place_code", tpssm01.SLAB_PLACE_CODE);
				cmd_upd.Parameters.Set("pono", strPono);
				cmd_upd.ExecuteNonQuery();
				strPrev_st_no = strSt_no;
			}	
		}
		/*计算开浇时间*/
		v_unit_code = strUnit_code;
		v_plan_date = strPlan_date;
		v_cc_mach_no = strCc_mach_no;
		return_flag = f_pssm_compute_time(v_unit_code,v_plan_date,v_cc_mach_no,conn);
		if  (return_flag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		strcpy(s.msg, "开浇时间计算完成!");
		//_RES("PSSMS0000205")/*开浇时间计算完成!*/
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


