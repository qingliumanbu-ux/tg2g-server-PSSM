/*=========================================================================
//程序名称:     f_pssm01_pour_time
//隶属子系统:   PSBW
//产品名称:     BSM1
//创建人员:
//创建时间:     2016-6-27
//修改人员:
//修改日期:
// 计算方法:
//A)计算PONO内总的板坯块数S
//B)加权平均宽度W = 累计（板坯宽度×（1÷S））
//C)实际操作拉速L = 工艺规定最大的拉速 － 0.2  (工艺卡规定最大的拉速:需质量有)
//D)加权平均炉次浇铸时间计算 ：炉浇铸时间（分钟） ＝ 炉产量 ÷（厚度×比重×拉速×宽度×2）
//=========================================================================*/
#include "stdafx.h"





BM2_FUNCTION_EXPORT
int f_pssm01_pour_time(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序用变量
	int doFlag = 0;
	int fetchRowCount = 0;
	int blknum = 0;
	CDecimal v_cnt = 0;                  /* 计数 */


	EIClass inBlock;
	EIClass outBlock;
	CString slab_no;
	CString v_slab_no;

	/*定义业务用变量*/
	CString v_factory_div = "";
	CString v_pono = "";
	CString v_cc_mach_no = "";
	CString plan_date = "";
	CString pono = "";
	CString cc_req_time = "";
	CString strCast_lot_no = "";
	CString strPono = "";
	CString strSt_no = "";
	CString strPrev_st_no = "";
	CString strTemp_whole_backlog;
	CString v_plan_date = "";
	CDecimal v_cc_seq = 0;
	CDecimal slab_num = 0;
	CDecimal sum_slab_width = 0;
	CDecimal value_md = 0;
	CDecimal value_lz = 0;
	CDecimal value_dj = 0;
	CDecimal v_seq_no = 0;
	CDecimal cast_speed_max = 0;
	CDateTime cc_req_time_max;
	CDateTime cc_req_time_nom;
	CDateTime cc_req_times;
	CDateTime cc_req_times_prev;
	CDateTime v_plan_date_prev;
	CDecimal t_element_wk = 0;
	CDecimal pour_times_prev = 0;
	CDecimal pour_times = 0;
	CDecimal slab_thick = 0;
	CDecimal slab_width = 0;
	CDecimal slab_len = 0;
	CDecimal strand_num = 0;
	double pour_time;
	double pour_time_prev;
	CString v_cc_req_time = "";
	CString pono_prev = "";
	CString shift_no = "";
	CString shift_group = "";
	CString v_prev_plan_date = "";
	CString billet_type = "";
	CDecimal slab_thick_max = 0;
	CDecimal slab_width_max = 0;

	CString sqlstr = ""; 
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm02_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_inq(conn);

	// 定义表的实体对象
	CModel tpssm03("TPSSM03");
	CModel tpssm01("TPSSM01");

	try
	{
		if (!bcls_ret->Tables[0].Columns.Contains("POUR_TIME"))
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "POUR_TIME");
		}

		bcls_ret->Tables[0].Rows.Add();

		/*置板坯位置代码*/
		for(int i = 0;i < bcls_rec->Tables[0].Rows.get_Count();i++)
		{
			v_factory_div = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"];/*炼钢区分*/
			v_pono = bcls_rec->Tables[0].Rows[i]["PONO"];/*PONO号*/
			v_cc_mach_no = bcls_rec->Tables[0].Rows[i]["CC_MACH_NO"];/*连铸机号*/

			////Log::Trace("", __FUNCTION__, "FACTORY_DIV	= [{0}]", v_factory_div);
			////Log::Trace("", __FUNCTION__, "PONO		= [{0}]", v_pono);
			////Log::Trace("", __FUNCTION__, "CC_MACH_NO		= [{0}]", v_cc_mach_no);

			//查询指定PONO的信息
			sqlstr = "SELECT * "
				" FROM TPSSM01 "
				" WHERE PONO = @PONO "
				" AND FACTORY_DIV = @FACTORY_DIV ";

			/*给查询SQL赋条件值*/
			cmd_tpssm01_inq.SetCommandText(sqlstr);
			cmd_tpssm01_inq.Parameters.Set("PONO", v_pono);
			cmd_tpssm01_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
			////Log::Debug("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm01_inq.ExecuteReader();

			if (cmd_tpssm01_inq.Read())
			{
				cmd_tpssm01_inq.Fetch(tpssm01);
			}
			cmd_tpssm01_inq.Close();

			//查询指定PONO的信息
			sqlstr = "SELECT SLAB_THICK, BILLET_TYPE "
				" FROM TPSSM02 "
				" WHERE CAST_LOT_NO = @CAST_LOT_NO "
				"   AND FACTORY_DIV = @FACTORY_DIV ";

			/*给查询SQL赋条件值*/
			CDbCommand cmd_tpssm02_inq(sqlstr, conn);
			cmd_tpssm02_inq.Parameters.Set("CAST_LOT_NO", tpssm01["CAST_LOT_NO"].ToString());
			cmd_tpssm02_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
			////Log::Debug("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm02_inq.ExecuteReader();

			if (cmd_tpssm02_inq.Read())
			{
				slab_thick = cmd_tpssm02_inq.GetDecimal(1);
				billet_type = cmd_tpssm02_inq.GetString(2);
			}
			cmd_tpssm02_inq.Close();

			//查询指定PONO的信息
			sqlstr = " SELECT SUM(SLAB_LEN * SLAB_NUM), \
					 SUM(SLAB_WIDTH), \
					 MAX(SLAB_THICK), \
					 MAX(SLAB_WIDTH), \
					 MAX(STRAND_NUM), \
					 COUNT(*)	"
				" FROM TPSSM03 "
				" WHERE PONO = @PONO "
				"   AND FACTORY_DIV = @FACTORY_DIV ";

			/*给查询SQL赋条件值*/
			CDbCommand cmd_tpssm03_inq(sqlstr, conn);
			cmd_tpssm03_inq.Parameters.Set("PONO", v_pono);
			cmd_tpssm03_inq.Parameters.Set("FACTORY_DIV", v_factory_div);
			////Log::Debug("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
			cmd_tpssm03_inq.ExecuteReader();

			if (cmd_tpssm03_inq.Read())
			{
				slab_len = cmd_tpssm03_inq.GetDecimal(1);
				slab_width = cmd_tpssm03_inq.GetDecimal(2);
				slab_thick_max = cmd_tpssm03_inq.GetDecimal(3);
				slab_width_max = cmd_tpssm03_inq.GetDecimal(4);
				strand_num = cmd_tpssm03_inq.GetDecimal(5);
				slab_num = cmd_tpssm03_inq.GetDecimal(6);
			}
			cmd_tpssm03_inq.Close();

			if (slab_len <= 0)
			{
				CFormattable arguments[] = { v_pono };
				CMessageFormat::Format(s.msg, "PONO[{0}]下的钢坯总长统计为零！", arguments, 1);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			slab_width = slab_width / slab_num;

			////Log::Trace("", __FUNCTION__, "pour_time2 = [{0}]", pour_time);
			cast_speed_max = 0;
			//第一步查询最大浇铸速度//
			sqlstr = " SELECT MAX(CAST_SPEED) "
				" FROM TQMTS0C "
				" WHERE CC_MACH_NO		= @tpssm01.CC_MACH_NO "
				" AND BILLET_TYPE		= @BILLET_TYPE "
				" AND ST_NO				= @ST_NO "
				" AND CAST_WIDTH_MIN   <= @SLAB_WIDTH "
				" AND CAST_WIDTH_MAX   >= @SLAB_WIDTH "
				" AND CAST_THICK_MIN   <= @SLAB_THICK "
				" AND CAST_THICK_MAX   >= @SLAB_THICK ";

			CDbCommand cmd_inq(sqlstr, conn);
			cmd_inq.Parameters.Set("tpssm01.CC_MACH_NO", tpssm01["CC_MACH_NO"].ToString());
			cmd_inq.Parameters.Set("BILLET_TYPE", billet_type);
			cmd_inq.Parameters.Set("ST_NO", tpssm01["ST_NO"].ToString());
			cmd_inq.Parameters.Set("SLAB_WIDTH", slab_width_max);
			cmd_inq.Parameters.Set("SLAB_THICK", slab_thick_max);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				cast_speed_max = cmd_inq.GetInt32(1);
				////调试用
				//if (cast_speed_max == 0)
				//{
				//	cast_speed_max = 1.1;
				//}
				//else
				//{
				//	cast_speed_max = cast_speed_max / 10 - 0.4;
				//}
				//////Log::Trace("", __FUNCTION__, "cast_speed_max = [{0}]", cast_speed_max);
			}
			cmd_inq.Close();

			if (cast_speed_max == 0)
			{
				//调试用	
				cast_speed_max = 1.1;
				////Log::Trace("", __FUNCTION__, "cast_speed_max = [{0}]", cast_speed_max.ToDouble());
			}

			if (value_md == 0)
			{
				value_md = 7.85;
			}

			if (value_lz == 0)
			{
				value_lz = tpssm01["PLAN_TAP_WT"];
			}

			//计算浇铸时间:单位分钟
			t_element_wk = value_lz * 1000 * 1000 / (slab_len * value_md * cast_speed_max * slab_width * strand_num);
			//计算浇铸时间：时分
			pour_times = t_element_wk.Floor();
			pour_time = pour_times.ToDouble();
			////Log::Trace("", __FUNCTION__, "pour_time = [{0}]", pour_time);


			bcls_ret->Tables[0].Rows[0]["POUR_TIME"] = pour_time;
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)
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

	return(doFlag);
}
