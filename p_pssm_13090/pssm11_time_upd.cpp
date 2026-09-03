/*=========================================================================
//程序名称:     pssm11_time_inq
//隶属子系统:   PSSM
//产品名称:     BM2MES
//创建人员:     ZHP
//创建时间:     2016/11/16 9:52:35
//修改人员:
//修改日期:
//=========================================================================*/
#include "stdafx.h"



/*<remark>=========================================================
/// <summary>
/// 出钢计划熔炼时间查询
/// <para>根据炉座号，查询每个计划的熔炼时间。       </para>
/// <para>1.根据传入的主工序代码，查询所有炼钢设备。            </para>
/// <para>2.查询选择的计划内容,包括主计划和工序计划内容。       </para>
/// <para>数据库表：TPSSM11/12(炼钢出钢计划表)                  </para>
/// <para>主调用函数：前台PSSM11DevDlg(设备及工序调整),FormPSSM11SeqDlg(顺序调整)画面调用。</para>
/// </summary>
/// <param name="main_backlog_code">炼钢厂别代码     </param>
/// <param name="pono">制造命令号                    </param>
/// <returns>无</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm11_time_upd)


int f_pssm11_time_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int i, rows;
	int doFlag = 0;

	CString datetime = "";
	CString v_start_time = "";

	// 定义表的实体对象
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");

	CString sqlstr = "";
	CDbCommand cmd_inq(conn);

	try
	{
		//--------------------------------------------------------------
		//获得输入参数
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		tpssm11["FACTORY_DIV"] = bcls_rec->Tables["MAIN"].Rows[0]["FACTORY_DIV"].ToString();
		////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}]", tpssm11["FACTORY_DIV"].ToString());

		rows = bcls_rec->Tables["BOF"].Rows.get_Count();
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm11["PONO"]	= bcls_rec->Tables["BOF"].Rows[i]["PONO"].ToString();
			v_start_time	= bcls_rec->Tables["BOF"].Rows[i]["START_TIME"].ToString();
			
			////Log::Trace("", __FUNCTION__, "PONO=[{0}]", tpssm11["PONO"].ToString());
			////Log::Trace("", __FUNCTION__, "START_TIME=[{0}]", tpssm12["START_TIME"].ToString());
			
			tpssm11["PONO"] = tpssm11["PONO"];
			//判断是否有原记录
			sqlstr = "tpssm11.QueryCount()";

			if (tpssm11.Query("FACTORY_DIV, PONO") == false) //没有原记录
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "出钢计划中没有制造命令[{0}]，请查询后再调整!", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm12["FACTORY_DIV"] = tpssm11["FACTORY_DIV"];
			tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
			tpssm12["AREA_ID"] = 3;

			if (tpssm12.Query("FACTORY_DIV, SM_PLAN_NO, AREA_ID") == false) //没有原记录
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令[{0}]在转炉区域没有工序信息，不能调整时间。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (tpssm12["PROC_NO"].ToString().Trim() != "") //进入生产
			{
				CFormattable arguments[] = { tpssm11["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令[{0}]已进入冶炼，不能修改时间。。", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			tpssm12["START_TIME"] = v_start_time;
			tpssm12.Update("START_TIME", "FACTORY_DIV, SM_PLAN_NO, CHARGE_NO, DEV_CODE");

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

	return doFlag;
}
