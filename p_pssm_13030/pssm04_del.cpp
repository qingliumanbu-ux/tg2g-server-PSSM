//框架公用头文件，勿删
#include "stdafx.h"
 
/// <summary > 
/// Description: 炼钢连铸计划预收池删除
/// Copyright: Baosight Software LTD.co Copyright (c) 2010
/// Company: 上海宝信软件股份有限公司
/// Author:   涂献计
/// Version: 1.0
/// History:
 
/// </summary >  


//程序用头文件，请包含在""中
#include "tpssm01.h"

// service入口，pssm04_del为service名称
BM2F_ENTERACE(pssm04_del)

//service对应函数定义，确保函数名称为："f_" + "service名称"
int f_pssm04_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	
    //返回值，勿删
	int doFlag = 0;
	try
	{

		CTPSSM01 tpssm01(conn);
		int i = 0;
		for(; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tpssm01.Delete();
		}

	
	}
	catch(CException& ex)  //用于捕获数据库操作异常
	{		
		strcpy(s.msg, ex.GetMsg());  //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应
		s.flag = ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
 
	//返回-1时事务将回滚，返回为0是事务将提交
	return doFlag;
}