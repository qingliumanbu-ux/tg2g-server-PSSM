/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-12-13
Description:	出钢计划新增炉次计算开浇时刻。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



/*<remark>=========================================================
/// <summary>
/// 新增炉次计算开浇时刻
/// <para>新增计划时, 重新设定开浇时刻时调用。</para>
/// <para>1.获取当前计划中最后一个炉次的开浇信息；</para>
/// <para>2.对新增追加炉次，根据前一炉次计划，计算本炉次开浇时刻；</para>
/// <para>3.计算过程按开铸顺序依次计算：</para>
/// <para> 1)由开浇时刻 -> 浇铸结束时刻；</para>
/// <para> 2)由浇完时刻 -> 下一炉开浇时刻；</para>
/// <para>4.根据开浇时刻, 计算包到达时刻。</para>
/// <para>数据库表：TPSSM11(炼钢出钢计划主表)</para>
/// <para>主调用函数：f_pssm11_cast()</para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码</param>
/// <param name="PONO">制造命令号</param>
/// <returns>无</returns>
===========================================================</remark>*/
int f_pssm11_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i, j, rows, blkseq;
	int count = 0;
	int fetchRowCount = 0;
	int ret = 0;
	bool has = false;
	CString msg = "";

    CString cc_req_time;     //记录各连铸机要求开浇时刻
	CString ccm_no = "";
	CDecimal diff_time = 0;
	CString base_time = "";
	CString dest_time = "";
	CDateTime tmp_time;
	CString sqlstr = "";
	CString end_time_pre = "";
	CString v_cast_no_pre = "";
	int v_cast_div_no_pre = 0;

	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	
	try
	{
		//-----------------------------------------------------------------------
		//获得输入参数
		//1.读取编入计划的PONO
		blkseq = bcls_rec->Tables.IndexOf("POUR"); //浇铸信息
		if (blkseq < 0) 
		{
			strcpy(s.msg,_RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.sysmsg, "TABLE [POUR] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		////Log::Info("", __FUNCTION__, "rows = [{0}]", rows);

		for (i = 0; i < rows; i++ )
		{
			//获取传入参数
			tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			tpssm11["CC_MACH_NO"] = bcls_rec->Tables[blkseq].Rows[i]["CC_MACH_NO"];
			tpssm11["CC_REQ_TIME"] = bcls_rec->Tables[blkseq].Rows[i]["CC_REQ_TIME"]; //以cc_req_time作为开浇时刻

			//打印传入参数
			////Log::Info("", __FUNCTION__, "f_pssm11_pour_time>tpssm11["FACTORY_DIV"] = [{0}]", tpssm11["FACTORY_DIV"].ToString());
			////Log::Info("", __FUNCTION__, "f_pssm11_pour_time>tpssm11["CC_MACH_NO"] = [{0}]", tpssm11["CC_MACH_NO"].ToString());
			////Log::Info("", __FUNCTION__, "f_pssm11_pour_time>tpssm11["CC_REQ_TIME"] = [{0}]", tpssm11["CC_REQ_TIME"].ToString());

			//这里改掉cc_req_time[j]  HYF 0713 不然模铸传不进时间
			//j = atol((const char*)tpssm11["CC_MACH_NO"].ToString()) - 1;
		
			cc_req_time = tpssm11["CC_REQ_TIME"];
			//j=atol((const char*)tpssm11["CC_MACH_NO"].ToString());
			//cc_req_time[j] = tpssm11["CC_REQ_TIME"];
		}

		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库k
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句

				sqlstr =  "SELECT * FROM TPSSM11 "
					"WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
					"AND PLAN_EDIT_FLAG IN ( 'N', 'U') "
					"AND CC_MACH_NO = @tpssm11.CC_MACH_NO "
					"AND RUN_STATUS < '53' " //不加上第一炉有错
					"ORDER BY CAST_NO, CAST_DIV_NO ASC ";
			break;
		}
		cmd_tpssm11_inq.SetCommandText( sqlstr );
		cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
		cmd_tpssm11_inq.ExecuteReader();
		while ( cmd_tpssm11_inq.Read() )
		{
			fetchRowCount++;

			//tpssm11["CC_REQ_TIME"]="";
			cmd_tpssm11_inq.Fetch(tpssm11);
			tpssm11.TrimOrBlank();

			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"]	= tpssm11["SM_PLAN_NO"];
			tpssm12["AREA_ID"]		= 5;
			has = tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, AREA_ID");
			if (!has)
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000153")/*制造命令号[{0}]的连铸工序信息不存在。*/, arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}
			////Log::Info("", __FUNCTION__, "tpssm11["PONO"] = [{0}]", tpssm11["PONO"].ToString());
			////Log::Info("", __FUNCTION__, "tpssm11["CC_REQ_TIME"] = [{0}]", tpssm11["CC_REQ_TIME"].ToString());

			if (tpssm11["CC_REQ_TIME"].ToString().Trim() == "") //如果主计划表中没有设置开浇时刻, 则为新增炉次
			{
				//1.设置开浇时刻
				if ( ccm_no.Trim() != tpssm11["CC_MACH_NO"].ToString().Trim() )  //计算第一炉, fetchRowCount == 1
				{
					////Log::Info("", __FUNCTION__, "计算第一炉");

					//j=atol((const char*)tpssm11["CC_MACH_NO"].ToString())-1;//计算当前的连铸机号
					//tpssm12["START_TIME"] = cc_req_time[j]; //前台只送一个开浇时刻, xuwen 2010-3-23 修改cc_req_time[j]
					//ccm_no = tpssm11["CC_MACH_NO"];

					if(cc_req_time.Trim() == "")
					{
						diff_time = tpssm12["PREP_TIME"];
						//根据上一炉的结束时刻和准备时间, 计算本炉的开浇时刻			
						
						tmp_time = CDateTime::Parse(end_time_pre);
						tpssm12["START_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");

						//ret = EPTimeOffset(tpssm12.end_time, diff_time, tpssm12.start_time, msg);
						//if(ret < 0)	EDLog(1,1, "1.EPTimeOffset failure: %s\n", msg);
					}
					else
					{					
						tpssm12["START_TIME"] = cc_req_time; //前台只送一个开浇时刻, xuwen 2010-3-23 修改cc_req_time[j]
					}		
					
					ccm_no= tpssm11["CC_MACH_NO"];

				}
				else
				{
					////Log::Info("", __FUNCTION__, "计算第[{0}]炉", fetchRowCount);

					diff_time = tpssm12["PREP_TIME"];

					////Log::Info("", __FUNCTION__, "准备时间 = [{0}]", tpssm12["PREP_TIME"].ToDecimal());
					////Log::Info("", __FUNCTION__, "end_time_pre = [{0}]", end_time_pre);

					//根据上一炉的结束时刻和准备时间, 计算本炉的开浇时刻
					tmp_time = CDateTime::Parse(end_time_pre);
					tpssm12["START_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");


				}
			}
			else //有值时, 表示出钢计划调整开浇时刻, 以此时刻为准计算连铸各时刻点
			{
				tpssm12["START_TIME"] = tpssm11["CC_REQ_TIME"];				
			}
			
			////Log::Info("", __FUNCTION__, "tpssm12["START_TIME"] = [{0}]", tpssm12["START_TIME"].ToString());

			//2.根据本炉次开浇时刻, 计算浇完时刻
			diff_time = tpssm12["PROC_TIME"];

			////Log::Info("", __FUNCTION__, "diff_time = [{0}]", diff_time);

			tmp_time=CDateTime::Parse(tpssm12["START_TIME"].ToString());
			tpssm12["END_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");
			
			////Log::Info("", __FUNCTION__, "tpssm12["END_TIME"] = [{0}]", tpssm12["END_TIME"].ToString());

			end_time_pre = tpssm12["END_TIME"];

//			//3.根据开浇时刻, 计算包到达时刻
//			diff_time = tpssm12.WAITING_TIME;
//			tmp_time=CDateTime::Parse(tpssm12["START_TIME"].ToString());
//			tpssm12["LADLE_ARRIVE_TIME"] =tmp_time.AddMinutes(-diff_time.ToDouble()).ToString("yyyyMMddHHmmss");
//
//			//4.根据浇完时刻, 计算包离开时刻
//			diff_time = 4;
//			tmp_time=CDateTime::Parse(tpssm12["END_TIME"].ToString());
//			tpssm12["LADLE_LEAVE_TIME"] = tmp_time.AddMinutes(diff_time.ToDouble()).ToString("yyyyMMddHHmmss");

			tpssm12["LADLE_ARRIVE_TIME"] = tpssm12["START_TIME"];
			tpssm12["LADLE_LEAVE_TIME"] = tpssm12["END_TIME"];

			//将计算的结果写入计划子表中
			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["AREA_ID"] = 5;
			tpssm12.Update("LADLE_ARRIVE_TIME, START_TIME, END_TIME, LADLE_LEAVE_TIME", "FACTORY_DIV, SM_PLAN_NO, AREA_ID");

			if (tpssm11["RUN_STATUS"].ToString() == "52")
			{
				//修改计划主表的cc_req_time
				//0223 HYF 暂时注释
				tpssm11["CC_REQ_TIME"] = tpssm12["START_TIME_REAL"];
			}
			else
			{
				//修改计划主表的cc_req_time
				//0223 HYF 暂时注释
				tpssm11["CC_REQ_TIME"] = tpssm12["START_TIME"];
			}

			tpssm11.Update("CC_REQ_TIME", "FACTORY_DIV, SM_PLAN_NO");
			//ccm_no = tpssm11["CC_MACH_NO"];

			v_cast_no_pre = tpssm11["CAST_NO"];
			v_cast_div_no_pre = tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32();

		}//while
		cmd_tpssm11_inq.Close();

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
