/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-13
Version:1.0
Description: 切断实绩接收,命令板坯置产出标记
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


//#include "tmmsm38.h"

/* =========================================================================
/// <remark>
/// <summary>
/// 切断实绩接收,命令板坯置产出标记
/// 1-产出,  2-部分产出
/// <para>  </para>
/// <para>  </para>
/// </summary>
/// <param name= "xxxx">输入参数</param>
/// <returns>成功:0</returns>
/// <returns>失败:-1</returns>
/// </remark>
========================================================================= */
// 函数入口
BM2_FUNCTION_EXPORT
 int f_pssm03_prodflag_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int blkseq, rows, i;
	CDecimal    dummy=0;

	CDecimal  sum_slab_wt  = 0;     //已有铸坯总重量
	CDecimal  sum_mat_tube = 0;     //已有铸坯总支数
	CDecimal  mat_cut_num = 0;     //消耗铸坯总支数
	CString sqlstr;
	CModel tpssm03("TPSSM03");
	CModel tmmsm33("TMMSM33");
	//CTMMSM38 tmmsm38(conn);
	CDbCommand cmd_inq(conn);

	int  v_cc_flag = 0;

	try
	{
		/* 获得输入参数 */
		blkseq = bcls_rec->Tables.IndexOf("PSSM03");
		if (blkseq < 0) 
		{
			strcpy(s.msg, _RES("GCRSS0000011")/*系统出现异常，数据块有误，请联系系统维护人员。*/);
			throw CApplicationException(-1, s.msg, log.Location);
		}

		/* 对输入信息循环处理 */
		rows = bcls_rec->Tables[blkseq].Rows.get_Count();
		for (i = 0; i < rows; i++ )
		{

			tpssm03["FACTORY_DIV"] = bcls_rec->Tables[blkseq].Rows[i]["FACTORY_DIV"];
			tpssm03["SLAB_NO"] = bcls_rec->Tables[blkseq].Rows[i]["SLAB_NO"];        //板坯号
			tpssm03["SLAB_PROD_FLAG"] = bcls_rec->Tables[blkseq].Rows[i]["SLAB_PROD_FLAG"]; //板坯产出标记: 0-未产出，1-产出， 2-部分产出
			tpssm03["SLAB_NUM"] = bcls_rec->Tables[blkseq].Rows[i]["SLAB_NUM"];

			////Log::Trace("", __FUNCTION__, "FACTORY_DIV=[{0}]", tpssm03.FACTORY_DIV );
			////Log::Trace("", __FUNCTION__, "SLAB_NO=[{0}]", tpssm03.SLAB_NO );
			////Log::Trace("", __FUNCTION__, "SLAB_PROD_FLAG=[{0}]", tpssm03.SLAB_PROD_FLAG );
			////Log::Trace("", __FUNCTION__, "SLAB_NUM=[{0}]", tpssm03["SLAB_NUM"].ToDecimal());

			
			
			//2.获取计划数量
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:           // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:        // Oracle 数据库
			default:
				//读取铸坯实绩, 总支数、重量、最大报送数
				sqlstr = CString(
					" SELECT SLAB_NUM,SLAB_CUT_NUM,SLAB_WT "
					"   FROM TPSSM03 "
				//	"  WHERE FACTORY_DIV = @factory_div "
					"    WHERE SLAB_NO           = @slab_no "
					);
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("factory_div", tpssm03["FACTORY_DIV"].ToString());
			cmd_inq.Parameters.Set("slab_no", tpssm03["SLAB_NO"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				sum_mat_tube = cmd_inq.GetDecimal(1);
				tpssm03["SLAB_CUT_NUM"] = cmd_inq.GetDecimal(2);
				sum_slab_wt = cmd_inq.GetDecimal(3);
			}
			else
			{
				//没有该命令铸坯，就退出，不用修改
				return 0;
			}
			cmd_inq.Close();

				//根据计划数量和产出数量，确定命令铸坯的产出状态
				if (tpssm03["SLAB_PROD_FLAG"].ToString().Trim() == "1")
				{
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() + tpssm03["SLAB_NUM"].ToDecimal() == sum_mat_tube)//剩余命令为0，产出
					{
						tpssm03["SLAB_PROD_FLAG"] = "1"; //1-产出
					}
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() + tpssm03["SLAB_NUM"].ToDecimal() < sum_mat_tube)//剩余命令不为0，部分产出
					{
						tpssm03["SLAB_PROD_FLAG"] = "2"; //2-部分产出
					}
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() + tpssm03["SLAB_NUM"].ToDecimal() ==0)//全部未产出
					{
						tpssm03["SLAB_PROD_FLAG"] = "0"; //2-部分产出
					}
					tpssm03["SLAB_CUT_NUM"] = tpssm03["SLAB_CUT_NUM"].ToDecimal() + tpssm03["SLAB_NUM"].ToDecimal();
				}
				if (tpssm03["SLAB_PROD_FLAG"].ToString().Trim() == "0")
				{
					CDecimal  v_rs = 0;     
					v_rs = tpssm03["SLAB_CUT_NUM"].ToDecimal() - tpssm03["SLAB_NUM"];
					////Log::Trace("", __FUNCTION__, "SLAB_CUT_NUM=[{0}]SLAB_NUM=[{1}]", tpssm03["SLAB_CUT_NUM"].ToDecimal(), tpssm03["SLAB_NUM"].ToDecimal());
					////Log::Trace("", __FUNCTION__, "v_rs=[{0}]", v_rs);
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() - tpssm03["SLAB_NUM"].ToDecimal() >0)//回退总和少于计划，部分产出
					{
						tpssm03["SLAB_PROD_FLAG"] = "2"; //2-部分产出
					}
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() - tpssm03["SLAB_NUM"].ToDecimal() == 0)//回退总和等于计划，未产出
					{
						tpssm03["SLAB_PROD_FLAG"] = "0"; //0-未产出
					}
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() - tpssm03["SLAB_NUM"].ToDecimal() == sum_mat_tube)//
					{
						tpssm03["SLAB_PROD_FLAG"] = "1"; //1-产出
					}
					tpssm03["SLAB_CUT_NUM"] = tpssm03["SLAB_CUT_NUM"].ToDecimal() - tpssm03["SLAB_NUM"];
					if (tpssm03["SLAB_CUT_NUM"].ToDecimal() < 0)
					{
						tpssm03["SLAB_CUT_NUM"] = 0;
					}
				}
				
				////Log::Trace("", __FUNCTION__, "sum_mat_tube=[{0}]", sum_mat_tube);
				////Log::Trace("", __FUNCTION__, "tpssm03.SLAB_NUM    =[{0}]", tpssm03["SLAB_NUM"].ToDecimal());
				////Log::Trace("", __FUNCTION__, "tpssm03.SLAB_CUT_NUM    =[{0}]", tpssm03["SLAB_CUT_NUM"].ToDecimal());
				////Log::Trace("", __FUNCTION__, "计算后: SLAB_PROD_FLAG =[{0}]", tpssm03["SLAB_PROD_FLAG"].ToString());
				
				tpssm03.Update("SLAB_PROD_FLAG,SLAB_CUT_NUM","SLAB_NO");
				//读取铸坯实绩, 总支数、重量、最大报送数
				sqlstr = CString(
					" update TPSSM03 set SLAB_PROD_FLAG =@slab_prod_flag,SLAB_CUT_NUM = @slab_cut_num "
					"    WHERE SLAB_NO           = @slab_no "
					);

				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("slab_prod_flag", tpssm03["SLAB_PROD_FLAG"].ToString());
				cmd_inq.Parameters.Set("slab_cut_num", tpssm03["SLAB_CUT_NUM"].ToDecimal());
				cmd_inq.Parameters.Set("slab_no", tpssm03["SLAB_NO"].ToString());
				cmd_inq.ExecuteNonQuery();
				////Log::Trace("", __FUNCTION__, "tpssm03.Update");

	/*		tpssm03.Update("SLAB_PROD_FLAG,SLAB_CUT_NUM","FACTORY_DIV,SLAB_NO");*/

		}

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
