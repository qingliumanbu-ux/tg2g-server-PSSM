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



int f_pssm11_pour_time(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn); 
int f_pssm12_time_calc(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);

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
int f_pssm_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i, j, rows, blkseq;
	int count = 0;
	int ret = 0;
	bool has = false;
	char msg[200] = "";
	int blkNum = 0;
	int heat_scope = 0;
	int time_upd = 0;
	int proc_no_upd	= 0;	

	CString cc_req_time = " "; //记录各连铸机要求开浇时刻
	CDateTime tmp_time;
	CString sqlstr = " ";

	EIClass inBlock;
	EIClass outBlock;
	
	CDbCommand cmd_tpssm11_inq(conn);

	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	
	try
	{
		inBlock.Tables[0].set_TableName("POUR");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CC_REQ_TIME");		
		
		inBlock.Tables.Add("PONO");
		inBlock.Tables[1].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[1].Columns.Add(DT_STRING, "PONO");
			
		
		blkseq = bcls_rec->Tables.IndexOf("COND");
		if (blkseq < 0) 
		{
			strcpy(s.msg, "系统出现异常，数据块有误，请联系系统维护人员");
			throw CApplicationException(-1, s.msg,  log.Location );
		}
		/* 获得输入参数 */
		tpssm11["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		
		heat_scope = 3;
		time_upd = 1;
		proc_no_upd	= 0;
		
		//-----------------------------------------------------------
		//读取新CAST信息
		blkseq = bcls_rec->Tables.IndexOf("CAST"); //浇铸信息		
		if (blkseq < 0) 
		{
			strcpy(s.msg,"系统出现异常，数据块有误，请联系系统维护人员");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{
			//获取传入参数
			tpssm11["PONO"] = bcls_rec->Tables[blkseq].Rows[i]["PONO"];
			cc_req_time	= bcls_rec->Tables[blkseq].Rows[i]["CC_REQ_TIME"]; //以cc_req_time作为开浇时刻

			//打印传入参数
			////Log::Info("", __FUNCTION__, "f_pssm_time>PONO = [{0}]", tpssm11["PONO"].ToString());
			////Log::Info("", __FUNCTION__, "f_pssm_time>cc_req_time = [{0}]", cc_req_time);
		
			if (tpssm11.QueryCount("PONO") == 0)
			{
				sprintf(s.msg, "制造命令[%s]已不在计划中.", tpssm11["PONO"].ToString());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			sqlstr = "tpssm11.Query()";
			tpssm11.Query("FACTORY_DIV,PONO");
			tpssm11.TrimOrBlank();
		
			if (cc_req_time.Trim() == "")
			{
				//CCtime 为空，什么也不做
				strcpy(s.msg, "CC时刻未输入!");
			}		
		
			if(heat_scope == 3)
			{
				switch(conn->DatabaseKind)
				{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库k
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
					sqlstr = " UPDATE TPSSM11 "
							"	SET CC_REQ_TIME = ' ', "
							"	PLAN_EDIT_FLAG  = 'U'  "
							"  WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
							"  AND RUN_STATUS < '53' "
							"  AND CC_MACH_NO = @tpssm11.CC_MACH_NO "
							"  AND ((CAST_NO = @tpssm11.CAST_NO AND CAST_DIV_NO >= @tpssm11.CAST_DIV_NO) "
							"  OR CAST_NO > @tpssm11.CAST_NO ) ";
					break;
				}
				cmd_tpssm11_inq.SetCommandText( sqlstr );
				cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
				cmd_tpssm11_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
				cmd_tpssm11_inq.Parameters.Set("tpssm11.CAST_NO", tpssm11["CAST_NO"].ToString());
				cmd_tpssm11_inq.Parameters.Set("tpssm11.CAST_DIV_NO", tpssm11["CAST_DIV_NO"].ToDecimal());
				cmd_tpssm11_inq.ExecuteNonQuery();

				/*tpssm11["CC_REQ_TIME"] = " ";
				tpssm11["PLAN_EDIT_FLAG"] = "U";
				tpssm11.Update("CC_REQ_TIME, PLAN_EDIT_FLAG", "FACTORY_DIV, PONO");*/
			}
		}
		
		//------------------------------------------------------
		// 写入连铸机
		CDataRow & row = inBlock.Tables["POUR"].Rows.Add();
		row["FACTORY_DIV"] 	= tpssm11["FACTORY_DIV"];
		row["CC_MACH_NO"] 	= tpssm11["CC_MACH_NO"];
		row["CC_REQ_TIME"] 	= cc_req_time;
		
		//1.浇铸时刻计算, 输入参数:修改PONO
		ret = f_pssm11_pour_time(&inBlock, &outBlock, conn);
		if (ret < 0)
		{	
			throw CApplicationException(-1,s.msg,log.Location);
		}
		
		//2.前工序时刻调整标记: 1-计算前工序的作业时刻
		if (time_upd == 1)
		{
			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库k
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句

					sqlstr = " SELECT PONO "
							" FROM TPSSM11 "
							" WHERE FACTORY_DIV = @tpssm11.FACTORY_DIV "
							" AND PLAN_EDIT_FLAG = 'U' "
							" AND CC_MACH_NO = @tpssm11.CC_MACH_NO ";
				break;
			}
			cmd_tpssm11_inq.SetCommandText( sqlstr );
			cmd_tpssm11_inq.Parameters.Set("tpssm11.FACTORY_DIV", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm11_inq.Parameters.Set("tpssm11.CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
			cmd_tpssm11_inq.ExecuteReader();
			
			while ( cmd_tpssm11_inq.Read() )
			{
				tpssm11["PONO"] = cmd_tpssm11_inq.GetString(1);
				
				CDataRow & row = inBlock.Tables["PONO"].Rows.Add();
				row["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				row["PONO"] = tpssm11["PONO"];
			}
			cmd_tpssm11_inq.Close();
		
			//调用函数
			ret = f_pssm12_time_calc(&inBlock, &outBlock, conn);
			if (ret < 0)
			{	
				throw CApplicationException(-1,s.msg,log.Location);
			}
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
