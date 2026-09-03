/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2015-10-12
Version:1.0
Description: 甘特图一键删除
Update: 
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
//#include "tpssm10.h"
//
//#include "tpssm12.h"


int f_pssm11_del_heat_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢作业计划删除,标志为“D”的炉次
int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢作业计划删除,标志为“D”的炉次
int f_pssm11_planno_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢作业计划删除,标志为“D”的炉次
int f_pssm12_sequ_calc_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢作业计划删除,标志为“D”的炉次
//int f_pssm_pas1p1_snd(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);//出钢计划下达电文

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 状态回退
/// <para>数据库表：tpssm10/11/12/13/14/31/25                    </para>
/// <para>主调用函数：PSSM18S画面调用。                </para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <returns></returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm18_all_del)
//-EP_SYSTEM_HEAD_END
int f_pssm18_all_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0, ret = 0;
	int length = 0;
	CDecimal count = 0;
	int count_upd = 0;

	CString sqlstr = "";
	CString v_factory_div = "";	//厂别区分
	CString v_cc_mach_no = "";//连铸机号

	EIClass inblk;        //调用函数用
	EIClass inSendL2;

	CModel tpssm11("TPSSM11");

	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm11_upd(conn);	

	try
	{
		//定义块
		inblk.Tables[0].set_TableName("PLAN");  //
		inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //厂别区分
		inblk.Tables[0].Rows.Add();

		//下发L2
		inSendL2.Tables[0].set_TableName("PAS");
		inSendL2.Tables[0].Columns.Add(DT_STRING, "OPER_FLAG");  //
		inSendL2.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	/*	inSendL2.Tables[0].Columns.Add(DT_STRING, "PONO");*/
		inSendL2.Tables[0].Rows.Add();

		//获取输入参数
		v_factory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		v_cc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"];

		//打印输入参数
		////Log::Info("", __FUNCTION__, "FACTORY_DIV=[{0}]", v_factory_div);
		////Log::Info("", __FUNCTION__, "CC_MACH_NO=[{0}]", v_cc_mach_no);

		//块赋值
		inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		
		length = v_cc_mach_no.GetLength();
		////Log::Info("", __FUNCTION__, "length=[{0}]", length);
		
		for (int i = 0; i < length; i++)
		{
			tpssm11["CC_MACH_NO"] = v_cc_mach_no.SubstringNE(i, 1);//对铸机号进行截位
			////Log::Info("", __FUNCTION__, "tpssm11["CC_MACH_NO"] =[{0}]", tpssm11["CC_MACH_NO"].ToString());

			//增加判断，TPSSM11有则更新
			sqlstr = "SELECT COUNT(*) FROM TPSSM11 "
				"  WHERE PONO_STATUS <20 "
				"  AND RUN_STATUS ='00'"
				"  AND CURR_WP_NO = 0 "
				"  AND CC_MACH_NO = @CC_MACH_NO ";

			cmd_tpssm11_inq.SetCommandText(sqlstr);
			cmd_tpssm11_inq.Parameters.Set("CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());

			count = cmd_tpssm11_inq.ExecuteScalar();
			if (count <= 0)
			{
				continue;
			}
			else
			{
				count_upd++;
			}
			cmd_tpssm11_inq.Close();


			//1、把未生产的计划的PLAN_EDIT_FLAG设成置‘D’
			//增加 连铸机选择，chejs/20151029
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default:

				sqlstr = " UPDATE TPSSM11 SET  PLAN_EDIT_FLAG ='D'  "
					"  WHERE PONO_STATUS <20 "
					"  AND RUN_STATUS ='00'"
					"  AND CURR_WP_NO = 0 "
					"  AND CC_MACH_NO = @CC_MACH_NO "
					;
				break;
			}
			cmd_tpssm11_upd.SetCommandText(sqlstr);
			cmd_tpssm11_upd.Parameters.Set("CC_MACH_NO", tpssm11["CC_MACH_NO"].ToString());
			cmd_tpssm11_upd.ExecuteNonQuery();
			cmd_tpssm11_upd.Close();
		}	

		//有更新数据才做后面操作
		////Log::Info("", __FUNCTION__, "count_upd = [{0}]", count_upd);
		if (count_upd > 0)
		{
			////下发出钢计划电文给L2
			//inSendL2.Tables[0].Rows[0]["OPER_FLAG"] = "D"; //I：新增，D删除。
			//inSendL2.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div.Trim();
			//////Log::Trace("", __FUNCTION__, "发送L2制造命令删除电文");
			//ret = f_pssm_pas1p1_snd(&inSendL2, bcls_ret, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			//2、删除编制标志“D”的炉次，必须先删，再做后续的计算
			////Log::Trace("", __FUNCTION__, "检查是否有需要删除的PONO");
			ret = f_pssm11_del_heat_n(&inblk, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//3、重新计算浇次号
			////Log::Trace("", __FUNCTION__, "重新计算CAST号");
			ret = f_pssm21_cast_cre_n(&inblk, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//4、重新计算计划号
			////Log::Trace("", __FUNCTION__, "重新计算计划号");
			ret = f_pssm11_planno_n(&inblk, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//5、重新计算处理号
			////Log::Trace("", __FUNCTION__, "重新计算处理号");
			ret = f_pssm12_sequ_calc_n(&inblk, bcls_ret, conn);
			if (ret < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
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
