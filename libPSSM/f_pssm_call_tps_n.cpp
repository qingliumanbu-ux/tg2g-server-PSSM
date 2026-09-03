/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:    顾东亮
Version:   3.1.0
Date:     2011-12-27
Description:	 模型接口程序 TPS2.0。
Update：   2014-11-14  xuwen  炉次条件合并，增加转炉区子阶段计算
2015-03-23  lijie  增加动态调用
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "CUtils.h"

#if WIN32
#include "WinBase.h"
#else
#include <dlfcn.h>
#endif


#if WIN32
void* GetPSTSLibHandle(const char * funcname, HMODULE m)
#else
void* GetPSTSLibHandle(const char * funcname, void * m)
#endif
{
	CTracer log(__FUNCTION__);
	CString strDynamicLibFullName("");

	//获取动态库句柄
#if WIN32
	//Log::Trace("", __FUNCTION__, "111");
	strDynamicLibFullName = ".\\libPSTS.dll";
	m = LoadLibrary((const char *)strDynamicLibFullName);
	if (m == NULL)
	{
		Log::Trace("", __FUNCTION__, "无法加载{0}的动态库！", strDynamicLibFullName);
		return NULL;
	}
	Log::Trace("", __FUNCTION__, "开始获取函数句柄");
	return (void*)GetProcAddress(m, funcname);
#else
	////获取环境变量
	//char* pEnvBuildDirValue = getenv("BM2_BUILD_DIR");
	//if (pEnvBuildDirValue == NULL)
	//{
	//	return NULL;
	//}
	//strDynamicLibFullName = pEnvBuildDirValue;
	//if (strDynamicLibFullName[strDynamicLibFullName.GetLength()-1] != '/')
	//{
	//	strDynamicLibFullName += "/";
	//}
	//strDynamicLibFullName += "Lib/libPSTS.so";
	//Log::Trace("", __FUNCTION__, "动态库位置：[{0}]", strDynamicLibFullName); 
	//m = dlopen((const char *)strDynamicLibFullName, RTLD_LAZY);
	//if (m == NULL)
	//{
	//	Log::Trace("", __FUNCTION__, "动态库获取失败原因是：[{0}]", dlerror()); 
	//	return NULL;
	//}
	//Log::Trace("", __FUNCTION__, "开始获取函数句柄");
	//return dlsym(m, funcname);
#endif
}



/************************************************************
全局C变量定义
************************************************************/
char file_name_in[100];  //模型计算用输入参数文件
char file_name_in_1[100];  //模型计算用输入参数文件
char file_name_out[100]; //模型计算输出参数文件
char file_name_err[100]; //模型计算输出错误文件
char equipmentInfo_in[100]; //设备代码传入文件
char trantime_in[100];//传搁时间传入文件
char equipmentStateInfo_in[100];//设备故障传入文件
char plan_llc_in[100];//炉次路径传入文件
char Stno_route_in[100]; //钢种对应工艺路径传入文件
char ShareEquipmentInfo_in[100]; //双工位交错时间传入文件
char CCM_abnormal_time_in[100]; //连铸异常处理时间传入文件
char tpssmdh_in[100]; //钢种-路径包传入文件
char tpssmdj_in[100]; //路径包-路径传入文件
char tpssmd7_in[100]; //路径-设备列表传入文件
char tpssmd3_in[100]; //处理时间传入文件
char tpssm10_in[100]; //预计划传入文件
char CAST_LOT_TIME_in[100]; //浇次间隔传入文件
char DevLastTime[100]; 

CString sys_time = "";
CString sys_time_1 = "";
CString v_factory_div = "";

void readfile(char *readstr, int max, FILE *fileptr);
void readfile_l(char *readstr, int max, FILE *fileptr);
//int TPS_Test(char *path);
int GenTpsIn_output_pono_condition(int mode, CDbConnection * conn);
int GenTpsIn_output_pono_condition_charge(int mode, CDbConnection * conn);
int GenTpsIn_output_pono_condition_1(int mode, CDbConnection * conn);//用于滚动编制文本生成
int GenTpsIn_read_scheduling_err(int mode, CDbConnection * conn);
int GenTpsIn_scheduling_pono(int mode);  //mode: 0-编制; 1-优化
int GenTpsIn_read_scheduling(int mode, CDbConnection * conn);
int GenTpsIn_read_scheduling_charge(int mode, CDbConnection * conn);
int GenTpsIn_device_code(CDbConnection * conn);
int GenTpsIn_device_codeb(CDbConnection * conn);
int GenTpsIn_device_trantime(CDbConnection * conn);
int GenTpsIn_device_trantimeb(CDbConnection * conn);
int GenTpsIn_device_StateInfo(CDbConnection * conn);
int GenTpsIn_device_StateInfob(CDbConnection * conn);
int GenTpsIn_route_create(const char * pono, char *pono_route, char *route_relaion, char *route_div, CDbConnection * conn);
int GenTpsIn_route_createb(const char * pono, char *pono_route, char *route_relaion, char *route_div, CDbConnection * conn);
int GenTpsIn_llc(CDbConnection * conn);//炉次路径配置文件

int GenTpsIn_TAPBD001S2N(CDbConnection * conn);
int GenTpsIn_TAPBD006S2N(CDbConnection * conn);
int GenTpsIn_TAPBD006S2Nb(CDbConnection * conn);
int GenTpsIn_TAPBD008S2N(CDbConnection * conn);
int GenTpsIn_TPSSMDH(CDbConnection * conn);
int GenTpsIn_TPSSMDJ(CDbConnection * conn);
int GenTpsIn_TPSSMD7(CDbConnection * conn);
int GenTpsIn_TPSSMD3(CDbConnection * conn);
int GenTpsIn_TPSSM10(CDbConnection * conn);
int GenTpsIn_CAST_LOT_TIME(CDbConnection * conn);
int GenTpsIn_DEV_LAST_TIME(CDbConnection * conn);

int f_pssm_query(EIClass inblock_condition, EIClass inblock_source, EIClass& outblock_result, CDbConnection * conn);

extern "C"
int TPS_Test(char *path);


extern "C"
int PlanWeave(char *infilepath, char *outfilepath, char * equipmentfilepath, char * equipmentstattfilepath, char * trantimefilepath, int modeoption, int customizeoption, int optimizeoption, int pathsensitivity);

extern "C"
int PlanTimeOptimazation(char *infilepath, char *outfilepath, char * equipmentfilepath, char * equipmentstattfilepath, char * trantimefilepath, int modeoption);

extern "C"
int PlanTimeOptimazation2(char *infilepath, char *outfilepath, char * equipmentfilepath, char * equipmentstattfilepath, char * trantimefilepath, int modeoption);


BM2_FUNCTION_EXPORT
int f_pssm_call_tps_n(CString factory_div, int mode, CDbConnection * conn) //
{
	CTracer log(__FUNCTION__);
	int iRet;
	int doFlag = 0;
	CString sqlstr = "";
	try
	{
		sys_time = CDateTime::Now().ToString("yyyyMMddHHmmss");
		////Log::Trace("", __FUNCTION__,"sys_time=[{0}]",(const char *)sys_time);
		sys_time_1 = CDateTime::Now().AddSeconds(1).ToString("yyyyMMddHHmmss");
		////Log::Trace("", __FUNCTION__,"sys_time_1=[{0}]",(const char *)sys_time_1);


		/*--------------------------------------------------
		数据初始化 全局变量
		--------------------------------------------------*/
		v_factory_div = factory_div;
		Log::Trace("", __FUNCTION__, "=FACTORY_DIV=[{0}],mode=[{1}]", v_factory_div, mode);


		if (mode == 4)
		{
			iRet = GenTpsIn_device_codeb(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_device_code err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			iRet = GenTpsIn_TAPBD006S2Nb(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_TAPBD006S2N err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			iRet = GenTpsIn_device_trantimeb(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_device_trantime err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			iRet = GenTpsIn_device_StateInfob(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_device_StateInfo err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			iRet = GenTpsIn_output_pono_condition_charge(mode, conn);
			if (iRet == -1)
			{
				//sprintf(s.msg,"GenTpsIn_output_pono_condition err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			iRet = GenTpsIn_DEV_LAST_TIME(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_DEV_LAST_TIME err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			Log::Trace("", __FUNCTION__, "启动计算");
			iRet = GenTpsIn_scheduling_pono(mode);
			if (iRet == -1)
			{
				////Log::Trace("", __FUNCTION__, "启动计算");
				sprintf(s.msg, "计算失败---GenTpsIn_scheduling_pono err");
				throw CApplicationException(-1, s.msg, log.Location);

			}

			Log::Trace("", __FUNCTION__, "读入计算结果");
			iRet = GenTpsIn_read_scheduling_charge(mode, conn);
			if (iRet == -1)
			{
				////Log::Trace("", __FUNCTION__, "读入计算结果");
				sprintf(s.msg, "读入计算结果失败---GenTpsIn_read_scheduling err");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			
		}
		else
		{
			/*--------------------------------------------------
			设备代码文件输出//第一块，设备代码
			--------------------------------------------------*/
			iRet = GenTpsIn_device_code(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_device_code err");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			/*--------------------------------------------------
			设备代码文件输出
			--------------------------------------------------*/
			/*iRet = GenTpsIn_TAPBD001S2N(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD001S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			//第五块，双工位交错时间
			--------------------------------------------------*/
			iRet = GenTpsIn_TAPBD006S2N(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_TAPBD006S2N err");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			/*--------------------------------------------------
			//增加第六块 连铸异常处理时间
			--------------------------------------------------*/
			/*iRet = GenTpsIn_TAPBD008S2N(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD008S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			钢种-路径包文件输出//增加第七块 钢种-路径包
			--------------------------------------------------*/
			/*iRet = GenTpsIn_TPSSMDH(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD008S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			路径包-路径文件输出//增加第八块 路径包-路径
			--------------------------------------------------*/
			/*iRet = GenTpsIn_TPSSMDJ(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD008S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			路径-设备列表文件输出//增加第九块 设路径-设备
			--------------------------------------------------*/
			/*iRet = GenTpsIn_TPSSMD7(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD008S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			处理时间文件输出//增加第十块 设备处理时间
			--------------------------------------------------*/
			/*iRet = GenTpsIn_TPSSMD3(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD008S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			浇次间隔文件输出//增加第十二块 预浇次间隔时间
			--------------------------------------------------*/
			/*iRet = GenTpsIn_CAST_LOT_TIME(conn);
			if (iRet == -1)
			{
			sprintf(s.msg, "GenTpsIn_TAPBD008S2N err");
			throw CApplicationException(-1, s.msg, log.Location);
			}*/
			/*--------------------------------------------------
			设备传搁时间文件输出//第二块，传搁时间
			--------------------------------------------------*/
			iRet = GenTpsIn_device_trantime(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_device_trantime err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*--------------------------------------------------
			设备定修文件输出//第三块，设备定修
			--------------------------------------------------*/
			iRet = GenTpsIn_device_StateInfo(conn);
			if (iRet == -1)
			{
				sprintf(s.msg, "GenTpsIn_device_StateInfo err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*--------------------------------------------------
			炉次路径配置文件输出
			--------------------------------------------------*/
			//if (mode == 0) //全局优化增加此文件
			//{
			//	iRet = GenTpsIn_llc(conn);
			//	if (iRet == -1)
			//	{
			//		sprintf(s.msg, "GenTpsIn_llc err");
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			//}

			if (mode == 2)	//滚动编制，增加不改变的计划in文件
			{
				Log::Trace("", __FUNCTION__, "add by zhengqq 2023.5.25 滚动计划增加此段，增加不改变的出钢计划输出文件");
				//add by zhengqq 2023.5.25 滚动计划增加此段，增加不改变的出钢计划输出文件
				iRet = GenTpsIn_output_pono_condition_1(mode, conn);
				if (iRet == -1)
				{
					//sprintf(s.msg,"GenTpsIn_output_pono_condition_1 err");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}


			/*--------------------------------------------------
			炉次条件文件输出//第四块，计划相关
			--------------------------------------------------*/
			iRet = GenTpsIn_output_pono_condition(mode, conn);
			if (iRet == -1)
			{
				//sprintf(s.msg,"GenTpsIn_output_pono_condition err");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			////Log::Trace("", __FUNCTION__, "v_factory_div=[{0}], mode=[{1}]", (const char *)v_factory_div, mode);
			/*--------------------------------------------------
			启动模型计算
			--------------------------------------------------*/
			iRet = GenTpsIn_scheduling_pono(mode);
			if (iRet == -1)
			{
				////Log::Trace("", __FUNCTION__, "启动计算");
				sprintf(s.msg, "计算失败---GenTpsIn_scheduling_pono err");
				throw CApplicationException(-1, s.msg, log.Location);

			}
			/*--------------------------------------------------
			读入模型计算结果--计划编制
			--------------------------------------------------*/
			iRet = GenTpsIn_read_scheduling(mode, conn);
			if (iRet == -1)
			{
				////Log::Trace("", __FUNCTION__, "读入计算结果");
				sprintf(s.msg, "读入计算结果失败---GenTpsIn_read_scheduling err");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			iRet = GenTpsIn_read_scheduling_err(mode, conn);
			if (iRet == -1)
			{
				////Log::Trace("", __FUNCTION__, "读入计算结果");
				sprintf(s.msg, "读入计算结果失败---GenTpsIn_read_scheduling err");
				throw CApplicationException(-1, s.msg, log.Location);
			}
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

//设备代码文件输出

/************************************************************
设备代码IN文件输出
注:
1设备代码
2设备类型
3设备名称
4设备区分标志
5设备分类标志    转炉是1、精炼是2、连铸等待是4、连铸是5、模铸是6（扒渣预留为2)、预处理是7
************************************************************/
//第一块，设备代码
int GenTpsIn_device_code(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmd6("TPSSMD6");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(equipmentInfo_in, "%s\\Trace\\EquipmentInfo.in", p);  //模型计算用输入参数文件
#else
			sprintf(equipmentInfo_in, "%s/Trace/EquipmentInfo.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(equipmentInfo_in, "..\\Trace\\EquipmentInfo.in");  //默认系统运行在UBin目录下
#else
			sprintf(equipmentInfo_in, "../Trace/EquipmentInfo.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(equipmentInfo_in, "w");
		setbuf(outstream, NULL);

		sqlstr = "SELECT COUNT(1) FROM TPSSMD1 WHERE FACTORY_DIV=@v_factory_div AND  AREA_ID>=2 AND DEV_TECH_CODE <> 'X' AND DEV_TECH_CODE <> 'Y' AND DEV_TECH_CODE <> 'L' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;设备数量\n", dev_count);

		sqlstr = "SELECT * FROM TPSSMD1 WHERE FACTORY_DIV=@v_factory_div AND  AREA_ID>=2 AND DEV_TECH_CODE <> 'X' AND DEV_TECH_CODE <> 'Y' AND DEV_TECH_CODE <> 'L' ";
		sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;设备序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;设备代码\n", (const char *)tpssmd1["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备类型\n", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备名称\n", (const char *)tpssmd1["STATION_NAME"].ToString());


			dev_tab = tpssmd1["DEV_CODE"];

			fprintf(outstream, "%s\t\t\t\t;设备区分标志\n", (const char *)dev_tab);
			if (tpssmd1["AREA_ID"].ToDecimal() == 1)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "7");
			}
			if (tpssmd1["AREA_ID"].ToDecimal() == 3 || tpssmd1["AREA_ID"].ToDecimal() == 2)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "1");
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 4)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "2");
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 5)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "5");
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 6)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "6");
			}
			fprintf(outstream, "%s\t\t\t\t;设备准备时间\n\n", "0");
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}
int GenTpsIn_device_codeb(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmd6("TPSSMD6");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(equipmentInfo_in, "%s\\Trace\\bEquipmentInfo.in", p);  //模型计算用输入参数文件
#else
			sprintf(equipmentInfo_in, "%s/Trace/bEquipmentInfo.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(equipmentInfo_in, "..\\Trace\\bEquipmentInfo.in");  //默认系统运行在UBin目录下
#else
			sprintf(equipmentInfo_in, "../Trace/bEquipmentInfo.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(equipmentInfo_in, "w");
		setbuf(outstream, NULL);

		sqlstr = "SELECT COUNT(1) FROM TPSSMD1 WHERE FACTORY_DIV=@v_factory_div AND  AREA_ID>=2 AND DEV_TECH_CODE <> 'X' AND DEV_TECH_CODE <> 'Y' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;设备数量\n", dev_count);

		sqlstr = "SELECT * FROM TPSSMD1 WHERE FACTORY_DIV=@v_factory_div AND  AREA_ID>=2 AND DEV_TECH_CODE <> 'X' AND DEV_TECH_CODE <> 'Y' ";
		sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd1);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;设备序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;设备代码\n", (const char *)tpssmd1["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备类型\n", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备名称\n", (const char *)tpssmd1["STATION_NAME"].ToString());


			dev_tab = tpssmd1["DEV_CODE"];

			fprintf(outstream, "%s\t\t\t\t;设备区分标志\n", (const char *)dev_tab);
			if (tpssmd1["AREA_ID"].ToDecimal() == 1)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "7");
			}
			if (tpssmd1["AREA_ID"].ToDecimal() == 3 || tpssmd1["AREA_ID"].ToDecimal() == 2)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "1");
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 4)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "2");
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 5)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "5");
			}
			else if (tpssmd1["AREA_ID"].ToDecimal() == 6)
			{
				fprintf(outstream, "%s\t\t\t\t;设备分类标志\n", "6");
			}
			fprintf(outstream, "%s\t\t\t\t;设备准备时间\n\n", "0");
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}


/*钢种对应工艺路径*/
int GenTpsIn_TAPBD001S2N(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tapbd001s2n("TAPBD001S2N");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(Stno_route_in, "%s\\Trace\\Stno_route.in", p);  //模型计算用输入参数文件
#else
			sprintf(Stno_route_in, "%s/Trace/Stno_route.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(Stno_route_in, "..\\Trace\\Stno_route.in");  //默认系统运行在UBin目录下
#else
			sprintf(Stno_route_in, "../Trace/Stno_route.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(Stno_route_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT COUNT(1) FROM TAPBD001S2N ";
		cmd_inq.SetCommandText(sqlstr);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = "SELECT * FROM TAPBD001S2N ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tapbd001s2n);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;钢种\n", (const char *)tapbd001s2n["ST_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;路径类型\n", (const char *)tapbd001s2n["ROUTE_TYPE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;工艺路径\n", (const char *)tapbd001s2n["RULE_DESC"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//第五块，双工位交错时间
/*双工位交错时间*/
int GenTpsIn_TAPBD006S2N(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tapbd006s2n("TAPBD006S2N");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(ShareEquipmentInfo_in, "%s\\Trace\\ShareEquipmentInfo.in", p);  //模型计算用输入参数文件
#else
			sprintf(ShareEquipmentInfo_in, "%s/Trace/ShareEquipmentInfo.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(ShareEquipmentInfo_in, "..\\Trace\\ShareEquipmentInfo.in");  //默认系统运行在UBin目录下
#else
			sprintf(ShareEquipmentInfo_in, "../Trace/ShareEquipmentInfo.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(ShareEquipmentInfo_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT COUNT(1) FROM TAPBD006S2N ";
		cmd_inq.SetCommandText(sqlstr);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TAPBD006S2N ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tapbd006s2n);
			fetchRowCount++;

			
			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s,%s\t\t\t\t;共享子设备\n", (const char *)tapbd006s2n["STATION_NAME"].ToString(), (const char *)tapbd006s2n["STATION_NAME_2"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;设备代码2\n", (const char *)tapbd006s2n["STATION_NAME_2"].ToString());
			fprintf(outstream, "%s\t\t\t\t;类型\n", (const char *)tapbd006s2n["TD_TYPE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;不可共用时间\n", (const char *)tapbd006s2n["STAG_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;移行时间\n", (const char *)tapbd006s2n["MOVE_TIME"].ToString());
			fprintf(outstream, "\n");
			
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

int GenTpsIn_TAPBD006S2Nb(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tapbd006s2n("TAPBD006S2N");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(ShareEquipmentInfo_in, "%s\\Trace\\bShareEquipmentInfo.in", p);  //模型计算用输入参数文件
#else
			sprintf(ShareEquipmentInfo_in, "%s/Trace/bShareEquipmentInfo.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(ShareEquipmentInfo_in, "..\\Trace\\bShareEquipmentInfo.in");  //默认系统运行在UBin目录下
#else
			sprintf(ShareEquipmentInfo_in, "../Trace/bShareEquipmentInfo.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(ShareEquipmentInfo_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT COUNT(1) FROM TAPBD006S2N ";
		cmd_inq.SetCommandText(sqlstr);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TAPBD006S2N ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tapbd006s2n);
			fetchRowCount++;


			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s,%s\t\t\t\t;共享子设备\n", (const char *)tapbd006s2n["STATION_NAME"].ToString(), (const char *)tapbd006s2n["STATION_NAME_2"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;设备代码2\n", (const char *)tapbd006s2n["STATION_NAME_2"].ToString());
			fprintf(outstream, "%s\t\t\t\t;类型\n", (const char *)tapbd006s2n["TD_TYPE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;不可共用时间\n", (const char *)tapbd006s2n["STAG_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;移行时间\n", (const char *)tapbd006s2n["MOVE_TIME"].ToString());
			fprintf(outstream, "\n");

		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第六块 连铸异常处理时间
/*连铸异常处理时间*/
int GenTpsIn_TAPBD008S2N(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tapbd008s2n("TAPBD008S2N");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(CCM_abnormal_time_in, "%s\\Trace\\CCM_abnormal_time.in", p);  //模型计算用输入参数文件
#else
			sprintf(CCM_abnormal_time_in, "%s/Trace/CCM_abnormal_time.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(CCM_abnormal_time_in, "..\\Trace\\CCM_abnormal_time.in");  //默认系统运行在UBin目录下
#else
			sprintf(CCM_abnormal_time_in, "../Trace/CCM_abnormal_time.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(CCM_abnormal_time_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT COUNT(1) FROM TAPBD008S2N ";
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TAPBD008S2N ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tapbd008s2n);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;厂别区分\n", (const char *)tapbd008s2n["FACTORY_DIV"].ToString());
			fprintf(outstream, "%s\t\t\t\t;炼钢区域标识\n", (const char *)tapbd008s2n["AREA_ID"].ToString());
			fprintf(outstream, "%s\t\t\t\t;区域中文\n", (const char *)tapbd008s2n["AREA_CNAME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备 状态\n", (const char *)tapbd008s2n["DEVICE_STATUS"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备状态描述\n", (const char *)tapbd008s2n["DEV_STATUS_REMARK"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备代码\n", (const char *)tapbd008s2n["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备工艺代码\n", (const char *)tapbd008s2n["DEV_TECH_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;工作时间\n", (const char *)tapbd008s2n["WORK_TIME"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第七块 钢种-路径包
/*钢种-路径包*/
int GenTpsIn_TPSSMDH(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmdh("TPSSMDH");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(tpssmdh_in, "%s\\Trace\\tpssmdh.in", p);  //模型计算用输入参数文件
#else
			sprintf(tpssmdh_in, "%s/Trace/tpssmdh.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(tpssmdh_in, "..\\Trace\\tpssmdh.in");  //默认系统运行在UBin目录下
#else
			sprintf(tpssmdh_in, "../Trace/tpssmdh.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(tpssmdh_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT count(1) FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm10) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TPSSMDH WHERE FACTORY_DIV = @v_factory_div and st_no in (select distinct st_no from tpssm10) ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdh);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;厂别区分\n", (const char *)tpssmdh["FACTORY_DIV"].ToString());
			fprintf(outstream, "%s\t\t\t\t;钢种\n", (const char *)tpssmdh["ST_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;路径包\n", (const char *)tpssmdh["ROUTEBAGKEY"].ToString());
			fprintf(outstream, "%s\t\t\t\t;注释\n", (const char *)tpssmdh["REMARK"].ToString());
			fprintf(outstream, "%s\t\t\t\t;成本\n", (const char *)tpssmdh["COST_ST_LINE"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第八块 路径包-路径
/*路径包-路径*/
int GenTpsIn_TPSSMDJ(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmdj("TPSSMDJ");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(tpssmdj_in, "%s\\Trace\\tpssmdj.in", p);  //模型计算用输入参数文件
#else
			sprintf(tpssmdj_in, "%s/Trace/tpssmdj.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(tpssmdj_in, "..\\Trace\\tpssmdj.in");  //默认系统运行在UBin目录下
#else
			sprintf(tpssmdj_in, "../Trace/tpssmdj.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(tpssmdj_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT count(1) FROM TPSSMDJ ";
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TPSSMDJ ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmdj);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;路径包\n", (const char *)tpssmdj["ROUTEBAGKEY"].ToString());
			fprintf(outstream, "%s\t\t\t\t;路径\n", (const char *)tpssmdj["ROUTELIST"].ToString());
			fprintf(outstream, "%s\t\t\t\t;默认标记\n", (const char *)tpssmdj["PLANTSECTIONTYPE"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第九块 设路径-设备
/*路径-设备*/
int GenTpsIn_TPSSMD7(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmd7("TPSSMD7");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(tpssmd7_in, "%s\\Trace\\tpssmd7.in", p);  //模型计算用输入参数文件
#else
			sprintf(tpssmd7_in, "%s/Trace/tpssmd7.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(tpssmd7_in, "..\\Trace\\tpssmd7.in");  //默认系统运行在UBin目录下
#else
			sprintf(tpssmd7_in, "../Trace/tpssmd7.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(tpssmd7_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT count(1) FROM TPSSMD7 ";
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TPSSMD7 ";
		sqlstr += CString(" ORDER BY ROUTELIST,CHARGE_NO ");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd7);
			if (tpssmd7["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd7["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd7["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd7["DEV_TECH_CODE"] = "B";
			}
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;厂别\n", (const char *)tpssmd7["FACTORY_DIV"].ToString());
			fprintf(outstream, "%s\t\t\t\t;路径\n", (const char *)tpssmd7["ROUTELIST"].ToString());
			fprintf(outstream, "%s\t\t\t\t;路径顺序\n", (const char *)tpssmd7["CHARGE_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;区域代码\n", (const char *)tpssmd7["AREA_ID"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备工艺代码\n", (const char *)tpssmd7["DEV_TECH_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;预溶液代码\n", (const char *)tpssmd7["PRE_SOLUTION_FLAG"].ToString());
			fprintf(outstream, "%s\t\t\t\t;扒渣标记\n", (const char *)tpssmd7["FLAG_POS_1"].ToString());
			fprintf(outstream, "%s\t\t\t\t;分包标记\n", (const char *)tpssmd7["FLAG_POS_2"].ToString());
			fprintf(outstream, "%s\t\t\t\t;等待标记\n", (const char *)tpssmd7["FLAG_POS_3"].ToString());
			fprintf(outstream, "%s\t\t\t\t;默认标记\n", (const char *)tpssmd7["FLAG_POS_4"].ToString());
			fprintf(outstream, "%s\t\t\t\t;默认标记\n", (const char *)tpssmd7["FLAG_POS_5"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第十块 设备处理时间
/*设备处理时间*/
int GenTpsIn_TPSSMD3(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmd3("TPSSMD3");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(tpssmd3_in, "%s\\Trace\\tpssmd3.in", p);  //模型计算用输入参数文件
#else
			sprintf(tpssmd3_in, "%s/Trace/tpssmd3.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(tpssmd3_in, "..\\Trace\\tpssmd3.in");  //默认系统运行在UBin目录下
#else
			sprintf(tpssmd3_in, "../Trace/tpssmd3.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(tpssmd3_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT count(1) FROM ( "
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,"
			" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
			" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE "
			" UNION"
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
			" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) "
			" )";
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr =
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,"
			" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
			" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE "
			" UNION"
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
			" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssmd3);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;厂别\n", (const char *)tpssmd3["FACTORY_DIV"].ToString());
			fprintf(outstream, "%s\t\t\t\t;钢种\n", (const char *)tpssmd3["ST_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;设备号\n", (const char *)tpssmd3["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;处理时间\n", (const char *)tpssmd3["STD_PROC_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;准备时间\n", (const char *)tpssmd3["STD_PREP_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;扒渣时间\n", (const char *)tpssmd3["DRAW_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;等待时间\n", (const char *)tpssmd3["WAITING_TIME"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第十一块 预计划
/*预计划*/
int GenTpsIn_TPSSM10(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssm10("TPSSM10");
	//CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(tpssm10_in, "%s\\Trace\\tpssm10.in", p);  //模型计算用输入参数文件
#else
			sprintf(tpssm10_in, "%s/Trace/tpssm10.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(tpssm10_in, "..\\Trace\\tpssm10.in");  //默认系统运行在UBin目录下
#else
			sprintf(tpssm10_in, "../Trace/tpssm10.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(tpssm10_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " SELECT count(1) FROM TPSSM10 ";
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT * FROM TPSSM10 ORDER BY CAST_LOT_NO,CAST_LOT_DIV_NO ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm10);
			fetchRowCount++;

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;厂别\n", (const char *)tpssm10["FACTORY_DIV"].ToString());
			fprintf(outstream, "%s\t\t\t\t;钢种\n", (const char *)tpssm10["ST_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;制造命令号\n", (const char *)tpssm10["PONO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;铸机\n", (const char *)tpssm10["CC_MACH_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;预定浇次号\n", (const char *)tpssm10["CAST_LOT_NO"].ToString());
			fprintf(outstream, "%s\t\t\t\t;预定浇注时间\n", (const char *)tpssm10["CC_REQ_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;碳锈区分\n", (const char *)tpssm10["C_DIV"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;处理时间\n", (const char *)tpssm10["STD_PROC_TIME"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;准备时间\n", (const char *)tpssm10["STD_PREP_TIME"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;扒渣时间\n", (const char *)tpssm10["DRAW_TIME"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;等待时间\n", (const char *)tpssm10["WAITING_TIME"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

//增加第十二块 预浇次间隔时间
/*浇次时间*/
int GenTpsIn_CAST_LOT_TIME(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssm11_1("TPSSM11");
	CModel tpssm11_2("TPSSM11");
	CModel tpssm10("TPSSM10");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CString sqlstr;
	CString dev_tab = "";
	CString cast_lot_no1 = "";
	CString cast_lot_no2 = "";
	CString in_flag1 = "";
	CString in_flag2 = "";
	CDecimal slab_width1 = 0, slab_width2 = 0;
	CDecimal slab_thick1 = 0, slab_thick2 = 0;
	CString c_div1,c_div2 = "";
	CString st_no1,st_no2 = "";
	CString pono1, pono2 = "";
	CString device_status = "";
	CString cc_no = "0";
	CDecimal cc_perp_time = 0;
	CDbCommand cmd_tapb08_inq(conn);
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(CAST_LOT_TIME_in, "%s\\Trace\\CAST_LOT_TIME.in", p);  //模型计算用输入参数文件
#else
			sprintf(CAST_LOT_TIME_in, "%s/Trace/CAST_LOT_TIME.in", p);  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			sprintf(CAST_LOT_TIME_in, "..\\Trace\\CAST_LOT_TIME.in");  //默认系统运行在UBin目录下
#else
			sprintf(CAST_LOT_TIME_in, "../Trace/CAST_LOT_TIME.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(CAST_LOT_TIME_in, "w");
		setbuf(outstream, NULL);

		sqlstr = " select count (distinct CAST_LOT_NO) from tpssm10 ";
		cmd_inq.SetCommandText(sqlstr);
		dev_count = cmd_inq.ExecuteScalar().ToInt32();
		dev_count = dev_count * (dev_count - 1);
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);

		sqlstr = " SELECT distinct CAST_LOT_NO FROM tpssm10 order by CAST_LOT_NO ";
		//sqlstr += CString(" ORDER BY AREA_ID,DEV_TECH_CODE,STATION_NO");
		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
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

				fetchRowCount++;

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

				fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
				fprintf(outstream, "%s\t\t\t\t;起点浇次\n", (const char *)cast_lot_no1);
				fprintf(outstream, "%s\t\t\t\t;传搁时间\n", (const char *)cc_perp_time.ToString());
				fprintf(outstream, "%s\t\t\t\t;终点浇次\n", (const char *)cast_lot_no2);
				//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}],[{2}],[{3}]", cc_perp_time.ToInt32(), device_status, cast_lot_no1, cast_lot_no2);

			}
			cmd_inq2.Close();

			cmd_tpssm10_inq.Close();
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code end  ");
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
	fclose(outstream);
	return doFlag;

}

int GenTpsIn_DEV_LAST_TIME(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	int Count = 0;
	FILE * outstream;

	CDbCommand cmd_inq(conn);
	CString sqlstr;
	CString dev_code2 = "";
	CString sm_plan_no = "";
	CString pono = "";
	CString cast_no = "";
	CString dev_code = "";
	CString time = "";
	try
	{

		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_code begin");
		//sprintf(equipmentInfo_in,   "%s/Trace/EquipmentInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			//sprintf(DevLastTime, "%s\\Trace\\DevLastTime.in", p);  //模型计算用输入参数文件
			sprintf(DevLastTime, "%s\\Trace\\%s.np", p, (const char *)("b" + sys_time));
#else
			//sprintf(DevLastTime, "%s/Trace/DevLastTime.in", p);  //模型计算用输入参数文件
			sprintf(DevLastTime, "%s/Trace/%s.np", p, (const char *)("b" + sys_time));  //模型计算用输入参数文件
#endif

		}
		else
		{
#if WIN32
			//(DevLastTime, "..\\Trace\\DevLastTime.in");  //默认系统运行在UBin目录下
			sprintf(DevLastTime, "..\\Trace\\%s.np", p, (const char *)("b" + sys_time));
#else
			//sprintf(DevLastTime, "../Trace/DevLastTime.in");  //默认系统运行在UBin目录下
			sprintf(DevLastTime, "../Trace/%s.np", p, (const char *)("b" + sys_time));
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentInfo_in);
		outstream = fopen(DevLastTime, "w");
		setbuf(outstream, NULL);

		// = " SELECT COUNT(*) FROM (SELECT MAX(a.END_TIME) AS TIME ,a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO FROM tpssm12 a,tpssm11 b WHERE a.SM_PLAN_NO = b.SM_PLAN_NO GROUP BY a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO ) ORDER BY DEV_CODE,TIME desc  ";
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		//dev_count = cmd_inq.ExecuteScalar().ToInt32();
		//fprintf(outstream, "%d\t\t\t\t;数据数量\n", dev_count);



		sqlstr = " SELECT * FROM (SELECT MAX(a.END_TIME) AS TIME ,a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO FROM tpssm12 a,tpssm11 b WHERE a.SM_PLAN_NO = b.SM_PLAN_NO GROUP BY a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO ) ORDER BY DEV_CODE,TIME desc  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();

		while (cmd_inq.Read())
		{
			if (dev_code2 != cmd_inq.GetString(2))
			{
				Count++;
				dev_code2 = cmd_inq.GetString(2);
			}
		}
		//CString dev_code2 = " ";
		fprintf(outstream, "%d\t\t\t\t;数据数量\n", Count);
		dev_code2 = "";

		sqlstr = " SELECT * FROM (SELECT MAX(a.END_TIME) AS TIME ,a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO FROM tpssm12 a,tpssm11 b WHERE a.SM_PLAN_NO = b.SM_PLAN_NO GROUP BY a.DEV_CODE,a.SM_PLAN_NO,b.PONO,b.CAST_NO ) ORDER BY DEV_CODE,TIME desc  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			if (dev_code2 != cmd_inq.GetString(2))
			{
				fetchRowCount++;

				sm_plan_no = cmd_inq.GetString(3);
				pono = cmd_inq.GetString(4);
				time = sys_time;
				dev_code = cmd_inq.GetString(2);
				cast_no = cmd_inq.GetString(5);

				fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
				fprintf(outstream, "%s\t\t\t\t;设备\n", (const char *)dev_code);
				fprintf(outstream, "%s\t\t\t\t;最晚时间\n", (const char *)time);
				dev_code2 = cmd_inq.GetString(2);
			}
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
	fclose(outstream);
	return doFlag;

}

//第二块，传搁时间
//传搁时间
/************************************************************
设备传搁时间IN文件输出
注:
1起始设备代码
2起始设备类型
3终点设备代码
4终点设备类型
5传搁时间
************************************************************/
int GenTpsIn_device_trantime(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmd6("TPSSMD6");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_tpssmd6_inq(conn);
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD1_RESULT;
	EIClass TPSSMD1_CONDI;

	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_ID");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_NO");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	try
	{
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_trantime begin");
		//sprintf(trantime_in,   "%s/Trace/trantime.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(trantime_in, "%s\\Trace\\trantime.in", p);  //模型计算用输入参数文件
#else
			sprintf(trantime_in, "%s/Trace/trantime.in", p);  //模型计算用输入参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(trantime_in, "..\\Trace\\trantime.in");  //默认系统运行在UBin目录下
#else
			sprintf(trantime_in, "../Trace/trantime.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", trantime_in);
		outstream = fopen(trantime_in, "w");
		setbuf(outstream, NULL);

		sqlstr = "SELECT * FROM TPSSMD1 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);
		cmd_inq.Close();

		tpssmd6["FACTORY_DIV"] = v_factory_div;

		dev_count = tpssmd6.QueryCount("FACTORY_DIV");
		fprintf(outstream, "%d\t\t\t\t;个数\n", dev_count);
		sqlstr =    
			" SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,"
					" COALESCE(T2.MOVE_TIME, T1.MOVE_TIME) AS MOVE_TIME FROM TPSSMD6 t1 LEFT JOIN TPSSMD6_X t2"
					" ON  t1.DEV_MOVE_START = t2.DEV_MOVE_START AND  t1.DEV_MOVE_END = t2.DEV_MOVE_END"
					" WHERE t1.FACTORY_DIV=@v_factory_div "
					" UNION SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,t1.MOVE_TIME"
					" FROM TPSSMD6_X t1   WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD6) AND  t1.FACTORY_DIV = @v_factory_div ";
		cmd_tpssmd6_inq.SetCommandText(sqlstr);
		cmd_tpssmd6_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssmd6_inq.ExecuteReader();
		while (cmd_tpssmd6_inq.Read())
		{
			cmd_tpssmd6_inq.Fetch(tpssmd6);
			fetchRowCount++;
			/*tpssmd1["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			tpssmd1["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			tpssmd1["FACTORY_DIV"] = v_factory_div;

			tpssmd1.Query("STATION_ID,STATION_NO,FACTORY_DIV");*/

			TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			TPSSMD1_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
			tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd1["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd1["DEV_TECH_CODE"] = "B";
			}

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			//fprintf(outstream, "%s\t\t\t\t;起点设备代码\n", (const char *)tpssmd6["DEV_MOVE_START"].ToString());
			fprintf(outstream, "%s\t\t\t\t;起点设备代码\n", (const char *)tpssmd1["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;起点设备类型\n", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
			fprintf(outstream, "%d\t\t\t\t;传搁时间\n", tpssmd6["MOVE_TIME"].ToDecimal().ToInt32());
			fprintf(outstream, "%s\t\t\t\t;交叉物流/直线物流\n", (const char *)tpssmd6["TRAN_TYPE"].ToString()); //0-交叉 1-直线

			/*tpssmd1["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			tpssmd1["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);

			tpssmd1.Query("STATION_ID,STATION_NO,FACTORY_DIV");*/

			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);
			TPSSMD1_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
			tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd1["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd1["DEV_TECH_CODE"] = "B";
			}

			//fprintf(outstream, "%s\t\t\t\t;终点设备代码\n", (const char *)tpssmd6["DEV_MOVE_END"].ToString());
			fprintf(outstream, "%s\t\t\t\t;终点设备代码\n", (const char *)tpssmd1["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;终点设备类型\n\n", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
		}
		cmd_tpssmd6_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_trantime end  ");
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
	cmd_tpssmd6_inq.Close();
	fclose(outstream);
	return doFlag;

}

int GenTpsIn_device_trantimeb(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int dev_count = 0;
	int doFlag = 0;
	int fetchRowCount = 0;
	FILE * outstream;
	CModel tpssmd6("TPSSMD6");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_tpssmd6_inq(conn);
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD1_RESULT;
	EIClass TPSSMD1_CONDI;

	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_ID");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_NO");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	try
	{
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_trantime begin");
		//sprintf(trantime_in,   "%s/Trace/trantime.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(trantime_in, "%s\\Trace\\btrantime.in", p);  //模型计算用输入参数文件
#else
			sprintf(trantime_in, "%s/Trace/btrantime.in", p);  //模型计算用输入参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(trantime_in, "..\\Trace\\btrantime.in");  //默认系统运行在UBin目录下
#else
			sprintf(trantime_in, "../Trace/btrantime.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", trantime_in);
		outstream = fopen(trantime_in, "w");
		setbuf(outstream, NULL);

		sqlstr = "SELECT * FROM TPSSMD1 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);
		cmd_inq.Close();

		tpssmd6["FACTORY_DIV"] = v_factory_div;

		dev_count = tpssmd6.QueryCount("FACTORY_DIV");
		fprintf(outstream, "%d\t\t\t\t;个数\n", dev_count);
		sqlstr =  " SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,"
					" COALESCE(T2.MOVE_TIME, T1.MOVE_TIME) AS MOVE_TIME FROM TPSSMD6 t1 LEFT JOIN TPSSMD6_X t2"
					" ON  t1.DEV_MOVE_START = t2.DEV_MOVE_START AND  t1.DEV_MOVE_END = t2.DEV_MOVE_END"
					" WHERE t1.FACTORY_DIV=@v_factory_div "
					" UNION  SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,t1.MOVE_TIME"
					" FROM TPSSMD6_X t1   WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD6) AND  t1.FACTORY_DIV=@v_factory_div ";
		cmd_tpssmd6_inq.SetCommandText(sqlstr);
		cmd_tpssmd6_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_tpssmd6_inq.ExecuteReader();
		while (cmd_tpssmd6_inq.Read())
		{
			cmd_tpssmd6_inq.Fetch(tpssmd6);
			fetchRowCount++;
			/*tpssmd1["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			tpssmd1["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			tpssmd1["FACTORY_DIV"] = v_factory_div;

			tpssmd1.Query("STATION_ID,STATION_NO,FACTORY_DIV");*/

			TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(0, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd6["DEV_MOVE_START"].ToString().Substring(1, 1);
			TPSSMD1_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
			tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd1["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd1["DEV_TECH_CODE"] = "B";
			}

			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			//fprintf(outstream, "%s\t\t\t\t;起点设备代码\n", (const char *)tpssmd6["DEV_MOVE_START"].ToString());
			fprintf(outstream, "%s\t\t\t\t;起点设备代码\n", (const char *)tpssmd1["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;起点设备类型\n", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
			fprintf(outstream, "%d\t\t\t\t;传搁时间\n", tpssmd6["MOVE_TIME"].ToDecimal().ToInt32());
			fprintf(outstream, "%s\t\t\t\t;交叉物流/直线物流\n", (const char *)tpssmd6["TRAN_TYPE"].ToString()); //0-交叉 1-直线

			/*tpssmd1["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			tpssmd1["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);

			tpssmd1.Query("STATION_ID,STATION_NO,FACTORY_DIV");*/

			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(0, 1);
			TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd6["DEV_MOVE_END"].ToString().Substring(1, 1);
			TPSSMD1_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
			tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);

			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				tpssmd1["DEV_TECH_CODE"] = "E";
			}
			if (tpssmd1["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				tpssmd1["DEV_TECH_CODE"] = "B";
			}

			//fprintf(outstream, "%s\t\t\t\t;终点设备代码\n", (const char *)tpssmd6["DEV_MOVE_END"].ToString());
			fprintf(outstream, "%s\t\t\t\t;终点设备代码\n", (const char *)tpssmd1["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;终点设备类型\n\n", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
		}
		cmd_tpssmd6_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_trantime end  ");
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
	cmd_tpssmd6_inq.Close();
	fclose(outstream);
	return doFlag;

}
//第三块，设备定修
//设备定修
/************************************************************
设备辅助时间IN文件输出
注:
新增TPSS M18表 和画面
1			;设备状态数量，以下循环开始
1			;序号，从1开始索引
A			;设备代码
2			;状态代码，0表示完全可用，1表示不可用，2表示部分可用
20090311070700;状态开始时间
20090311130700	;状态结束时间

************************************************************/
int GenTpsIn_device_StateInfo(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int fetchRowCount = 0;
	int dev_count = 0;
	int doFlag = 0;
	FILE * outstream;
	CModel tpssm18("TPSSM18");
	CModel tpssm19("TPSSM19");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	try
	{
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_StateInfo begin");
		//sprintf(equipmentStateInfo_in,   "%s/Trace/EquipmentStateInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(equipmentStateInfo_in, "%s\\Trace\\EquipmentStateInfo.in", p);  //模型计算用输入参数文件
#else
			sprintf(equipmentStateInfo_in, "%s/Trace/EquipmentStateInfo.in", p);  //模型计算用输入参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(equipmentStateInfo_in, "..\\Trace\\EquipmentStateInfo.in");  //默认系统运行在UBin目录下
#else
			sprintf(equipmentStateInfo_in, "../Trace/EquipmentStateInfo.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentStateInfo_in);
		outstream = fopen(equipmentStateInfo_in, "w");

		setbuf(outstream, NULL);

		tpssm18["DEV_STATUS"] = "1";
		dev_count = tpssm18.QueryCount("DEV_STATUS");
		fprintf(outstream, "%d\t\t\t\t;设备状态数量\n", dev_count);

		sqlstr = "SELECT * FROM TPSSM18 WHERE DEV_STATUS='1' AND END_TIME > TO_CHAR(SYSDATE, 'YYYYMMDDHH24MISS')  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm18);
			if (tpssm18["STOP_FLAG"].ToString() == "0")//数据库表里0表示不可用，1表示部分   模型的话0,表示可用，1完全不可用，2部分可用
			{
				tpssm18["STOP_FLAG"] = "1";
			}
			else
			{
				tpssm18["STOP_FLAG"] = "2";
			}
			if (tpssm18["DEV_CODE"].ToString().Substring(0, 1) == "F" && tpssm18["DEV_STATUS_REMARK"].ToString().Trim() == "检修")
			{
				tpssm18["STOP_FLAG"] = "1";
			}
			//tpssm18["STOP_FLAG"] = "1";
			tpssm19["DEV_STATUS_REMARK"] = tpssm18["DEV_STATUS_REMARK"].ToString().Trim();
			tpssm19["AREA_ID"] = tpssm18["AREA_ID"].ToString().Trim();
			tpssm19["FACTORY_DIV"] = tpssm18["FACTORY_DIV"].ToString().Trim();
			tpssm19["DEV_TECH_CODE"] = tpssm18["DEV_CODE"].ToString().Trim().Substring(0,1);
			tpssm19.Query("DEV_STATUS_REMARK,AREA_ID,FACTORY_DIV,DEV_TECH_CODE"); 


			fetchRowCount++;
			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;设备代码\n", (const char *)tpssm18["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;状态代码\n", (const char *)tpssm18["STOP_FLAG"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;时长\n", (const char *)tpssm19["WORK_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;状态开始时间\n", (const char *)tpssm18["START_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;状态结束时间\n", (const char *)tpssm18["END_TIME"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_StateInfo end  ");
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
	fclose(outstream);
	return doFlag;
}

int GenTpsIn_device_StateInfob(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int fetchRowCount = 0;
	int dev_count = 0;
	int doFlag = 0;
	FILE * outstream;
	CModel tpssm18("TPSSM18");
	CModel tpssm19("TPSSM19");
	CDbCommand cmd_inq(conn);
	CString sqlstr;
	try
	{
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_StateInfo begin");
		//sprintf(equipmentStateInfo_in,   "%s/Trace/EquipmentStateInfo.in",  getenv("HOME"));  //模型计算用输入参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(equipmentStateInfo_in, "%s\\Trace\\bEquipmentStateInfo.in", p);  //模型计算用输入参数文件
#else
			sprintf(equipmentStateInfo_in, "%s/Trace/bEquipmentStateInfo.in", p);  //模型计算用输入参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(equipmentStateInfo_in, "..\\Trace\\bEquipmentStateInfo.in");  //默认系统运行在UBin目录下
#else
			sprintf(equipmentStateInfo_in, "../Trace/bEquipmentStateInfo.in");  //默认系统运行在UBin目录下
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		////Log::Trace("", __FUNCTION__,"	file_name_in=[{0}]\n", equipmentStateInfo_in);
		outstream = fopen(equipmentStateInfo_in, "w");

		setbuf(outstream, NULL);

		tpssm18["DEV_STATUS"] = "1";
		dev_count = tpssm18.QueryCount("DEV_STATUS");
		fprintf(outstream, "%d\t\t\t\t;设备状态数量\n", dev_count);

		sqlstr = "SELECT * FROM TPSSM18 WHERE DEV_STATUS='1' AND END_TIME > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tpssm18);
			if (tpssm18["STOP_FLAG"].ToString() == "0")//数据库表里0表示不可用，1表示部分   模型的话0,表示可用，1完全不可用，2部分可用
			{
				tpssm18["STOP_FLAG"] = "1";
			}
			else
			{
				tpssm18["STOP_FLAG"] = "2";
			}
			if (tpssm18["DEV_CODE"].ToString().Substring(0, 1) == "F" && tpssm18["DEV_STATUS_REMARK"].ToString().Trim() == "检修")
			{
				tpssm18["STOP_FLAG"] = "1";
			}
			//tpssm18["STOP_FLAG"] = "1";
			tpssm19["DEV_STATUS_REMARK"] = tpssm18["DEV_STATUS_REMARK"].ToString().Trim();
			tpssm19["AREA_ID"] = tpssm18["AREA_ID"].ToString().Trim();
			tpssm19["FACTORY_DIV"] = tpssm18["FACTORY_DIV"].ToString().Trim();
			tpssm19["DEV_TECH_CODE"] = tpssm18["DEV_CODE"].ToString().Trim().Substring(0, 1);
			tpssm19.Query("DEV_STATUS_REMARK,AREA_ID,FACTORY_DIV,DEV_TECH_CODE");


			fetchRowCount++;
			fprintf(outstream, "%d\t\t\t\t;序号\n", fetchRowCount);
			fprintf(outstream, "%s\t\t\t\t;设备代码\n", (const char *)tpssm18["DEV_CODE"].ToString());
			fprintf(outstream, "%s\t\t\t\t;状态代码\n", (const char *)tpssm18["STOP_FLAG"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;时长\n", (const char *)tpssm19["WORK_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;状态开始时间\n", (const char *)tpssm18["START_TIME"].ToString());
			fprintf(outstream, "%s\t\t\t\t;状态结束时间\n", (const char *)tpssm18["END_TIME"].ToString());
		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_device_StateInfo end  ");
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
	fclose(outstream);
	return doFlag;
}
//炉次路径配置
/************************************************************
炉次路径配置IN文件输出
注:
新增TPSS M18表 和画面
PONO号	  路径类别	工位1	工位2	工位…
A6400010     1        F1      0       0
A6400012     0        0      F1       0
************************************************************/
int GenTpsIn_llc(CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	FILE * outstream;
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_12(conn);
	CString sqlstr;
	CString v_pono = "", v_heat_no = "";

	try
	{
		////Log::Trace("", __FUNCTION__, "GenTpsIn_llc begin");
		sprintf(plan_llc_in, "%s/Trace/%s.llc", getenv("BM2_BUILD_DIR"), (const char *)sys_time);
		////Log::Trace("", __FUNCTION__, "	file_name_in=[{0}]\n", plan_llc_in);
		outstream = fopen(plan_llc_in, "w");

		setbuf(outstream, NULL);

		sqlstr = "SELECT PONO,HEAT_NO FROM TPSSM11 WHERE SMELT_MODE='1' and PONO_STATUS<83  ORDER BY CAST_NO,CAST_DIV_NO  ASC";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			v_pono = cmd_inq.GetString(1);
			v_heat_no = cmd_inq.GetString(2);

			fprintf(outstream, "%s\t\%s\t\%s\t%s", (const char *)v_pono, "1", "0", "F1");

			sqlstr = "SELECT dev_code FROM TPSSM12 WHERE HEAT_NO=@v_heat_no and area_id >2 and area_id <5 ORDER BY charge_no  ASC";
			cmd_inq_12.SetCommandText(sqlstr);
			cmd_inq_12.Parameters.Set("v_heat_no", v_heat_no);
			cmd_inq_12.ExecuteReader();
			while (cmd_inq_12.Read())
			{
				fprintf(outstream, "\t%s", "0");
			}
			cmd_inq_12.Close();
			fprintf(outstream, "\n");

		}
		cmd_inq.Close();
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__, "GenTpsIn_llc end  ");
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
	fclose(outstream);
	return doFlag;
}

//第四块，计划相关
/************************************************************
炉次条件文件输出
注:
1.模型中第一个工序必须是脱硫，但其只有4个数据项：准备时间、处理时间、开始时刻、结束时刻，并且都为空
2.其他工序则有5个数据项：准备时间、处理时间、前工序至本工序移动时间、开始时刻、结束时刻
************************************************************/
int GenTpsIn_output_pono_condition(int mode, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int	index_11;
	int	index_12;
	int	dummy = 0;
	char	route_relaion[30];
	char	pono_route[30];
	char	route_div[30];
	char	dev_assign[30];  //设备指定：0-人工指定; 1-模型推荐
	FILE * outstream;

	int doFlag = 0;
	int count = 0;
	int count1 = 0;
	CString station_id = "";
	CString station_no = "";
	CDecimal slab_width = 0, slab_width_pre = 0;
	CDecimal slab_thick = 0, slab_thick_pre = 0;
	CDecimal v_charge_no = 0;
	CString device_status = "";
	CString c_div_pre = "";
	CString st_no_pre = "";
	CDecimal cc_perp_time = 0;

	CModel tpssmd3("TPSSMD3");
	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd6("TPSSMD6");
	//CTPSSMD5 tpssmd5(conn);
	CModel tpssmd9("TPSSMD9");
	CModel tpssm11_pre("TPSSM11");
	CModel tpssm41("TPSSM41");
	CModel tpssmdd("tpssmdd");
	CModel tapbd008s2n("TAPBD008S2N");
	CDbCommand cmd_inq(conn);
	CDataTable tb_tpssm11("TPSSM11");
	CDataTable tb_tpssm12("TPSSM12");

	CString sqlstr = "";
	CDbCommand cmd_tpssmd4_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tapb08_inq(conn);

	EIClass TPSSM10_SOURCE;
	EIClass TPSSMD9_SOURCE;
	EIClass TPSSMD6_SOURCE;
	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD3_SOURCE;
	EIClass TAPBD008S2N_SOURCE;

	EIClass TPSSM10_CONDI;
	EIClass TPSSMD9_CONDI;
	EIClass TPSSMD6_CONDI;
	EIClass TPSSMD1_CONDI;
	EIClass TPSSMD3_CONDI;
	EIClass TAPBD008S2N_CONDI;

	TPSSM10_CONDI.Tables[0].Columns.Add(DT_STRING, "PONO");
	TPSSM10_CONDI.Tables[0].Rows.Add();

	TPSSMD9_CONDI.Tables[0].Columns.Add(DT_STRING, "CAST_THICK");
	TPSSMD9_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD9_CONDI.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
	TPSSMD9_CONDI.Tables[0].Rows.Add();

	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "AREA_ID");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	TPSSMD6_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_MOVE_START");
	TPSSMD6_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD6_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_MOVE_END");
	TPSSMD6_CONDI.Tables[0].Rows.Add();

	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "ST_NO");
	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "SMELT_MODE");
	TPSSMD3_CONDI.Tables[0].Rows.Add();

	EIClass TPSSM10_RESULT;
	EIClass TPSSMD9_RESULT;
	EIClass TPSSMD6_RESULT;
	EIClass TPSSMD1_RESULT;
	EIClass TPSSMD3_RESULT;
	EIClass TAPBD008S2N_RESULT;

	try
	{
		Log::Trace("", __FUNCTION__,"GenTpsIn_output_pono_condition begin");

		//sprintf(file_name_in,   "%s/Trace/%s.in",  getenv("HOME"), (const char *)sys_time);  //模型计算用输入参数文件
		//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("HOME"), (const char *)sys_time);  //模型计算输出参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(file_name_in, "%s\\Trace\\%s.in", p, (const char *)sys_time);  //模型计算用输入参数文件
			sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)sys_time);  //模型计算输出参数文件
#else
			sprintf(file_name_in, "%s/Trace/%s.in", p, (const char *)sys_time);  //模型计算用输入参数文件
			sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)sys_time);  //模型计算输出参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(file_name_in, "..\\Trace\\%s.in", (const char *)sys_time);  //模型计算用输入参数文件
			sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)sys_time);  //模型计算输出参数文件
#else
			sprintf(file_name_in, "../Trace/%s.in", (const char *)sys_time);  //模型计算用输入参数文件
			sprintf(file_name_out, "../Trace/%s.out", (const char *)sys_time);  //模型计算输出参数文件
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		Log::Trace("", __FUNCTION__,"	file_name_in=[{0}],file_name_out=[{2}]\n", file_name_in, file_name_out);
		outstream = fopen(file_name_in, "w");
		setbuf(outstream, NULL);

		////相关数据准备
		sqlstr = "SELECT * FROM TPSSM10 WHERE FACTORY_DIV=@v_factory_div AND PONO IN ( SELECT a.PONO FROM TPSSM11 a,TPSSM12 b WHERE a.STEEL_RETURN_CODE = ' ' AND a.PONO_STATUS < 83  AND a.STEEL_RETURN_CODE = ' ' AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 3 AND (b.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.END_TIME_REAL = ' ' )) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSM10_SOURCE.Tables[0]);

		sqlstr = "SELECT * FROM TPSSMD9 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD9_SOURCE.Tables[0]);

		sqlstr =  " SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,"
					" COALESCE(T2.MOVE_TIME, T1.MOVE_TIME) AS MOVE_TIME FROM TPSSMD6 t1 LEFT JOIN TPSSMD6_X t2"
					" ON  t1.DEV_MOVE_START = t2.DEV_MOVE_START AND  t1.DEV_MOVE_END = t2.DEV_MOVE_END"
					" WHERE t1.FACTORY_DIV=@v_factory_div "
					" UNION  SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,T1.MOVE_TIME"
					" FROM TPSSMD6_X t1   WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD6) AND  t1.FACTORY_DIV=@v_factory_div ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD6_SOURCE.Tables[0]);

		sqlstr = "SELECT * FROM TPSSMD1 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);

		sqlstr = 
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,"
			" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
			" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE "
			" UNION"
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
			" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD3_SOURCE.Tables[0]);

		sqlstr = "SELECT * FROM TAPBD008S2N ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TAPBD008S2N_SOURCE.Tables[0]);

		//数据准备
		sqlstr = "SELECT a.* FROM TPSSM11 a,TPSSM12 b WHERE a.FACTORY_DIV=@v_factory_div AND a.PONO_STATUS < 83 AND a.STEEL_RETURN_CODE = ' ' AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 3 AND (b.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.END_TIME_REAL = ' ' ) ";
		sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(tb_tpssm11);
		//校验可编计划数如果小于0,报警提示
		if (tb_tpssm11.Rows.get_Count() <= 0)
		{
			EDLog(1, 1, "计划数量小于1,不需优化。");
			strcpy(s.msg, _RES("计划数量小于1,不需优化。")/*PSSMS0000213计划数量小于1,不需优化。*/);
			throw CApplicationException(-1, s.msg, log.Location);
			//CFormattable arguments[] = { tb_tpssm11.Rows.get_Count() }; // 定义参数列表的数组
			//CMessageFormat::Format(s.msg, _RES("计划数量小于1,不需优化。"), arguments, 1); //格式化字符串
			//throw CApplicationException(-1, s.msg, log.Location);
		}

		fprintf(outstream, "%d\t\t\t\t;可替换设备数\n", 0);
		fprintf(outstream, "%d\t\t\t\t;PONO数\n", tb_tpssm11.Rows.get_Count());//浇注开始的为第一炉，指定的计划数量
		/*--------------------------------------------------
		PONO信息
		--------------------------------------------------*/
		//Log::Trace("", __FUNCTION__, "tpssm11 begin");
		for (index_11 = 0; index_11< tb_tpssm11.Rows.get_Count(); index_11++)
		{
			tpssm11.MergeFrom(tb_tpssm11.Rows[index_11]);

			fprintf(outstream, "%d\t\t\t\t;序号\n", index_11 + 1);
			fprintf(outstream, "%s\t\t\t\t;PONO\n", (const char *)tpssm11["PONO"].ToString());

			//根据条件内容，生成"路径","路径关联关系","设备区分"
			memset(pono_route, 0, 30);
			memset(route_relaion, 0, 30);
			memset(route_div, 0, 30);
			GenTpsIn_route_create((const char *)tpssm11["PONO"].ToString(), pono_route, route_relaion, route_div, conn);

			fprintf(outstream, "%s\t\t\t\t;路径设备区分\n", pono_route);
			fprintf(outstream, "%s\t\t\t\t;路径关联关系\n", route_relaion);
			fprintf(outstream, "%s\t\t\t\t;设备类型区分\n", route_div);

			if (tpssm11["CC_REQ_TIME"].ToString()[0] == ' ')
			{
				tpssm11["CC_REQ_TIME"] = "00000000000000";
			}
			fprintf(outstream, "%s\t\t\t;CC要求时刻\n", (const char *)tpssm11["CC_REQ_TIME"].ToString());
			////增加辅助作业时间
			//tpssmdd["JOB_CODE"] = tpssm11["BOF_ASSIST_OPT"].ToString();
			//tpssmdd.Query();
			//fprintf(outstream, "%s\t\t\t;当前炉辅助作业代码\n", (const char *)tpssmdd["JOB_CODE"].ToString().Trim());
			//fprintf(outstream, "%d\t\t\t\t;当前炉辅助作业时间\n", tpssmdd["STD_PROC_TIME"].ToDecimal().ToInt32());

			fprintf(outstream, "%s\t\t\t\t;浇次号\n", (const char *)tpssm11["CAST_NO"].ToString());
			fprintf(outstream, "%d\t\t\t\t;浇次分割号\n", tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32());
			//fprintf(outstream, "%s\t\t\t\t;工艺路径包\n", (const char *)tpssm11["ROUTEBAGKEY"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;工艺路径\n", (const char *)tpssm11["ROUTELIST"].ToString());




			//fprintf(outstream, "%d\t\t\t\t;上炉辅助作业准备时间\n", tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32());

			//"正在处理的工位"和"炉次状态"决定了实绩点，告知模型该点之前的数据不必计算
			//模型暂时不读这2个字段
			fprintf(outstream, "%02d\t\t\t\t;正在处理的工位\n", 0);
			fprintf(outstream, "%02d\t\t\t\t;炉次状态      \n", 0);
			/*--------------------------------------------------
			CHARGE信息
			--------------------------------------------------*/
			fprintf(outstream, "%d\t\t\t\t;工序%d\n", 0,0);
			fprintf(outstream, "%d\t\t\t\t;工序0准备时间\n", 0);
			fprintf(outstream, "%d\t\t\t\t;工序0处理时间\n", 0);
			fprintf(outstream, "%s\t\t\t;工序0计划开始时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0计划结束时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0实绩开始时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0实绩结束时刻\n", "00000000000000");

			sqlstr = "SELECT * FROM TPSSM12 WHERE FACTORY_DIV=@v_factory_div ";
			sqlstr += CString(" AND SM_PLAN_NO=@tpssm11.SM_PLAN_NO AND AREA_ID >1 ORDER BY CHARGE_NO ASC");
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_inq.ExecuteQuery(tb_tpssm12);

			device_status = " ";

			TPSSM10_CONDI.Tables[0].Rows[0]["PONO"] = tpssm11["PONO"];
			TPSSM10_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSM10_CONDI, TPSSM10_SOURCE, TPSSM10_RESULT, conn);		
			//PrintDataTable(TPSSM10_RESULT.Tables[0]);
			slab_width = TPSSM10_RESULT.Tables[0].Rows[0]["SLAB_WIDTH"].ToDecimal();
			slab_width = TPSSM10_RESULT.Tables[0].Rows[0]["SLAB_THICK"].ToDecimal();
			tpssm10.MergeFrom(TPSSM10_RESULT.Tables[0].Rows[0]);

			/*tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10.Query("PONO");
			slab_width = tpssm10["SLAB_WIDTH"];
			slab_thick = tpssm10["SLAB_THICK"];*/

			if (tpssm11["RESTRAND_FLG"].ToString() == "T")
			{
				//tpssm10["PONO"] = tpssm11["PONO"];
				//Log::Trace("", __FUNCTION__, "tpssm10.PONO = [{0}]", tpssm10["PONO"].ToString());
				//tpssm10.Query("PONO");

				/*tpssmd9["CAST_THICK"] = tpssm10["SLAB_THICK"];
				tpssmd9["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				tpssmd9["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
				if (tpssmd9.QueryCount("FACTORY_DIV,CC_MACH_NO,CAST_THICK") != 1)
				{
					tpssmd9["TT_PREP_W0_CAST"] = 70;
				}
				else
				{
					tpssmd9.Query("FACTORY_DIV,CC_MACH_NO,CAST_THICK");
				}*/

				TPSSMD9_CONDI.Tables[0].Rows[0]["CAST_THICK"] = tpssm10["SLAB_THICK"];
				TPSSMD9_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
				TPSSMD9_CONDI.Tables[0].Rows[0]["CC_MACH_NO"] = tpssm11["CC_MACH_NO"];
				TPSSMD9_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD9_CONDI, TPSSMD9_SOURCE, TPSSMD9_RESULT, conn);

				if (TPSSMD9_RESULT.Tables[0].Rows.get_Count() != 1)
				{
					tpssmd9["TT_PREP_W0_CAST"] = 70;
				}
				else
				{
					tpssmd9.MergeFrom(TPSSMD9_RESULT.Tables[0].Rows[0]);
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

			//循环查询各工序
			for (index_12 = 0; index_12<tb_tpssm12.Rows.get_Count(); index_12++)
			{
				//Log::Trace("", __FUNCTION__, "tpssm12 begin");
				tpssm12.MergeFrom(tb_tpssm12.Rows[index_12]);
				//Log::Trace("", __FUNCTION__, "tpssm12 begin[{0}]",tpssm12["SM_PLAN_NO"].ToString());
				//v_charge_no = tpssm12["CHARGE_NO"];

				//---- 得到移行时间  --------------
				//查到达侧设备代码
				/*tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");*/

				TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
				TPSSMD1_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm12["DEV_CODE"];
				TPSSMD1_CONDI.Tables[0].Rows[0]["AREA_ID"] = tpssm12["AREA_ID"];
				TPSSMD1_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
				tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);


				//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd1 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
				/*tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
				tpssmd6["DEV_MOVE_START"] = station_id + station_no;
				tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

				if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
				{
					tpssmd6["MOVE_TIME"] = 0;
				}
				else
				{
					tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
				}*/

				if (tpssmd1["STATION_ID"].ToString() == "X")
				{
					tpssmd1["STATION_ID"] = "E";
				}
				else if (tpssmd1["STATION_ID"].ToString() == "Y")
				{
					tpssmd1["STATION_ID"] = "B";
				}

				TPSSMD6_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
				TPSSMD6_CONDI.Tables[0].Rows[0]["DEV_MOVE_START"] = station_id + station_no;
				TPSSMD6_CONDI.Tables[0].Rows[0]["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();
				TPSSMD6_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD6_CONDI, TPSSMD6_SOURCE, TPSSMD6_RESULT, conn);
				if (TPSSMD6_RESULT.Tables[0].Rows.get_Count() != 1)
				{
					tpssmd6["MOVE_TIME"] = 0;
				}
				else
				{
					tpssmd6.MergeFrom(TPSSMD6_RESULT.Tables[0].Rows[0]);
				}
				//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd6 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());

				fprintf(outstream, "%d\t\t\t\t;工序%d\n", tpssm12["CHARGE_NO"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else
				{
					/*tpssmd3["ST_NO"] = tpssm10["ST_NO"];
					tpssmd3["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd3["SMELT_MODE"] = 0;
					if (tpssmd3.QueryCount("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE") == 1)
					{
						tpssmd3.Query("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE");
					}
					else
					{
						tpssmd3["DEV_CODE"] = tpssm12["DEV_CODE"].ToString().Substring(0, 1);
						if (tpssmd3.QueryCount("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE") == 1)
						{
							tpssmd3.Query("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE");
						}
						else
						{
							tpssmd3["DEV_CODE"] = tpssm12["DEV_CODE"];
							if (tpssm10["C_DIV"].ToString() == "1")
							{
								tpssmd3["ST_NO"] = "DEFAULTS";
							}
							else if (tpssm10["C_DIV"].ToString() == "2")
							{
								tpssmd3["ST_NO"] = "DEFAULTC";
							}
							tpssmd3.Query("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE");
						}
					}*/
					TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = tpssm10["ST_NO"];
					TPSSMD3_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm10["FACTORY_DIV"];
					TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm12["DEV_CODE"];
					TPSSMD3_CONDI.Tables[0].Rows[0]["SMELT_MODE"] = 0;
					TPSSMD3_RESULT.Tables[0].Clear();
					f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);
					//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd3 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
					if (TPSSMD3_RESULT.Tables[0].Rows.get_Count() == 1)
					{
						tpssmd3.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);
					}
					else
					{
						TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm12["DEV_CODE"].ToString().Substring(0, 1);
						TPSSMD3_RESULT.Tables[0].Clear();
						f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);
						//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd32 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
						if (TPSSMD3_RESULT.Tables[0].Rows.get_Count() == 1)
						{
							tpssmd3.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);
						}
						else
						{
							TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm12["DEV_CODE"];
							if (tpssm10["C_DIV"].ToString() == "1")
							{
								TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTS";
							}
							else if (tpssm10["C_DIV"].ToString() == "2")
							{
								TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTC";
							}
							else if (tpssm10["C_DIV"].ToString().Trim() == "")
							{
								if (tpssm10["ROUTEBAGKEY"].ToString().Substring(0, 1) == "C")
								{
									TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTC";
								}
								else if (tpssm10["ROUTEBAGKEY"].ToString().Substring(0, 1) == "S")
								{
									TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTS";
								}
							}
							TPSSMD3_RESULT.Tables[0].Clear();
							//PrintDataTable(TPSSMD3_CONDI.Tables[0]);
							f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);
							//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd33 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
							tpssmd3.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);
							//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd33 end[{0}]", tpssm12["SM_PLAN_NO"].ToString());
						}

					}
					

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", tpssmd3["STD_PREP_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				} 

				fprintf(outstream, "%d\t\t\t\t;工序%d处理时间\n", tpssm12["PROC_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				//---- 写入移行时间  --------------
				fprintf(outstream, "%d\t\t\t\t;工序%d至工序%d移动时间 \n", tpssmd6["MOVE_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32() - 1, tpssm12["CHARGE_NO"].ToDecimal().ToInt32());

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
				//---- 读取开始时刻/结束时刻 --------------
				//判断实绩表中的PONO是否存在，新编制的计划在实绩表中是不存在的
				if (tpssm12["START_TIME_REAL"].ToString().Trim() == "") tpssm12["START_TIME_REAL"] = "00000000000000";
				if (tpssm12["START_TIME"].ToString().Trim() == "") tpssm12["START_TIME"] = "00000000000000";
				//实绩结束时刻
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "") tpssm12["END_TIME_REAL"] = "00000000000000";
				if (tpssm12["END_TIME"].ToString().Trim() == "") tpssm12["END_TIME"] = "00000000000000";

				fprintf(outstream, "%s\t\t\t;工序%d计划开始时刻\n", (const char *)tpssm12["START_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d计划结束时刻\n", (const char *)tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d实绩开始时刻\n", (const char *)tpssm12["START_TIME_REAL"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d实绩结束时刻\n", (const char *)tpssm12["END_TIME_REAL"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());

			}
			//Log::Trace("", __FUNCTION__, "tpssm12 tpssm12 END[{0}]", tpssm12["SM_PLAN_NO"].ToString());
			slab_width_pre = slab_width;
			slab_thick_pre = slab_thick;
			c_div_pre = tpssm10["C_DIV"];
			st_no_pre = tpssm10["ST_NO"];
		}
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_output_pono_condition end  ");
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
	cmd_tpssmd4_inq.Close();
	cmd_tpssm03_inq.Close();
	cmd_tpssm11_inq.Close();
	fclose(outstream);
	return doFlag;
}


/************************************************************
炉次条件文件输出
注:
1.模型中第一个工序必须是脱硫，但其只有4个数据项：准备时间、处理时间、开始时刻、结束时刻，并且都为空
2.其他工序则有5个数据项：准备时间、处理时间、前工序至本工序移动时间、开始时刻、结束时刻
************************************************************/
int GenTpsIn_output_pono_condition_charge(int mode, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int	index_11;
	int	index_12;
	int	dummy = 0;
	char	route_relaion[30];
	char	pono_route[30];
	char	route_div[30];
	char	dev_assign[30];  //设备指定：0-人工指定; 1-模型推荐
	FILE * outstream;

	int doFlag = 0;
	int count = 0;
	int count1 = 0;
	CString station_id = "";
	CString station_no = "";
	CDecimal slab_width = 0, slab_width_pre = 0;
	CDecimal slab_thick = 0, slab_thick_pre = 0;
	CDecimal v_charge_no = 0;
	CString device_status = "";
	CString c_div_pre = "";
	CString st_no_pre = "";
	CDecimal cc_perp_time = 0;

	CModel tpssmd3("TPSSMD3");
	CModel tpssm17("TPSSM17");
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd6("TPSSMD6");
	//CTPSSMD5 tpssmd5(conn);
	CModel tpssmd9("TPSSMD9");
	CModel tpssm15_pre("TPSSM15");
	CModel tpssm41("TPSSM41");
	CModel tpssmdd("tpssmdd");
	CModel tapbd008s2n("TAPBD008S2N");
	CDbCommand cmd_inq(conn);
	CDataTable tb_tpssm15("TPSSM15");
	CDataTable tb_tpssm16("TPSSM16");

	CString sqlstr = "";
	CDbCommand cmd_tpssmd4_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tapb08_inq(conn);

	EIClass TPSSM17_SOURCE;
	EIClass TPSSMD9_SOURCE;
	EIClass TPSSMD6_SOURCE;
	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD3_SOURCE;
	EIClass TAPBD008S2N_SOURCE;

	EIClass TPSSM17_CONDI;
	EIClass TPSSMD9_CONDI;
	EIClass TPSSMD6_CONDI;
	EIClass TPSSMD1_CONDI;
	EIClass TPSSMD3_CONDI;
	EIClass TAPBD008S2N_CONDI;

	TPSSM17_CONDI.Tables[0].Columns.Add(DT_STRING, "PONO");
	TPSSM17_CONDI.Tables[0].Rows.Add();

	TPSSMD9_CONDI.Tables[0].Columns.Add(DT_STRING, "CAST_THICK");
	TPSSMD9_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD9_CONDI.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
	TPSSMD9_CONDI.Tables[0].Rows.Add();

	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "AREA_ID");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	TPSSMD6_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_MOVE_START");
	TPSSMD6_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD6_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_MOVE_END");
	TPSSMD6_CONDI.Tables[0].Rows.Add();

	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "ST_NO");
	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "DEV_CODE");
	TPSSMD3_CONDI.Tables[0].Columns.Add(DT_STRING, "SMELT_MODE");
	TPSSMD3_CONDI.Tables[0].Rows.Add();

	EIClass TPSSM17_RESULT;
	EIClass TPSSMD9_RESULT;
	EIClass TPSSMD6_RESULT;
	EIClass TPSSMD1_RESULT;
	EIClass TPSSMD3_RESULT;
	EIClass TAPBD008S2N_RESULT;

	try
	{
		Log::Trace("", __FUNCTION__, "GenTpsIn_output_pono_condition begin");

		//sprintf(file_name_in,   "%s/Trace/%s.in",  getenv("HOME"), (const char *)sys_time);  //模型计算用输入参数文件
		//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("HOME"), (const char *)sys_time);  //模型计算输出参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(file_name_in, "%s\\Trace\\%s.in", p, (const char *)("b" + sys_time));  //模型计算用输入参数文件
			sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)("b" + sys_time));  //模型计算输出参数文件
#else
			sprintf(file_name_in, "%s/Trace/%s.in", p, (const char *)("b" + sys_time));  //模型计算用输入参数文件
			sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)("b" + sys_time));  //模型计算输出参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(file_name_in, "..\\Trace\\%s.in", (const char *)("b" + sys_time));  //模型计算用输入参数文件
			sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)("b" + sys_time));  //模型计算输出参数文件
#else
			sprintf(file_name_in, "../Trace/%s.in", (const char *)("b" + sys_time));  //模型计算用输入参数文件
			sprintf(file_name_out, "../Trace/%s.out", (const char *)("b" + sys_time));  //模型计算输出参数文件
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		Log::Trace("", __FUNCTION__, "	file_name_in=[{0}],file_name_out=[{2}]\n", file_name_in, file_name_out);
		outstream = fopen(file_name_in, "w");
		setbuf(outstream, NULL);

		////相关数据准备
		sqlstr = "SELECT * FROM TPSSM17 WHERE FACTORY_DIV=@v_factory_div AND PONO IN ( SELECT a.PONO FROM TPSSM15 a,TPSSM16 b WHERE a.STEEL_RETURN_CODE = ' ' AND a.PONO_STATUS < 83  AND a.STEEL_RETURN_CODE = ' ' AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 3 AND (b.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.END_TIME_REAL = ' ' )) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSM17_SOURCE.Tables[0]);

		sqlstr = "SELECT * FROM TPSSMD9 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD9_SOURCE.Tables[0]);

		sqlstr =   " SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,"
					" COALESCE(T2.MOVE_TIME, T1.MOVE_TIME) AS MOVE_TIME FROM TPSSMD6 t1 LEFT JOIN TPSSMD6_X t2"
					" ON  t1.DEV_MOVE_START = t2.DEV_MOVE_START AND  t1.DEV_MOVE_END = t2.DEV_MOVE_END"
					" WHERE t1.FACTORY_DIV=@v_factory_div "
					" UNION  SELECT t1.DEV_MOVE_START,t1.DEV_MOVE_END,t1.TRAN_TYPE,t1.FACTORY_DIV,T1.MOVE_TIME"
					" FROM TPSSMD6_X t1   WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD6) AND  t1.FACTORY_DIV=@v_factory_div ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD6_SOURCE.Tables[0]);

		sqlstr = "SELECT * FROM TPSSMD1 ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);

		sqlstr = 
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,"
			" COALESCE(T2.STD_PROC_TIME, T1.STD_PROC_TIME) AS STD_PROC_TIME FROM TPSSMD3 t1 LEFT JOIN TPSSMD3_X t2"
			" ON  t1.ST_NO = t2.ST_NO AND  t1.DEV_CODE = t2.DEV_CODE "
			" UNION"
			" SELECT t1.FACTORY_DIV,t1.ST_NO,t1.DEV_CODE,t1.SMELT_MODE,t1.SMELT_MODE2,t1.COST_HJ,t1.STD_PREP_TIME,"
			" t1.FEED_TIME,t1.WORK_TIME,t1.DRAW_TIME,t1.WAITING_TIME,t1.REMARK,t1.STD_PROC_TIME FROM TPSSMD3_X t1"
			" WHERE  t1.ST_NO NOT IN (SELECT DISTINCT ST_NO FROM TPSSMD3) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TPSSMD3_SOURCE.Tables[0]);

		sqlstr = "SELECT * FROM TAPBD008S2N ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(TAPBD008S2N_SOURCE.Tables[0]);

		//数据准备
		sqlstr = "SELECT a.* FROM TPSSM15 a,TPSSM16 b WHERE a.FACTORY_DIV=@v_factory_div AND a.PONO_STATUS < 83 AND a.STEEL_RETURN_CODE = ' ' AND a.SM_PLAN_NO = b.SM_PLAN_NO AND b.AREA_ID = 3 AND (b.END_TIME_REAL > TO_CHAR(SYSDATE - 1, 'YYYYMMDDHH24MISS') OR b.END_TIME_REAL = ' ' ) ";
		sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(tb_tpssm15);
		//校验可编计划数如果小于0,报警提示
		if (tb_tpssm15.Rows.get_Count() <= 0)
		{
			EDLog(1, 1, "计划数量小于1,不需优化。");
			strcpy(s.msg, _RES("计划数量小于1,不需优化。")/*PSSMS0000213计划数量小于1,不需优化。*/);
			throw CApplicationException(-1, s.msg, log.Location);
			//CFormattable arguments[] = { tb_tpssm15.Rows.get_Count() }; // 定义参数列表的数组
			//CMessageFormat::Format(s.msg, _RES("计划数量小于1,不需优化。"), arguments, 1); //格式化字符串
			//throw CApplicationException(-1, s.msg, log.Location);
		}

		fprintf(outstream, "%d\t\t\t\t;可替换设备数\n", 0);
		fprintf(outstream, "%d\t\t\t\t;PONO数\n", tb_tpssm15.Rows.get_Count());//浇注开始的为第一炉，指定的计划数量
		/*--------------------------------------------------
		PONO信息
		--------------------------------------------------*/
		//Log::Trace("", __FUNCTION__, "tpssm15 begin");
		for (index_11 = 0; index_11< tb_tpssm15.Rows.get_Count(); index_11++)
		{
			tpssm15.MergeFrom(tb_tpssm15.Rows[index_11]);

			fprintf(outstream, "%d\t\t\t\t;序号\n", index_11 + 1);
			fprintf(outstream, "%s\t\t\t\t;PONO\n", (const char *)tpssm15["PONO"].ToString());

			//根据条件内容，生成"路径","路径关联关系","设备区分"
			memset(pono_route, 0, 30);
			memset(route_relaion, 0, 30);
			memset(route_div, 0, 30);
			GenTpsIn_route_createb((const char *)tpssm15["PONO"].ToString(), pono_route, route_relaion, route_div, conn);

			fprintf(outstream, "%s\t\t\t\t;路径设备区分\n", pono_route);
			fprintf(outstream, "%s\t\t\t\t;路径关联关系\n", route_relaion);
			fprintf(outstream, "%s\t\t\t\t;设备类型区分\n", route_div);

			if (tpssm15["CC_REQ_TIME"].ToString()[0] == ' ')
			{
				tpssm15["CC_REQ_TIME"] = "00000000000000";
			}
			fprintf(outstream, "%s\t\t\t;CC要求时刻\n", (const char *)tpssm15["CC_REQ_TIME"].ToString());
			////增加辅助作业时间
			//tpssmdd["JOB_CODE"] = tpssm15["BOF_ASSIST_OPT"].ToString();
			//tpssmdd.Query();
			//fprintf(outstream, "%s\t\t\t;当前炉辅助作业代码\n", (const char *)tpssmdd["JOB_CODE"].ToString().Trim());
			//fprintf(outstream, "%d\t\t\t\t;当前炉辅助作业时间\n", tpssmdd["STD_PROC_TIME"].ToDecimal().ToInt32());

			fprintf(outstream, "%s\t\t\t\t;浇次号\n", (const char *)tpssm15["CAST_NO"].ToString());
			fprintf(outstream, "%d\t\t\t\t;浇次分割号\n", tpssm15["CAST_DIV_NO"].ToDecimal().ToInt32());
			//fprintf(outstream, "%s\t\t\t\t;工艺路径包\n", (const char *)tpssm15["ROUTEBAGKEY"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;工艺路径\n", (const char *)tpssm15["ROUTELIST"].ToString());




			//fprintf(outstream, "%d\t\t\t\t;上炉辅助作业准备时间\n", tpssm15["CAST_DIV_NO"].ToDecimal().ToInt32());

			//"正在处理的工位"和"炉次状态"决定了实绩点，告知模型该点之前的数据不必计算
			//模型暂时不读这2个字段
			fprintf(outstream, "%02d\t\t\t\t;正在处理的工位\n", 0);
			fprintf(outstream, "%02d\t\t\t\t;炉次状态      \n", 0);
			/*--------------------------------------------------
			CHARGE信息
			--------------------------------------------------*/
			fprintf(outstream, "%d\t\t\t\t;工序%d\n", 0,0);
			fprintf(outstream, "%d\t\t\t\t;工序0准备时间\n", 0);
			fprintf(outstream, "%d\t\t\t\t;工序0处理时间\n", 0);
			fprintf(outstream, "%s\t\t\t;工序0计划开始时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0计划结束时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0实绩开始时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0实绩结束时刻\n", "00000000000000");

			sqlstr = "SELECT * FROM TPSSM16 WHERE FACTORY_DIV=@v_factory_div ";
			sqlstr += CString(" AND SM_PLAN_NO=@tpssm15.SM_PLAN_NO AND AREA_ID >1 AND DEV_CODE NOT IN ('Z1','Z2','Z3','Z4','Z5','Z6','Z7','Z8') ORDER BY CHARGE_NO ASC");
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.Parameters.Set("tpssm15.SM_PLAN_NO", tpssm15["SM_PLAN_NO"].ToString());
			cmd_inq.ExecuteQuery(tb_tpssm16);

			device_status = " ";

			TPSSM17_CONDI.Tables[0].Rows[0]["PONO"] = tpssm15["PONO"];
			TPSSM17_RESULT.Tables[0].Clear();
			f_pssm_query(TPSSM17_CONDI, TPSSM17_SOURCE, TPSSM17_RESULT, conn);
			//PrintDataTable(TPSSM17_RESULT.Tables[0]);
			slab_width = TPSSM17_RESULT.Tables[0].Rows[0]["SLAB_WIDTH"].ToDecimal();
			slab_width = TPSSM17_RESULT.Tables[0].Rows[0]["SLAB_THICK"].ToDecimal();
			tpssm17.MergeFrom(TPSSM17_RESULT.Tables[0].Rows[0]);

			/*tpssm17["PONO"] = tpssm15["PONO"];
			tpssm17.Query("PONO");
			slab_width = tpssm17["SLAB_WIDTH"];
			slab_thick = tpssm17["SLAB_THICK"];*/

			if (tpssm15["RESTRAND_FLG"].ToString() == "T")
			{
				//tpssm17["PONO"] = tpssm15["PONO"];
				//Log::Trace("", __FUNCTION__, "tpssm17.PONO = [{0}]", tpssm17["PONO"].ToString());
				//tpssm17.Query("PONO");

				/*tpssmd9["CAST_THICK"] = tpssm17["SLAB_THICK"];
				tpssmd9["FACTORY_DIV"] = tpssm17["FACTORY_DIV"];
				tpssmd9["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
				if (tpssmd9.QueryCount("FACTORY_DIV,CC_MACH_NO,CAST_THICK") != 1)
				{
				tpssmd9["TT_PREP_W0_CAST"] = 70;
				}
				else
				{
				tpssmd9.Query("FACTORY_DIV,CC_MACH_NO,CAST_THICK");
				}*/

				TPSSMD9_CONDI.Tables[0].Rows[0]["CAST_THICK"] = tpssm17["SLAB_THICK"];
				TPSSMD9_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm17["FACTORY_DIV"];
				TPSSMD9_CONDI.Tables[0].Rows[0]["CC_MACH_NO"] = tpssm15["CC_MACH_NO"];
				TPSSMD9_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD9_CONDI, TPSSMD9_SOURCE, TPSSMD9_RESULT, conn);

				if (TPSSMD9_RESULT.Tables[0].Rows.get_Count() != 1)
				{
					tpssmd9["TT_PREP_W0_CAST"] = 70;
				}
				else
				{
					tpssmd9.MergeFrom(TPSSMD9_RESULT.Tables[0].Rows[0]);
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

			//循环查询各工序
			for (index_12 = 0; index_12<tb_tpssm16.Rows.get_Count(); index_12++)
			{
				//Log::Trace("", __FUNCTION__, "tpssm12 begin");
				tpssm16.MergeFrom(tb_tpssm16.Rows[index_12]);
				//Log::Trace("", __FUNCTION__, "tpssm12 begin[{0}]",tpssm12["SM_PLAN_NO"].ToString());
				//v_charge_no = tpssm12["CHARGE_NO"];

				//---- 得到移行时间  --------------
				//查到达侧设备代码
				/*tpssmd1["FACTORY_DIV"] = v_factory_div;
				tpssmd1["DEV_CODE"] = tpssm12["DEV_CODE"];
				tpssmd1["AREA_ID"] = tpssm12["AREA_ID"];
				tpssmd1.Query("FACTORY_DIV,DEV_CODE,AREA_ID");*/

				TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
				TPSSMD1_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm16["DEV_CODE"];
				TPSSMD1_CONDI.Tables[0].Rows[0]["AREA_ID"] = tpssm16["AREA_ID"];
				TPSSMD1_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
				tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);


				//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd1 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
				/*tpssmd6["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
				tpssmd6["DEV_MOVE_START"] = station_id + station_no;
				tpssmd6["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();

				if (tpssmd6.QueryCount("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END") != 1)
				{
				tpssmd6["MOVE_TIME"] = 0;
				}
				else
				{
				tpssmd6.Query("FACTORY_DIV,DEV_MOVE_START,DEV_MOVE_END");
				}*/

				if (tpssmd1["STATION_ID"].ToString() == "X")
				{
					tpssmd1["STATION_ID"] = "E";
				}
				else if (tpssmd1["STATION_ID"].ToString() == "Y")
				{
					tpssmd1["STATION_ID"] = "B";
				}

				TPSSMD6_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssmd1["FACTORY_DIV"];
				TPSSMD6_CONDI.Tables[0].Rows[0]["DEV_MOVE_START"] = station_id + station_no;
				TPSSMD6_CONDI.Tables[0].Rows[0]["DEV_MOVE_END"] = tpssmd1["STATION_ID"].ToString() + tpssmd1["STATION_NO"].ToString();
				TPSSMD6_RESULT.Tables[0].Clear();
				f_pssm_query(TPSSMD6_CONDI, TPSSMD6_SOURCE, TPSSMD6_RESULT, conn);
				if (TPSSMD6_RESULT.Tables[0].Rows.get_Count() != 1)
				{
					tpssmd6["MOVE_TIME"] = 0;
				}
				else
				{
					tpssmd6.MergeFrom(TPSSMD6_RESULT.Tables[0].Rows[0]);
				}
				//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd6 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());

				fprintf(outstream, "%d\t\t\t\t;工序%d\n", tpssm16["CHARGE_NO"].ToDecimal().ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());

				//---- 统计该 charge_no 下的"准备时间", "处理时间" --------------
				////Log::Trace("", __FUNCTION__,"tpssm12.area_id = [{0}]",tpssm12["AREA_ID"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else
				{
					/*tpssmd3["ST_NO"] = tpssm17["ST_NO"];
					tpssmd3["FACTORY_DIV"] = tpssm17["FACTORY_DIV"];
					tpssmd3["DEV_CODE"] = tpssm12["DEV_CODE"];
					tpssmd3["SMELT_MODE"] = 0;
					if (tpssmd3.QueryCount("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE") == 1)
					{
					tpssmd3.Query("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE");
					}
					else
					{
					tpssmd3["DEV_CODE"] = tpssm12["DEV_CODE"].ToString().Substring(0, 1);
					if (tpssmd3.QueryCount("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE") == 1)
					{
					tpssmd3.Query("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE");
					}
					else
					{
					tpssmd3["DEV_CODE"] = tpssm12["DEV_CODE"];
					if (tpssm17["C_DIV"].ToString() == "1")
					{
					tpssmd3["ST_NO"] = "DEFAULTS";
					}
					else if (tpssm17["C_DIV"].ToString() == "2")
					{
					tpssmd3["ST_NO"] = "DEFAULTC";
					}
					tpssmd3.Query("FACTORY_DIV,ST_NO,DEV_CODE,SMELT_MODE");
					}
					}*/
					TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = tpssm17["ST_NO"];
					TPSSMD3_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = tpssm17["FACTORY_DIV"];
					TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm16["DEV_CODE"];
					TPSSMD3_CONDI.Tables[0].Rows[0]["SMELT_MODE"] = 0;
					TPSSMD3_RESULT.Tables[0].Clear();
					f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);
					//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd3 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
					if (TPSSMD3_RESULT.Tables[0].Rows.get_Count() == 1)
					{
						tpssmd3.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);
					}
					else
					{
						TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm16["DEV_CODE"].ToString().Substring(0, 1);
						TPSSMD3_RESULT.Tables[0].Clear();
						f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);
						//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd32 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
						if (TPSSMD3_RESULT.Tables[0].Rows.get_Count() == 1)
						{
							tpssmd3.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);
						}
						else
						{
							TPSSMD3_CONDI.Tables[0].Rows[0]["DEV_CODE"] = tpssm16["DEV_CODE"];
							if (tpssm17["C_DIV"].ToString() == "1")
							{
								TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTS";
							}
							else if (tpssm17["C_DIV"].ToString() == "2")
							{
								TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTC";
							}
							else if (tpssm17["C_DIV"].ToString().Trim() == "")
							{
								if (tpssm17["ROUTEBAGKEY"].ToString().Substring(0, 1) == "C")
								{
									TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTC";
								}
								else if (tpssm17["ROUTEBAGKEY"].ToString().Substring(0, 1) == "S")
								{
									TPSSMD3_CONDI.Tables[0].Rows[0]["ST_NO"] = "DEFAULTS";
								}
							}
							TPSSMD3_RESULT.Tables[0].Clear();
							//PrintDataTable(TPSSMD3_CONDI.Tables[0]);
							f_pssm_query(TPSSMD3_CONDI, TPSSMD3_SOURCE, TPSSMD3_RESULT, conn);
							//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd33 begin[{0}]", tpssm12["SM_PLAN_NO"].ToString());
							tpssmd3.MergeFrom(TPSSMD3_RESULT.Tables[0].Rows[0]);
							//Log::Trace("", __FUNCTION__, "tpssm12 tpssmd33 end[{0}]", tpssm12["SM_PLAN_NO"].ToString());
						}

					}


					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", tpssmd3["STD_PREP_TIME"].ToDecimal().ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				}

				fprintf(outstream, "%d\t\t\t\t;工序%d处理时间\n", tpssm16["PROC_TIME"].ToDecimal().ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				//---- 写入移行时间  --------------
				fprintf(outstream, "%d\t\t\t\t;工序%d至工序%d移动时间 \n", tpssmd6["MOVE_TIME"].ToDecimal().ToInt32(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32() - 1, tpssm16["CHARGE_NO"].ToDecimal().ToInt32());

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
				//---- 读取开始时刻/结束时刻 --------------
				//判断实绩表中的PONO是否存在，新编制的计划在实绩表中是不存在的
				if (tpssm16["START_TIME_REAL"].ToString().Trim() == "") tpssm16["START_TIME_REAL"] = "00000000000000";
				if (tpssm16["START_TIME"].ToString().Trim() == "") tpssm16["START_TIME"] = "00000000000000";
				//实绩结束时刻
				if (tpssm16["END_TIME_REAL"].ToString().Trim() == "") tpssm16["END_TIME_REAL"] = "00000000000000";
				if (tpssm16["END_TIME"].ToString().Trim() == "") tpssm16["END_TIME"] = "00000000000000";

				fprintf(outstream, "%s\t\t\t;工序%d计划开始时刻\n", (const char *)tpssm16["START_TIME"].ToString(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d计划结束时刻\n", (const char *)tpssm16["END_TIME"].ToString(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d实绩开始时刻\n", (const char *)tpssm16["START_TIME_REAL"].ToString(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d实绩结束时刻\n", (const char *)tpssm16["END_TIME_REAL"].ToString(), tpssm16["CHARGE_NO"].ToDecimal().ToInt32());

			}
			//Log::Trace("", __FUNCTION__, "tpssm12 tpssm12 END[{0}]", tpssm12["SM_PLAN_NO"].ToString());
			slab_width_pre = slab_width;
			slab_thick_pre = slab_thick;
			c_div_pre = tpssm17["C_DIV"];
			st_no_pre = tpssm17["ST_NO"];
		}
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_output_pono_condition end  ");
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
	cmd_tpssmd4_inq.Close();
	cmd_tpssm03_inq.Close();
	cmd_tpssm11_inq.Close();
	fclose(outstream);
	return doFlag;
}
/************************************************************
炉次条件文件输出
注:
1.模型中第一个工序必须是脱硫，但其只有4个数据项：准备时间、处理时间、开始时刻、结束时刻，并且都为空
2.其他工序则有5个数据项：准备时间、处理时间、前工序至本工序移动时间、开始时刻、结束时刻
************************************************************/
int GenTpsIn_output_pono_condition_1(int mode, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int	index_11;
	int	index_12;
	int	dummy = 0;
	char	route_relaion[30];
	char	pono_route[30];
	char	route_div[30];
	char	dev_assign[30];  //设备指定：0-人工指定; 1-模型推荐
	FILE * outstream;

	int doFlag = 0;
	int count = 0;
	int count1 = 0;
	CString station_id = "";
	CString station_no = "";
	CDecimal slab_width = 0, slab_width_pre = 0;
	CDecimal slab_thick = 0, slab_thick_pre = 0;
	CDecimal v_charge_no = 0;
	CString device_status = "";
	CString c_div_pre = "";
	CString st_no_pre = "";
	CDecimal cc_perp_time = 0;

	CModel tpssm10("TPSSM10");
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd6("TPSSMD6");
	//CTPSSMD5 tpssmd5(conn);
	CModel tpssmd9("TPSSMD9");
	CModel tpssm11_pre("TPSSM11");
	CModel tpssm41("TPSSM41");
	CModel tapbd008s2n("TAPBD008S2N");
	CModel tpssmdd("tpssmdd");
	CDbCommand cmd_inq(conn);
	CDataTable tb_tpssm11("TPSSM11");
	CDataTable tb_tpssm12("TPSSM12");

	CString sqlstr = "";
	CDbCommand cmd_tpssmd4_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm11_inq(conn);
	CDbCommand cmd_tapb08_inq(conn);
	try
	{
		Log::Trace("", __FUNCTION__, "GenTpsIn_output_pono_condition_1 begin");

		//sprintf(file_name_in_1,   "%s/Trace/%s.in",  getenv("HOME"), (const char *)sys_time_1);  //模型计算用输入参数文件
		//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("HOME"), (const char *)sys_time_1);  //模型计算输出参数文件
		char *p;
		if ((p = getenv("BM2_BUILD_DIR")))
		{
#if WIN32
			sprintf(file_name_in_1, "%s\\Trace\\%s.in", p, (const char *)sys_time_1);  //模型计算用输入参数文件
			sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)sys_time_1);  //模型计算输出参数文件
#else
			sprintf(file_name_in_1, "%s/Trace/%s.in", p, (const char *)sys_time_1);  //模型计算用输入参数文件
			sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)sys_time_1);  //模型计算输出参数文件
#endif
		}
		else
		{
#if WIN32
			sprintf(file_name_in_1, "..\\Trace\\%s.in", (const char *)sys_time_1);  //模型计算用输入参数文件
			sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)sys_time_1);  //模型计算输出参数文件
#else
			sprintf(file_name_in_1, "../Trace/%s.in", (const char *)sys_time_1);  //模型计算用输入参数文件
			sprintf(file_name_out, "../Trace/%s.out", (const char *)sys_time_1);  //模型计算输出参数文件
#endif
			//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
			//return -1;
		}
		Log::Trace("", __FUNCTION__, "	file_name_in_1=[{0}],file_name_out=[{2}]\n", file_name_in_1, file_name_out);
		outstream = fopen(file_name_in_1, "w");
		setbuf(outstream, NULL);
		//数据准备
		sqlstr = "SELECT * FROM TPSSM11 WHERE FACTORY_DIV=@v_factory_div AND PONO_STATUS < 83 AND STEEL_RETURN_CODE = ' ' ";//回炉不传
		sqlstr += CString(" ORDER BY CAST_NO,CAST_DIV_NO  ASC");
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
		cmd_inq.ExecuteQuery(tb_tpssm11);
		//校验可编计划数如果小于0,报警提示
		if (tb_tpssm11.Rows.get_Count() <= 0)
		{
			EDLog(1, 1, "计划数量小于1,不需优化。");
			strcpy(s.msg, _RES("计划数量小于1,不需优化。")/*PSSMS0000213计划数量小于1,不需优化。*/);
			throw CApplicationException(-1, s.msg, log.Location);
			//CFormattable arguments[] = { tb_tpssm11.Rows.get_Count() }; // 定义参数列表的数组
			//CMessageFormat::Format(s.msg, _RES("计划数量小于1,不需优化。"), arguments, 1); //格式化字符串
			//throw CApplicationException(-1, s.msg, log.Location);
		}

		fprintf(outstream, "%d\t\t\t\t;可替换设备数\n", 0);
		fprintf(outstream, "%d\t\t\t\t;PONO数\n", tb_tpssm11.Rows.get_Count());//浇注开始的为第一炉，指定的计划数量
		/*--------------------------------------------------
		PONO信息
		--------------------------------------------------*/
		for (index_11 = 0; index_11< tb_tpssm11.Rows.get_Count(); index_11++)
		{
			tpssm11.MergeFrom(tb_tpssm11.Rows[index_11]);

			fprintf(outstream, "%d\t\t\t\t;序号\n", index_11 + 1);
			fprintf(outstream, "%s\t\t\t\t;PONO\n", (const char *)tpssm11["PONO"].ToString());

			//根据条件内容，生成"路径","路径关联关系","设备区分"
			memset(pono_route, 0, 30);
			memset(route_relaion, 0, 30);
			memset(route_div, 0, 30);
			GenTpsIn_route_create((const char *)tpssm11["PONO"].ToString(), pono_route, route_relaion, route_div, conn);

			fprintf(outstream, "%s\t\t\t\t;路径设备区分\n", pono_route);
			fprintf(outstream, "%s\t\t\t\t;路径关联关系\n", route_relaion);
			fprintf(outstream, "%s\t\t\t\t;设备类型区分\n", route_div);

			if (tpssm11["CC_REQ_TIME"].ToString()[0] == ' ')
			{
				tpssm11["CC_REQ_TIME"] = "00000000000000";
			}
			fprintf(outstream, "%s\t\t\t;CC要求时刻\n", (const char *)tpssm11["CC_REQ_TIME"].ToString());
			////增加辅助作业时间
			//tpssmdd["JOB_CODE"] = tpssm11["BOF_ASSIST_OPT"].ToString();
			//tpssmdd.Query();
			//fprintf(outstream, "%s\t\t\t;当前炉辅助作业代码\n", (const char *)tpssmdd["JOB_CODE"].ToString().Trim());
			//fprintf(outstream, "%d\t\t\t\t;当前炉辅助作业时间\n", tpssmdd["STD_PROC_TIME"].ToDecimal().ToInt32());

			fprintf(outstream, "%s\t\t\t\t;浇次号\n", (const char *)tpssm11["CAST_NO"].ToString());
			fprintf(outstream, "%d\t\t\t\t;浇次分割号\n", tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32());
			//fprintf(outstream, "%s\t\t\t\t;工艺路径包\n", (const char *)tpssm11["ROUTEBAGKEY"].ToString());
			//fprintf(outstream, "%s\t\t\t\t;工艺路径\n", (const char *)tpssm11["ROUTELIST"].ToString());




			//fprintf(outstream, "%d\t\t\t\t;上炉辅助作业准备时间\n", tpssm11["CAST_DIV_NO"].ToDecimal().ToInt32());

			//"正在处理的工位"和"炉次状态"决定了实绩点，告知模型该点之前的数据不必计算
			//模型暂时不读这2个字段
			fprintf(outstream, "%02d\t\t\t\t;正在处理的工位\n", 0);
			fprintf(outstream, "%02d\t\t\t\t;炉次状态      \n", 0);
			/*--------------------------------------------------
			CHARGE信息
			--------------------------------------------------*/
			fprintf(outstream, "%d\t\t\t\t;工序0准备时间\n", 0);
			fprintf(outstream, "%d\t\t\t\t;工序0处理时间\n", 0);
			fprintf(outstream, "%s\t\t\t;工序0计划开始时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0计划结束时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0实绩开始时刻\n", "00000000000000");
			fprintf(outstream, "%s\t\t\t;工序0实绩结束时刻\n", "00000000000000");

			sqlstr = "SELECT * FROM TPSSM12 WHERE FACTORY_DIV=@v_factory_div ";
			sqlstr += CString(" AND SM_PLAN_NO=@tpssm11.SM_PLAN_NO AND AREA_ID >1 ORDER BY CHARGE_NO ASC");
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
			cmd_inq.Parameters.Set("tpssm11.SM_PLAN_NO", tpssm11["SM_PLAN_NO"].ToString());
			cmd_inq.ExecuteQuery(tb_tpssm12);

			device_status = " ";
			tpssm10["PONO"] = tpssm11["PONO"];
			tpssm10.Query("PONO");
			slab_width = tpssm10["SLAB_WIDTH"];
			slab_thick = tpssm10["SLAB_THICK"];
			if (tpssm11["RESTRAND_FLG"].ToString() == "T")
			{
				//Log::Trace("", __FUNCTION__, "tpssm10.PONO = [{0}]", tpssm10["PONO"].ToString());
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




			//循环查询各工序
			for (index_12 = 0; index_12<tb_tpssm12.Rows.get_Count(); index_12++)
			{
				tpssm12.MergeFrom(tb_tpssm12.Rows[index_12]);

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

					sqlstr = "SELECT MAX(WORK_TIME) FROM TAPBD008S2N WHERE FACTORY_DIV=@v_factory_div AND AREA_ID =@AREA_ID AND DEV_CODE = @DEV_CODE AND DEVICE_STATUS IN (" + device_status +")";
					cmd_tapb08_inq.SetCommandText(sqlstr);
					cmd_tapb08_inq.Parameters.Set("v_factory_div", v_factory_div);
					cmd_tapb08_inq.Parameters.Set("AREA_ID", tpssm12["AREA_ID"].ToDecimal());
					cmd_tapb08_inq.Parameters.Set("DEV_CODE", tpssm12["DEV_CODE"].ToString());
					cc_perp_time = cmd_tapb08_inq.ExecuteScalar();

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
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

					fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", cc_perp_time.ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
					//Log::Trace("", __FUNCTION__, "cc_perp_time = [{0}],device_status[{1}]", cc_perp_time.ToInt32(), device_status);
				}
				else
				{
					//当前炉准备时间=原有准备时间+上一炉辅助作业时间
					//当前炉辅助作业时间作用下一炉炉前准备时间
					/*if (tpssm12["DEV_CODE"].ToString().Substring(0, 1).Trim() == "B")
					{
						sqlstr = " SELECT A.*									 "
							" FROM TPSSM11 A, tpssm12 B							 "
							" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO					 "
							" AND B.DEV_CODE = @dev_code						 "
							" AND B.SM_PLAN_NO<@sm_plan_no						 "
							" order by SM_PLAN_NO desc fetch first 1 row only  	 "
							;
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("sm_plan_no", tpssm12["SM_PLAN_NO"].ToString());
						cmd_inq.Parameters.Set("dev_code", tpssm12["DEV_CODE"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							cmd_inq.Fetch(tpssm11_pre);
						}
						cmd_inq.Close();

						Log::Trace("", __FUNCTION__, "==BOF_ASSIST_TIME = [{0}]", tpssm11_pre["BOF_ASSIST_TIME"].ToDecimal().ToInt32());

						fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", tpssm12["PREP_TIME"].ToDecimal().ToInt32() + tpssm11_pre["BOF_ASSIST_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
					}
					else
					{*/
						fprintf(outstream, "%d\t\t\t\t;工序%d准备时间\n", tpssm12["PREP_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
					//}

				}

				fprintf(outstream, "%d\t\t\t\t;工序%d处理时间\n", tpssm12["PROC_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				//---- 写入移行时间  --------------
				fprintf(outstream, "%d\t\t\t\t;工序%d至工序%d移动时间 \n", tpssmd6["MOVE_TIME"].ToDecimal().ToInt32(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32() - 1, tpssm12["CHARGE_NO"].ToDecimal().ToInt32());

				station_id = tpssmd1["STATION_ID"];
				station_no = tpssmd1["STATION_NO"];
				//---- 读取开始时刻/结束时刻 --------------
				//判断实绩表中的PONO是否存在，新编制的计划在实绩表中是不存在的
				if (tpssm12["START_TIME_REAL"].ToString().Trim() == "") tpssm12["START_TIME_REAL"] = "00000000000000";
				if (tpssm12["START_TIME"].ToString().Trim() == "") tpssm12["START_TIME"] = "00000000000000";
				//实绩结束时刻
				if (tpssm12["END_TIME_REAL"].ToString().Trim() == "") tpssm12["END_TIME_REAL"] = "00000000000000";
				if (tpssm12["END_TIME"].ToString().Trim() == "") tpssm12["END_TIME"] = "00000000000000";

				fprintf(outstream, "%s\t\t\t;工序%d计划开始时刻\n", (const char *)tpssm12["START_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d计划结束时刻\n", (const char *)tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d实绩开始时刻\n", (const char *)tpssm12["START_TIME_REAL"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());
				fprintf(outstream, "%s\t\t\t;工序%d实绩结束时刻\n", (const char *)tpssm12["END_TIME_REAL"].ToString(), tpssm12["CHARGE_NO"].ToDecimal().ToInt32());

			}

			slab_width_pre = slab_width;
			slab_thick_pre = slab_thick;
			c_div_pre = tpssm10["C_DIV"];
			st_no_pre = tpssm10["ST_NO"];
		}
		//fclose(outstream);
		////Log::Trace("", __FUNCTION__,"GenTpsIn_output_pono_condition end  ");
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
	cmd_tpssmd4_inq.Close();
	cmd_tpssm03_inq.Close();
	cmd_tpssm11_inq.Close();
	fclose(outstream);
	return doFlag;
}

//模型调用函数接口定义
#ifndef PlanWeaveFuncPtr
typedef int(*PlanWeaveFuncPtr)(char *, char *, char *, char *, char *, int, int, int, int);
#endif

#ifndef PlanTimeOptimazationFuncPtr
typedef int(*PlanTimeOptimazationFuncPtr)(char *, char *, char *, char *, char *, int);
#endif


#ifndef TPS_TestFuncPtr
typedef int(*TPS_TestFuncPtr)(char *);
#endif

/************************************************************
模型计算
************************************************************/
int GenTpsIn_scheduling_pono(int mode)  //mode: 0-全局编制; 1-时间优化;2-滚动编制(局部编制);3-早到时间模式
{
	CTracer log(__FUNCTION__);
	int  ret_tps;

#if WIN32
	HMODULE pModleHandle = NULL;
#else
	void * pModleHandle = NULL;
#endif

	//void* mrf = GetPSTSLibHandle("TPS_Test", pModleHandle);
	//TPS_TestFuncPtr func = (TPS_TestFuncPtr)mrf;
	TPS_TestFuncPtr func = TPS_Test;
	if (func == NULL)
	{
		EDLog(1, 1, "没有找到入口函数TPS_Test");
		return -1;
	}
	ret_tps = func(file_name_in);
	////Log::Trace("", __FUNCTION__,"ret_tps111111111=[{0}]",ret_tps);	
	ret_tps = func(equipmentInfo_in);
	////Log::Trace("", __FUNCTION__,"ret_tps111111111=[{0}]",ret_tps);
	ret_tps = func(equipmentStateInfo_in);
	////Log::Trace("", __FUNCTION__,"ret_tps111111111=[{0}]",ret_tps);
	ret_tps = func(trantime_in);
	////Log::Trace("", __FUNCTION__,"ret_tps111111111=[{0}]",ret_tps);

	//mode: 0-全局编制; 1-时间优化;2-滚动编制(局部编制);3-早到时间模式
	if (mode == 0)
	{
		//void* mrf = GetPSTSLibHandle("PlanWeave", pModleHandle);
		//PlanWeaveFuncPtr func = (PlanWeaveFuncPtr)mrf;
		PlanWeaveFuncPtr func = PlanWeave;
		if (func == NULL)
		{
			////Log::Trace("", __FUNCTION__,"没有找到入口函数PlanWeave");
#if WIN32
			//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
			//FreeLibrary(mrf);
#else
			//dlerror();
			////Log::Trace("", __FUNCTION__, "开始释放...");
			//dlclose(mrf);
			//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
			return -1;
		}
		////Log::Trace("", __FUNCTION__,"123");
		ret_tps = func(file_name_in, file_name_out, equipmentInfo_in, equipmentStateInfo_in, trantime_in, 1, 1, 3, 5);//1120
		////Log::Trace("", __FUNCTION__,"12344");
#if WIN32
		//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
		//FreeLibrary(mrf);
#else
		//dlerror();
		////Log::Trace("", __FUNCTION__, "开始释放...");
		//dlclose(mrf);
		//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
	}
	else if (mode == 1)
	{
		//void* mrf = GetPSTSLibHandle("PlanTimeOptimazation", pModleHandle);
		//PlanTimeOptimazationFuncPtr func = (PlanTimeOptimazationFuncPtr)mrf;
		PlanTimeOptimazationFuncPtr func = PlanTimeOptimazation;
		if (func == NULL)
		{
			////Log::Trace("", __FUNCTION__,"没有找到入口函数PlanTimeOptimazation");
#if WIN32
			//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
			//FreeLibrary(mrf);
#else
			//dlerror();
			////Log::Trace("", __FUNCTION__, "开始释放...");
			//dlclose(mrf);
			//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif

			return -1;
		}
		ret_tps = func(file_name_in, file_name_out, equipmentInfo_in, equipmentStateInfo_in, trantime_in, 1);
#if WIN32
		//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
		//FreeLibrary(mrf);
#else
		//dlerror();
		////Log::Trace("", __FUNCTION__, "开始释放...");
		//dlclose(mrf);
		//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
	}
	else if (mode == 2)
	{
		//void* mrf = GetPSTSLibHandle("PlanWeave", pModleHandle);
		//PlanWeaveFuncPtr func = (PlanWeaveFuncPtr)mrf;
		PlanWeaveFuncPtr func = PlanWeave;
		if (func == NULL)
		{
			////Log::Trace("", __FUNCTION__,"没有找到入口函数PlanWeave");
#if WIN32
			//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
			//FreeLibrary(mrf);
#else
			//dlerror();
			////Log::Trace("", __FUNCTION__, "开始释放...");
			//dlclose(mrf);
			//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
			return -1;
		}
		////Log::Trace("", __FUNCTION__,"123");
		ret_tps = func(file_name_in, file_name_in_1, equipmentInfo_in, equipmentStateInfo_in, trantime_in, 1, 4, 1, 5);//1415

		////Log::Trace("", __FUNCTION__,"12344");
#if WIN32
		//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
		//FreeLibrary(mrf);
#else
		//dlerror();
		////Log::Trace("", __FUNCTION__, "开始释放...");
		//dlclose(mrf);
		//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
	}
	else if (mode == 3)
	{
		//void* mrf = GetPSTSLibHandle("PlanTimeOptimazation", pModleHandle);
		//PlanTimeOptimazationFuncPtr func = (PlanTimeOptimazationFuncPtr)mrf;
		//PlanTimeOptimazationFuncPtr func = PlanTimeOptimazation2;
		PlanWeaveFuncPtr func = PlanWeave;
		if (func == NULL)
		{
			////Log::Trace("", __FUNCTION__,"没有找到入口函数PlanTimeOptimazation");
#if WIN32
			//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
			//FreeLibrary(mrf);
#else
			//dlerror();
			////Log::Trace("", __FUNCTION__, "开始释放...");
			//dlclose(mrf);
			//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif

			return -1;
		}
		ret_tps = func(file_name_in, file_name_out, equipmentInfo_in, equipmentStateInfo_in, trantime_in, 1, 1, 3, 6);
#if WIN32
		//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
		//FreeLibrary(mrf);
#else
		//dlerror();
		////Log::Trace("", __FUNCTION__, "开始释放...");
		//dlclose(mrf);
		//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
	}
	else if (mode == 4)
	{
		{
			//void* mrf = GetPSTSLibHandle("PlanWeave", pModleHandle);
			//PlanWeaveFuncPtr func = (PlanWeaveFuncPtr)mrf;
			PlanWeaveFuncPtr func = PlanWeave;
			if (func == NULL)
			{
				////Log::Trace("", __FUNCTION__,"没有找到入口函数PlanWeave");
#if WIN32
				//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
				//FreeLibrary(mrf);
#else
				//dlerror();
				////Log::Trace("", __FUNCTION__, "开始释放...");
				//dlclose(mrf);
				//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
				return -1;
			}
			////Log::Trace("", __FUNCTION__,"123");
			ret_tps = func(file_name_in, file_name_out, equipmentInfo_in, equipmentStateInfo_in, trantime_in, 1, 7, 3, 5);//1120
			////Log::Trace("", __FUNCTION__,"12344");
#if WIN32
			//////Log::Trace("", __FUNCTION__, "开始释放Win32...");
			//FreeLibrary(mrf);
#else
			//dlerror();
			////Log::Trace("", __FUNCTION__, "开始释放...");
			//dlclose(mrf);
			//////Log::Trace("", __FUNCTION__, "释放完成，返回doflag=%d", doflag);
#endif
		}
	}

	if (ret_tps != 0)
	{
		//0825 暂时不报错，直接跳出
		////Log::Trace("", __FUNCTION__,"模型运行出错 ret_tps=[%d]",ret_tps);
		return -1;
	}
	////Log::Trace("", __FUNCTION__,"TPS2OK");
	////Log::Trace("", __FUNCTION__,"GenTpsIn_scheduling_pono end  ");
	return(0);
}


/************************************************************
读入模型计算结果，更新出钢计划
注:

************************************************************/
int GenTpsIn_read_scheduling(int mode, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int		index_11, index_12;
	int		i = 0, j = 0;
	char	ss[80];
	char	edit_mode[2]; //'N'-新增, 'U'-修改
	char    pono[10];
	int     route_len;
	char    time_start[30][15];  //模型计算后的各工序的开始时间
	char    time_end[30][15];    //模型计算后的各工序的结束时间
	int     charge_no;
	char    model_dev_code[3];   //模型输出设备
	char    ref_route[20];
	char    backlog_ea[20];


	int pono_count = 0;//总PONO数
	int doFlag = 0;
	CString pono_route = "";
	CModel tpssm11("TPSSM11");
	CModel tpssm12("TPSSM12");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd6("TPSSMD6");
	CDbCommand cmd_inq(conn);
	CDataTable tb_tpssm12("TPSSM12");
	CString sqlstr = "";
	CDateTime end_time;
	CTimeSpan tpd_diff_time;
	CString v_tpd_time = "";
	CString dp_flag = 1;

	CString string_update11 = " UPDATE TPSSM11 A SET ( REC_REVISOR,REC_REVISE_TIME,BACKLOG_EA,REFINE_ROUTE_CODE ) = ( SELECT REC_REVISOR,REC_REVISE_TIME,BACKLOG_EA,REFINE_ROUTE_CODE FROM ( ";
	CString string_update12_1 = " UPDATE TPSSM12 A SET ( START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR ) = ( SELECT START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR FROM ( ";
	CString string_update12_2 = " UPDATE TPSSM12 A SET ( START_TIME,END_TIME ) = ( SELECT START_TIME,END_TIME FROM ( ";

	CString string_update11_sub = " WHERE EXISTS ( SELECT 1 FROM ( ";
	CString string_update12_1_sub = " WHERE EXISTS ( SELECT 1 FROM ( ";
	CString string_update12_2_sub = " WHERE EXISTS ( SELECT 1 FROM ( ";
	FILE	*fp;

	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD1_RESULT;
	EIClass TPSSMD1_CONDI;

	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_ID");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_NO");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	sqlstr = "SELECT * FROM TPSSMD1 ";
	cmd_inq.SetCommandText(sqlstr);
	cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
	cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);
	cmd_inq.Close();

	try
	{
		Log::Trace("", __FUNCTION__,"GenTpsIn_read_scheduling begin");
		Log::Trace("", __FUNCTION__,"-------模型计算[{0}]---------", (const char *)v_factory_div);
		/*主工序代码*/
		tpssm11["FACTORY_DIV"] = v_factory_div;
		/*公共字段*/
		tpssm11["REC_CREATOR"] = s.userid;
		tpssm11["REC_CREATE_TIME"] = sys_time;
		tpssm11["REC_REVISOR"] = s.userid;
		tpssm11["REC_REVISE_TIME"] = sys_time;
		tpssm11["ARCHIVE_FLAG"] = " ";

		if (mode != 2)
		{
			//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("BM2_BUILD_DIR"),(const char *)sys_time);
			char *p;
			if ((p = getenv("BM2_BUILD_DIR")))
			{
#if WIN32				
				sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)sys_time);  //模型计算输出参数文件
#else				
				sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)sys_time);  //模型计算输出参数文件
#endif
			}
			else
			{
#if WIN32
				sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)sys_time);  //模型计算输出参数文件
#else
				sprintf(file_name_out, "../Trace/%s.out", (const char *)sys_time);  //模型计算输出参数文件
#endif
				//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
				//return -1;
			}

		}
		else
		{
			//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("BM2_BUILD_DIR"),(const char *)sys_time);
			char *p;
			if ((p = getenv("BM2_BUILD_DIR")))
			{
#if WIN32				
				sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)sys_time);  //模型计算输出参数文件
#else				
				sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)sys_time);  //模型计算输出参数文件
#endif
			}
			else
			{
#if WIN32
				sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)sys_time);  //模型计算输出参数文件
#else
				sprintf(file_name_out, "../Trace/%s.out", (const char *)sys_time);  //模型计算输出参数文件
#endif
				//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
				//return -1;
			}
		}
		fp = fopen(file_name_out, "r");
		/*--------- pono数 -----------*/
		readfile(ss, 200, fp);
		pono_count = atoi(ss);
		/*--------------------------------------------------
		读取炉次条件信息及模型计算后的PONO信息
		--------------------------------------------------*/
		//Log::Trace("", __FUNCTION__,"pono_count=[{0}]",pono_count);
		//Log::Trace("", __FUNCTION__, "4444");
		for (index_11 = 0; index_11 < pono_count; index_11++)
		{
			//读取模型计算输出的文件
			//PONO主体:
			/*------- 序号 -------*/
			readfile(ss, 200, fp);
			/*------- PONO号 -------*/
			readfile(ss, 200, fp);
			strcpy(pono, ss);
			/*------- 浇次号 -------*/
			readfile(ss, 200, fp);
			/*------- 浇次分割号 -------*/
			readfile(ss, 200, fp);
			/*------- 路径 -------*/
			readfile(ss, 200, fp);
			pono_route = ss;
			////Log::Trace("", __FUNCTION__, "ss=[{0}]", (const char *)ss);
			/*------- 钢包早到时间 -------*/
			readfile(ss, 200, fp);
			tpssm11["PONO"] = pono;
			tpssm11.Query("PONO");

			//Log::Trace("", __FUNCTION__,"pono=[{0}],pono_route=[{1}]",pono,pono_route);

			strcpy(ref_route, "");
			strcpy(backlog_ea, "");
			/*------- 工序作业时刻 -------*/
			//根据路径长度,读取各工序的开始、结束时刻
			route_len = strlen(pono_route);
			if (pono_route[2] == pono_route[4]) dp_flag = 2; //双联
			else dp_flag = 1;
			//Log::Trace("", __FUNCTION__,"1111");
			for (i = 0, j = 0; i<route_len; i++, j++)
			{
				/*------- 工序i -------*/
				readfile(ss, 200, fp);
				/*------- 工序i在设备上的顺序 -------*/
				readfile(ss, 200, fp);
				/*------- 工序i开始时刻 -------*/
				readfile(ss, 200, fp);
				strcpy(time_start[j], ss);
				/*------- 工序i结束时刻 -------*/
				readfile(ss, 200, fp);
				strcpy(time_end[j], ss);
				//2008-01-04 xuwen 新增
				//读取模型推荐的设备
				model_dev_code[0] = pono_route[i++];
				model_dev_code[1] = pono_route[i];
				model_dev_code[2] = '\0';

				//将模型设备转换为物理设备
				tpssmd1["DEV_CODE"] = model_dev_code;
				tpssmd1["FACTORY_DIV"] = v_factory_div;


				//Log::Trace("", __FUNCTION__, " tpssmd1.dev_code=[{0}],FACTORY_DIV={1},i={2}"
					//,(const char *)tpssmd1["DEV_CODE"].ToString(), tpssmd1["FACTORY_DIV"].ToString(), i);

				

				//if (i == 3)
				//{
				//	tpssmd1["AREA_ID"] = 2;
				//	tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
				//}
				//else if (i == 5)
				//{
				//	tpssmd1["AREA_ID"] = 3;
				//	tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
				//}
				//else
				//{
				//	if (i != 1)
				//		tpssmd1.Query("DEV_CODE,FACTORY_DIV");
				//}
				//tpssmd1["AREA_ID"] = 3;
				/*if (tpssmd1.QueryCount("DEV_CODE,FACTORY_DIV") == 1)
				{
					tpssmd1.Query("DEV_CODE,FACTORY_DIV");
				}
				else
				{
					tpssmd1["AREA_ID"] = 3;
					tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
				}*/
				if (model_dev_code[0] != '0')
				{
					TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
					TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd1["DEV_CODE"].ToString().Substring(0, 1);
					TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd1["DEV_CODE"].ToString().Substring(1, 1);
					TPSSMD1_RESULT.Tables[0].Clear();
					f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
					tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);
				}

				//0设备在这套产品化计划中无需处理
				if (model_dev_code[0] == '0')
				{
				}
				else
				{
					//Log::Trace("", __FUNCTION__,"tpssmd1.dev_code=[{0}]",(const char *)tpssmd1["DEV_CODE"].ToString());
					//Log::Trace("", __FUNCTION__, "tpssmd1.DEV_TECH_CODE=[{0}]", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
					//Log::Trace("", __FUNCTION__, "tpssmd1.AREA_ID=[{0}]", (const char *)tpssmd1["AREA_ID"].ToString());

					strcat(backlog_ea,(const char *)tpssmd1["DEV_CODE"].ToString().Substring(0,1));

					if (tpssmd1["AREA_ID"].ToDecimal() == 4)
					{
						//strcat(ref_route, (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
						strcat(ref_route, (const char *)tpssmd1["DEV_CODE"].ToString());
					}
					//Log::Trace("", __FUNCTION__,"ref_route=[{0}]",ref_route);
				}
				//Log::Trace("", __FUNCTION__,"----PONO=[{0}]: 工序[{1}]设备[{2}]开始[{3}], 结束[{4}], edit_mode=[{5}]----",
				//pono,  i, (const char *)tpssmd1["DEV_CODE"].ToString(), time_start[j], time_end[j], edit_mode);
			}
			//预处理时间的话单独处理
			charge_no = 1;
			////Log::Trace("", __FUNCTION__,"tpssm11.curr_wp_no=[{0}]",tpssm11["CURR_WP_NO"].ToDecimal().ToInt32());
			//Log::Trace("", __FUNCTION__, "2222");
			sqlstr = "SELECT * FROM TPSSM12 T2,TPSSM11 T1 WHERE T1.SM_PLAN_NO = T2.SM_PLAN_NO  AND T1.PONO=@tpssm11.PONO ORDER BY CHARGE_NO";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tpssm11.PONO", tpssm11["PONO"].ToString());
			cmd_inq.ExecuteQuery(tb_tpssm12);
			////Log::Trace("",__FUNCTION__,"tb_tpssm12.Rows={0}",tb_tpssm12.Rows.get_Count());
			for (index_12 = 0; index_12<tb_tpssm12.Rows.get_Count(); index_12++)
			{
				if (tb_tpssm12.Rows[index_12]["CHARGE_NO"].ToDecimal()<tpssm11["CURR_WP_NO"].ToDecimal())
				{
					continue;
				}
				tpssm12["SM_PLAN_NO"] = tpssm11["SM_PLAN_NO"];
				tpssm12["CHARGE_NO"] = tb_tpssm12.Rows[index_12]["CHARGE_NO"].ToDecimal();
				tpssm12["REC_REVISE_TIME"] = sys_time;
				tpssm12["REC_REVISOR"] = s.userid;
				tpssm12["PRE_PROC_NO"] = " ";
				tpssm12["PROC_NO"] = " ";

				if (pono_route.Substring(0, 2) == "00")
				{
					tpssm12["DEV_CODE"] = pono_route.Substring((index_12 * 2) + 2, 2);
					tpssm12["START_TIME"] = time_start[index_12 + 1];
					tpssm12["END_TIME"] = time_end[index_12 + 1];
				}
				else
				{
					tpssm12["DEV_CODE"] = pono_route.Substring(index_12 * 2, 2);
					tpssm12["START_TIME"] = time_start[index_12];
					tpssm12["END_TIME"] = time_end[index_12];
				}

				////Log::Trace("",__FUNCTION__,"tpssm12.Dev_code = {0},index_16={1},pono_route={2}",tpssm12["DEV_CODE"].ToString(),index_12,pono_route);
				if (index_12 == 0)
					tpssmd6["DEV_MOVE_END"] = tpssm12["DEV_CODE"];

				if (tb_tpssm12.Rows[index_12]["CHARGE_NO"].ToDecimal() == tpssm11["CURR_WP_NO"].ToDecimal())
				{
					//tpssm12.Update("START_TIME,END_TIME", "SM_PLAN_NO,CHARGE_NO");

					/*string_update12_2 = string_update12_2 + " SELECT '" + tpssm12["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
						 + tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL UNION ";

					string_update12_2_sub = string_update12_2_sub + " SELECT '" + tpssm12["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
						+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL UNION ";*/
					continue;
				}
				//tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PRE_PROC_NO,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");
				//Log::Trace("", __FUNCTION__, "tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());
				//tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");

				string_update12_1 = string_update12_1 + " SELECT '" + tpssm12["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '" 
					+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME, '"
					+ tpssm12["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm12["PROC_NO"].ToString() + "' AS PROC_NO, '"
					+ tpssm12["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm12["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL UNION ";

				string_update12_1_sub = string_update12_1_sub + " SELECT '" + tpssm12["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '" 
					+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME, '"
					+ tpssm12["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm12["PROC_NO"].ToString() + "' AS PROC_NO, '"
					+ tpssm12["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm12["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL UNION ";
				//Log::Trace("", __FUNCTION__, "111tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());
			}

			//Log::Trace("", __FUNCTION__, "3333");
			tpssm11["BACKLOG_EA"] = backlog_ea;
			tpssm11["REFINE_ROUTE_CODE"] = ref_route;
			tpssm11["REFINE_ROUTE_CODE"] = tpssm11["REFINE_ROUTE_CODE"].ToString().TrimOrBlank();

			//Log::Trace("", __FUNCTION__,"tpssm11.backlog_ea=[{0}]",(const char *)tpssm11["BACKLOG_EA"].ToString());
			//Log::Trace("", __FUNCTION__,"tpssm11.refine_route_code=[{0}]",(const char *)tpssm11["REFINE_ROUTE_CODE"].ToString());

			//修改工序主表的内容
			//tpssm11.Update("REC_REVISOR,REC_REVISE_TIME,BACKLOG_EA,REFINE_ROUTE_CODE", "PONO");

			string_update11 = string_update11 + " SELECT '" + tpssm11["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm11["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
				+ tpssm11["BACKLOG_EA"].ToString() + "' AS BACKLOG_EA, '" + tpssm11["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, '"
				+ tpssm11["PONO"].ToString() + "' AS PONO FROM DUAL UNION ";

			string_update11_sub = string_update11_sub + " SELECT '" + tpssm11["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm11["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
				+ tpssm11["BACKLOG_EA"].ToString() + "' AS BACKLOG_EA, '" + tpssm11["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, '"
				+ tpssm11["PONO"].ToString() + "' AS PONO FROM DUAL UNION ";
			//Log::Trace("", __FUNCTION__,"更新11表完毕");
			//Log::Trace("", __FUNCTION__, "6666");
		}
		//Log::Trace("", __FUNCTION__, "5555");

		string_update11 = string_update11 + " SELECT '" + tpssm11["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm11["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
			+ tpssm11["BACKLOG_EA"].ToString() + "' AS BACKLOG_EA, '" + tpssm11["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, 'XXXXXXXXX' AS PONO FROM DUAL) B "
			+ " WHERE A.PONO = B.PONO) ";
		string_update11_sub = string_update11_sub + " SELECT '" + tpssm11["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm11["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
			+ tpssm11["BACKLOG_EA"].ToString() + "' AS BACKLOG_EA, '" + tpssm11["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, 'XXXXXXXXX' AS PONO FROM DUAL) B "
			+ " WHERE A.PONO = B.PONO) ";

		string_update11 = string_update11 + string_update11_sub;

		string_update12_1 = string_update12_1 + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '" 
			+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME, '"
			+ tpssm12["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm12["PROC_NO"].ToString() + "' AS PROC_NO, '"
			+ tpssm12["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm12["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL ) B "
			" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) " ;
		string_update12_1_sub = string_update12_1_sub + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
			+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME, '"
			+ tpssm12["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm12["PROC_NO"].ToString() + "' AS PROC_NO, '"
			+ tpssm12["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm12["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL ) B "
			" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";

		string_update12_1 = string_update12_1 + string_update12_1_sub;

		/*string_update12_2 = string_update12_2 + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
			+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL ) B "
			" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";
		string_update12_2_sub = string_update12_2_sub + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
			+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL ) B "
			" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";

		string_update12_2 = string_update12_2 + string_update12_2_sub;*/

		cmd_inq.SetCommandText(string_update12_1);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		/*cmd_inq.SetCommandText(string_update12_2);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();*/

		cmd_inq.SetCommandText(string_update11);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		//Log::Trace("", __FUNCTION__, "string_update11 = [{0}]", string_update11);
		//Log::Trace("", __FUNCTION__, "string_update12_1 = [{0}]", string_update12_1);
		//Log::Trace("", __FUNCTION__, "string_update12_2 = [{0}]", string_update12_2);

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

int GenTpsIn_read_scheduling_err(int mode, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int		index_11, index_12;
	int		i = 0, j = 0;
	char	ss[500];
	char	edit_mode[2]; //'N'-新增, 'U'-修改
	char    pono[10];
	int     route_len;
	char    time_start[30][15];  //模型计算后的各工序的开始时间
	char    time_end[30][15];    //模型计算后的各工序的结束时间
	int     charge_no;
	char    model_dev_code[3];   //模型输出设备
	char    ref_route[20];
	char    backlog_ea[20];
	char    cs_bz[10];
	CString cs_time = "";
	int pono_count = 0;//总PONO数
	int doFlag = 0;
	CString pono_route = "";
	CModel tpssmerr("TPSSMERR");
	CDbCommand cmd_inq(conn);
	CString sqlstr = "";
	CDateTime end_time;
	CTimeSpan tpd_diff_time;
	CString v_tpd_time = "";
	CString dp_flag = 1;
	CString string_update11 = " ";
	FILE	*fp;

	

	try
	{
		Log::Trace("", __FUNCTION__, "GenTpsIn_read_scheduling begin");
		Log::Trace("", __FUNCTION__, "-------模型计算[{0}]---------", (const char *)v_factory_div);
		/*主工序代码*/
		

		if (mode != 2)
		{
			//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("BM2_BUILD_DIR"),(const char *)sys_time);
			char *p;
			if ((p = getenv("BM2_BUILD_DIR")))
			{
#if WIN32				
				sprintf(file_name_err, "%s\\Trace\\%s.err", p, (const char *)sys_time);  //模型计算输出参数文件
#else				
				sprintf(file_name_err, "%s/Trace/%s.err", p, (const char *)sys_time);  //模型计算输出参数文件
#endif
			}
			else
			{
#if WIN32
				sprintf(file_name_err, "..\\Trace\\%s.err", (const char *)sys_time);  //模型计算输出参数文件
#else
				sprintf(file_name_err, "../Trace/%s.err", (const char *)sys_time);  //模型计算输出参数文件
#endif
				//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
				//return -1;
			}

		}
		else
		{
			//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("BM2_BUILD_DIR"),(const char *)sys_time);
			char *p;
			if ((p = getenv("BM2_BUILD_DIR")))
			{
#if WIN32				
				sprintf(file_name_err, "%s\\Trace\\%s.err", p, (const char *)sys_time);  //模型计算输出参数文件
#else				
				sprintf(file_name_err, "%s/Trace/%s.err", p, (const char *)sys_time);  //模型计算输出参数文件
#endif
			}
			else
			{
#if WIN32
				sprintf(file_name_err, "..\\Trace\\%s.err", (const char *)sys_time);  //模型计算输出参数文件
#else
				sprintf(file_name_err, "../Trace/%s.err", (const char *)sys_time);  //模型计算输出参数文件
#endif
				//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
				//return -1;
			}
		}
		fp = fopen(file_name_err, "r");
		/*--------- pono数 -----------*/
		readfile_l(ss, 500, fp);
		Log::Trace("", __FUNCTION__, "-------模型计算[{0}]---------", (const char *)ss);
		for (index_11 = 0; index_11 < 500; index_11++)
		{
			//读取模型计算输出的文件
			//PONO主体:
			/*------- 序号 -------*/
			readfile_l(ss, 500, fp);
			//Log::Trace("", __FUNCTION__, "-------模型计算[{0}]---------", (const char *)ss);
			strcpy(cs_bz, ss);
			tpssmerr["REMARK_GCPM"] = cs_bz;
		}
		Log::Trace("", __FUNCTION__, "tpssmerr.REMARK_GCPM=[{0}]", (const char *)tpssmerr["REMARK_GCPM"]);
		tpssmerr["REC_CREATE_TIME"] = s.datetime;
		tpssmerr.TrimOrBlank();
		tpssmerr.Insert();
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

int GenTpsIn_read_scheduling_charge(int mode, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int		index_11, index_12;
	int		i = 0, j = 0;
	char	ss[80];
	char	edit_mode[2]; //'N'-新增, 'U'-修改
	char    pono[10];
	int     route_len;
	char    time_start[30][15];  //模型计算后的各工序的开始时间
	char    time_end[30][15];    //模型计算后的各工序的结束时间
	int     charge_no;
	char    model_dev_code[3];   //模型输出设备
	char    ref_route[20];
	char    backlog_ea[20];


	int pono_count = 0;//总PONO数
	int doFlag = 0;
	CString pono_route = "";
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssmd1("TPSSMD1");
	CModel tpssmd6("TPSSMD6");
	CDbCommand cmd_inq(conn);
	CDataTable tb_tpssm12("TPSSM16");
	CString sqlstr = "";
	CDateTime end_time;
	CTimeSpan tpd_diff_time;
	CString v_tpd_time = "";
	CString dp_flag = 1;

	CString string_update11 = " UPDATE TPSSM15 A SET ( REC_REVISOR,REC_REVISE_TIME,REFINE_ROUTE_CODE ) = ( SELECT REC_REVISOR,REC_REVISE_TIME,REFINE_ROUTE_CODE FROM ( ";
	CString string_update12_1 = " UPDATE TPSSM16 A SET ( START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR ) = ( SELECT START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR FROM ( ";
	CString string_update12_2 = " UPDATE TPSSM16 A SET ( START_TIME,END_TIME ) = ( SELECT START_TIME,END_TIME FROM ( ";

	CString string_update11_sub = " WHERE EXISTS ( SELECT 1 FROM ( ";
	CString string_update12_1_sub = " WHERE EXISTS ( SELECT 1 FROM ( ";
	CString string_update12_2_sub = " WHERE EXISTS ( SELECT 1 FROM ( ";
	FILE	*fp;

	EIClass TPSSMD1_SOURCE;
	EIClass TPSSMD1_RESULT;
	EIClass TPSSMD1_CONDI;

	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_ID");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "STATION_NO");
	TPSSMD1_CONDI.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
	TPSSMD1_CONDI.Tables[0].Rows.Add();

	sqlstr = "SELECT * FROM TPSSMD1 ";
	cmd_inq.SetCommandText(sqlstr);
	cmd_inq.Parameters.Set("v_factory_div", v_factory_div);
	cmd_inq.ExecuteQuery(TPSSMD1_SOURCE.Tables[0]);
	cmd_inq.Close();

	try
	{
		Log::Trace("", __FUNCTION__, "GenTpsIn_read_scheduling begin");
		Log::Trace("", __FUNCTION__, "-------模型计算[{0}]---------", (const char *)v_factory_div);
		/*主工序代码*/
		tpssm15["FACTORY_DIV"] = v_factory_div;
		/*公共字段*/
		tpssm15["REC_CREATOR"] = s.userid;
		tpssm15["REC_CREATE_TIME"] = sys_time;
		tpssm15["REC_REVISOR"] = s.userid;
		tpssm15["REC_REVISE_TIME"] = sys_time;
		tpssm15["ARCHIVE_FLAG"] = " ";

		if (mode != 2)
		{
			//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("BM2_BUILD_DIR"),(const char *)sys_time);
			char *p;
			if ((p = getenv("BM2_BUILD_DIR")))
			{
#if WIN32				
				sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)("b" + sys_time));  //模型计算输出参数文件
#else				
				sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)("b" + sys_time));  //模型计算输出参数文件
#endif
			}
			else
			{
#if WIN32
				sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)("b" + sys_time));  //模型计算输出参数文件
#else
				sprintf(file_name_out, "../Trace/%s.out", (const char *)("b" + sys_time));  //模型计算输出参数文件
#endif
				//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
				//return -1;
			}

		}
		else
		{
			//sprintf(file_name_out,  "%s/Trace/%s.out", getenv("BM2_BUILD_DIR"),(const char *)sys_time);
			char *p;
			if ((p = getenv("BM2_BUILD_DIR")))
			{
#if WIN32				
				sprintf(file_name_out, "%s\\Trace\\%s.out", p, (const char *)("b" + sys_time));  //模型计算输出参数文件
#else				
				sprintf(file_name_out, "%s/Trace/%s.out", p, (const char *)("b" + sys_time));  //模型计算输出参数文件
#endif
			}
			else
			{
#if WIN32
				sprintf(file_name_out, "..\\Trace\\%s.out", (const char *)("b" + sys_time));  //模型计算输出参数文件
#else
				sprintf(file_name_out, "../Trace/%s.out", (const char *)("b" + sys_time));  //模型计算输出参数文件
#endif
				//////Log::Trace("", __FUNCTION__, "Can not get environment [BM2_BUILD_DIR] value.");
				//return -1;
			}
		}
		fp = fopen(file_name_out, "r");
		/*--------- pono数 -----------*/
		readfile(ss, 200, fp);
		pono_count = atoi(ss);
		/*--------------------------------------------------
		读取炉次条件信息及模型计算后的PONO信息
		--------------------------------------------------*/
		//Log::Trace("", __FUNCTION__,"pono_count=[{0}]",pono_count);
		//Log::Trace("", __FUNCTION__, "4444");
		for (index_11 = 0; index_11 < pono_count; index_11++)
		{
			//读取模型计算输出的文件
			//PONO主体:
			/*------- 序号 -------*/
			readfile(ss, 200, fp);
			/*------- PONO号 -------*/
			readfile(ss, 200, fp);
			strcpy(pono, ss);
			/*------- 浇次号 -------*/
			readfile(ss, 200, fp);
			/*------- 浇次分割号 -------*/
			readfile(ss, 200, fp);
			/*------- 路径 -------*/
			readfile(ss, 200, fp);
			pono_route = ss;
			////Log::Trace("", __FUNCTION__, "ss=[{0}]", (const char *)ss);
			/*------- 钢包早到时间 -------*/
			readfile(ss, 200, fp);
			tpssm15["PONO"] = pono;
			tpssm15.Query("PONO");

			//Log::Trace("", __FUNCTION__,"pono=[{0}],pono_route=[{1}]",pono,pono_route);

			strcpy(ref_route, "");
			strcpy(backlog_ea, "");
			/*------- 工序作业时刻 -------*/
			//根据路径长度,读取各工序的开始、结束时刻
			route_len = strlen(pono_route);
			if (pono_route[2] == pono_route[4]) dp_flag = 2; //双联
			else dp_flag = 1;
			//Log::Trace("", __FUNCTION__,"1111");
			for (i = 0, j = 0; i<route_len; i++, j++)
			{
				/*------- 工序i -------*/
				readfile(ss, 200, fp);
				//strcpy(time_start[j], ss);
				/*------- 工序i在设备上的顺序 -------*/
				readfile(ss, 200, fp);
				/*------- 工序i开始时刻 -------*/
				readfile(ss, 200, fp);
				strcpy(time_start[j], ss);
				/*------- 工序i结束时刻 -------*/
				readfile(ss, 200, fp);
				strcpy(time_end[j], ss);
				//2008-01-04 xuwen 新增
				//读取模型推荐的设备
				model_dev_code[0] = pono_route[i++];
				model_dev_code[1] = pono_route[i];
				model_dev_code[2] = '\0';

				//将模型设备转换为物理设备
				tpssmd1["DEV_CODE"] = model_dev_code;
				tpssmd1["FACTORY_DIV"] = v_factory_div;


				//Log::Trace("", __FUNCTION__, " tpssmd1.dev_code=[{0}],FACTORY_DIV={1},i={2}"
				//,(const char *)tpssmd1["DEV_CODE"].ToString(), tpssmd1["FACTORY_DIV"].ToString(), i);



				//if (i == 3)
				//{
				//	tpssmd1["AREA_ID"] = 2;
				//	tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
				//}
				//else if (i == 5)
				//{
				//	tpssmd1["AREA_ID"] = 3;
				//	tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
				//}
				//else
				//{
				//	if (i != 1)
				//		tpssmd1.Query("DEV_CODE,FACTORY_DIV");
				//}
				//tpssmd1["AREA_ID"] = 3;
				/*if (tpssmd1.QueryCount("DEV_CODE,FACTORY_DIV") == 1)
				{
				tpssmd1.Query("DEV_CODE,FACTORY_DIV");
				}
				else
				{
				tpssmd1["AREA_ID"] = 3;
				tpssmd1.Query("DEV_CODE,AREA_ID,FACTORY_DIV");
				}*/
				if (model_dev_code[0] != '0')
				{
					TPSSMD1_CONDI.Tables[0].Rows[0]["FACTORY_DIV"] = v_factory_div;
					TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_ID"] = tpssmd1["DEV_CODE"].ToString().Substring(0, 1);
					TPSSMD1_CONDI.Tables[0].Rows[0]["STATION_NO"] = tpssmd1["DEV_CODE"].ToString().Substring(1, 1);
					TPSSMD1_RESULT.Tables[0].Clear();
					f_pssm_query(TPSSMD1_CONDI, TPSSMD1_SOURCE, TPSSMD1_RESULT, conn);
					tpssmd1.MergeFrom(TPSSMD1_RESULT.Tables[0].Rows[0]);
				}

				//0设备在这套产品化计划中无需处理 
				if (model_dev_code[0] == '0')
				{
				}
				else
				{
					//Log::Trace("", __FUNCTION__,"tpssmd1.dev_code=[{0}]",(const char *)tpssmd1["DEV_CODE"].ToString());
					//Log::Trace("", __FUNCTION__, "tpssmd1.DEV_TECH_CODE=[{0}]", (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
					//Log::Trace("", __FUNCTION__, "tpssmd1.AREA_ID=[{0}]", (const char *)tpssmd1["AREA_ID"].ToString());

					strcat(backlog_ea, (const char *)tpssmd1["DEV_CODE"].ToString().Substring(0, 1));

					if (tpssmd1["AREA_ID"].ToDecimal() == 4)
					{
						//strcat(ref_route, (const char *)tpssmd1["DEV_TECH_CODE"].ToString());
						strcat(ref_route, (const char *)tpssmd1["DEV_CODE"].ToString());
					}
					//Log::Trace("", __FUNCTION__,"ref_route=[{0}]",ref_route);
				}
				//Log::Trace("", __FUNCTION__,"----PONO=[{0}]: 工序[{1}]设备[{2}]开始[{3}], 结束[{4}], edit_mode=[{5}]----",
				//pono,  i, (const char *)tpssmd1["DEV_CODE"].ToString(), time_start[j], time_end[j], edit_mode);
			}
			//预处理时间的话单独处理
			charge_no = 1;
			////Log::Trace("", __FUNCTION__,"tpssm11.curr_wp_no=[{0}]",tpssm11["CURR_WP_NO"].ToDecimal().ToInt32());
			//Log::Trace("", __FUNCTION__, "2222");
			sqlstr = "SELECT * FROM TPSSM16 T2,TPSSM15 T1 WHERE T1.SM_PLAN_NO = T2.SM_PLAN_NO  AND T1.PONO=@tpssm15.PONO AND T2.DEV_CODE NOT IN ('Z1','Z2','Z3','Z4','Z5','Z6','Z7','Z8') ORDER BY CHARGE_NO";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tpssm15.PONO", tpssm15["PONO"].ToString());
			cmd_inq.ExecuteQuery(tb_tpssm12);
			////Log::Trace("",__FUNCTION__,"tb_tpssm12.Rows={0}",tb_tpssm12.Rows.get_Count());
			for (index_12 = 0; index_12<tb_tpssm12.Rows.get_Count(); index_12++)
			{
				if (tb_tpssm12.Rows[index_12]["CHARGE_NO"].ToDecimal()<tpssm15["CURR_WP_NO"].ToDecimal())
				{
					continue;
				}
				tpssm16["SM_PLAN_NO"] = tpssm15["SM_PLAN_NO"];
				tpssm16["CHARGE_NO"] = tb_tpssm12.Rows[index_12]["CHARGE_NO"].ToDecimal();
				tpssm16["REC_REVISE_TIME"] = sys_time;
				tpssm16["REC_REVISOR"] = s.userid;
				tpssm16["PRE_PROC_NO"] = " ";
				tpssm16["PROC_NO"] = " ";

				if (pono_route.Substring(0, 2) == "00")
				{
					tpssm16["DEV_CODE"] = pono_route.Substring((index_12 * 2) + 2, 2);
					tpssm16["START_TIME"] = time_start[index_12 + 1];
					tpssm16["END_TIME"] = time_end[index_12 + 1];
				}
				else
				{
					tpssm16["DEV_CODE"] = pono_route.Substring(index_12 * 2, 2);
					tpssm16["START_TIME"] = time_start[index_12];
					tpssm16["END_TIME"] = time_end[index_12];
				}

				////Log::Trace("",__FUNCTION__,"tpssm12.Dev_code = {0},index_16={1},pono_route={2}",tpssm12["DEV_CODE"].ToString(),index_12,pono_route);
				if (index_12 == 0)
					tpssmd6["DEV_MOVE_END"] = tpssm16["DEV_CODE"];

				if (tb_tpssm12.Rows[index_12]["CHARGE_NO"].ToDecimal() == tpssm15["CURR_WP_NO"].ToDecimal())
				{
					//tpssm12.Update("START_TIME,END_TIME", "SM_PLAN_NO,CHARGE_NO");

					/*string_update12_2 = string_update12_2 + " SELECT '" + tpssm12["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
					+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL UNION ";

					string_update12_2_sub = string_update12_2_sub + " SELECT '" + tpssm12["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
					+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL UNION ";*/
					continue;
				}
				//tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PRE_PROC_NO,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");
				//Log::Trace("", __FUNCTION__, "tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());
				//tpssm12.Update("START_TIME,END_TIME,DEV_CODE,PROC_NO,REC_REVISE_TIME,REC_REVISOR", "SM_PLAN_NO,CHARGE_NO");

				string_update12_1 = string_update12_1 + " SELECT '" + tpssm16["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm16["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
					+ tpssm16["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm16["END_TIME"].ToString() + "' AS END_TIME, '"
					+ tpssm16["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm16["PROC_NO"].ToString() + "' AS PROC_NO, '"
					+ tpssm16["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm16["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL UNION ";

				string_update12_1_sub = string_update12_1_sub + " SELECT '" + tpssm16["SM_PLAN_NO"].ToString() + "' AS SM_PLAN_NO, " + tpssm16["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
					+ tpssm16["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm16["END_TIME"].ToString() + "' AS END_TIME, '"
					+ tpssm16["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm16["PROC_NO"].ToString() + "' AS PROC_NO, '"
					+ tpssm16["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm16["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL UNION ";
				//Log::Trace("", __FUNCTION__, "111tpssm12.Dev_code = {0},START_TIME={1},END_TIME={2},CHARGE_NO={3},SM_PLAN_NO={4}", tpssm12["DEV_CODE"].ToString(), tpssm12["START_TIME"].ToString(), tpssm12["END_TIME"].ToString(), tpssm12["CHARGE_NO"].ToDecimal(), tpssm12["SM_PLAN_NO"].ToString());
			}

			//Log::Trace("", __FUNCTION__, "3333");
			tpssm15["BACKLOG_EA"] = backlog_ea;
			tpssm15["REFINE_ROUTE_CODE"] = ref_route;
			tpssm15["REFINE_ROUTE_CODE"] = tpssm15["REFINE_ROUTE_CODE"].ToString().TrimOrBlank();

			//Log::Trace("", __FUNCTION__,"tpssm11.backlog_ea=[{0}]",(const char *)tpssm11["BACKLOG_EA"].ToString());
			//Log::Trace("", __FUNCTION__,"tpssm11.refine_route_code=[{0}]",(const char *)tpssm11["REFINE_ROUTE_CODE"].ToString());

			//修改工序主表的内容
			//tpssm11.Update("REC_REVISOR,REC_REVISE_TIME,BACKLOG_EA,REFINE_ROUTE_CODE", "PONO");

			string_update11 = string_update11 + " SELECT '" + tpssm15["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm15["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
				+ tpssm15["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, '"
				+ tpssm15["PONO"].ToString() + "' AS PONO FROM DUAL UNION ";

			string_update11_sub = string_update11_sub + " SELECT '" + tpssm15["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm15["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
				+ tpssm15["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, '"
				+ tpssm15["PONO"].ToString() + "' AS PONO FROM DUAL UNION ";
			//Log::Trace("", __FUNCTION__,"更新11表完毕");
			//Log::Trace("", __FUNCTION__, "6666");
		}
		//Log::Trace("", __FUNCTION__, "5555");

		string_update11 = string_update11 + " SELECT '" + tpssm15["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm15["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
			+ tpssm15["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, 'XXXXXXXXX' AS PONO FROM DUAL) B "
			+ " WHERE A.PONO = B.PONO) ";
		string_update11_sub = string_update11_sub + " SELECT '" + tpssm15["REC_REVISOR"].ToString() + "' AS REC_REVISOR, '" + tpssm15["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '"
			+ tpssm15["REFINE_ROUTE_CODE"].ToString() + "' AS REFINE_ROUTE_CODE, 'XXXXXXXXX' AS PONO FROM DUAL) B "
			+ " WHERE A.PONO = B.PONO) ";

		string_update11 = string_update11 + string_update11_sub;

		string_update12_1 = string_update12_1 + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm16["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
			+ tpssm16["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm16["END_TIME"].ToString() + "' AS END_TIME, '"
			+ tpssm16["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm16["PROC_NO"].ToString() + "' AS PROC_NO, '"
			+ tpssm16["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm16["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL ) B "
			" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";
		string_update12_1_sub = string_update12_1_sub + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm16["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
			+ tpssm16["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm16["END_TIME"].ToString() + "' AS END_TIME, '"
			+ tpssm16["DEV_CODE"].ToString() + "' AS DEV_CODE, '" + tpssm16["PROC_NO"].ToString() + "' AS PROC_NO, '"
			+ tpssm16["REC_REVISE_TIME"].ToString() + "' AS REC_REVISE_TIME, '" + tpssm16["REC_REVISOR"].ToString() + "' AS REC_REVISOR FROM DUAL ) B "
			" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";

		string_update12_1 = string_update12_1 + string_update12_1_sub;

		/*string_update12_2 = string_update12_2 + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
		+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL ) B "
		" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";
		string_update12_2_sub = string_update12_2_sub + " SELECT 'XXXXXXXX' AS SM_PLAN_NO, " + tpssm12["CHARGE_NO"].ToString() + " AS CHARGE_NO, '"
		+ tpssm12["START_TIME"].ToString() + "' AS START_TIME, '" + tpssm12["END_TIME"].ToString() + "' AS END_TIME FROM DUAL ) B "
		" WHERE A.SM_PLAN_NO = B.SM_PLAN_NO AND A.CHARGE_NO = B.CHARGE_NO) ";

		string_update12_2 = string_update12_2 + string_update12_2_sub;*/

		cmd_inq.SetCommandText(string_update12_1);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		/*cmd_inq.SetCommandText(string_update12_2);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();*/

		cmd_inq.SetCommandText(string_update11);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		//Log::Trace("", __FUNCTION__, "string_update11 = [{0}]", string_update11);
		//Log::Trace("", __FUNCTION__, "string_update12_1 = [{0}]", string_update12_1);
		//Log::Trace("", __FUNCTION__, "string_update12_2 = [{0}]", string_update12_2);

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
/************************************************************
根据条件内容，生成"路径","路径关联关系","设备区分"
注: 路径        pono_route, == model_dev_code
设备区分    route_div   == model_dev_type
************************************************************/
int GenTpsIn_route_create(const char * pono, char *pono_route, char *route_relaion, char *route_div, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);
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
		pono_route[0] = '0';
		pono_route[1] = '0';
		route_relaion[0] = '0';
		route_div[0] = '0';

		sqlstr = "SELECT T1.PONO,T2.AREA_ID,T2.DEV_CODE,T2.CHARGE_NO,T3.DEV_TECH_CODE FROM TPSSM11 T1, TPSSM12 T2,TPSSMD1 T3 ";
		sqlstr += "WHERE T1.PONO=@pono AND T1.SM_PLAN_NO = T2.SM_PLAN_NO AND T2.DEV_CODE=T3.DEV_CODE AND T2.AREA_ID=T3.AREA_ID AND T2.FACTORY_DIV=T3.FACTORY_DIV ORDER BY T2.CHARGE_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("pono", pono);
		cmd_inq.ExecuteQuery(dev_info);
		for (i = 0, j = 2, n = 1; i<dev_info.Rows.get_Count(); i++)
		{
			if (dev_info.Rows[i]["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				dev_info.Rows[i]["DEV_TECH_CODE"] = "E";
			}
			if (dev_info.Rows[i]["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				dev_info.Rows[i]["DEV_TECH_CODE"] = "B";
			}

			//将找到的 dev_code 转换为模型识别的代码	
			pono_route[j] = dev_info.Rows[i]["DEV_CODE"].ToString()[0];
			pono_route[j + 1] = dev_info.Rows[i]["DEV_CODE"].ToString()[1];
			route_relaion[n] = '0';

			route_div[n] = dev_info.Rows[i]["DEV_TECH_CODE"].ToString()[0];
			n = n + 1;
			j = j + 2;


		}
		pono_route[j + 1] = '\0';
		route_relaion[n + 1] = '\0';
		route_div[n + 1] = '\0';
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

int GenTpsIn_route_createb(const char * pono, char *pono_route, char *route_relaion, char *route_div, CDbConnection * conn)
{
	//CTracer log(__FUNCTION__);
	int k = 0;//各相关路径的下标索引
	int doFlag = 0;
	int  i, j, n;
	CModel tpssm15("TPSSM15");
	CModel tpssm16("TPSSM16");
	CModel tpssmd1("TPSSMD1");
	CDbCommand cmd_inq(conn);
	CString sqlstr = "";
	CDataTable dev_info("DEV_INFO");
	try
	{
		//设置铁水预处理的相关路径 default
		//k=0;
		pono_route[0] = '0';
		pono_route[1] = '0';
		route_relaion[0] = '0';
		route_div[0] = '0';

		sqlstr = "SELECT T1.PONO,T2.AREA_ID,T2.DEV_CODE,T2.CHARGE_NO,T3.DEV_TECH_CODE FROM TPSSM15 T1, TPSSM16 T2,TPSSMD1 T3 ";
		sqlstr += "WHERE T1.PONO=@pono AND T1.SM_PLAN_NO = T2.SM_PLAN_NO AND T2.DEV_CODE=T3.DEV_CODE AND T2.AREA_ID=T3.AREA_ID AND T2.FACTORY_DIV=T3.FACTORY_DIV AND T2.DEV_CODE NOT IN ('Z1','Z2','Z3','Z4','Z5','Z6','Z7','Z8') ORDER BY T2.CHARGE_NO";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("pono", pono);
		cmd_inq.ExecuteQuery(dev_info);
		for (i = 0, j = 2, n = 1; i<dev_info.Rows.get_Count(); i++)
		{
			if (dev_info.Rows[i]["DEV_TECH_CODE"].ToString().Trim() == "X")
			{
				dev_info.Rows[i]["DEV_TECH_CODE"] = "E";
			}
			if (dev_info.Rows[i]["DEV_TECH_CODE"].ToString().Trim() == "Y")
			{
				dev_info.Rows[i]["DEV_TECH_CODE"] = "B";
			}

			//将找到的 dev_code 转换为模型识别的代码	
			pono_route[j] = dev_info.Rows[i]["DEV_CODE"].ToString()[0];
			pono_route[j + 1] = dev_info.Rows[i]["DEV_CODE"].ToString()[1];
			route_relaion[n] = '0';

			route_div[n] = dev_info.Rows[i]["DEV_TECH_CODE"].ToString()[0];
			n = n + 1;
			j = j + 2;


		}
		pono_route[j + 1] = '\0';
		route_relaion[n + 1] = '\0';
		route_div[n + 1] = '\0';
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


/* **************************************************
*函数介绍：	在指定的文件中读取一行数据，并按制表符和空格截取
*输入参数：	max表示一行数据的最大容量，fileptr表示文件句柄指针
*输出参数：	readstr表示截取后获取的字符串
*返回值	：	非0值表示未找到匹配的设备
************************************************** */
void readfile(char *readstr, int max, FILE *fileptr)
{
	int i;
	char tmp[1000];
	fgets(tmp, max, fileptr);
	max = (int)strlen(tmp);
	for (i = 0; i<max; i++)
	{
		if (tmp[i] == '\t' || tmp[i] == ' ' || tmp[i] == '\n')
		{
			tmp[i] = '\0';
			strncpy(readstr, tmp, i + 1);
			break;
		}
	}
}
void readfile_l(char *readstr, int max, FILE *fileptr)
{
	int i;
	char tmp[1000];
	fgets(tmp, max, fileptr);
	max = (int)strlen(tmp);
	for (i = 0; i<max; i++)
	{
		if (tmp[i] == '\n')
		{
			tmp[i] = '\0';
			strncpy(readstr, tmp, i + 1);
			break;
		}
	}
}




