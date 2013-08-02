#include "VMMigrate.h"
#include <fstream>



extern Log log;

VMMigrate::VMMigrate()
{
	
}

VMMigrate::~VMMigrate()
{
}

bool VMMigrate::init()
{
	bool bRet = ConfigFile::getIntValue("NCVMServerPort", ncServerPort);	
		
	if( bRet == false )
	{
		printf("read NCVMServerPort error,exit.\n");
		return false;
	}
	return true;
}

void VMMigrate::Do()
{
}

bool VMMigrate::migrate(String vmName)
{
	
	String newNCIP;
	
	newNCIP = findNC( vmName );


	//获取虚拟机所在的NC节点地址
	String sql = "SELECT ncip FROM OnlineVMS WHERE vmname='" + vmName + "'";
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;
	String oldncip = query.getStringField("ncip");
			
	if( newNCIP != "" )
	{
		cout<<"find " + newNCIP<<" NC to migrate."<<endl;
		if( migrateVM( oldncip, newNCIP, vmName) == false )
		{
			log.write("migrate vm " + vmName + " failed.", Log::ERROR);
			return false;
		}

		cout<<"migrate vm success."<<endl;
		return true;
	}
	else
		log.write("do not find new nc node to migrate the overload vm.", Log::WARN);
		
	return false;
}

bool VMMigrate::prepareDir(String ncIP, String vmName)
{
	String sql = "select id from VMDetail where vmname='" + vmName + "'";
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;
	String id = query.getStringField("id");
	
	sql = "SELECT ip,dir FROM SCSP INNER JOIN SCStorDetail ON SCSP.ID=SCStorDetail.SID WHERE SCStorDetail.VMID=";
	sql += id;
	if( executeDBQuery( sql, query) == false )
		return false;
	rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;

	String nfsIP = query.getStringField("ip");
	String nfsDir = query.getStringField("dir");
	
	String request = "PREPARE DIR:IP=" + nfsIP;
	request += ",DIR=" + nfsDir;

	
	int sock = connect2Server( ncIP.c_str(), ncServerPort);
	if( sock == -1 )
	{
		log.write("connect nc vm server error.", Log::ERROR);		
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to nc vm server error.", Log::ERROR);		
		return false;
	}

	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from nc vm server error.", Log::ERROR);		
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);
	
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);		
		return false;
	}	

	closeSock( sock );	
	return iRetn == 0;

}

bool VMMigrate::migrateVM(String oldNCIP, String newNCIP, String vmName)
{
	cout<<"migrate vm from "<<oldNCIP<<" to "<<newNCIP<<endl;
	//1、准备目录
	if( prepareDir( newNCIP, vmName) == false )
	{
		log.write("prepare dir for vm migrate on new nc failed.", Log::ERROR);
		return false;
	}

	//2、开始migrate
	String request = "MIGRATE VM:VMNAME=" + vmName;
	request += ",IP=" + newNCIP;
	int sock = connect2Server( oldNCIP.c_str(), ncServerPort);
	if( sock == -1 )
	{
		log.write("connect nc vm server error.", Log::ERROR);		
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to nc vm server error.", Log::ERROR);		
		return false;
	}

	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from nc vm server error.", Log::ERROR);		
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);
	
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);		
		return false;
	}	

	if( iRetn != 0 )
		return false;
	String sql = "delete from OnlineVMS where vmname='" + vmName + "'";
	executeDBSQL( sql );
	
	sql = "insert into OnlineVMS (vmname,ncip,starttime) values('";
	sql += vmName + "','" + newNCIP + "',CURRENT_TIMESTAMP())";
	executeDBSQL( sql );
	
	return true;
}

String VMMigrate::findNC( String vmName )
{
	String sql = "SELECT CPU-cpu_used AS free_cpu ,ip FROM onlineNC ORDER BY CPU DESC LIMIT 0,1";
	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return "";

	if( query.numRow() != 1 )
		return "";

	int freeCPU = query.getIntField( "free_cpu" );
	String ncip = query.getStringField( "ip" );

	sql = "select VCPU from VMDetail where vmname='" + vmName + "'";
	if( executeDBQuery( sql, query) == false )
		return "";

	if( query.numRow() != 1 )
		return "";

	int vmCPUCnt = query.getIntField( "VCPU" );

	if( freeCPU > vmCPUCnt + 2 )
		return ncip;

	return "";
	
}

bool VMMigrate::isNCOverLoad(String ncip)
{
	String sql = "select cpu,ram from PerfData where ip='" + ncip + "'";
	sql += " ORDER BY TIME DESC limit 0,10";
	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	if( rowCnt != 10 ) 
		return false;
	int cpu;
	ncUsedMem = query.getIntField("ram");
	cout<<"ncUsedMem:"<<ncUsedMem<<endl;
	while( !query.eof() )
	{		
		cpu = query.getIntField("cpu");
		cpu = cpu/10;
		if( cpu > 90 )
			return true;

		query.nextRow();
	}
	return false;
}


bool VMMigrate::isVMOverLoad(String vmName)
{
	String sql = "SELECT cpu FROM VMPerfData WHERE vmname='";
	sql += vmName + "' ORDER BY TIME DESC  LIMIT 0,3";

	
	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	if( rowCnt != 3 ) 
		return false;

	int cpu ;

	while( !query.eof() )
	{
		
		cpu = (int)query.getFloatField("cpu");	
		//cout<<cpu<<endl;
		if( cpu < 90 )
			return false;

		query.nextRow();
	}
	return true;
}

int VMMigrate::getNCFreeMemory(String ncip)
{
	String sql = "SELECT ram FROM onlineNC WHERE ip='" + ncip + "'";

	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;

	int totalMem = query.getIntField("ram");


	return totalMem - ncUsedMem;
}

int VMMigrate::getVMMemory(String vmname)
{
		
	String sql = "SELECT ram FROM VMDetail WHERE vmname='" + vmname + "'";

	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;

	return query.getIntField("ram");

}


bool VMMigrate::expandVMMem(String vmName, String ncip)
{
	
	
	String request = "EXPAND VM:VMNAME=" + vmName;
	
	int sock = connect2Server( ncip.c_str(), ncServerPort);
	
	if( sock == -1 )
	{
		log.write("connect nc vm server error.", Log::ERROR);		
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to nc vm server error.", Log::ERROR);		
		return false;
	}

	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from nc vm server error.", Log::ERROR);		
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);
	
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);		
		return false;
	}		
	
	return iRetn == 0;
}








