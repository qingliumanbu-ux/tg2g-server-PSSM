/*=========================================================================
//程序名称:     pssm13_inq
//隶属子系统:   PSSM
//产品名称:     BSM1
//创建人员:     JHZHAO
//创建时间:     2015-01-16 17:13:56
//修改人员:
//修改日期:
//-----------------------------------------------------------------------
//功能描述:		出钢计划监控
//数据库表:     TPSSM13(出钢计划编制表);
//主调用函数:   前台PSSM11画面F2(查询)按钮
//需调用函数:
//=========================================================================*/
#include "stdafx.h"
#include "tpssm13.h"
#include "tpssm14.h"
#include "tpssm19.h"  //铁水段计划与跟踪
#include "tpssmd1.h"
#include "tpssmc1.h"


/*<remark>=========================================================
/// <summary>
/// 出钢计划查询
/// <para>1.查询炼钢计划编制表 tpssm13 主表中的记录,由PONO查询子工序的记录。</para>
/// <para>2.查询子工序表, 将内容写入相应的列中                     </para>
/// <para>  1)根据计划子表的内容, 确定记录写入BLOCK的列名；        </para>
/// <para>  2)判断实绩是否接收过,以置实绩标志和取实绩时刻；        </para>
/// <para>  3)写入实绩数据和各子工序实绩标志；                     </para>
/// <para>3.对转炉区,要送出转炉名称,并将各子工序送出。             </para>
/// <para>主调用函数:   前台PSSM11画面F2(查询)按钮。               </para>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm13_inq)


int f_pssm13_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	int   fetchRowCount;
	int   blknum;

	CString colname = "";

	CString ccm_no;
	CString str_show_flag = ""; //为运转开始信号颜色显示用
	int first_srf; //精炼工序的第一个charge_no
	int  early_flag = 0;   //早到时间超标: 0-范围内; 1-超出
	CString start_time = "";

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	CDbCommand cmd_inq2(conn);

	CTracer log(__FUNCTION__);

	try
	{
		// 定义表的实体对象
		CTPSSM13 tpssm13(conn);
		CTPSSM14 tpssm14(conn);
		CTPSSM19 tpssm19(conn);
		CTPSSMD1 tpssmd1(conn);
		CTPSSMC1 tpssmc1(conn);


		//------------------------------------------
		//设置返回块参数
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("TPSSM_PLAN");
		bcls_ret->Tables[blknum].Columns.Add(tpssm13);   //从实体对象创建架构

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
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG2"); //脱C转炉_装入
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "MAIN_SMELT_FLAG3"); //脱C转炉_装入
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR1_FLAG");   //1重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR2_FLAG");   //2重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR3_FLAG");   //3重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "SR4_FLAG");   //4重精炼
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_FLAG1");    //连铸_包到
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "CC_FLAG2");    //连铸_浇注
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "EARLY_FLAG"); //早到时间超标: 0-范围内; 1-超出
		bcls_ret->Tables[blknum].Columns.Add(DT_DECIMAL, "recordCount"); //总记录数

		//------------------------------------------
		// 出钢计划查询
		sqlstr = CString(
			//"SELECT a.* FROM TPSSM13 a "
			"SELECT a.*, b.* FROM TPSSM13 a LEFT JOIN TPSSM19 b "
			" ON( a.SM_PLAN_NO = b.SM_PLAN_NO ) "
			" ORDER BY a.STEEL_START_TIME ASC "            //按转炉开始排序
			//" ORDER BY CAST_NO ASC, CAST_DIV_NO ASC "  //按浇铸顺
			);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();

		fetchRowCount = 0;
		blknum = 0; //第1块

		while (cmd_inq.Read())
		{
			//tpssm19.Reset();

			int k = cmd_inq.Fetch(tpssm13, 1);  //把数据都压在头文件里面
			cmd_inq.Fetch(tpssm19, k);

			Log::Trace("", __FUNCTION__, "SM_PLAN_NO=[{0}][{1}]，fetchRowCount=[{2}]", tpssm13.SM_PLAN_NO, tpssm19.SM_PLAN_NO, fetchRowCount);

			CDataRow& row1 = bcls_ret->Tables[blknum].Rows.Add();
			CDataRow& row2 = bcls_ret->Tables[blknum].Rows.Add();

			CDataRow * prow_plan = &(bcls_ret->Tables[blknum].Rows[fetchRowCount * 2]);
			CDataRow * prow_fact = &(bcls_ret->Tables[blknum].Rows[fetchRowCount * 2 + 1]);
			prow_plan->Merge(tpssm13);  //计划行
			prow_fact->Merge(tpssm13);  //实际行

			(*prow_plan)["DS_END_TIME"] = tpssm13.DS_END_TIME.SubstringNE(8, 4);
			(*prow_plan)["PI_END_TIME"] = tpssm13.PI_END_TIME.SubstringNE(8, 4);
			(*prow_plan)["CAST_NO_SHOW"] = tpssm13.CAST_NO.Trim() + "-" + tpssm13.CAST_DIV_NO.ToString();

			(*prow_fact)["DS_END_TIME"] = " ";
			(*prow_fact)["PI_END_TIME"] = " ";
			(*prow_fact)["CAST_NO_SHOW"] = tpssm13.CAST_NO.Trim() + "-" + tpssm13.CAST_DIV_NO.ToString();

			//精炼工序的第一个charge_no 必定不为0
			first_srf = 0;
			//------------------------------------------
			//查询出钢计划子表, 将炉次、时间等内容写入相应的列中
			sqlstr = "SELECT * FROM TPSSM14 "
				" WHERE  PONO = @tpssm13.pono "
				" ORDER BY CHARGE_NO ASC ";
			cmd_inq1.SetCommandText(sqlstr);
			cmd_inq1.Parameters.Set("tpssm13.pono", tpssm13.PONO.Trim());
			cmd_inq1.ExecuteReader();

			while (cmd_inq1.Read())
			{
				cmd_inq1.Fetch(tpssm14);

				Log::Trace("", __FUNCTION__, "pono=[{0}], charge_no=[{1}], dev_code=[{2}]", tpssm14.PONO, tpssm14.CHARGE_NO.ToInt32(), tpssm14.DEV_CODE);

				tpssm14.REST_TIME = tpssm14.REST_TIME.ToInt32(); //去小数点

				//根据计划子表的内容, 确定记录写入BLOCK的列名
				int tpssm14_charge_no = 0;
				CString str = "";
				switch (tpssm14.AREA_ID.ToInt32())
				{
				case 2: //脱磷
					if (tpssm14.SUB_CHARGE_NO == 1)
					{
						(*prow_plan)["DP_LOAD_START_TIME"] = tpssm14.END_TIME.SubstringNE(8, 4);

						//实际
						if (tpssm14.END_TIME_REAL.Trim() == "")
						{
							(*prow_fact)["DP_LOAD_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
						}
						else
						{
							(*prow_fact)["DP_LOAD_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
						}
						
					}
					else if (tpssm14.SUB_CHARGE_NO == 2)
					{
						(*prow_plan)["DP_BLOW_START_TIME"] = tpssm14.END_TIME.SubstringNE(8, 4);

						//实际
						if (tpssm14.END_TIME_REAL.Trim() == "")
						{
							(*prow_fact)["DP_BLOW_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
						}
						else
						{
							(*prow_fact)["DP_BLOW_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
						}

					}
					else if (tpssm14.SUB_CHARGE_NO == 3)
					{
						(*prow_plan)["DP_TAP_START_TIME"] = tpssm14.END_TIME.SubstringNE(8, 4);

						//实际
						if (tpssm14.END_TIME_REAL.Trim() == "")
						{
							(*prow_fact)["DP_TAP_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
						}
						else
						{
							(*prow_fact)["DP_TAP_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
						}

					}
					if (tpssm14.SUB_CHARGE_NO == 0) //主工序
					{
						//返回脱磷转炉号
						(*prow_plan)["DP_DEV_NO"] = tpssm14.DEV_CODE.SubstringNE(1, 1);
						(*prow_fact)["DP_DEV_NO"] = tpssm14.DEV_CODE.SubstringNE(1, 1);
						//如果有休辅时间，则返回
						if (tpssm14.REST_TIME > 0)
							(*prow_plan)["BOF_REST_TIME"] = tpssm14.REST_TIME.ToString();
					}
					break;
				case 3: //转炉区
					if (tpssm14.SUB_CHARGE_NO == 1)
					{
						(*prow_plan)["DC_LOAD_START_TIME"] = tpssm14.END_TIME.SubstringNE(8, 4);
						//实际
						if (tpssm14.END_TIME_REAL.Trim() == "")
						{
							(*prow_fact)["DC_LOAD_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
						}
						else
						{
							(*prow_fact)["DC_LOAD_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
						}

					}
					else if (tpssm14.SUB_CHARGE_NO == 2)
					{
						(*prow_plan)["DC_BLOW_START_TIME"] = tpssm14.END_TIME.SubstringNE(8, 4);
						//实际
						if (tpssm14.END_TIME_REAL.Trim() == "")
						{
							(*prow_fact)["DC_BLOW_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
						}
						else
						{
							(*prow_fact)["DC_BLOW_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
						}
					}
					else if (tpssm14.SUB_CHARGE_NO == 3)
					{
						(*prow_plan)["DC_TAP_START_TIME"] = tpssm14.END_TIME.SubstringNE(8, 4);
						//实际
						if (tpssm14.END_TIME_REAL.Trim() == "")
						{
							(*prow_fact)["DC_TAP_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
						}
						else
						{
							(*prow_fact)["DC_TAP_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
						}
					}
					if (tpssm14.SUB_CHARGE_NO == 0) //主工序
					{
						//返回脱C转炉号
						(*prow_plan)["DC_DEV_NO"] = tpssm14.DEV_CODE.SubstringNE(1, 1);
						(*prow_fact)["DC_DEV_NO"] = tpssm14.DEV_CODE.SubstringNE(1, 1);
						//如果有休辅时间，则返回
						if (tpssm14.REST_TIME > 0)
							(*prow_plan)["BOF_REST_TIME"] = tpssm14.REST_TIME.ToString();
					}
					break;

				case 4: //精炼区域
					tpssm14_charge_no = tpssm14.CHARGE_NO.ToInt32();
					if (first_srf == 0)
					{
						first_srf = tpssm14_charge_no;
					}
					str = "SR" + CDecimal(tpssm14_charge_no - first_srf + 1).ToString();
					colname = str + "_START_TIME";

					(*prow_plan)[colname] = tpssm14.END_TIME.SubstringNE(8, 4);
					//实际
					if (tpssm14.END_TIME_REAL.Trim() == "")
					{
						(*prow_fact)[colname] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
					}
					else
					{
						(*prow_fact)[colname] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
					}

					//如果有休辅时间，则返回
					if (tpssm14.REST_TIME > 0)
						(*prow_plan)["SR_REST_TIME"] = tpssm14.REST_TIME.ToString();

					break;

				case 5: //浇铸

					(*prow_plan)["LD_ARRIVE_TIME"] = tpssm14.LADLE_ARRIVE_TIME.SubstringNE(8, 4);
					(*prow_plan)["CC_START_TIME"] = tpssm14.START_TIME.SubstringNE(8, 4);
					(*prow_plan)["CC_END_TIME"]   = tpssm14.END_TIME.SubstringNE(8, 4);
					//如果有休辅时间，则返回
					if (tpssm14.REST_TIME > 0)
						(*prow_plan)["SR_REST_TIME"] = tpssm14.REST_TIME.ToString();

					//实际
					(*prow_fact)["LD_ARRIVE_TIME"] = tpssm14.ARRIVE_TIME_REAL.SubstringNE(8, 4);
					(*prow_fact)["CC_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
					(*prow_fact)["CC_END_TIME"]   = tpssm14.END_TIME_REAL.SubstringNE(8, 4);

					break;
				}//switch


				//----------------------------------------------------
				//运转信号颜色设置
				// 0-未开始; 1-开始; 2-结束
				str_show_flag = "0";
				//计划无颜色
				(*prow_plan)["PI_FLAG"] = str_show_flag;
				(*prow_plan)["DS_FLAG"] = str_show_flag;
				(*prow_plan)["PRE_SMELT_FLAG1"] = str_show_flag;
				(*prow_plan)["PRE_SMELT_FLAG2"] = str_show_flag;
				(*prow_plan)["PRE_SMELT_FLAG3"] = str_show_flag;
				(*prow_plan)["MAIN_SMELT_FLAG1"] = str_show_flag;
				(*prow_plan)["MAIN_SMELT_FLAG2"] = str_show_flag;
				(*prow_plan)["MAIN_SMELT_FLAG3"] = str_show_flag;
				(*prow_plan)["SR1_FLAG"] = str_show_flag;
				(*prow_plan)["SR2_FLAG"] = str_show_flag;
				(*prow_plan)["SR3_FLAG"] = str_show_flag;
				(*prow_plan)["SR4_FLAG"] = str_show_flag;
				(*prow_plan)["CC_FLAG1"] = str_show_flag;
				(*prow_plan)["CC_FLAG2"] = str_show_flag;
				//Log::Trace("", __FUNCTION__, "str_show_flag=[{0}]", str_show_flag);

				//------------------实际状态------------------------
				//运转信号颜色设置
				// 0-未开始; 1-开始; 2-结束
				if (tpssm14.END_TIME_REAL.Trim() == "")
				{
					if (tpssm14.START_TIME_REAL.Trim() == "")
					{
						str_show_flag = "0";
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

				switch (tpssm14.AREA_ID.ToInt32())
				{
				case 2: //脱P转炉
					if (tpssm14.SUB_CHARGE_NO == 1)
					{
						(*prow_fact)["PRE_SMELT_FLAG1"] = str_show_flag;
					}
					if (tpssm14.SUB_CHARGE_NO == 2)
					{
						(*prow_fact)["PRE_SMELT_FLAG2"] = str_show_flag;
					}
					if (tpssm14.SUB_CHARGE_NO == 3)
					{
						(*prow_fact)["PRE_SMELT_FLAG3"] = str_show_flag;
					}

					break;
				case 3: //脱C转炉
					if (tpssm14.SUB_CHARGE_NO == 1)
					{
						(*prow_fact)["MAIN_SMELT_FLAG1"] = str_show_flag;
					}
					if (tpssm14.SUB_CHARGE_NO == 2)
					{
						(*prow_fact)["MAIN_SMELT_FLAG2"] = str_show_flag;
					}
					if (tpssm14.SUB_CHARGE_NO == 3)
					{
						(*prow_fact)["MAIN_SMELT_FLAG3"] = str_show_flag;
					}
					break;
				case 4: //精炼
					str = "SR" + CDecimal(tpssm14_charge_no - first_srf + 1).ToString();
					colname = str + "_FLAG";
					(*prow_fact)[colname] = str_show_flag;
					break;
				case 5: //连铸
					if (tpssm14.ARRIVE_TIME_REAL.Trim() != "")
					{
						(*prow_fact)["CC_FLAG1"] = "1";
					}
					if (tpssm14.LEAVE_TIME_REAL.Trim() != "")
					{
						(*prow_fact)["CC_FLAG1"] = "2";
					}
					(*prow_fact)["CC_FLAG2"] = str_show_flag;
					break;
				}
				//------------------实际状态结束----------------------------

				//------------------铁水段作业信息--------------------------
				//根据铁水段作业信息，写入倒罐、脱硫信息
				if (tpssm19.SM_PLAN_NO > 0) //有计划号
				{

					//倒罐时刻
					if (tpssm19.PI_END_TIME.Trim() == "")  //倒罐未结束
					{

						if (tpssm19.PI_START_TIME.Trim() == "")  //倒罐未开始
						{
							//(*prow_fact)["PI_END_TIME"] = tpssm13.PI_END_TIME.SubstringNE(8, 4);  //赋计划值，前边已赋值
							str_show_flag = "0";  //未开始
						}
						else
						{
							(*prow_fact)["PI_END_TIME"] = tpssm19.PI_START_TIME.SubstringNE(8, 4);  //赋实际开始时刻
							str_show_flag = "1";  //开始
						}

					}
					else
					{
						(*prow_fact)["PI_END_TIME"] = tpssm19.PI_END_TIME.SubstringNE(8, 4);  //赋实际结束时刻
						str_show_flag = "2";  //结束
					}
					(*prow_fact)["PI_FLAG"] = str_show_flag;


					//脱硫时刻
					if (tpssm19.DS_END_TIME.Trim() == "")
					{

						if (tpssm19.DS_START_TIME.Trim() == "")
						{
							//(*prow_fact)["DS_END_TIME"] = tpssm13.DS_END_TIME.SubstringNE(8, 4);  //赋计划值，前边已赋值
							str_show_flag = "0";  //未开始
						}
						else
						{
							(*prow_fact)["DS_END_TIME"] = tpssm19.DS_START_TIME.SubstringNE(8, 4);  //赋实际开始时刻
							str_show_flag = "1";  //开始
						}

					}
					else
					{
						(*prow_fact)["DS_END_TIME"] = tpssm19.DS_END_TIME.SubstringNE(8, 4);  //赋实际结束时刻
						str_show_flag = "2";  //结束
					}
					(*prow_fact)["DS_FLAG"] = str_show_flag;

				}//if 铁水段
				//------------------铁水段作业信息--------------------------


			}//while tpssm14
			cmd_inq1.Close();


			////------------------实际值监控开始------------------------
			//CDataRow& row1 = bcls_ret->Tables[blknum].Rows.Add();
			//row1.Merge(tpssm13);
			//row1["DS_END_TIME"] = tpssm13.DS_END_TIME.SubstringNE(8, 4);
			//row1["PI_END_TIME"] = tpssm13.PI_END_TIME.SubstringNE(8, 4);
			//row1["CAST_NO_SHOW"] = tpssm13.CAST_NO.Trim() + "-" + tpssm13.CAST_DIV_NO.ToString();

			////精炼工序的第一个charge_no 必定不为0
			//first_srf = 0;
			////------------------------------------------
			////查询出钢计划子表, 将炉次、时间等内容写入相应的列中
			//sqlstr = "SELECT * FROM tpssm14 "
			//	" WHERE  PONO = @tpssm13.pono "
			//	" ORDER BY CHARGE_NO ASC ";
			//cmd_inq1.SetCommandText(sqlstr);
			//cmd_inq1.Parameters.Set("tpssm13.pono", tpssm13.PONO.Trim());
			//cmd_inq1.ExecuteReader();

			//while (cmd_inq1.Read())
			//{
			//	cmd_inq1.Fetch(tpssm14);

			//	Log::Trace("", __FUNCTION__, "pono=[{0}], charge_no=[{1}], dev_code=[{2}]", tpssm14.PONO, tpssm14.CHARGE_NO.ToInt32(), tpssm14.DEV_CODE);

			//	//根据计划子表的内容, 确定记录写入BLOCK的列名
			//	int tpssm14_charge_no = 0;
			//	CString str = "";
			//	switch (tpssm14.AREA_ID.ToInt32())
			//	{
			//	case 2: //脱磷
			//		if (tpssm14.SUB_CHARGE_NO == 1)
			//		{
			//			if (tpssm14.END_TIME_REAL.Trim() == "")
			//			{
			//				row1["DP_LOAD_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			else
			//			{
			//				row1["DP_LOAD_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 2)
			//		{
			//			if (tpssm14.END_TIME_REAL.Trim() == "")
			//			{
			//				row1["DP_BLOW_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			else
			//			{
			//				row1["DP_BLOW_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 3)
			//		{
			//			if (tpssm14.END_TIME_REAL.Trim() == "")
			//			{
			//				row1["DP_TAP_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			else
			//			{
			//				row1["DP_TAP_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			
			//		}
			//		//返回脱磷炉号
			//		row1["DP_DEV_NO"] = tpssm14.DEV_CODE.SubstringNE(1, 1);
			//		break;
			//	case 3: //转炉区
			//		if (tpssm14.SUB_CHARGE_NO == 1)
			//		{
			//			if (tpssm14.END_TIME_REAL.Trim() == "")
			//			{
			//				row1["DC_LOAD_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			else
			//			{
			//				row1["DC_LOAD_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 2)
			//		{
			//			if (tpssm14.END_TIME_REAL.Trim() == "")
			//			{
			//				row1["DC_BLOW_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			else
			//			{
			//				row1["DC_BLOW_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 3)
			//		{
			//			if (tpssm14.END_TIME_REAL.Trim() == "")
			//			{
			//				row1["DC_TAP_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			else
			//			{
			//				row1["DC_TAP_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//			}
			//			
			//		}
			//		//返回脱C转炉号
			//		row1["DC_DEV_NO"] = tpssm14.DEV_CODE.SubstringNE(1, 1);
			//		break;
			//	case 4: //精炼区域
			//		tpssm14_charge_no = tpssm14.CHARGE_NO.ToInt32();
			//		if (first_srf == 0)
			//		{
			//			first_srf = tpssm14_charge_no;
			//		}
			//		str = "SR" + CDecimal(tpssm14_charge_no - first_srf + 1).ToString();
			//		colname = str + "_START_TIME";
			//		if (tpssm14.END_TIME_REAL.Trim() == "")
			//		{
			//			row1[colname] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//		}
			//		else
			//		{
			//			row1[colname] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//		}

			//		break;
			//	case 5: //浇铸

			//		row1["LD_ARRIVE_TIME"] = tpssm14.ARRIVE_TIME_REAL.SubstringNE(8, 4);
			//		if (tpssm14.END_TIME_REAL.Trim() == "")
			//		{
			//			row1["CC_START_TIME"] = tpssm14.START_TIME_REAL.SubstringNE(8, 4);
			//		}
			//		else
			//		{
			//			row1["CC_START_TIME"] = tpssm14.END_TIME_REAL.SubstringNE(8, 4);
			//		}

			//		break;
			//	}//switch


			//	//----------------------------------------------------
			//	//运转信号颜色设置
			//	// 0-未开始; 1-开始; 2-结束
			//	if (tpssm14.END_TIME_REAL.Trim() == "")
			//	{
			//		if (tpssm14.START_TIME_REAL.Trim() == "")
			//		{
			//			str_show_flag = "0";
			//		}
			//		else
			//		{
			//			str_show_flag = "1";
			//		}

			//	}
			//	else
			//	{
			//		str_show_flag = "2";
			//	}

			//	//Log::Trace("", __FUNCTION__, "str_show_flag=[{0}]", str_show_flag);

			//	switch (tpssm14.AREA_ID.ToInt32())
			//	{
			//	case 2: //脱P转炉
			//		if (tpssm14.SUB_CHARGE_NO == 1)
			//		{
			//			row1["PRE_SMELT_FLAG1"] = str_show_flag;
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 2)
			//		{
			//			row1["PRE_SMELT_FLAG2"] = str_show_flag;
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 3)
			//		{
			//			row1["PRE_SMELT_FLAG3"] = str_show_flag;
			//		}

			//		break;
			//	case 3: //脱C转炉
			//		if (tpssm14.SUB_CHARGE_NO == 1)
			//		{
			//			row1["MAIN_SMELT_FLAG1"] = str_show_flag;
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 2)
			//		{
			//			row1["MAIN_SMELT_FLAG2"] = str_show_flag;
			//		}
			//		if (tpssm14.SUB_CHARGE_NO == 3)
			//		{
			//			row1["MAIN_SMELT_FLAG3"] = str_show_flag;
			//		}
			//		break;
			//	case 4: //精炼
			//		str = "SR" + CDecimal(tpssm14_charge_no - first_srf + 1).ToString();
			//		colname = str + "_FLAG";
			//		row1[colname] = str_show_flag;
			//		break;
			//	case 5: //连铸
			//		if (tpssm14.ARRIVE_TIME_REAL.Trim() != "")
			//		{
			//			row1["CC_FLAG1"] = "1";
			//		}
			//		if (tpssm14.LEAVE_TIME_REAL.Trim() != "")
			//		{
			//			row1["CC_FLAG1"] = "2";
			//		}
			//		row1["CC_FLAG2"] = str_show_flag;
			//		break;
			//	}

			//}//while tpssm14
			//cmd_inq1.Close();
			////------------------实际值监控结束------------------------

			fetchRowCount++;

		}//while tpssm13
		cmd_inq.Close();

		if (fetchRowCount>0)
		{
			bcls_ret->Tables[blknum].Rows[0]["recordCount"] = fetchRowCount;
		}

		Log::Trace("", __FUNCTION__, "query records. [{0}]", bcls_ret->Tables[0].Rows.get_Count());

	}
	catch (CDbException& ex)  //用于捕获数据库操作异常
	{

		//CFormattable arguments[] = { _S("TPSSMD1"), ex.GetCode() };
		//CMessageFormat::Format(s.msg,  _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 2);

		CString str = sqlstr + "\r\n" + ex.GetMsg();

		Log::Trace("", __FUNCTION__, "error=[{0}]", str);

		strncpy(s.sysmsg, (const char*)str, 399);  //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		return -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399);
		s.flag = ex.GetCode();
		return -1;
	}
	return 0;
}
