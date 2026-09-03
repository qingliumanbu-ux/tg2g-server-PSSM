/*=========================================================================
//程序名称:     pssm11stchg_stno_inq
//隶属子系统:   PSSM
//产品名称:     BSM1
//创建人员:     魏晨祥
//创建时间:     2023-06-14 17:13:56
//修改人员:
//修改日期:
//-----------------------------------------------------------------------
//功能描述:		钢种变更时出钢记号推荐
//数据库表:     TQMTS02(成分标准);
//主调用函数:   前台PSSM28STChgDlgSI 画面(钢种推荐)按钮
//需调用函数:
//=========================================================================*/
#include "stdafx.h"
#include "math.h"
//#include "tpssmc1.h"

/*<remark>=========================================================
/// <summary>
/// 钢种变更时出钢记号推荐
/// </para>
/// <para>末道工序成分            </para>
/// <returns>出钢记号、是否有对应的计划</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11stchg_stno_inq)


int f_pssm11stchg_stno_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int allStnoCount;
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
	CString  cc_mach_no = "";

	// 定义表的实体对象
	//CModel tpssm11("TPSSM11");
	CModel tqmts02("TQMTS02");

	CString sqlstr = "";
	CString sqlPlanStr;
	CString sqlorder = "";
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	int			elm_ok = 0;	//成分合否标志 0:未判定,1:主试不合且特采不合,2:大于主试最大且无特采,4:小于主试最小且无特采,3:主试不合但特采合,8:主试合,9:不需判
	int			elm_0_num = 0;	//未收到实绩的元素数量
	int			elm_1_num = 0;	//判定结果为不合的元素数量
	CString		s_judge_code = "";	//最终判定结果 0:未判定,1:合格,2:不合格
	CString		s_o_gas_div = "";
	CString		s_n_gas_div = "";
	CString		s_h_gas_div = "";
	CDecimal elm_act;
	EIClass bcls_elm;//存放元素标准
	//bcls_elm.Tables[0].Columns.Add(tqmts02);
	//bcls_elm.Tables[0].Columns.Add(DT_INT16, "ELM_OK");

	try
	{

		//------------------------------------------
		//设置返回块参数
		blknum = 0; //第1块
		bcls_ret->Tables[blknum].set_TableName("ST_NO");
		//bcls_ret->Tables[blknum].Columns.Add(tpssm11);   //从实体对象创建架构

		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "ST_NO");     //ST_NO
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "HAS_PONO");  //PONO_COUNT
		bcls_ret->Tables[blknum].Columns.Add(DT_STRING, "OK_FLAG");   //合格标记

		//--------------------------------------------------------------
		//获得输入参数
		cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString().Trim();    
		factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();

		////Log::Info("", __FUNCTION__, "order_mode = [{0}]", order_mode);
		////Log::Info("", __FUNCTION__, "factory_div = [{0}]", factory_div);

		if (cc_mach_no == "")
		{
			sqlPlanStr = CString(" select count(PONO) HAS_PONO,ST_NO FROM TPSSM10 "
				" where PONO_STATUS<18 AND FACTORY_DIV = @TPSSM10_FACTORY_DIV  GROUP BY ST_NO " );
		}
		else
		{
			sqlPlanStr = CString(" select count(PONO) HAS_PONO,ST_NO FROM TPSSM10 "
				" where PONO_STATUS<18 AND FACTORY_DIV = @TPSSM10_FACTORY_DIV AND CC_MACH_NO = @TPSSM10_CC_MACH_NO GROUP BY ST_NO ");
		}


		//------------------------------------------
		// 出钢记号查询
		sqlstr = CString(
			" select A.ST_NO,HAS_PONO,'9' OK_FLAG "
			" from(select distinct(ST_NO) ST_NO FROM TQMTS0X) A, "
			" (" + sqlPlanStr + ") B"
		 " where A.ST_NO = B.ST_NO(+) "
			);
		sqlstr += "and exists(select st_no from TQMTS02 where st_no=A.ST_NO AND SMELT_CHEMI_FLAG >= '3' AND FACTORY_DIV = 'A' AND  SUBSTR(ELM_CODE, 1, 1) NOT BETWEEN 'A' AND 'Z' AND WHOLE_BACKLOG_CODE = 'C')";
		cmd_inq.SetCommandText(sqlstr);
		Log::Info("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.Parameters.Set("TPSSM10_FACTORY_DIV", factory_div);
		cmd_inq.Parameters.Set("TPSSM10_CC_MACH_NO", cc_mach_no);
		//cmd_inq.ExecuteReader();
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		blknum = 0; //第1块
		allStnoCount = bcls_ret->Tables[0].Rows.get_Count();
		for (int sti = allStnoCount - 1; sti >= 0; sti--)
		{
			/*------------------------  查询工序成分标准信息  ------------------------*/
			CString v_st_no = bcls_ret->Tables[0].Rows[sti]["ST_NO"].ToString();
			Log::Info("", __FUNCTION__, "row = [{0}] st_no[{1}]", sti, v_st_no);
			fetchRowCount = 0;
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:				// MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:						// 所有数据库适用，通用SQL语句
				sqlstr = " SELECT * "
					"   FROM TQMTS02 "
					"  WHERE ST_NO = @TQMTS02_ST_NO "
					"   AND SMELT_CHEMI_FLAG >= '3'"
					"  AND FACTORY_DIV =@s_factory_div"
					" AND  SUBSTR(ELM_CODE,1,1) NOT BETWEEN 'A' AND 'Z' "
					;
				sqlstr = sqlstr + " AND WHOLE_BACKLOG_CODE = @whole_backlog_code ";
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("TQMTS02_ST_NO", v_st_no);
			cmd_inq.Parameters.Set("whole_backlog_code", "C");
			cmd_inq.Parameters.Set("s_factory_div", "A");
			//cmd_inq.ExecuteReader();
			//while (cmd_inq.Read())
			//{
			//	fetchRowCount++;
			//	cmd_inq.Fetch(tqmts02);
			//	Log::Trace("", __FUNCTION__, " cmd_inq.Fetch(tqmts02) [{1}]:[{2}]", sti, tqmts02["ELM_CODE"].ToString(), tqmts02["ELM_NAME"].ToString());
			//	CDataRow& row = bcls_elm.Tables[0].Rows.Add();
			//	row.Merge(tqmts02);
			//	//row["ELM_OK"] = elm_ok;
			//}
			cmd_inq.ExecuteQuery(bcls_elm.Tables[0]);
			cmd_inq.Close();
			fetchRowCount = bcls_elm.Tables[0].Rows.get_Count();
			Log::Trace("", "", " TQMTS02_ST_NO[{0}]", v_st_no);
			Log::Trace("", "", "s_whole_backlog_code[{0}]", "C");
			Log::Trace("", "", "s_factory_div[{0}]", "A");
			Log::Trace("", "", "sqlstr[{0}]", sqlstr);
			Log::Trace("", "", "f_qmts_judfetchRowCount = [{0}]", fetchRowCount);
			if (fetchRowCount == 0)  ////////用厂别没查到，用‘ ’查，update by yiling 20170106
			{
				Log::Trace("", __FUNCTION__, "没有成分标准，认为合格");
				bcls_ret->Tables[0].Rows[sti]["OK_FLAG"] = "9";
				continue;
			}

			/*------------------------  元素判定  ------------------------*/
			CString s_table_name = "TQMTS25";
			//对元素逐次判定
			for (int i = 0; i < bcls_elm.Tables[0].Rows.get_Count(); i++)
			{
				Log::Trace("", __FUNCTION__, " bcls_elm 主试[{0}/{1}] ----", bcls_elm.Tables[0].Rows[i]["MAIN_MIN"].ToDecimal(), bcls_elm.Tables[0].Rows[i]["MAIN_MAX"].ToDecimal());
				//1.取元素标准值
				tqmts02.MergeFrom(bcls_elm.Tables[0].Rows[i]);

				Log::Trace("", __FUNCTION__, "========== 第 [{0}] 项 ------ 元素代码-名称 [{1}]:[{2}]==============================", i, tqmts02["ELM_CODE"].ToString(), tqmts02["ELM_NAME"].ToString());
				Log::Trace("", __FUNCTION__, " 主试[{0}/{1}]-特采[{2}/{3}]-目标[{4}] ----", tqmts02["MAIN_MIN"].ToDecimal(), tqmts02["MAIN_MAX"].ToDecimal(), tqmts02["SPE_MIN"].ToDecimal(), tqmts02["SPE_MAX"].ToDecimal(), tqmts02["MAIN_AIM"].ToDecimal());

				//初始化判定结果
				elm_ok = 0;


				//2.取成分元素实绩值
				CString elm_name = tqmts02["ELM_CODE"].ToString();
				if (bcls_rec->Tables[0].Columns.Contains(elm_name))
				{
					elm_act = bcls_rec->Tables[0].Rows[0][elm_name].ToDecimal();
				}
				
				CString s_st_sample_div = bcls_rec->Tables[0].Rows[0]["ST_SAMPLE_DIV"].ToString();
				CString s_gas_type_div = bcls_rec->Tables[0].Rows[0]["GAS_TYPE_DIV"].ToString();
				if (elm_act > 0) //有实绩
				{			

					Log::Trace("", __FUNCTION__, " 有实绩：实绩值 [{0}] ", elm_act);
					/////////////begin修约update by yiling 20170803
					if (tqmts02["ELM_ACCU"].ToDecimal() > 0)
					{
						CDecimal v_elm_format = pow(10.0, tqmts02["ELM_ACCU"].ToDecimal().ToDouble());
						elm_act = elm_act * v_elm_format;
						int z_value = floor(elm_act.ToDouble());//取传入长度的整数部分
						Log::Trace("", __FUNCTION__, " z_value [{0}] elm_act[{1}] ", z_value, elm_act);

						float x_value = (elm_act.ToDouble() - z_value);//取传入长度的小数部分
						Log::Trace("", __FUNCTION__, " x_value [{0}]  ", x_value);

						if (x_value == 0.5)
						{
							if (z_value % 2 == 0)
							{
								elm_act = z_value;
							}
							else
							{
								elm_act = z_value + 1;
							}
						}
						else
						{
							elm_act = floor(elm_act.ToDouble() + 0.5);
						}
						elm_act = elm_act.ToDouble() / v_elm_format.ToDouble();
						Log::Trace("", __FUNCTION__, " 修约后elm_act [{0}]  ", elm_act);
					}
					

					if ("TQMTS25" == s_table_name)
					{
						if ("1" == s_st_sample_div)//QV样中, 对'H,N,O'元素不做判定
						{
							Log::Trace("", __FUNCTION__, " QV样 ");
							if (tqmts02["ELM_CODE"].ToString() == "001" || tqmts02["ELM_CODE"].ToString() == "014" || tqmts02["ELM_CODE"].ToString() == "016")// H|N|O元素 	  
							{
								elm_ok = 9;
							}
						}
						else if ("4" == s_st_sample_div) //气体样中, 对非'H,N,O'元素不做判定
						{
							Log::Trace("", __FUNCTION__, " 气体样 ");
							//ON样、H样：考虑气体区分指示，来决定该成分是否需要判定。
							if (tqmts02["ELM_CODE"].ToString() != "001" && tqmts02["ELM_CODE"].ToString() != "014" && tqmts02["ELM_CODE"].ToString() != "016")// H|N|O元素 	  
							{
								elm_ok = 9;
							}
							switch (CDecimal::Parse(s_gas_type_div).ToInt16())
							{
								//GAS_TYPE_DIV:1:ON样 2:H样
							case 1: //进行ON样判定
								Log::Trace("", __FUNCTION__, " ON样 ");
								if (tqmts02["ELM_CODE"].ToString() == "014" && s_n_gas_div != "1")
								{
									elm_ok = 9;
								}
								else if (tqmts02["ELM_CODE"].ToString() == "016" && s_o_gas_div != "1")
								{
									elm_ok = 9;
								}
								break;
							case 2: //进行H样判定
								Log::Trace("", __FUNCTION__, " H样 ");
								if (tqmts02["ELM_CODE"].ToString() == "001" && s_h_gas_div != "1")
								{
									elm_ok = 9;
								}
								break;
							default:
								break;
							}
						}
					}
					//未有判定结果的(即无特殊要求，走正常判定流程的)
					if (elm_ok == 0)  // 0:未判定
					{
						//add by reason 2007-3-13
						//要求判定
						//1.标准值为0～0则，判混杂元素  混杂元素管理区分在制造标准中:有值,则判; 无值,??????。
						//2.工序成分的判定
					      //不判混杂元素 ,按工序成分标准判定
						{
							Log::Trace("", __FUNCTION__, "主试标准[{0}-{1}],实绩[{2}]", tqmts02["MAIN_MIN"].ToDecimal(), tqmts02["MAIN_MAX"].ToDecimal(), elm_act);
							if (elm_act >= tqmts02["MAIN_MIN"].ToDecimal() &&
								(elm_act < tqmts02["MAIN_MAX"].ToDecimal() || (fabs(tqmts02["MAIN_MAX"].ToDecimal().ToDouble() - elm_act.ToDouble()) <= 0.000009)))//主试合
							{
								elm_ok = 8;
							}
							else //主试不合
							{
								if (tqmts02["SPE_MAX"].ToDecimal() >= 99.999)//主试不合且无特采要求
								{
									if (elm_act < tqmts02["MAIN_MIN"].ToDecimal())//实际值小于主试最小值
									{
										elm_ok = 4;
									}
									else  //实际值大于主试最大值
									{
										elm_ok = 2;
									}

								}
								else
								{
									if (elm_act >= tqmts02["SPE_MIN"].ToDecimal() &&
										(elm_act <= tqmts02["SPE_MAX"].ToDecimal() || (fabs(tqmts02["SPE_MAX"].ToDecimal().ToDouble() - elm_act.ToDouble()) <= 0.000009)))//主试不合但特采合
									{
										elm_ok = 3;
									}
									else  //主试不合且特采不合
									{
										elm_ok = 1;
									}
								}//特采要求
							}//主试判断
						}////不判混杂元素 ,按工序成分标准判定 end
					}
					Log::Trace("", __FUNCTION__, " 判定结果 = [{0}] ", elm_ok);
				}//有实绩
				else //没有实绩
				{
					Log::Trace("", __FUNCTION__, " 没有实绩 ");
					if ("TQMTS25" == s_table_name)
					{
						if ("1" == s_st_sample_div)//QV样中, 对'H,N,O'元素不做判定
						{
							Log::Trace("", __FUNCTION__, " QV样 ");
							if (tqmts02["ELM_CODE"].ToString() == "001" || tqmts02["ELM_CODE"].ToString() == "014" || tqmts02["ELM_CODE"].ToString() == "016")// H|N|O元素 	  
							{
								elm_ok = 9;
							}
						}
						else if ("4" == s_st_sample_div) //气体样中, 对非'H,N,O'元素不做判定
						{
							Log::Trace("", __FUNCTION__, " 气体样 ");
							//ON样、H样：考虑气体区分指示，来决定该成分是否需要判定。
							if (tqmts02["ELM_CODE"].ToString() != "001" && tqmts02["ELM_CODE"].ToString() != "014" && tqmts02["ELM_CODE"].ToString() != "016")// H|N|O元素 	  
							{
								elm_ok = 9;
							}
							if (s_gas_type_div == "1")////////update by yiling 20180906  GAS_TYPE_DIV:1:ON样 2:H样,
							{
								if (tqmts02["ELM_CODE"].ToString() == "001")  ///ON样则H元素默认合格
								{
									elm_ok = 9;
								}
							}
							else if (s_gas_type_div == "2")  //// H样则ON默认合格
							{
								if (tqmts02["ELM_CODE"].ToString() == "014" || tqmts02["ELM_CODE"].ToString() == "016")
								{
									elm_ok = 9;
								}
							}  ////////update by yiling 20180906
							switch (CDecimal::Parse(s_gas_type_div).ToInt16())
							{
								//GAS_TYPE_DIV:1:ON样 2:H样
							case 1: //进行ON样判定
								Log::Trace("", __FUNCTION__, " ON样 ");
								if (tqmts02["ELM_CODE"].ToString() == "014" && s_n_gas_div != "1")
								{
									elm_ok = 9;
								}
								else if (tqmts02["ELM_CODE"].ToString() == "016" && s_o_gas_div != "1")
								{
									elm_ok = 9;
								}
								break;
							case 2: //进行H样判定
								Log::Trace("", __FUNCTION__, " H样 ");
								if (tqmts02["ELM_CODE"].ToString() == "001" && s_h_gas_div != "1")
								{
									elm_ok = 9;
								}
								break;
							default:
								break;
							}
						}
					}
					//未有判定结果的(即无特殊要求，走正常判定流程的)
					if (elm_ok == 0)  // 0:未判定
					{
						if (tqmts02["MAIN_MIN"].ToDecimal() == 0 && tqmts02["MAIN_MAX"].ToDecimal() >= 99.999)
							elm_ok = 8;//标准无要求，没有实绩判成合格
						//else
						//	elm_ok = 1;//标准有要求，没有实绩判成不合格
						else
							elm_ok = 7;//标准有要求，没有实绩判成合格
					}
					Log::Trace("", __FUNCTION__, " 判定结果 = [{0}] ", elm_ok);
				}//没有实绩

				if (0 == elm_ok)
					elm_0_num++;
				else if (1 == elm_ok || 2 == elm_ok || 3 == elm_ok || 4 == elm_ok)
					elm_1_num++;


				//更新成分合否标志
				
			}// for i /*------------------------  元素判定  ------------------------*/

			/*------------------------  试样总判定  ------------------------*/
			Log::Trace("", __FUNCTION__, " elm_0_num /elm_1_num  [{0}]:[{1}] ", elm_0_num, elm_1_num);
			if (elm_0_num > 0)
			{
				s_judge_code = "0";//未判定
			}
			else if (elm_1_num > 0)
			{
				s_judge_code = "2";//不合格
			}
			else
			{
				s_judge_code = "1";//合格
			}

			//不合格的排除
			if (s_judge_code != "1")
			{
				bcls_ret->Tables[0].Rows[sti].Delete();
			}


		}//出钢记号循环end
		

		//////Log::Trace("", __FUNCTION__, "VVVVVVVVVVVVV");

		

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
