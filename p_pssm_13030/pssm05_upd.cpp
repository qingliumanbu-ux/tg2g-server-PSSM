/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 038887
日期: 2014-12-08
功能: 炉次热送试验修改
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:
**************************************************/

#include "stdafx.h"


/*<remark >========================================================= 
/// <summary > 
/// 炉次热送试验修改
/// <para > 
/// 1.根据传入的PONO号修改热送试验；
/// </para > 
/// <para > 数据库表：TPSSM01(制造命令炉次表)  </para > 
/// <para > 主调用函数：前台PSSM05画面按钮(热试修改)调用。 </para > 
/// </summary > 
/// <returns > 成功：0</returns > 
/// <returns > 失败：-1</returns > 
=========================================================== </remark > */
BM2F_ENTERACE(pssm05_upd);

int f_pssm05_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;	//返回值
	int j = 0;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*业务变量*/
	CString strCast_lot_no = "";
	CString strPono = "";
	CString strDiv_flag = "";
	CString v_heat_hot_charge_flag = "";
	CString strUnit_code = "";
	CString strCc_mach_no = "";

	/*实体类定义*/
	CModel tpssm01("TPSSM01");

	try
	{	
		/*获取前台输入数据*/ 
		//CString strCc_mach_no = bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();/*连铸机号*/
		//CString strUnit_code = bcls_rec->Tables[0].Rows[0]["UNIT_CODE"].ToString();/*炼钢区分*/
		//////Log::Trace("",__FUNCTION__,"strCc_mach_no = [{0}]strUnit_code[{1}]",strCc_mach_no,strUnit_code);
		for(int i = 0;i < bcls_rec->Tables[0].Rows.get_Count();i++)
		{
			strCast_lot_no = bcls_rec->Tables[0].Rows[i]["CAST_LOT_NO"].ToString().Trim();/*CAST_LOT号*/
			strPono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().Trim();/*PONO号*/
			strDiv_flag = bcls_rec->Tables[0].Rows[i]["DIV_FLAG"].ToString().Trim();/*热送试验标记*/
			////Log::Trace("",__FUNCTION__,"strCast_lot_no = [{0}]strDiv_flag[{1}]",strCast_lot_no,strDiv_flag);
			
			sqlstr = " UPDATE  PSSM.TPSSM01  "
					 " SET  DIV_FLAG = @div_flag "
					 " WHERE PONO = @pono"
					 " AND   PONO_STATUS IN(2,3)";

			/*修改炉热装标记*/
			CDbCommand cmd_up1(sqlstr,conn);
			cmd_up1.Parameters.Set("div_flag" ,strDiv_flag);
			cmd_up1.Parameters.Set("pono" ,strPono);
			cmd_up1.ExecuteNonQuery();
			j = j++;

		}
		if (j > 0 )
		{
			strcpy(s.msg,"修改热送试验代码成功");
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
