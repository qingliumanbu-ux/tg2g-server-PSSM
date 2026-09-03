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
#include <map>
//程序用头文件

#include "epex.h"

int GenTpsIn_route_create(CString pono, CString& pono_route, CString& route_relaion, CString& route_div, CDbConnection * conn);
int f_epex_call_rest_tpsmodel_lib(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm21_cast_cre_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//甘特图炉次生成CAST号

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
int f_pssm_call_tps_n2(CString factory_div, int mode, EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	CString ref_route = "";
	CString backlog_ea = "";
	CString routelist = "";
	CString pono15 = "";
	CString routelist15 = "";
	CString backlogea15 = "";
	CString dev15 = "";
	CString devchoose15 = "";
	CString st_no15 = "";
	CDecimal aod_proc_time = 0;
	CDecimal aod_prep_time = 0;

	CString st_no = " ";

	CString CC_CHOOSE_S = "C0,C1,C2"; //不锈钢连铸机选择
	CString CC_CHOOSE_C = "C3,C4"; //碳钢连铸机选择
	//CString dev_code = "";
	/*实体类定义*/
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd3("TPSSMD3");
	CModel tpssmd6("TPSSMD6");
	CModel tpssmd7("TPSSMD7");
	CModel tpssmd9("TPSSMD9");
	CModel tpssm10("TPSSM10");
	CModel tpssm17("TPSSM17");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssm18("TPSSM18");
	CModel tpssm19("TPSSM19");
	CModel tpssmdh("TPSSMDH");
	CModel tpssmdj("TPSSMDJ");
	CModel tpssmdi("TPSSMDI");
	CModel tqmts0x("TQMTS0X");
	CModel tapbd006s2n("TAPBD006S2N");
	CModel tapbd008s2n("TAPBD008S2N");
	CModel tpssmdm("TPSSMDM");

	struct RhythmInfo
	{
		CDecimal lower;
		CDecimal upper;
		CDecimal timeOffset;
	};
	std::map<CString, RhythmInfo> rhythmCache;

	CDataTable tb_tpssm11("TPSSM11");
	CDataTable tb_tpssm12("TPSSM12");
	CDataTable tb_tpssmd7("TPSSMD7");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);
	CDbCommand cmd_inq3(conn);
	CDbCommand cmd_inq4(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tpssm12_inq(conn);
	CDbCommand cmd_tapb08_inq(conn);

	EIClass inblock;
	EIClass outblock;
	EIClass inblk;
	EIClass inblk_out;

	inblk.Tables[0].set_TableName("PLAN");  //
	inblk.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");  //炼钢单元号
	inblk.Tables[0].Columns.Add(DT_STRING, "SPECIAL_FLAG");
	inblk.Tables[0].Rows.Add();

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
		inblock.Tables[blkseq - 1].set_TableName("PLAN");  //tpssm11 tpssm12  日平衡
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

		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "RHYTHM_LOWER");
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "RHYTHM_UPPER");
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


		//增加第十二块 预浇次间隔时间(TPSSM15)
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
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "MODE");
		CDataRow& row_13 = inblock.Tables["MODE"].Rows.Add();
		row_13["MODE"] = mode;

		//增加第十四块 铸机设备倾向
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("CC_TENDENCY");
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CC_MACH_NO");//铸机号
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//设备倾向

		//增加第十五块 设备路径推荐
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("DEV_ARRANGE");//日平衡
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");//制造命令号
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ROUTELIST");//路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "BACKLOG_EA");//路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CHOOSE_LIST");//当前路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "COST_ST_LINE");//成本
		//////////20240221 add
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "AOD_ROUTEFLAG");//A0D前路径代码
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "AOD_ROUTELIST");//AOD前路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CHOOSE_LIST2");//当前路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "COST_HJ");//A0D前成本
		//////////
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PROC_TIME1");//AOD前工序1处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PREP_TIME1");//AOD前工序1准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PRE_DEV_CODE1");//AOD前工序1可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PROC_TIME2");//AOD前工序2处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PREP_TIME2");//AOD前工序2准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PRE_DEV_CODE2");//AOD前工序2可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PROC_TIME3");//AOD前工序3处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PREP_TIME3");//AOD前工序3准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PRE_DEV_CODE3");//AOD前工序3可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PROC_TIME4");//AOD前工序4处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PRE_PREP_TIME4");//AOD前工序4准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PRE_DEV_CODE4");//AOD前工序4可行设备
		//////////
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME1");//工序1处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME1");//工序1准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE1");//工序1可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME2");//工序2处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME2");//工序2准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE2");//工序2可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME3");//工序3处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME3");//工序3准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE3");//工序3可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME4");//工序4处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME4");//工序4准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE4");//工序4可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME5");//工序5处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME5");//工序5准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE5");//工序5可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME6");//工序6处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME6");//工序6准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE6");//工序6可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME7");//工序7处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME7");//工序7准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE7");//工序7可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME8");//工序8处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME8");//工序8准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE8");//工序8可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME9");//工序9处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME9");//工序9准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE9");//工序9可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME10");//工序10处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME10");//工序10准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE10");//工序10可行设备

		//增加第十六块 预溶液设备路径推荐
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("PRE_SOLUTION_ARRANGE");//tpssmd3 日平衡
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "SM_PLAN_NO");//计划号
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");//制造命令
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "ST_NO");//钢种
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//AOD设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "SMELT_MODE2");//预溶液路径模式
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PRE_ROUTE");//预溶液路径
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "AOD_BACKLOG_EA");//预溶液路径拆分
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "STD_PROC_TIME");//该路径下AOD处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "AOD_CHOOSE_LIST");//AOD当前路径
		//inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "STD_PREP_TIME");//该路径下AOD间隔时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "COST_HJ");//该路径成本
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME1");//工序1处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME1");//工序1准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE1");//工序1可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME2");//工序2处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME2");//工序2准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE2");//工序2可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME3");//工序3处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME3");//工序3准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE3");//工序3可行设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PROC_TIME4");//工序4处理时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_DECIMAL, "PREP_TIME4");//工序4准备时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE4");//工序4可行设备

		//增加第十七块 设备最后时间
		blkseq++;
		inblock.Tables.Add();
		inblock.Tables[blkseq - 1].set_TableName("DEV_LAST_TIME");//tpssm11 tpssm12
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "SM_PLAN_NO");//计划号
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "PONO");//制造命令
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "DEV_CODE");//设备
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "TIME");//时间
		inblock.Tables[blkseq - 1].Columns.Add(DT_STRING, "CAST_NO");//浇次号

		inblk.Tables["PLAN"].Rows[0]["FACTORY_DIV"] = v_factory_div;
		inblk.Tables["PLAN"].Rows[0]["SPECIAL_FLAG"] = "1";

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
		//PrintDataTable(inblock.Tables["EquipmentInfo"]);
		//给第贰块赋值
		Log::Trace("", __FUNCTION__, "数据块2查询开始");
		sqlstr = "SELECT DEV_MOVE_START,DEV_MOVE_END,MOVE_TIME,TRAN_TYPE FROM TPSSMD6 WHERE FACTORY_DIV=@v_factory_div";
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

		//给第肆块赋值，修改，有实绩的浇次都不排
		Log::Trace("", __FUNCTION__, "数据块4查询开始");
		sqlstr = " SELECT *  "
			" FROM TPSSM15 "
			" WHERE CAST_NO IN( "
			" SELECT CAST_NO "
			" FROM TPSSM15 A "
			" WHERE A.FACTORY_DIV = @v_factory_div "
			" AND A.STEEL_RETURN_CODE = ' ' "
			" AND A.CAST_DIV_NO = 1 "
			" AND A.PONO_STATUS < 20 "
			" AND NOT EXISTS( "
			" SELECT 1 "
			" FROM TPSSM15 B "
			" WHERE B.CAST_NO = A.CAST_NO "
			" AND B.PONO_STATUS >= 20 "
			" ) "
			" ) ";//
		//sqlstr = "SELECT * FROM TPSSM15 WHERE FACTORY_DIV=@v_factory_div AND PONO_STATUS < 83 AND STEEL_RETURN_CODE = ' ' ";
		sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(tb_tpssm11);
		cmd_inq.Close();


		// 加载节奏数据
		Log::Trace("", __FUNCTION__, "TPSSMDM rhythm config load start");
		sqlstr = "SELECT ST_NO, LOWER_LIMIT_VALUE, UPPER_LIMIT_VALUE, TIME_OFFSET FROM TPSSMDM";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			CString stNo = cmd_inq.GetString(1);
			RhythmInfo info;
			info.lower = cmd_inq.GetDecimal(2);
			info.upper = cmd_inq.GetDecimal(3);
			info.timeOffset = cmd_inq.GetDecimal(4);
			// 保留第一次匹配
			if (rhythmCache.find(stNo) == rhythmCache.end())
			{
				rhythmCache[stNo] = info;
			}
		}
		cmd_inq.Close();
		for (int index_11 = 0; index_11 < tb_tpssm11.Rows.get_Count(); index_11++)
		{
			tpssm15.MergeFrom(tb_tpssm11.Rows[index_11]);
			GenTpsIn_route_create(tpssm15["PONO"].ToString(), pono_route, route_relaion, route_div, conn);
			//Log::Trace("", __FUNCTION__, "pono_route = {0} route_div = {1}", pono_route, route_div);
			if (tpssm15["CC_REQ_TIME"].ToString()[0] == ' ')
			{
				tpssm15["CC_REQ_TIME"] = "00000000000000";
			}

			device_status = " ";
			tpssm17.Reset();
			tpssm17["PONO"] = tpssm15["PONO"];
			tpssm17.Query("PONO");
			slab_width = tpssm17["SLAB_WIDTH"];
			slab_thick = tpssm17["SLAB_THICK"];

			if (tpssm15["RESTRAND_FLG"].ToString() == "T")
			{
				//tpssm17["PONO"] = tpssm15["PONO"];
				//Log::Trace("", __FUNCTION__, "tpssm17.PONO = [{0}]", tpssm17["PONO"].ToString());
				//tpssm17.Query("PONO");
				tpssmd9["CAST_THICK"] = tpssm17["SLAB_THICK"];
				tpssmd9["FACTORY_DIV"] = tpssm17["FACTORY_DIV"];
				tpssmd9["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
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
				if (c_div_pre == tpssm17["C_DIV"].ToString() && tpssm17["C_DIV"].ToString() == "1")
				{
					if (slab_thick_pre == 200 && slab_thick == 250)
					{
						if (device_status.Trim() == "") device_status = "8";
						else device_status = device_status + ",8";
					}
				}

				if (c_div_pre == tpssm17["C_DIV"].ToString() && tpssm17["C_DIV"].ToString() == "2")
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

				if (device_status.Trim() == "" && st_no_pre != tpssm17["ST_NO"].ToString())
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

			sqlstr = "SELECT * FROM TPSSM16 WHERE FACTORY_DIV=@v_factory_div ";
			sqlstr += CString(" AND SM_PLAN_NO=@tpssm15.SM_PLAN_NO AND AREA_ID >1 ORDER BY CHARGE_NO ASC");
			cmd_tpssm12_inq.SetCommandText(sqlstr);
			cmd_tpssm12_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_tpssm12_inq.Parameters.Set("tpssm15.SM_PLAN_NO", tpssm15["SM_PLAN_NO"].ToString());
			cmd_tpssm12_inq.ExecuteQuery(tb_tpssm12);
			cmd_tpssm12_inq.Close();

			//循环查询各工序
			for (int index_12 = 0; index_12<tb_tpssm12.Rows.get_Count(); index_12++)
			{
				tpssm16.MergeFrom(tb_tpssm12.Rows[index_12]);

				if (index_12 == 0)
				{
					CDataRow& row_44 = inblock.Tables["PLAN"].Rows.Add();
					row_44["PONO"] = tpssm15["PONO"].ToString();
					row_44["BACKLOG_EA"] = pono_route;
					row_44["ROUTE_DEV_TECH_CODE"] = route_div;
					row_44["CC_REQ_TIME"] = tpssm15["CC_REQ_TIME"].ToString();
					row_44["CAST_NO"] = tpssm15["CAST_NO"].ToString();
					row_44["CAST_DIV_NO"] = tpssm15["CAST_DIV_NO"].ToDecimal();
					row_44["ROUTELIST"] = tpssm15["ROUTELIST"].ToString();

					row_44["DEV_CODE"] = "00";
					row_44["PREP_TIME"] = 0;
					row_44["MOVE_TIME"] = 0;
					row_44["PROC_TIME"] = 0;
					row_44["START_TIME"] = "00000000000000";
					row_44["END_TIME"] = "00000000000000";
					row_44["START_TIME_REAL"] = "00000000000000";
					row_44["END_TIME_REAL"] = "00000000000000";

					CString st_no_key = tpssm15["ST_NO"].ToString().Trim();
					if (!st_no_key.IsEmpty() && rhythmCache.find(st_no_key) != rhythmCache.end())
					{
						const RhythmInfo& info = rhythmCache[st_no_key];
						if (tpssm15["CAST_DIV_NO"].ToDecimal() == 1)
							row_44["RHYTHM_LOWER"] = info.timeOffset;
						else
							row_44["RHYTHM_LOWER"] = info.lower;
						row_44["RHYTHM_UPPER"] = info.upper;
					}
					else
					{
						row_44["RHYTHM_LOWER"] = 0;
						row_44["RHYTHM_UPPER"] = 0;
					}
				}

				

				CDataRow& row_4 = inblock.Tables["PLAN"].Rows.Add();
				row_4["PONO"] = tpssm15["PONO"].ToString();
				row_4["BACKLOG_EA"] = pono_route;
				row_4["ROUTE_DEV_TECH_CODE"] = route_div;
				row_4["CC_REQ_TIME"] = tpssm15["CC_REQ_TIME"].ToString();
				row_4["CAST_NO"] = tpssm15["CAST_NO"].ToString();
				row_4["CAST_DIV_NO"] = tpssm15["CAST_DIV_NO"].ToDecimal();
				row_4["ROUTELIST"] = tpssm15["ROUTELIST"].ToString();

				row_4["DEV_CODE"] = tpssm16["DEV_CODE"].ToString();
				//v_charge_no = tpssm16["CHARGE_NO"];

				//---- 得到移行时间  --------------
				//查到达侧设备代码
				tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = tpssm16["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm16["AREA_ID"];
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
				////Log::Trace("", __FUNCTION__,"tpssm16.area_id = [{0}]",tpssm16["AREA_ID"].ToDecimal().ToInt32());
				if (tpssm15["RESTRAND_FLG"].ToString() == "T" && tpssm16["AREA_ID"].ToString().Trim() == "5")
				{
					if (device_status.Trim() == "") device_status = "3";
					else device_status = device_status + ",3";

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm16["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm16["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
					cmd_tapb08_inq.Close();

					row_4["PREP_TIME"] = cc_perp_time;
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else if (tpssm16["AREA_ID"].ToString().Trim() == "5" && tpssm15["RESTRAND_FLG"].ToString() != "T" && tpssm15["TD_CHG_FLG"].ToString() == "1")
				{
					if (device_status.Trim() == "") device_status = "2";
					else device_status = device_status + ",2";

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm16["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm16["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
					cmd_tapb08_inq.Close();

					row_4["PREP_TIME"] = cc_perp_time;
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}

				else if (tpssm16["AREA_ID"].ToString().Trim() == "5" && tpssm15["RESTRAND_FLG"].ToString() != "T" && tpssm15["TD_CHG_FLG"].ToString() == "0")
				{
					if (device_status.Trim() == "") device_status = "1";
					else device_status = device_status + ",1";

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status + ")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm16["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm16["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();
					cmd_tapb08_inq.Close();

					row_4["PREP_TIME"] = cc_perp_time;
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else
				{
					row_4["PREP_TIME"] = tpssm16["PREP_TIME"].ToDecimal();
					//fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", tpssm16["PREP_TIME"].ToDecimal().ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				}

				row_4["PROC_TIME"] = tpssm16["PROC_TIME"].ToDecimal();
				//fprintf(outstream, "%d\t\t\t\t;工序%d处理时间\n", tpssm16["PROC_TIME"].ToDecimal().ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				//---- 写入移行时间  --------------
				row_4["MOVE_TIME"] = tpssmd6["MOVE_TIME"].ToDecimal();

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
				//---- 读取开始时刻/结束时刻 --------------
				//判断实绩表中的PONO是否存在，新编制的计划在实绩表中是不存在的
				if (tpssm16["START_TIME_REAL"].ToString().Trim() == "") tpssm16["START_TIME_REAL"] = "00000000000000";
				if (tpssm16["START_TIME"].ToString().Trim() == "") tpssm16["START_TIME"] = "00000000000000";
				//实绩结束时刻
				if (tpssm16["END_TIME_REAL"].ToString().Trim() == "") tpssm16["END_TIME_REAL"] = "00000000000000";
				if (tpssm16["END_TIME"].ToString().Trim() == "") tpssm16["END_TIME"] = "00000000000000";

				row_4["START_TIME"] = tpssm16["START_TIME"].ToString();
				row_4["START_TIME_REAL"] = tpssm16["START_TIME_REAL"].ToString();
				row_4["END_TIME"] = tpssm16["END_TIME"].ToString();
				row_4["END_TIME_REAL"] = tpssm16["END_TIME_REAL"].ToString();

				CString st_no_key_4 = tpssm15["ST_NO"].ToString().Trim();
				if (!st_no_key_4.IsEmpty() && rhythmCache.find(st_no_key_4) != rhythmCache.end())
				{
					const RhythmInfo& info_4 = rhythmCache[st_no_key_4];
					if (tpssm15["CAST_DIV_NO"].ToDecimal() == 1)
						row_4["RHYTHM_LOWER"] = info_4.timeOffset;
					else
						row_4["RHYTHM_LOWER"] = info_4.lower;
					row_4["RHYTHM_UPPER"] = info_4.upper;
				}
				else
				{
					row_4["RHYTHM_LOWER"] = 0;
					row_4["RHYTHM_UPPER"] = 0;
				}
			}

			

			slab_width_pre = slab_width;
			slab_thick_pre = slab_thick;
			c_div_pre = tpssm17["C_DIV"];
			st_no_pre = tpssm17["ST_NO"];
		}
		//PrintDataTable(inblock.Tables["PLAN"]);
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
		sqlstr = " SELECT * FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm17 WHERE PONO_STATUS > 14 ) ";
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
		sqlstr = " SELECT * FROM TPSSMDJ WHERE ROUTEBAGKEY IN "
			"(SELECT ROUTEBAGKEY FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm17 WHERE PONO_STATUS > 14 )) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
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
		sqlstr = " SELECT * FROM TPSSMD7 WHERE ROUTELIST IN "
			"(SELECT ROUTELIST FROM TPSSMDJ WHERE ROUTEBAGKEY IN "
			"(SELECT ROUTEBAGKEY FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm17 WHERE PONO_STATUS > 14 ))) ";
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
				" SELECT DISTINCT(ST_NO) FROM TPSSM17 "
				"  WHERE FACTORY_DIV = @v_factory_div "
				"    AND PONO_STATUS < 83 "
				"    AND PONO_STATUS > 14 "
				);
			break;
		}
		cmd_tpssm10_inq.SetCommandText(sqlstr);
		cmd_tpssm10_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssm10_inq.ExecuteReader();
		while (cmd_tpssm10_inq.Read())
		{
			tpssm17["ST_NO"] = cmd_tpssm10_inq.GetString(1);
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
					"   AND d1.factory_div = @tpssm17.factory_div"
					);
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tpssm17["ST_NO"].ToString());
			cmd_inq.Parameters.Set("tpssm17.factory_div", v_factory_div);
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

				if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
				{
					tqmts0x.Reset();
					tqmts0x["ST_NO"] = tpssmd3["ST_NO"];
					tqmts0x.Query("ST_NO");
					if (tqmts0x["C_DIV"].ToString() == "1")
					{
						st_no = "DEFAULTS";
					}
					else if (tqmts0x["C_DIV"].ToString() == "2")
					{
						st_no = "DEFAULTC";
					}
					else
					{
						st_no = "DEFAULTC";
					}

					sqlstr = CString(
						" SELECT DISTINCT d3.ST_NO, d1.DEV_TECH_CODE, d3.STD_PROC_TIME, d1.AREA_ID, d3.SMELT_MODE, d3.FACTORY_DIV, d3.STD_PREP_TIME, d3.DRAW_TIME, d3.WAITING_TIME "
						"   FROM TPSSMD3 d3,  TPSSMD1 d1 "
						"  WHERE d3.ST_NO         = @st_no "
						"    AND d3.DEV_CODE = d1.DEV_TECH_CODE  "
						"   AND d3.factory_div = d1.factory_div "
						"   AND d1.factory_div = @tpssm17.factory_div"
						"   AND d1.DEV_TECH_CODE = @DEV_TECH_CODE"
						);
					cmd_inq2.SetCommandText(sqlstr);
					cmd_inq2.Parameters.Set("st_no", st_no);
					cmd_inq2.Parameters.Set("tpssm17.factory_div", v_factory_div);
					cmd_inq2.Parameters.Set("DEV_TECH_CODE", tpssmd1["DEV_TECH_CODE"]);
					cmd_inq2.ExecuteReader();
					while (cmd_inq2.Read())
					{
						tpssmd3["STD_PROC_TIME"] = cmd_inq2.GetDecimal(3);
						//tpssmd1["AREA_ID"] = cmd_inq2.GetDecimal(4);
						//tpssmd3["SMELT_MODE"] = cmd_inq2.GetDecimal(5);
						//tpssmd3["FACTORY_DIV"] = cmd_inq2.GetString(6);
						tpssmd3["STD_PREP_TIME"] = cmd_inq2.GetDecimal(7);
						tpssmd3["DRAW_TIME"] = cmd_inq2.GetDecimal(8);
						tpssmd3["WAITING_TIME"] = cmd_inq2.GetDecimal(9);
					}
					cmd_inq2.Close();
				}

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
				"   AND T.factory_div = @tpssm17.factory_div"
				"   AND T.st_no in ('DEFAULTC','DEFAULTS')"
				;


			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm17.factory_div", v_factory_div);

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


			if (tpssmd1["AREA_ID"].ToDecimal() == 3 && tpssmd3["SMELT_MODE"].ToDecimal() == 2)// || (tpssmd1["AREA_ID"].ToDecimal() == 2 && tpssmd3["SMELT_MODE"].ToDecimal() == 0)
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
		//PrintDataTable(inblock.Tables["Dev_proc_time"]);
		//给第拾壹块赋值
		Log::Trace("", __FUNCTION__, "数据块11查询开始");
		sqlstr = " SELECT * FROM TPSSM17 WHERE PONO_STATUS < 83 AND PONO_STATUS > 14 ORDER BY CAST_LOT_NO,CAST_LOT_DIV_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm17);
			CDataRow& row_11 = inblock.Tables["Pre_plan"].Rows.Add();

			row_11["FACTORY_DIV"] = tpssm17["FACTORY_DIV"].ToString();
			row_11["ST_NO"] = tpssm17["ST_NO"].ToString();
			row_11["PONO"] = tpssm17["PONO"].ToString();
			row_11["CC_MACH_NO"] = tpssm17["CC_MACH_NO"].ToString();
			row_11["CAST_LOT_NO"] = tpssm17["CAST_LOT_NO"].ToString();
			row_11["CAST_LOT_DIV_NO"] = tpssm17["CAST_LOT_DIV_NO"].ToDecimal();
			row_11["CC_REQ_TIME"] = tpssm17["CC_REQ_TIME"].ToString();
			row_11["C_DIV"] = tpssm17["C_DIV"].ToString();
		}
		cmd_inq.Close();

		//给第拾贰块赋值
		Log::Trace("", __FUNCTION__, "数据块12查询开始");
		sqlstr = " SELECT distinct CAST_NO FROM tpssm15 WHERE PONO_STATUS < 83 AND PONO_STATUS > 14 order by CAST_NO ";
		//sqlstr = " SELECT distinct CAST_LOT_NO FROM tpssm17 WHERE PONO_STATUS < 83 AND PONO_STATUS > 14 order by CAST_LOT_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cast_lot_no1 = cmd_inq.GetString(1);

			sqlstr = " SELECT PONO FROM tpssm15 where CAST_NO = @CAST_NO  order by CAST_DIV_NO desc ";
			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("CAST_NO", cast_lot_no1);
			cmd_tpssm10_inq.ExecuteReader();
			if (cmd_tpssm10_inq.Read())
			{
				pono1 = cmd_tpssm10_inq.GetString(1);
			}

			sqlstr = " SELECT SLAB_WIDTH,SLAB_THICK,C_DIV,ST_NO,PONO FROM tpssm17 where PONO = @PONO ";
			cmd_tpssm10_inq.SetCommandText(sqlstr);
			cmd_tpssm10_inq.Parameters.Set("PONO", pono1);
			cmd_tpssm10_inq.ExecuteReader();
			if (cmd_tpssm10_inq.Read())
			{
				slab_width1 = cmd_tpssm10_inq.GetDecimal(1);
				slab_thick1 = cmd_tpssm10_inq.GetDecimal(2);
				c_div1 = cmd_tpssm10_inq.GetString(3);
				st_no1 = cmd_tpssm10_inq.GetString(4);
				pono1 = cmd_tpssm10_inq.GetString(5);
			}

			//sqlstr = " SELECT distinct CAST_LOT_NO FROM tpssm17 WHERE PONO_STATUS < 83 AND PONO_STATUS > 14 order by CAST_LOT_NO ";
			sqlstr = " SELECT distinct CAST_NO FROM tpssm15 WHERE PONO_STATUS < 83 AND PONO_STATUS > 14 order by CAST_NO ";
			cmd_inq2.SetCommandText(sqlstr);
			cmd_inq2.ExecuteReader();
			while (cmd_inq2.Read())
			{
				cast_lot_no2 = cmd_inq2.GetString(1);
				if (cast_lot_no1 == cast_lot_no2) continue;

				//fetchRowCount++;

				sqlstr = " SELECT PONO FROM tpssm15 where CAST_NO = @CAST_NO  order by CAST_DIV_NO asc ";
				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("CAST_NO", cast_lot_no2);
				cmd_tpssm10_inq.ExecuteReader();
				if (cmd_tpssm10_inq.Read())
				{
					pono2 = cmd_tpssm10_inq.GetString(1);
				}

				sqlstr = " SELECT SLAB_WIDTH,SLAB_THICK,C_DIV,ST_NO,PONO,CC_MACH_NO FROM tpssm17 where PONO = @PONO ";
				cmd_tpssm10_inq.SetCommandText(sqlstr);
				cmd_tpssm10_inq.Parameters.Set("PONO", pono2);
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

		//PrintDataTable(inblock.Tables["CAST_LOT_TIME"]);

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
			" TPSSM15 a, "
			" TPSSM17 b, "
			" TPSSMDJ c, "
			" TPSSMDK d "
			" WHERE "
			" a.PONO = b.PONO "
			" AND b.ROUTEBAGKEY = c.ROUTEBAGKEY "
			" AND c.ROUTELIST = d.ROUTELIST "
			" AND a.PONO_STATUS < 83 "
			" ORDER BY a.PONO, c.ROUTELIST ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		CDecimal dev_count = 0;
		CString dev_code = "DEV_CODE";
		CString prep_time = "PREP_TIME";
		CString proc_time = "PROC_TIME";
		CString dev_code2 = "PRE_DEV_CODE";
		CString prep_time2 = "PRE_PREP_TIME";
		CString proc_time2 = "PRE_PROC_TIME";
		CString route_dev = " ";
		CString pre_route = " ";

		while (cmd_inq.Read())
		{
			pono15 = cmd_inq.GetString(1);
			st_no15 = cmd_inq.GetString(2);
			routelist15 = cmd_inq.GetString(4);

			sqlstr = " SELECT count(1) "
				" FROM tpssm15 a, tpssmd3 c, tep0002 d, tpssm17 e "
				" WHERE a.ST_NO = c.ST_NO AND c.DEV_CODE = 'A' AND c.SMELT_MODE2 = d.CODE AND d.CODE_CLASS = 'PSAL2N' AND a.PONO = e.PONO AND e.C_DIV = '1' AND a.PONO = @pono15 "
				" ORDER BY a.SM_PLAN_NO, c.SMELT_MODE2 ";
			cmd_inq4.SetCommandText(sqlstr);
			cmd_inq4.Parameters.Set("pono15", pono15);
			if (cmd_inq4.ExecuteScalar() > 0)
			{
				sqlstr = " SELECT a.PONO, a.ST_NO, c.SMELT_MODE2, d.CODE_DESC_2_CONTENT, c.STD_PROC_TIME, c.STD_PREP_TIME, c.COST_HJ "
					" FROM tpssm15 a, tpssmd3 c, tep0002 d, tpssm17 e "
					" WHERE a.ST_NO = c.ST_NO AND c.DEV_CODE = 'A' AND c.SMELT_MODE2 = d.CODE AND d.CODE_CLASS = 'PSAL2N' AND a.PONO = e.PONO AND e.C_DIV = '1' AND a.PONO = @pono15 "
					" ORDER BY a.SM_PLAN_NO, c.SMELT_MODE2 ";
				cmd_inq4.SetCommandText(sqlstr);
				cmd_inq4.Parameters.Set("pono15", pono15);
				cmd_inq4.ExecuteReader();
				while (cmd_inq4.Read())
				{
					pre_route = " ";
					route_dev = "";

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

					row_15["PRE_PROC_TIME4"] = 0;
					row_15["PRE_PREP_TIME4"] = 0;
					row_15["PRE_DEV_CODE4"] = " ";
					row_15["PRE_PROC_TIME3"] = 0;
					row_15["PRE_PREP_TIME3"] = 0;
					row_15["PRE_DEV_CODE3"] = " ";
					row_15["PRE_PROC_TIME2"] = 0;
					row_15["PRE_PREP_TIME2"] = 0;
					row_15["PRE_DEV_CODE2"] = " ";
					row_15["PRE_PROC_TIME1"] = 0;
					row_15["PRE_PREP_TIME1"] = 0;
					row_15["PRE_DEV_CODE1"] = " ";


					backlogea15 = " ";

					row_15["PONO"] = pono15;
					row_15["ROUTELIST"] = routelist15;
					row_15["COST_ST_LINE"] = cmd_inq.GetDecimal(5);

					tpssm15["PONO"] = pono15;
					tpssm15["ROUTELIST"] = routelist15;

					dev_count = 1;
					row_15["AOD_ROUTEFLAG"] = cmd_inq4.GetString(3);//A0D前路径代码
					row_15["AOD_ROUTELIST"] = cmd_inq4.GetString(4);//AOD前路径
					row_15["CHOOSE_LIST2"] = " ";//当前路径
					row_15["COST_HJ"] = cmd_inq4.GetDecimal(7);//A0D前成本
					aod_proc_time = cmd_inq4.GetDecimal(5);
					aod_prep_time = cmd_inq4.GetDecimal(6);

					for (int i = 0; i < row_15["AOD_ROUTELIST"].ToString().Trim().GetLength(); i++)
					{
						//row_16[dev_code + to_string(i + 1)] = " ";//预溶液所有设备可行

						tpssmd3["DEV_CODE"] = row_15["AOD_ROUTELIST"][i];
						route_dev = row_15["AOD_ROUTELIST"][i];
						//Log::Info("", __FUNCTION__, "route_dev=[{0}]", route_dev);

						if (tpssmd3["DEV_CODE"].ToString().Trim() == "B" || tpssmd3["DEV_CODE"].ToString().Trim() == "E")
						{
							tpssmd3["SMELT_MODE"] = 2;
						}
						else
						{
							tpssmd3["SMELT_MODE"] = 0;
						}
						tpssmd3["SMELT_MODE2"] = " ";
						tpssmd3["FACTORY_DIV"] = v_factory_div;

						if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
						{
							tpssmd3["ST_NO"] = "DEFAULTS";
						}
						//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
						tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
						//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal());

						if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
						{
							tpssmd3["ST_NO"] = "DEFAULTS";
							tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
						}

						row_15[proc_time2 + to_string(i + 1)] = tpssmd3["STD_PROC_TIME"];
						row_15[prep_time2 + to_string(i + 1)] = tpssmd3["STD_PREP_TIME"];

						if (pre_route.Trim() == "")
						{
							pre_route = route_dev;
						}
						else
						{
							pre_route = pre_route + "," + route_dev;
						}
						//Log::Info("", __FUNCTION__, "pre_route=[{0}]", pre_route);
					}
					row_15["AOD_ROUTELIST"] = pre_route;


					if (tpssm15.QueryCount("PONO,ROUTELIST") == 0)//备用路径
					{
						row_15["CHOOSE_LIST"] = "0";
						tpssm15.Query("PONO");
						sqlstr = "SELECT * FROM TPSSMD7 WHERE FACTORY_DIV=@v_factory_div AND ROUTELIST = @ROUTELIST AND AREA_ID > 2 ";
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

							if (dev15.Trim() != "C" && dev15.Trim() != "A")//非连铸设备取静态数据配置
							{
								if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
								{
									if (cmd_inq.GetString(6) == "1")
									{
										tpssmd3["ST_NO"] = "DEFAULTS";
									}
									if (cmd_inq.GetString(6) == "2")
									{
										tpssmd3["ST_NO"] = "DEFAULTC";
									}
								}
								//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
								tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
								//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}] SMELT_MODE = [{4}] v_factory_div = [{5}] STD_PREP_TIME = [{6}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal(), tpssmd3["SMELT_MODE2"].ToString(), v_factory_div, tpssmd3["STD_PREP_TIME"].ToDecimal());

								if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
								{
									if (cmd_inq.GetString(6) == "1")
									{
										tpssmd3["ST_NO"] = "DEFAULTS";
									}
									if (cmd_inq.GetString(6) == "2")
									{
										tpssmd3["ST_NO"] = "DEFAULTC";
									}
									tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
									//Log::Info("", __FUNCTION__, "11111ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}] SMELT_MODE = [{4}] v_factory_div = [{5}] STD_PREP_TIME = [{6}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal(), tpssmd3["SMELT_MODE2"].ToString(), v_factory_div, tpssmd3["STD_PREP_TIME"].ToDecimal());
								}

								row_15[proc_time + dev_count.ToString()] = tpssmd3["STD_PROC_TIME"];
								row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
							}
							else if (dev15.Trim() == "A")
							{
								if (aod_proc_time == 0)
								{
									if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
									{
										if (cmd_inq.GetString(6) == "1")
										{
											tpssmd3["ST_NO"] = "DEFAULTS";
										}
										if (cmd_inq.GetString(6) == "2")
										{
											tpssmd3["ST_NO"] = "DEFAULTC";
										}
									}
									tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");

									if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
									{
										if (cmd_inq.GetString(6) == "1")
										{
											tpssmd3["ST_NO"] = "DEFAULTS";
										}
										if (cmd_inq.GetString(6) == "2")
										{
											tpssmd3["ST_NO"] = "DEFAULTC";
										}
										tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
									}

									row_15[proc_time + dev_count.ToString()] = tpssmd3["STD_PROC_TIME"];
									row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
								}
								else
								{
									row_15[proc_time + dev_count.ToString()] = aod_proc_time;
									row_15[prep_time + dev_count.ToString()] = aod_prep_time;
								}
							}
							else//连铸设备取计划数据
							{
								tpssm16["SM_PLAN_NO"] = tpssm15["SM_PLAN_NO"];
								tpssm16["AREA_ID"] = 5;
								tpssm16.Query("SM_PLAN_NO,AREA_ID");
								row_15[proc_time + dev_count.ToString()] = tpssm16["PROC_TIME"];
								row_15[prep_time + dev_count.ToString()] = tpssm16["PREP_TIME"];
							}

							//可行设备
							if (dev15.Trim() != "C")
							{
								tpssmdi["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
								tpssmdi["DEV_TECH_CODE"] = dev15;
								//Log::Info("", __FUNCTION__, "1111CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
								if (tpssmdi.QueryCount("CC_MACH_NO,DEV_TECH_CODE") == 0)
								{
									//row_15[dev_code + dev_count.ToString()] = " ";
								}
								else
								{
									//Log::Info("", __FUNCTION__, "2222CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
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
									//Log::Info("", __FUNCTION__, "22223333CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
								}
							}
							else
							{
								if (tpssm15["CC_MACH_NO"].ToString().Trim() == "0" || tpssm15["CC_MACH_NO"].ToString().Trim() == "1" || tpssm15["CC_MACH_NO"].ToString().Trim() == "2")
								{
									devchoose15 = CC_CHOOSE_S;
								}
								if (tpssm15["CC_MACH_NO"].ToString().Trim() == "3" || tpssm15["CC_MACH_NO"].ToString().Trim() == "4")
								{
									devchoose15 = CC_CHOOSE_C;
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
						tpssm15.Query("PONO");
						sqlstr = "SELECT * FROM TPSSM16 WHERE FACTORY_DIV=@v_factory_div AND SM_PLAN_NO = @SM_PLAN_NO AND AREA_ID > 2 ";
						sqlstr += CString(" ORDER BY CHARGE_NO ASC");
						cmd_inq2.SetCommandText(sqlstr);
						cmd_inq2.Parameters.Set("v_factory_div", v_factory_div);
						cmd_inq2.Parameters.Set("SM_PLAN_NO", tpssm15["SM_PLAN_NO"]);
						cmd_inq2.ExecuteQuery(tb_tpssm12);
						cmd_inq2.Close();

						for (int index_12 = 0; index_12 < tb_tpssm12.Rows.get_Count(); index_12++)
						{
							devchoose15 = " ";
							tpssm16.MergeFrom(tb_tpssm12.Rows[index_12]);

							tpssmd1["DEV_CODE"] = tpssm16["DEV_CODE"];
							tpssmd1["AREA_ID"] = tpssm16["AREA_ID"];
							tpssmd1["FACTORY_DIV"] = tpssm16["FACTORY_DIV"];
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

							if (dev15.Trim() == "A")
							{
								if (aod_proc_time == 0)
								{
									tpssmd3["ST_NO"] = cmd_inq.GetString(2);
									tpssmd3["SMELT_MODE"] = "0";
									tpssmd3["FACTORY_DIV"] = v_factory_div;
									tpssmd3["DEV_CODE"] = dev15;
									tpssmd3["SMELT_MODE2"] = " ";

									if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
									{
										if (cmd_inq.GetString(6) == "1")
										{
											tpssmd3["ST_NO"] = "DEFAULTS";
										}
										if (cmd_inq.GetString(6) == "2")
										{
											tpssmd3["ST_NO"] = "DEFAULTC";
										}
									}
									tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");

									if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
									{
										if (cmd_inq.GetString(6) == "1")
										{
											tpssmd3["ST_NO"] = "DEFAULTS";
										}
										if (cmd_inq.GetString(6) == "2")
										{
											tpssmd3["ST_NO"] = "DEFAULTC";
										}
										tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
									}

									row_15[proc_time + dev_count.ToString()] = tpssmd3["STD_PROC_TIME"];
									row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
								}
								else
								{
									row_15[proc_time + dev_count.ToString()] = aod_proc_time;
									row_15[prep_time + dev_count.ToString()] = aod_prep_time;
								}
							}
							else if ((dev15.Trim() == "C"))
							{
								row_15[proc_time + dev_count.ToString()] = tpssm16["PROC_TIME"];
								row_15[prep_time + dev_count.ToString()] = tpssm16["PREP_TIME"];
							}
							else
							{
								row_15[proc_time + dev_count.ToString()] = tpssm16["PROC_TIME"];

								tpssmd3["ST_NO"] = cmd_inq.GetString(2);
								tpssmd3["SMELT_MODE"] = "0";
								tpssmd3["FACTORY_DIV"] = v_factory_div;
								tpssmd3["DEV_CODE"] = dev15;
								tpssmd3["SMELT_MODE2"] = " ";

								if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
								{
									if (cmd_inq.GetString(6) == "1")
									{
										tpssmd3["ST_NO"] = "DEFAULTS";
									}
									if (cmd_inq.GetString(6) == "2")
									{
										tpssmd3["ST_NO"] = "DEFAULTC";
									}
								}
								//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
								tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
								//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}] SMELT_MODE = [{4}] v_factory_div = [{5}] STD_PREP_TIME = [{6}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal(), tpssmd3["SMELT_MODE2"].ToString(), v_factory_div, tpssmd3["STD_PREP_TIME"].ToDecimal());
								row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
							}

							//可行设备
							if (dev15.Trim() != "C")
							{
								tpssmdi["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
								tpssmdi["DEV_TECH_CODE"] = dev15;
								//Log::Info("", __FUNCTION__, "3333CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
								if (tpssmdi.QueryCount("CC_MACH_NO,DEV_TECH_CODE") == 0)
								{
									//row_15[dev_code + dev_count.ToString()] = " ";
								}
								else
								{
									//Log::Info("", __FUNCTION__, "4444CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
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
							else
							{
								if (tpssm15["CC_MACH_NO"].ToString().Trim() == "0" || tpssm15["CC_MACH_NO"].ToString().Trim() == "1" || tpssm15["CC_MACH_NO"].ToString().Trim() == "2")
								{
									devchoose15 = CC_CHOOSE_S;
								}
								if (tpssm15["CC_MACH_NO"].ToString().Trim() == "3" || tpssm15["CC_MACH_NO"].ToString().Trim() == "4")
								{
									devchoose15 = CC_CHOOSE_C;
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
			}
			else
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

				row_15["PRE_PROC_TIME4"] = 0;
				row_15["PRE_PREP_TIME4"] = 0;
				row_15["PRE_DEV_CODE4"] = " ";
				row_15["PRE_PROC_TIME3"] = 0;
				row_15["PRE_PREP_TIME3"] = 0;
				row_15["PRE_DEV_CODE3"] = " ";
				row_15["PRE_PROC_TIME2"] = 0;
				row_15["PRE_PREP_TIME2"] = 0;
				row_15["PRE_DEV_CODE2"] = " ";
				row_15["PRE_PROC_TIME1"] = 0;
				row_15["PRE_PREP_TIME1"] = 0;
				row_15["PRE_DEV_CODE1"] = " ";


				backlogea15 = " ";

				row_15["PONO"] = pono15;
				row_15["ROUTELIST"] = routelist15;
				row_15["COST_ST_LINE"] = cmd_inq.GetDecimal(5);

				row_15["AOD_ROUTEFLAG"] = " ";//A0D前路径代码
				row_15["AOD_ROUTELIST"] = " ";//AOD前路径
				row_15["CHOOSE_LIST2"] = " ";//当前路径
				row_15["COST_HJ"] = 0;//A0D前成本

				tpssm15["PONO"] = pono15;
				tpssm15["ROUTELIST"] = routelist15;

				dev_count = 1;
				//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
				if (tpssm15.QueryCount("PONO,ROUTELIST") == 0)//备用路径
				{
					row_15["CHOOSE_LIST"] = "0";
					tpssm15.Query("PONO");
					sqlstr = "SELECT * FROM TPSSMD7 WHERE FACTORY_DIV=@v_factory_div AND ROUTELIST = @ROUTELIST AND AREA_ID > 2 ";
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
							if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
							{
								if (cmd_inq.GetString(6) == "1")
								{
									tpssmd3["ST_NO"] = "DEFAULTS";
								}
								if (cmd_inq.GetString(6) == "2")
								{
									tpssmd3["ST_NO"] = "DEFAULTC";
								}
							}
							//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
							tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
							//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal());

							if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
							{
								if (cmd_inq.GetString(6) == "1")
								{
									tpssmd3["ST_NO"] = "DEFAULTS";
								}
								if (cmd_inq.GetString(6) == "2")
								{
									tpssmd3["ST_NO"] = "DEFAULTC";
								}
								tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
							}

							row_15[proc_time + dev_count.ToString()] = tpssmd3["STD_PROC_TIME"];
							row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
						}
						else//连铸设备取计划数据
						{
							tpssm16["SM_PLAN_NO"] = tpssm15["SM_PLAN_NO"];
							tpssm16["AREA_ID"] = 5;
							tpssm16.Query("SM_PLAN_NO,AREA_ID");
							row_15[proc_time + dev_count.ToString()] = tpssm16["PROC_TIME"];
							row_15[prep_time + dev_count.ToString()] = tpssm16["PREP_TIME"];
						}

						//可行设备
						if (dev15.Trim() != "C")
						{
							tpssmdi["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
							tpssmdi["DEV_TECH_CODE"] = dev15;
							//Log::Info("", __FUNCTION__, "5555CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
							if (tpssmdi.QueryCount("CC_MACH_NO,DEV_TECH_CODE") == 0)
							{
								//row_15[dev_code + dev_count.ToString()] = " ";
							}
							else
							{
								//Log::Info("", __FUNCTION__, "6666CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
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
						else
						{
							if (tpssm15["CC_MACH_NO"].ToString().Trim() == "0" || tpssm15["CC_MACH_NO"].ToString().Trim() == "1" || tpssm15["CC_MACH_NO"].ToString().Trim() == "2")
							{
								devchoose15 = CC_CHOOSE_S;
							}
							if (tpssm15["CC_MACH_NO"].ToString().Trim() == "3" || tpssm15["CC_MACH_NO"].ToString().Trim() == "4")
							{
								devchoose15 = CC_CHOOSE_C;
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
					tpssm15.Query("PONO");
					sqlstr = "SELECT * FROM TPSSM16 WHERE FACTORY_DIV=@v_factory_div AND SM_PLAN_NO = @SM_PLAN_NO AND AREA_ID > 2 ";
					sqlstr += CString(" ORDER BY CHARGE_NO ASC");
					cmd_inq2.SetCommandText(sqlstr);
					cmd_inq2.Parameters.Set("v_factory_div", v_factory_div);
					cmd_inq2.Parameters.Set("SM_PLAN_NO", tpssm15["SM_PLAN_NO"]);
					cmd_inq2.ExecuteQuery(tb_tpssm12);
					cmd_inq2.Close();

					for (int index_12 = 0; index_12 < tb_tpssm12.Rows.get_Count(); index_12++)
					{
						devchoose15 = " ";
						tpssm16.MergeFrom(tb_tpssm12.Rows[index_12]);

						tpssmd1["DEV_CODE"] = tpssm16["DEV_CODE"];
						tpssmd1["AREA_ID"] = tpssm16["AREA_ID"];
						tpssmd1["FACTORY_DIV"] = tpssm16["FACTORY_DIV"];
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

						if (tpssm16["AREA_ID"].ToDecimal() == 5)
						{
							row_15[proc_time + dev_count.ToString()] = tpssm16["PROC_TIME"];
							row_15[prep_time + dev_count.ToString()] = tpssm16["PREP_TIME"];
						}
						else
						{
							row_15[proc_time + dev_count.ToString()] = tpssm16["PROC_TIME"];

							tpssmd3["ST_NO"] = cmd_inq.GetString(2);
							tpssmd3["SMELT_MODE"] = "0";
							tpssmd3["FACTORY_DIV"] = v_factory_div;
							tpssmd3["DEV_CODE"] = dev15;
							tpssmd3["SMELT_MODE2"] = " ";

							if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
							{
								if (cmd_inq.GetString(6) == "1")
								{
									tpssmd3["ST_NO"] = "DEFAULTS";
								}
								if (cmd_inq.GetString(6) == "2")
								{
									tpssmd3["ST_NO"] = "DEFAULTC";
								}
							}
							//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
							tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
							//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}] SMELT_MODE = [{4}] v_factory_div = [{5}] STD_PREP_TIME = [{6}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal(), tpssmd3["SMELT_MODE2"].ToString(), v_factory_div, tpssmd3["STD_PREP_TIME"].ToDecimal());
							row_15[prep_time + dev_count.ToString()] = tpssmd3["STD_PREP_TIME"];
						}
						//可行设备
						if (dev15.Trim() != "C")
						{
							tpssmdi["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
							tpssmdi["DEV_TECH_CODE"] = dev15;
							//Log::Info("", __FUNCTION__, "7777CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
							if (tpssmdi.QueryCount("CC_MACH_NO,DEV_TECH_CODE") == 0)
							{
								//row_15[dev_code + dev_count.ToString()] = " ";
							}
							else
							{
								//Log::Info("", __FUNCTION__, "88888CC_MACH_NO=[{0}] DEV_TECH_CODE = [{1}]", tpssmdi["CC_MACH_NO"].ToString(), tpssmdi["DEV_TECH_CODE"].ToString());
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
						else
						{
							if (tpssm15["CC_MACH_NO"].ToString().Trim() == "0" || tpssm15["CC_MACH_NO"].ToString().Trim() == "1" || tpssm15["CC_MACH_NO"].ToString().Trim() == "2")
							{
								devchoose15 = CC_CHOOSE_S;
							}
							if (tpssm15["CC_MACH_NO"].ToString().Trim() == "3" || tpssm15["CC_MACH_NO"].ToString().Trim() == "4")
							{
								devchoose15 = CC_CHOOSE_C;
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

			cmd_inq4.Close();
		}
		cmd_inq.Close();
		//PrintDataTable(inblock.Tables["DEV_ARRANGE"]);

		//给第拾陆块赋值
		Log::Trace("", __FUNCTION__, "数据块16查询开始");
		sqlstr = " SELECT a.SM_PLAN_NO,a.PONO,a.ST_NO,b.DEV_CODE,c.SMELT_MODE2,d.CODE_DESC_2_CONTENT,c.STD_PROC_TIME,c.COST_HJ,a.FACTORY_DIV FROM tpssm15 a,tpssm16 b,tpssmd3 c,tep0002 d,tpssm17 e WHERE a.sm_plan_no = b.sm_plan_no AND b.DEV_CODE LIKE 'A%' AND a.RUN_STATUS = '00' AND a.ST_NO = c.ST_NO AND c.DEV_CODE = 'A' AND c.SMELT_MODE2 <> ' ' AND c.SMELT_MODE2 = d.CODE AND d.CODE_CLASS = 'PSAL2N' AND a.PONO = e.PONO AND e.C_DIV = '1' ORDER BY a.SM_PLAN_NO,c.SMELT_MODE2 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();

		//CString pre_route = " ";
		//CString route_dev = " ";
		while (cmd_inq.Read())
		{
			CDataRow& row_16 = inblock.Tables["PRE_SOLUTION_ARRANGE"].Rows.Add();

			row_16["SM_PLAN_NO"] = cmd_inq.GetString(1);
			row_16["PONO"] = cmd_inq.GetString(2);
			row_16["ST_NO"] = cmd_inq.GetString(3);
			row_16["DEV_CODE"] = cmd_inq.GetString(4);
			row_16["SMELT_MODE2"] = cmd_inq.GetString(5);
			row_16["PRE_ROUTE"] = cmd_inq.GetString(6);
			row_16["STD_PROC_TIME"] = cmd_inq.GetDecimal(7);
			row_16["COST_HJ"] = cmd_inq.GetDecimal(8);
			row_16["AOD_CHOOSE_LIST"] = " ";

			row_16["PROC_TIME4"] = 0;
			row_16["PREP_TIME4"] = 0;
			row_16["DEV_CODE4"] = " ";
			row_16["PROC_TIME3"] = 0;
			row_16["PREP_TIME3"] = 0;
			row_16["DEV_CODE3"] = " ";
			row_16["PROC_TIME2"] = 0;
			row_16["PREP_TIME2"] = 0;
			row_16["DEV_CODE2"] = " ";
			row_16["PROC_TIME1"] = 0;
			row_16["PREP_TIME1"] = 0;
			row_16["DEV_CODE1"] = " ";

			tpssmd3["ST_NO"] = row_16["ST_NO"];
			tpssmd3["FACTORY_DIV"] = cmd_inq.GetString(9);
			tpssmd3["SMELT_MODE2"] = " ";
			pre_route = " ";
			route_dev = "";

			for (int i = 0; i < row_16["PRE_ROUTE"].ToString().Trim().GetLength(); i++)
			{
				//row_16[dev_code + to_string(i + 1)] = " ";//预溶液所有设备可行

				tpssmd3["DEV_CODE"] = row_16["PRE_ROUTE"][i];
				route_dev = row_16["PRE_ROUTE"][i];
				//Log::Info("", __FUNCTION__, "route_dev=[{0}]", route_dev);

				if (tpssmd3["DEV_CODE"].ToString().Trim() == "B" || tpssmd3["DEV_CODE"].ToString().Trim() == "E")
				{
					tpssmd3["SMELT_MODE"] = 2;
				}
				else
				{
					tpssmd3["SMELT_MODE"] = 0;
				}

				if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
				{
					tpssmd3["ST_NO"] = "DEFAULTS";
				}
				//Log::Info("", __FUNCTION__, "pono=[{0}] routelist15 = [{1}]", tpssm15["PONO"].ToString(), routelist15);
				tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
				//Log::Info("", __FUNCTION__, "ST_NO=[{0}] SMELT_MODE = [{1}]  DEV_CODE = [{2}] STD_PROC_TIME = [{3}]", tpssmd3["ST_NO"].ToString(), tpssmd3["SMELT_MODE"].ToString(), tpssmd3["DEV_CODE"].ToString(), tpssmd3["STD_PROC_TIME"].ToDecimal());

				if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
				{
					tpssmd3["ST_NO"] = "DEFAULTS";
					tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
				}

				row_16[proc_time + to_string(i + 1)] = tpssmd3["STD_PROC_TIME"];
				row_16[prep_time + to_string(i + 1)] = tpssmd3["STD_PREP_TIME"];

				if (pre_route.Trim() == "")
				{
					pre_route = route_dev;
				}
				else
				{
					pre_route = pre_route + "," + route_dev;
				}
				//Log::Info("", __FUNCTION__, "pre_route=[{0}]", pre_route);
			}
			row_16["AOD_BACKLOG_EA"] = pre_route;
			if (row_16["STD_PROC_TIME"].ToDecimal() == 0)
			{
				//tpssmd3["ST_NO"] = "DEFAULTS";
				tpssmd3["DEV_CODE"] = "A";
				tpssmd3["SMELT_MODE"] = 0;
				tpssmd3["ST_NO"] = row_16["ST_NO"];
				if (tpssmd3.QueryCount("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2") == 0)
				{
					tpssmd3["ST_NO"] = "DEFAULTS";
				}
				tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");

				if (tpssmd3["STD_PROC_TIME"].ToDecimal() == 0)
				{
					tpssmd3["ST_NO"] = "DEFAULTS";
					tpssmd3.Query("ST_NO,SMELT_MODE,DEV_CODE,FACTORY_DIV,SMELT_MODE2");
				}

				row_16["STD_PROC_TIME"] = tpssmd3["STD_PROC_TIME"];
			}
		}
		cmd_inq.Close();
		//PrintDataTable(inblock.Tables["PRE_SOLUTION_ARRANGE"]);

		//给第拾柒块赋值
		Log::Trace("", __FUNCTION__, "数据块17查询开始");
		sqlstr = " SELECT * FROM (SELECT MAX(a.END_TIME) AS TIME ,a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO FROM tpssm16 a,tpssm15 b WHERE a.SM_PLAN_NO = b.SM_PLAN_NO AND b.CAST_NO IN (SELECT CAST_NO FROM TPSSM15 WHERE FACTORY_DIV = 'LG1' AND PONO_STATUS >= 20  AND STEEL_RETURN_CODE = ' ' AND CAST_DIV_NO = 1) GROUP BY a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO ) ORDER BY DEV_CODE,TIME desc ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();

		//CString dev_code2 = " ";
		while (cmd_inq.Read())
		{
			if (dev_code2 != cmd_inq.GetString(2))
			{
				CDataRow& row_17 = inblock.Tables["DEV_LAST_TIME"].Rows.Add();

				row_17["SM_PLAN_NO"] = cmd_inq.GetString(3);
				row_17["PONO"] = cmd_inq.GetString(4);
				row_17["TIME"] = cmd_inq.GetString(1);
				row_17["DEV_CODE"] = cmd_inq.GetString(2);
				row_17["CAST_NO"] = cmd_inq.GetString(5);

				dev_code2 = cmd_inq.GetString(2);
			}
		}
		cmd_inq.Close();
		//PrintDataTable(inblock.Tables["DEV_LAST_TIME"]);

		ret = f_epex_call_rest_tpsmodel_lib(&inblock, &outblock, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (outblock.Tables.Contains("INFO"))
		{
			//PrintDataTable(outblock.Tables["INFO"]);
			bcls_ret->Tables[0].Copy(outblock.Tables["INFO"]);
			//更新计划 
			//tpssm15.Reset();
			//tpssm17.Reset();
			//for (int index = 0; index < outblock.Tables["INFO"].Rows.get_Count(); index++)
			//{
			//	Log::Trace("", __FUNCTION__, "PONO = {0},CHARGE_NO={1},DEV_CODE={2} ", outblock.Tables["INFO"].Rows[index]["PONO"].ToString(), outblock.Tables["INFO"].Rows[index]["CHARGE_NO"].ToString(), outblock.Tables["INFO"].Rows[index]["DEV_CODE"].ToString());
			//	if (tpssm15["PONO"].ToString().Trim() != outblock.Tables["INFO"].Rows[index]["PONO"].ToString())
			//	{
			//		tpssm15["PONO"] = outblock.Tables["INFO"].Rows[index]["PONO"].ToString();
			//		tpssm15.Query("PONO");

			//		ref_route = "";
			//		backlog_ea = "";
			//		routelist = outblock.Tables["INFO"].Rows[index]["ROUTELIST"].ToString();
			//	}
			//	
			//	tpssmd1["DEV_CODE"] = outblock.Tables["INFO"].Rows[index]["DEV_CODE"].ToString();
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

			//	if (outblock.Tables["INFO"].Rows[index]["CHARGE_NO"].ToDecimal() < tpssm15["CURR_WP_NO"].ToDecimal())
			//	{
			//		continue;
			//	}
			//	tpssm16["SM_PLAN_NO"] = tpssm15["SM_PLAN_NO"];
			//	tpssm16["CHARGE_NO"] = outblock.Tables["INFO"].Rows[index]["CHARGE_NO"].ToDecimal();
			//	tpssm16["REC_REVISE_TIME"] = dateNow;
			//	tpssm16["REC_REVISOR"] = s.userid;
			//	tpssm16["PRE_PROC_NO"] = " ";
			//	tpssm16["PROC_NO"] = " ";
			//	tpssm16["DEV_CODE"] = outblock.Tables["INFO"].Rows[index]["DEV_CODE"].ToString();
			//	tpssm16["START_TIME"] = outblock.Tables["INFO"].Rows[index]["START_TIME"].ToString();
			//	tpssm16["END_TIME"] = outblock.Tables["INFO"].Rows[index]["END_TIME"].ToString();

			//	if (outblock.Tables["INFO"].Rows[index]["CHARGE_NO"].ToDecimal() == tpssm15["CURR_WP_NO"].ToDecimal())
			//	{
			//		tpssm16.Update("START_TIME,END_TIME", "SM_PLAN_NO,CHARGE_NO");
			//		continue;
			//	}
			//	//tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PRE_PROC_NO,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");
			//	//Log::Trace("", __FUNCTION__, "tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());
			//	tpssm16.Update("START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");
			//	//Log::Trace("", __FUNCTION__, "111tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());

			//	tpssm15["BACKLOG_EA"] = backlog_ea.TrimOrBlank();
			//	tpssm15["REFINE_ROUTE_CODE"] = ref_route.TrimOrBlank();
			//	tpssm15["ROUTELIST"] = routelist.TrimOrBlank();

			//	//修改工序主表的内容
			//	tpssm15.Update("REC_REVISOR,REC_REVISE_TIME,BACKLOG_EA,REFINE_ROUTE_CODE,ROUTELIST", "PONO");

			//	if (tpssmd1["AREA_ID"].ToDecimal() == 5)
			//	{
			//		tpssm15["CC_MACH_NO"] = tpssmd1["STATION_NO"];
			//		tpssm15.Update("CC_MACH_NO", "PONO");
			//	}

			//	//tpssm17["PONO"] = tpssm15["PONO"].ToString();
			//	//tpssm17["ROUTELIST"] = tpssm15["ROUTELIST"].ToString();
			//	//tpssm17.Update("ROUTELIST", "PONO");
			//}
			//ret = f_pssm21_cast_cre_n(&inblk, &inblk_out, conn);
			//if (ret < 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}
		}
		if (outblock.Tables.Contains("IFF"))
		{
			bcls_ret->Tables.Add();
			bcls_ret->Tables[1].Copy(outblock.Tables["IFF"]);
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

int GenTpsIn_route_create(CString pono, CString& pono_route, CString& route_relaion, CString& route_div, CDbConnection * conn)
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

		sqlstr = "SELECT T1.PONO,T2.AREA_ID,T2.DEV_CODE,T2.CHARGE_NO,T3.DEV_TECH_CODE FROM TPSSM15 T1, TPSSM16 T2,TPSSMD1 T3 ";
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
