#include "DeleteVM.h"
#include <sys/time.h>

extern Log log;
extern int g_ncVMServerPort;

extern String scServerIP;
extern int scServerPort;
extern String g_gwAddr;

DeleteVM::DeleteVM()
{
	vmID = "";
}

DeleteVM::~DeleteVM()
{

}

void DeleteVM::Do()
{
	String logContent;

	log.write("begin DeleteVM::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:DELETE VM:RETN=4,DESC=parameters error";
		return;
	}
	log.write("end process parameter.", Log::INFO);

	//检测虚拟机是否存在
	if( isVMExist() == false)
	{
		ackCode =  "ACK:DELETE VM:RETN=1001,DESC=vm is not exist";
		return;
	}

	//鉴权	
	if( checkRight( vmID ) == false )
	{		
		ackCode =  "ACK:DELETE VM:RETN=14,DESC=no right";
		return;	
	}

	if( isVMStarted() == true )
	{
		ackCode =  "ACK:DELETE VM:RETN=1002,DESC=vm is running,please close it first";
		return;
	}

	if( getNFSInfo() == false )
	{
		ackCode =  "ACK:DELETE VM:RETN=8,DESC=system internal error";
		return;
	}
	if( mountNFS() == 1 )
	{
		ackCode =  "ACK:DELETE VM:RETN=8,DESC=system internal error";
		return;
	}


	if ( deleteNATRule() == false )
	{
		ackCode =  "ACK:DELETE VM:RETN=8,DESC=system internal error,delete nat rule failed.";
		return;
	}
	
	if( delVMFile() == false )
	{
		ackCode =  "ACK:DELETE VM:RETN=8,DESC=system internal error,delete vm files error.";
		return;
	}

	if( applySC2FreeDisk() == false )
	{
		ackCode =  "ACK:DELETE VM:RETN=8,DESC=system internal error,free disk space error.";
		return;
	}
	
	delVMFromDB();
	
	
	
	ackCode = "ACK:DELETE VM:RETN=0,DESC=success";
}

void DeleteVM::setUserName(String user)
{
	userName = user;
}

bool DeleteVM::delVMFile()
{
	String fileName = localNFSDir + "/" + vmUUID + ".img";
	

	time_t now;
	time(&now);
	char buff[50];
	memset(buff,0,sizeof(buff));
	sprintf(buff,"_%d",now);

	
	String destFileName = fileName + buff + ".waitdel";
	
	//先把要删除的文件改名，定时任务会删除文件
	rename(fileName.c_str(), destFileName.c_str());

	return true;
}

bool DeleteVM::applySC2FreeDisk()
{
	String request = "DEL NFS STOR:ID=" + vmID + ",TYPE=0";
	int sock = connect2Server( scServerIP.c_str(), scServerPort);
	if( sock == -1 )
	{
		log.write("connect sc node server error.", Log::ERROR);
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to sc node server error.", Log::ERROR);
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from sc node server error.", Log::ERROR);
		return false;
	}
	log.write("recv ack message:" + ack + " from sc node server.", Log::INFO);

	
	//分析结果
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para ip error from ack message.", Log::ERROR);
		return false;
	}
	closeSock( sock );
	
	if( iRetn == 0 )
	{
		log.write("apply sc server to free disk success.", Log::INFO);
		return true;
	}
	

	log.write("apply sc server to free disk failed.", Log::ERROR);
	return false;
}


bool DeleteVM::getNFSInfo()
{	
	String sql = "select SID from SCStorDetail where vmid=" + vmID;
	CppMySQLQuery query ;
	

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;
	
	String sid = query.getStringField("SID");

	sql = "SELECT SCSP.IP,SCSP.DIR FROM SCSP INNER JOIN ";
	sql += "SCStorDetail ON SCSP.ID=SCStorDetail.SID WHERE SCStorDetail.SID=" + sid;
	if( executeDBQuery( sql, query) == false )
		return false;	
	
	rowCnt = query.numRow();
	if( rowCnt < 1 ) 
		return false;

	nfsIP = query.getStringField("IP");
	nfsDir = query.getStringField("DIR");
	return true;
}

int DeleteVM::mountNFS()
{
	String logContent;
	char dirName[1024];
	memset(dirName, 0, sizeof(dirName));
	strcpy(dirName, nfsDir.c_str());
	for(int i=0; i<strlen(dirName); i++)
	{
		if( dirName[i] == '/' )
			dirName[i] = '-';
	}
	String temp = dirName + 1;
	temp = nfsIP + "_" + temp;
	
	String dir = getCurrentDir() + "/nfs/" + temp;
	localNFSDir = dir;
	if( !isFileExist(dir) )
	{		
		String mkdir = "mkdir -p " + dir;
		system(mkdir.c_str());
		if( !isFileExist(dir) )
		{
			log.write("create dir " + dir + " error.", Log::ERROR);
			return 1;
		}
	}
	
	/////////	
	
	String mountPoint = "mountpoint " + dir;
	bool bMounted = false;
	int iRet = system( mountPoint.c_str());
	if( iRet == 0 )
		return 0;	
	
	
	
	String mountSrc = nfsIP+ ":" + nfsDir;
	logContent = "begin to mount " + mountSrc + " to " + dir;
	log.write(logContent, Log::INFO);
	if( mount(mountSrc, dir) == false )
	{
		logContent = "mount " + mountSrc + " to " + dir + " failed.";
		log.write(logContent, Log::ERROR);		
		return 1;
	}
	
	logContent = "mount " + mountSrc + " to " + dir + " success.";
	log.write(logContent, Log::INFO);
	return 0;
}


bool DeleteVM::mount( String src, String dest)
{	
	
	String cmd = "mount " + src + " " + dest;
	
	int iRet = system( cmd.c_str() );

	return iRet == 0;
}
 


bool DeleteVM::delVMFromDB()
{
	String sql = "update MACIP set vmid=0 where vmid=" + vmID;
	executeDBSQL( sql );

	
	sql = "delete from SCStorDetail where vmid=" + vmID;
	executeDBSQL( sql );	

	//sql = "DELETE FROM ExtPort2IntIP WHERE PortID IN (SELECT ExtPort.id FROM ExtPort INNER JOIN VMDetail ON VMDetail.ID=ExtPort.VMID WHERE VMDetail.VMNAME='";
	//sql += vmName + "')";
	//executeDBSQL( sql );

	sql = "update ExtPort set vmid=0,IntPort=0 where vmid=";
	sql += vmID;
	executeDBSQL( sql );


	sql = "delete from VMDetail where id=" + vmID;
	executeDBSQL( sql );


	return true;
}


bool DeleteVM::getPara()
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


bool DeleteVM::isVMStarted()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select host from VMDetail where status =1 and id=" + vmID;


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


bool DeleteVM::isVMExist()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select uuid from VMDetail where id=" + vmID;


	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();

	if( rowCnt >= 1 )	
		return true;
	vmUUID = query.getStringField( "uuid" );
	return false;
	
}


bool DeleteVM::deleteNATRule()
{

	String sql = "select ExtPort from ExtPort where vmid=" + vmID;
	CppMySQLQuery query ;

	
	
	if( executeDBQuery( sql, query ) == false )
		return false;

	if( query.numRow() == 0 )
		return true;

	String port = query.getStringField( "ExtPort" );

	String service = g_gwAddr + ":" + port;
	
	
	String cmd = "ipvsadm -D -t " + service;
	
	int iRet = system(cmd.c_str());
		
	
	return true;
}








