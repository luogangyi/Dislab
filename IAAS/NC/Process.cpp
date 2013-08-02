#include "Process.h"
#include "../common/Config.h"
#include <sys/socket.h>
#include <sys/types.h>

#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <cstdio>

#include "VM.h"
#include <arpa/inet.h>
#include "StartVM.h"
#include "UnknownCmd.h"
#include <errno.h>
#include "CloseVM.h"
#include <string>
#include "PrepareDir.h"
#include "MigrateVM.h"
#include "ExpandVMMem.h"
#include "GetVNCConsole.h"



#define BUFF_SIZE 4000


using namespace std;

void processCmd(int sock);
VM* GetObject(char* cmd);
extern Log log;


extern String dbServerIP;
extern String dbUserName;
extern String dbUserPass;
extern string bridge;
extern string localip;
extern int heartbeatinterval;


Process::Process()
{
	serverPort = -1;
	listenSocket = -1;
	ConfigFile::strFileName = "nccfg.cfg";
}

Process::~Process()
{
	if ( listenSocket != -1 )
		close(listenSocket);
}

bool Process::Init()
{
	
	bool bRet = ConfigFile::getIntValue("ServerPort", serverPort);	
		
	if( bRet == false )
	{
		printf("read NC vm server port error.\n");
		return false;
	}


	/////∂¡»° ˝æ›ø‚≈‰÷√
	bRet = ConfigFile::getStrValue( "DBServerAddr", dbServerIP );
	
	bool bRet2 = ConfigFile::getStrValue( "DBUserName", dbUserName );
	bool bRet3 = ConfigFile::getStrValue( "DBUserPass", dbUserPass );
	

	if( !bRet || !bRet2 || !bRet3 )
	{
		log.write("read db config error", Log::ERROR);
		return false;
	}

	String tmp;
	bRet = ConfigFile::getStrValue( "Bridge", tmp );
	if( !bRet )
	{
		log.write("read Bridge config error", Log::ERROR);
		return false;
	}

	bridge = tmp.c_str();

	ConfigFile::getIntValue( "heartbeatinterval", heartbeatinterval );
	
	
	//////////////////////////
	bRet = ConfigFile::getStrValue( "NetCard", tmp );
	if( !bRet )
	{
		log.write("read NetCard config error", Log::ERROR);
		return false;
	}
	
	localip = getNetCardIP( tmp.c_str() );
	if( localip == "" )
	{
		log.write("get NetCard ip error", Log::ERROR);
		return false;
	}

	///////////////////////
	if( (listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("create socket error:");
		return false;
	}

	int flag=1,len=sizeof(int); 
	setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &flag, len);
   
	
	struct sockaddr_in serveraddr;

	bzero(&serveraddr, sizeof(serveraddr));
 	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(serverPort);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	

	if (bind(listenSocket,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
    {
        perror("bind to port error:");
        return false;
    }

	return true;
 	
		
}


string Process::getNetCardIP(string dev)
{
	struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct!=NULL) 
    {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { 
			// check it is IP4 
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if( dev == ifAddrStruct->ifa_name )
				return addressBuffer;
             
        }
        ifAddrStruct=ifAddrStruct->ifa_next;
    }

	return "";

}


void Process::Do()
{
	string str;

	char buff[BUFF_SIZE];
	struct sockaddr_in clientAddr;
	int clientSocket = -1;
	
	int iRet;
	
	if (listen(listenSocket,10) < 0)
    {
        str = strerror(errno);
		str = "call socket listen error:" + str + ".";
		log.write(str.c_str(), Log::ERROR);
        return;
    }
	
	
	while(1)
	{
		socklen_t length = sizeof(clientAddr);
        clientSocket= accept(listenSocket,(struct sockaddr*)&clientAddr,&length);
		if ( clientSocket < 0 )
		{
			str = strerror( errno );
			str = "call accept error:" + str + ".";
			log.write(str.c_str(), Log::ERROR);

			continue;
		}
		str = inet_ntoa(clientAddr.sin_addr);
		str = "receive a connection from " + str + ".";
		log.write(str.c_str(), Log::INFO);
		processCmd(clientSocket);
		
	}

}

void processCmd(int sock)
{
		
	int msgLen = 0;
	string str;
	if( recv(sock, &msgLen, sizeof(msgLen), 0) != sizeof(msgLen) )
	{
		str = strerror(errno);
		str = "in processCmd,call recv to receive message size error:" + str + ".";
		log.write(str.c_str(), Log::ERROR);

		close(sock);
		return;
	}

	int recvLen = 0;
	int iRet = 0;
	char buff[BUFF_SIZE];
	memset(buff, 0, sizeof(buff));
	
	while(recvLen<msgLen)
	{
		iRet = recv(sock, buff + recvLen, msgLen, 0);
		
		if ( iRet > 0 )
			recvLen += iRet;
		else
		{
			str = strerror(errno);
			str = "processCmd call recv to receive message error in while:" + str + ".";
			log.write(str.c_str(), Log::ERROR);
			close(sock);
			return ;
		}
	}	

	
	VM* vm = NULL;
	vm = GetObject(buff);
	
	if(vm == NULL)
	{
		log.write("Call GetObject failed,object is NULL",Log::ERROR);
		close(sock);
		return ;
	}

	vm->setSock(sock);
	
	log.write("begin call vm->Do.", Log::INFO);
	vm->Do();
	log.write("call vm->Do down.", Log::INFO);

	
	log.write("begin call vm->sendAck.", Log::INFO);
	vm->sendAck();
	log.write("call vm->sendAck down.", Log::INFO);
	

	delete vm;
	

	return ;
}

VM* GetObject(char* cmd)
{
	char buf[50];
	memset(buf, 0, sizeof(buf));

	string logContent = cmd;
	
	for(int i=0;i<strlen(cmd);i++)
	{
		if( cmd[i] == ':' )
		{
			strncpy(buf, cmd, i);
			cmd += i + 1;
			break;
		}	
		cmd[i] = tolower(cmd[i]);
	}

	string para = "";
	if( strlen( cmd ) != 0)
	{
		para = cmd;		
	}
	
	if ( strcmp(buf, "start vm") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a StartVM object.";
		log.write(logContent, Log::INFO);
		
		logContent = "use para " + para + " to init the StartVM object.";
		log.write(logContent.c_str(), Log::INFO);
		
		VM* vm = new StartVM(para);
		if( vm == NULL)
			perror("new error:");
		return vm;
	}
	else if( strcmp(buf, "close vm") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a CloseVM object.";
		log.write(logContent, Log::INFO);

		logContent = "use para " + para + " to init the CloseVM object.";
		log.write(logContent, Log::INFO);

		VM* vm = new CloseVM(para);
		if( vm == NULL )
		{
			logContent = strerror(errno);	
			logContent = "new CloseVM error:" + logContent + ".";
			log.write(logContent.c_str(), Log::ERROR);
		}
		return vm;
		
	}
	else if( strcmp(buf, "prepare dir") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a PrepareDir object.";
		log.write(logContent, Log::INFO);

		logContent = "use para " + para + " to init the PrepareDir object.";
		log.write(logContent, Log::INFO);

		VM* vm = new PrepareDir(para);
		if( vm == NULL )
		{
			logContent = strerror(errno);		
			logContent = "new PrepareDir error:" + logContent + ".";
			log.write(logContent, Log::ERROR);
		}
		return vm;
		
	}
	else if( strcmp(buf, "migrate vm") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a MigrateVM object.";
		log.write(logContent, Log::INFO);

		logContent = "use para " + para + " to init the MigrateVM object.";
		log.write(logContent, Log::INFO);

		VM* vm = new MigrateVM(para);
		if( vm == NULL )
		{
			logContent = strerror(errno);	
			logContent = "new MigrateVM error:" + logContent + ".";
			log.write(logContent, Log::ERROR);
		}
		return vm;
		
	}
	else if( strcmp(buf, "expand vm") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a ExpandVM object.";
		log.write(logContent, Log::INFO);

		logContent = "use para " + para + " to init the ExpandVM object.";
		log.write(logContent, Log::INFO);

		VM* vm = new ExpandVMMem(para);
		
		if( vm == NULL )
		{
			logContent = strerror(errno);
			logContent = "new ExpandVMMem error:" + logContent + ".";
			log.write(logContent, Log::ERROR);
		}
		return vm;
		
	}
	else if( strcmp(buf, "get vnc console") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a GetVNCConsole object.";
		log.write(logContent, Log::INFO);

		logContent = "use para " + para + " to init the GetVNCConsole object.";
		log.write(logContent, Log::INFO);

		VM* vm = new GetVNCConsole(para);
		
		if( vm == NULL )
		{
			logContent = strerror(errno);
			logContent = "new GetVNCConsole error:" + logContent + ".";
			log.write(logContent, Log::ERROR);
		}
		return vm;
		
	}
	cout<<buf<<endl;


	UnknownCmd* unknown = new UnknownCmd();
	logContent = "receive cmd " + logContent;
	logContent += ",system do not supplied, return a UnknownCmd object.";
	log.write(logContent, Log::ERROR);
	return unknown;
}














