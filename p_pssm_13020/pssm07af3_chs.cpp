/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   闫云鹏
Version:    1.0
Date:     2018-05-09
Description: 收池炼钢计划
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件








int f_pmom_status_upd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_cm_002021_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_pssm99_trace(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //炼钢履历跟踪

/*<remark>=========================================================
/// <summary>
/// 制造命令回收
/// <para>
/// 1.校验是否未排入出钢计划
/// 2.将PONO置为命令接受状态
/// 3.调用PM函数
/// 4.下达PES命令删除电文
/// </para>
/// <para>数据库表：TPSSM01(炼钢连铸制造命令炉次表)         </para>
/// <para>主调用函数：前台PSSM01画面F6 PONO收回调用 </para>
/// </summary>
/// <param name="pono">制造命令   </param>
/// <returns>炉次制造命令</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(pssm07af3_chs)
//-EP_SYSTEM_HEAD_END
int f_pssm07af3_chs(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int i, rows;
	int ret;
	//CString temp_cast_lot_no = "";

	/* 业务变量 */
	CString v_update = "";  //修改的字段信息
	CString v_condi = "";  //过滤的字段信息
	CString sqlstr;
	CDecimal    cc_seq = 0;                         /* 连铸顺序号 */
	CDecimal    dummy = 0;
	//CString   v_cast_lot_no = "";
	CString v_restrand_flg = ""; //重引锭标记
	int v_lack_per = 0;
	//int v_cast_lot_sum = 0;
	int v_pono_sum = 0;
	CString   temp_factory_div = "";
	CString   v_pes_host_no = "";
	CString	  v_pes_host_name = "";

	int temp_pono_status = 0;


	/* 实体类定义 */
	CModel tpssm01("TPSSM01");
	CModel tpssm03("TPSSM03");
	CModel tpssm10("TPSSM10");

	/* 数据库操作类定义 */
	CDbCommand cmd_tpssm01_inq(conn);
	CDbCommand cmd_tpssm03_inq(conn);
	CDbCommand cmd_tpssm10_inq(conn);
	CDbCommand cmd_sql(conn); //与DB 建立连接。
	CDbCommand cmd_del(conn); //与DB 建立连接。
	CDbCommand cmd_tpssm10_upd(conn);
	CDbCommand cmd_sql_count(conn);
	CDbCommand cmd_tep0002_inq(conn);
	//调用生产接口
//	EIClass inBlock;
	EIClass inBlock99; //调用炼钢履历跟踪
	EIClass outBlock;

	//调用电文接口
//	EIClass inBlock1;

	try
	{

		/*	//声明调用生产的接口参数
			inBlock.Tables[0].set_TableName("PSSMCHS");
			inBlock.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
			inBlock.Tables[0].Columns.Add(DT_STRING, "PONO");

			//声明调用电文的接口参数
			inBlock1.Tables[0].set_TableName("PONOSEND");
			inBlock1.Tables[0].Columns.Add(DT_STRING, "MARKS1");
			inBlock1.Tables[0].Columns.Add(DT_STRING, "PONO");
			inBlock1.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
			inBlock1.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
			*/
		//调用炼钢履历跟踪
		inBlock99.Tables[0].set_TableName("TRACE");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "FACTORY_DIV");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "PONO_STATUS");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "CAST_LOT_NO");
		inBlock99.Tables[0].Columns.Add(DT_STRING, "EVENT_ID"); //事件代码

		//获得输入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		////Log::Trace("", __FUNCTION__, "rows = [{0}]", rows);
		for (i = 0; i < rows; i++)
		{
			// 获取前台传入参数
			tpssm03.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			//打印传入参数
			////Log::Trace("", __FUNCTION__, "pssm07af3_chs>tpssm03["PONO"] = [{0}]", tpssm03["PONO"].ToString());

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT  * FROM TPSSM03 "
					" WHERE  PONO = @tpssm03.PONO "
					"ORDER BY SLAB_SEQ_1"
					);
				break;
			}

			cmd_tpssm03_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
			cmd_tpssm03_inq.Parameters.Set("tpssm03.PONO", tpssm03["PONO"].ToString());
			cmd_tpssm03_inq.ExecuteReader();

			//Read一次性只会读取一行数据，所以ORDER BY之后由于没有循环，只会拿取SLAB_SEQ_1最小的一行数据
			if (cmd_tpssm03_inq.Read())
			{
				cmd_tpssm03_inq.Fetch(tpssm03); //将从SQL语句读出的一行数据，压入到tpssm03

			}
			else
			{
				CFormattable arguments[] = { tpssm03["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]不存在于表TPSSM03中{1}。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			cmd_tpssm03_inq.Close();

			tpssm10.CopyFrom(tpssm03);


			// 获取前台传入参数
			tpssm01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//打印传入参数
			////Log::Trace("", __FUNCTION__, "pssm07af3_chs>tpssm01["PONO"] = [{0}]", tpssm01["PONO"].ToString());

			//	v_pes_host_no = "";
			//	v_pes_host_name = "";

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = CString(" SELECT PONO_STATUS FROM TPSSM01 "
					" WHERE  PONO = @tpssm01.PONO "
					);
				break;
			}
			cmd_tpssm01_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
			cmd_tpssm01_inq.Parameters.Set("tpssm01.PONO", tpssm01["PONO"].ToString());
			cmd_tpssm01_inq.ExecuteReader();

			if (cmd_tpssm01_inq.Read())
			{
				temp_pono_status = cmd_tpssm01_inq.GetInt32(1);
				if (temp_pono_status != 11)
				{
					CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
					CMessageFormat::Format(s.msg, "制造命令号[{0}]的制造命令状态不为材料接收{1}，不能收池。", arguments, 2);
					throw CApplicationException(-1, s.msg, log.Location);
				}

			}
			else
			{
				CFormattable arguments[] = { tpssm01["PONO"].ToString() }; // 定义参数列表的数组
				CMessageFormat::Format(s.msg, "制造命令号[{0}]不存在于表TPSSM01中{1}，不能收池。", arguments, 2);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			cmd_tpssm01_inq.Close();

			//更新修改者，修改时间
			tpssm01["REC_REVISOR"] = CString(s.userid);
			tpssm01["REC_REVISE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

			//更新制造命令状态为12（计划收池）
			tpssm01["PONO_STATUS"] = 12;

			v_update = "REC_REVISOR,REC_REVISE_TIME,PONO_STATUS";
			v_condi = "PONO"; //查询条件

			if (tpssm01.Update(v_update, v_condi) != true)
			{
				strcpy(s.msg, "Update failed.");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tpssm10.CopyFrom(tpssm01);

		}
	}

			catch (CDbException& ex)  //捕获数据库操作异常
			{
				CFormattable arguments[] = { ex.GetCode() };
				CMessageFormat::Format(s.msg, _RES("GCRSS0000017")/*读取数据失败,表[{0}],sqlcode=[{1}]。请联系系统维护人员。*/, arguments, 1);
				CString str = sqlstr + "\r\n" + ex.GetMsg();
				strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
				s.flag = -1;
				doFlag = -1;                  //数据库异常时返回-1，事务将被回滚
			}
			catch (CApplicationException& ex)
			{
				s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
				doFlag = -1;
			}
			catch (CException& ex)
			{
				strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1); //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.msg参数对应
				s.flag = ex.GetCode();       //返回前台，与EI.EITuxedo.CallService()方法返回的EI.EIInfo对象的sys_info.flag参数对应
				doFlag = -1;
			}
			cmd_tpssm01_inq.Close();

			return doFlag;
		
}
