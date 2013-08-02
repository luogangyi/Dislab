#include "QueryVM.h"

extern Log log;

QueryVM::QueryVM()
{

}


QueryVM::~QueryVM()
{

}

void QueryVM::Do()
{
	String logContent;

	log.write("begin QueryVM::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:Query VM:RETN=4,DESC=参数错误";
		return;
	}
	log.write("end process parameter.", Log::INFO);

	
	//鉴权	
	if( checkRight( vmName ) == false )
	{		
		ackCode =  "ACK:Query VM:RETN=14,DESC=鉴权不通过";
		return;	
	}

	getVMInfo();
	
}


bool QueryVM::getVMInfo()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "SELECT VMNAME,OS,DISK,RAM,VCPU FROM VMDetail where id=" + vmID;

	if( executeDBQuery( sql, query) == false )
	{
		ackCode =  "ACK:Query VM:RETN=8,DESC=system internal error";
		return false;	
	}
	

	int rowCnt = query.numRow();

	if( rowCnt != 1 ) 
	{
		log.write("query the vm " + vmID + "info error,the result record count is not one.", Log::ERROR);
		ackCode =  "ACK:Query VM:RETN=1001,DESC=vm is not exist";
		return false;
	}

	String sql2 = "select * from OnlineVMS where vmid=" + vmID;
	CppMySQLQuery query2 ;

	if( executeDBQuery( sql2, query2) == false )	
		return false;	
	rowCnt = query2.numRow();

	String tmp;
	tmp = query.getStringField( "VMNAME" );
	
	ackCode = "ACK:Query VM:RETN=0,DESC=成功,RESULT=" + tmp + "|";

	tmp = query.getStringField( "OS" );

	ackCode += tmp + "|";

	tmp = query.getStringField( "DISK" );

	ackCode += tmp + "|";


	tmp = query.getStringField( "RAM" );

	ackCode += tmp + "|";


	tmp = query.getStringField( "VCPU" );

	ackCode += tmp + "|";

	if( rowCnt >= 1 )
		ackCode += "1";
	else
		ackCode += "0";

	
	return true;
}

bool QueryVM::getPara()
{
	processPara();

	if( paraMap.size() != 1 )
		return false;
	
	map<String,String>::iterator it;

	
	it = paraMap.find("vmid");
	if( it == paraMap.end() )		
		return false;
	

	vmID = it->second;
	
	if( vmID.size() == 0 )
		return false;	
	
	return true;
}


void QueryVM::setUserName(String user)
{
	userName = user;
}




