#include "Object.h"
#include <sys/socket.h>
#include <sys/types.h>

#include <libvirt/libvirt.h>


using namespace std;

extern Log log;
Object::Object()
{
	strCmd = "";
	ackCode = "";
	sVMID = "";
}

Object::~Object()
{
	//close(sock);
}

void Object::Do()
{
	cout<<"in Object::Do"<<endl;
	
}


void Object::setSock(int s)
{
	sock = s;
}

bool Object::sendAck()
{
	String logContent;
	if( ackCode == "" )
		return false;
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	
	int msgLen = ackCode.size();


	strncpy(buff, ackCode.c_str(), msgLen);
	

	if( send(sock, &msgLen, sizeof(msgLen), 0) != sizeof(msgLen))
	{

		log.write("Object::sendAck() send ack message size error.", Log::ERROR);
		
		return false;

	}

	int iSend = 0;

	iSend = send(sock, buff, msgLen, 0);

	if( iSend < 0 )
	{
		log.write("Object::sendAck() call send error.", Log::ERROR);
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
			log.write("Object::sendAck() call send error in while.", Log::ERROR);
			return false;
		}
	}

	log.write( "send ack message:" + ackCode, Log::INFO);
	return true;
}



void Object::processPara()
{
	char buff[960];
	char para[100];
	const char * split = "="; 
	memset(buff, 0, sizeof(buff));
	
	strcpy(buff, strCmd.c_str());	
	
	char* p = buff;

	
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
			
			String str1 = strtok (para,split);
			String str2 = strtok (NULL,split);
			
			paraMap.insert(pair<String, String>(str1, str2));
			
		}
	}

	//此处存在一个漏洞，会使程序core，要修改
	//用NULL初始化string会core
	char * tmp = strtok (p,split);
	if( tmp )
	{
		String str1 = tmp;
		String str2 = strtok (NULL,split);
		paraMap.insert(pair<String, String>(str1, str2));
	}	
	
}




