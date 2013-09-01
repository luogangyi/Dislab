#include "Process.h"
#include "../../common/Config.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "Object.h"
#include <pthread.h>
#include <arpa/inet.h>
#include "Dispatch.h"
#include "UnknownCmd.h"
#include <errno.h>



#define BUFF_SIZE 4000


using namespace std;

extern void* threadFunc(void* arg);
Object* GetObject(char* cmd);
extern Log log;


Process::Process()
{
	serverPort = -1;
	listenSocket = -1;
	ConfigFile::strFileName = "clccfg.cfg";
}

Process::~Process()
{
	if ( listenSocket != -1 )
		close(listenSocket);
}

bool Process::Init()
{
	
	bool bRet = ConfigFile::getIntValue("DispatchServerPort", serverPort);	
		
	if( bRet == false )
	{
		printf("read DispatchServerPort error.\n");
		return false;
	}

	//读取数据库配置信息
	bRet = ConfigFile::getStrValue( "DBServerAddr", Dispatch::dbServerIP );
	
	bool bRet2 = ConfigFile::getStrValue( "DBUserName", Dispatch::dbUserName );
	bool bRet3 = ConfigFile::getStrValue( "DBUserPass", Dispatch::dbUserPass );

	if( !bRet || !bRet2 || !bRet3 )
	{
		log.write("read db config error", Log::ERROR);
		return false;
	}

	
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


void Process::Do()
{
	String str;

	char buff[BUFF_SIZE];
	struct sockaddr_in clientAddr;
	int clientSocket = -1;
	pthread_t pid;
	int iRet;
	
	if (listen(listenSocket,10) < 0)
    {
        str = strerror(errno);
		log.write("call socket listen error:" + str + ".", Log::ERROR);
        return;
    }
	
	
	while(1)
	{
		socklen_t length = sizeof(clientAddr);
        clientSocket= accept(listenSocket,(struct sockaddr*)&clientAddr,&length);
		if ( clientSocket < 0 )
		{
			str = strerror(errno);
			log.write("call accept error:" + str + ".", Log::ERROR);

			continue;
		}
		str = inet_ntoa(clientAddr.sin_addr);
		log.write("receive a connection from " + str + ".", Log::INFO);
		iRet = pthread_create(&pid, NULL, threadFunc, (void*)&clientSocket);
		if ( iRet )
		{	
			str = strerror(errno);			
			log.write("create thread error:" + str + ".", Log::ERROR);
			close(clientSocket);			
		}
	}

}

void* threadFunc(void* arg)
{
	
	
	int sock = *((int*)arg);
	
	int msgLen = 0;
	String str;
	int recvLen = 0;
	int iRet = 0;
	char buff[BUFF_SIZE];
	int iRecv = 0;

	while(1)
	{
		recvLen = 0;
		iRecv = recv(sock, &msgLen, sizeof(msgLen), 0);
		//cout<<"len:"<<msgLen<<endl;
		if( iRecv == -1 )
		{
			log.write("socket is closed by peer.", Log::INFO);
			close(sock);
			return NULL;
		}
		if( iRecv != sizeof(msgLen) )
		{
			str = strerror(errno);
			log.write("in threadFunc,call recv to receive message size error.", Log::ERROR);

			close(sock);
			return NULL;
		}
		
		memset(buff, 0, sizeof(buff));
		while(recvLen<msgLen)
		{
			iRet = recv(sock, buff + recvLen, msgLen, 0);
			if ( iRet > 0 )
				recvLen += iRet;
			else
			{
				str = strerror(errno);
				log.write("threadFunc call recv to receive message error in while:" + str + ".", Log::ERROR);
				close(sock);
				return NULL;
			}
		}	


		Object* obj = NULL;
		obj = GetObject(buff);
		
		if(obj == NULL)
		{
			log.write("Call GetObject failed,object is NULL",Log::ERROR);
			close(sock);
			return NULL;
		}

		obj->setSock(sock);
		
		log.write("begin call obj->Do.", Log::INFO);
		obj->Do();
		log.write("call obj->Do down.", Log::INFO);

			
		log.write("begin call obj->sendAck.", Log::INFO);
		obj->sendAck();
		log.write("call obj->sendAck down.", Log::INFO);
		

		delete obj;	
		
	}
	pthread_detach(pthread_self());
	return NULL;
}

Object* GetObject(char* cmd)
{
	char buf[50];
	memset(buf, 0, sizeof(buf));

	String logContent = cmd;

	for(int i=0;i<strlen(cmd);i++)
	{
		cmd[i] = tolower(cmd[i]);
	}

	for(int i=0;i<strlen(cmd);i++)
	{
		if( cmd[i] == ':' )
		{
			strncpy(buf, cmd, i);
			cmd += i + 1;
			break;
		}		
	}

	String para = "";
	if( strlen( cmd ) != 0)
	{
		para = cmd;		
	}
	
	if ( strcmp(buf, "get best nc") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a Dispatch object.";
		log.write(logContent, Log::INFO);
		
		logContent = "use para " + para + " to init the Dispatch object.";
		log.write(logContent, Log::INFO);
		
		Object* obj = new Dispatch(para);
		if( obj == NULL)
			perror("new error:");
		return obj;
	}
	

	UnknownCmd* unknown = new UnknownCmd();
	logContent = "receive cmd " + logContent;
	logContent += ",system do not supplied, return a UnknownCmd object.";
	log.write(logContent, Log::ERROR);
	return unknown;
}














