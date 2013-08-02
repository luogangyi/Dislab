#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
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
#include"CMDreceiver.h"
#include"CMDType.h"
#define BACKLOG 10
using namespace std;


CMDreceiver::CMDreceiver(int portno)
{
 port=portno;
}
void CMDreceiver::receiver()//监听端口
{
	sockfd=socket(AF_INET,SOCK_STREAM,0);//初始化socket
	if(sockfd<0)
	{
		cout<<"error in socket!"<<endl;
		return;
	}

	int flag=1,len=sizeof(int); 
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
	
	int ret;
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(port);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	bzero(&(server_addr.sin_zero),8);
	ret=bind(sockfd,(sockaddr*)&server_addr,sizeof(sockaddr));//绑定端口
	if(ret<0)
	{
		cout<<"error in binding"<<endl;
		return ;
	}
	cout<<"success in binding \n";

	int new_fd;
	sockaddr_in client_addr;

	ret=listen(sockfd,BACKLOG);
	if(ret<0)
	{
		cout<<"error in listening\n";
		return;
	}
	socklen_t sin_size=sizeof(sockaddr_in);

	while(1)//开始监听
	{
		new_fd=accept(sockfd,(sockaddr*)&client_addr,&sin_size);
		if(new_fd<0)
		{
			cout<<"error in accept\n";
			return;
		}
		//pthread_t t_id;

		//用线程处理请求
		infoProcess(new_fd);
	}
	close(sockfd);

}

void* CMDreceiver::infoProcess(int new_fd)
{
	int client_sockfd=new_fd;
    int msgLen = 0;
			int recvLen = 0;
			int iRet = 0;
			char *buff=new char[256];
			int iRecv = 0;

			iRecv = recv(client_sockfd, &msgLen, sizeof(msgLen), 0);

			if( iRecv <= 0 )
			{
				cout<<"client close the connection."<<endl;
				close(client_sockfd);
				return NULL;
			}
			if( iRecv != sizeof(msgLen) )
			{

				printf("in threadFunc,call recv to receive message size error1.");

				close(client_sockfd);
				return NULL;
			}

			recvLen = 0;
		//	cout<<msgLen<<endl;
			memset(buff, 0, sizeof(buff));
			while(recvLen<msgLen)
			{
				iRet = recv(client_sockfd, buff + recvLen, msgLen, 0);
				if ( iRet > 0 )
				{
					recvLen += iRet;
				//	cout<<"recelen "<<recvLen<<" data="<<buff<<endl;
				}
				else
				{

					cout<<"threadFunc call recv to receive message error2";
					close(client_sockfd);
					return NULL;
				}
			}
			cout<<"received cmd:"<<buff<<endl;
			 CMDType a;
			String msg=a.processack(buff,a.processcmd(buff));
			 cout<<"send ack:"<<msg.c_str()<<endl;
		    	    int msglen;
					msglen=strlen(msg.c_str());
				//	cout<<msglen<<endl;
					send(client_sockfd,&msglen,sizeof(msglen),0);
					send(client_sockfd,msg.c_str(),msglen,0);
                     close(client_sockfd);


}


