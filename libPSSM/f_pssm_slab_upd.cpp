/*=========================================================================
//程序名称:     f_psbw_slab_upd
//隶属子系统:   PSBW
//产品名称:     BSM1
//创建人员:     
//创建时间:     2010-4-19
//修改人员:     
//修改日期:     
//=========================================================================*/
#include "stdafx.h"






BM2_FUNCTION_EXPORT
 int f_pssm_slab_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	
	//程序用变量
	int doFlag = 0;
	int fetchRowCount = 0;
	int blknum = 0;
	CDecimal v_cnt =0;                  /* 计数 */


	EIClass inBlock;
    EIClass outBlock;
	CString slab_no;
	CString v_slab_no;

	CString sqlstr = "";
	CDbCommand cmd_tpssm03_inq(conn);


	// 定义表的实体对象
	CModel tpssm03("TPSSM03");
	CModel tpssm01("TPSSM01");

	try
	{
		//获得输入参数
		blknum = bcls_rec->Tables.IndexOf("PSSM"); 
		if (blknum < 0) 
		{
			sprintf(s.sysmsg, "没有找到数据块[PSSM].");
			strcpy(s.msg, _RES("GCRSS0000013")/*没有满足条件的记录。*/ );
			throw CApplicationException(-1, s.msg, log.Location);
		}

 		

		//获得输入参数
		slab_no			= bcls_rec->Tables["PSSM"].Rows[0]["SLAB_NO"];//功能号（对应计划状态）


		////Log::Trace("", __FUNCTION__, "===trace = [{0}]",slab_no);

		//截取前十位
		v_slab_no = slab_no.Substring(0,10);
	
		////Log::Trace("", __FUNCTION__, "===trace = [{0}]",v_slab_no);

      //读取TPSSM03表，查询HOT_SEND_FLAG

			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
				sqlstr = " SELECT COUNT(*) FROM TPSSM03 "
						 " WHERE substr(SLAB_NO,0,10) = @slab_no ";
				break;
			}
			cmd_tpssm03_inq.SetCommandText( sqlstr );
			cmd_tpssm03_inq.Parameters.Set("slab_no",v_slab_no);
			v_cnt = cmd_tpssm03_inq.ExecuteScalar();

			if(v_cnt==0)
			{
				//没有材料
			}else
			{
				//更新热装热送标记
				switch(conn->DatabaseKind)
				{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
					sqlstr = " SELECT HOT_CHARGE_FLAG,SLAB_NO,PONO FROM TPSSM03 "
							 " WHERE substr(SLAB_NO,0,10) = @slab_no ";
					break;
				}
				cmd_tpssm03_inq.SetCommandText( sqlstr );
				cmd_tpssm03_inq.Parameters.Set("slab_no",v_slab_no);
				cmd_tpssm03_inq.ExecuteReader();

				while(cmd_tpssm03_inq.Read())
				{
					tpssm03["HOT_CHARGE_FLAG"] = cmd_tpssm03_inq.GetString(1);
					tpssm03["SLAB_NO"] =cmd_tpssm03_inq.GetString(2); 
					tpssm03["PONO"] = cmd_tpssm03_inq.GetString(3); 
					
					if(tpssm03["HOT_CHARGE_FLAG"].ToString() =="2")
					{
						//更新为0
						tpssm03["HOT_CHARGE_FLAG"] ="0";
						
						tpssm03.Update("HOT_CHARGE_FLAG","SLAB_NO");
					}
				}

				

			}

			cmd_tpssm03_inq.Close();

			//更新01表的热装标记
			switch(conn->DatabaseKind)
			{
				case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
				case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
				case DB_KIND_MSSQL:	        // MS SQL Server数据库
				case DB_KIND_ORACLE:	    // Oracle 数据库
				default: // 所有数据库适用，通用SQL语句
				sqlstr = " SELECT MAX(HOT_CHARGE_FLAG) FROM TPSSM03 "
						 " WHERE PONO = @pono ";
				break;
			}
			cmd_tpssm03_inq.SetCommandText( sqlstr );
			cmd_tpssm03_inq.Parameters.Set("pono",tpssm03["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();

			if(cmd_tpssm03_inq.Read())
			{
				tpssm01["HOT_CHARGE_FLAG"]= cmd_tpssm03_inq.GetString(1);
				tpssm01["PONO"]=tpssm03["PONO"];
				tpssm01.Update("HOT_CHARGE_FLAG","PONO");
			}
			cmd_tpssm03_inq.Close();

			
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch(const CApplicationException& ex)
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

	return(doFlag);
}

