/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Date:     2015-07-16
Version:  3.1.0
Description:  PES系统接受对MMS系统下发的板坯制造命令进行接收
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件



#include "epex.h"

// service入口
BM2F_ENTERACE_TELE(cm_002022_rcv)

int f_cm_002022_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int    doFlag = 0;
	CString   datetime = "";
	int    dummy = 0;
	int    blkseq = 0;

	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");

	CDbCommand cmd_inq(conn);
	CString sqlstr = "";

	try
	{
		/* ***** 取系统时间 ***** */
		tpssm03["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//责任者
		tpssm03["REC_CREATOR"] = "XCOM";

		/* ***** 获取输入参数 ***** */
		tpssm03.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		//Log::Info("", __FUNCTION__, "tpssm03["SLAB_MIN_LEN"] =[{0}]", tpssm03["SLAB_MIN_LEN"].ToDecimal().ToInt32());
		//Log::Info("", __FUNCTION__, "tpssm03["SLAB_GROUP_NO"] =[{0}]", tpssm03["SLAB_GROUP_NO"].ToDecimal().ToInt32());
		//Log::Info("", __FUNCTION__, "tpssm03["SLAB_NUM"] =[{0}]", tpssm03["SLAB_NUM"].ToDecimal());
		//Log::Info("", __FUNCTION__, "tpssm03["SLAB_LEN"] =[{0}]", tpssm03["SLAB_LEN"].ToDecimal());
		//Log::Info("", __FUNCTION__, "tpssm03["SLAB_MAX_LEN"] =[{0}]", tpssm03["SLAB_MAX_LEN"].ToDecimal());
		//Log::Info("", __FUNCTION__, "tpssm03["SLAB_MIN_LEN"] =[{0}]", tpssm03["SLAB_MIN_LEN"].ToDecimal());


		//铸坯命令只有新增。删除随炉次命令删		

		//删除原铸坯
		sqlstr = "tpssm03.Delete(SLAB_NO)";
		tpssm03.Delete("SLAB_NO");

		//长坯号为空时，板坯号赋给长坯号
		if (tpssm03["LSLAB_NO"].ToString().Trim() == "")
		{
			tpssm03["LSLAB_NO"] = tpssm03["SLAB_NO"];
		}

		//新增铸坯
		sqlstr = "tpssm03.Insert()";
		tpssm03.Insert();

		//更新浇铸计划表
		tpssm10["FACTORY_DIV"] = tpssm03["FACTORY_DIV"];
		tpssm10["PONO"] = tpssm03["PONO"];
		tpssm10["SLAB_THICK"] = tpssm03["SLAB_THICK"];
		tpssm10["SLAB_WIDTH"] = tpssm03["SLAB_WIDTH"];
		tpssm10["SLAB_LEN"] = tpssm03["SLAB_LEN"];
		sqlstr = "tpssm10.Update()";
		tpssm10.Update(
			"SLAB_THICK,"
			"SLAB_WIDTH,"
			"SLAB_LEN",
			"FACTORY_DIV,PONO");

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
