#include "Login.h"
#include "../../common/Config.h"
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

extern Log log;


Login::Login()
{
	
	authSock = -1;
}

Login::~Login()
{
	if( authSock != -1)
		close( authSock );
}

void Login::Do()
{
	log.write("begin Login::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:LOGIN:RETN=4,DESC=Wrong Parameter";
		return;
	}
	log.write("end process parameter.", Log::INFO);


	String cmd = "login:user=" + userName + ",passwd=" + userPass;


	log.write("begin to send login request to auth server.", Log::INFO);

	if( send2AuthServer( cmd ) == false )
	{
		log.write("send login request to auth server failed first,try again.", Log::INFO);
		close( authSock );
		init();
		if( send2AuthServer( cmd ) == false )
		{
			log.write("send login request to auth server failed.", Log::ERROR);
			ackCode = "ACK:LOGIN:RETN=8,DESC=send login request to auth server failed";

			return ;
		}
		else
		{
			log.write("send login request to auth server success.", Log::INFO);

			log.write("begin to read auth server ack.", Log::INFO);
			ackCode= recvAckFromAuthServer();
			if( ackCode == "" )
			{
				log.write("read auth server ack failed.", Log::ERROR);
				ackCode = "ACK:LOGIN:RETN=8,DESC=read auth server ack failed.";			
			}
			log.write("read auth server ack success.", Log::INFO);
		}
		return;
	}
	log.write("send login request to auth server success.", Log::INFO);
	log.write("begin to read auth server ack.", Log::INFO);
	ackCode= recvAckFromAuthServer();
	if( ackCode == "" )
	{
		log.write("read auth server ack failed.", Log::ERROR);
		ackCode = "ACK:LOGIN:RETN=8,DESC=read auth server ack failed.";			
	}	
	log.write("read auth server ack success.", Log::INFO);
	

}

String Login::recvAckFromAuthServer()
{
	int msgLen = 0;
	String str;
	int recvLen = 0;
	int iRet = 0;
	char buff[BUFF_SIZE];
	int iRecv = 0;
	String ackMsg = "";
	recvLen = 0;
	iRecv = recv(authSock, &msgLen, sizeof(msgLen), 0);

	if( iRecv <= 0 )
	{
		log.write("receive ack message size from auth server error.", Log::INFO);
		close(authSock);
		return ackMsg;
	}
	if( iRecv != sizeof(msgLen) )
	{
		str = strerror(errno);
		log.write("receive ack message size from auth server error.", Log::ERROR);

		close(authSock);
		return ackMsg;
	}

	memset(buff, 0, sizeof(buff));
	while(recvLen<msgLen)
	{
		iRet = recv(authSock, buff + recvLen, msgLen, 0);
		if ( iRet > 0 )
			recvLen += iRet;
		else
		{
			str = strerror(errno);
			log.write("receive ack message from auth server error.:" + str + ".", Log::ERROR);
			close(authSock);
			return ackMsg;
		}
	}
	ackMsg = buff;
	return ackMsg;
}

bool Login::send2AuthServer(String cmd)
{
	String logContent;
	
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	
	int msgLen = cmd.size();


	strncpy(buff, cmd.c_str(), msgLen);
	
	
	if( send(authSock, &msgLen, sizeof(msgLen), 0) != sizeof(msgLen))
	{
		
		log.write("Login::send2AuthServer(String cmd) send message size error.", Log::ERROR);
		
		return false;

	}

	int iSend = 0;

	iSend = send(authSock, buff, msgLen, 0);

	if( iSend < 0 )
	{
		log.write("Login::send2AuthServer(String cmd) call send error.", Log::ERROR);
		return false;
	}
	int iRet = 0;
	while( iSend < msgLen )
	{
		iRet = send(authSock, &buff[iSend], msgLen - iSend, 0);
		if ( iRet > 0 )
			iSend + iRet;
		else
		{
			log.write("Login::send2AuthServer(String cmd) call send error in while.", Log::ERROR);
			return false;
		}
	}

	
	return true;
}


bool Login::init()
{
	/////////读取配置文件
	int authServerPort;
	bool bRet = ConfigFile::getIntValue("AuthServerPort", authServerPort);	
		
	if( bRet == false )
	{
		printf("Login::init() read AuthServerPort error.\n");
		return false;
	}

	String authServerAddr;

	bRet = ConfigFile::getStrValue( "AuthServerIP", authServerAddr);
	if( bRet == false )
	{
		printf("Login::init() read AuthServerIP error.\n");
		return false;
	}
	/////////////////////////	

	if( ( authSock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Login::init() create socket error:");
		return false;
	}
   
	
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;

		
	inet_aton(authServerAddr.c_str(), &(serveraddr.sin_addr));
	serveraddr.sin_port=htons(authServerPort);

	int iRet = connect(authSock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	if ( iRet != 0 )
	{
		perror("Login::init() connect Auth server error");
		return false;
	}
	
	return true;
}



bool Login::getPara()
{
	
	processPara();


	if( paraMap.size() != 2 )
		return false;
	
	map<String,String>::iterator it;

	
	it = paraMap.find("user");
	if( it == paraMap.end() )		
		return false;
	

	userName = it->second;
	
	if( userName.size() == 0 )
		return false;	

	it = paraMap.find("passwd");
	if( it == paraMap.end() )		
		return false;
	

	userPass= it->second;
	
	if( userPass.size() == 0 )
		return false;	

	
	return true;
}





