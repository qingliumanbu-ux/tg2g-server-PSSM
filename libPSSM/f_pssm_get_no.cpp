/*************************************************
Copyright:Baosight Software LTD.co Copyright (c) 2010
Author:lijie
Date:2011-12-16
Version:1.0
Description: 生成pono和lot号
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件


/*<remark>=========================================================
/// <summary>
///生成pono和lot号
/// <para>1.后备pono生成规则：pono 类型+年末位+5位流水号；</para>
/// <para>2.后备lot生成规则：lot   类型+月末位+4位流水号；</para>
/// <para>数据库表：TPSSM01              </para>
/// <para>主调用函数：FormPSSM09后台新增计划调用。        </para>
/// </summary>
/// <param name="main_backlog_code">炼钢主工序代码    </param>
/// <returns>下达计划信息</returns>
===========================================================</remark>*/ 

BM2_FUNCTION_EXPORT
int f_pssm_get_no(EIClass *bcls_rec,  EIClass *bcls_ret,CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int ret = 0;
	int v_getseq_dif=0; //pono和lot 区分标志         
	CString num1="";	
	CString num2=" ";
	CString num0="9";//后备新增为9
	CString type  = "";
	CString datetime = "";
	CString maxseq = "";
	CString ret_seqno=" ";
	CString sqlstr = "";
	CString billet_type = "";
	CModel tpssm01("TPSSM01");

	CDbCommand cmd_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//获得输入参数
		tpssm01["FACTORY_DIV"] = bcls_rec->Tables[0].Rows[0]["FACTORY_DIV"];
		tpssm01["REC_CREATOR"] = bcls_rec->Tables[0].Rows[0]["CS_PLAN_MAKER"];
		v_getseq_dif = bcls_rec->Tables[0].Rows[0]["GETSEQ_DIF"];
		

		//设置输出参数
		bcls_ret->Tables[0].Clear();
		bcls_ret->Tables[0].set_TableName("SEQ_NO");	
		

		EDLog	(1,1,"factory_div=[%s]",(const char*)tpssm01["FACTORY_DIV"].ToString());
		EDLog	(1,1,"rec_creator=[%s]",(const char*)tpssm01["REC_CREATOR"].ToString());
		EDLog	(1,1,"getseq_dif=[%d]",v_getseq_dif);

		EDLog(1, 1, "-----新增调用区分开始-----");

		if(v_getseq_dif==2)//新增pono
		{
			EDLog	(1,1,"-----新增pono开始-----");
			EDLog	(1,1,"ret_seqno=[%s]",(const char*)ret_seqno);	

			num1 = datetime.Substring(3,1);//年末位
			num2 = num0+num1;
			EDLog	(1,1,"num2=[%s]",(const char*)num2);

			//调用函数EPGETNEXTSEQ，获得当前最大流水号
			/*type = "PONO_1";

			EDLog(1,1,"type = %s" ,(const char*)type);

			maxseq = EPGetNextSeq(type,conn);*/

			sqlstr = "  SELECT LPAD(PSSM_CAST_NO_INS.nextval, 6, '0') FROM dual  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				maxseq = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			//if (ret != 0)
			//{
			//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			EDLog(1, 1, " 获得当前流水号maxseq = [%s]", (const char*)maxseq); 
			ret_seqno = ret_seqno.Format("%2s%06s%s" ,(const char*)num2 , (const char*)maxseq,"1");
			EDLog	(1,1,"ret_seqno=[%s]",(const char*)ret_seqno);
			bcls_ret->Tables[0].Columns.Add(DT_STRING,"PONO");
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[0]["PONO"] = ret_seqno;
		}
		else if (v_getseq_dif == 32)//3-梅钢 2-新增pono
		{
			EDLog(1, 1, "-----新增pono开始-----");
			EDLog(1, 1, "ret_seqno=[%s]", (const char*)ret_seqno);

			if (strcmp(bcls_rec->Tables[0].Rows[0]["RET_FLAG"].ToString(), "1") == 0)
			{
				num0 = "9";
			}
			else
			{
				num0 = "8"; //8-L3后备，9-返送新增
			}

			num1 = "3"+datetime.Substring(3,1);//年末位
			if (tpssm01["FACTORY_DIV"].ToString() == "A20")
				num2 = num1 + "2" + num0;
			else
				num2 = num1 + "1" + num0;
			EDLog	(1,1,"num2=[%s]",(const char*)num2);

			//调用函数EPGETNEXTSEQ，获得当前最大流水号
			type = "PONO_1";

			EDLog(1,1,"type = %s" ,(const char*)type);

			maxseq = EPGetNextSeq(type,conn);

			//if (ret != 0)
			//{
			//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			EDLog(1, 1, " 获得当前流水号maxseq = [%s]", (const char*)maxseq); 
			ret_seqno = ret_seqno.Format("%4s%04d" ,(const char*)num2 , atoi(maxseq));
			EDLog(1, 1, "ret_seqno=[%s]", (const char*)ret_seqno);
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "PONO");
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[0]["PONO"] = ret_seqno;
		}
		else if(v_getseq_dif==4)//新增lot
		{			
			EDLog	(1,1,"ret_seqno=[%s]",(const char*)ret_seqno);	
			ret_seqno = " ";		
#if 0		
			num1 = CDateTime::Now().ToString("MM");

			if(num1.Compare("10")==0)
			{
				num1 = "A";
			}
			else if(num1.Compare("11")==0)
			{
				num1 = "B";
			}
			else if(num1.Compare("12")==0)
			{
				num1 = "C";
			}  
			else
			{
				num1 = num1.Substring(1,1);
			}
#endif
			num1 = datetime.Substring(3,1);//年末位
			EDLog	(1,1,"num1=[%s]",(const char*)num1);  


			num2 =  num0 + num1;

			EDLog	(1,1,"num2=[%s]",(const char*)num2);  
			
			//调用函数EPGETNEXTSEQ，获得当前最大流水号
			type = "LOT_1";

			EDLog(1,1,"type1 = %s" ,(const char*)type);

			maxseq = EPGetNextSeq(type,conn);

			//if (ret != 0)
			//{
			//	//strcpy(s.msg, _RES("GCRSS0000012")/*系统出现异常，调用函数出错，请联系系统维护人员。*/);
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			EDLog(1, 1, " 获得当前流水号maxseq1 =%s", (const char*)maxseq);

			ret_seqno = ret_seqno.Format("%2s%4s" , (const char*)num2 , (const char*)maxseq);

			EDLog(1,1,"ret_seqno=[%s]",(const char*)ret_seqno);	
			bcls_ret->Tables[0].Columns.Add(DT_STRING,"CAST_LOT_NO");
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[0]["CAST_LOT_NO"] = ret_seqno;
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
