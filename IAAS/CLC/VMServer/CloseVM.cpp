#include "CloseVM.h"


extern Log log;
extern int g_ncVMServerPort;

CloseVM::CloseVM()
{
	ncNodeIP = "";
}

CloseVM::~CloseVM()
{

}


void CloseVM::Do()
{
	String logContent;

	log.write("begin CloseVM::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:CLOSE VM:RETN=4,DESC=parameters error";
		return;
	}
	log.write("end process parameter.", Log::INFO);

	//检测虚拟机是否存在
	if( isVMExist() == false)
	{
		ackCode =  "ACK:CLOSE VM:RETN=1001,DESC=vm is not exist";
		return;
	}

	//鉴权	
	if( checkRight( vmID ) == false )
	{		
		ackCode =  "ACK:CLOSE VM:RETN=14,DESC=no right";
		return;	
	}

	if( isVMStarted() == false )
	{
		ackCode =  "ACK:CLOSE VM:RETN=1001,DESC=vm is not started";
		return;
	}

	//先关闭副本虚拟机
	if( closeCopyVM() == false )
	{
		log.write("close the copy vm failed.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=1003,DESC=system internal error，close the copy vm failed";
	}

	if( closeVM() == false )
	{
		log.write("close vm " + vmID + " failed.", Log::ERROR);
		return;
	}

	updateDB();
	
	ackCode = "ACK:CLOSE VM:RETN=0,DESC=success";
}

bool CloseVM::closeCopyVM()
{
	/*String sql = "SELECT vmname FROM CopyVM WHERE MotherVM='";
	sql += vmName + "'";

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	String name;
	String ip;
	while( !query.eof() )
	{
		name = query.getStringField("vmname");
		if( getNCIPByVMName( name, ip) == false )
			return false;
		if( closeVMByName(name, ip) == false )
			return false;
		updateDBByName( name );

		query.nextRow();
	}
*/
	return true;
}

/*
bool CloseVM::closeVMByName(String name, String ip)
{
	String request = "CLOSE VM:vmname=" + name;

	
	int sock = connect2Server( ip.c_str(), g_ncVMServerPort);
	if( sock == -1 )
	{
		log.write("connect nc vm server error.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to nc vm server error.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from nc vm server error.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);
	//分析结果，得出IP
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}	
	
	closeSock( sock );
	ackCode = ack;
	return iRetn == 0;
}

*/




bool CloseVM::updateDB()
{
	

	String sql = "update VMDetail set status=0 where id=" + vmID;
	
	if( executeDBSQL( sql ) == -1 )
	{
		log.write("update started vm info to VMDetail table failed.", Log::ERROR);
		return false;
	}
	return true;

}


void CloseVM::setUserName(String user)
{
	userName = user;
}


bool CloseVM::closeVM()
{
	String request = "CLOSE VM:vmid=" + vmID;

	
	int sock = connect2Server( ncNodeIP.c_str(), g_ncVMServerPort);
	if( sock == -1 )
	{
		log.write("connect nc vm server error.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to nc vm server error.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from nc vm server error.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);
	//分析结果，得出IP
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);
		ackCode =  "ACK:CLOSE VM:RETN=8,DESC=system internal error";
		return false;
	}	
	
	closeSock( sock );
	ackCode = ack;
	return iRetn == 0;
}

bool CloseVM::getPara()
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


bool CloseVM::isVMExist()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select id from VMDetail where id=" + vmID;

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();

	if( rowCnt >= 1 ) 
		return true;
	return false;
	
}

bool CloseVM::isVMStarted()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select host from VMDetail where status=1 and id=" + vmID;


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


/*
bool CloseVM::getNCIPByVMName(String name, String& ip)
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select ncip from OnlineVMS where vmname='";
	sql += name+ "'";

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();
	
	if( rowCnt >= 1 ) 
	{
		ip = query.getStringField( "ncip" );
		return true;
	}
	return false;
	
}
*/







