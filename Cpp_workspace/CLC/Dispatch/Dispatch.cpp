#include "Dispatch.h"
#include "../../common/CppMysql.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <ctime>


extern Log log;

String Dispatch::dbServerIP;
String Dispatch::dbUserName;
String Dispatch::dbUserPass;

extern bool findFirstCapableNC(const char* dbServerIP, const char* dbUserName, const char* dbUserPass, int iRamSize, int iCPUCnt, NCNodeInfo* node);
Dispatch::Dispatch(String cmd)
{
	strCmd = cmd;	
	iRamSize = 0;
	iCPUCnt = 0;
	virtType = "";
}

Dispatch::~Dispatch()
{

}


void Dispatch::Do()
{
	log.write("in Dispatch::Do", Log::INFO);	
	
	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:GET BEST NC:RETN=4,DESC=parameter error";
		return;
	}

	if( getVMInfo() == false )
	{
		ackCode =  "ACK:GET BEST NC:RETN=8,DESC=system db error";
		return;
	}

	if( iRamSize == 0 )
	{
		ackCode =  "ACK:GET BEST NC:RETN=1002,DESC=no vm info";
		return;
	}	
	
	if( findNC() == false )
	{
		ackCode =  "ACK:GET BEST NC:RETN=8,DESC=system interal error";
		return;
	}
	
	if( capableNCIP.size() != 0 )
		ackCode =  "ACK:GET BEST NC:RETN=0,DESC=success,IP=" + capableNCIP;
	else
		ackCode =  "ACK:GET BEST NC:RETN=1001,DESC=system is busy ,try later";
}

bool Dispatch::findNC()
{
	CppMySQL3DB db;
	
	int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	if ( iRet != 0 )			
	{
		log.write("open db error", Log::ERROR);
		return false;
	}
	

	CppMySQLQuery query ;

	String sql = "SELECT CPU-cpu_used AS free_cpu,ram-memory_used AS free_memory,ip,update_at FROM onlineNC";
	sql += " where VirtType='" + virtType + "'";

	
	virtType.toLower();
	if( virtType == String("kvm") )
	{		
		sql += " or VirtType='qemu'";
	}
	try
	{
		query = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		log.write(sErr, Log::ERROR);
		return false;
	}

	int free_cpu;
	
	int free_ram;
	int hostCnt = 0;
	long update_at;

	String ip;
	
	std::vector<String> host_array;
	time_t t;
	time(&t);
	
	while( !query.eof() )
	{
		ip = query.getStringField( "ip" );
		free_cpu = query.getIntField( "free_cpu" );
		free_ram = query.getIntField( "free_memory" );
		update_at = query.getIntField( "update_at" );

		if( abs(t-update_at) < 20 && free_cpu >= iCPUCnt && free_ram > iRamSize + 1024 )
		{
			host_array.push_back( ip );
			hostCnt++;			
		}
		query.nextRow();
	}

	db.close();
	
	srand((unsigned)time(NULL));
	
	if( host_array.size() == 0 )
	{
		log.write("not found capable nc node.", Log::INFO);
		return false;
	}
	
	int index = rand() % host_array.size();
	
	capableNCIP = host_array[index];
	
	
	return true;
	
	/*NCNodeInfo node;
	if( findFirstCapableNC(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), iRamSize, iCPUCnt, &node) )
	{		
		capableNCIP = node.ipAddr;
		return true;
	}
	return false;*/
	
}


bool Dispatch::getVMInfo()
{
	
	CppMySQL3DB db;
	String logContent;
	int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	
	if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return false;
	}
	
	
	CppMySQLQuery query ;
	String sql = "select ram,VCPU,VirtType from VMDetail where id=" + sVMID;
	try
	{
		query = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		db.close();
		return false;
	}	
	
	int rowCnt = query.numRow();
	iRamSize = 0;
	if( rowCnt != 1 )
	{
		char buf[10];
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		db.close();
		return false;
	}

	iRamSize = query.getIntField("ram");
	iCPUCnt = query.getIntField("VCPU");
	virtType = query.getStringField( "VirtType" );
	db.close();
	return true;
	
}


bool Dispatch::getPara()
{
	processPara();


	if( paraMap.size() != 1 )
		return false;
	
	map<String,String>::iterator it;

	
	it = paraMap.find("groupid");
	if( it != paraMap.end() )
	{
		groupID = it->second;
		if( groupID.size() == 0 )
			return false;
		else
		{
			groupStartFlag = true;
			return true;
		}
	}



	it = paraMap.find("vmid");
	if( it == paraMap.end() )		
		return false;
	
	sVMID = it->second;
	if( sVMID.size() == 0 )
		return false;


	return true;
}









