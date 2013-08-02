#include "Log.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include<stdlib.h>
#include<stdio.h>

using namespace std;


Log::Log()
{
	
}

Log::~Log()
{
	close(sock);
}

bool Log::Init(String ip, int port)
{
	serverIP = ip;
	serverPort = port;
	struct sockaddr_in serveraddr;

	bzero(&serveraddr, sizeof(serveraddr));
 	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(serverPort);
	const char *local_addr=serverIP.c_str();
    inet_aton(local_addr,&(serveraddr.sin_addr));


    sock = socket(AF_INET, SOCK_STREAM, 0);
	int iRet = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	if ( iRet !=0 )
	{
	   perror("connect log server error:");
	   return false;
	}

	return true;
	
}

void Log::write( String content, int type)
{
	char buff[9600];
	String contentType;
	time_t t = time(0);
	char tmp[64];
	strftime( tmp, sizeof(tmp), "%Y/%m/%d %X %A ",localtime(&t) );
	
	
	if( type == Log::INFO )
		contentType = "[INFO] ";
	else if( type == Log::WARN )
		contentType = "[WARN] ";
	else if( type == Log::ERROR )
		contentType = "[ERROR] ";
	memset(buff, 0, sizeof(buff));
	sprintf(buff,"%s%s%s", tmp, contentType.c_str(), content.c_str());

	cout<<buff<<endl;
}



