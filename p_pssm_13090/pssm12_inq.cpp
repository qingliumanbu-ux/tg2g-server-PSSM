/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-21
Description: 出钢计划炉次条件查询
**************************************************/
#include "stdafx.h"


//#include "tpssm26.h"

//#include "tpssmc1.h"


/*<remark>=========================================================
/// <summary>
/// 出钢计划炉次条件查询
/// <para>1.查询炼钢计划编制表 tpssm11 主表中的记录,由PONO查询子工序的记录。</para>
/// <para>2.查询子工序表, 将内容写入相应的列中                     </para>
/// <para>  1)根据计划子表的内容, 确定记录写入BLOCK的列名；        </para>
/// <para>  2)判断实绩是否接收过,以置实绩标志和取实绩时刻；        </para>
/// <para>  3)写入实绩数据和各子工序实绩标志；                     </para>
/// <para>3.对转炉区,要送出转炉名称,并将各子工序送出。             </para>
/// <para>主调用函数:   前台PSSM12P画面F2(查询)按钮。               </para>
/// </summary>
/// <returns>出钢计划炉次条件</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm12_inq)


int f_pssm12_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	//int   srf_length;    /* 精炼最大路径长度 */
	int   fetchRowCount;
	int   blknum, rows = 0;

	CString sort_flag = "";  //排序要求：1-按浇铸顺排序
	CString cc_mach_no = "";
	CString sm_plan_no = ""; //炼钢计划号

	CString colname = "";  //
	CString colname2 = ""; //显示计划时间
	CString ccm_no;
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	CString show_flag = "0";
	CString cast_no_show = "";
	CString v_refine_route_desc = "";
	int first_srf; //精炼工序的第一个charge_no
	int sr_seq = 0;//精炼重数
	//int  early_flag = 0;   //早到时间超标: 0-范围内; 1-超出
	CString start_time = "";
	CString proc_no = ""; //各工序处理号
	CString factory_div = "";

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);  //定义出钢计划子表查询命令
	CDbCommand cmd_tpssmd1_inq(conn);  //定义炼钢设备表查询命令
	CDbCommand cmd_tpssmc1_inq(conn);  //定义连铸标准查询命令


	try
	{
		// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
		//CTPSSM26 tpssm26(conn);
	CModel tpssmd1("TPSSMD1");
		//CTPSSMC1 tpssmc1(conn);


		//------------------------------------------
		//设置返回块参数
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("TPSSM_CONST");   //与客户端 dsPSSM11.TPSSM_CONST 一致
		bcls_ret->Tables[blknum].Columns.Add(tpssm11);   //从实体对象创建架构
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "POUR_TIME");  //开浇时刻（C4 HHmm），与CC_REQ_TIME用法一致
		//各工序的设备号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DS_DEV");       //脱硫设备号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DP_DEV");       //脱磷转炉号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SM_DEV");       //转炉号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR1_DEV");		 //精炼1
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR2_DEV");		 //精炼2
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR3_DEV");		 //精炼3
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR4_DEV");		 //精炼4
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_DEV");		 //连铸

		//各工序的处理时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DS_PROC_TIME");	//铁水预处理（脱S）处理时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DP_LOAD_TIME");	//转炉脱P装入时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DP_PROC_TIME");	//转炉脱P处理时间
		//bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DP_BLOW_TIME");   //脱P吹炼开始
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DP_TAP_TIME");	    //转炉脱P出钢时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SM_LOAD_TIME");    //炼钢(脱C)装入时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SM_PROC_TIME");	//转炉 即为熔炼时间
		//bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SM_MELT_TIME");   //炼钢(脱C)冶炼炼开始
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SM_TAP_TIME");    //炼钢(脱C)出钢时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR1_PROC_TIME");   //第一重精炼处理时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR2_PROC_TIME");
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR3_PROC_TIME");
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR4_PROC_TIME");
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "CC_PROC_TIME");       //连模铸处理号
		//各工序的休辅时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DS_REST_TIME");   //脱硫休辅时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "DP_REST_TIME");   //脱P休辅时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SM_REST_TIME");   //炼钢(脱C)休辅时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR1_REST_TIME");  //精炼1休辅时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR2_REST_TIME");
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR3_REST_TIME");
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "SR4_REST_TIME");
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "CC_REST_TIME");	    //连铸休辅时间
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "CC_START_TIME");

		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CAST_NO_SHOW");
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "REFINE_ROUTE_DESC");
		//为运转开始信号颜色显示用
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DS_FLAG");    //脱硫预处理开始标记：1-开始;
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DP_FLAG");    //脱P开始标记: 1-装入; 3-吹炼; 5-出钢
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SM_FLAG");    //炼钢(脱C)标记: 1-装入; 3-吹炼; 5-出钢
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR1_FLAG");   //1重精炼标记: 1-包到; 2-开始; 3-结束; 4-包离
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR2_FLAG");   //2重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR3_FLAG");   //3重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR4_FLAG");   //4重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_FLAG");    //连铸标记:1-包到; 2-开浇; 3-浇完; 4-包离
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "EARLY_FLAG"); //早到时间超标记: 0-范围内; 1-超出


		//------------------------------------------
		//获得输入参数
		blknum = 0;  //输入第一块
		rows = bcls_rec->Tables[blknum].Rows.get_Count();
		if (rows > 0)  //有传入参数，获取
		{
			factory_div = bcls_rec->Tables[blknum].Rows[0]["FACTORY_DIV"];
			sm_plan_no	= bcls_rec->Tables[blknum].Rows[0]["SM_PLAN_NO"];
			//sort_flag = bcls_rec->Tables[0].Rows[0]["FLAG"];                 //排序标识：1-根据浇铸顺排序
			//cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"];
		}
		else
		{
			sm_plan_no = " ";
		}

		////Log::Info("", __FUNCTION__, "SM_PLAN_NO=[{0}]", sm_plan_no);
		////Log::Info("", __FUNCTION__, "FACTORY_DIV=[{0}]", factory_div);

		//------------------------------------------
		// 出钢计划查询
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = "SELECT * FROM TPSSM11 WHERE 1 = 1 ";

			if (factory_div.Trim() != "")   //按指定铸机号查询
				sqlstr += " AND FACTORY_DIV =@factory_div ";

			if (sm_plan_no.Trim() != "")//按指定计划号查询
				sqlstr += " AND SM_PLAN_NO =@sm_plan_no ";

			//if (cc_mach_no.Trim() != "")   //按指定铸机号查询
			//	sqlstr += " AND CC_MACH_NO =@cc_mach_no ";

			if (sort_flag == "1")
			{
				sqlstr += " ORDER BY CAST_NO ASC, CAST_DIV_NO ASC ";
			}
			else
			{
				//sqlstr += " ORDER BY SUBSTR(HEAT_NO,0,2),SUBSTR(HEAT_NO,2) ";
				sqlstr += " ORDER BY SM_PLAN_NO ASC ";
			}
			break;
		}
		
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.Parameters.Set("sm_plan_no", sm_plan_no);
		cmd_inq.Parameters.Set("cc_mach_no", cc_mach_no);
		cmd_inq.ExecuteReader();
		fetchRowCount = 0;
		blknum = 0; //第1块
		while (cmd_inq.Read())
		{

			cmd_inq.Fetch(tpssm11);//把数据都压在头文件里面
			//tpssm11.MergeTo( bcls_ret->Tables[blknum], false );

			CDataRow& row = bcls_ret->Tables["TPSSM_CONST"].Rows.Add();
			row.Merge(tpssm11);
			row["POUR_TIME"] = tpssm11["CC_REQ_TIME"].ToString().Trim().SubstringNE(8, 4);  //4位开浇时刻
			////Log::Trace("", __FUNCTION__, "row=[{0}], pono=[{1}]", fetchRowCount, tpssm11["PONO"].ToString());

			//精炼工序的第一个charge_no 必定不为0
			first_srf = 0;
			//------------------------------------------
			//查询出钢计划子表, 将炉次、时间等内容写入相应的列中
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					"SELECT * FROM TPSSM12 "
					" WHERE SM_PLAN_NO = @sm_plan_no "
					"   AND FACTORY_DIV = @factory_div "
					" ORDER BY CHARGE_NO ASC, SUB_CHARGE_NO ASC "
					);
				break;
			}

			cmd_tpssm12_inq.SetCommandText(sqlstr);
			////Log::Info("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
			cmd_tpssm12_inq.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_tpssm12_inq.ExecuteReader();

			while (cmd_tpssm12_inq.Read())
			{
				cmd_tpssm12_inq.Fetch(tpssm12);

				////Log::Trace("", __FUNCTION__, "sm_plan_no =[{0}], charge_no=[{1}]-[{2}], dev_code=[{3}]",
					//tpssm12["SM_PLAN_NO"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SUB_CHARGE_NO"].ToDecimal(), tpssm12["DEV_CODE"].ToString());

				//------------------------------------------
				/* 查询设备信息，定义显示项
				*/
				//switch (conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//case DB_KIND_ORACLE:        // Oracle 数据库
				//default: // 所有数据库适用，通用SQL语句
				//	sqlstr = CString(
				//		"SELECT STATION_ID, STATION_NO FROM TPSSMD1"
				//		" WHERE DEV_CODE   = @tpssm12.dev_code "
				//		"   AND AREA_ID    = @tpssm12.area_id "
				//		);
				//	break;
				//}

				//cmd_tpssmd1_inq.SetCommandText(sqlstr);
				//cmd_tpssmd1_inq.Parameters.Set("tpssm12.dev_code", tpssm12["DEV_CODE"].ToString().Trim());
				//cmd_tpssmd1_inq.Parameters.Set("tpssm12.area_id", tpssm12["AREA_ID"].ToDecimal());
				//cmd_tpssmd1_inq.ExecuteReader();
				//if (cmd_tpssmd1_inq.Read())
				//{
				//	cmd_tpssmd1_inq.Fetch(tpssmd1);
				//}
				//cmd_tpssmd1_inq.Close();


				//根据计划子表的内容, 确定记录写入BLOCK的列名
				colname = ""; //列置空
				int tpssm12_charge_no = 0;  //计算精炼重数用
				CString str = "";

				////Log::Trace("", __FUNCTION__, "tpssm12["AREA_ID"] =[{0}]", tpssm12["AREA_ID"].ToDecimal());

				switch (tpssm12["AREA_ID"].ToDecimal().ToInt32())
				{
				case 1: //预处理
					row["DS_DEV"] = tpssm12["DEV_CODE"];
					row["DS_PROC_TIME"] = tpssm12["PROC_TIME"];
					row["DS_REST_TIME"] = tpssm12["REST_TIME"];      //休辅时间
					row["DS_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					break;
				case 2: //脱磷
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 0)
					{
						row["DP_DEV"] = tpssm12["DEV_CODE"];
						row["DP_REST_TIME"] = tpssm12["REST_TIME"];      //休辅时间
						row["DP_PROC_TIME"] = tpssm12["PROC_TIME"];
						row["DP_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 1) row["DP_LOAD_TIME"] = tpssm12["PROC_TIME"];  //装入时间
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 2) row["DP_PROC_TIME"] = tpssm12["PROC_TIME"];  //脱P 时间
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 3) row["DP_TAP_TIME"] = tpssm12["PROC_TIME"];   //出钢时间
					break;
				case 3: //转炉区
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 0)
					{
						row["SM_DEV"] = tpssm12["DEV_CODE"];
						row["SM_REST_TIME"] = tpssm12["REST_TIME"];      //休辅时间
						row["SM_PROC_TIME"] = tpssm12["PROC_TIME"];
						row["SM_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 1) row["SM_LOAD_TIME"] = tpssm12["PROC_TIME"];  //装入时间 PRE_PROC_TIME
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 2) row["SM_PROC_TIME"] = tpssm12["PROC_TIME"];  //脱C 时间
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal().ToInt32() == 3) row["SM_TAP_TIME"] = tpssm12["PROC_TIME"];   //出钢时间 POST_PROC_TIME
					break;

				case 4: //精炼区域
					tpssm12_charge_no = tpssm12["CHARGE_NO"].ToDecimal().ToInt32();
					if (first_srf == 0)
					{
						first_srf = tpssm12_charge_no;
					}
					sr_seq = tpssm12_charge_no - first_srf + 1;

					if (sr_seq == 1) //精炼1
					{
						row["SR1_DEV"] = tpssm12["DEV_CODE"];
						row["SR1_PROC_TIME"] = tpssm12["PROC_TIME"];
						row["SR1_REST_TIME"] = tpssm12["REST_TIME"];  //休辅时间
						row["SR1_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					}
					if (sr_seq == 2) //精炼2
					{
						row["SR2_DEV"] = tpssm12["DEV_CODE"];
						row["SR2_PROC_TIME"] = tpssm12["PROC_TIME"];
						row["SR2_REST_TIME"] = tpssm12["REST_TIME"];  //休辅时间
						row["SR2_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					}
					if (sr_seq == 3) //精炼3
					{
						row["SR3_DEV"] = tpssm12["DEV_CODE"];
						row["SR3_PROC_TIME"] = tpssm12["PROC_TIME"];
						row["SR3_REST_TIME"] = tpssm12["REST_TIME"];  //休辅时间
						row["SR3_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					}
					if (sr_seq == 4) //精炼4
					{
						row["SR4_DEV"] = tpssm12["DEV_CODE"];
						row["SR4_PROC_TIME"] = tpssm12["PROC_TIME"];
						row["SR4_REST_TIME"] = tpssm12["REST_TIME"];  //休辅时间
						row["SR4_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					}
					sr_seq = 0;
					break;

				case 5: //浇铸
					row["CC_DEV"] = tpssm12["DEV_CODE"];
					row["CC_PROC_TIME"] = tpssm12["PROC_TIME"];
					//row["CC_REST_TIME"] = tpssm12["REST_TIME"];  //休辅时间
					row["CC_START_TIME"] = tpssm12["START_TIME"];
					row["CC_FLAG"] = (tpssm11["CURR_WP_NO"].ToDecimal() >= tpssm12["CHARGE_NO"].ToDecimal()) ? "1" : "";
					break;
				default:
					break;
				}//switch

			}//while TPSSM12
			cmd_tpssm12_inq.Close();  //一定要Close，否则CLI0115E  Invalid cursor state.

			cast_no_show = tpssm11["CAST_NO"].ToString() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();
			row["CAST_NO_SHOW"] = cast_no_show;

		}//while TPSSM11


	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, 399);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399);
		s.flag = ex.GetCode();
		return -1;
	}
	cmd_tpssm12_inq.Close();
	cmd_tpssmd1_inq.Close();
	cmd_tpssmc1_inq.Close();
	//cmd_inq.Close();

	return doFlag;
}
