/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-03-09
功能: 对传入的PONO进行浇铸时间计算
修改历史:
	日期:________;修改人:________; 需求提出人:________
	变更内容:	
**************************************************/
#include "stdafx.h"

#include "tpssm02.h"
#include "tpssm03.h"
#include "tpssm20.h"
#include "tqmtms2.h"

//#include "Log.h"
/*<remark>=========================================================
/// <summary >
///  开浇时间计算函数
/// <para >
///   1.根据传入的PONO号计算浇铸时间。
/// </para >
/// <para > 数据库表：TPSSM01(制造命令炉次表) / TPSSM03(板坯命令表) </para >
/// <para > 主调用函数：后台server:pssm05_time调用。</para >
/// </summary >
/// <param name = "PONO" > 制造命令号 </param >
/// <returns > 浇铸时间、CC要求时刻</returns >
/// <returns > 成功：0</returns >
/// <returns > 失败：-1</returns>
===========================================================</remark>*/
int f_pssm_compute_class(CString &shift_no,CString cc_req_time,CString whole_backlog_code,CDbConnection * conn);
int f_pssm_compute_time(CString unit_code,CString plan_date,CString cc_mach_no,CDbConnection * conn)
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int logFlag = 1;
	int doFlag = 0;
	int n = 2;//流数
	CString v_unit_code = ""; //机组号
	CString v_plan_date = ""; //计划日期
	CString v_cc_mach_no = "";//连铸机号
	CDecimal slab_num;
	CDecimal sum_slab_width;
	CDecimal slab_width;
	CDecimal value_bz ; 
	CDecimal value_lz;
	CDecimal value_dj;
	CDecimal v_seq_no;
	CDecimal cast_speed_max;
	CDateTime cc_req_time_max;
    CDateTime cc_req_time_nom;
    CDateTime cc_req_times;
	CDateTime cc_req_times_prev;
	CDateTime v_plan_date_prev;
	CDecimal t_element_wk;
	CDecimal pour_times_prev;
	CDecimal pour_times;
	CDecimal slab_thick;
	double pour_time;
	double pour_time_prev;
    CString v_cc_req_time;
	CString pono_prev;
	CString shift_no;
	CString temp_cc_req_time;
	CString v_prev_plan_date;
	CString v_whole_backlog_code = "";

 
	int sqlcode = 0;
	CString event_status = "";    //事件属性
	CString event_table = "";     //事件表
	CString event_key = "";       //事件关键字

	CString sqlstr("");
	
	/*实体类定义*/
	CModel tpssm01("TPSSM01");
	CModel tpssmerrm("TPSSMERRM");
   
	/*获取前一天日期*/
	//CString v_prev_plan_date = CDateTime::Now().AddDays(-1).ToString("yyyyMMdd");	//取前一天日期

	try
	{

		CString v_unit_code = unit_code;//得到传来的机组号
		CString v_plan_date = plan_date;//得到传来的计划日期
		CString v_cc_mach_no = cc_mach_no;//得到传来的连铸机号
		v_plan_date_prev = CDateTime::Parse(v_plan_date);
		v_prev_plan_date = v_plan_date_prev.AddDays(-1).ToString("yyyyMMdd");	//取生产日期前一天日期
        ////Log::Trace("",__FUNCTION__,"IN:unit_code = [{0}]",v_unit_code);
		//查询连铸转炉产量、比重
		if  (v_unit_code == "S001" ||v_unit_code == "S005")
		{
			v_seq_no = 1;
			v_whole_backlog_code = "A1";
		}
		if  (v_unit_code == "S002")
		{
			v_seq_no = 2;
			v_whole_backlog_code = "A2";
		}
		if  (v_unit_code == "S015")
		{
			v_seq_no = 3;
			n = 1;
		}
		
		sqlstr = "SELECT VALUE "
			     "  FROM TPSSM20 "
		         " WHERE SEQ_NO = @seq_no ";
		CDbCommand cmd_inq1(sqlstr,conn);
		cmd_inq1.Parameters.Set("seq_no",v_seq_no);
		event_status = "查询连铸转炉产量";   
		event_table = "TPSSM20";      
		event_key = v_seq_no.ToString();       
		cmd_inq1.ExecuteReader();
		if (cmd_inq1.Read())
		{
			value_lz = cmd_inq1.GetDecimal(1);
			////Log::Trace("",__FUNCTION__,"value_lz = [{0}]",value_lz.ToDouble());
		}
		cmd_inq1.Close();	

		v_seq_no = 4 ;
		CDbCommand cmd_inq2(sqlstr,conn);
		cmd_inq2.Parameters.Set("seq_no",v_seq_no);
		event_status = "查询钢水比重";   
		event_table = "TPSSM20";      
		event_key = v_seq_no.ToString();       
		cmd_inq2.ExecuteReader();
		if (cmd_inq2.Read())
		{
			value_bz = cmd_inq2.GetDecimal(1);
		}
		cmd_inq2.Close();			

		v_seq_no = 5 ;
		CDbCommand cmd_inq6(sqlstr,conn);
		cmd_inq6.Parameters.Set("seq_no",v_seq_no);
		event_status = "查询断浇时间";   
		event_table = "TPSSM20";      
		event_key = v_seq_no.ToString();       
		cmd_inq6.ExecuteReader();
		if (cmd_inq6.Read())
		{
		value_dj = cmd_inq6.GetDecimal(1);
		}
		cmd_inq6.Close();	
		////Log::Trace("",__FUNCTION__,"value_dj = [{0}]",value_dj.ToDouble());

		/*建立查询数据SQL语句*/
		sqlstr = " SELECT PONO,ST_NO,CAST_LOT_NO,POUR_START_TIME_L3,CC_REQ_TIME, "
				 " SLAB_PLACE_CODE,JOINT_CAST_LOT_NO,POUR_TIME,CC_REQ_TIME_FLAG "	
			     " FROM  TPSSM01 "
				 " WHERE  PONO_STATUS > 1  "
				 " AND PLAN_DATE = @plan_date "
				 " AND UNIT_CODE = @unit_code "
				 " AND CC_MACH_NO = @cc_mach_no"
                 " ORDER BY PONO_STATUS DESC,"
				 " CC_SEQ ASC,CAST_LOT_NO ASC,"
				 " CAST_LOT_DIV_NO ASC";
		
		/*给查询SQL赋条件值*/
		////Log::Trace("",__FUNCTION__,"IN:unit_code = [{0}]",v_unit_code);
		CDbCommand cmd_inq(sqlstr,conn);
		cmd_inq.Parameters.Set("unit_code", v_unit_code);
		cmd_inq.Parameters.Set("plan_date", v_plan_date);
		cmd_inq.Parameters.Set("cc_mach_no",v_cc_mach_no);
		cmd_inq.ExecuteReader();
		int fetchRowCount = 0;
		while (cmd_inq.Read())
		{
			tpssm01.Reset();
			tpssm01["PONO"] = cmd_inq.GetString(1);
			tpssm01["ST_NO"] = cmd_inq.GetString(2);
			tpssm01["CAST_LOT_NO"] = cmd_inq.GetString(3);
			tpssm01["POUR_START_TIME_L3"] = cmd_inq.GetString(4).Trim();
			tpssm01["CC_REQ_TIME"] = cmd_inq.GetString(5).Trim();
			tpssm01["SLAB_PLACE_CODE"] = cmd_inq.GetString(6).Trim();
			tpssm01["JOINT_CAST_LOT_NO"] = cmd_inq.GetString(7).Trim();
			tpssm01["POUR_TIME"] = cmd_inq.GetInt32(8);
			tpssm01["CC_REQ_TIME_FLAG"] = cmd_inq.GetString(9);
			////Log::Trace("",__FUNCTION__,
				"pour_start_time_l3 = [{0}]",tpssm01.POUR_START_TIME_L3);
			if (tpssm01["POUR_START_TIME_L3"].ToString().Trim() != "" )
			{
			  pour_times = tpssm01["POUR_TIME"];
			  pour_time = pour_times.ToDouble();
			  v_cc_req_time = tpssm01["POUR_START_TIME_L3"];
			  cc_req_times = CDateTime::Parse(v_cc_req_time);
			  ////Log::Trace("",__FUNCTION__,"pour_time1 = [{0}]",pour_time);
			}
			else
			{
				//第一步查询最大浇铸速度//
				sqlstr = "SELECT CAST_SPEED_MAX "
						 "  FROM TQMTMS2 "
						 " WHERE ST_NO = @st_no " 
						 " AND LINE_DIF = '1' " ;
				CDbCommand cmd_inq3(sqlstr,conn);
				cmd_inq3.Parameters.Set("st_no",tpssm01["ST_NO"].ToString());
				event_status = "查询连铸作业表取浇铸速度";   
				event_table = "TQMTMS2";      
				event_key = tpssm01["ST_NO"];       
				cmd_inq3.ExecuteReader();
				if(cmd_inq3.Read())
				{
					cast_speed_max = cmd_inq3.GetInt32(1);
					//浇铸速度=表中的浇铸速度减0.4
					cast_speed_max = cast_speed_max / 10 ; 
					////Log::Trace("",__FUNCTION__,"cast_speed_max1 = [{0}]",cast_speed_max);
					cast_speed_max = cast_speed_max - 0.4 ; 
					////Log::Trace("",__FUNCTION__,"cast_speed_max2 = [{0}]",cast_speed_max);
				}
				else
				{
					cast_speed_max = 0.98;
					////Log::Trace("",__FUNCTION__,"cast_speed_max = [{0}]",cast_speed_max.ToDouble());
				}		
				cmd_inq3.Close();

				//第二步查询厚度//
				sqlstr = "SELECT SLAB_THICK "
						 "  FROM TPSSM02 "
						 " WHERE CAST_LOT_NO = @cast_lot_no";
				CDbCommand cmd_inqa(sqlstr,conn);
				cmd_inqa.Parameters.Set("cast_lot_no",tpssm01["CAST_LOT_NO"].ToString());
				event_status = "查询连铸计划LOT表取板坯厚度";   
				event_table = "TPSSM02";      
				event_key = tpssm01["CAST_LOT_NO"];       
				cmd_inqa.ExecuteReader();
				if(cmd_inqa.Read())
				{
					slab_thick = cmd_inqa.GetDecimal(1);
				}			
				cmd_inqa.Close();

				//第三步计算加权平均宽度
				sqlstr = "SELECT COUNT(*) ,SUM(NOM_SLAB_WIDTH),SUM(NOM_SLAB_WT) "
						 "  FROM TPSSM03 "
						 " WHERE PONO = @pono ";
				CDbCommand cmd_inq4(sqlstr,conn);
				cmd_inq4.Parameters.Set("pono",tpssm01["PONO"].ToString());
				event_status = "查询连铸计划板坯表取块数和宽度";   
				event_table = "TPSSM03";      
				event_key = tpssm01["PONO"];       
				cmd_inq4.ExecuteReader();
				////Log::Trace("",__FUNCTION__,"pono = [{0}]",tpssm01["PONO"].ToString());
				if(cmd_inq4.Read())
				{
				  slab_num = cmd_inq4.GetInt32(1);
				  ////Log::Trace("",__FUNCTION__,
					  "slab_num = [{0}]",slab_num.ToString());
				  sum_slab_width = cmd_inq4.GetInt32(2);
				  slab_width = sum_slab_width / slab_num;
				  ////Log::Trace("",__FUNCTION__,
					  "sum_slab_width = [{0}]",sum_slab_width.ToString());
				  cmd_inq4.Close();
				 }
				else
				{
					sqlstr = "SELECT SLAB_WIDTH FROM TPSSM04  "
							 " WHERE CAST_LOT_NO = @cast_lot_no "
							 "   AND STRAND_NO = '1' ";
					CDbCommand cmd_inq9(sqlstr,conn);
					cmd_inq9.Parameters.Set("cast_lot_no",tpssm01["CAST_LOT_NO"].ToString());
					event_status = "查询连铸计划铸机流表取宽度";   
					event_table = "TPSSM04";      
					event_key = tpssm01["CAST_LOT_NO"];       
					cmd_inq9.ExecuteReader();
					////Log::Trace("",__FUNCTION__,
						"cast_lot_no = [{0}]",tpssm01.CAST_LOT_NO);
					if(cmd_inq9.Read())
					{	
						slab_width = cmd_inq9.GetDecimal(1);
						cmd_inq9.Close();
					}
				}
				////Log::Trace("",__FUNCTION__,"slab_width = [{0}]",slab_width.ToDouble() );
			    //计算浇铸时间:单位分钟
				t_element_wk = value_lz * 1000 * 1000 / (slab_thick * value_bz * cast_speed_max * slab_width * n);
				//计算浇铸时间：时分
				pour_times = t_element_wk.Floor();
				pour_time = pour_times.ToDouble();
			    ////Log::Trace("",__FUNCTION__,"pour_time = [{0}]",pour_time);
				//计划日期下第一炉的开浇时间
				if ( fetchRowCount == 0 && tpssm01["CC_REQ_TIME_FLAG"].ToString().Trim() == "")
				{	
					////Log::Trace("",__FUNCTION__,	"第一炉浇注时间开始",tpssm01["PONO"].ToString());
					////Log::Trace("",__FUNCTION__,"prev_plan_date = [{0}]",v_prev_plan_date);
					//读取同连铸机号前一天的最晚开浇时间
					sqlstr = "SELECT MAX(CC_REQ_TIME) "
							 "  FROM TPSSM01 "
							 " WHERE PLAN_DATE = @plan_date "
							 "  AND  UNIT_CODE = @unit_code "
							 "  AND  CC_MACH_NO = @cc_mach_no";
					CDbCommand cmd_inq5(sqlstr,conn);
					cmd_inq5.Parameters.Set("plan_date",v_prev_plan_date);
					cmd_inq5.Parameters.Set("unit_code", v_unit_code);
					cmd_inq5.Parameters.Set("cc_mach_no",v_cc_mach_no);
					event_status = "查询前一天的最晚开浇时间";   
					event_table = "TPSSM01";      
					event_key = v_prev_plan_date;       
					cmd_inq5.ExecuteReader();
					if (cmd_inq5.Read())
					{
						temp_cc_req_time = cmd_inq5.GetString(1);
						////Log::Trace("",__FUNCTION__,"temp_cc_req_time = [{0}]",temp_cc_req_time);
						if (temp_cc_req_time.Trim() != "")
						{
							cc_req_time_max = CDateTime::Parse(temp_cc_req_time);
						}
					 } 
						cc_req_time_nom = CDateTime::Parse(v_prev_plan_date + "200000");
					//最晚开浇时间大于20点，则CC要求时刻为最晚开浇时刻 + 炉浇注时间
					int k = cc_req_time_max.CompareTo(cc_req_time_nom);
					if (k > 0)
					{
						cc_req_times = cc_req_time_max.AddMinutes(pour_time);
					 }
					   //最晚开浇时间小于20点，则CC要求时刻为生产日期下的22点				 
					else
					{
						cc_req_times = CDateTime::Parse(v_prev_plan_date + "220000" );
						////Log::Trace("",__FUNCTION__,
							"cc_req_times = [{0}]",cc_req_times.ToString("yyyyMMddHHmmss"));
					 }
				}
				//其它炉的开浇时间
				else if(  tpssm01["CC_REQ_TIME_FLAG"].ToString().Trim() == "*")
				{
					////Log::Trace("",__FUNCTION__,	"CC_REQ_TIME_FLAG=[{0}]",tpssm01["CC_REQ_TIME_FLAG"].ToString());

					////Log::Trace("",__FUNCTION__,	"浇注时间是设定的时间",tpssm01["CC_REQ_TIME_FLAG"].ToString());
					v_cc_req_time = tpssm01["CC_REQ_TIME"];
					cc_req_times = CDateTime::Parse(v_cc_req_time);
					////Log::Trace("",__FUNCTION__,"pour_time2 = [{0}]",pour_time);
				}
				else if(fetchRowCount >  0)
				{
					////Log::Trace("",__FUNCTION__,	"推算浇注时间开始",tpssm01["PONO"].ToString());
				  //如果是断浇，则前一炉的炉浇注时间 = 前一炉的炉浇注时间 + 断浇时间；
					////Log::Trace("",__FUNCTION__,"slab_place_code1 = [{0}]",tpssm01["SLAB_PLACE_CODE"].ToString());
					pour_time_prev = pour_times_prev.ToDouble();
					if (tpssm01["SLAB_PLACE_CODE"].ToString() == "F" &&
						tpssm01["JOINT_CAST_LOT_NO"].Trim() == "")
					{
					    pour_times_prev = pour_times_prev + value_dj ;
						pour_time_prev = pour_times_prev.ToDouble();
						sqlstr = " UPDATE TPSSM01 "
								 "  SET POUR_TIME = @pour_time"
								 "  WHERE PONO = @pono ";
						CDbCommand cmd_inq8(sqlstr,conn);
						cmd_inq8.Parameters.Set("pono",pono_prev );
						cmd_inq8.Parameters.Set("pour_time",pour_times_prev );
						event_status = "修改前炉的浇铸时间出错";   
						event_table = "TPSSM01";      
						event_key = pono_prev;       
						cmd_inq8.ExecuteNonQuery();
						////Log::Trace("",__FUNCTION__,"slab_place_code2 = [{0}]",tpssm01["SLAB_PLACE_CODE"].ToString());
					 }
					//当前炉的开浇时间 = 前一炉的开浇时间 + 炉浇注时间
					cc_req_times = cc_req_times_prev.AddMinutes(pour_time_prev);
				}
				v_cc_req_time = cc_req_times.ToString("yyyyMMddHHmmss");
				////Log::Trace("",__FUNCTION__,"v_cc_req_time = [{0}]",v_cc_req_time);
			}

			f_pssm_compute_class(tpssm01["SHIFT_NO"].ToString(),v_cc_req_time,v_whole_backlog_code,conn);

			 sqlstr = " UPDATE TPSSM01 "
					 "  SET  CC_REQ_TIME = @cc_req_time "
					 "       ,POUR_TIME = @pour_time"
					 "       ,SHIFT_NO = @shift_no "
					 " WHERE PONO = @pono ";
			CDbCommand cmd_inq7(sqlstr,conn);
			cmd_inq7.Parameters.Set("pono",tpssm01.PONO );
			cmd_inq7.Parameters.Set("cc_req_time",v_cc_req_time );
            cmd_inq7.Parameters.Set("pour_time",pour_times );
			cmd_inq7.Parameters.Set("shift_no",tpssm01["SHIFT_NO"].ToString());
			event_status = "修改开浇时间出错";   
			event_table = "TPSSM01";      
			event_key = tpssm01["PONO"];       
			cmd_inq7.ExecuteNonQuery();
			cc_req_times_prev = cc_req_times;
			pour_times_prev = pour_times;
			pono_prev = tpssm01["PONO"];
			fetchRowCount++;
			////Log::Trace("",__FUNCTION__,"pono_prev = [{0}]",pono_prev);
			////Log::Trace("",__FUNCTION__,"pour_times_prev = [{0}]",pour_times_prev.ToDouble());
		}
		cmd_inq.Close();

		//开始整理班次号 2012 / 8 / 14 10:34:03 顾东亮去除f_pssm_compute_class后添加 用来改进速度
		//CDbCommand cmd_update_tpssm01_shift_no(conn);
		//cmd_update_tpssm01_shift_no.SetCommandText(" UPDATE TPSSM01 "
		//	       " SET SHIFT_NO = '1' WHERE PLAN_DATE = @plan_date "
		//		   " AND CC_MACH_NO = @cc_mach_no AND UNIT_CODE = @unit_code "
	 //              " AND (substr(cc_req_time,9,2) >= '22' "
		//		   " or  substr(cc_req_time,9,2) < '07')  ");
		//cmd_update_tpssm01_shift_no.Parameters.Set("plan_date",plan_date);
		//cmd_update_tpssm01_shift_no.Parameters.Set("unit_code",unit_code);
		//cmd_update_tpssm01_shift_no.Parameters.Set("cc_mach_no",cc_mach_no);
		//cmd_update_tpssm01_shift_no.ExecuteNonQuery();

		//cmd_update_tpssm01_shift_no.SetCommandText(" UPDATE TPSSM01 "
		//	" SET SHIFT_NO = '2' WHERE PLAN_DATE = @plan_date "
		//	" AND CC_MACH_NO = @cc_mach_no AND UNIT_CODE = @unit_code "
		//	" AND (substr(cc_req_time,9,2) >= '07' "
		//	" AND substr(cc_req_time,9,2) < '15')  ");
		//cmd_update_tpssm01_shift_no.Parameters.Set("plan_date",plan_date);
		//cmd_update_tpssm01_shift_no.Parameters.Set("unit_code",unit_code);
		//cmd_update_tpssm01_shift_no.Parameters.Set("cc_mach_no",cc_mach_no);
		//cmd_update_tpssm01_shift_no.ExecuteNonQuery();

		//cmd_update_tpssm01_shift_no.SetCommandText(" UPDATE TPSSM01 "
		//	" SET SHIFT_NO = '2' WHERE PLAN_DATE = @plan_date "
		//	" AND CC_MACH_NO = @cc_mach_no AND UNIT_CODE = @unit_code "
		//	" AND (substr(cc_req_time,9,2) >= '15' "
		//	" AND substr(cc_req_time,9,2) < '22')  ");
		//cmd_update_tpssm01_shift_no.Parameters.Set("plan_date",plan_date);
		//cmd_update_tpssm01_shift_no.Parameters.Set("unit_code",unit_code);
		//cmd_update_tpssm01_shift_no.Parameters.Set("cc_mach_no",cc_mach_no);
		//cmd_update_tpssm01_shift_no.ExecuteNonQuery();
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		sqlcode = ex.GetCode();
		//CFormattable arguments[] = { "TPSBW102F", ex.GetCode() };
		//CMessageFormat::Format(s.msg,  _RES("GCRSS0000009")/*修改记录失败，表[{0}]，sqlcode = [{1}]。请联系系统维护人员。*/, arguments, 2);
		CString str = "DB error:" + sqlstr + "\r\n" + ex.GetMsg();
		//strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch(const CApplicationException& ex)
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

	////Log::Trace("pssm_compue_time",__FUNCTION__,"Program End:doFlag = [{0}],s.msg = [{1}],s.sysmsg = [{2}]",doFlag,s.msg,s.sysmsg);

	//返回-1时事务将回滚，返回为0是事务将提交
	//出错事务回滚
	if (doFlag != 0)
	{
#ifdef _DCDS_
		conn->Rollback();
		conn->BeginTransaction();
#else
		tpabort(0);
		tpbegin(0,0);
#endif
	}

	//对出错信息表赋值	
	if  (doFlag != 0)
	{
		tpssmerrm["REC_CREATOR"] = s.userid;
		tpssmerrm["REC_CREATE_TIME"] = s.datetime;
		tpssmerrm["REC_REVISE_TIME"] = tpssmerrm["REC_CREATE_TIME"];
		tpssmerrm["REC_REVISOR"] = s.svc_name;
		//CString(s.datetime).Substring(0,8); 
		//CDateTime::Now().ToString("yyyyMMddHHmmss");
		tpssmerrm["ERROR_EVENT_DATE"] = CString(s.datetime).Substring(0,8);
		CDbCommand cmd_query_tc_seq_no(conn);
		cmd_query_tc_seq_no.SetCommandText("SELECT  SUBSTR(TO_CHAR(current timestamp,'yyyymmddhh24missff6'),1,20)  from sysibm.sysdummy1; ");
		cmd_query_tc_seq_no.ExecuteReader();
		if (cmd_query_tc_seq_no.Read()) 
		{
			tpssmerrm["ERROR_TIME"] = cmd_query_tc_seq_no.GetString(1); //将数据获取到实体对象中
		} 
		cmd_query_tc_seq_no.Close();
		tpssmerrm["ERROR_PROGRAM"] = "f_pssm_compute_time";
		tpssmerrm["ERROR_SQL_CODE"] = sqlcode;
		tpssmerrm["ERROR_STATUS"] = event_status;
		tpssmerrm["EVENT_TABLE"] = event_table;
		tpssmerrm["ERROR_REASON"] = sqlstr + event_key;

		//写出错处理履历
		tpssmerrm.Insert();

#ifdef _DCDS_
		conn->Commit();
		conn->BeginTransaction();
#else
		tpcommit(0);
		tpbegin(0,0);
#endif

	}
	return(doFlag);
}
