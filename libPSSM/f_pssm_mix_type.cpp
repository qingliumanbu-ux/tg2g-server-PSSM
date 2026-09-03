/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023-6-28
*  程序名称			: f_pssm_mix_type  新混浇小分类计算模型
*  程序描述			: 根据成分判断混浇分类(计划--大包上台之前计算出来)(实际--炉次确定的时候调用计算出来)
*  备注说明			: 1、中间包第一炉且为浇次第一炉代码为“A”。
2、中间包第一炉且不是为浇次第一炉，按照以下条件判断：（建立静态表1）
1）、碳C相差0.08%；
2）、锰Mn相差0.5%；
3）、铜Cu相差0.15%
4）、钛当量PTi相差0.15%（Pti为元素Ti+V+Ni+Nb+Cr+Mo
5）、磷P相差0.03%
6）、非IF钢与IF钢换包连浇（2个出钢记号在连铸标准中钢组为小于2和大于2的关系）
***上面其中一个条件超出将混浇代码置为“C”；
***上面其中任何一个条件也不超出将混浇代码置为“B”；
3、不是中间包第一炉。
*** 炼钢L3连铸作业计划中下一炉混浇标志为0(原画面中为空白)、1、2、4时，混浇代码不变。
4、下一炉混浇标志为3。
4.1 比较前后炉次化学成分对含铜与非含铜（铜Cu相差0.2%）、非IF钢与IF钢连浇（2个出钢记号在连铸标准中钢组为小于2和大于2的关系） 以及碳C相差0.08%、锰Mn相差0.6%，建立静态表2。
***满足上面其中一个条件将混浇代码置为“7”；
4.2比较前后炉次化学成分对碳C相差0.04%、锰Mn相差0.3%；钛当量PTi相差0.04%（Pti为元素Ti+V+Ni）、碳当量相差0.08％，建立静态表3。
***如果上面其中一个条件超出将混浇代码置为“6”；
***如果上面其中如何一个条件未超出并且C、Mn、PTi未超出上一炉标准范围将混浇代码置为“8”；
***如果上面其中如何一个条件未超出并且C、Mn、PTi之一超出上一炉标准范围将混浇代码置为“5”；

heat_confm_flag 炉次确定标记：0-计划或成分变更，修改计划混浇分类
1-炉次确定，修改实际混浇分类
                 
Update: 
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 调宽标记
/// <para>数据库表： </para>
/// <para>主调用函数: f_pssm_mix_type </para>
/// </summary>
/// <param name="i=0">前一炉       </param>
/// <param name="i=1">当前炉       </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm_mix_type(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount;
	int i;
	int rows = 0;
	CString	create_time;            /* 记录创建时刻 */
	CString intermix_flag_pono;
	CString v_pono, v_pono_pre, heat_confm_flag;
	
	CModel tpssm10("TPSSM10");
	CDbCommand cmd_inq(conn);
	CString sqlstr, sqlstr_10, sqlstr_40;

	try
	{
		create_time=CDateTime::Now().ToString("yyyyMMddHHmmss");

		i = 1;
		//获取传入参数
		v_pono = bcls_rec->Tables[0].Rows[i]["PONO"].ToString().TrimOrBlank();         //当前炉pono
		v_pono_pre = bcls_rec->Tables[0].Rows[i-1]["PONO"].ToString().TrimOrBlank(); //上一炉pono
		heat_confm_flag = bcls_rec->Tables[0].Rows[i - 1]["HEAT_CONFM_FLAG"].ToString().TrimOrBlank(); //炉次确定标记

		//检查传入参数
		if (strcmp(v_pono, " ") == 0)
		{
			sprintf(s.msg, "传入参数v_pono[%s]为空。", (const char*)v_pono);
			Log::Info("", __FUNCTION__, "传入参数v_pono[{0}]为空", v_pono);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sqlstr_10 = CString(
			" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, nvl(E.ST_NO_TO,A.ST_NO) CAN_MIX_GROUP,nvl(F.IDX_NO,A.ST_NO) WORRY_MIX_GROUP "
			" ,G.ARCHIVE_FLAG,G.HARDNESS_GROUP,G.HARD_GROUP "
			" from TPSSM11 A "
			" LEFT JOIN TQMTS13 E ON E.ST_NO_FROM = A.ST_NO "
			" LEFT JOIN TQMTS14 F ON F.IDX_NO_01 = E.ST_NO_TO "
			" LEFT JOIN TQMTS0X G ON G.ST_NO = A.PONO "
			" where PONO = @PONO "
			);
		sqlstr_40 = CString(
			" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, nvl(E.ST_NO_TO,A.ST_NO) CAN_MIX_GROUP,nvl(F.IDX_NO,A.ST_NO) WORRY_MIX_GROUP "
			" ,G.ARCHIVE_FLAG,G.HARDNESS_GROUP,G.HARD_GROUP "
			" from TPSSM41 A "
			" LEFT JOIN TQMTS13 E ON E.ST_NO_FROM = A.ST_NO "
			" LEFT JOIN TQMTS14 F ON F.IDX_NO_01 = E.ST_NO_TO "
			" LEFT JOIN TQMTS0X G ON G.ST_NO = A.PONO "
			" where PONO = @PONO "
			);
		bcls_ret->Tables.Add();

		cmd_inq.SetCommandText(sqlstr_10);
		cmd_inq.Parameters.Set("PONO", v_pono);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
		cmd_inq.Close();
		if (bcls_ret->Tables[1].Rows.get_Count() <= 0)
		{
			//炉次已确定
			heat_confm_flag = "1";

			cmd_inq.SetCommandText(sqlstr_40);
			cmd_inq.Parameters.Set("PONO", v_pono);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
			cmd_inq.Close();
			if (bcls_ret->Tables[1].Rows.get_Count() <= 0)
			{
				sprintf(s.msg, "传入参数v_pono[%s]计划不存在。", (const char*)v_pono);
				Log::Info("", __FUNCTION__, "传入参数v_pono[{0}]计划不存在", v_pono);
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		if (bcls_ret->Tables[1].Rows[0]["CAST_DIV_NO"].ToDecimal() <= 1)
		{
			//浇次第一炉
			tpssm10["INTERMIX_TYPE"] = "A";
			tpssm10["INTERMIX_FLAG_PONO"] = "3";
			tpssm10["INTERMIX_TYPE_ROUTE"] = "A10";
		}
		else{
			//查找下一炉信息
			sqlstr_10 = CString(
				" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, nvl(E.ST_NO_TO,A.ST_NO) CAN_MIX_GROUP,nvl(F.IDX_NO,A.ST_NO) WORRY_MIX_GROUP "
				" ,G.ARCHIVE_FLAG,G.HARDNESS_GROUP,G.HARD_GROUP "
				" from TPSSM11 A "
				" LEFT JOIN TQMTS13 E ON E.ST_NO_FROM = A.ST_NO "
				" LEFT JOIN TQMTS14 F ON F.IDX_NO_01 = E.ST_NO_TO "
				" LEFT JOIN TQMTS0X G ON G.ST_NO = A.PONO "
				" where CAST_NO = @CAST_NO and CAST_DIV_NO = @CAST_DIV_NO "
				);
			sqlstr_40 = CString(
				" select CC_MACH_NO,CC_SEQ,PONO_STATUS,A.ST_NO, A.PONO,A.RESTRAND_FLG,A.TD_CHG_FLG, '0' ORDER_CC_SEQ_FLAG, nvl(E.ST_NO_TO,A.ST_NO) CAN_MIX_GROUP,nvl(F.IDX_NO,A.ST_NO) WORRY_MIX_GROUP "
				" ,G.ARCHIVE_FLAG,G.HARDNESS_GROUP,G.HARD_GROUP "
				" from TPSSM41 A "
				" LEFT JOIN TQMTS13 E ON E.ST_NO_FROM = A.ST_NO "
				" LEFT JOIN TQMTS14 F ON F.IDX_NO_01 = E.ST_NO_TO "
				" LEFT JOIN TQMTS0X G ON G.ST_NO = A.PONO "
				" where CAST_NO = @CAST_NO and CAST_DIV_NO = @CAST_DIV_NO "
				);

			cmd_inq.SetCommandText(sqlstr_10);
			cmd_inq.Parameters.Set("CAST_NO", bcls_ret->Tables[1].Rows[0]["CAST_NO"].ToString());
			cmd_inq.Parameters.Set("CAST_DIV_NO", bcls_ret->Tables[1].Rows[0]["CAST_DIV_NO"].ToDecimal()-1);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
			if (bcls_ret->Tables[0].Rows.get_Count() <= 0)
			{
				cmd_inq.SetCommandText(sqlstr_40);
				cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
				cmd_inq.Close();
				if (bcls_ret->Tables[0].Rows.get_Count() <= 0)
				{
					sprintf(s.msg, "CAST_NO[%s] CAST_DIV_NO[%d]查找上一炉信息没找到，当作浇次第一炉处理。", bcls_ret->Tables[1].Rows[0]["CAST_NO"].ToString(), bcls_ret->Tables[1].Rows[0]["CAST_DIV_NO"].ToDecimal().ToInt32());
					Log::Info("", __FUNCTION__, "%s", s.msg);
					tpssm10["INTERMIX_TYPE"] = "A";
					tpssm10["INTERMIX_FLAG_PONO"] = "3";
					tpssm10["INTERMIX_TYPE_ROUTE"] = "A10";
				}
			}
		}

		/* 机号计算 */
		if (bcls_ret->Tables[0].Rows.get_Count() > 0
			&& strcmp(bcls_ret->Tables[1].Rows[0]["CC_MACH_NO"].ToString(), bcls_ret->Tables[0].Rows[0]["CC_MACH_NO"].ToString()) != 0)
		{
			sprintf(s.msg, "传入参数pono_pre[%s],pono[%s]连铸记号不同。", (const char*)v_pono_pre, (const char*)v_pono);
			Log::Info("", __FUNCTION__, "%s", s.msg);
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//--------------查询上炉信息end--------------------
		else if (strcmp(bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString(), bcls_rec->Tables[0].Rows[i - 1]["ST_NO"].ToString()) == 0)
		{
			tpssm10["INTERMIX_TYPE"] = "0";
			tpssm10["INTERMIX_FLAG_PONO"] = "0";
			tpssm10["INTERMIX_TYPE_ROUTE"] = "000";
		}else
		{
			//IF钢判断,steel_class_IF为0则是IF钢(wcx20130619 修改为目标碳大于50ppm和小于50ppm关系 注:50ppm = 0.0050%)

		}
		

		tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"];

		tpssm10.Update("INTERMIX_TYPE_ROUTE", "PONO");

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

