
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#include<stdarg.h>
#include<string.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include"String.h"
#include"CMDsender.h"
using namespace std;


CMDsender::CMDsender(String IP,int portno)
{
	strcpy(serverIP,IP.c_str());
	serverPort=portno;
}

char* CMDsender::sendCMD(String cmd)
{
	int sockfd,ret;
	sockaddr_in ser_addr;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		cout<<"error in socket";
		return false;
	}
	ser_addr.sin_family=AF_INET;
	ser_addr.sin_port=htons(serverPort);
	inet_aton(serverIP,&(ser_addr.sin_addr));
	bzero(ser_addr.sin_zero,8);
	ret=connect(sockfd,(sockaddr*)&ser_addr,sizeof(sockaddr));
	if(ret==0)
	{
		cout<<"success connect"<<endl;
	}
	else
	{
		cout<<"connect error"<<endl;
		return NULL;
	}
	int cmdlen;
	//发送信息
	cmdlen=sizeof(cmd.c_str());
	send(sockfd,&cmdlen,sizeof(cmdlen),0);
	send(sockfd,cmd.c_str(),cmdlen,0);


	//接受返回信息

	int msgLen = 0;
	int recvLen = 0;
	int iRet = 0;
	char *buff=new char[64];
	int iRecv = 0;

	iRecv = recv(sockfd, &msgLen, sizeof(msgLen), 0);

	if( iRecv <= 0 )
	{
		cout<<"client close the connection."<<endl;
		close(sockfd);
		return NULL;
	}
	if( iRecv != sizeof(msgLen) )
	{

		printf("in threadFunc,call recv to receive message size error1.");

		close(sockfd);
		return NULL;
	}

	recvLen = 0;
	cout<<msgLen<<endl;
	memset(buff, 0, sizeof(buff));
	while(recvLen<msgLen)
	{
		iRet = recv(sockfd, buff + recvLen, msgLen, 0);
		if ( iRet > 0 )
		{
			recvLen += iRet;

		}
		else
		{

			cout<<"threadFunc call recv to receive message error2";
			close(sockfd);
			return NULL;
		}
	}
	cout<<"received ACK: "<<buff<<endl;
	return buff;

}

