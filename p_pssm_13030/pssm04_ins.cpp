//框架公用头文件，勿删

#include "stdafx.h"

 

/// <summary>

/// Description: 炼钢连铸计划预收池新增

/// Copyright: Baosight Software LTD.co Copyright (c) 2010

/// Company: 上海宝信软件股份有限公司

/// Author:   涂献计

/// Version: 1.0

/// History:

 

/// </summary> 





// service入口，pssm04_ins为service名称

BM2F_ENTERACE(pssm04_ins);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪
int f_pmom_status_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

//service对应函数定义，确保函数名称为："f_" + "service名称"

int f_pssm04_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 

{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);
	//返回值，勿删

	int  doFlag = 0;
	int  ret = 0;
	EIClass inBlock;  //调用调用生产的接口参数
	EIClass inBlock99; //调用炼钢履历跟踪
	//将业务代码包含在try-catch结构中

	try
	{
		//----------------------------------------------
		//声明调用连铸机校验的接口参数
		inBlock.Tables[0].set_TableName("PSSM01");
		inBlock.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CC_MACH_NO");
		inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");

		//声明调用生产的接口参数
		inBlock.Tables.Add("PSSMCHS");
		inBlock.Tables["PSSMCHS"].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock.Tables["PSSMCHS"].Columns.Add(DT_STRING, "PONO");

		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		// 定义表的实体对象
	CModel tpssm01("TPSSM01");

		int count=bcls_rec->Tables[0].Rows.get_Count();

		for(int i=0;i<count;i++)
		{
			//重置表结构变量
			tpssm01.Reset();

			// 取得单行传入信息 
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//通过print方法可以将结构信息输出至trace中，与EPLogInfo功能类似
			tpssm01.Print();

			//数据校验等操作可在此进行
			if(tpssm01["PONO"].ToString() == " ")
			{
				strcpy(s.msg,"主键为空,请重新输入");
				s.flag  = -1;                 
				return doFlag;
			}

			CDbCommand cmd1("select count(*) from PSSM.TPSSM01 where PONO=@PONO",conn);

			cmd1.Parameters.Set("PONO",tpssm01["PONO"].ToString());

			/*cmd1.Parameters.Set("ST_NO",tpssm01["ST_NO"].ToString());
			cmd1.Parameters.Set("CC_MACH_NO",tpssm01["CC_MACH_NO"].ToString());
			cmd1.Parameters.Set("STEEL_APP_DATE",tpssm01["STEEL_APP_DATE"].ToString());
			cmd1.Parameters.Set("MAT_DESTION",tpssm01.MAT_DESTION);
			cmd1.Parameters.Set("FLAME_CLEAN",tpssm01.FLAME_CLEAN);
			cmd1.Parameters.Set("HOT_CHARGE_FLAG",tpssm01["HOT_CHARGE_FLAG"].ToString());
			cmd1.Parameters.Set("HOT_SEND_FLAG",tpssm01["HOT_SEND_FLAG"].ToString());
			cmd1.Parameters.Set("APPLY_CC_MACH",tpssm01.APPLY_CC_MACH);
			cmd1.Parameters.Set("CAST_LOT_DIV_NO",tpssm01["CAST_LOT_DIV_NO"].ToDecimal());
			cmd1.Parameters.Set("CAST_LOT_NO",tpssm01["CAST_LOT_NO"].ToString());
			cmd1.Parameters.Set("CAST_LOT_SUM",tpssm01["CAST_LOT_SUM"].ToDecimal());
			cmd1.Parameters.Set("REFINE_ROUTE_CODE",tpssm01.REFINE_ROUTE_CODE);*/

			CDecimal con = (cmd1.ExecuteScalar()).ToInt32();

			if(con>1)
			{
				strcpy(s.msg, "数据已存在,请重新输入"); 
				s.flag  = -1;                 
				return doFlag;
			}

			//新增记录
			tpssm01.Insert();

			//调用生产函数的数据准备
			CDataRow &row = inBlock.Tables["PSSMCHS"].Rows.Add();
			row["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			row["PONO"] = tpssm01["PONO"];

			//炼钢履历跟踪
			CDataRow &row99 = inBlock99.Tables["TRACE"].Rows.Add();
			row99["EVENT_ID"] = "11"; //编入计划
			row99["FACTORY_DIV"] = tpssm01["FACTORY_DIV"];
			row99["CAST_LOT_NO"] = tpssm01["CAST_LOT_NO"];
			row99["PONO"] = tpssm01["PONO"];
			row99["PONO_STATUS"] = tpssm01["PONO_STATUS"];
		}

		//-----------------------------------------------
		//调用生产模块函数(材料申请状态置炼钢计划编入)
		ret = f_pmom_status_upd(&inBlock, bcls_ret, conn);
		//ret = 0;
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//-----------------------------------------------
		//炼钢履历跟踪
		ret = f_pssm99_trace(&inBlock99, bcls_ret, conn);
		if (ret < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
				

	}

	catch(const CApplicationException& ex)

	{

		doFlag = ex.GetCode();

		strcpy(s.msg, (const char*)ex.GetMsg());

	}

	catch(CException& ex)  //用于捕获数据库操作异常

	{		

		strcpy(s.msg, ex.GetMsg());  //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.msg参数对应

		s.flag  =  ex.GetCode();       //返回前台，与EI.EIManager.Instance.CallService(v_curr_part_name,)方法返回的EI.EIInfo对象的sys_info.flag参数对应

		doFlag  =  -1;                 //数据库异常时返回-1，事务将被回滚

	}

 

	//返回-1时事务将回滚，返回为0是事务将提交

	return(doFlag);

}
