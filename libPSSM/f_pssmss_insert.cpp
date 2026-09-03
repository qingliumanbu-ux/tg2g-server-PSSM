/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
/**
*                             _ooOoo_
*                            o8888888o
*                            88" . "88
*                            (| -_- |)
*                            O\  =  /O
*                         ____/`---'\____
*                       .'  \\|     |//  `.
*                      /  \\|||  :  |||//  \
*                     /  _||||| -:- |||||-  \
*                     |   | \\\  -  /// |   |
*                     | \_|  ''\---/''  |   |
*                     \  .-\__  `-`  ___/-. /
*                   ___`. .'  /--.--\  `. . __
*                ."" '<  `.___\_<|>_/___.'  >'"".
*               | | :  `- \`.;`\ _ /`;.`/ - ` : | |
*               \  \ `-.   \_ __\ /__ _/   .-` /  /
*          ======`-.____`-.___\_____/___.-`____.-'======
*                             `=---='
*          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*                     佛祖保佑        永无BUG
*            佛曰:
*                   写字楼里写字间，写字间里程序员；
*                   程序人员写程序，又拿程序换酒钱。
*                   酒醒只在网上坐，酒醉还来网下眠；
*                   酒醉酒醒日复日，网上网下年复年。
*                   但愿老死电脑间，不愿鞠躬老板前；
*                   奔驰宝马贵者趣，公交自行程序员。
*                   别人笑我忒疯癫，我笑自己命太贱；
*                   不见满街漂亮妹，哪个归得程序员？
*/
#include "stdafx.h"

//程序用头文件



#if defined _SYS_PES || defined _SYS_MES


#endif

BM2_FUNCTION_EXPORT
int f_pssmss_insert(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	CString maxseq = "";
	CString newSeqNo = "";
	int blkseq = 0;

	CModel tpssmss("TPSSMSS");
#if defined _SYS_PES || defined _SYS_MES
#endif

	CDbCommand cmd_inq(conn);
	CString sqlstr;


	try
	{ 
		tpssmss["ID_SJ"] = bcls_rec->Tables[0].Rows[0]["ID"];
		if (tpssmss.QueryCount("ID_SJ")>0)
		{
			tpssmss.Delete("ID_SJ");
		}
		tpssmss["DEV_CODE"] = bcls_rec->Tables[0].Rows[0]["AGGREGATE_NAME"];
		tpssmss["EVENT_ID"] = bcls_rec->Tables[0].Rows[0]["EVENT"];
		tpssmss["EVENT_TIME"] = bcls_rec->Tables[0].Rows[0]["EVENT_TIME"].ToString();
		tpssmss["START_TIME"] = bcls_rec->Tables[0].Rows[0]["EVENT_TIME"].ToString(); 
		tpssmss["ST_NO"] = bcls_rec->Tables[0].Rows[0]["GRADE"];
		tpssmss["HEAT_NO"] = bcls_rec->Tables[0].Rows[0]["HEAT_NUMBER"];
		tpssmss["STATUS_NAME"] = bcls_rec->Tables[0].Rows[0]["HEAT_STATUS"];
		tpssmss["LADLE_NO"] = bcls_rec->Tables[0].Rows[0]["LADLE_NUMBER"];
		tpssmss["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"];
		tpssmss["SPLIT_INDICATION"] = bcls_rec->Tables[0].Rows[0]["SPLIT_INDICATION"];
		tpssmss["TREATMENT_COUNTER"] = bcls_rec->Tables[0].Rows[0]["TREATMENT_COUNTER"];
		tpssmss["NOTE"] = bcls_rec->Tables[0].Rows[0]["NOTE"];

		if (tpssmss["SM_PLAN_NO"].ToString().Trim().GetLength() > 8)
		{
			tpssmss["SM_PLAN_NO"] = bcls_rec->Tables[0].Rows[0]["PLAN_NUMBER"].ToString().Substring(0, 8);
		}

		tpssmss.TrimOrBlank(); 
		tpssmss.Insert();
		tpcommit(0);
		tpbegin(0, 0);
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		////Log::Trace("", __FUNCTION__, "tpssm99--ex.GetCode()=[{0}]", ex.GetCode());
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		////Log::Trace("", __FUNCTION__, "s.flag=[{0}],doFlag=[{1}]", s.flag, doFlag);
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

