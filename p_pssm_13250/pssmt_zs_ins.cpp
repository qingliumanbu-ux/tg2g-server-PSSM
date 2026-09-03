/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhengqiangqiang1
Version:    1.0
Date:     2023-05-12
Description:	炼钢计划准点率统计
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件

// service入口
BM2F_ENTERACE(pssmt_zs_ins)

int f_pssmt_zs_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{	  
	CTracer log(__FUNCTION__);
	/* 程序用变量 */
	int doFlag = 0;
	int fetchRowCount;
	int i;
	int cd_count=0;
	int  heat_count = 0;
	int zy_time_count=0;//作业时间
	int gz_time_count=0;//故障时间
	CString	create_time = "";    
	CString	pre_time = "";

	CDecimal zdl=0;//准点率
    CDecimal gzl=0;//故障率

	CString	factory_div = "";
	CString	station_id = "";
    CString	cs_start_time=""; 
    CString	cs_end_time=""; 	
	CString	cs_dev_code="";  
	CString prod_shift_no = "";
	CString prod_shift_group = "";

   
	char msg[100] = " ";
	long days_diffx = 0;
	long hours_diffx = 0;
	long minutes_diffx = 0;
	long seconds_diffx = 0;
	CDecimal time_diff=0; //时间差

	int total_count = 0; //总记录数
	int total_count_a = 0; //甲班记录数
	int total_count_b = 0; //乙班记录数
	int total_count_c = 0; //丙班记录数
	int total_count_d = 0; //丁班记录数

	int zd_count = 0;//准点个数
	int zd_count_a = 0; //甲班准点个数
	int zd_count_b = 0; //乙班准点个数
	int zd_count_c = 0; //丙班准点个数
	int zd_count_d = 0; //丁班准点个数

	CModel tpssmb1("TPSSMB1");
	CModel tpssmd1("TPSSMD1");
	CModel tpssm12("TPSSM12");
	CModel tpssm11("TPSSM11");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_12(conn);
	CDbCommand cmd_inq_11(conn);

	CString sqlstr="";
	CString sqlstr_12="";
	CString sqlstr_11 = "";



	try
	{
		//测试用
		create_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
		pre_time = CDateTime::Now().AddDays(-1).ToString("yyyyMMddHHmmss");
		cs_start_time = pre_time.Substring(0, 8) + "000000";
		cs_end_time = pre_time.Substring(0, 8) + "235959";

		Log::Trace("", __FUNCTION__, "...打印传入参数...");


		Log::Trace("", __FUNCTION__, "...cs_start_time=[{0}]", cs_start_time);
		Log::Trace("", __FUNCTION__, "...cs_end_time=[{0}]", cs_end_time);

		//=======================================================================
		//按设备类型循环-计算各工序准点率
		//======================================
		sqlstr = " SELECT FACTORY_DIV, STATION_ID		"
			" FROM TPSSMD1							"
			" where 1 = 1							"
			" AND STATION_ID IN('B', 'L', 'R', 'C')	"
			" group by FACTORY_DIV, STATION_ID		"
			" order by FACTORY_DIV					"
			;
		Log::Trace("", __FUNCTION__, "设备查询sqlstr=[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{

			factory_div = cmd_inq.GetString(1).Trim();
			station_id = cmd_inq.GetString(2).Trim();

			Log::Trace("", __FUNCTION__, "...factory_div=[{0}]", factory_div);
			Log::Trace("", __FUNCTION__, "...station_id=[{0}]", station_id);

			Log::Trace("", __FUNCTION__, "...开始统计各工序准点率...");
			sqlstr_12 = " SELECT  START_TIME, START_TIME_REAL, PROC_TIME, DEV_CODE	"
				" FROM TPSSM12												"
				" WHERE  START_TIME >= @cs_start_time						"
				" AND    START_TIME <= @cs_end_time							"
				" AND    DEV_CODE    LIKE @station_id || '%'				"
				" UNION ALL													"
				" SELECT  START_TIME, START_TIME_REAL, PROC_TIME, DEV_CODE	"
				" FROM TPSSM42												"
				" WHERE  START_TIME >= @cs_start_time						"
				" AND    START_TIME <= @cs_end_time							"
				" AND    DEV_CODE    LIKE @station_id || '%'				"
				" ORDER BY DEV_CODE											"
				;
			Log::Trace("", __FUNCTION__, "sqlstr_12=[{0}]", sqlstr_12);
			cmd_inq_12.SetCommandText(sqlstr_12);
			cmd_inq_12.Parameters.Set("cs_start_time", cs_start_time);
			cmd_inq_12.Parameters.Set("cs_end_time", cs_end_time);
			cmd_inq_12.Parameters.Set("station_id", station_id);
			cmd_inq_12.ExecuteReader();
			while (cmd_inq_12.Read())
			{
				tpssm12["START_TIME"] = cmd_inq_12.GetString(1);
				tpssm12["START_TIME_REAL"] = cmd_inq_12.GetString(2);
				tpssm12["PROC_TIME"] = cmd_inq_12.GetDecimal(3);
				tpssm12["DEV_CODE"] = cmd_inq_12.GetString(4);

				f_epep_get_shift_group("SM", tpssm12["START_TIME"].ToString(), prod_shift_no, prod_shift_group, conn);

				Log::Trace("", __FUNCTION__, "==班次==[{0}]", prod_shift_no);
				Log::Trace("", __FUNCTION__, "==班组==[{0}]", prod_shift_group);

				if (prod_shift_group == "A")
				{
					total_count_a++;
				}
				else if (prod_shift_group == "B")
				{
					total_count_b++;
				}
				else if (prod_shift_group == "C")
				{
					total_count_c++;
				}
				else if (prod_shift_group == "D")
				{
					total_count_d++;
				}

				total_count++;

				if (tpssm12["START_TIME_REAL"].ToString().Trim() != "")
				{
					char end_prod_time[15] = " ";
					char start_prod_time[15] = " ";

					strcpy(end_prod_time, (const char*)tpssm12["START_TIME_REAL"].ToString());
					strcpy(start_prod_time, (const char*)tpssm12["START_TIME"].ToString());

					//1.计算时间差，时间差小于5min认为准点
					doFlag = EPTimeDiff(end_prod_time, start_prod_time, &days_diffx, &hours_diffx, &minutes_diffx, &seconds_diffx, msg);
					if (doFlag < 0)
					{
						throw CApplicationException(-1, msg, s.svc_name);
					}
					time_diff = days_diffx * 60 * 24 + hours_diffx * 60 + minutes_diffx;
					if (abs(time_diff.ToInt32()) <= 5)
					{
						zd_count = zd_count + 1;
						Log::Trace("", __FUNCTION__, "总准点个数 zd_count=[{0}]", zd_count);

						if (prod_shift_group == "A")
						{
							zd_count_a++;
						}
						else if (prod_shift_group == "B")
						{
							zd_count_b++;
						}
						else if (prod_shift_group == "C")
						{
							zd_count_c++;
						}
						else if (prod_shift_group == "D")
						{
							zd_count_d++;
						}

					}

				}
			}
			cmd_inq_12.Close();


			Log::Trace("", __FUNCTION__, "工序类型station_id=[{0}],总计划数量total_count=[{1}]，总准点个数zd_count=[{2}]", station_id,total_count, zd_count);
			Log::Trace("", __FUNCTION__, "A: 计划数量total_count_a=[{0}]，准点个数zd_count_a=[{1}]", total_count_a, zd_count_a);
			Log::Trace("", __FUNCTION__, "B: 计划数量total_count_b=[{0}]，准点个数zd_count_b=[{1}]", total_count_b, zd_count_b);
			Log::Trace("", __FUNCTION__, "C: 计划数量total_count_c=[{0}]，准点个数zd_count_c=[{1}]", total_count_c, zd_count_c);
			Log::Trace("", __FUNCTION__, "D: 计划数量total_count_d=[{0}]，准点个数zd_count_d=[{1}]", total_count_d, zd_count_d);


			//按厂别、设备类型插表
			tpssmb1["FACTORY_DIV"] = factory_div;
			tpssmb1["STATION_ID"] = station_id;
			tpssmb1["PROD_TIME"] = pre_time.Substring(0, 8);

			tpssmb1["GROUP1_DATA"] = total_count_a > 0 ? ((double)zd_count_a / (double)total_count_a) * 100 : 0;
			tpssmb1["GROUP2_DATA"] = total_count_b > 0 ? ((double)zd_count_b / (double)total_count_b) * 100 : 0;
			tpssmb1["GROUP3_DATA"] = total_count_c > 0 ? ((double)zd_count_c / (double)total_count_c) * 100 : 0;
			tpssmb1["GROUP4_DATA"] = total_count_d > 0 ? ((double)zd_count_d / (double)total_count_d) * 100 : 0;
			tpssmb1["RATIO_NUM"] = total_count > 0 ? ((double)zd_count/ (double)total_count) * 100 : 0;
			tpssmb1["REC_CREATE_TIME"] = create_time;
			tpssmb1["REC_CREATOR"] = s.userid;

			//Log::Trace("", __FUNCTION__, "打印");
			//tpssmb1.Print();
			tpssmb1.Insert();
			
		}
		cmd_inq.Close();


		//=======================================================================
		//按炉次-计算各炉次运行周期(精炼出站，到大包上回转台)
		//======================================
		sqlstr = "SELECT CODE FROM TEP0002 WHERE CODE_CLASS='PSA0' ";
		Log::Trace("", __FUNCTION__, "分厂别sqlstr=[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			factory_div = cmd_inq.GetString(1);

			//按厂别、设备类型插表
			tpssmb1["FACTORY_DIV"] = factory_div;
			tpssmb1["STATION_ID"] = "F";
			tpssmb1["PROD_TIME"] = pre_time.Substring(0, 8);
			tpssmb1["REC_CREATE_TIME"] = create_time;
			tpssmb1["REC_CREATOR"] = s.userid;

			//临时测试用，具体计算规则后续补充
			tpssmb1["GROUP1_DATA"] = 65.78;
			tpssmb1["GROUP2_DATA"] = 55.43;
			tpssmb1["GROUP3_DATA"] = 46.48;
			tpssmb1["GROUP4_DATA"] = 49.92;
			tpssmb1["RATIO_NUM"] = 50.41;


			//Log::Trace("", __FUNCTION__, "打印");
			//tpssmb1.Print();
			tpssmb1.Insert();

		}
		cmd_inq.Close();

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
