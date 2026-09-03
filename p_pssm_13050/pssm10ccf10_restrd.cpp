/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   魏晨祥
Version:    1.0
Date:     2023年5月11日
Description: 炼钢计划编制（PSSM10）-执行新的浇铸顺重引锭工作
**************************************************/

/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** C++ 的业务头文件部分 *****/



/* ***** 静态函数申明 ***** */
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

//-----------------------------------------------------------------------
//功能描述:		执行新的浇铸顺重引锭工作:需修改此pono重引锭标志(=1)
//数据库表:     TPSSM10
//表中文名:     炼钢连铸制造命令炉次表
//主调用函数:   前台 PSS10画面(制造命令编制)调用
//-----------------------------------------------------------------------
//1.取得pono, 必须是 LOT 中的第1炉
//2.修改TPSSM10表中重引锭
//3.去向不一致不能连浇
//4.厚板向出钢记号不一致不能连浇
//=========================================================================*/

// service入口
BM2F_ENTERACE(pssm10ccf10_restrd)

int f_pssm10ccf10_restrd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 自定义变量 ***** */
	int doFlag = 0;
	int rows = 0;
	int ret = 0;
	int dummy = 0;
	CString date_Now = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString sqlstr = "";
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息
	CString move_type = "";

	CString billet_type = ""; //钢坯类型 [PSA6]
	CString prev_billet_type = ""; //上一炉次的钢坯类型
	CString	v_restrand_flg = "";
	int	v_lack_per = 0;

	CModel tpssm01("TPSSM01");
	CModel tpssm10("TPSSM10");
	CModel tpssm02("TPSSM02");

	EIClass inBlock99; //调用炼钢履历跟踪

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_upd(conn);  //与DB 建立连接。

	try
	{
		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		move_type = bcls_rec->Tables[0].Rows[0]["MOVE_TYPE"].ToString().TrimOrBlank();
		rows = bcls_rec->Tables[0].Rows.get_Count();
		for (int i = 0; i < rows; i++)
		{
			//----------------------------------------------------------------------
			//获取传入参数
			tpssm10["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[i]["FACTORY_DIV"].ToString();
			tpssm10["PONO"] = bcls_rec->Tables[0].Rows[i]["PONO"].ToString();

			if (tpssm10.Query("FACTORY_DIV,PONO"))
			{
				if (16 < tpssm10["PONO_STATUS"].ToDecimal())
				{
					CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "PONO[{0}]的制造命令已排入计划，无法修改", arguments, 1); //格式化字符串
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else
			{
				CFormattable arguments[] = { tpssm10["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "PONO[{0}]不存在，请确认", arguments, 1); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/* ***** 打印输入参数 ***** */
			Log::Info("", __FUNCTION__, "pssm01f10_restrd>FACTORY_DIV = [{0}]", tpssm10["FACTORY_DIV"].ToString());
			Log::Info("", __FUNCTION__, "pssm01f10_restrd>PONO = [{0}]", tpssm10["PONO"].ToString());
			Log::Info("", __FUNCTION__, "pssm01f10_restrd>初始：RESTRAND_FLG = [{0}]", tpssm10["RESTRAND_FLG"].ToString());


			//----------------------------------------------------------------------
			//重引锭标志
			if (strcmp(move_type, "T") == 0)
			{
				v_update = "REC_REVISE_TIME,REC_REVISOR,RESTRAND_FLG";
				if (tpssm10["RESTRAND_FLG"].ToString().Trim() == "")
				{
					//置重引锭标志
					tpssm10["RESTRAND_FLG"] = "T";
				}
				else
				{
					tpssm10["RESTRAND_FLG"] = " ";
				}
			}
			else
			{
				v_update = "REC_REVISE_TIME,REC_REVISOR,TD_CHG_FLG";
				if (tpssm10["TD_CHG_FLG"].ToString().Trim() == "0")
				{
					//置重引锭标志
					tpssm10["TD_CHG_FLG"] = 1;
				}
				else
				{
					tpssm10["TD_CHG_FLG"] = 0;
				}
			}

			//执行更新
			tpssm10["REC_REVISE_TIME"] = date_Now;
			tpssm10["REC_REVISOR"] = s.userid;

			v_condi = "FACTORY_DIV,PONO"; //查询条件

			sqlstr = "tpssm10.Update()";
			if (tpssm10.Update(v_update, v_condi) < 0)
			{
				strcpy(s.msg, "tpssm10 Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//炼钢履历跟踪
			CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
			row99["EVENT_ID"] = "1T"; //重引锭
			row99["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
			row99["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"];
			row99["PONO"] = tpssm10["PONO"];
			row99["PONO_STATUS"] = tpssm10["PONO_STATUS"];
			

			
		}//for end
		if (strcmp(move_type, "T") == 0)
		{
			sqlstr = " UPDATE TPSSM01 A "
				"  SET  RESTRAND_FLG = (SELECT RESTRAND_FLG FROM TPSSM10 B WHERE B.PONO=A.PONO) "
				"  WHERE PONO in(SELECT PONO FROM TPSSM10)";
		}
		else
		{
			sqlstr = " UPDATE TPSSM01 A "
				"  SET  TD_CHG_FLG = (SELECT TD_CHG_FLG FROM TPSSM10 B WHERE B.PONO=A.PONO) "
				"  WHERE PONO in(SELECT PONO FROM TPSSM10)";
		}

		cmd_tpssm01_upd.SetCommandText(sqlstr);
		cmd_tpssm01_upd.ExecuteNonQuery();
		cmd_tpssm01_upd.Close();
		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
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
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
