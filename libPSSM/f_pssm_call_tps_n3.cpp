/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:    1.0
Date:     2015-7-27
Description:	 调用模型。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "CUtils.h"
//程序用头文件

#include "epex.h"

int GenTpsIn_route_create2(CString pono, CString& pono_route, CString& route_relaion, CString& route_div, CDbConnection * conn);
int f_epex_call_rest_tpsmodel_lib(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
/*<remark>=========================================================
/// <summary>
/// 发送计划状态信息至MMS
/// <para>1.读取传入的厂别区分、制造命令号、制造命令状态</para>
/// <para>2.拼接电文后发送MMS。 </para>
/// <para>数据库表：无         </para>
/// <para>主调用函数：由计划编制，计划删除调用。                             </para>
/// </summary>
/// <param name="factory_div">厂别区分          </param>
/// <param name="pono">制造命令号          </param>
/// <param name="pono_status">制造命令状态          </param>
/// <returns>无</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
int f_pssm_call_tps_n3(CString factory_div, int mode, EIClass inblockadd, EIClass & outblockadd, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int  doFlag = 0;
	int ret = 0;

	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CString sqlstr;
	CString v_factory_div = factory_div;
	CString	route_relaion;
	CString	pono_route;
	CString	route_div;
	int blkseq = 0;
	CString station_id = "";
	CString station_no = "";
	CDecimal slab_width = 0, slab_width_pre = 0;
	CDecimal slab_thick = 0, slab_thick_pre = 0;
	CDecimal v_charge_no = 0;
	CString device_status = "";
	CString c_div_pre = "";
	CString st_no_pre = "";
	CDecimal cc_perp_time = 0;
	CString cast_lot_no1 = "";
	CString cast_lot_no2 = "";
	CString in_flag1 = "";
	CString in_flag2 = "";
	CDecimal slab_width1 = 0, slab_width2 = 0;
	CDecimal slab_thick1 = 0, slab_thick2 = 0;
	CString c_div1, c_div2 = "";
	CString st_no1, st_no2 = "";
	CString pono1, pono2 = "";
	CString cc_no = "0";
	CString v_pono = "";

	CString ref_route = "";
	CString backlog_ea = "";
	CString routelist = "";
	CString pono15 = "";
	CString routelist15 = "";
	CString backlogea15 = "";
	CString dev15 = "";
	CString devchoose15 = "";
	CString st_no15 = "";

	//CString dev_code = "";
	/*实体类定义*/
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
	CModel tpssmd6("TPSSMD6");
	CModel tpssmd7("TPSSMD7");
	CModel tpssmd9("TPSSMD9");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm18("TPSSM18");
	CModel tpssm19("TPSSM19");
	CModel tpssmdh("TPSSMDH");
	CModel tpssmdj("TPSSMDJ");
	CModel tpssmdi("TPSSMDI");
	CModel tapbd006s2n("TAPBD006S2N");
	CModel tapbd008s2n("TAPBD008S2N");

	CDataTable tb_tpssm11("TPSSM11");
	CDataTable tb_tpssm12("TPSSM12");
	CDataTable tb_tpssmd7("TPSSMD7");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);
	CDbCommand cmd_inq3(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tapb08_inq(conn);

	EIClass inblock;
	EIClass outblock;

	try
	{
		//---------------------------------------------------
		//设置返回块参数
		//第一块，设备代码
		blkseq = 1;
		inblock.Tables[blkseq - 1].set_TableName("EquipmentInfo"); //tpssmd1
		//inblock.Tables[blkseq - 1].Columns.Add(tpssmd1);
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//设备代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TECH_CODE");//设备类型区分
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_NAME");//设备中文名
		/*暂时没用到inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_DIV");//设备区分标志 传入和设备代码一致？？？*/
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_DIV");//设备分类标志
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_PREP_TIME");//设备准备时间 传入都是0？？？

		//第二块，传搁时间
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("trantime"); //tpssmd6
		//inblock.Tables[blkseq - 1].Columns.Add(tpssmd6); 
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_MOVE_START"); //传搁开始设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TECH_CODE_START");//传搁开始设备类型区分
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "MOVE_TIME");//传搁时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "TRAN_TYPE");//0-交叉 1-直线
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_MOVE_END");//传搁结束设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TECH_CODE_END");//传搁结束设备类型区分

		//第三块，设备定修
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("EquipmentStateInfo");  //tpssm18 tpssm19
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");  //设备代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "STOP_FLAG"); //数据库表里0表示不可用，1表示部分   模型的话0,表示可用，1完全不可用，2部分可用
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "WORK_TIME");  //时长
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME");  //开始时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME");      //结束时间

		//第四块，计划相关
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("PLAN");  //tpssm11 tpssm12
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "BACKLOG_EA");//路径设备区分
		//inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTE_CONTACT");//路径关联关系？？传入为00000
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTE_DEV_TECH_CODE");//设备类型区分
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_REQ_TIME");//CC要求时刻
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_NO");//浇次号
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_DIV_NO");//浇次分割号
		//inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTEBAGKEY");//工艺路径包
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTELIST");//工艺路径
		//inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "WORK_DEV");//正在处理的工位 模型暂时不读这2个字段？？
		//inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "RUN_STATUS");//炉次状态 模型暂时不读这2个字段？？
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//工序设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME");//工序准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "MOVE_TIME");//上工序至本工序传搁时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME");//工序处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME");//计划开始时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME");//计划结束时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "START_TIME_REAL");//实绩开始时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "END_TIME_REAL");//实绩结束时间

		//第五块，双工位交错时间
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("ShareEquipmentInfo");  //tapbd006s2n
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_NAME1");//共享子设备1
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "STATION_NAME2");//共享子设备2
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "TD_TYPE");//类型
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "STAG_TIME");//不可用时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "MOVE_TIME");//移动时间

		//增加第六块 连铸异常处理时间
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("CCM_abnormal_time");  //tapbd008s2n
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别区分
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "AREA_ID");//炼钢区域标识
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "AREA_CNAME");//区域中文
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEVICE_STATUS");//设备 状态
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_STATUS_REMARK");//设备状态描述
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//设备代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TECH_CODE");//设备工艺代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "WORK_TIME");//工作时间

		//增加第七块 钢种-路径包
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("St_no_routebag");  //tpssmdh
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别区分
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");//钢种
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTEBAGKEY");//路径包
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "REMARK");//备注
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "COST_ST_LINE");//成本

		//增加第八块 路径包-路径
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("Routebag_route");  //tpssmdj
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTEBAGKEY");//路径包
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTELIST");//路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PLANTSECTIONTYPE");//默认标记

		//增加第九块 设路径-设备
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("Route_dev");  //tpssmd7
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTELIST");//路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "CHARGE_NO");//路径顺序
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "AREA_ID");//区域代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_TECH_CODE");//设备工艺代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PRE_SOLUTION_FLAG");//预溶液代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FLAG_POS_1");//扒渣标记
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FLAG_POS_2");//分包标记
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FLAG_POS_3");//等待标记
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FLAG_POS_4");//默认标记
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FLAG_POS_5");//默认标记

		//增加第十块 设备处理时间
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("Dev_proc_time");  //tpssmd3
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");//钢种 DEFAULTS为不锈钢 DEFAULTC为碳钢
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//设备号
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "STD_PROC_TIME");//处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "STD_PREP_TIME");//准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "DRAW_TIME");//扒渣时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "WAITING_TIME");//等待时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "SMELT_MODE");//冶炼模式 0-脱碳 2-预溶液

		//增加第十一块 预计划
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("Pre_plan");  //tpssm10
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "FACTORY_DIV");//厂别
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");//钢种
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");//制造命令
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_MACH_NO");//铸机
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO");//预浇次
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "CAST_LOT_DIV_NO");//预浇次内顺序
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_REQ_TIME");//预计开浇时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "C_DIV");//碳锈区分 1-不锈钢 2-碳钢


		//增加第十二块 预浇次间隔时间
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("CAST_LOT_TIME");
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO1");//起点浇次
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "CC_PERP_TIME");//浇次间时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_LOT_NO2");//终点浇次

		//增加第十三块 模型模式
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("MODE");
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "MODE");//浇次间时间
		CDataRow& row_13 = inblock.Tables["MODE"].Rows.Add();
		row_13["MODE"] = mode;

		//增加第十四块 铸机设备倾向
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("CC_TENDENCY");
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_MACH_NO");//铸机号
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//设备倾向

		//给第壹块赋值
		Log::Trace("", __FUNCTION__, "数据块1查询开始");
		sqlstr = "SELECT * FROM TPSSMD1 WHERE FACTORY_DIV=@v_factory_div AND  AREA_ID>=2";
		sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X" || tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				continue;
			}
			CDataRow& row_1 = inblock.Tables["EquipmentInfo"].Rows.Add();
			row_1["DEV_CODE"] = tpssmd1["DEV_CODE"];
			row_1["DEV_TECH_CODE"] = tpssmd1["DEV_TECH_CODE"];
			row_1["STATION_NAME"] = tpssmd1["STATION_NAME"];
			//row_1["DEV_DIV"] = tpssmd1["DEV_CODE"];
			if (tpssmd1["AREA_ID"].ToDecimal() == 1)
			{
				row_1["STATION_DIV"] = "7";
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 3 || tpssmd1["AREA_ID"].ToDecimal() == 2)
			{
				row_1["STATION_DIV"] = "1";
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 4)
			{
				row_1["STATION_DIV"] = "2";
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 5)
			{
				row_1["STATION_DIV"] = "5";
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 6)
			{
				row_1["STATION_DIV"] = "6";
			}
			row_1["DEV_PREP_TIME"] = "0";
		}
		cmd_inq.Close();

		//给第贰块赋值
		Log::Trace("", __FUNCTION__, "数据块2查询开始");
		sqlstr = "SELECT DEV_MOVE_START,DEV_MOVE_END,MOVE_TIME FROM TPSSMD6 WHERE FACTORY_DIV=@v_factory_div";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd6);
			CDataRow& row_2 = inblock.Tables["trantime"].Rows.Add();

			tpssmd1.Reset();
			tpssmd1["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			tpssmd1["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			tpssmd1["FACTORY_DIV"] = v_factory_div;
			tpssmd1.Query("STATION_ID,STATION_NO,FACTORY_DIV");

			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd1["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd1["DEV_TECH_CODE"] = "B";
			}

			row_2["DEV_MOVE_START"] = tpssmd1["DEV_CODE"].ToString();
			row_2["DEV_TECH_CODE_START"] = tpssmd1["DEV_TECH_CODE"].ToString();
			row_2["MOVE_TIME"] = tpssmd6["MOVE_TIME"].ToDecimal();
			row_2["TRAN_TYPE"] = tpssmd6["TRAN_TYPE"].ToString();

			tpssmd1["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			tpssmd1["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);
			tpssmd1.Query("STATION_ID,STATION_NO,FACTORY_DIV");

			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd1["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd1["DEV_TECH_CODE"] = "B";
			}

			row_2["DEV_MOVE_END"] = tpssmd1["DEV_CODE"].ToString();
			row_2["DEV_TECH_CODE_END"] = tpssmd1["DEV_TECH_CODE"].ToString();
		}
		cmd_inq.Close();

		//给第叁块赋值
		Log::Trace("", __FUNCTION__, "数据块3查询开始");
		sqlstr = " SELECT * FROM TPSSM18 WHERE DEV_STATUS = '1' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm18);
			CDataRow& row_3 = inblock.Tables["EquipmentStateInfo"].Rows.Add();

			if (tpssm18["STOP_FLAG"].ToString() == "0")//数据库表里0表示不可用，1表示部分   模型的话0,表示可用，1完全不可用，2部分可用
			{
				tpssm18["STOP_FLAG"] = "1";
			}
			else
			{
				tpssm18["STOP_FLAG"] = "2";
			}

			tpssm19["DEV_STATUS_REMARK"] = tpssm18["DEV_STATUS_REMARK"].ToString().Trim();
			tpssm19["AREA_ID"] = tpssm18["AREA_ID"].ToString().Trim();
			tpssm19["FACTORY_DIV"] = tpssm18["FACTORY_DIV"].ToString().Trim();
			tpssm19["DEV_TECH_CODE"] = tpssm18["DEV_CODE"].ToString().Trim().Substring(0, 1);
			tpssm19.Query("DEV_STATUS_REMARK,AREA_ID,FACTORY_DIV,DEV_TECH_CODE");

			row_3["DEV_CODE"] = tpssm18["DEV_CODE"].ToString();
			row_3["STOP_FLAG"] = tpssm18["STOP_FLAG"].ToString();
			row_3["WORK_TIME"] = tpssm19["WORK_TIME"].ToDecimal();
			row_3["START_TIME"] = tpssm18["START_TIME"].ToString();
			row_3["END_TIME"] = tpssm18["END_TIME"].ToString();

		}
		cmd_inq.Close();

		//给第肆块赋值
		Log::Trace("", __FUNCTION__, "数据块4查询开始");
		sqlstr = "SELECT * FROM TPSSM11 WHERE FACTORY_DIV=@v_factory_div AND PONO_STATUS <= 83 AND STEEL_RETURN_CODE = ' ' ";
		sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(tb_tpssm11);
		cmd_inq.Close();

		for (int index_11 = 0; index_11 < tb_tpssm11.Rows.get_Count(); index_11++)
		{
			tpssm11.MergeFrom(tb_tpssm11.Rows[index_11]);
			GenTpsIn_route_create2(tpssm11["PONO"].ToString(), pono_route, route_relaion, route_div, conn);
			//Log::Trace("", __FUNCTION__, "pono_route = {0} route_div = {1}", pono_route, route_div);
			if (tpssm11["CC_REQ_TIME"].ToString()[0] == ' ')
			{
				tpssm11["CC_REQ_TIME"] = "00000000000000";
			}

			device_status = " ";
			tpssm10.Reset();
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10.Query("PONO");
			slab_width = tpssm10["SLAB_WIDTH"];
			slab_thick = tpssm10["SLAB_THICK"];

			if (tpssm11["RESTRAND_FLG"].ToString() == "T")
			{
				//tpssm10["PONO"] = tpssm11["PONO"];
				//Log::Trace("", __FUNCTION__, "tpssm10.PONO = [{0}]", tpssm10["PONO"].ToString());
				//tpssm10.Query("PONO");
				tpssmd9["CAST_THICK"] = tpssm10["SLAB_THICK"];
				tpssmd9["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssmd9["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
				if (tpssmd9.QueryCount("FACTORY_DIV,CC_MACH_NO,CAST_THICK") != 1)
				{
					tpssmd9["TT_PREP_W0_CAST"] = 70;
				}
				else
				{
					tpssmd9.Query("FACTORY_DIV,CC_MACH_NO,CAST_THICK");
				}

			}
			else
			{
				tpssmd9.Reset();
				tpssmd9["TT_PREP_LAST_2CH"] = 0;
			}

			//校验前后厚宽数据
			if (slab_thick != 0 && slab_width != 0 && slab_thick_pre != 0 && slab_width_pre != 0)
			{
				if (c_div_pre == tpssm10["C_DIV"].ToString() && tpssm10["C_DIV"].ToString() == "1")
				{
					if (slab_thick_pre == 200 && slab_thick == 250)
					{
						if (device_status.Trim() == "") device_status = "8";
						else device_status = device_status + ",8";
					}
				}

				if (c_div_pre == tpssm10["C_DIV"].ToString() && tpssm10["C_DIV"].ToString() == "2")
				{
					if (slab_thick_pre == 230 && slab_thick == 280)
					{
						if (device_status.Trim() == "") device_status = "6";
						else device_status = device_status + ",6";
					}
					if (slab_thick_pre == 280 && slab_thick == 230)
					{
						if (device_status.Trim() == "") device_status = "7";
						else device_status = device_status + ",7";
					}
				}

				if (device_status.Trim() == "" && st_no_pre != tpssm10["ST_NO"].ToString())
				{
					if (device_status.Trim() == "") device_status = "10";
					else device_status = device_status + ",10";
				}

				if (device_status.Trim() == "" && slab_width != slab_width_pre)
				{
					if (device_status.Trim() == "") device_status = "9";
					else device_status = device_status + ",9";
				}

			}

			sqlstr = "SELECT * FROM TPSSM12 WHERE FACTORY_DIV=@v_factory_div ";
			sqlstr += CString(" AND SM_PLAN_NO=@tpssm11.SM_PLAN_NO AND AREA_ID >1 ORDER BY CHARGE_NO ASC");
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_tpssm12_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.ExecuteQuery(tb_tpssm12);
			cmd_tpssm12_inq.Close();

			//循环查询各工序
			for (int index_12 = 0; index_12<tb_tpssm12.Rows.get_Count(); index_12++)
			{
				tpssm12.MergeFrom(tb_tpssm12.Rows[index_12]);

				if (index_12 == 0)
				{
					CDataRow& row_44 = inblock.Tables["PLAN"].Rows.Add();
					row_44["PONO"] = tpssm11["PONO"].ToString();
					row_44["BACKLOG_EA"] = pono_route;
					row_44["ROUTE_DEV_TECH_CODE"] = route_div;
					row_44["CC_REQ_TIME"] = tpssm11["CC_REQ_TIME"].ToString();
					row_44["CAST_NO"] = tpssm11["CAST_NO"].ToString();
					row_44["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"].ToDecimal();
					row_44["ROUTELIST"] = tpssm11["ROUTELIST"].ToString();

					row_44["DEV_CODE"] = "00";
					row_44["PREP_TIME"] = 0;
					row_44["MOVE_TIME"] = 0;
					row_44["PROC_TIME"] = 0;
					row_44["START_TIME"] = "00000000000000";
					row_44["END_TIME"] = "00000000000000";
					row_44["START_TIME_REAL"] = "00000000000000";
					row_44["END_TIME_REAL"] = "00000000000000";
				}

				CDataRow& row_4 = inblock.Tables["PLAN"].Rows.Add();
				row_4["PONO"] = tpssm11["PONO"].ToString();
				row_4["BACKLOG_EA"] = pono_route;
				row_4["ROUTE_DEV_TECH_CODE"] = route_div;
				row_4["CC_REQ_TIME"] = tpssm11["CC_REQ_TIME"].ToString();
				row_4["CAST_NO"] = tpssm11["CAST_NO"].ToString();
				row_4["CAST_DIV_NO"] = tpssm11["CAST_DIV_NO"].ToDecimal();
				row_4["ROUTELIST"] = tpssm11["ROUTELIST"].ToString();

				row_4["DEV_CODE"] = tpssm12["DEV_CODE"].ToString();
				//v_charge_no = tpssm12["CHARGE_NO"];

				//---- 得到移行时间  --------------
				//查到达侧设备代码
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");

				tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
				tpssmd6["DEV_MOVE_START"] = station_id + station_no;
				tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

				if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
				{
					tpssmd6["MOVE_TIME"] = 0;
				}
				else
				{
					tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
				}
				//---- 统计该 charge_no 下的"准备时间", "处理时间" --------------
				////Log::Trace("", __FUNCTION__,"tpssm12.area_id = [{0}]",tpssm12["AREA_ID"].ToDecimal().ToInt32());
				if (tpssm11["RESTRAND_FLG"].ToString() == "T" && tpssm12["AREA_ID"].ToString().Trim() == "5")
				{
					if (device_status.Trim() == "") device_status = "3";
					else device_status = device_status + ",3";

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm12["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
					cmd_tapb08_inq.Close();

					row_4["PREP_TIME"] = cc_perp_time;
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else if (tpssm12["AREA_ID"].ToString().Trim() == "5" && tpssm11["RESTRAND_FLG"].ToString() != "T" && tpssm11["TD_CHG_FLG"].ToString() == "1")
				{
					if (device_status.Trim() == "") device_status = "2";
					else device_status = device_status + ",2";

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm12["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
					cmd_tapb08_inq.Close();

					row_4["PREP_TIME"] = cc_perp_time;
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}

				else if (tpssm12["AREA_ID"].ToString().Trim() == "5" && tpssm11["RESTRAND_FLG"].ToString() != "T" && tpssm11["TD_CHG_FLG"].ToString() == "0")
				{
					if (device_status.Trim() == "") device_status = "1";
					else device_status = device_status + ",1";

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm12["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
					cmd_tapb08_inq.Close();

					row_4["PREP_TIME"] = cc_perp_time;
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else
				{
					row_4["PREP_TIME"] = tpssm12["PREP_TIME"].ToDecimal();
					//fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", tpssm12["PREP_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				}

				row_4["PROC_TIME"] = tpssm12["PROC_TIME"].ToDecimal();
				//fprintf(outstream, "%d\t\t\t\t;工序%d处理时间\n", tpssm12["PROC_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				//---- 写入移行时间  --------------
				row_4["MOVE_TIME"] = tpssmd6["MOVE_TIME"].ToDecimal();

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
				//---- 读取开始时刻/结束时刻 --------------
				//判断实绩表中的PONO是否存在，新编制的计划在实绩表中是不存在的
				if (tpssm12["START_TIME_REAL"].ToString().Trim() == "") tpssm12["START_TIME_REAL"] = "00000000000000";
				if (tpssm12["START_TIME"].ToString().Trim() == "") tpssm12["START_TIME"] = "00000000000000";
				//实绩结束时刻
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "") tpssm12["END_TIME_REAL"] = "00000000000000";
				if (tpssm12["END_TIME"].ToString().Trim() == "") tpssm12["END_TIME"] = "00000000000000";

				row_4["START_TIME"] = tpssm12["START_TIME"].ToString();
				row_4["START_TIME_REAL"] = tpssm12["START_TIME_REAL"].ToString();
				row_4["END_TIME"] = tpssm12["END_TIME"].ToString();
				row_4["END_TIME_REAL"] = tpssm12["END_TIME_REAL"].ToString();
			}

			slab_width_pre = slab_width;
			slab_thick_pre = slab_thick;
			c_div_pre = tpssm10["C_DIV"];
			st_no_pre = tpssm10["ST_NO"];
		}

		int rows = inblock.Tables["PLAN"].Rows.get_Count();
		for (int i = 0; i < inblockadd.Tables["PLAN"].Rows.get_Count(); i++)
		{
			inblock.Tables["PLAN"].Rows.Add();
			inblock.Tables["PLAN"].Rows[rows + i].Merge(inblockadd.Tables["PLAN"].Rows[i]);
		}

		PrintDataTable(inblock.Tables["PLAN"]);
		//给第伍块赋值
		Log::Trace("", __FUNCTION__, "数据块5查询开始");
		sqlstr = " SELECT * FROM TAPBD006S2N ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tapbd006s2n);
			CDataRow& row_5 = inblock.Tables["ShareEquipmentInfo"].Rows.Add();

			row_5["STATION_NAME1"] = tapbd006s2n["STATION_NAME"].ToString();
			row_5["STATION_NAME2"] = tapbd006s2n["STATION_NAME_2"].ToString();
			row_5["TD_TYPE"] = tapbd006s2n["TD_TYPE"].ToString();
			row_5["STAG_TIME"] = tapbd006s2n["STAG_TIME"].ToDecimal();
			row_5["MOVE_TIME"] = tapbd006s2n["MOVE_TIME"].ToDecimal();
		}
		cmd_inq.Close();

		//给第陆块赋值
		Log::Trace("", __FUNCTION__, "数据块6查询开始");
		sqlstr = " SELECT * FROM TAPBD008S2N ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tapbd008s2n);
			CDataRow& row_6 = inblock.Tables["CCM_abnormal_time"].Rows.Add();

			row_6["FACTORY_DIV"] = tapbd008s2n["FACTORY_DIV"].ToString();
			row_6["AREA_ID"] = tapbd008s2n["AREA_ID"].ToDecimal();
			row_6["AREA_CNAME"] = tapbd008s2n["AREA_CNAME"].ToString();
			row_6["DEVICE_STATUS"] = tapbd008s2n["DEVICE_STATUS"].ToString();
			row_6["DEV_STATUS_REMARK"] = tapbd008s2n["DEV_STATUS_REMARK"].ToString();
			row_6["DEV_CODE"] = tapbd008s2n["DEV_CODE"].ToString();
			row_6["DEV_TECH_CODE"] = tapbd008s2n["DEV_TECH_CODE"].ToString();
			row_6["WORK_TIME"] = tapbd008s2n["WORK_TIME"].ToDecimal();
		}
		cmd_inq.Close();

		//给第柒块赋值
		Log::Trace("", __FUNCTION__, "数据块7查询开始");
		sqlstr = " SELECT * FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm10) ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdh);
			CDataRow& row_7 = inblock.Tables["St_no_routebag"].Rows.Add();

			row_7["FACTORY_DIV"] = tpssmdh["FACTORY_DIV"].ToString();
			row_7["ST_NO"] = tpssmdh["ST_NO"].ToString();
			row_7["ROUTEBAGKEY"] = tpssmdh["ROUTEBAGKEY"].ToString();
			row_7["REMARK"] = tpssmdh["REMARK"].ToString();
			row_7["COST_ST_LINE"] = tpssmdh["COST_ST_LINE"].ToDecimal();
		}
		cmd_inq.Close();

		//给第捌块赋值
		Log::Trace("", __FUNCTION__, "数据块8查询开始");
		sqlstr = " SELECT * FROM TPSSMDJ ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdj);
			CDataRow& row_8 = inblock.Tables["Routebag_route"].Rows.Add();

			row_8["ROUTEBAGKEY"] = tpssmdj["ROUTEBAGKEY"].ToString();
			row_8["ROUTELIST"] = tpssmdj["ROUTELIST"].ToString();
			row_8["PLANTSECTIONTYPE"] = tpssmdj["PLANTSECTIONTYPE"].ToString();
		}
		cmd_inq.Close();

		//给第玖块赋值
		Log::Trace("", __FUNCTION__, "数据块9查询开始");
		sqlstr = " SELECT * FROM TPSSMD7 ";
		sqlstr += CString(" ORDER BY ROUTELIST,CHARGE_NO ");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd7);
			CDataRow& row_9 = inblock.Tables["Route_dev"].Rows.Add();

			if (tpssmd7["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd7["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd7["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd7["DEV_TECH_CODE"] = "B";
			}

			row_9["FACTORY_DIV"] = tpssmd7["FACTORY_DIV"].ToString();
			row_9["ROUTELIST"] = tpssmd7["ROUTELIST"].ToString();
			row_9["CHARGE_NO"] = tpssmd7["CHARGE_NO"].ToDecimal();
			row_9["AREA_ID"] = tpssmd7["AREA_ID"].ToDecimal();
			row_9["DEV_TECH_CODE"] = tpssmd7["DEV_TECH_CODE"].ToString();
			row_9["PRE_SOLUTION_FLAG"] = tpssmd7["PRE_SOLUTION_FLAG"].ToString();
			row_9["FLAG_POS_1"] = tpssmd7["FLAG_POS_1"].ToString();
			row_9["FLAG_POS_2"] = tpssmd7["FLAG_POS_2"].ToString();
			row_9["FLAG_POS_3"] = tpssmd7["FLAG_POS_3"].ToString();
			row_9["FLAG_POS_4"] = tpssmd7["FLAG_POS_4"].ToString();
			row_9["FLAG_POS_5"] = tpssmd7["FLAG_POS_5"].ToString();
		}
		cmd_inq.Close();

		//给第拾块赋值
		Log::Trace("", __FUNCTION__, "数据块10查询开始");
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句
			sqlstr = CString(
				" SELECT DISTINCT(ST_NO) FROM TPSSM10 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND PONO_STATUS < 83 "
				);
			break;
		}
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm10_inq.ExecuteReader();
		while (cmd_tpssm10_inq.Read())
		{
			tpssm10["ST_NO"] = cmd_tpssm10_inq.GetString(1);
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				sqlstr = CString(
					" SELECT DISTINCT d3.ST_NO, d1.DEV_TECH_CODE, d3.STD_PROC_TIME, d1.AREA_ID, d3.SMELT_MODE, d3.FACTORY_DIV, d3.STD_PREP_TIME, d3.DRAW_TIME, d3.WAITING_TIME "
					"   FROM TPSSMD3 d3,  TPSSMD1 d1 "
					"  WHERE d3.ST_NO         = @st_no "
					"    AND d3.DEV_CODE = d1.DEV_TECH_CODE  "
					"   AND d3.factory_div = d1.factory_div "
					"   AND d1.factory_div = @tpssm10.factory_div"
					);
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tpssm10["ST_NO"].ToString());
			cmd_inq.Parameters.Set("tpssm10.factory_div", v_factory_div);
			cmd_inq.ExecuteReader();
			//Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
			while (cmd_inq.Read())
			{
				tpssmd3["ST_NO"] = cmd_inq.GetString(1);
				tpssmd1["DEV_TECH_CODE"] = cmd_inq.GetString(2);
				tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(3);
				tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(4);
				tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(5);
				tpssmd3["FACTORY_DIV"] = cmd_inq.GetString(6);
				tpssmd3["STD_PREP_TIME"] = cmd_inq.GetDecimal(7);
				tpssmd3["DRAW_TIME"] = cmd_inq.GetDecimal(8);
				tpssmd3["WAITING_TIME"] = cmd_inq.GetDecimal(9);

				sqlstr = CString(
					" SELECT DISTINCT DEV_CODE FROM TPSSMD1 WHERE DEV_TECH_CODE = @DEV_TECH_CODE AND AREA_ID = @AREA_ID AND FACTORY_DIV = @FACTORY_DIV "
					);
				cmd_inq2.SetCommandText(sqlstr);
				cmd_inq2.Parameters.Set("DEV_TECH_CODE", tpssmd1["DEV_TECH_CODE"]);
				cmd_inq2.Parameters.Set("AREA_ID", tpssmd1["AREA_ID"]);
				cmd_inq2.Parameters.Set("FACTORY_DIV", v_factory_div);
				cmd_inq2.ExecuteReader();
				while (cmd_inq2.Read())
				{
					tpssmd1["DEV_CODE"] = cmd_inq2.GetString(1);
					CDataRow& row_10 = inblock.Tables["Dev_proc_time"].Rows.Add();
					row_10["FACTORY_DIV"] = tpssmd3["FACTORY_DIV"].ToString();
					row_10["ST_NO"] = tpssmd3["ST_NO"].ToString();
					row_10["DEV_CODE"] = tpssmd1["DEV_CODE"].ToString();
					row_10["STD_PROC_TIME"] = tpssmd3["STD_PROC_TIME"].ToDecimal();
					row_10["STD_PREP_TIME"] = tpssmd3["STD_PREP_TIME"].ToDecimal();
					row_10["DRAW_TIME"] = tpssmd3["DRAW_TIME"].ToDecimal();
					row_10["WAITING_TIME"] = tpssmd3["WAITING_TIME"].ToDecimal();
					row_10["SMELT_MODE"] = tpssmd3["SMELT_MODE"].ToDecimal();//0-常规 2-预溶液
				}
				cmd_inq2.Close();
			}
			cmd_inq.Close();
		}
		cmd_tpssm10_inq.Close();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:

			sqlstr = " SELECT T.ST_NO,A.DEV_CODE,T.STD_PROC_TIME ,A.AREA_ID, T.SMELT_MODE, T.FACTORY_DIV, T.STD_PREP_TIME, T.DRAW_TIME, T.WAITING_TIME "
				" FROM TPSSMD3 T ,TPSSMD1 A "
				" WHERE T.DEV_CODE = A.DEV_CODE "
				"   AND T.factory_div = A.factory_div "
				"   AND T.factory_div = @tpssm10.factory_div"
				"   AND T.st_no in ('DEFAULTC','DEFAULTS')"
				;


			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm10.factory_div", v_factory_div);

		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tpssmd3["ST_NO"] = cmd_inq.GetString(1);
			tpssmd1["DEV_CODE"] = cmd_inq.GetString(2);
			tpssmd3["STD_PROC_TIME"] = cmd_inq.GetDecimal(3);
			tpssmd1["AREA_ID"] = cmd_inq.GetDecimal(4);
			tpssmd3["SMELT_MODE"] = cmd_inq.GetDecimal(5);
			tpssmd3["FACTORY_DIV"] = cmd_inq.GetString(6);
			tpssmd3["STD_PREP_TIME"] = cmd_inq.GetDecimal(7);
			tpssmd3["DRAW_TIME"] = cmd_inq.GetDecimal(8);
			tpssmd3["WAITING_TIME"] = cmd_inq.GetDecimal(9);


			if ((tpssmd1["AREA_ID"].ToDecimal() == 3 && tpssmd3["SMELT_MODE"].ToDecimal() == 2) || (tpssmd1["AREA_ID"].ToDecimal() == 2 && tpssmd3["SMELT_MODE"].ToDecimal() == 0))
			{
				continue;
			}
			CDataRow& row_10 = inblock.Tables["Dev_proc_time"].Rows.Add();
			row_10["FACTORY_DIV"] = tpssmd3["FACTORY_DIV"].ToString();
			row_10["ST_NO"] = tpssmd3["ST_NO"].ToString();
			row_10["DEV_CODE"] = tpssmd1["DEV_CODE"].ToString();
			row_10["STD_PROC_TIME"] = tpssmd3["STD_PROC_TIME"].ToDecimal();
			row_10["STD_PREP_TIME"] = tpssmd3["STD_PREP_TIME"].ToDecimal();
			row_10["DRAW_TIME"] = tpssmd3["DRAW_TIME"].ToDecimal();
			row_10["WAITING_TIME"] = tpssmd3["WAITING_TIME"].ToDecimal();
			row_10["SMELT_MODE"] = tpssmd3["SMELT_MODE"].ToDecimal();//0-常规 2-预溶液
		}
		cmd_inq.Close();

		//给第拾壹块赋值
		Log::Trace("", __FUNCTION__, "数据块11查询开始");
		sqlstr = " SELECT * FROM TPSSM10 ORDER BY CAST_LOT_NO,CAST_LOT_DIV_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm10);
			CDataRow& row_11 = inblock.Tables["Pre_plan"].Rows.Add();

			row_11["FACTORY_DIV"] = tpssm10["FACTORY_DIV"].ToString();
			row_11["ST_NO"] = tpssm10["ST_NO"].ToString();
			row_11["PONO"] = tpssm10["PONO"].ToString();
			row_11["CC_MACH_NO"] = tpssm10["CC_MACH_NO"].ToString();
			row_11["CAST_LOT_NO"] = tpssm10["CAST_LOT_NO"].ToString();
			row_11["CAST_LOT_DIV_NO"] = tpssm10["CAST_LOT_DIV_NO"].ToDecimal();
			row_11["CC_REQ_TIME"] = tpssm10["CC_REQ_TIME"].ToString();
			row_11["C_DIV"] = tpssm10["C_DIV"].ToString();
		}
		cmd_inq.Close();

		//给第拾贰块赋值
		Log::Trace("", __FUNCTION__, "数据块12查询开始");
		sqlstr = " SELECT distinct CAST_LOT_NO FROM tpssm10 order by CAST_LOT_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cast_lot_no1 = cmd_inq.GetString(1);

			sqlstr = " SELECT SLAB_WIDTH,SLAB_THICK,C_DIV,ST_NO,PONO FROM tpssm10 where CAST_LOT_NO = @CAST_LOT_NO  order by CAST_LOT_DIV_NO desc ";
			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("CAST_LOT_NO", cast_lot_no1);
			cmd_tpssm10_inq.ExecuteReader();
			if (cmd_tpssm10_inq.Read())
			{
				slab_width1 = cmd_tpssm10_inq.GetDecimal(1);
				slab_thick1 = cmd_tpssm10_inq.GetDecimal(2);
				c_div1 = cmd_tpssm10_inq.GetString(3);
				st_no1 = cmd_tpssm10_inq.GetString(4);
				pono1 = cmd_tpssm10_inq.GetString(5);
			}

			sqlstr = " SELECT distinct CAST_LOT_NO FROM tpssm10 order by CAST_LOT_NO ";
			cmd_inq2.SetCommandText(sqlstr);
			cmd_inq2.ExecuteReader();
			while (cmd_inq2.Read())
			{
				cast_lot_no2 = cmd_inq2.GetString(1);
				if (cast_lot_no1 == cast_lot_no2) continue;

				//fetchRowCount++;

				sqlstr = " SELECT SLAB_WIDTH,SLAB_THICK,C_DIV,ST_NO,PONO,CC_MACH_NO FROM tpssm10 where CAST_LOT_NO = @CAST_LOT_NO  order by CAST_LOT_DIV_NO asc ";
				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("CAST_LOT_NO", cast_lot_no2);
				cmd_tpssm10_inq.ExecuteReader();
				if (cmd_tpssm10_inq.Read())
				{
					slab_width2 = cmd_tpssm10_inq.GetDecimal(1);
					slab_thick2 = cmd_tpssm10_inq.GetDecimal(2);
					c_div2 = cmd_tpssm10_inq.GetString(3);
					st_no2 = cmd_tpssm10_inq.GetString(4);
					pono2 = cmd_tpssm10_inq.GetString(5);
					cc_no = cmd_tpssm10_inq.GetString(6);
				}

				device_status = " ";
				//校验前后厚宽数据
				if (slab_thick1 != 0 && slab_width1 != 0 && slab_thick2 != 0 && slab_width2 != 0)
				{
					if (c_div1 == c_div2 && c_div2 == "1")
					{
						if (slab_thick1 == 200 && slab_thick2 == 250)
						{
							if (device_status.Trim() == "") device_status = "8";
							else device_status = device_status + ",8";
						}
					}

					if (c_div1 == c_div2 && c_div2 == "2")
					{
						if (slab_thick1 == 230 && slab_thick2 == 280)
						{
							if (device_status.Trim() == "") device_status = "6";
							else device_status = device_status + ",6";
						}
						if (slab_thick1 == 280 && slab_thick2 == 230)
						{
							if (device_status.Trim() == "") device_status = "7";
							else device_status = device_status + ",7";
						}
					}

					if (device_status.Trim() == "" && st_no1 != st_no2)
					{
						if (device_status.Trim() == "") device_status = "10";
						else device_status = device_status + ",10";
					}

					if (device_status.Trim() == "" && slab_width1 != slab_width2)
					{
						if (device_status.Trim() == "") device_status = "9";
						else device_status = device_status + ",9";
					}

				}
				/*tpssm11_1["PONO"] = pono1;
				tpssm11_2["PONO"] = pono2;
				if (tpssm11_1.QueryCount("PONO") == 1 && tpssm11_2.QueryCount("PONO") == 1)
				{
				tpssm11_1.Reset();
				tpssm11_2.Reset();
				tpssm11_1.Query("PONO");
				tpssm11_2.Query("PONO");
				if (tpssm11_2["RESTRAND_FLG"].ToString() != "T" && tpssm11_2["TD_CHG_FLG"].ToString() == "1")
				{
				if (device_status.Trim() == "") device_status = "2";
				else device_status = device_status + ",2";
				}
				else if (tpssm11_2["RESTRAND_FLG"].ToString() == "T")
				{
				if (device_status.Trim() == "") device_status = "3";
				else device_status = device_status + ",3";
				}
				else
				{
				if (device_status.Trim() == "") device_status = "1";
				else device_status = device_status + ",1";
				}
				}
				else
				{*/
				if (device_status.Trim() == "") device_status = "3";
				else device_status = device_status + ",3";
				//}

				sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID = 5 AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
				cmd_tapb08_inq.SetCommandText(sqlstr);
				cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
				cmd_tapb08_inq.Parameters.Set("DEV_CODE", "C" + cc_no);
				cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
				cmd_tapb08_inq.Close();

				CDataRow& row_12 = inblock.Tables["CAST_LOT_TIME"].Rows.Add();
				row_12["CAST_LOT_NO1"] = cast_lot_no1;
				row_12["CC_PERP_TIME"] = cc_perp_time;
				row_12["CAST_LOT_NO2"] = cast_lot_no2;

				//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}],[{2}],[{3}]", cc_perp_time.ToInt32(), device_status, cast_lot_no1, cast_lot_no2);

			}
			cmd_inq2.Close();

			cmd_tpssm10_inq.Close();
		}
		cmd_inq.Close();

		//给第拾肆块赋值
		Log::Trace("", __FUNCTION__, "数据块14查询开始");
		sqlstr = " SELECT * FROM tpssmdi ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdi);
			CDataRow& row_14 = inblock.Tables["CC_TENDENCY"].Rows.Add();

			row_14["CC_MACH_NO"] = tpssmdi["CC_MACH_NO"].ToString();
			row_14["DEV_CODE"] = tpssmdi["DEV_CODE"].ToString();
		}
		cmd_inq.Close();


		//给第拾伍块赋值
		Log::Trace("", __FUNCTION__, "数据块15查询开始");
		sqlstr = " SELECT "
			" a.PONO, "
			" a.ST_NO, "
			" b.ROUTEBAGKEY, "
			" c.ROUTELIST, "
			" d.COST_ST_LINE, "
			" b.C_DIV, "
			" a.CC_MACH_NO "
			" FROM "
			" TPSSM11 a, "
			" TPSSM10 b, "
			" TPSSMDJ c, "
			" TPSSMDK d "
			" WHERE "
			" a.PONO = b.PONO "
			" AND b.ROUTEBAGKEY = c.ROUTEBAGKEY "
			" AND c.ROUTELIST = d.ROUTELIST "
			" ORDER BY a.PONO, c.ROUTELIST ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		CDecimal dev_count = 0;
		CString dev_code = "DEV_CODE";
		CString prep_time = "PREP_TIME";
		CString proc_time = "PROC_TIME";
		while (cmd_inq.Read())
		{
			CDataRow& row_15 = inblock.Tables["DEV_ARRANGE"].Rows.Add();
			row_15["PROC_TIME10"] = 0;
			row_15["PREP_TIME10"] = 0;
			row_15["DEV_CODE10"] = " ";
			row_15["PROC_TIME9"] = 0;
			row_15["PREP_TIME9"] = 0;
			row_15["DEV_CODE9"] = " ";
			row_15["PROC_TIME8"] = 0;
			row_15["PREP_TIME8"] = 0;
			row_15["DEV_CODE8"] = " ";
			row_15["PROC_TIME7"] = 0;
			row_15["PREP_TIME7"] = 0;
			row_15["DEV_CODE7"] = " ";
			row_15["PROC_TIME6"] = 0;
			row_15["PREP_TIME6"] = 0;
			row_15["DEV_CODE6"] = " ";
			row_15["PROC_TIME5"] = 0;
			row_15["PREP_TIME5"] = 0;
			row_15["DEV_CODE5"] = " ";
			row_15["PROC_TIME4"] = 0;
			row_15["PREP_TIME4"] = 0;
			row_15["DEV_CODE4"] = " ";
			row_15["PROC_TIME3"] = 0;
			row_15["PREP_TIME3"] = 0;
			row_15["DEV_CODE3"] = " ";
			row_15["PROC_TIME2"] = 0;
			row_15["PREP_TIME2"] = 0;
			row_15["DEV_CODE2"] = " ";
			row_15["PROC_TIME1"] = 0;
			row_15["PREP_TIME1"] = 0;
			row_15["DEV_CODE1"] = " ";

			pono15 = cmd_inq.GetString(1);
			st_no15 = cmd_inq.GetString(2);
			routelist15 = cmd_inq.GetString(4);
			backlogea15 = " ";

			row_15["PONO"] = pono15;
			row_15["ROUTELIST"] = routelist15;
			row_15["COST_ST_LINE"] = cmd_inq.GetDecimal(5);

			tpssm11["PONO"] = pono15;
			tpssm11["ROUTELIST"] = routelist15;

			dev_count = 1;
			//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm11["PONO"].ToString(), routelist15);
			if (tpssm11.QueryCount("PONO,ROUTELIST") == 0)//备用路径
			{
				row_15["CHOOSE_LIST"] = "0";
				tpssm11.Query("PONO");
				sqlstr = "SELECT * FROM TPSSMD7 WHERE FACTORY_DIV=@v_factory_div AND ROUTELIST = @ROUTELIST ";
				sqlstr += CString(" ORDER BY CHARGE_NO ASC");
				cmd_inq2.SetCommandText(sqlstr);
				cmd_inq2.Parameters.Set("v_factory_div", v_factory_div);
				cmd_inq2.Parameters.Set("ROUTELIST", routelist15);
				cmd_inq2.ExecuteQuery(tb_tpssmd7);
				cmd_inq2.Close();

				for (int index_7 = 0; index_7 < tb_tpssmd7.Rows.get_Count(); index_7++)
				{
					devchoose15 = " ";
					tpssmd7.MergeFrom(tb_tpssmd7.Rows[index_7]);
					dev15 = tpssmd7["DEV_TECH_CODE"].ToString();
					tpssmd3["ST_NO"] = cmd_inq.GetString(2);
					tpssmd3["SMELT_MODE"] = "0";
					tpssmd3["FACTORY_DIV"] = v_factory_div;
					tpssmd3["DEV_CODE"] = dev15;
					tpssmd3["SMELT_MODE2"] = " ";
					if (dev15.Trim() == "X")
					{
						dev15 = "E";
						tpssmd3["DEV_CODE"] = "E";
						tpssmd3["SMELT_MODE"] = "2";
					}
					if (dev15.Trim() == "Y")
					{
						dev15 = "B";
						tpssmd3["DEV_CODE"] = "B";
						tpssmd3["SMELT_MODE"] = "2";
					}

					if (dev15.Trim() != "C")//非连铸设备取静态数据配置
					{
						if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV") == 0)
						{
							if (cmd_inq.GetString(7) == "1")
							{
								tpssmd3["ST_NO"] = "DEFAULTS";
							}
							if (cmd_inq.GetString(7) == "2")
							{
								tpssmd3["ST_NO"] = "DEFAULTC";
							}
						}
						tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
						//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal());
						row_15[proc_time + dev_count.ToString()] = tpssmd3["STD_PROC_TIME"];
						row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
					}
					else//连铸设备取计划数据
					{
						tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
						tpssm12["AREA_ID"] = 5;
						tpssm12.Query("SM_PLAN_NO,AREA_ID");
						row_15[proc_time + dev_count.ToString()] = tpssm12["PROC_TIME"];
						row_15[prep_time + dev_count.ToString()] = tpssm12["PREP_TIME"];
					}

					//可行设备
					if (dev15.Trim() != "C")
					{
						tpssmdi["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
						tpssmdi["DEV_TECH_CODE"] = dev15;
						if (tpssmdi.QueryCount("CC_MACH_NO,DEV_TECH_CODE") == 0)
						{
							//row_15[dev_code + dev_count.ToString()] = " ";
						}
						else
						{
							sqlstr = "SELECT DEV_CODE FROM TPSSMDI WHERE CC_MACH_NO=@CC_MACH_NO AND DEV_TECH_CODE = @DEV_TECH_CODE ";
							cmd_inq3.SetCommandText(sqlstr);
							cmd_inq3.Parameters.Set("CC_MACH_NO", tpssmdi["CC_MACH_NO"].ToString());
							cmd_inq3.Parameters.Set("DEV_TECH_CODE", tpssmdi["DEV_TECH_CODE"].ToString());
							cmd_inq3.ExecuteReader();
							while (cmd_inq3.Read())
							{
								if (devchoose15.Trim() == "")
								{
									devchoose15 = cmd_inq3.GetString(1);
								}
								else devchoose15 = devchoose15 + "," + cmd_inq3.GetString(1);
							}
							cmd_inq3.Close();
						}
					}
					row_15[dev_code + dev_count.ToString()] = devchoose15;


					if (backlogea15.Trim() == "")
					{
						backlogea15 = dev15;
					}
					else backlogea15 = backlogea15 + "-" + dev15;
					dev_count = dev_count + 1;
				}
			}
			else//当前使用路径
			{
				row_15["CHOOSE_LIST"] = "1";
				tpssm11.Query("PONO");
				sqlstr = "SELECT * FROM TPSSM12 WHERE FACTORY_DIV=@v_factory_div AND SM_PLAN_NO = @SM_PLAN_NO ";
				sqlstr += CString(" ORDER BY CHARGE_NO ASC");
				cmd_inq2.SetCommandText(sqlstr);
				cmd_inq2.Parameters.Set("v_factory_div", v_factory_div);
				cmd_inq2.Parameters.Set("SM_PLAN_NO", tpssm11["SM_PLAN_NO"]);
				cmd_inq2.ExecuteQuery(tb_tpssm12);
				cmd_inq2.Close();

				for (int index_12 = 0; index_12 < tb_tpssm12.Rows.get_Count(); index_12++)
				{
					devchoose15 = " ";
					tpssm12.MergeFrom(tb_tpssm12.Rows[index_12]);
					row_15[proc_time + dev_count.ToString()] = tpssm12["PROC_TIME"];
					row_15[prep_time + dev_count.ToString()] = tpssm12["PREP_TIME"];

					tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
					tpssmd1["FACTORY_DIV"] = tpssm12["FACTORY_DIV"];
					tpssmd1.Query("FACTORY_DIV,AREA_ID,DEV_CODE");
					dev15 = tpssmd1["DEV_TECH_CODE"];

					if (dev15.Trim() == "X")
					{
						dev15 = "E";
					}
					if (dev15.Trim() == "Y")
					{
						dev15 = "B";
					}
					//可行设备
					if (dev15.Trim() != "C")
					{
						tpssmdi["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
						tpssmdi["DEV_TECH_CODE"] = dev15;
						if (tpssmdi.QueryCount("CC_MACH_NO,DEV_TECH_CODE") == 0)
						{
							//row_15[dev_code + dev_count.ToString()] = " ";
						}
						else
						{
							sqlstr = "SELECT DEV_CODE FROM TPSSMDI WHERE CC_MACH_NO=@CC_MACH_NO AND DEV_TECH_CODE = @DEV_TECH_CODE ";
							cmd_inq3.SetCommandText(sqlstr);
							cmd_inq3.Parameters.Set("CC_MACH_NO", tpssmdi["CC_MACH_NO"].ToString());
							cmd_inq3.Parameters.Set("DEV_TECH_CODE", tpssmdi["DEV_TECH_CODE"].ToString());
							cmd_inq3.ExecuteReader();
							while (cmd_inq3.Read())
							{
								if (devchoose15.Trim() == "")
								{
									devchoose15 = cmd_inq3.GetString(1);
								}
								else devchoose15 = devchoose15 + "," + cmd_inq3.GetString(1);
							}
							cmd_inq3.Close();
						}
					}
					row_15[dev_code + dev_count.ToString()] = devchoose15;

					if (backlogea15.Trim() == "")
					{
						backlogea15 = dev15;
					}
					else backlogea15 = backlogea15 + "-" + dev15;
					dev_count = dev_count + 1;
				}
			}

			row_15["BACKLOG_EA"] = backlogea15;
		}
		cmd_inq.Close();


		ret = f_epex_call_rest_tpsmodel_lib(&inblock, &outblock, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//PrintDataTable(outblock.Tables[0]);
		if (outblock.Tables.Contains("INFO"))
		{
			for (int i = outblock.Tables["INFO"].Rows.get_Count() - 1; i >= 0; i--)
			{
				v_pono = outblock.Tables["INFO"].Rows[i]["PONO"].ToString().Trim();
				tpssm11["PONO"] = v_pono;
				if (tpssm11.QueryCount("PONO") == 1 || outblock.Tables["INFO"].Rows[i]["CHARGE_NO"].ToDecimal() == 0)
				{
					outblock.Tables["INFO"].Rows.Remove(i);
				}
			}
			//PrintDataTable(outblock.Tables[0]);
			outblockadd.Tables[0].Copy(outblock.Tables["INFO"]);
		}

		//更新计划
		//tpssm11.Reset();
		//for (int index = 0; index < outblock.Tables[0].Rows.get_Count(); index++)
		//{
		//	if (tpssm11["PONO"].ToString().Trim() != outblock.Tables[0].Rows[index]["PONO"].ToString())
		//	{
		//		tpssm11["PONO"] = outblock.Tables[0].Rows[index]["PONO"].ToString();
		//		tpssm11.Query("PONO");

		//		ref_route = "";
		//		backlog_ea = "";
		//	}

		//	tpssmd1["DEV_CODE"] = outblock.Tables[0].Rows[index]["DEV_CODE"].ToString();
		//	tpssmd1["FACTORY_DIV"] = "LG1";

		//	if (tpssmd1.QueryCount("DEV_CODE,FACTORY_DIV") == 1)
		//	{
		//		tpssmd1.Query("DEV_CODE,FACTORY_DIV");
		//	}
		//	else
		//	{
		//		tpssmd1["AREA_ID"] = 3;
		//		tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
		//	}

		//	if (tpssmd1["DEV_CODE"].ToString() == "00")
		//	{

		//	}
		//	else
		//	{
		//		Log::Trace("", __FUNCTION__, "tpssmd1.dev_code=[{0}]", (const char *)tpssmd1["DEV_CODE"].ToString());
		//		Log::Trace("", __FUNCTION__, "tpssmd1.DEV_TECH_CODE=[{0}]", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
		//		Log::Trace("", __FUNCTION__, "tpssmd1.AREA_ID=[{0}]", (const char *)tpssmd1["AREA_ID"].ToString());

		//		backlog_ea = backlog_ea + tpssmd1["DEV_CODE"].ToString().Substring(0, 1);
		//		if (tpssmd1["AREA_ID"].ToDecimal() == 4)
		//		{
		//			ref_route = ref_route + tpssmd1["DEV_CODE"].ToString();
		//		}
		//	}

		//	if (outblock.Tables[0].Rows[index]["CHARGE_NO"].ToDecimal()<tpssm11["CURR_WP_NO"].ToDecimal())
		//	{
		//		continue;
		//	}
		//	tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
		//	tpssm12["CHARGE_NO"] = outblock.Tables[0].Rows[index]["CHARGE_NO"].ToDecimal();
		//	tpssm12["REC_REVISE_TIME"] = dateNow;
		//	tpssm12["REC_REVISOR"] = s.userid;
		//	tpssm12["PRE_PROC_NO"] = " ";
		//	tpssm12["PROC_NO"] = " ";
		//	tpssm12["DEV_CODE"] = outblock.Tables[0].Rows[index]["DEV_CODE"].ToString();
		//	tpssm12["START_TIME"] = outblock.Tables[0].Rows[index]["START_TIME"].ToString();
		//	tpssm12["END_TIME"] = outblock.Tables[0].Rows[index]["END_TIME"].ToString();

		//	if (outblock.Tables[0].Rows[index]["CHARGE_NO"].ToDecimal() == tpssm11["CURR_WP_NO"].ToDecimal())
		//	{
		//		tpssm12.Update("START_TIME,END_TIME", "SM_PLAN_NO,CHARGE_NO");
		//		continue;
		//	}
		//	//tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PRE_PROC_NO,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");
		//	//Log::Trace("", __FUNCTION__, "tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());
		//	tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");
		//	//Log::Trace("", __FUNCTION__, "111tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());

		//	tpssm11["BACKLOG_EA"] = backlog_ea.TrimOrBlank();
		//	tpssm11["REFINE_ROUTE_CODE"] = ref_route.TrimOrBlank();

		//	//修改工序主表的内容
		//	tpssm11.Update("REC_REVISOR,REC_REVISE_TIME,BACKLOG_EA,REFINE_ROUTE_CODE", "PONO");
		//}

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

int GenTpsIn_route_create2(CString pono, CString& pono_route, CString& route_relaion, CString& route_div, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int k = 0;//各相关路径的下标索引
	int doFlag = 0;
	int  i, j, n;
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr = "";
	CDataTable dev_info("DEV_INFO");
	try
	{
		//设置铁水预处理的相关路径 default
		//k=0;
		pono_route = "00";
		route_relaion = "0";
		route_div = "0";

		sqlstr = "SELECT T1.PONO,T2.AREA_ID,T2.DEV_CODE,T2.CHARGE_NO,T3.DEV_TECH_CODE FROM TPSSM11 T1, TPSSM12 T2,TPSSMD1 T3 ";
		sqlstr += "WHERE T1.PONO=@pono AND T1.SM_PLAN_NO = T2.SM_PLAN_NO AND T2.DEV_CODE=T3.DEV_CODE AND T2.AREA_ID=T3.AREA_ID AND T2.FACTORY_DIV=T3.FACTORY_DIV ORDER BY T2.CHARGE_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("pono", pono);
		cmd_inq.ExecuteQuery(dev_info);
		for (i = 0; i<dev_info.Rows.get_Count(); i++)
		{
			//将找到的 dev_code 转换为模型识别的代码	
			if (dev_info.Rows[i]["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				dev_info.Rows[i]["DEV_TECH_CODE"] = "E";
			}
			if (dev_info.Rows[i]["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				dev_info.Rows[i]["DEV_TECH_CODE"] = "B";
			}

			pono_route = pono_route + dev_info.Rows[i]["DEV_CODE"].ToString();
			route_relaion = route_relaion + "0";
			route_div = route_div + dev_info.Rows[i]["DEV_TECH_CODE"].ToString();
		}
		cmd_inq.Close();
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
	cmd_inq.Close();
	return doFlag;
}
