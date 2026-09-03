/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-03-19
Version:1.0
Description: 钢种变更
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


int f_pssm99_trace(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //出钢计划履历写入

#if defined _SYS_PES
int f_cm_200008_snd	(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);  //送MMS电文
//int f_pssm_pas1p3_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);  //送炼钢L2电文
#endif

int f_mmsm0055_proc(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);//物料实绩接口, 使用inBlock3
int f_qmts_stno_chg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);//质量接口
int f_pssm_ccmchg_chk_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn);//钢种变更连铸机检验
int f_pssm21_cast_cre_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); //生成CAST号


//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 钢种变更
/// <para>前提条件是炉次未开浇。钢种变更分转炉区域和各精炼区域的变更。
///钢种变更后，精炼路径沿用原计划的路径，炉次的成分重新判定，重新进行炉次品质判定。
/// </para>
/// <para>1.检查参数：                                             </para>
/// <para>1）传入的制造命令号不为空；                              </para>
/// <para>2）校验是否是同一台连铸机的钢种变更；                    </para>
/// <para>3）已经开浇的制造命令不能钢种变更；                      </para>
/// <para>校验为正常时,继续以下的处理;否则,只输出警报,操作无效。   </para>
/// <para>2.上述替换制造命令号输入时,作以下处理.                   </para>
/// <para>a.制造命令置换处理                                       </para>
/// <para>确认用以置换的制造命令号对应的制造命令在制造命令表（TPSSM10）中存在,且对出钢计划画面作制造命令置换。
///否则,不作置换操作.但是,生产时刻、热送要求时刻、浇次号及浇次分割号不置换。
/// </para>
/// <para>b.出钢计划置换处理                                          </para>
/// <para>用未列入出钢计划的由操作员输入制造命令号指定的制造命令替代出钢计划中的被替代制造命令。
///从出钢计划外制造命令号文件中删除操作员输入的制造命令号,追加被替换的制造命令号。
/// </para>
/// <para>c.作业实绩替换处理                                          </para>
/// <para>当钢种替换变更输入时,MES对已收到的作业实绩中,把实绩文件中的制造命令号和
///出钢记号变更为替换制造命令号和替换出钢记号,其它不变；
/// </para>
/// <para>d. 生产管理模块的材料状态处理：                             </para>
/// <para>将钢种变更的两个制造命令下的材料状态进行重新计算；          </para>
/// <para>d.向L2发送钢种变更通知                                      </para>
/// <para>向已送出作业实绩的的L2系统送出钢种变更通知                  </para>
/// <para>e.炉次的成分重新判定，重新进行炉次品质判定。                </para>
/// <para>数据库表：TPSSM01/10/11/12/33 </para>
/// <para>主调用函数：前台PSSM18画面(钢水对换)调用              </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_chg_out_pono)
//-EP_SYSTEM_HEAD_END
int f_pssm18_chg_out_pono(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret = 0;
	CString datetime = "";	
	int    diff_type = 0;                 //判断钢种变更的两个PONO是否能换机浇注, >0不能, =0可换机
	CDecimal dummy = 0;
	CString sqlstr = "";
	CString cc_dev_code_out = "";
	CString cc_dev_code_in = "";
	CDecimal    cc_seq_out_max = 0;            /* 连铸顺序号:换出PONO所在连铸机的排入出钢计划炉次的最大浇注顺序号 */
	CDecimal    cc_seq_in_max = 0;             /* 连铸顺序号:换入PONO所在连铸机的排入出钢计划炉次的最大浇注顺序号 */	
	CDecimal v_area_id = 0;
	CString v_dev_code = "";
	bool is_have = false;
	int replan = 0; //重编计划标志: 0-不重编; 1-重新编制
	EIClass inBlock1;  //钢种变更发送电文
	EIClass inBlock2;  //调用PM接口
	EIClass inBlock3;  //调用MM接口
	EIClass inBlock4;
	EIClass inBlock5;
	EIClass inBlock6;
	EIClass inBlock7;
	EIClass outBlock;
	
	CModel in_tpssm11("TPSSM11");//替换用的计划
	CModel out_tpssm11("TPSSM11");//被变更出的计划
	CModel in_tpssm10("TPSSM10");//替换用的计划
	CModel out_tpssm10("TPSSM10");//被变更出的计划
	CModel tpssm01("TPSSM01");
	CModel tpssmd1("TPSSMD1");
	CModel tpssm12("TPSSM12");
	CModel tpssm11("TPSSM11");
	CModel tpssm10("TPSSM10");
	CModel tpssm99("TPSSM99");//履历
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssmd1_inq(conn);
	CDbCommand cmd_tpssm11_upd(conn);
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_tpssm01_upd(conn);
	CDbCommand cmd_tpssm12_upd(conn);
	CDbCommand cmd_tpssm33_upd(conn);
	EIClass in_pssm99trace;//调用履历函数
	in_pssm99trace.Tables[0].set_TableName("TRACE");//计划履历按一炉为单位
	in_pssm99trace.Tables[0].Clone(tpssm99);


	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//------------------------------------
		//输入参数定义
		//1)送MMS电文参数
		inBlock1.Tables[0].set_TableName("X200008");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO_OLD");//老的制造命令号
		inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO_NEW");//新的制造命令号

		//2)设置PM接口的输入参数:
		inBlock2.Tables[0].set_TableName("STNO_CHG");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO_OUT");//老的制造命令号
		inBlock2.Tables[0].Columns.Add(DT_STRING, "ST_NO_OUT");//老的内部钢种
		inBlock2.Tables[0].Columns.Add(DT_STRING, "PONO_IN");//新的制造命令号
		inBlock2.Tables[0].Columns.Add(DT_STRING, "ST_NO_IN");//新的内部钢种

		//3)设置MM接口的输入参数:
		inBlock3.Tables[0].set_TableName("MMSM0055");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_OUT");//老的制造命令号
		inBlock3.Tables[0].Columns.Add(DT_STRING, "ST_NO_OUT");//老的内部钢种
		inBlock3.Tables[0].Columns.Add(DT_STRING, "PONO_IN");//新的制造命令号
		inBlock3.Tables[0].Columns.Add(DT_STRING, "ST_NO_IN");//新的内部钢种

		//4)调用重新生成cast_no,cast_div_no的接口
		inBlock4.Tables[0].set_TableName("POUR");
		inBlock4.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");

		//5)设置校验函数的输入参数:
		inBlock5.Tables[0].set_TableName("TPSSM10");
		inBlock5.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock5.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock5.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");

		//6)设置QM接口的输入参数:
		inBlock6.Tables[0].set_TableName("STNO_CHG");
		inBlock6.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock6.Tables[0].Columns.Add(DT_STRING, "WHOLE_BACKLOG_CODE");//钢种变更的工序点
		inBlock6.Tables[0].Columns.Add(DT_STRING, "PONO_OUT");//老的制造命令号
		inBlock6.Tables[0].Columns.Add(DT_STRING, "ST_NO_OUT");//老的内部钢种 质量中该参数不用
		inBlock6.Tables[0].Columns.Add(DT_STRING, "PONO_IN");//新的制造命令号
		inBlock6.Tables[0].Columns.Add(DT_STRING, "ST_NO_IN");//新的内部钢种

		inBlock7.Tables[0].set_TableName("PONO_CHG_1");
		inBlock7.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock7.Tables[0].Columns.Add(DT_STRING, "OPER_FLAG");
		inBlock7.Tables[0].Columns.Add(DT_STRING, "PONO_OLD");//老的制造命令号
		inBlock7.Tables[0].Columns.Add(DT_STRING, "PONO_NEW");//新的制造命令号

		//-------------------------------------------------------
		//获得输入参数
		out_tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString().Trim();
		out_tpssm10["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO_OUT"].ToString().Trim();
		in_tpssm10["PONO"] = bcls_rec->Tables[0].Rows[0]["PONO_IN"].ToString().Trim();

		////Log::Info("", __FUNCTION__, "factory_div=[{0}]", out_tpssm10["FACTORY_DIV"].ToString());
		////Log::Info("", __FUNCTION__, "out_tpssm10.pono =[{0}]", out_tpssm10["PONO"].ToString());
		////Log::Info("", __FUNCTION__, "in_tpssm10.pono  =[{0}]", in_tpssm10["PONO"].ToString());

		tpssm01["REC_REVISE_TIME"] = datetime;
		tpssm10["REC_REVISE_TIME"] = datetime;
		tpssm01["REC_REVISOR"] = s.userid;
		tpssm10["REC_REVISOR"] = s.userid;
	
		//============================================================
		//1.保存数据，校验条件

		//读取换出PONO的计划内容		
		out_tpssm11["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		out_tpssm11["PONO"] = out_tpssm10["PONO"];
		is_have = out_tpssm11.Query("FACTORY_DIV,PONO");
		out_tpssm11.TrimOrBlank();

		//检查替换出的PONO状态
		if (is_have == false)
		{
			CFormattable arguments[] = { out_tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000088")/*制造命令号[{0}]在出钢计划中不存在。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}
		out_tpssm11["REC_REVISE_TIME"] = datetime;
		out_tpssm11["REC_REVISOR"] = s.userid;


		//增加钢种变更条件判断		
		if (out_tpssm11["RUN_STATUS"].ToString() < "20") //20-脱磷开始
		{
			CFormattable arguments[] = { out_tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000065")/*制造命令号[{0}]未生产，不能做钢种变更操作。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (out_tpssm11["RUN_STATUS"].ToString() > "51") //51-包到连铸（模铸）
		{
			CFormattable arguments[] = { out_tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000091")/*制造命令号[{0}]已开浇，不能做钢种变更操作。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//读取被变更换出的PONO的浇注顺内容		
		out_tpssm10.Query("FACTORY_DIV,PONO");
		out_tpssm10.TrimOrBlank();

		//-------------------------------------------------------------------
		//检查替换入的PONO存在否
		in_tpssm10["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		////Log::Trace("", __FUNCTION__, "factory_div=[{0}], out_pono=[{1}], in_pono=[{2}]", (const char*)in_tpssm10["FACTORY_DIV"].ToString(), (const char*)out_tpssm10["PONO"].ToString(), (const char*)in_tpssm10["PONO"].ToString());

		is_have = in_tpssm10.Query("FACTORY_DIV,PONO");
		in_tpssm10.TrimOrBlank();
		if (is_have == false)
		{
			CFormattable arguments[] = { in_tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000024")/*制造命令号[{0}]不存在。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (in_tpssm10["PONO_STATUS"].ToDecimal() >= 82) //82-浇注开始
		{
			CFormattable arguments[] = { in_tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000091")/*制造命令号[{0}]已开浇，不能做钢种变更操作。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//检查替换入的PONO在计划表TPSSM11中是否存在或生产	
		in_tpssm11["PONO"] = in_tpssm10["PONO"];
		in_tpssm11["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
		dummy = 0;
		dummy = in_tpssm11.QueryCount("FACTORY_DIV,PONO");
		if (dummy > 0)
		{
			CFormattable arguments[] = { in_tpssm10["PONO"].ToString() }; // 定义参数列表的数组
			CMessageFormat::Format(s.msg, _RES("PSSMS0000082")/*制造命令号[{0}]已排入出钢计划，不能做钢种变更操作。*/, arguments, 1); //格式化字符串
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//查询当前处理位的设备代码，给调用QM参数准备
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT DISTINCT AREA_ID, DEV_CODE FROM TPSSM12 \
					    WHERE FACTORY_DIV = @out_tpssm11.FACTORY_DIV  \
					      AND SM_PLAN_NO = @out_tpssm11.SM_PLAN_NO   \
					      AND CHARGE_NO	 = @out_tpssm11.CURR_WP_NO";
			break;
		}		
		cmd_tpssm11_inq.SetCommandText(sqlstr);
		cmd_tpssm11_inq.Parameters.Set("out_tpssm11.FACTORY_DIV",out_tpssm11["FACTORY_DIV"].ToString());
		cmd_tpssm11_inq.Parameters.Set("out_tpssm11.SM_PLAN_NO",out_tpssm11["SM_PLAN_NO"].ToString());
		cmd_tpssm11_inq.Parameters.Set("out_tpssm11.CURR_WP_NO",out_tpssm11["CURR_WP_NO"].ToDecimal());
		cmd_tpssm11_inq.ExecuteReader();
		if(cmd_tpssm11_inq.Read())
		{
			tpssmd1["AREA_ID"] = cmd_tpssm11_inq.GetDecimal(1);
			tpssmd1["DEV_CODE"] = cmd_tpssm11_inq.GetString(2).TrimOrBlank();
		}
		cmd_tpssm11_inq.Close();

		tpssmd1["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");
		

		//============================================================
		//2.PONO内容对换: 浇注顺(tpssm10), 计划表(TPSSM11/12)

		//------------------------------------------------------------
		//1).连铸的制造命令变更
		// 判断是否为同一连铸机
		if (out_tpssm10["CC_MACH_NO"].ToString() == in_tpssm10["CC_MACH_NO"].ToString())
		{
			/* 浇注顺 tpssm10 表中对应的顺序号,状态,  restrand_flg, cc_req_time 等替换
			* 1)浇注顺交换, 便于将原CAST的后续计划能排出"出钢计划"体
			//1.状态交换;
			//2.浇注顺, 重开机标记交换, 保证原始的浇次计划内容不变; (不能兼顾LOT的整体性:如同时2炉进入生产状态, 而做第1炉的交换)
			2)同一连铸机,浇注不同类型的钢坯时, 能否进行变更??????
			*/

			//修改被变更出的PONO, 将其改为变更入的信息
			tpssm10["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
			tpssm10["PONO"] = out_tpssm10["PONO"];
			tpssm10["CC_SEQ"] = in_tpssm10["CC_SEQ"];
			tpssm10["RESTRAND_FLG"] = in_tpssm10["RESTRAND_FLG"];
			tpssm10["PONO_STATUS"] = in_tpssm10["PONO_STATUS"];
			tpssm10.Update(
				"CC_SEQ,"
				"RESTRAND_FLG,"
				"PONO_STATUS,REC_REVISE_TIME,REC_REVISOR",
				"FACTORY_DIV,PONO"); /*浇铸顺序表*/

			//增加TPSSM01表的状态修改 2008-01-18 xuwen
			tpssm01["PONO_STATUS"] = 15;
			tpssm01["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
			tpssm01["PONO"] = out_tpssm10["PONO"];
			tpssm01.Update("PONO_STATUS,REC_REVISE_TIME,REC_REVISOR", "FACTORY_DIV,PONO");/*炉次命令表*/

			//修改被变更入的PONO
			tpssm10["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
			tpssm10["PONO"] = in_tpssm10["PONO"];
			tpssm10["CC_SEQ"] = out_tpssm10["CC_SEQ"];
			tpssm10["RESTRAND_FLG"] = out_tpssm10["RESTRAND_FLG"];
			tpssm10["PONO_STATUS"] = out_tpssm10["PONO_STATUS"];
			tpssm10.Update(
				"CC_SEQ,"
				"RESTRAND_FLG,"
				"PONO_STATUS,REC_REVISE_TIME,REC_REVISOR",
				"FACTORY_DIV,PONO");

			tpssm01["PONO_STATUS"] = 18;
			tpssm01["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
			tpssm01["PONO"] = in_tpssm10["PONO"];
			tpssm01.Update("PONO_STATUS,REC_REVISE_TIME,REC_REVISOR", "FACTORY_DIV,PONO");/*炉次命令表*/

			//读取连铸机设备代码
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:
				sqlstr = " SELECT DEV_CODE FROM TPSSMD1 \
						 	WHERE FACTORY_DIV = @out_tpssm10.factory_div \
							AND AREA_ID			= 5 \
							AND STATION_NO		= @out_tpssm10.cc_mach_no ";
				break;
			}

			cmd_tpssmd1_inq.SetCommandText(sqlstr);
			cmd_tpssmd1_inq.Parameters.Set("out_tpssm10.factory_div", out_tpssm10["FACTORY_DIV"].ToString());
			cmd_tpssmd1_inq.Parameters.Set("out_tpssm10.cc_mach_no", out_tpssm10["CC_MACH_NO"].ToString());
			cmd_tpssmd1_inq.ExecuteReader();
			if (cmd_tpssmd1_inq.Read())
			{
				cc_dev_code_out = cmd_tpssmd1_inq.GetString(1).Trim();
				cc_dev_code_in = cc_dev_code_out;
			}
			cmd_tpssmd1_inq.Close();
		}
		else //不同连铸机间的变更,有换机浇注要求
		{
			/* -------------------------------------------------
			* 判断是否是同种连铸机,以区分是否为产线间的钢种变更
			* 即检查PONO1换到PONO2的连铸机上和PONO2换到PONO1的连铸机上是否可行
			* 判断依据:
			* 1.PONO1与PONO2所在连铸机的类型检查: 可浇注的坯型, 连铸机的流数；通过连铸机设备参数表(TSISM32)来判断
			* 2.连铸机类型相同时, 检查PONO1的浇注的钢坯类型、浇铸流数和规格能否在PONO2的连铸机上浇注, 反之亦然
			* 3.浇注规格的检查, 只检查厚,宽
			*/

			//1.确定换出PONO和换入PONO能否换机浇注
			//1)检查换出PONO到换入PONO的连铸机
			CDataRow &row5 = inBlock5.Tables[0].Rows.Add();
			row5["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
			row5["PONO"] = out_tpssm10["PONO"];
			row5["CC_MACH_NO"] = in_tpssm10["CC_MACH_NO"];

			ret = f_pssm_ccmchg_chk_n(&inBlock5, bcls_ret, conn);   
			if (ret < 0)
			{
				strcpy(s.sysmsg, "f_pssm_ccmchg_chk()调用出错.");
				//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			diff_type = bcls_ret->Tables[0].Rows[0]["DIFF_TYPE"];

			if (diff_type == 0)//换出PONO可到换入PONO的连铸机浇注
			{
				//2)检查换入PONO到换出PONO的连铸机
				row5["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
				row5["PONO"] = in_tpssm10["PONO"];
				row5["CC_MACH_NO"] = out_tpssm10["CC_MACH_NO"];

				ret = f_pssm_ccmchg_chk_n(&inBlock5, bcls_ret, conn);
				if (ret < 0)
				{
					strcpy(s.sysmsg, "f_pssm_ccmchg_chk()调用出错.");
					//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				diff_type = bcls_ret->Tables[0].Rows[0]["DIFF_TYPE"];
				
			}

			////Log::Trace("", __FUNCTION__, "diff_type = [{0}]", diff_type);

			//判断是否为换机浇注
			if (diff_type == 0)//可换机浇注, 同种连铸机
			{
				//修改被变更出的PONO, 将其改为变更入的信息				
				tpssm10["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
				tpssm10["PONO"] = out_tpssm10["PONO"];
				tpssm10["CC_SEQ"] = in_tpssm10["CC_SEQ"];
				tpssm10["RESTRAND_FLG"] = in_tpssm10["RESTRAND_FLG"];
				tpssm10["PONO_STATUS"] = in_tpssm10["PONO_STATUS"];
				tpssm10["CC_MACH_NO"] = in_tpssm10["CC_MACH_NO"];
				tpssm10.Update(
					"CC_SEQ,"
					"RESTRAND_FLG,"
					"PONO_STATUS,"
					"CC_MACH_NO,REC_REVISE_TIME,REC_REVISOR",
					"FACTORY_DIV,PONO"); /*浇铸顺序表*/

				tpssm01["PONO_STATUS"] = 15;
				tpssm01["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
				tpssm01["PONO"] = out_tpssm10["PONO"];
				tpssm01.Update("PONO_STATUS,REC_REVISE_TIME,REC_REVISOR", "FACTORY_DIV,PONO");/*炉次命令表*/

				//修改被变更入的PONO				
				tpssm10["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
				tpssm10["PONO"] = in_tpssm10["PONO"];
				tpssm10["CC_SEQ"] = out_tpssm10["CC_SEQ"];
				tpssm10["RESTRAND_FLG"] = out_tpssm10["RESTRAND_FLG"];
				tpssm10["PONO_STATUS"] = out_tpssm10["PONO_STATUS"];
				//tpssm10["SMELT_MODE"] = out_tpssm10["SMELT_MODE"];
				//tpssm10["CC_REQ_TIME"] = out_tpssm10["CC_REQ_TIME"];
				tpssm10["CC_MACH_NO"] = out_tpssm10["CC_MACH_NO"];
				tpssm10.Update(
					"CC_SEQ,"
					"RESTRAND_FLG,"
					"PONO_STATUS,"
					"CC_MACH_NO,REC_REVISE_TIME,REC_REVISOR",
					"FACTORY_DIV,PONO");

				tpssm01["PONO_STATUS"] = 18;
				tpssm01["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
				tpssm01["PONO"] = in_tpssm10["PONO"];
				tpssm01.Update("PONO_STATUS,REC_REVISE_TIME,REC_REVISOR", "FACTORY_DIV,PONO");/*炉次命令表*/

				//可换机浇注情况下,因为连铸机交换了,所以连铸设备代码相应对换
				//1) 读取换出PONO连铸机设备代码				
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT DEV_CODE FROM TPSSMD1 \
							    WHERE FACTORY_DIV = @in_tpssm10.factory_div \
							      AND AREA_ID           = 5 \
								  AND STATION_NO        = @in_tpssm10.cc_mach_no ";
					break;
				}

				cmd_tpssmd1_inq.SetCommandText(sqlstr);
				cmd_tpssmd1_inq.Parameters.Set("in_tpssm10.factory_div", in_tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssmd1_inq.Parameters.Set("in_tpssm10.cc_mach_no", in_tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssmd1_inq.ExecuteReader();
				if (cmd_tpssmd1_inq.Read())
				{
					cc_dev_code_out = cmd_tpssmd1_inq.GetString(1).Trim();
				}
				cmd_tpssmd1_inq.Close();
				//2) 读取换入PONO连铸机设备代码				
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT DEV_CODE FROM TPSSMD1 \
							 	WHERE FACTORY_DIV = @out_tpssm10.factory_div \
								 AND AREA_ID           = 5 \
								 AND STATION_NO        = @out_tpssm10.cc_mach_no ";
					break;
				}

				cmd_tpssmd1_inq.SetCommandText(sqlstr);
				cmd_tpssmd1_inq.Parameters.Set("out_tpssm10.factory_div", out_tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssmd1_inq.Parameters.Set("out_tpssm10.cc_mach_no", out_tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssmd1_inq.ExecuteReader();
				if (cmd_tpssmd1_inq.Read())
				{
					cc_dev_code_in = cmd_tpssmd1_inq.GetString(1).Trim();
				}
				cmd_tpssmd1_inq.Close();
			}
			else//不同种连铸机且不可换机浇注时
			{
				/* 对不可换机浇注的钢种变更,需要先调整连铸计划(即浇次计划), 然后重启模型。

				/* 1.自动调整浇次计划:
				1)对换出PONO, 其浇注顺改为未排入出钢计划的第一炉;
				2)对换入PONO, 其浇注顺改为排入出钢计划的最后一炉;
				*/
				//(1)取换出PONO所在连铸机的排入出钢计划炉次的最大浇注顺序号
				#if 0
				cc_seq_out_max = 0;

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = CString(" SELECT  MAX(CC_SEQ) FROM TPSSM10 \
									    WHERE FACTORY_DIV = @out_tpssm10.factory_div \
										AND CC_MACH_NO = @out_tpssm10.cc_mach_no \
										AND PONO_STATUS   >= 18 ");/* 编入出钢计划 */
					break;
				}
				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("out_tpssm10.factory_div", out_tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssm10_inq.Parameters.Set("out_tpssm10.cc_mach_no", out_tpssm10["CC_MACH_NO"].ToString());
				cc_seq_out_max = cmd_tpssm10_inq.ExecuteScalar();

				//判断换出PONO是否就为排入出钢计划炉次的最后一炉
				if (out_tpssm10["CC_SEQ"].ToDecimal() == cc_seq_out_max)
				{
					//是，直接修改					
					tpssm10["PONO_STATUS"] = 16;
					tpssm10["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
					tpssm10["PONO"] = out_tpssm10["PONO"];
					tpssm10.Update("PONO_STATUS", "FACTORY_DIV,PONO");/*浇铸顺序表*/

					tpssm01["PONO_STATUS"] = 15;/*15-命令接收*/
					tpssm01["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
					tpssm01["PONO"] = out_tpssm10["PONO"];
					tpssm01.Update("PONO_STATUS", "FACTORY_DIV,PONO");/*炉次命令表*/
				}
				else //将换出PONO之后的并且编入出钢计划的炉次的浇注顺序号上移
				{
					
					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = " UPDATE TPSSM10 \
								 	SET CC_SEQ            = CC_SEQ - 1 \
									WHERE FACTORY_DIV	= @out_tpssm10.factory_div \
									 AND CC_MACH_NO		= @out_tpssm10.cc_mach_no \
									 AND CC_SEQ BETWEEN @out_tpssm10.cc_seq AND @cc_seq_out_max ";
						break;
					}

					cmd_tpssm10_upd.SetCommandText(sqlstr);
					cmd_tpssm10_upd.Parameters.Set("out_tpssm10.factory_div", out_tpssm10["FACTORY_DIV"].ToString());
					cmd_tpssm10_upd.Parameters.Set("out_tpssm10.cc_mach_no", out_tpssm10["CC_MACH_NO"].ToString());
					cmd_tpssm10_upd.Parameters.Set("out_tpssm10.cc_seq", out_tpssm10["CC_SEQ"].ToDecimal());
					cmd_tpssm10_upd.Parameters.Set("cc_seq_out_max", cc_seq_out_max);
					cmd_tpssm10_upd.ExecuteNonQuery();

					tpssm10["CC_SEQ"] = cc_seq_out_max;
					tpssm10["PONO_STATUS"] = 16;
					tpssm10["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
					tpssm10["PONO"] = out_tpssm10["PONO"];
					tpssm10.Update("CC_SEQ,PONO_STATUS", "FACTORY_DIV,PONO");

					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = " UPDATE TPSSM01 \
								 	SET PONO_STATUS       = 15 \
																		WHERE FACTORY_DIV = '" + out_tpssm10["FACTORY_DIV"].ToString() + "' \
									 AND PONO		        = '" + out_tpssm10["PONO"].ToString() + "' ";
						break;
					}

					cmd_tpssm01_upd.SetCommandText(sqlstr);
					cmd_tpssm01_upd.ExecuteNonQuery();

				}

				//1) 读取换出PONO连铸机设备代码				
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT DEV_CODE FROM TPSSMD1 \
							 	WHERE FACTORY_DIV = @out_tpssm10.factory_div \
								  AND AREA_ID           = 5 \
								  AND STATION_NO        = @out_tpssm10.cc_mach_no ";
					break;
				}

				cmd_tpssmd1_inq.SetCommandText(sqlstr);
				cmd_tpssmd1_inq.Parameters.Set("out_tpssm10.factory_div", out_tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssmd1_inq.Parameters.Set("out_tpssm10.cc_mach_no", out_tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssmd1_inq.ExecuteReader();
				if (cmd_tpssmd1_inq.Read())
				{
					cc_dev_code_out = cmd_tpssmd1_inq.GetString(1).Trim();
					cc_dev_code_in = cc_dev_code_out;
				}
				cmd_tpssmd1_inq.Close();

				//2)取换入PONO所在连铸机的排入出钢计划炉次的最大浇注顺序号
				cc_seq_in_max = 0;

				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = CString(" SELECT MAX(CC_SEQ) FROM TPSSM10 \
									    WHERE FACTORY_DIV = @in_tpssm10.factory_div \
										 AND CC_MACH_NO        = @in_tpssm10.cc_mach_no \
										 AND PONO_STATUS	   >= 18 ");/* 编入出钢计划 */
					break;
				}

				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("in_tpssm10.factory_div", in_tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssm10_inq.Parameters.Set("in_tpssm10.cc_mach_no", in_tpssm10["CC_MACH_NO"].ToString());
				cc_seq_out_max = cmd_tpssm10_inq.ExecuteScalar();
				//判断换入PONO是否为未排入出钢计划炉次的第一炉
				if (in_tpssm10["CC_SEQ"].ToDecimal() == cc_seq_in_max + 1)
				{
					tpssm10["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
					tpssm10["PONO"] = in_tpssm10["PONO"];
					tpssm10["PONO_STATUS"] = out_tpssm10["PONO_STATUS"];
					tpssm10.Update("PONO_STATUS", "FACTORY_DIV,PONO");

					tpssm01["PONO_STATUS"] = 18;
					tpssm01["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
					tpssm01["PONO"] = in_tpssm10["PONO"];
					tpssm01.Update("PONO_STATUS", "FACTORY_DIV,PONO");/*炉次命令表*/
				}

				else //将换入PONO调到未排入出钢计划炉次的第一炉
				{
					cc_seq_in_max = cc_seq_in_max + 1;

					switch (conn->DatabaseKind)
					{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default:
						sqlstr = "UPDATE TPSSM10 \
								     SET CC_SEQ            = CC_SEQ + 1 \
									  WHERE FACTORY_DIV = @in_tpssm10.FACTORY_DIV \
										AND CC_MACH_NO		   = @in_tpssm10.CC_MACH_NO \
										AND CC_SEQ BETWEEN @cc_seq_in_max AND @in_tpssm10.CC_SEQ ";
						break;
					}

					cmd_tpssm10_upd.SetCommandText(sqlstr);
					cmd_tpssm10_upd.Parameters.Set("in_tpssm10.FACTORY_DIV", in_tpssm10["FACTORY_DIV"].ToString());
					cmd_tpssm10_upd.Parameters.Set("in_tpssm10.CC_MACH_NO", in_tpssm10["CC_MACH_NO"].ToString());
					cmd_tpssm10_upd.Parameters.Set("cc_seq_in_max", cc_seq_in_max);
					cmd_tpssm10_upd.Parameters.Set("in_tpssm10.CC_SEQ", in_tpssm10["CC_SEQ"].ToDecimal());
					cmd_tpssm10_upd.ExecuteNonQuery();

					tpssm10["CC_SEQ"] = cc_seq_in_max;
					tpssm10["PONO_STATUS"] = out_tpssm10["PONO_STATUS"];
					tpssm10["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
					tpssm10["PONO"] = in_tpssm10["PONO"];
					tpssm10.Update("CC_SEQ,PONO_STATUS", "FACTORY_DIV,PONO");

					tpssm01["PONO_STATUS"] = 18;
					tpssm01["FACTORY_DIV"] = in_tpssm10["FACTORY_DIV"];
					tpssm01["PONO"] = in_tpssm10["PONO"];
					tpssm01.Update("PONO_STATUS", "FACTORY_DIV,PONO");/*炉次命令表*/

				}

				//3) 读取换入PONO连铸机设备代码
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " SELECT DEV_CODE FROM TPSSMD1 \
							 	WHERE FACTORY_DIV = @in_tpssm10.factory_div \
								AND AREA_ID           = 5 \
								AND STATION_NO        = @in_tpssm10.cc_mach_no ";
					break;
				}

				cmd_tpssmd1_inq.SetCommandText(sqlstr);
				cmd_tpssmd1_inq.Parameters.Set("in_tpssm10.factory_div", in_tpssm10["FACTORY_DIV"].ToString());
				cmd_tpssmd1_inq.Parameters.Set("in_tpssm10.cc_mach_no", in_tpssm10["CC_MACH_NO"].ToString());
				cmd_tpssmd1_inq.ExecuteReader();
				if (cmd_tpssmd1_inq.Read())
				{
					cc_dev_code_in = cmd_tpssmd1_inq.GetString(1).Trim();
				}
				cmd_tpssmd1_inq.Close();
				
				//----------------------------------------------------
				//根据自动调整后的浇次计划, 重新编制出钢计划(重启模型)
				//重编出钢计划前, 应将出钢计划内的PONO对换, 否则换出PONO的状态是生产中,计划不会将其删除
				replan = 1;
#endif

				strcpy(s.msg, "不同类型连铸机，规格不同，不能换机浇铸。");
				throw CApplicationException(-1, s.msg, log.Location);

			}//if 判断是否为换机浇注

		}//if 不同连铸机判断

		//------------------------------------------------------------
		//2).计划表TPSSM11/12 中的PONO变更; 将变更出的计划改为换入的计划内容

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " UPDATE TPSSM11 \
					 	SET REC_REVISOR	= '" + out_tpssm11["REC_REVISOR"].ToString() + "',\
						REC_REVISE_TIME   = '" + out_tpssm11["REC_REVISE_TIME"].ToString() + "',\
												 PONO	= '" + in_tpssm10["PONO"].ToString() + "',\
						 						ST_NO	= '" + in_tpssm10["ST_NO"].ToString() + "',\
												CC_MACH_NO	= '" + in_tpssm10["CC_MACH_NO"].ToString() + "' \
												WHERE FACTORY_DIV   = '" + out_tpssm10["FACTORY_DIV"].ToString() + "' \
						 AND PONO			    = '" + out_tpssm10["PONO"].ToString() + "' ";
			break;
		}

		cmd_tpssm11_upd.SetCommandText(sqlstr);
		cmd_tpssm11_upd.ExecuteNonQuery();

		tpssm12["DEV_CODE"] = cc_dev_code_in;
		tpssm12["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		tpssm12["SM_PLAN_NO"] = out_tpssm11["SM_PLAN_NO"];
		tpssm12["AREA_ID"] = 5;
		tpssm12.Update("DEV_CODE", "FACTORY_DIV,SM_PLAN_NO,AREA_ID");
		
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " UPDATE TPSSM12 \
					 	SET REC_REVISOR		= '" + out_tpssm11["REC_REVISOR"].ToString() + "',\
												 REC_REVISE_TIME	= '" + out_tpssm11["REC_REVISE_TIME"].ToString() + "'\
						 						   WHERE FACTORY_DIV = '" + out_tpssm10["FACTORY_DIV"].ToString() + "'\
						AND SM_PLAN_NO			    = '" + out_tpssm11["SM_PLAN_NO"].ToString() + "' ";
			break;
		}
		cmd_tpssm12_upd.SetCommandText(sqlstr);
		cmd_tpssm12_upd.ExecuteNonQuery();

		//------------------------------------------------------------
		//3).修改炼钢计划运行监控表(TPSSM33)
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " UPDATE TPSSM33 \
					      SET PONO              = @in_tpssm10.PONO, \
						  	  REC_REVISE_TIME   = @tpssm01.REC_REVISE_TIME, \
							  REC_REVISOR       = @tpssm01.REC_REVISOR \
						WHERE FACTORY_DIV = @out_tpssm10.FACTORY_DIV \
					      AND PONO			    = @out_tpssm10.PONO ";
			break; 
		}
		cmd_tpssm33_upd.SetCommandText(sqlstr);
		cmd_tpssm33_upd.Parameters.Set("in_tpssm10.PONO", in_tpssm10["PONO"].ToString());
		cmd_tpssm33_upd.Parameters.Set("out_tpssm10.FACTORY_DIV", out_tpssm10["FACTORY_DIV"].ToString());
		cmd_tpssm33_upd.Parameters.Set("out_tpssm10.PONO", out_tpssm10["PONO"].ToString());
		cmd_tpssm33_upd.Parameters.Set("tpssm01.REC_REVISE_TIME", tpssm01["REC_REVISE_TIME"].ToString());
		cmd_tpssm33_upd.Parameters.Set("tpssm01.REC_REVISOR", tpssm01["REC_REVISOR"].ToString());
		cmd_tpssm33_upd.ExecuteNonQuery();

		//------------------------------------------------
		if (replan == 0)
		{
			//重新编制出钢计划,重启模型
		}

		//============================================================
		//3. 调用PM, MM, QM的接口
#ifdef _SYS_PES		/* 定义了分层结构 */

		//CDataRow &row1 = inBlock1.Tables[0].Rows.Add();
		//row1["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		//row1["PONO_OLD"] = out_tpssm10["PONO"];
		//row1["PONO_NEW"] = in_tpssm10["PONO"];
		//ret = f_cm_pam1p3_snd(&inBlock1, &outBlock, conn);
		//if (ret != 0)
		//{
		//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
		//CDataRow &row7 = inBlock7.Tables[0].Rows.Add();
		//row7["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		//row7["PONO_OLD"] = out_tpssm10["PONO"];
		//row7["PONO_NEW"] = in_tpssm10["PONO"];
		//row7["OPER_FLAG"] = "1";
		//ret = f_pssm_pas1p3_snd(&inBlock7, &outBlock, conn);
		//if (ret != 0)
		//{
		//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}
#endif
		
		/*CDataRow &row2 = inBlock2.Tables[0].Rows.Add();
		row2["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		row2["PONO_OUT"] = out_tpssm10["PONO"];
		row2["ST_NO_OUT"] = out_tpssm10["ST_NO"];
		row2["PONO_IN"] = in_tpssm10["PONO"];
		row2["ST_NO_IN"] = in_tpssm10["ST_NO"];*/

		//1)调用MM接口
		CDataRow &row3 = inBlock3.Tables[0].Rows.Add();
		row3["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		row3["PONO_OUT"] = out_tpssm10["PONO"];
		row3["ST_NO_OUT"] = out_tpssm10["ST_NO"];
		row3["PONO_IN"] = in_tpssm10["PONO"];
		row3["ST_NO_IN"] = in_tpssm10["ST_NO"];
		ret = f_mmsm0055_proc(&inBlock3, &outBlock, conn);
		if (ret < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//2)调用重新生成cast_no,cast_div_no的接口
		CDataRow &row4 = inBlock4.Tables[0].Rows.Add();
		row4["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		if (out_tpssm10["CC_MACH_NO"].ToString() != in_tpssm10["CC_MACH_NO"].ToString())
		{
			if (out_tpssm10["RESTRAND_FLG"].ToString().Trim() == "T") //把换出连铸机且编入的计划的后面一炉次设置成“T”
			{
				switch (conn->DatabaseKind)
				{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	        // Oracle 数据库
				default:
					sqlstr = " UPDATE TPSSM10 \
							      SET RESTRAND_FLG = 'T' \
								  							    WHERE FACTORY_DIV = '" + out_tpssm10["FACTORY_DIV"].ToString() + "' \
							      AND CC_SEQ = ( SELECT MIN(CC_SEQ) FROM TPSSM10 \
								  							                      WHERE CC_MACH_NO = '" + out_tpssm10["CC_MACH_NO"].ToString() + "' \
							                        AND PONO IN ( SELECT PONO FROM TPSSM11 \
																				                                       WHERE FACTORY_DIV = '" + out_tpssm10["FACTORY_DIV"].ToString() + "' \
							                                        ) ) ";
					break;
				}

				cmd_tpssm10_upd.SetCommandText(sqlstr);
				cmd_tpssm10_upd.ExecuteNonQuery();

			}

			ret = f_pssm21_cast_cre_n(&inBlock4, &outBlock, conn);
			if (ret < 0)
			{
				//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}


		//3) 调用QM的接口
		CDataRow &row = inBlock6.Tables[0].Rows.Add();
		row["FACTORY_DIV"] = out_tpssm10["FACTORY_DIV"];
		row["PONO_OUT"] = out_tpssm10["PONO"];
		row["ST_NO_OUT"] = out_tpssm10["ST_NO"];
		row["PONO_IN"] = in_tpssm10["PONO"];
		row["ST_NO_IN"] = in_tpssm10["ST_NO"];
		row["WHOLE_BACKLOG_CODE"] = tpssmd1["STATION_ID"];

		////Log::Trace("", __FUNCTION__, "area_id=[{0}], dev_code=[{1}], whole_backlog_code=[{2}]", tpssmd1["AREA_ID"].ToDecimal().ToInt32(), (const char*)tpssmd1["DEV_CODE"].ToString(), (const char*)tpssmd1["STATION_ID"].ToString());
		////Log::Trace("", __FUNCTION__, "factory_div=[{0}], out:pono[%s]st_no[{1}] in:pono[{2}] st_no[{3}]",
			//out_tpssm10["FACTORY_DIV"], out_tpssm10["PONO"].ToString(), out_tpssm10["ST_NO"].ToString(), in_tpssm10["PONO"].ToString(), in_tpssm10["ST_NO"].ToString());

		//调用QM接口
		ret = f_qmts_stno_chg(&inBlock6, &outBlock, conn);
		if (ret < 0)
		{
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//写履历表----dclian---add----2015-11-20
		tpssm99["FACTORY_DIV"] = in_tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = in_tpssm10["PONO"];
		//tpssm99["HEAT_NO"] = in_tpssm10.HEAT_NO;
		tpssm99["PONO_STATUS"] = in_tpssm10["PONO_STATUS"];
		tpssm99["ST_NO"] = in_tpssm10["ST_NO"];
		tpssm99["EVENT_ID"] = "E2";
		
		tpssm99["PONO_OLD"] = out_tpssm11["PONO"];
		tpssm99["HEAT_NO_OLD"] = out_tpssm11["HEAT_NO"];
		tpssm99["ST_NO_OLD"] = out_tpssm11["ST_NO"];

		////Log::Trace("", __FUNCTION__, "doFlag=[{0}]", doFlag);

		if (doFlag == -1)
		{//发生异常
			tpssm99["VALID_FLAG"] = "0";

		}
		else
		{
			tpssm99["VALID_FLAG"] = "1";
		}

		
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		//针对交换炉次PONO2
		tpssm99["FACTORY_DIV"] = out_tpssm11["FACTORY_DIV"];
		tpssm99["PONO"] = out_tpssm11["PONO"];
		tpssm99["HEAT_NO"] = out_tpssm11["HEAT_NO"];
		tpssm99["PONO_STATUS"] = out_tpssm11["PONO_STATUS"];
		tpssm99["ST_NO"] = out_tpssm11["ST_NO"];
		
		tpssm99["EVENT_ID"] = "E2";
		
		tpssm99["PONO_OLD"] = in_tpssm10["PONO"];
		//tpssm99["HEAT_NO_OLD"] = in_tpssm10.HEAT_NO;
		tpssm99["ST_NO_OLD"] = in_tpssm10["ST_NO"];
		tpssm99.MergeTo(in_pssm99trace.Tables[0], false);
		////Log::Trace("", __FUNCTION__, "事件号tpssm99["EVENT_ID"] =[{0}]", tpssm99["EVENT_ID"].ToString());
		////Log::Trace("", __FUNCTION__, "记录履历传入块行数=[{0}]", in_pssm99trace.Tables[0].Rows.get_Count());
		//记录履历
		ret = 0;
		ret = f_pssm99_trace(&in_pssm99trace, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
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
	cmd_tpssm11_inq.Close();
	cmd_tpssm10_inq.Close();
	cmd_tpssmd1_inq.Close();
	return doFlag;
}
