
#include "stdafx.h"


// Service Èë¿Ú
BM2F_ENTERACE(pssm04_inqs);

int f_pssm04_inqs(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	int  doFlag = 0;
	try
	{
		CString sql = "SELECT DISTINCT REFINE_ROUTE_CODE FROM tpssm01";

	    CDbCommand cmd(conn);
		cmd.SetCommandText(sql);
		cmd.ExecuteQuery(bcls_ret->Tables[0]);		
	}
	catch(const CApplicationException& ex)
	{
		doFlag = ex.GetCode();
		strcpy(s.msg, (const char*)ex.GetMsg());
	}
	catch(const CException& ex)
	{
		doFlag = -1;
		strcpy(s.msg, (const char*)ex.GetMsg());
	}

	s.flag = doFlag;

	return(doFlag);
}
