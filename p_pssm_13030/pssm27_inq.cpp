/*************************************************
版权: Baosight Software LTD.co Copyright (c) 2012
作者: 顾东亮
日期: 2012-07-19
功能: 交接部出钢记号转换表查询
修改历史:
日期:________;修改人:________; 需求提出人:________
变更内容:

**************************************************/

#include "stdafx.h"
#include "tpssm17.h"

/*<remark >========================================================= 
/// <summary > 
/// 交接部出钢记号转换表查询
/// <para > 
/// 查询交接部出钢记号转换表；
/// </para > 
/// <para > 数据库表：TPSSM17(交接部出钢记号转换表)     </para > 
/// <para > 主调用函数：前台PSSM27画面F2(查询)调用。   </para > 
/// </summary > 
/// <param name = "ST_NO" > 主出钢记号  </param > 
/// <returns > 交接部出钢记号转换信息</returns > =========================================================== </remark > */
BM2F_ENTERACE(pssm27_inq)

int f_pssm27_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	/*打印程序起止LOG*/
	CTracer log(__FUNCTION__);

	/*程序内部变量*/	
	int doFlag = 0;		//返回值
	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";

	/*定义业务用变量*/
	CString ST_NO = "";
	/*实体类定义*/

	/******业务处理开始******/

	try
	{
		// 定义表的实体对象
		CTPSSM17 tpssm17(conn);

		//分页查询用参数
		int  nStart = (int)bcls_rec->Tables[0].Rows[0]["START"];//获取查询起始值
		int  nPageSize = (int)bcls_rec->Tables[0].Rows[0]["PAGE_SIZE"];//获取页面值
		int  RECORD_TOTAL = 0;

		//定义数据库操作命令对象comm_inq执行sql语句，sql字符串用""包括，可以分行，但每行前后务必留出一个空格。
		CString sql = " SELECT * FROM TPSSM17 WHERE 1 = 1 ";

		CString sql_count = " SELECT COUNT(1) FROM TPSSM17 WHERE 1 = 1 ";	

		CString sql_order = " ORDER BY ST_NO ASC , SEQ_NO ASC";

		CString sql_where = " ";

		//获取输入参数
		tpssm17.ST_NO = (CString)bcls_rec->Tables[0].Rows[0]["ST_NO"];
		tpssm17.ST_NO1 = (CString)bcls_rec->Tables[0].Rows[0]["ST_NO1"];
		//输入条件判断
		if(tpssm17.ST_NO.GetLength() > 0)
		{
			sql_where += " and  ST_NO = @tpssm17.ST_NO  ";
		}
			if(tpssm17.ST_NO1.GetLength() > 0)
		{
			sql_where += " and  ST_NO1 = @tpssm17.ST_NO1  ";
		}
		//获取记录数
		CDbCommand cmd(sql_count + sql_where, conn);
		cmd.Parameters.Set("tpssm17.ST_NO",tpssm17.ST_NO);
		cmd.Parameters.Set("tpssm17.ST_NO1",tpssm17.ST_NO1);
		CDecimal rc = cmd.ExecuteScalar();
		bcls_ret->ExtendedProperties.Add("RECORD_TOTAL",rc.ToString());

		//查询出结果
		CDbCommand comm_inq(sql + sql_where + sql_order,conn);
		comm_inq.Parameters.Set("tpssm17.ST_NO",tpssm17.ST_NO);
		comm_inq.Parameters.Set("tpssm17.ST_NO1",tpssm17.ST_NO1);
		comm_inq.ExecuteQuery(bcls_ret->Tables[0],nStart,nPageSize);

		//如果前台要求通过表名进行关联，需要进行设置对应块的表名信息
		bcls_ret->Tables[0].set_TableName("TPSSM17");
		strcpy(s.msg, "查询成功!"); ;
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char * )str, sizeof(s.sysmsg) - 1);
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
		strncpy(s.msg, (const char * )ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return(doFlag);
}