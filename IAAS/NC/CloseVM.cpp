#include "CloseVM.h"
#include <libvirt/libvirt.h>

extern Log log;

CloseVM::CloseVM( string cmd)
{
	strCmd = cmd;	
	
}

CloseVM::~CloseVM()
{

}

void CloseVM::Do()
{
	string logContent;
	logContent = "in CloseVM::Do().";
	log.write(logContent.c_str(), Log::INFO);

	if( getPara() == false)
	{	
		ackCode =  "ACK:Close VM:RETN=4,DESC=参数错误";
		return;
	}

	if( getVMInfoFromDB() == false)
	{
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return;
	}
	
	int iRet = isAlreadyStart();
	if( iRet == 0 )
	{
		ackCode = "ACK:CLOSE VM:RETN=1002,DESC=虚拟机未启动";
		return;
	}
	else if( iRet == 2)
	{	
		ackCode = "ACK:CLOSE VM:RETN=8,DESC=系统内部错误";
		return;
	}

	iRet = closeVM();
	if( iRet == 0 )
	{	
		ackCode = "ACK:CLOSE VM:RETN=0,DESC=关闭虚拟机成功";
	}
	else if( iRet == 2)
	{	
		ackCode = "ACK:CLOSE VM:RETN=8,DESC=系统内部错误";		
	}
	else	
	{	
		ackCode = "ACK:CLOSE VM:RETN=8,DESC=关闭虚拟机失败";
	}

	
}

////////////////////////////////////
//返回值
//0-成功
//1-失败
//2-系统内部错误
////////////////////////////////////
int CloseVM::closeVM()
{
	
    virConnectPtr conn = NULL;
    virDomainPtr dom = NULL;
	int iRet;

    conn = virConnectOpen("");

	if (conn == NULL)
	{		
		log.write(String("CloseVM::closeVM() Failed to connect to hypervisor."), Log::ERROR);
		return 2;
	}
		    
	dom = virDomainLookupByUUIDString(conn,vmUUID.c_str());

	if( dom == NULL )
	{
		virConnectClose(conn);
		return 0;
	}
    
   
    if( virDomainDestroy(dom) == 0 )
    {    
    	dom = virDomainLookupByUUIDString(conn,vmUUID.c_str());
    	virDomainUndefine(dom);
    	virDomainFree(dom);
    	virConnectClose(conn);
		return 0;
    }

	virConnectClose(conn);
	
	return 1;

	
}

bool CloseVM::getPara()
{
	processPara();


	if( paraMap.size() != 1 )
		return false;
	
	map<string,string>::iterator it;

	
	it = paraMap.find("vmid");
	if( it == paraMap.end() )		
		return false;
	
	vmID = it->second;

	if( vmID.size() == 0 )
		return false;

	return true;

}


bool CloseVM::getVMInfoFromDB()
{		
	CppMySQLQuery query;
	string sql = "select vmname,ram,vcpu,virttype,uuid from VMDetail where id=" + vmID;
	if( executeDBQuery( sql, query) == false )
		return false;

	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;

	vmName = query.getStringField("vmname");	
	vmUUID = query.getStringField("uuid");
	
	
	return true;
}



