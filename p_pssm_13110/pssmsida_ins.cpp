/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   向萍
Version:  1.0
Date:     2015-04-13
Description: 新增炼钢工序_连铸机拉速参数表	
**************************************************************************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件


/*<remark>=========================================================
/// <summary>
/// 新增炼钢工序_连铸机拉速参数表
/// <para>
/// 新增炼钢工序_连铸机拉速参数表
/// </para>
/// <para>数据库表：TPSSMDA(连铸机拉速参数表) </para>
/// <para>主调用函数：前台PSSMDA画面F3 新增调用。    </para>
/// </summary>
/// <param name="factory_div">炼钢厂别代码    </param>
/// <param name="cc_mach_no">连铸机号				</param>
/// <param name="st_no">出钢记号				    </param>
/// <returns>连铸机拉速参数表</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssmsida_ins)

int f_pssmsida_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection* conn)
{

	CTracer log(__FUNCTION__);

	/* 程序用变量 */
	int   doFlag = 0;
	int   fetchRowCount = 0;
	int   dummy = 0;
	int   v_idx_no = 0;
	CDecimal v_idx_no1=0;


	/* 业务变量 */
	CString  dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString  v_factory_div = CString("");
	CString  sqlstr("");             


	/* 实体类定义 */
	CModel tpssmda("TPSSMDA");

	/*数据库操作类定义*/
	CDbCommand cmd_inq(conn);

	try
	{
		//获取前台输入数据
		tpssmda.MergeFrom(bcls_rec->Tables[0].Rows[0]);

	//	v_factory_div = bcls_rec->Tables[1].Rows[0]["FACTORY_DIV"];

		//////Log::Trace("", __FUNCTION__, "factory_div=[{0}]", v_factory_div);


		//非圆坯（板、方），输入的厚-宽度检查
		if (tpssmda["BILLET_TYPE"].ToString().Trim() != "4") //4-圆坯
		{
			if (tpssmda["CAST_WIDTH_MIN"].ToDecimal() == tpssmda["CAST_WIDTH_MAX"].ToDecimal())
			{
				CFormattable arguments[] = { tpssmda["CAST_WIDTH_MIN"].ToDecimal().ToInt32(), tpssmda["CAST_WIDTH_MAX"].ToDecimal().ToInt32() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, _RES("PSSMS0000054")/*输入的宽度最小值[{0}]与最大值[{1}]相同，请重新设定。*/, arguments, 2); //格式化字符串
				throw CApplicationException(-1, s.msg, log.Location);
			}


			//if (tpssmda["CAST_THICK_MIN"].ToDecimal() == tpssmda["CAST_THICK_MAX"].ToDecimal())
			//{
			//	CFormattable arguments[] = { tpssmda["CAST_THICK_MIN"].ToDecimal().ToInt32(), tpssmda["CAST_THICK_MAX"].ToDecimal().ToInt32() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, _RES("PSSMS0000052")/*输入的厚度最小值[{0}]与最大值[{1}]相同，请重新设定。*/, arguments, 2); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			//根据输入的连铸机号, 读取宽/厚度范围的检查
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT COUNT(1) FROM TPSSMDA "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND CC_MACH_NO = @cc_mach_no  "
					"	AND ST_NO = @st_no "
					"   AND (( CAST_WIDTH_MIN <= @cast_width_min   AND "
					"          CAST_WIDTH_MAX >  @cast_width_min )	OR "//宽度最小区间
					"        ( CAST_WIDTH_MIN <  @cast_width_max   AND "
					"          CAST_WIDTH_MAX >  @cast_width_max))     "//宽度最大区间
					"   AND (( CAST_THICK_MIN <= @cast_thick_min AND   "
					"          CAST_THICK_MAX >  @cast_thick_min )	OR "//厚度最小区间
					"        ( CAST_THICK_MIN <  @cast_thick_max AND   "
					"          CAST_THICK_MAX >  @cast_thick_max ))    "//厚度最大区间
					);
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("factory_div", tpssmda["FACTORY_DIV"].ToString());
			cmd_inq.Parameters.Set("cc_mach_no", tpssmda["CC_MACH_NO"].ToString());
			cmd_inq.Parameters.Set("st_no", tpssmda["ST_NO"].ToString());
			cmd_inq.Parameters.Set("cast_width_min", tpssmda["CAST_WIDTH_MIN"].ToDecimal());
			cmd_inq.Parameters.Set("cast_width_max", tpssmda["CAST_WIDTH_MAX"].ToDecimal());
			/*cmd_inq.Parameters.Set("cast_thick_min", tpssmda["CAST_THICK_MIN"].ToDecimal());
			cmd_inq.Parameters.Set("cast_thick_max", tpssmda["CAST_THICK_MAX"].ToDecimal());
*/

			dummy = cmd_inq.ExecuteScalar().ToInt32();

			//if (dummy > 0)
			//{
			//	CFormattable arguments[] = { tpssmda["CAST_WIDTH_MIN"].ToDecimal().ToInt32(), tpssmda["CAST_WIDTH_MAX"].ToDecimal().ToInt32(), tpssmda["CAST_THICK_MIN"].ToDecimal().ToInt32(), tpssmda["CAST_THICK_MAX"].ToDecimal().ToInt32() }; // 定义参数列表的数组
			//	//CMessageFormat::Format(s.msg, _RES("PSSMS0000053")/*输入的宽/厚度参数与表中的宽度范围[{0},{1}), 厚度范围[{2},{3})有重叠，请重新设定。*/, arguments, 4); //格式化字符串
			//	CMessageFormat::Format(s.msg, "输入的宽/厚度参数与表中的厚度范围[{2},{3}), 宽度范围[{0},{1})有重叠，请重新设定。", arguments, 4); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}


		}
		else  //圆坯
		{

			//if (tpssmda["CAST_THICK_MIN"].ToDecimal() == tpssmda["CAST_THICK_MAX"].ToDecimal())
			//{
			//	CFormattable arguments[] = { tpssmda["CAST_THICK_MIN"].ToDecimal().ToInt32(), tpssmda["CAST_THICK_MAX"].ToDecimal().ToInt32(), tpssmda["ST_NO"].ToString() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "输入的内部钢种[{2}]直径(厚度)最小值[{0}]与最大值[{1}]相同，请重新设定。", arguments, 3);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			//对于圆坯, 直径存储在厚度字段项，宽度值应为0
			tpssmda["CAST_WIDTH_MIN"] = 0;
			tpssmda["CAST_WIDTH_MAX"] = 0;

			//校验圆坯的直径范围是否重叠，根据输入的连铸机号, 读取厚度范围的检查
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT COUNT(1) FROM TPSSMDA "
					" WHERE FACTORY_DIV = @factory_div "
					"	AND CC_MACH_NO = @cc_mach_no  "
					"	AND ST_NO = @st_no "
					"   AND (( CAST_THICK_MIN <= @cast_thick_min AND   "
					"          CAST_THICK_MAX >  @cast_thick_min )	OR "//厚度最小区间
					"        ( CAST_THICK_MIN <  @cast_thick_max AND   "
					"          CAST_THICK_MAX >  @cast_thick_max ))    "//厚度最大区间
					);
				break;
			}

			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("factory_div", tpssmda["FACTORY_DIV"].ToString());
			cmd_inq.Parameters.Set("cc_mach_no", tpssmda["CC_MACH_NO"].ToString());
			cmd_inq.Parameters.Set("st_no", tpssmda["ST_NO"].ToString());
			/*cmd_inq.Parameters.Set("cast_thick_min", tpssmda["CAST_THICK_MIN"].ToDecimal());
			cmd_inq.Parameters.Set("cast_thick_max", tpssmda["CAST_THICK_MAX"].ToDecimal());*/
			
			dummy = cmd_inq.ExecuteScalar().ToInt32();

			//if (dummy > 0)
			//{
			//	CFormattable arguments[] = { tpssmda["CAST_THICK_MIN"].ToDecimal().ToInt32(), tpssmda["CAST_THICK_MAX"].ToDecimal().ToInt32() }; // 定义参数列表的数组
			//	CMessageFormat::Format(s.msg, "输入的圆坯直径(厚度)参数与表中的直径(厚度)范围[{0},{1})有重叠，请重新设定。", arguments, 2); //格式化字符串
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

		}


		//-----------------------------------------
		//取IDX_NO 
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = CString(" SELECT COUNT(1) FROM TPSSMDA "); 
			break;
		}

		cmd_inq.SetCommandText( sqlstr );

		v_idx_no = cmd_inq.ExecuteScalar().ToInt32();
		cmd_inq.Close();

		if(dummy = 0)
		{
			v_idx_no = 1 ;
		}
		else
		{
			//v_idx_no = v_idx_no + 1 ;
			//取最大值
			switch(conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT MAX(IDX_NO) FROM TPSSMDA "); 
				break;
			}

			cmd_inq.SetCommandText( sqlstr );
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tpssmda["IDX_NO"] =cmd_inq.GetString(1);
			}
			cmd_inq.Close();
			v_idx_no1 = v_idx_no1.Parse(tpssmda["IDX_NO"].ToString())+1;
			v_idx_no =v_idx_no1.ToInt32();


		}

		tpssmda["IDX_NO"] = CString::Format("%.4d", v_idx_no);

		cmd_inq.Parameters.Set("factory_div", tpssmda["FACTORY_DIV"]);
		cmd_inq.Parameters.Set("cc_mach_no", tpssmda["CC_MACH_NO"]);
		cmd_inq.Parameters.Set("st_no", tpssmda["ST_NO"]);
		cmd_inq.Parameters.Set("cast_width_min", tpssmda["CAST_WIDTH_MIN"]);
		cmd_inq.Parameters.Set("cast_width_max", tpssmda["CAST_WIDTH_MAX"]);
		/*cmd_inq.Parameters.Set("cast_thick_min", tpssmda.CAST_THICK_MIN );
		cmd_inq.Parameters.Set("cast_thick_max", tpssmda.CAST_THICK_MAX );*/


		tpssmda["REC_CREATOR"] = s.userid;
		tpssmda["REC_CREATE_TIME"] = dateNow;
	//	tpssmda["FACTORY_DIV"] = v_factory_div ;


		try
		{
			tpssmda.Insert();
		}

		catch(CDbException& cde)
		{
			strcpy(s.msg,  _RES("GCRSS0000018"));//_RES("GCRSS0000018")/*新增信息失败。*/
			sprintf(s.sysmsg,"DB[tpssmda]idx_no[%s]sqlcode[%d]",(const char*)tpssmda["IDX_NO"].ToString(),cde.GetCode());//sysmsg
			throw CApplicationException(-1, s.msg, log.Location);	
		}


		/*设置系统返回参数*/
		strcpy(s.msg,  _RES("GCRSS0000002"));//处理成功。  


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000018")/*新增信息失败。*/, arguments, 1);
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

