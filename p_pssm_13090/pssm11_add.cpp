/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    xuwen
Version:    3.1
Date:      2014-11-10
Description: 出钢计划新增编制
**************************************************/
#include "stdafx.h"

  //出钢计划表
//#include "tpssm20.h"  //出钢计划整体约束及状态表


//int f_pssm11_set_dev(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);  //炉机对应调整

//调用TPSSM11表计划插入函数
int f_pssm11t_ins_pono(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
//CAST计算函数
int f_pssm11t_cast(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
//工序作业时刻计算
int f_pssm12_time_calc(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);  
//根据作业时刻，赋流水号
int f_pssm12_sequ_calc_n(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
//计算计划号
int f_pssm11t_planno(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn);
////浇铸顺联动调整
//int f_pssm10_upd_by_cast(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

//调用TPS模型
int f_pssm_call_tps(CString main_backlog_code, int mode, int bof_cc_matching, CDbConnection * conn);//


/*<remark>=========================================================
/// <summary>
/// 新增出钢计划（拉模式，根据连铸浇铸顺及时刻，推算各工序时刻）
/// <para>1.根据各铸机选择的PONO顺序，生成出钢计划。                </para>
/// <para>2.根据新的浇铸顺，生成CAST号。                            </para>
/// <para>3.推算各PONO的开浇时刻, 及各工序的作业时刻；              </para>
/// <para>4.推算各精炼的顺序号。                                    </para>
/// <para>5.生成计划号，并修改计划主表的计划编辑标记。              </para>
/// <para>数据库表：tpssm11/12(炼钢出钢计划主表)                    </para>
/// <para>主调用函数：前台PSSM11画面F3(计划新增)调用。              </para>
/// </summary>
/// <param name="sm_unit_no">炼钢厂别代码     </param>
/// <param name="PONO">制造命令号                    </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_add)

int f_pssm11_add(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	//程序用变量
	int doFlag = 0;
	int ret;
	int blkseq = 0;

	/* 业务变量 */
	CString v_sm_unit_no = "";	//炼钢单元号
	int     bof_cc_matching = 0;     /*炉机匹配标记: 0-不指定,模型推荐; 1-指定,模型按要求计算*/
	int		mode = 0;      //调用模型功能:  0-模型;   1-匹配;

	CString date_time = "";
	CString v_factory_div = "";

	CString sqlstr = "";
	CModel tpssm11("TPSSM11");
	CDbCommand cmd_upd(conn);

	try
	{
		date_time = CDateTime::Now().ToString("yyyyMMddHHmmss");

		v_factory_div = bcls_rec->Tables["POUR"].Rows[0]["FACTORY_DIV"].ToString();

		////----------------------------------------------------------
		////1.炉机对应调整
		//ret = f_pssm11_set_dev(bcls_rec, bcls_ret, conn);
		//if (ret < 0)
		//{
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		//----------------------------------------------------------
		//1.生成出钢计划
		ret = f_pssm11t_ins_pono(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{  
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//----------------------------------------------------------
		//2.计算CAST: CAST_NO计算、浇注周期计算、浇注时刻计算
		ret = f_pssm11t_cast(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{  
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------------
		//3.根据传入的炉次，依据CAST的时刻，推算各工序的作业时刻（有模型计算，可省略本计算）
		ret = f_pssm12_time_calc(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////----------------------------------------------------------
		////模型计算
		////mode=0 正常编制   mode=1 时间优化 mode=2 局部编制  mode=3 转炉编制  mode=4 非转炉编制 mode=5 模铸编制
		////mode = bcls_rec->Tables[1].Rows[0]["MODE"];
		//mode = 1;
		//bof_cc_matching = 1;
		//ret = f_pssm_call_tps(v_sm_unit_no, mode, bof_cc_matching, conn);
		//ret = 0;
		//if (ret < 0)
		//{
		//	//strcpy(s.msg, "模型调用出错.");
		//	throw CApplicationException(-1, s.msg, log.Location);
		//}

		int blkNum = bcls_rec->Tables.IndexOf("PLAN");
		if (blkNum <= 0)
		{
			bcls_rec->Tables.Add("PLAN");
			blkNum = bcls_rec->Tables.IndexOf("PLAN");
		}
		else
		{
			bcls_rec->Tables["PLAN"].Clear();
		}
		bcls_rec->Tables["PLAN"].Columns.Add(DT_STRING, "FACTORY_DIV");
		bcls_rec->Tables["PLAN"].Rows.Add();

		bcls_rec->Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;

		//----------------------------------------------------------
		//4.根据作业时刻，赋流水号（预定炉号）
		ret = f_pssm12_sequ_calc_n(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{  
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//----------------------------------------------------------
		//5.浇铸顺联动调整(编入出钢计划的炉次顺序有变动时的TPSSM10表调整，一定在计算CAST后调用)
		//ret = f_pssm10_upd_by_cast(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{  
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//----------------------------------------------------------
		//6.生成计划号
		ret = f_pssm11t_planno(bcls_rec, bcls_ret, conn);
		if (ret < 0)
		{  
			////Log::Trace("", __FUNCTION__, "msg=[{0}]", s.msg);
			//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}


		//修改计划主表的计划编辑标记
		sqlstr = CString(" UPDATE TPSSM11 SET PLAN_EDIT_FLAG = ' ' WHERE FACTORY_DIV = @v_factory_div "); 
		cmd_upd.SetCommandText(sqlstr);
		cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
		cmd_upd.ExecuteNonQuery();


		////----------------------------------------------------------
		////修改出钢计划应答表的计划编辑标记（整体计划状态）
		//sqlstr = CString(
		//	" UPDATE TPSSM23 SET "
		//	" PLAN_EDIT_FLAG = '1' " //1-新增计划
		//	",PLAN_EDIT_TIME = @date_time " //计划编辑时刻
		//	" WHERE FACTORY_DIV = v_factory_div  "
		//	);
		//cmd_upd.SetCommandText(sqlstr);
		//cmd_upd.Parameters.Set("date_time", date_time);
		//cmd_upd.Parameters.Set("v_factory_div", v_factory_div);
		//cmd_upd.ExecuteNonQuery();


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
