/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:   3.1.0
Date:     2014-11-26
Description: 出钢计划之主计划（炉次）新增。
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"


//程序用头文件



#if defined _SYS_PES
//发送计划状态给MMS(不应该此处调用????，在所有计划处理成功后调用，避免异步电文在调用点就发送)
int f_cm_200009_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
#endif

int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入

/*<remark>=========================================================
/// <summary>
///  出钢计划之主计划（炉次）新增 - 表格与甘特图共用
/// <para>处理内容：根据指定命令新增炉次计划，本函数只处理主计划信息。</para>
/// <para>1.读取编入计划的PONO</para>
/// <para>2.判断PONO状态,是否已编入计划</para>
/// <para>3.生成出钢计划主表数据, 生成临时计划号;       </para>
/// <para>4.对新编入出钢计划的纪录, 修改制造命令炉次表(tpssm10)中状态信息;</para>
/// <para>5.修改制造命令LOT表(TPSSM04)中状态信息</para>
/// <para>6.向MMS的炼钢作业计划发送编入出钢计划电文。</para>
/// <para>数据库表：TPSSM11(出钢计划主表) </para>
/// <para>主调用函数：f_pssm11_ins。           </para>
/// </summary>
/// <param name="factory_div">厂别区分         </param>
/// <param name="pono">制造命令号          </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm11_ins_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int blkseq, rows, i;

	//业务用变量
	CString dateNow14 = "";
	CString v_factory_div = "";	//炼钢单元号
	CDecimal plan_id = 0;       //临时计划号基准值
	CString  v_pono = "", v_heat_no = " ", v_run_status;
	CDecimal pono_status;         //PONO状态
	CDecimal v_smelt_mode = 1;
	CString v_restrand_flg = "";
	CString routebagkey = "";
	CString routelist = "";
	CString smelt_mode2 = "";
	CString special_flag = "";

	EIClass inBlock2;        //调用函数用
	EIClass in_pssm99trace;  //调用履历函数

	//CDbCommand cmd_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CString sqlstr = "";


	try
	{
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm15("TPSSM15");
	CModel tpssm01("TPSSM01");
	CModel tpssm02("TPSSM02");
	CModel tpssm99("TPSSM99");
	CModel tqmts0x("TQMTS0X");
	CModel tep0002("TEP0002");

		//计划履历按一炉为单位
		in_pssm99trace.Tables[0].set_TableName("TRACE");
		in_pssm99trace.Tables[0].Clone(tpssm99);

		dateNow14 = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//--------------------------------
		//设定返回块信息结构，单记录
		blkseq = 0; //第1块
		if (bcls_ret->Tables.Contains("PONO") == false)
		{
			bcls_ret->Tables.Add("PONO");  //增加表
			bcls_ret->Tables["PONO"].Columns.Add(DT_STRING, "SM_PLAN_NO");
		}
		bcls_ret->Tables["PONO"].Rows.Clear();  //返回前清空

		//--------------------------------
		//定义函数调用输入块结构
		inBlock2.Tables[0].set_TableName("X200009");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock2.Tables[0].Rows.Add();

		//----------------------------------------------------------------------------------
		//获得输入参数
		//读取炼钢单元号
		blkseq = bcls_rec->Tables.IndexOf("PLAN");
		if (blkseq < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			sprintf(s.msg, "没有找到计划数据块[PLAN]，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PLAN] NOT EXIST in f_pssm11_ins_heat_n().");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		v_factory_div = bcls_rec->Tables[blkseq].Rows[0]["FACTORY_DIV"];
		if (bcls_rec->Tables[blkseq].Columns.Contains("SPECIAL_FLAG"))
		{
			special_flag = bcls_rec->Tables[blkseq].Rows[0]["SPECIAL_FLAG"].ToString();
		}
		////Log::Info(" ", __FUNCTION__, "factory_div =[{0}]", v_factory_div);


		/* 对输入信息循环处理 */
		//2.读取编入计划的PONO
		blkseq = bcls_rec->Tables.IndexOf("PONO");
		if (blkseq < 0)
		{
			strcpy(s.msg, "传入编制计划的命令数据块[PONO]不存在，请联系系统维护人员。");
			sprintf(s.sysmsg, "TABLE [PONO] NOT EXIST.");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (special_flag != "1")
		{
			rows = bcls_rec->Tables[blkseq].Rows.get_Count();
			for (i = 0; i < rows; i++)
			{
				//tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[i]);
				plan_id = bcls_rec->Tables[blkseq].Rows[i]["PLID"].ToDecimal();  //ID:参与临时计划号计算用
				v_pono = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString().Trim();
				v_smelt_mode = bcls_rec->Tables[blkseq].Rows[i]["SMELT_MODE"].ToDecimal();
				v_restrand_flg = bcls_rec->Tables[blkseq].Rows[i]["RESTRAND_FLG"].ToString().TrimOrBlank();
				if (bcls_rec->Tables[blkseq].Columns.Contains("ROUTEBAGKEY"))
				{
					routebagkey = bcls_rec->Tables[blkseq].Rows[i]["ROUTEBAGKEY"].ToString().Trim();
				}
				if (bcls_rec->Tables[blkseq].Columns.Contains("ROUTELIST"))
				{
					routelist = bcls_rec->Tables[blkseq].Rows[i]["ROUTELIST"].ToString().Trim();
				}
				if (bcls_rec->Tables[blkseq].Columns.Contains("SMELT_MODE2"))
				{
					smelt_mode2 = bcls_rec->Tables[blkseq].Rows[i]["SMELT_MODE2"].ToString().Trim();
				}

				tpssm10["FACTORY_DIV"] = v_factory_div;
				tpssm10["PONO"] = v_pono;

				////Log::Info("", __FUNCTION__, "factory_div=[{0}], pono=[{1}], plan_id=[{2}]", tpssm10["FACTORY_DIV"].ToString(), tpssm10["PONO"].ToString(), plan_id);


				//校验该PONO是否在TPSSM10表中存在
				sqlstr = "tpssm10.Query(FACTORY_DIV,PONO)";
				bool has10 = tpssm10.Query("FACTORY_DIV,PONO");  //主键查询

				if (has10 == false)
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000024")/*制造命令号[{0}]不存在。*/, arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm10.TrimOrBlank();

				tpssm10["RESTRAND_FLG"] = v_restrand_flg;//20151123lijie获取甘特图传入重引锭标记

				//----------------------------------
				//1.判断PONO状态,是否已编入计划
				tpssm11["PONO"] = tpssm10["PONO"];			/* 制造命令号 */
				if (tpssm10["PONO_STATUS"].ToDecimal() >= 18 && tpssm11.QueryCount("PONO") > 0) //18 - 编入计划
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000084")/*制造命令号[{0}]已排入出钢计划。*/, arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				////柳钢定制，物料要求读取tpssm03表中的锭型代码给tpssm11表
				//switch (conn->DatabaseKind)
				//{
				//case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				//	sqlstr = "  SELECT INGOT_CODE FROM TPSSM03 \
							//				WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV \
							//				AND PONO    = @tpssm10.PONO \
							//				FETCH FIRST 1 ROWS ONLY ";
				//	break;
				//case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				//case DB_KIND_MSSQL:	        // MS SQL Server数据库
				//case DB_KIND_ORACLE:	        // Oracle 数据库
				//default:
				//	sqlstr = "  SELECT INGOT_CODE FROM TPSSM03 \
							//				WHERE FACTORY_DIV = @tpssm10.FACTORY_DIV \
							//				AND PONO    = @tpssm10.PONO \
							//				AND ROWNUM  = 1 ";
				//	break;
				//}
				//cmd_tpssm03_inq.SetCommandText(sqlstr);
				//cmd_tpssm03_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
				//cmd_tpssm03_inq.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
				//cmd_tpssm03_inq.ExecuteReader();
				//if (cmd_tpssm03_inq.Read())
				//{
				//	tpssm11.INGOT_CODE = cmd_tpssm03_inq.GetString(1);
				//}
				//cmd_tpssm03_inq.Close();

				//对新编入出钢计划的记录，修改状态信息 
				if (tpssm10["PONO_STATUS"].ToDecimal() <= 18)//排入出钢计划
				{
					pono_status = 18;	/*排入出钢计划*/
				}
				else
				{
					pono_status = tpssm10["PONO_STATUS"];	/*保留原来的运行状态*/
				}
				if (bcls_rec->Tables[blkseq].Columns.Contains("HEAT_NO"))
					v_heat_no = bcls_rec->Tables[blkseq].Rows[i]["HEAT_NO"].ToString().Trim();
				else
					v_heat_no = " ";
				v_run_status = "00";
				if (bcls_rec->Tables[blkseq].Columns.Contains("RUN_STATUS"))
					v_run_status = bcls_rec->Tables[blkseq].Rows[i]["RUN_STATUS"].ToString().Trim();
				if (v_run_status == "")
					v_run_status = "00";
				Log::Info("", __FUNCTION__, "PONO[{0}]： 排入计划HEAT_NO[{1}] RUN_STATUS[{2}]", tpssm10["PONO"].ToString(), v_heat_no, v_run_status);
				//----------------------------------------------------------------------------------------
				//生成出钢计划主表数据
				tpssm11["REC_CREATOR"] = s.userid;
				tpssm11["REC_CREATE_TIME"] = dateNow14;
				tpssm11["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];   /* 炼钢单元号 */
				tpssm11["PONO"] = tpssm10["PONO"];			/* 制造命令号 */
				tpssm11["ST_NO"] = tpssm10["ST_NO"];			/* 出钢记号 */
				//tpssm11["REFINE_ROUTE_CODE"] = tpssm10["REFINE_DIV"]; /* 精炼路径代码 在f_pssm12_save_job里更新*/
				tpssm11["HEAT_NO"] = v_heat_no; // " ";                    /* 熔炼号 */
				tpssm11["PONO_STATUS"] = pono_status; // 18;
				//tpssm11["SMELT_MODE"] = v_smelt_mode;			/* 冶炼模式 1-普通法, 2-双联法 */
				tpssm11["PLAN_TAP_WT"] = tpssm10["PLAN_TAP_WT"];	/* 分割炉次取浇铸量 */
				tpssm11["RESTRAND_FLG"] = tpssm10["RESTRAND_FLG"];   /* 连连铸标记 */
				tpssm11["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];   /* 连铸机号 */
				//tpssm11["CAST_NO"];                     /* 连连浇号(CAST号)，此处不赋值，f_pssm18_cast_create()函数中计算 */
				//tpssm11["CAST_DIV_NO"];                    /* CAST分割号 */
				//tpssm11["CAST_PONO_SUM"];                  /* CAST内炉数 */
				tpssm11["BACKLOG_EA"] = tpssm10["BACKLOG_EA"];   /* 钢区工序途径 */
				tpssm11["TD_CHG_FLG"] = tpssm10["TD_CHG_FLG"];	   /* 中间包更换标志，此处不赋值的 */
				tpssm11["LADLE_NO"] = " ";                  /* 钢包号 */
				//tpssm11["PLAN_STYLE"] = "0";                  /* 计划编制类型: 0-手工; 1-模型。 */
				tpssm11["CC_REQ_TIME"] = " ";                  /* CC要求时刻: 此处不赋值,炉次条件中设置。或出钢计划第一炉设定，f_pssm11_pour_time() 中计算*/
				tpssm11["EARLY_TIME"] = 0;	                   /* 早到时间 */
				tpssm11["RUN_STATUS"] = v_run_status;// "00";             /* 炉次状态: 新增的必然是'0' */
				tpssm11["OUT_STEEL_WT"] = tpssm11["PLAN_TAP_WT"];   /* 出钢钢水重量 */
				tpssm11["PLAN_EDIT_FLAG"] = "N";                  /* 计划编辑标记, N-新增计划，后续计算用 */
				tpssm11["IC_CC_FLAG"] = tpssm10["BACKLOG_EA"].ToString().Substring(tpssm10["BACKLOG_EA"].ToString().GetLength() - 1, 1);
				tpssm11["ROUTEBAGKEY"] = routebagkey;
				tpssm11["ROUTELIST"] = routelist.TrimOrBlank();
				tpssm11["SMELT_MODE2"] = smelt_mode2.TrimOrBlank();

				//吹氩指示 1-规定吹氩 0-无吹氩指示，必过可写死，非必过根据工艺卡tqmts0x.AR_BLOW_DEF
				tep0002["CODE_CLASS"] = "M00F";
				tep0002["CODE"] = tpssm11["FACTORY_DIV"];
				sqlstr = "tep0002.Query()";
				if (tep0002.Query("CODE_CLASS, CODE") == true)
				{
					tqmts0x["FACTORY_DIV"] = tep0002["CODE_DESC_3_CONTENT"];
				}

				tqmts0x["ST_NO"] = tpssm11["ST_NO"];
				tqmts0x["VALID_FLAG"] = "1";
				sqlstr = "tqmts0x.Query()";
				if (tqmts0x.Query("FACTORY_DIV,ST_NO,VALID_FLAG") == true)
				{
					tpssm11["AR_DIFF"] = tqmts0x["AR_BLOW_DEF"];
				}

				//tpssm11["AR_DIFF"] = "1";


				//生成临时计划号，这个计划生成后，f_pssm11_planno()函数统一计算。
				//sprintf(tpssm11["SM_PLAN_NO"].ToString(), "P%.5d", i);
				tpssm11["SM_PLAN_NO"] = CString::Format("P%.5d", i + plan_id.ToInt32());

				tpssm11.TrimOrBlank();
				////Log::Trace("", __FUNCTION__, "INS PONO=[{0}], plan_no=[{1}], plan_edit_flag=[{2}]", tpssm11["PONO"].ToString(), tpssm11["SM_PLAN_NO"].ToString(), tpssm11["PLAN_EDIT_FLAG"].ToString());

				sqlstr = "tpssm11.Insert()";
				tpssm11.Insert();


				//-------------------------------------------

				//1)修改制造命令炉次表(tpssm10)中状态信息
				tpssm10["PONO_STATUS"] = pono_status;
				tpssm10["REC_REVISOR"] = CString(s.userid);
				tpssm10["REC_REVISE_TIME"] = dateNow14;
				//tpssm10["SMELT_MODE"] = v_smelt_mode;
				sqlstr = "tpssm10.Update(PONO_STATUS)";
				tpssm10.Update(
					"PONO_STATUS,RESTRAND_FLG,"
					"REC_REVISE_TIME,REC_REVISOR",
					"FACTORY_DIV,PONO"); //主键


				//2) 修改制造命令炉次表(tpssm01)中状态信息
				tpssm01["PONO_STATUS"] = pono_status;
				tpssm01["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssm01["PONO"] = tpssm10["PONO"];
				sqlstr = "tpssm01.Update(PONO_STATUS)";
				tpssm01.Update(
					"PONO_STATUS,"
					"REC_REVISE_TIME,REC_REVISOR",
					"FACTORY_DIV,PONO"); //主键

				//3) 修改制造命令LOT表(TPSSM02)中状态信息
				tpssm02["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssm02["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
				tpssm02["LOT_STATUS"] = 4;
				sqlstr = "tpssm02.Update(LOT_STATUS)";
				tpssm02.Update(
					"LOT_STATUS",
					"FACTORY_DIV,CAST_LOT_NO"); //主键

#ifdef _SYS_PES    
				/***********2009-12-2 10:11 调用发送炉次状态的函数********start********/
				////Log::Trace("", __FUNCTION__, "tpssm10["FACTORY_DIV"] =[{0}]", (const char*)tpssm10["FACTORY_DIV"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssm10["PONO"] =[{0}]", (const char*)tpssm10["PONO"].ToString());
				////Log::Trace("", __FUNCTION__, "pono_status =[{0}]", pono_status);

				//blkseq = 1;
				//发送炼钢PONO状态

				inBlock2.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				inBlock2.Tables[0].Rows[0]["PONO"] = tpssm10["PONO"];
				inBlock2.Tables[0].Rows[0]["PONO_STATUS"] = pono_status;

				//	ret = f_cm_200009_snd(&inBlock2, bcls_ret, conn); //wcy 暂时取消
				if (ret != 0) //调用不成功
				{
					////Log::Trace("", __FUNCTION__, "f_cm_pam1p2_snd()发送炼钢PONO状态出错:[{0}]",(const char*) s.msg);
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				/***********2009-12-2 10:11 调用发送炉次状态的函数********end********/
#endif
				//写计划履历表
				tpssm99["EVENT_ID"] = "B1";
				tpssm99["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
				tpssm99["PONO"] = tpssm11["PONO"];
				tpssm99["PONO_STATUS"] = tpssm11["PONO_STATUS"];
				tpssm99["VALID_FLAG"] = "1";//操作成功
				/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
				tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
				////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

				////Log::Trace("", __FUNCTION__, "记录成功履历，传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
				//记录编入计划成功的履历
				ret = 0;
				ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				if (ret < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//-------------------------------------------
				//返回生成的计划号
				CDataRow &row = bcls_ret->Tables["PONO"].Rows.Add();
				row["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];


			}//读取编入计划的PONO
		}
		else//wcy 日平衡逻辑
		{
			rows = bcls_rec->Tables[blkseq].Rows.get_Count();
			for (i = 0; i < rows; i++)
			{
				//tpssm11.MergeFrom(bcls_rec->Tables[0].Rows[i]);
				plan_id = bcls_rec->Tables[blkseq].Rows[i]["PLID"].ToDecimal();  //ID:参与临时计划号计算用
				v_pono = bcls_rec->Tables[blkseq].Rows[i]["PONO"].ToString().Trim();
				v_smelt_mode = bcls_rec->Tables[blkseq].Rows[i]["SMELT_MODE"].ToDecimal();
				v_restrand_flg = bcls_rec->Tables[blkseq].Rows[i]["RESTRAND_FLG"].ToString().TrimOrBlank();
				if (bcls_rec->Tables[blkseq].Columns.Contains("ROUTEBAGKEY"))
				{
					routebagkey = bcls_rec->Tables[blkseq].Rows[i]["ROUTEBAGKEY"].ToString().Trim();
				}
				if (bcls_rec->Tables[blkseq].Columns.Contains("ROUTELIST"))
				{
					routelist = bcls_rec->Tables[blkseq].Rows[i]["ROUTELIST"].ToString().Trim();
				}
				if (bcls_rec->Tables[blkseq].Columns.Contains("SMELT_MODE2"))
				{
					smelt_mode2 = bcls_rec->Tables[blkseq].Rows[i]["SMELT_MODE2"].ToString().Trim();
				}

				tpssm10["FACTORY_DIV"] = v_factory_div;
				tpssm10["PONO"] = v_pono;

				////Log::Info("", __FUNCTION__, "factory_div=[{0}], pono=[{1}], plan_id=[{2}]", tpssm10["FACTORY_DIV"].ToString(), tpssm10["PONO"].ToString(), plan_id);


				//校验该PONO是否在TPSSM10表中存在
				sqlstr = "tpssm10.Query(FACTORY_DIV,PONO)";
				bool has10 = tpssm10.Query("FACTORY_DIV,PONO");  //主键查询

				if (has10 == false)
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000024")/*制造命令号[{0}]不存在。*/, arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tpssm10.TrimOrBlank();

				tpssm10["RESTRAND_FLG"] = v_restrand_flg;//20151123lijie获取甘特图传入重引锭标记

				//----------------------------------
				//1.判断PONO状态,是否已编入计划
				tpssm15["PONO"] = tpssm10["PONO"];			/* 制造命令号 */
				if (tpssm15.QueryCount("PONO") > 0) //18 - 编入计划
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, _RES("PSSMS0000084")/*制造命令号[{0}]已排入出钢计划。*/, arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//对新编入出钢计划的记录，修改状态信息 
				if (tpssm10["PONO_STATUS"].ToDecimal() <= 18)//排入出钢计划
				{
					pono_status = 18;	/*排入出钢计划*/
				}
				else
				{
					pono_status = tpssm10["PONO_STATUS"];	/*保留原来的运行状态*/
				}
				if (bcls_rec->Tables[blkseq].Columns.Contains("HEAT_NO"))
					v_heat_no = bcls_rec->Tables[blkseq].Rows[i]["HEAT_NO"].ToString().Trim();
				else
					v_heat_no = " ";
				v_run_status = "00";
				if (bcls_rec->Tables[blkseq].Columns.Contains("RUN_STATUS"))
					v_run_status = bcls_rec->Tables[blkseq].Rows[i]["RUN_STATUS"].ToString().Trim();
				if (v_run_status == "")
					v_run_status = "00";
				Log::Info("", __FUNCTION__, "PONO[{0}]： 排入计划HEAT_NO[{1}] RUN_STATUS[{2}]", tpssm10["PONO"].ToString(), v_heat_no, v_run_status);
				//----------------------------------------------------------------------------------------
				//生成出钢计划主表数据
				tpssm15["REC_CREATOR"] = s.userid;
				tpssm15["REC_CREATE_TIME"] = dateNow14;
				tpssm15["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];   /* 炼钢单元号 */
				tpssm15["PONO"] = tpssm10["PONO"];			/* 制造命令号 */
				tpssm15["ST_NO"] = tpssm10["ST_NO"];			/* 出钢记号 */
				//tpssm15["REFINE_ROUTE_CODE"] = tpssm10["REFINE_DIV"]; /* 精炼路径代码 在f_pssm12_save_job里更新*/
				tpssm15["HEAT_NO"] = v_heat_no; // " ";                    /* 熔炼号 */
				tpssm15["PONO_STATUS"] = pono_status; // 18;
				//tpssm15["SMELT_MODE"] = v_smelt_mode;			/* 冶炼模式 1-普通法, 2-双联法 */
				tpssm15["PLAN_TAP_WT"] = tpssm10["PLAN_TAP_WT"];	/* 分割炉次取浇铸量 */
				tpssm15["RESTRAND_FLG"] = tpssm10["RESTRAND_FLG"];   /* 连连铸标记 */
				tpssm15["CC_MACH_NO"] = tpssm10["CC_MACH_NO"];   /* 连铸机号 */
				//tpssm15["CAST_NO"];                     /* 连连浇号(CAST号)，此处不赋值，f_pssm18_cast_create()函数中计算 */
				//tpssm15["CAST_DIV_NO"];                    /* CAST分割号 */
				//tpssm15["CAST_PONO_SUM"];                  /* CAST内炉数 */
				tpssm15["BACKLOG_EA"] = tpssm10["BACKLOG_EA"];   /* 钢区工序途径 */
				tpssm15["TD_CHG_FLG"] = tpssm10["TD_CHG_FLG"];	   /* 中间包更换标志，此处不赋值的 */
				tpssm15["LADLE_NO"] = " ";                  /* 钢包号 */
				//tpssm15["PLAN_STYLE"] = "0";                  /* 计划编制类型: 0-手工; 1-模型。 */
				tpssm15["CC_REQ_TIME"] = " ";                  /* CC要求时刻: 此处不赋值,炉次条件中设置。或出钢计划第一炉设定，f_pssm11_pour_time() 中计算*/
				tpssm15["EARLY_TIME"] = 0;	                   /* 早到时间 */
				tpssm15["RUN_STATUS"] = v_run_status;// "00";             /* 炉次状态: 新增的必然是'0' */
				tpssm15["OUT_STEEL_WT"] = tpssm15["PLAN_TAP_WT"];   /* 出钢钢水重量 */
				tpssm15["PLAN_EDIT_FLAG"] = "N";                  /* 计划编辑标记, N-新增计划，后续计算用 */
				tpssm15["IC_CC_FLAG"] = tpssm10["BACKLOG_EA"].ToString().Substring(tpssm10["BACKLOG_EA"].ToString().GetLength() - 1, 1);
				tpssm15["ROUTEBAGKEY"] = routebagkey;
				tpssm15["ROUTELIST"] = routelist.TrimOrBlank();
				tpssm15["SMELT_MODE2"] = smelt_mode2.TrimOrBlank();

				//吹氩指示 1-规定吹氩 0-无吹氩指示，必过可写死，非必过根据工艺卡tqmts0x.AR_BLOW_DEF
				tep0002["CODE_CLASS"] = "M00F";
				tep0002["CODE"] = tpssm15["FACTORY_DIV"];
				sqlstr = "tep0002.Query()";
				if (tep0002.Query("CODE_CLASS, CODE") == true)
				{
					tqmts0x["FACTORY_DIV"] = tep0002["CODE_DESC_3_CONTENT"];
				}

				tqmts0x["ST_NO"] = tpssm15["ST_NO"];
				tqmts0x["VALID_FLAG"] = "1";
				sqlstr = "tqmts0x.Query()";
				if (tqmts0x.Query("FACTORY_DIV,ST_NO,VALID_FLAG") == true)
				{
					tpssm15["AR_DIFF"] = tqmts0x["AR_BLOW_DEF"];
				}

				//tpssm15["AR_DIFF"] = "1";


				//生成临时计划号，这个计划生成后，f_pssm11_planno()函数统一计算。
				//sprintf(tpssm15["SM_PLAN_NO"].ToString(), "P%.5d", i);
				tpssm15["SM_PLAN_NO"] = CString::Format("P%.5d", i + plan_id.ToInt32());

				tpssm15.TrimOrBlank();
				Log::Trace("", __FUNCTION__, "INS PONO=[{0}], plan_no=[{1}], plan_edit_flag=[{2}]", tpssm15["PONO"].ToString(), tpssm15["SM_PLAN_NO"].ToString(), tpssm15["PLAN_EDIT_FLAG"].ToString());

				sqlstr = "tpssm15.Insert()";
				tpssm15.Insert();


				//-------------------------------------------

				//1)修改制造命令炉次表(tpssm10)中状态信息
				//tpssm10["PONO_STATUS"] = pono_status;
				//tpssm10["REC_REVISOR"] = CString(s.userid);
				//tpssm10["REC_REVISE_TIME"] = dateNow14;
				//tpssm10["SMELT_MODE"] = v_smelt_mode;
				//sqlstr = "tpssm10.Update(PONO_STATUS)";
				//tpssm10.Update(
					//"PONO_STATUS,RESTRAND_FLG,"
					//"REC_REVISE_TIME,REC_REVISOR",
					//"FACTORY_DIV,PONO"); //主键


				//2) 修改制造命令炉次表(tpssm01)中状态信息
				//tpssm01["PONO_STATUS"] = pono_status;
				//tpssm01["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				//tpssm01["PONO"] = tpssm10["PONO"];
				//sqlstr = "tpssm01.Update(PONO_STATUS)";
				//tpssm01.Update(
					//"PONO_STATUS,"
					//"REC_REVISE_TIME,REC_REVISOR",
					//"FACTORY_DIV,PONO"); //主键

				//3) 修改制造命令LOT表(TPSSM02)中状态信息
				//tpssm02["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				//tpssm02["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
				//tpssm02["LOT_STATUS"] = 4;
				//sqlstr = "tpssm02.Update(LOT_STATUS)";
				//tpssm02.Update(
					//"LOT_STATUS",
					//"FACTORY_DIV,CAST_LOT_NO"); //主键

#ifdef _SYS_PES    
				/***********2009-12-2 10:11 调用发送炉次状态的函数********start********/
				////Log::Trace("", __FUNCTION__, "tpssm10["FACTORY_DIV"] =[{0}]", (const char*)tpssm10["FACTORY_DIV"].ToString());
				////Log::Trace("", __FUNCTION__, "tpssm10["PONO"] =[{0}]", (const char*)tpssm10["PONO"].ToString());
				////Log::Trace("", __FUNCTION__, "pono_status =[{0}]", pono_status);

				//blkseq = 1;
				//发送炼钢PONO状态

				//inBlock2.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				//inBlock2.Tables[0].Rows[0]["PONO"] = tpssm10["PONO"];
				//inBlock2.Tables[0].Rows[0]["PONO_STATUS"] = pono_status;

				//	ret = f_cm_200009_snd(&inBlock2, bcls_ret, conn); //wcy 暂时取消
				//if (ret != 0) //调用不成功
				//{
					////Log::Trace("", __FUNCTION__, "f_cm_pam1p2_snd()发送炼钢PONO状态出错:[{0}]",(const char*) s.msg);
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					//throw CApplicationException(-1, s.msg, log.Location);
				//}
				/***********2009-12-2 10:11 调用发送炉次状态的函数********end********/
#endif
				//写计划履历表
				//tpssm99["EVENT_ID"] = "B1";
				//tpssm99["FACTORY_DIV"] = tpssm15["FACTORY_DIV"];
				//tpssm99["PONO"] = tpssm15["PONO"];
				//tpssm99["PONO_STATUS"] = tpssm15["PONO_STATUS"];
				//tpssm99["VALID_FLAG"] = "1";//操作成功
				/*in_pssm99trace.Tables[0].Clone(tpssm99);*/
				//tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
				////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());

				////Log::Trace("", __FUNCTION__, "记录成功履历，传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
				//记录编入计划成功的履历
				//ret = 0;
				//ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
				//if (ret < 0)
				//{
					//throw CApplicationException(-1, s.msg, log.Location);
				//}

				//-------------------------------------------
				//返回生成的计划号
				CDataRow &row = bcls_ret->Tables["PONO"].Rows.Add();
				row["SM_PLAN_NO"] = tpssm15["SM_PLAN_NO"];


			}//读取编入计划的PONO
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
