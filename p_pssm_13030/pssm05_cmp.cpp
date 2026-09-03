/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 沈敏
日期: 2012-03-15
功能: 连铸计划编制确定
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
/// 连铸计划编制确定
/// <para > 
/// 1.根据传入的计划日期、连铸机号、炼钢区分来完成计划编制；
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表)     </para > 
/// <para > 主调用函数：前台PSSM05画面F11(确定)调用。   </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
BM2F_ENTERACE(pssm05_cmp);

int f_pssm05_cmp(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值
	int j = 0;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*业务变量*/
	CString strCast_lot_no;
	CString strFactory_div = "";
	CString strCc_mach_no = "";
	CString strPlan_date = "";
	CString cc_req_time = "";
	CString strPono = "";

	/*实体类定义*/
	CModel tpssm01("TPSSM01");

	try
	{	
		/*获取前台输入数据*/ 
		CString strCc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();/*连铸机号*/
		CString strFactory_div = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();/*炼钢区分*/
		CString strPlan_date = bcls_rec->Tables[0].Rows[0]["PLAN_DATE"].ToString();/*计划日期*/

		for (int i = 0; i <bcls_rec->Tables[1].Rows.get_Count();i++)
		{
			////Log::Trace("",__FUNCTION__,"开始读取外循环CAST_LOT_NO");
			strCast_lot_no = bcls_rec->Tables[1].Rows[i]["CAST_LOT_NO"].ToString().Trim();/*CAST_LOT号*/
			////Log::Trace("",__FUNCTION__,"外循环传入的LOT_NO strCast_lot_no = [{0}]",(const char * )strCast_lot_no);
			if  (strCast_lot_no != "" )
			{
				sqlstr = "SELECT CC_REQ_TIME,PONO "
						 " FROM TPSSM01"
						 " WHERE CAST_LOT_NO = @cast_lot_no "
						 " AND   FACTORY_DIV = @factory_div "
						 " AND   PONO_STATUS = 12 "
						 " ORDER BY CAST_LOT_DIV_NO ";

				CDbCommand cmd_inq(sqlstr,conn);
				cmd_inq.Parameters.Set("factory_div", strFactory_div);
				cmd_inq.Parameters.Set("cast_lot_no" ,strCast_lot_no);
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
					cc_req_time = cmd_inq.GetString(1).Trim();
					strPono = cmd_inq.GetString(2).Trim();
					if (cc_req_time == "")
					{
						CFormattable arguments[] = { strPono };
						CMessageFormat::Format(s.msg,  "该炉次[{0}]开浇时间未计算,请先计算再确定", arguments, 1);
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				cmd_inq.Close();

				sqlstr = " UPDATE  TPSSM01 "
					     " SET  PONO_STATUS = 13"
					     " WHERE CAST_LOT_NO = @cast_lot_no "
						 " AND   FACTORY_DIV = @factory_div "
					     " AND   PONO_STATUS = 12 ";

				CDbCommand cmd_upd(sqlstr,conn);
				cmd_upd.Parameters.Set("factory_div", strFactory_div);
				cmd_upd.Parameters.Set("cast_lot_no" ,strCast_lot_no);
				cmd_upd.ExecuteNonQuery();
				j++;
			}
		}
		
		if(j ==  0)
		{
			strcpy(s.msg,"请选择需要确定的CAST-LOT号");
		}
		else
		{
			strcpy(s.msg,"计划编制完成");
			//_RES("PSSMS0000177")/*计划编制完成*/
		}
	}

	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode = [{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
