#include "Process.h"
#include "../../common/Config.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "Object.h"
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "UnknownCmd.h"
#include <errno.h>
#include "../../common/String.h"
#include "Login.h"
#include "CreateVM.h"
#include "StartVM.h"
#include "CloseVM.h"
#include "DeleteVM.h"
#include "QueryVM.h"
#include <sys/time.h>
#include "GetVNCConsole.h"

#define BUFF_SIZE 4000


using namespace std;

int g_templateCnt = 0;

extern void* threadFunc(void* arg);
Object* GetObject(char* cmd);
extern Log log;

extern String dbServerIP;
extern String dbUserName;
extern String dbUserPass;
extern String scServerIP;
extern int scServerPort;

extern String g_dispatchServerIP;
extern int g_dispatchServerPort;
extern int g_ncVMServerPort;
extern String g_gwAddr;

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
	
	bool bRet = ConfigFile::getIntValue("CLCVMServerPort", serverPort);	
		
	if( bRet == false )
	{
		log.write("read CLCVMServerPort error,exit.", Log::ERROR);
		return false;
	}

	//max vm which a user could hava
	bRet = ConfigFile::getIntValue("UserMaxVM", CreateVM::UserMaxVM);	
	if( bRet == false )
	{
		log.write("read UserMaxVM error,exit.", Log::ERROR);
		return false;
	}
	//DBServer IP
	bRet = ConfigFile::getStrValue( "DBServerAddr", dbServerIP );
	
	bool bRet2 = ConfigFile::getStrValue( "DBUserName", dbUserName );
	bool bRet3 = ConfigFile::getStrValue( "DBUserPass", dbUserPass );


	if( !bRet || !bRet2 || !bRet3 )
	{
		log.write("read db config error", Log::ERROR);
		return false;
	}


	bRet = ConfigFile::getIntValue("TemplateFileCnt", g_templateCnt);		
	if( bRet == false )
	{
		log.write("read TemplateFileCnt error.", Log::ERROR);
		return false;
	}
	

	

	bRet = ConfigFile::getIntValue("SCServerPort", scServerPort);		
	if( bRet == false )
	{
		log.write("read SCServerPort port error.", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue("SCServerIP", scServerIP);		
	if( bRet == false )
	{
		log.write("read SCServerIP ip error.", Log::ERROR);
		return false;
	}

	//DispatchServerIP
	bRet = ConfigFile::getStrValue("DispatchServerIP", g_dispatchServerIP);		
	if( bRet == false )
	{
		log.write("read DispatchServerIP ip error.", Log::ERROR);
		return false;
	}
	bRet = ConfigFile::getIntValue("DispatchServerPort", g_dispatchServerPort);		
	if( bRet == false )
	{
		log.write("read DispatchServerPort ip error.", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getIntValue("NCVMServerPort", g_ncVMServerPort);		
	if( bRet == false )
	{
		log.write("read NCVMServerPort  error.", Log::ERROR);
		return false;
	}
	
	bRet = ConfigFile::getStrValue("GWAddr", g_gwAddr);		
	if( bRet == false )
	{
		log.write("read GWAddr  error.", Log::ERROR);
		return false;
	}


	///Init Login class
	bRet = login.init();
	if( bRet == false )
	{		
		return false;
	}


	if( (listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		log.write("create socket error", Log::ERROR);
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
        log.write("bind to port error", Log::ERROR);
        return false;
    }

	return true;
 	
		
}

void Process::setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}


void Process::Do()
{
	int  nfds;
	String str;


    struct epoll_event ev,events[50];

    epfd=epoll_create(1024);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
   
	

    setnonblocking( listenSocket );


    ev.data.fd = listenSocket;

    ev.events=EPOLLIN|EPOLLET;


    epoll_ctl(epfd,EPOLL_CTL_ADD,listenSocket, &ev);
   
    if (listen(listenSocket,10) < 0)
    {
        str = strerror(errno);
		log.write("call socket listen error:" + str + ".", Log::ERROR);
        return;
    }

	
	int acceptFD;
	socklen_t clilen;
	int sockfd,n;
	char buff[BUFF_SIZE];
    int msgLen = 0;
	int iRecv = 0;
	int recvLen = 0;
	int iRet = 0;
	time_t startTime,endTime;
	int usedTime;
	bool bFlag;
    for ( ; ; ) {

        nfds = epoll_wait(epfd,events,50,500);

        for(int i=0; i < nfds; i++ )
        {
            if(events[i].data.fd == listenSocket)
            {
            	
				clilen = sizeof(clientaddr);
                acceptFD = accept(listenSocket,(sockaddr *)&clientaddr, &clilen);
                if ( acceptFD < 0 )
				{
					str = strerror(errno);
					log.write("call accept error:" + str + ".", Log::ERROR);
					continue;
				}
				
                setnonblocking( acceptFD );

              	str = inet_ntoa(clientaddr.sin_addr);
				log.write("receive a connection from " + str + ".", Log::INFO);

                ev.data.fd = acceptFD;
                ev.events=EPOLLIN|EPOLLET;
                cout<<"receive connection!"<<endl;
                //ev.events=EPOLLIN;

                epoll_ctl(epfd,EPOLL_CTL_ADD,acceptFD,&ev);
            }
            else if( events[i].events&EPOLLIN )
            {				
				
				if ( (sockfd = events[i].data.fd) < 0)
                    continue;

				////
				char sizeBuff[10];
				memset(sizeBuff, 0, 10);
				
				iRecv = recv(sockfd, sizeBuff, 4, 0);
				String sTmp = "receive the command size:" ;
				sTmp += sizeBuff;
				log.write(sTmp, Log::INFO);
				//cout<<"buff:"<<sizeBuff<<endl;
				msgLen = atoi(sizeBuff);
				//cout<<"len:"<<msgLen<<endl;
				///
				//iRecv = recv(sockfd, &msgLen, sizeof(msgLen), 0);
				//cout<<"msgLen:"<<msgLen<<endl;
				//cout<<"iRecv:"<<iRecv<<endl;
				
				if( iRecv <= 0 )
				{
					log.write("clinet close the connection.", Log::INFO);
					deleteSockFromEpoll( sockfd );								
					continue;
				}
				recvLen = 0;
				memset(buff, 0, sizeof(buff));
				time ( &startTime );
				bFlag = true;
				while(recvLen<msgLen)
				{
					iRet = recv(sockfd, buff + recvLen, msgLen, 0);
					//cout<<"buff:"<<buff<<endl;
					if ( iRet > 0 )
						recvLen += iRet;
					else if( iRet == -1 && EAGAIN == errno )
					{
						
						time ( &endTime );
						usedTime = endTime - startTime;

						if( usedTime > 3 )
						{						
							log.write("recv more than 4S, close the socket.", Log::ERROR);
							deleteSockFromEpoll( sockfd );
							bFlag = false;
							break;
						}
						continue;
					}
					else
					{						
						str = strerror(errno);
						log.write("call recv to receive message error in while:" + str + ".", Log::ERROR);
						deleteSockFromEpoll( sockfd );
						bFlag = false;
						break;
					}
					
					
					time ( &endTime );
					usedTime = endTime - startTime;

					if( usedTime > 1 )
					{						
						log.write("recv more than 2S, close the socket.", Log::ERROR);
						deleteSockFromEpoll( sockfd );
						bFlag = false;
						break;
					}


				}	
				cout<<buff<<endl;
				if( bFlag == true )
					processCmd( buff, sockfd);
                


            }
           
        }
    }
	

}

void Process::processCmd(char * cmd, int sock)
{

	char buf[50];
	memset(buf, 0, sizeof(buf));
	

	String logContent = cmd;
	logContent = "receive cmd " + logContent + ".";
	
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

	String para = "";
	if( strlen( cmd ) != 0)
	{
		para = cmd;		
	}
	
	cmdType type = analyzeCmd( buf );

	//if user doesn't login yet, close channel
	if( isUserLogin( sock ) == false && type != LOGIN && type != GETVNCCONSOLE)
	{
		logContent = "receive a cmd " + logContent;
		logContent += ",but the user not login,close the connection.";
		log.write( logContent, Log::WARN );
		deleteSockFromEpoll( sock );
		return ;
	}

	
	
	log.write(logContent, Log::INFO);
	
	if( type == LOGIN )
	{		
		
		login.setSock(sock);
		login.setPara( para );
		//cout<<"para:"<<para<<endl;
		login.Do();
		String ackCode = login.getAck();		
		int iRetn;
		bool bRet = getIntParaValue( ackCode, "retn", iRetn);
		if( bRet == false )
		{
			log.write("analyze auth server ack code error,no RETN para found.", Log::ERROR);
			login.sendAck();
			deleteSockFromEpoll( sock );
			return ;
		}

		if( iRetn == 0 )
		{
			String result;
			String user;
			getStrParaValue( ackCode, "result", result);
			getStrParaValue2( para, "user", user);
			//cout<<"result:"<<result<<endl;
			saveUserPermissionData(user, result, sock);
			//cout<<"xxx"<<result<<endl;
			sendAck( "ACK:LOGIN:RETN=0,DESC=success", sock);
			return;
		}
		login.sendAck();
		deleteSockFromEpoll( sock );
	}
	else if( type == CREATEVM )
		processCreateVMCmd( sock, para);
	else if( type == STARTVM )
		processStartVMCmd( sock, para);
	else if( type == CLOSEVM )
		processCloseVMCmd( sock, para);
	else if( type == DELETEVM )
		processDeleteVMCmd( sock, para);
	else if( type == QUERYVM )
		processQueryVMCmd( sock, para);
	else if( type == GETVNCCONSOLE )
		processGetVNCConsoleCmd( sock, para);
	else
		processUnknownCmd( sock );
}


void Process::processDeleteVMCmd(int sock, String para)
{
	String logContent;
	DeleteVM dvm;
	String authData = getUserPermissionDataBySock( sock );
	dvm.setAuthData( authData );
	dvm.setUserName( getUserNameBySock( sock ));
	dvm.setSock(sock);
	dvm.setPara( para );
	dvm.Do();	
	dvm.sendAck();
}


void Process::processCloseVMCmd(int sock, String para)
{
	String logContent;
	CloseVM cvm;
	String authData = getUserPermissionDataBySock( sock );

	cvm.setAuthData( authData );
	cvm.setUserName( getUserNameBySock( sock ));
	cvm.setSock(sock);
	cvm.setPara( para );
	cvm.Do();	
	cvm.sendAck();
}

void Process::processUnknownCmd( int sock )
{
	UnknownCmd cmd;
	cmd.Do();
	cmd.setSock( sock );
	cmd.sendAck();
}


void Process::processCreateVMCmd(int sock, String para)
{
	String logContent;
	CreateVM cvm;

	cvm.setUserName( getUserNameBySock( sock ));
	cvm.setSock(sock);
	cvm.setPara( para );
	cvm.Do();	
	cvm.sendAck();

	if( cvm.isSuccess() )
	{
		appendUserPermissionData( cvm.getVMName(), sock);
	}
}


void Process::processQueryVMCmd(int sock, String para)
{
	String logContent;
	QueryVM qvm;
	String authData = getUserPermissionDataBySock( sock );
	qvm.setAuthData( authData );
	qvm.setUserName( getUserNameBySock( sock ));
	qvm.setSock(sock);
	qvm.setPara( para );
	qvm.Do();	
	qvm.sendAck();
}


void Process::processGetVNCConsoleCmd(int sock, String para)
{
	String logContent;
	GetVNCConsole vnc;
	String authData = getUserPermissionDataBySock( sock );
	vnc.setAuthData( authData );
	//vnc.setUserName( getUserNameBySock( sock ));
	vnc.setSock(sock);
	vnc.setPara( para );
	vnc.Do();	
	vnc.sendAck();
}


void Process::processStartVMCmd(int sock, String para)
{
	String logContent;
	String authData = getUserPermissionDataBySock( sock );	
	StartVM svm;
	svm.setAuthData( authData );
	svm.setUserName( getUserNameBySock( sock ));
	svm.setSock(sock);
	svm.setPara( para );
	svm.Do();	
	svm.sendAck();
}

String Process::getUserNameBySock(int sock)
{
	String user = "";
	map<int,userPermissionData*>::iterator it;

	
	it = userPermissionDataMap.find( sock );
	if( it != userPermissionDataMap.end() )
	{
		user = it->second->user;
	}
	return user;
}

String Process::getUserPermissionDataBySock(int sock)
{
	String data = "";
	map<int,userPermissionData*>::iterator it;

	
	it = userPermissionDataMap.find( sock );
	if( it != userPermissionDataMap.end() )
	{
		data = it->second->data;
	}
	return data;
}

bool Process::isUserLogin(int sock)
{
	map<int,userPermissionData*>::iterator it;

	
	it = userPermissionDataMap.find( sock );
	if( it == userPermissionDataMap.end() )		
		return false;
	return true;		
	
}

void Process::saveUserPermissionData(String user, String data, int sock)
{	
			
	map<int,userPermissionData*>::iterator it;

	
	it = userPermissionDataMap.find( sock );
	if( it == userPermissionDataMap.end() )		
	{
		userPermissionData* pdata = new userPermissionData;
		pdata->user = user;
		pdata->data = data;
		userPermissionDataMap.insert(pair<int, userPermissionData*>(sock, pdata));	
	}
	else
	{
		userPermissionData* pdata = it->second;
		pdata->user = user;
		pdata->data = data;
	}

	for(it = userPermissionDataMap.begin(); it != userPermissionDataMap.end(); it++ )
	{
		if( it->first != sock && it->second->user == user )
		{
			struct epoll_event ev;
			ev.data.fd = it->first;	                
			epoll_ctl(epfd,EPOLL_CTL_DEL,it->first,&ev);	
			close( it->first );
			userPermissionDataMap.erase( it );
			
			break;
		}
	}
	
}


void Process::appendUserPermissionData( String data, int sock)
{	
			
	map<int,userPermissionData*>::iterator it;
	
	it = userPermissionDataMap.find( sock );
	if( it != userPermissionDataMap.end() )		
	{
		userPermissionData* pdata = it->second;		
		pdata->data += data;
	}	
	
}

void Process::deleteSockFromEpoll(int sock)
{
	struct epoll_event ev;
	ev.data.fd = sock;	                
	epoll_ctl(epfd,EPOLL_CTL_DEL,sock,&ev);	

	map<int,userPermissionData*>::iterator it;

	
	it = userPermissionDataMap.find( sock );
	if( it != userPermissionDataMap.end() )		
	{
		userPermissionData* pdata = it->second;
		if( pdata != NULL )
			delete pdata;
		userPermissionDataMap.erase( it );
	}
	
	close(sock);		
	
}

Process::cmdType Process::analyzeCmd(char* cmd)
{	

	if ( strcmp(cmd, "login") == 0 )
	{		
		return Process::LOGIN;
	}
	else if( strcmp(cmd, "cre vm") == 0 )
		return Process::CREATEVM;
	else if( strcmp(cmd, "start vm") == 0 )
		return Process::STARTVM;
	else if( strcmp(cmd, "close vm") == 0 )
		return Process::CLOSEVM;
	else if( strcmp(cmd, "delete vm") == 0 )
		return Process::DELETEVM;
	else if( strcmp(cmd, "query vm") == 0 )
		return Process::QUERYVM;
	else if( strcmp(cmd, "get vnc console") == 0 )
		return Process::GETVNCCONSOLE;
	return Process::UNKNOWN;
}

bool Process::getIntParaValue(String cmdAck, String item, int& iValue)
{
	String sValue;
	bool bRet = getStrParaValue( cmdAck, item, sValue);
	if( bRet == false )
		return false;
	iValue = atoi( sValue.c_str() );
	return true;
}

bool Process::getIntParaValue2(String cmdAck, String item, int& iValue)
{
	String sValue;
	bool bRet = getStrParaValue2( cmdAck, item, sValue);
	if( bRet == false )
		return false;
	iValue = atoi( sValue.c_str() );
	return true;
}

bool Process::getStrParaValue(String cmdAck, String item, String& sValue)
{
	char buff[4096];
	char para[4096];
	char cTmp[4096];
	const char * split = "="; 
	memset(buff, 0, sizeof(buff));
	memset(cTmp, 0, sizeof(buff));
	
	strcpy(cTmp, cmdAck.c_str());	
	
	char* p = buff;

	int iCnt = 0;
	for(int i=0;i<cmdAck.size();i++)
	{
		if( cTmp[i] == ':' )
			iCnt++;
		if( iCnt == 2 )
		{
			strcpy(buff, &cTmp[i+1]);
			break;
		}
	}
	
	
	for(int i=0;i<cmdAck.size();i++)
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
			
			String str1 = strtok (para,split);
			String str2 = strtok (NULL,split);
			
			if( str1 == item )
			{
				sValue = str2;
				return true;
			}
			
		}
	}

	//此处存在一个漏洞，会使程序core，要修改
	//用NULL初始化string会core
	char * tmp = strtok (p,split);
	if( tmp )
	{
		String str1 = tmp;
		String str2 = strtok (NULL,split);
		sValue = str2;
		return true;
	}	
	return false;
}

bool Process::getStrParaValue2(String cmdAck, String item, String& sValue)
{
	char buff[4096];
	char para[4096];
	
	const char * split = "="; 
	memset(buff, 0, sizeof(buff));
	
	
	strcpy(buff, cmdAck.c_str());	
	
	char* p = buff;	
	
	for(int i=0;i<cmdAck.size();i++)
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
			
			String str1 = strtok (para,split);
			String str2 = strtok (NULL,split);
			
			if( str1 == item )
			{
				sValue = str2;
				return true;
			}
			
		}
	}

	//此处存在一个漏洞，会使程序core，要修改
	//用NULL初始化string会core
	char * tmp = strtok (p,split);
	if( tmp )
	{
		String str1 = tmp;
		String str2 = strtok (NULL,split);
		sValue = str2;
		return true;
	}	
	return false;
}

Object* GetObject(char* cmd)
{
	char buf[50];
	memset(buf, 0, sizeof(buf));

	String logContent = cmd;

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

	String para = "";
	if( strlen( cmd ) != 0)
	{
		para = cmd;		
	}
	
	/*if ( strcmp(buf, "login") == 0 )
	{
		logContent = "receive cmd " + logContent;
		logContent += " and return a Login object.";
		log.write(logContent, Log::INFO);
		
		logContent = "use para " + para + " to init the Login object.";
		log.write(logContent, Log::INFO);
		
		Object* obj = new Login(para);
		if( obj == NULL)
			perror("new error:");
		return obj;
		
	}
	*/

	UnknownCmd* unknown = new UnknownCmd();
	logContent = "receive cmd " + logContent;
	logContent += ",system do not supplied, return a UnknownCmd object.";
	log.write(logContent, Log::ERROR);
	return unknown;
}

bool Process::sendAck( String ackCode, int sock)
{
	String logContent;
	
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	
	int msgLen = ackCode.size();
	char lenBuff[10];
	memset(lenBuff, 0, sizeof(lenBuff));
	sprintf(lenBuff, "%04d", msgLen);

	strncpy(buff, ackCode.c_str(), msgLen);

	
	int len = strlen(lenBuff);
	//send(sock,lenBuff, len, 0);

	if( send(sock,lenBuff, len, 0) != len)
	{

		log.write("Process::sendAck send ack message size error.", Log::ERROR);
		
		return false;

	}

	int iSend = 0;

	iSend = send(sock, buff, msgLen, 0);

	if( iSend < 0 )
	{
		log.write("Process::sendAck call send error.", Log::ERROR);
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
			log.write("Process::sendAck call send error in while.", Log::ERROR);
			return false;
		}
	}

	log.write( "Process::sendAck send ack message:" + ackCode, Log::INFO);
	return true;
}














