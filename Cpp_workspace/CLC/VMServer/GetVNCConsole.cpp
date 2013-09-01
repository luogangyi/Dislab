#include "GetVNCConsole.h"


extern Log log;
extern int g_ncVMServerPort;

GetVNCConsole::GetVNCConsole()
{
	ncNodeIP = "";
}


GetVNCConsole::~GetVNCConsole()
{

}

void GetVNCConsole::Do()
{
	String logContent;

	log.write("begin GetVNCConsole::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:GET VNC CONSOLE:RETN=4,DESC=parameters error";
		return;
	}
	log.write("end process parameter.", Log::INFO);

	//检测虚拟机是否存在
	if( isVMExist() == false)
	{
		ackCode =  "ACK:GET VNC CONSOLE:RETN=1001,DESC=vm is not exist";
		return;
	}

	//鉴权	
	//if( checkRight( vmID ) == false )
	//{		
	//	ackCode =  "ACK:GET VNC CONSOLE:RETN=14,DESC=no right";
	//	return;	
	//}

	if( isVMStarted() == false )
	{
		ackCode =  "ACK:GET VNC CONSOLE:RETN=1001,DESC=vm is not started";
		return;
	}

	
	if( getVNCPort() == false )
	{
		log.write("get vm vnc port " + vmID + " failed.", Log::ERROR);
		return;
	}
	
	
}


bool GetVNCConsole::getPara()
{
	processPara();

	if( paraMap.size() != 1 )
		return false;
	
	map<String,String>::iterator it;
	
	it = paraMap.find("vmid");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	vmID = it->second;	
		
	return true;
}


bool GetVNCConsole::getVNCPort()
{
	String request = "GET VNC CONSOLE:vmuuid=" + vmUUID;

	
	int sock = connect2Server( ncNodeIP.c_str(), g_ncVMServerPort);
	if( sock == -1 )
	{
		log.write("connect nc vm server error.", Log::ERROR);
		ackCode =  "ACK:GET VNC CONSOLE:RETN=8,DESC=system internal error";
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		closeSock( sock );
		log.write("send request to nc vm server error.", Log::ERROR);
		ackCode =  "ACK:GET VNC CONSOLE:RETN=8,DESC=system internal error";
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		closeSock( sock );
		log.write("recv ack message from nc vm server error.", Log::ERROR);
		ackCode =  "ACK:GET VNC CONSOLE:RETN=8,DESC=system internal error";
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);
	//分析结果，得出IP
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		closeSock( sock );
		log.write("get para retn error from ack message.", Log::ERROR);
		ackCode =  "ACK:GET VNC CONSOLE:RETN=8,DESC=system internal error";
		return false;
	}	
	
	closeSock( sock );
	ackCode = ack;
	return iRetn == 0;
}

bool GetVNCConsole::isVMStarted()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select host from VMDetail where id=" + vmID;


	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();
	
	if( rowCnt >= 1 ) 
	{
		ncNodeIP = query.getStringField( "host" );
		return true;
	}
	return false;
	
}

bool GetVNCConsole::isVMExist()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select uuid from VMDetail where id=" + vmID;

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();

	if( rowCnt != 1 ) 
		return false;
	vmUUID = query.getStringField("uuid");
	return true;
	
}
	


