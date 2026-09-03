/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   顾东亮
Version:    1.0
Date:     2011-12-26
Description:	 PONO换机浇注的可行性校验。
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


//程序用头文件





/*<remark>=========================================================
/// <summary>
/// PONO换机浇注的可行性校验
/// <para>1.PONO与交换的连铸机类型检查: 可浇注的坯型, 连铸机的流数；通过连铸机设备参数表(TPSSMD9)来判断。</para>
/// <para>2.连铸机类型相同时, 检查PONO1的浇注的钢坯类型、浇铸流数和规格能否在指定连铸机上浇注。</para>
/// <para>3.浇注规格的检查, 只检查厚,宽。                           </para>
/// <para>数据库表：TPSSMC1(连铸参数表)                             </para>
/// <para>主调用函数：pssm18_chg_out() 函数调用。                </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码     </param>
/// <param name="pono">制造命令号                    </param>
/// <param name="cc_mach_no">目的连铸机号            </param>
/// <returns>diff_type：判断PONO所属的连铸机与指定的机连铸机是否为同种连铸机, =0 同种, >0 不同的</returns>
===========================================================</remark>*/
BM2_FUNCTION_EXPORT
 int f_pssm_ccmchg_chk_n(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i=0;
	int    dummy=0;
	int    diff_type=0;       //判断PONO所属的连铸机与指定的机连铸机是否为同种连铸机, =0 同种, >0 不同的
	CString   cc_mach_no;   /* 指定的连铸机号 */

	CModel tpssmd9("TPSSMD9");
	CModel tpssmd9_new("TPSSMD9");
	CModel tpssm02("TPSSM02");
	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");
	CDbCommand cmd_inq(conn);
	CString sqlstr;

	try
	{

		//设置返回的参数
		if (!bcls_ret->Tables[0].Columns.Contains("DIFF_TYPE"))
		{
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "DIFF_TYPE");
		}
		bcls_ret->Tables[0].Rows.Clear();

		//获得输入参数
		tpssm10["FACTORY_DIV"]=bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"].ToString();
		tpssm10["PONO"]=bcls_rec->Tables[0].Rows[0]["PONO"].ToString();
		cc_mach_no=bcls_rec->Tables[0].Rows[0]["CC_MACH_NO"].ToString();

		
		//读取要交换的PONO的浇注顺内容
		tpssm10.Query("FACTORY_DIV,PONO");
		tpssm10.TrimOrBlank();


		/* -------------------------------------------------
		* 判断是否是同种连铸机,以区分是否为产线间的钢种变更
		* 即检查PONO1换到PONO2的连铸机上和PONO2换到PONO1的连铸机上是否可行
		*/
		//PONO1与PONO2所在连铸机的类型检查
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
			sqlstr="SELECT COUNT(1) FROM (SELECT * FROM TPSSMD9 WHERE FACTORY_DIV=@tpssm10.FACTORY_DIV ";
			sqlstr+=CString(" AND CC_MACH_NO=@tpssm10.CC_MACH_NO) a, ");
			sqlstr+=CString(" (SELECT * FROM TPSSMD9 WHERE FACTORY_DIV=@tpssm10.FACTORY_DIV  AND CC_MACH_NO=@cc_mach_no) b");
			sqlstr+=CString(" WHERE a.BILLET_TYPE=b.BILLET_TYPE AND a.STRAND_NUM=b.STRAND_NUM");
		break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tpssm10.FACTORY_DIV",tpssm10["FACTORY_DIV"].ToString());
		cmd_inq.Parameters.Set("tpssm10.CC_MACH_NO",tpssm10["CC_MACH_NO"].ToString());
		cmd_inq.Parameters.Set("cc_mach_no",cc_mach_no);
		dummy=cmd_inq.ExecuteScalar().ToInt32();
		if (dummy==0 )
		{
			diff_type=1;//连铸机类型不同
		}
		else //2台连铸机的浇注参数相同
		{
			//1) 检查PONO的浇注的钢坯类型、浇铸流数和规格能否在指定的连铸机上浇注
			// 还要检查这些数据项是由于有些连铸机是一机多能,即可浇板坯,也可浇方坯;单从连铸机类型判断不出

			// 取PONO的浇注钢坯类型和浇注的流数
			tpssm02["FACTORY_DIV"]=tpssm10["FACTORY_DIV"];
			tpssm02["CAST_LOT_NO"]=tpssm10["CAST_LOT_NO"];
			tpssm02.Query("FACTORY_DIV,CAST_LOT_NO");
			tpssm02.TrimOrBlank();

			tpssmd9_new["FACTORY_DIV"]=tpssm10["FACTORY_DIV"];
			tpssmd9_new["CC_MACH_NO"]=cc_mach_no;
			tpssmd9_new["CAST_THICK"] = tpssm02["SLAB_THICK"];
			tpssmd9_new["BILLET_TYPE"]=tpssm02["BILLET_TYPE"];

			dummy=tpssmd9_new.QueryCount("FACTORY_DIV,CC_MACH_NO,BILLET_TYPE,CAST_THICK");//STRAND_NUM
			if (dummy == 0)//不能浇注
			{
				diff_type = 2; //指定的连铸机不能浇注PONO钢坯
			}
			else //能浇
			{
				//连铸机规格相同时,判断规格是否允许
				//读取指定连铸机的参数信息

				Log::Trace("", __FUNCTION__, "canshu[{0}],[{1}],[{2}],[{3}]", tpssmd9_new["FACTORY_DIV"].ToString(), tpssmd9_new["CC_MACH_NO"].ToString(), tpssmd9_new["CAST_THICK"].ToString(), tpssmd9_new["BILLET_TYPE"].ToString());
				tpssmd9_new.Query("FACTORY_DIV,CC_MACH_NO,BILLET_TYPE,CAST_THICK");//STRAND_NUM
				tpssmd9_new.TrimOrBlank();

				//浇注规格的检查, 只检查厚,宽
				switch(conn->DatabaseKind)
				{
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_ORACLE:	        // Oracle 数据库
					default: // 所有数据库适用，通用SQL语句
					sqlstr="SELECT MIN(SLAB_THICK),MIN(SLAB_WIDTH), MAX(SLAB_WIDTH) FROM TPSSM03 WHERE 1=1";
					sqlstr+=CString(" AND FACTORY_DIV=@tpssm10.FACTORY_DIV AND PONO = @tpssm10.PONO");
					break;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("tpssm10.FACTORY_DIV", tpssm10["FACTORY_DIV"].ToString());
				cmd_inq.Parameters.Set("tpssm10.PONO", tpssm10["PONO"].ToString());
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tpssmd9["CAST_THICK"]=cmd_inq.GetDecimal(1);
					tpssmd9["CAST_WIDTH_MIN"]=cmd_inq.GetDecimal(2);
					tpssmd9["CAST_WIDTH_MAX"]=cmd_inq.GetDecimal(3);
				}
				cmd_inq.Close();
				//超出规格的, 不能到指定连铸机下浇注
				if ( tpssmd9["CAST_THICK"].ToDecimal() != tpssmd9_new["CAST_THICK"].ToDecimal() ||
					tpssmd9["CAST_WIDTH_MIN"] < tpssmd9_new["CAST_WIDTH_MIN"].ToDecimal() ||
					tpssmd9["CAST_WIDTH_MAX"] > tpssmd9_new["CAST_WIDTH_MAX"].ToDecimal()
					)
				{
					diff_type = 3; //指定的连铸机不能浇注PONO钢坯规格
				}//if 规格检查

			}//if 浇注类型检查

		}//if 铸机类型检查
		CDataRow &row = bcls_ret->Tables[0].Rows.Add();
		row["DIFF_TYPE"]=diff_type;
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
