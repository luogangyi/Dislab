#include "VM.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <libvirt/libvirt.h>
#include <fstream>
#include <algorithm>

extern Log log;

extern String dbServerIP;
extern String dbUserName;
extern String dbUserPass;
extern CppMySQL3DB db;


VM::VM()
{
	strCmd = "";
	ackCode = "";
	vmName = "";
}

VM::~VM()
{
	close(sock);
}

void VM::Do()
{
	cout<<"in VM::Do"<<endl;
	
}


void VM::setSock(int s)
{
	sock = s;
}

bool VM::sendAck()
{
	string logContent;
	if( ackCode == "" )
		return false;
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	
	int msgLen = ackCode.size();


	strncpy(buff, ackCode.c_str(), msgLen);
	

	if( send(sock, &msgLen, sizeof(msgLen), 0) != sizeof(msgLen))
	{

		log.write("VM::sendAck() send ack message size error.", Log::ERROR);
		
		return false;

	}

	int iSend = 0;

	iSend = send(sock, buff, msgLen, 0);

	if( iSend < 0 )
	{
		log.write("VM::sendAck() call send error.", Log::ERROR);
		return false;
	}
	int iRet = 0;
	while( iSend < msgLen )
	{
		iRet = send(sock, &buff[iSend], msgLen - iSend, 0);
		if ( iRet > 0 )
			iSend + iRet;
		else
		{
			log.write("VM::sendAck() call send error in while.", Log::ERROR);
			return false;
		}
	}
	logContent = "send ack message:" + ackCode;
	log.write( logContent.c_str(), Log::INFO);
	return true;
}



void VM::processPara()
{
	char buff[960];
	char para[100];
	const char * split = "="; 
	memset(buff, 0, sizeof(buff));
	
	strcpy(buff, strCmd.c_str());	
	
	char* p = buff;
	paraMap.clear();
	
	for(int i=0;i<strCmd.size();i++)
	{
		
		if( buff[i] == ',' )
		{
			buff[i] = '\0';
			memset(para, 0, sizeof(para));
			
			strcpy(para, p);
			for(int j=0;j<strlen(para);j++)
			{
				if(para[j] == '=')
					break;
				para[j] = tolower(para[j]);
				
			}
			
			p = buff + i + 1;		
			
			string str1 = strtok (para,split);
			string str2 = strtok (NULL,split);
			
			paraMap.insert(pair<string, string>(str1, str2));
			
		}
	}

	//此处存在一个漏洞，会使程序core，要修改
	//用NULL初始化string会core
	char * tmp = strtok (p,split);
	if( tmp )
	{
		string str1 = tmp;
		string str2 = strtok (NULL,split);
		transform(str1.begin(),str1.end(),str1.begin(), ::tolower);
		//str1.tolower();
		paraMap.insert(pair<string, string>(str1, str2));
	}	
	
}

//判断虚拟机是否已启动
//返回值：0-未启动1-已启动2-系统内部错误
int VM::isAlreadyStart()
{
	
    
    virConnectPtr conn = NULL;
    virDomainPtr dom = NULL;

	
    conn = virConnectOpen("");
	
	if (conn == NULL)
	{		
		log.write("StartVM::isAlreadyStart() Failed to connect to hypervisor.", Log::ERROR);
		return 2;
	}
	
	dom = virDomainLookupByUUIDString(conn,vmUUID.c_str());

	if( dom == NULL )
	{
		
		virConnectClose(conn);
		return 0;
	}

	virDomainInfo info;
	if( virDomainGetInfo( dom, &info ) == 0 )
	{		
		if( info.cpuTime > 0 )
		{
			virDomainFree(dom);    
	    	virConnectClose(conn);
			return 1;
		}		
	}
   
    virDomainFree(dom);    
    virConnectClose(conn);

	return 0;
	
	
}

int VM::executeDBSQL(string sql)
{
	string logContent;
	int iRet = 0;
	if( db.ping() == -1 )
	{
		log.write("mysql not opened,now open mysql",Log::INFO);
		iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	
		if ( iRet != 0 )
		{
			logContent = "open mysql db failed";
			log.write(logContent.c_str(), Log::ERROR);
			return -1;
		}	
	}
	
	
	try
	{
		logContent = "execute: " + sql + ".";
		log.write( logContent.c_str(), Log::INFO);
		iRet = db.execSQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent.c_str(), Log::ERROR);
		return -1;
	}	
	return iRet;
}


int VM::executeDBSQL(String sql)
{
	string sql1 = sql.c_str();
	return executeDBSQL( sql1 );
}

bool VM::executeDBQuery( string sql, CppMySQLQuery & result)
{
	
	String logContent;	

	int iRet;
	
	if( db.ping() == -1 )
	{		
		log.write("mysql not opened,now open mysql",Log::INFO);
		iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	
		if ( iRet != 0 )
		{
			logContent = "open mysql db failed";
			log.write(logContent, Log::ERROR);
			return false;
		}	
	}
	
	
	try
	{
		logContent = "execute: ";
		logContent += sql.c_str();
		log.write( logContent, Log::INFO);
		result = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute: " ;
		logContent += sql.c_str();
		logContent += " failed:";
		logContent += sErr;
		log.write( logContent, Log::ERROR);
		return false;
	}	
	return true;
}



bool VM::executeDBQuery( String sql, CppMySQLQuery & result)
{
	
	string sql1 = sql.c_str();
	executeDBQuery( sql1, result );
}



