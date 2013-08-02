
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
using namespace std;
class CMDreceiver
{
protected:
	int sockfd;
	int port;
	sockaddr_in server_addr;
	static void* infoProcess(int new_fd);


public:

	CMDreceiver(int portno);
	void receiver();

};
