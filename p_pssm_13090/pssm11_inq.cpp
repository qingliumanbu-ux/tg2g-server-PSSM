/*=========================================================================
//程序名称:     pssm11_inq
//隶属子系统:   PSSM
//产品名称:     BSM1
//创建人员:     JHZHAO
//创建时间:     2012-10-29 17:13:56
//修改人员:		
//修改日期:
//-----------------------------------------------------------------------
//功能描述:		出钢计划查询
//数据库表:     TPSSM11(出钢计划编制表);
//主调用函数:   前台PSSM11画面F2(查询)按钮
//需调用函数:
//=========================================================================*/
#include "stdafx.h"




//#include "tpssmc1.h"


/*<remark>=========================================================
/// <summary>
/// 出钢计划查询
/// <para>1.查询炼钢计划编制表 tpssm11 主表中的记录,由PONO查询子工序的记录。</para>
/// <para>2.查询子工序表, 将内容写入相应的列中                     </para>
/// <para>  1)根据计划子表的内容, 确定记录写入BLOCK的列名；        </para>
/// <para>  2)判断实绩是否接收过,以置实绩标志和取实绩时刻；        </para>
/// <para>  3)写入实绩数据和各子工序实绩标志；                     </para>
/// <para>3.各工序时刻的要求及颜色：
///  1）倒罐时刻： 未开始-黑色(显示开始);  开始-蓝色;  结束-红色(显示结束）
///  2）脱S 时刻： 未开始-黑色(显示开始);  开始-蓝色;  结束-红色(显示结束）
///  3）转炉装入： 未开始-黑色(显示开始);  开始-蓝色;  结束-红色(显示开始）
///         吹炼： 未开始-黑色(显示开始);  开始-蓝色;  结束-红色(显示结束,2015-09-14修改，开始）
///         出钢： 未开始-黑色(显示结束);  开始-蓝色(显示开始);  结束-红色(显示结束）
///  5）精炼时刻： 未开始-黑色(显示开始);  开始-蓝色(显示开始);  结束-红色(显示结束）
///  6）连铸时刻： 开始-红色(显示开始)
/// </para>
/// <para>主调用函数:   前台PSSM11画面F2(查询)按钮。               </para>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_inq)


int f_pssm11_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int blknum;

	CString colname = "";  

	CString order_mode = "";  //排序方式: 1-出钢顺; 2-浇注顺
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	int first_srf; //精炼工序的第一个charge_no
	int early_flag = 0;   //早到时间超标: 0-范围内; 1-超出
	CString start_time = "";
	CDecimal st_no_flag = 0;
	CDecimal l_ca_main_min = 0;
	CDecimal l_ca_main_max = 0;
	CString  factory_div = "";

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmt1("TPSSMT1");
	CModel tpssmd1("TPSSMD1");
	//CTPSSMC1 tpssmc1(conn);

	CString sqlstr = "";
	CString sqlorder = "";
	CDbCommand cmd_inq(conn);  
	CDbCommand cmd_inq1(conn);

	try
	{

		//------------------------------------------
		//设置返回块参数
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("TPSSM_PLAN");
		bcls_ret->Tables[blknum].Columns.Add(tpssm11);   //从实体对象创建架构

		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "PONO_ORG");//下达过计划的PONO
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CAST_NO_SHOW");//连浇号显示
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DP_DEV_NO");   //脱P炉座号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DP_LOAD_START_TIME");//脱磷装入开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DP_BLOW_START_TIME");//脱磷吹炼开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DP_TAP_START_TIME");//脱磷出钢开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DC_DEV_NO");   //脱C炉座号
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DC_LOAD_START_TIME");//脱碳装入开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DC_BLOW_START_TIME");//脱碳吹炼开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DC_TAP_START_TIME");//脱碳出钢开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR1_START_TIME");//精炼1开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR2_START_TIME");//精炼2开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR3_START_TIME");//精炼3开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR4_START_TIME");//精炼4开始时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "LD_ARRIVE_TIME");//连铸包到时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_START_TIME");//连铸开浇时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_END_TIME");  //连铸浇完时刻
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "BOF_REST_TIME");  //转炉设备休辅时间（脱P、脱C共用）
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR_REST_TIME");  //精炼设备休辅时间（n重精炼共用）
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_REST_TIME");  //连铸设备休辅时间

		//为运转开始信号颜色显示用
		// 0-未开始; 1-开始; 2-结束
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "PI_FLAG");   //倒罐
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "DS_FLAG");   //脱硫
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "PRE_SMELT_FLAG1");  //脱P转炉_装入
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "PRE_SMELT_FLAG2");  //脱P转炉_吹炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "PRE_SMELT_FLAG3");  //脱P转炉_出钢
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG1"); //脱C转炉_装入
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG2"); //脱C转炉_吹炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG3"); //脱C转炉_出钢
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR1_FLAG");   //1重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR2_FLAG");   //2重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR3_FLAG");   //3重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR4_FLAG");   //4重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_FLAG1");    //连铸_包到
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_FLAG2");    //连铸_浇注: 1-开浇; 2-浇完
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "EARLY_FLAG"); //早到时间超标: 0-范围内; 1-超出
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "recordCount"); //总记录数
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "ST_NO_FLAG");    //出钢记号是否为CA要求钢种

		
		//--------------------------------------------------------------
		//获得输入参数
		order_mode = bcls_rec->Tables[0].Rows[0]["ORDER_MODE"].ToString().Trim();    //排序方式: 1-出钢顺; 2-浇注顺
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		////Log::Info("", __FUNCTION__, "order_mode = [{0}]", order_mode);
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", factory_div);

		if (order_mode == "1")
		{
			//sqlorder = "ORDER BY STEEL_START_TIME ASC ";  //按转炉开始排序
			sqlorder = "ORDER BY TO_NUMBER(SM_PLAN_NO) ASC ";  //按计划号排序
		}
		else
		{
			sqlorder = " ORDER BY CAST_NO ASC, CAST_DIV_NO ASC ";  //按浇铸顺
		}


		//------------------------------------------
		// 出钢计划查询
		sqlstr = CString(
			//"SELECT a.*, b.* FROM TPSSM11 a LEFT JOIN tpssmt1 b ON( a.SM_PLAN_NO = b.SM_PLAN_NO ) "
			"SELECT * FROM TPSSM11 "
			" WHERE FACTORY_DIV = @factory_div " //83-
			"   AND PONO_STATUS < 83 "
			+ sqlorder
			);
		cmd_inq.SetCommandText(sqlstr);
		////Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("factory_div", factory_div);
		cmd_inq.ExecuteReader();

		fetchRowCount = 0;
		blknum = 0; //第1块

		while (cmd_inq.Read())
		{
			//int k = cmd_inq.Fetch(tpssm11, 1);
			//cmd_inq.Fetch(tpssmt1, k);
			cmd_inq.Fetch(tpssm11);

			////连连标记
			//if (tpssm11["RESTRAND_FLG"].ToString().Trim() == "" &&
			//	(tpssm11["TD_CHG_FLG"].ToDecimal() != 0 || tpssm11["INS_FE_FLAG"].ToString().Trim() != "")
			//	)
			//{
			//	tpssm11["RESTRAND_FLG"] = tpssm11["TD_CHG_FLG"].ToDecimal().ToString() + tpssm11["INS_FE_FLAG"].ToString().Trim();
			//}


			////Log::Trace("", __FUNCTION__, "SM_PLAN_NO=[{0}][{1}]", tpssm11["SM_PLAN_NO"].ToString(), tpssmt1["SM_PLAN_NO"].ToString());
			CDataRow& row = bcls_ret->Tables["TPSSM_PLAN"].Rows.Add();
			row.Merge(tpssm11);


			row["CAST_NO_SHOW"] = tpssm11["CAST_NO"].ToString().Trim() + "-" + tpssm11["CAST_DIV_NO"].ToDecimal().ToString();


//			//查询下达过计划的PONO，传前台作为原PONO项
//			sqlstr = CString(
//				"SELECT PONO FROM TPSSM13 WHERE SM_PLAN_NO = @sm_plan_no "
//				);
//			cmd_inq1.SetCommandText(sqlstr);
//			cmd_inq1.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString().ToInt32());
//			cmd_inq1.ExecuteReader();
//			if (cmd_inq1.Read())
//			{
//				row["PONO_ORG"] = cmd_inq1.GetString(1).Trim();
//			}
//			else
//			{
//				row["PONO_ORG"] = "";
//			}
//			cmd_inq1.Close();

			//////Log::Trace("", __FUNCTION__, "查询子工序：SM_PLAN_NO=[{0}], PONO=[{1}]", tpssm11["SM_PLAN_NO"].ToString(), tpssm11["PONO"].ToString());

			//精炼工序的第一个charge_no 必定不为0
			first_srf = 0;
			//------------------------------------------
			//查询出钢计划子表, 将炉次、时间等内容写入相应的列中
			sqlstr = CString(
				"SELECT * FROM TPSSM12 "
				" WHERE FACTORY_DIV = @factory_div  "
				"   AND SM_PLAN_NO = @sm_plan_no  "
				" ORDER BY CHARGE_NO ASC "
				);
			cmd_inq1.SetCommandText(sqlstr);
			cmd_inq1.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_inq1.Parameters.Set("sm_plan_no", tpssm11["SM_PLAN_NO"].ToString());
			cmd_inq1.ExecuteReader();

			while (cmd_inq1.Read())
			{
				cmd_inq1.Fetch(tpssm12);

				//////Log::Trace("", __FUNCTION__, "pono=[{0}], charge_no=[{1}], dev_code=[{2}]", tpssm12.PONO, tpssm12["CHARGE_NO"].ToDecimal().ToInt32(), tpssm12["DEV_CODE"].ToString());
				
				tpssm12["REST_TIME"] = tpssm12["REST_TIME"].ToDecimal().ToInt32(); //去小数点

				//根据计划子表的内容, 确定记录写入BLOCK的列名
				int tpssm12_charge_no = 0;
				CString str = "";
				switch (tpssm12["AREA_ID"].ToDecimal().ToInt32())
				{
				case 2: //脱磷
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 1) //装入： 开始-蓝色;  结束-红色(显示开始）
					{
						//if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						//{
							if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
							{
								row["DP_LOAD_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row["DP_LOAD_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
							}
							
						//}
						//else
						//{
						//	row["DP_LOAD_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						//}
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 2)  //吹炼： 开始-蓝色;  结束-红色(显示结束）
					{
						if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						{
							if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
							{
								row["DP_BLOW_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row["DP_BLOW_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
							}
							
						}
						else
						{
							row["DP_BLOW_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
						
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 3) //出钢： 未开始-黑色(显示结束);  开始-蓝色(显示开始);  结束-红色(显示结束）
					{
						if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						{
							if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
							{
								row["DP_TAP_START_TIME"] = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row["DP_TAP_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
							}
							
						}
						else
						{
							row["DP_TAP_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
					
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 0) //主工序
					{
						//返回脱磷转炉号
						row["DP_DEV_NO"] = tpssm12["DEV_CODE"].ToString().SubstringNE(1, 1);
						//如果有休辅时间，则返回
						if (tpssm12["REST_TIME"].ToDecimal() > 0)
							row["BOF_REST_TIME"] = tpssm12["REST_TIME"].ToDecimal().ToString();

						//20161212，由于目前不区分转炉工序所以都记录在主工序下
						if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
						{
							row["DC_BLOW_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
						}
						else
						{
							row["DC_BLOW_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
						}

						if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						{
							row["DC_TAP_START_TIME"] = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
						}
						else
						{
							row["DC_TAP_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
					}
					break;

				case 3: //转炉区
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 1) //装入： 开始-蓝色;  结束-红色(显示开始）
					{
						//if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						//{
							if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
							{
								row["DC_LOAD_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row["DC_LOAD_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
							}
							
						//}
						//else
						//{
						//	row["DC_LOAD_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						//}
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 2)  //吹炼： 开始-蓝色;  结束-红色(显示结束）
					{
						if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						{
							if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
							{
								row["DC_BLOW_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row["DC_BLOW_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
							}
							
						}
						else
						{
							row["DC_BLOW_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 3)  //出钢： 开始-蓝色;  结束-红色(显示结束）
					{

						if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						{
							if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
							{
								row["DC_TAP_START_TIME"] = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row["DC_TAP_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
							}
						}
						else
						{
							row["DC_TAP_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 0) //主工序
					{
						//返回脱C转炉号
						row["DC_DEV_NO"] = tpssm12["DEV_CODE"].ToString().SubstringNE(1, 1);
						//如果有休辅时间，则返回
						if (tpssm12["REST_TIME"].ToDecimal() > 0)
							row["BOF_REST_TIME"] = tpssm12["REST_TIME"].ToDecimal().ToString();

						//20161212，由于目前不区分转炉工序所以都记录在主工序下
						if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
						{
							row["DC_BLOW_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
						}
						else
						{
							row["DC_BLOW_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
						}

						if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
						{
							row["DC_TAP_START_TIME"] = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
						}
						else
						{
							row["DC_TAP_START_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
						}
					}
					break;

				case 4: //精炼区域
					tpssm12_charge_no = tpssm12["CHARGE_NO"].ToDecimal().ToInt32();
					if (first_srf == 0)
					{
						first_srf = tpssm12_charge_no;
					}
					str = "SR" + CDecimal(tpssm12_charge_no - first_srf + 1).ToString();
					colname = str + "_START_TIME";
					if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")  //开始-蓝色;  结束-红色(显示结束）
					{
						if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
						{
							//2015-10-14 增加精炼包到时刻
							if (tpssm12["ARRIVE_REAL_TIME"].ToString().Trim() != "")
							{
								row[colname] = tpssm12["ARRIVE_REAL_TIME"].ToString().SubstringNE(8, 4);
							}
							else
							{
								row[colname] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
							}
						}
						else
						{
							row[colname] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
						}	
					}
					else
					{
						row[colname] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
					}

					//如果有休辅时间，则返回
					if (tpssm12["REST_TIME"].ToDecimal() > 0)
						row["SR_REST_TIME"] = tpssm12["REST_TIME"].ToDecimal().ToString();
					break;

				case 5: //浇铸
					//包到
					if (tpssm12["ARRIVE_REAL_TIME"].ToString().Trim() == "")
					{
						row["LD_ARRIVE_TIME"] = tpssm12["LEAVE_REAL_TIME"].ToString().SubstringNE(8, 4);
					}
					else
					{
						row["LD_ARRIVE_TIME"] = tpssm12["ARRIVE_REAL_TIME"].ToString().SubstringNE(8, 4);
					}
					//开始
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
					{
						row["CC_START_TIME"] = tpssm12["START_TIME"].ToString().SubstringNE(8, 4);
					}
					else
					{
						row["CC_START_TIME"] = tpssm12["START_TIME_REAL"].ToString().SubstringNE(8, 4);
					}
					//结束
					if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
					{						
						row["CC_END_TIME"] = tpssm12["END_TIME"].ToString().SubstringNE(8, 4);
					}
					else
					{
						row["CC_END_TIME"] = tpssm12["END_TIME_REAL"].ToString().SubstringNE(8, 4);
					}

					//如果有休辅时间，则返回
					if (tpssm12["REST_TIME"].ToDecimal() > 0)
						row["SR_REST_TIME"] = tpssm12["REST_TIME"].ToDecimal().ToString();
					break;
				}//switch


				//----------------------------------------------------
				//运转信号颜色设置
				// 0-未开始; 1-开始; 2-结束
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "")
				{
					if (tpssm12["START_TIME_REAL"].ToString().Trim() == "")
					{
						str_show_flag = "0";

						//2015-10-14 增加精炼包到状态
						if (tpssm12["AREA_ID"].ToDecimal() == 4 && tpssm12["ARRIVE_REAL_TIME"].ToString().Trim() != "")
						{
							str_show_flag = "3";  //3-精炼包到
						}
					}
					else
					{
						str_show_flag = "1";
					}

				}
				else
				{
					str_show_flag = "2";
				}

				//////Log::Trace("", __FUNCTION__, "str_show_flag=[{0}]", str_show_flag);

				switch (tpssm12["AREA_ID"].ToDecimal().ToInt32())
				{
				case 2: //脱P转炉
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 1)
					{
						row["PRE_SMELT_FLAG1"] = str_show_flag;
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 2)
					{
						row["PRE_SMELT_FLAG2"] = str_show_flag;
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 3)
					{
						row["PRE_SMELT_FLAG3"] = str_show_flag;
					}
					
					break;
				case 3: //脱C转炉
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 1)//装入
					{
						row["MAIN_SMELT_FLAG1"] = str_show_flag;
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 2)//吹炼
					{
						row["MAIN_SMELT_FLAG2"] = str_show_flag;
					}
					if (tpssm12["SUB_CHARGE_NO"].ToDecimal() == 3)//出钢
					{
						row["MAIN_SMELT_FLAG3"] = str_show_flag;
					}
					break;
				case 4: //精炼
					str = "SR" + CDecimal(tpssm12_charge_no - first_srf + 1).ToString();
					colname = str + "_FLAG";
					row[colname] = str_show_flag;
					break;
				case 5: //连铸
					if (tpssm12["ARRIVE_REAL_TIME"].ToString().Trim() != "")
					{
						row["CC_FLAG1"] = "1"; //1-包到
					}
					if (tpssm12["LEAVE_REAL_TIME"].ToString().Trim() != "")
					{
						row["CC_FLAG1"] = "2"; //2-包离
					}

					row["CC_FLAG2"] = str_show_flag;  //1-开浇; 2-浇完
					break;
				}

			}//while tpssm12
			cmd_inq1.Close();


			//////Log::Trace("", __FUNCTION__, "AAAAAAAAAAAAA");

			//------------------------------------------
			////根据铁水段作业信息，写入倒罐、脱硫时刻
			//row["DS_END_TIME"] = tpssm11.DS_END_TIME.SubstringNE(8, 4);
			//row["PI_END_TIME"] = tpssm11.PI_END_TIME.SubstringNE(8, 4);

			tpssmt1.Reset();
			sqlstr = CString(
				"SELECT * FROM TPSSMT1 "
				" WHERE FACTORY_DIV = @factory_div "
				"   AND PONO = @pono "
				);
			cmd_inq1.SetCommandText(sqlstr);
			cmd_inq1.Parameters.Set("factory_div", tpssm11["FACTORY_DIV"].ToString());
			cmd_inq1.Parameters.Set("pono", tpssm11["PONO"].ToString());
			cmd_inq1.ExecuteReader();
			if (cmd_inq1.Read())
			{
				cmd_inq1.Fetch(tpssmt1);
			}
			cmd_inq1.Close();

			if (tpssmt1["PLAN_NO"].ToString().Trim() != "") //有计划号
			{

				//倒罐时刻： 开始-蓝色;  结束-红色(显示开始） 2015-06-25
				//倒罐时刻： 未开始 - 黑色(显示开始);  开始 - 蓝色;  结束 - 红色(显示结束） 2015-09-01
				if (tpssmt1["TPD_END_TIME"].ToString().Trim() == "")  //倒罐未结束
				{

					if (tpssmt1["TPD_START_TIME"].ToString().Trim() == "")  //倒罐未开始
					{
						//row["PI_END_TIME"] = tpssmt1["TPD_END_TIME"].ToString().SubstringNE(8, 4);  //赋计划值，前边已赋值
						row["PI_END_TIME"] = tpssmt1["TPD_PLAN_END_TIME"].ToString().SubstringNE(8, 4); //2015-09-01
						str_show_flag = "0";  //未开始
					}
					else
					{
						row["PI_END_TIME"] = tpssmt1["TPD_START_TIME"].ToString().SubstringNE(8, 4);  //赋实际开始时刻
						str_show_flag = "1";  //开始
					}

				}
				else
				{
					row["PI_END_TIME"] = tpssmt1["TPD_END_TIME"].ToString().SubstringNE(8, 4);  //赋实际结束时刻
					//row["PI_END_TIME"] = tpssmt1["TPD_START_TIME"].ToString().SubstringNE(8, 4);  //赋实际开始时刻
					str_show_flag = "2";  //结束
				}
				row["PI_FLAG"] = str_show_flag;

				//////Log::Trace("", __FUNCTION__, "str_show_flag22=[{0}]", str_show_flag);

				//脱硫时刻： 开始-蓝色;  结束-红色(显示开始）   2015-06-25
				//脱S 时刻： 未开始-黑色(显示开始);  开始-蓝色;  结束-红色(显示结束） 2015-09-01
				if (tpssmt1["DES_END_TIME"].ToString().Trim() == "")
				{

					if (tpssmt1["DES_START_TIME"].ToString().Trim() == "")
					{
						row["DS_END_TIME"] = tpssmt1["DES_END_TIME"].ToString().SubstringNE(8, 4);  //赋计划值，前边已赋值
						//row["DS_END_TIME"] = tpssmt1.DS_PLAN_START_TIME.SubstringNE(8, 4);  //2015-09-01
						str_show_flag = "0";  //未开始
					}
					else
					{
						row["DS_END_TIME"] = tpssmt1["DES_START_TIME"].ToString().SubstringNE(8, 4);  //赋实际开始时刻
						str_show_flag = "1";  //开始
					}

				}
				else
				{
					row["DS_END_TIME"] = tpssmt1["DES_END_TIME"].ToString().SubstringNE(8, 4);  //赋实际结束时刻
					//row["DS_END_TIME"] = tpssmt1.DS_START_TIME.SubstringNE(8, 4);  //赋实际开始时刻
					str_show_flag = "2";  //结束
				}
				row["DS_FLAG"] = str_show_flag;

			}//if 铁水段

//			//增加出钢记号是否有CA要求标记
//			st_no_flag = 0;
//			l_ca_main_min = 0;
//			l_ca_main_max = 0;
//			sqlstr = CString(
//				"select CA_MAIN_MIN, CA_MAIN_MAX from tqmts02 where ST_NO = @st_no and WHOLE_BACKLOG_CODE = 'G'"
//				);
//			cmd_inq1.SetCommandText(sqlstr);
//			cmd_inq1.Parameters.Set("st_no", tpssm11["ST_NO"].ToString());
//			cmd_inq1.ExecuteReader();
//			if (cmd_inq1.Read())
//			{
//				l_ca_main_min = cmd_inq1.GetDecimal(1);
//				l_ca_main_max = cmd_inq1.GetDecimal(2);
//			}
//			cmd_inq1.Close();
//
//			if (l_ca_main_min>0 || l_ca_main_max>0)
//			{
//				st_no_flag = 1;
//			}
//			row["ST_NO_FLAG"] = st_no_flag;
			fetchRowCount++;

		}//while tpssm11
		cmd_inq.Close();

		//////Log::Trace("", __FUNCTION__, "VVVVVVVVVVVVV");

		if (fetchRowCount>0)
		{
			bcls_ret->Tables[blknum].Rows[0]["recordCount"] = fetchRowCount;
		}

		//////Log::Trace("", __FUNCTION__, "query records. [{0}]", bcls_ret->Tables[0].Rows.get_Count());

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
