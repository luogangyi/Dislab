#include "Object.h"
#include <sys/socket.h>
#include <sys/types.h>

#include <libvirt/libvirt.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
using namespace std;

extern Log log;
extern String dbServerIP;
extern String dbUserName;
extern String dbUserPass;
extern CppMySQL3DB db;

Object::Object()
{
	strCmd = "";
	ackCode = "";	
}

Object::~Object()
{
	//close(sock);
}

void Object::Do()
{
	cout<<"in Object::Do"<<endl;
	
}


void Object::setPara(String para)
{
	strCmd = para;
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


	char lenBuff[10];
	memset(lenBuff, 0, sizeof(lenBuff));
	sprintf(lenBuff, "%04d", msgLen);
	int len = strlen(lenBuff);

	if( send(sock,lenBuff, len, 0) != len)
	{

		log.write("Process::sendAck send ack message size error.", Log::ERROR);
		
		return false;

	}
	//cout<<"lenBuff:"<<lenBuff<<endl;

	/*if( send(sock, &msgLen, sizeof(msgLen), 0) != sizeof(msgLen))
	{

		log.write("Object::sendAck() send ack message size error.", Log::ERROR);
		
		return false;

	}*/

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
		str1.toLower();
		paraMap.insert(pair<String, String>(str1, str2));
	}	
	
}

String Object::getAck()
{
	return ackCode;
}

bool Object::sendData(const char * data, int len, int sock)
{
	String logContent;
		

	if( send(sock, &len, sizeof(len), 0) != sizeof(len))
	{

		log.write("Object::sendData send message size error.", Log::ERROR);
		return false;

	}

	int iSend = 0;

	iSend = send(sock, data, len, 0);

	if( iSend < 0 )
	{
		log.write("Object::sendData call send error.", Log::ERROR);
		return false;
	}
	int iRet = 0;
	while( iSend < len )
	{
		iRet = send(sock, &data[iSend], len - iSend, 0);
		if ( iRet > 0 )
			iSend + iRet;
		else
		{
			log.write("Object::sendData call send error in while.", Log::ERROR);
			return false;
		}
	}
	
	return true;
}

bool Object::recvData(int sock, String& data)
{
	int msgLen = 0;
	String str;
	int recvLen = 0;
	int iRet = 0;
	char buff[BUFF_SIZE];
	int iRecv = 0;
	String ackMsg = "";
	recvLen = 0;
	iRecv = recv(sock, &msgLen, sizeof(msgLen), 0);

	if( iRecv <= 0 )
	{
		log.write("Object::recvData receive ack message size error.", Log::ERROR);		
		return false;
	}
	if( iRecv != sizeof(msgLen) )
	{
		str = strerror(errno);
		log.write("Object::recvData receive ack message size error.", Log::ERROR);		
		return false;
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
			log.write("Object::recvData receive ack message error:" + str + ".", Log::ERROR);
			
			return false;
		}
	}
	data = buff;
	return true;
}

int Object::connect2Server(const char* ip, int port)
{
	int sock;
	if( ( sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		log.write("Object::connect2Server() create socket error:", Log::ERROR);
		return -1;
	}
   
	
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;

		
	inet_aton(ip, &(serveraddr.sin_addr));
	serveraddr.sin_port=htons(port);

	int iRet = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	if ( iRet != 0 )
	{
		perror("Object::connect2Server() connect to server error");
		close(sock);
		return -1;
	}
	return sock;
}


bool Object::getStrParaValue(String cmdAck, String item, String& sValue)
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
			str1.toLower();
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
		str1.toLower();
		String str2 = strtok (NULL,split);
		if( str1 == item )
		{
			sValue = str2;		
			return true;
		}
	}	
	return false;
}

bool Object::getIntParaValue(String cmdAck, String item, int& iValue)
{
	String sValue;
	bool bRet = getStrParaValue( cmdAck, item, sValue);
	if( bRet == false )
		return false;
	iValue = atoi( sValue.c_str() );
	return true;
}

void Object::closeSock(int sock)
{
	close( sock );
}

int Object::executeDBSQL(String sql)
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
		logContent = "execute: " + sql + ".";
		log.write( logContent, Log::INFO);
		iRet = db.execSQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		return -1;
	}	
	return iRet;
}

bool Object::executeDBQuery( String sql, CppMySQLQuery & result)
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
		logContent = "execute: " + sql + ".";
		log.write( logContent, Log::INFO);
		result = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		return false;
	}	
	return true;
}

String Object::getCurrentDir()
{
	

	String str = "pwd";
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	FILE* fp = popen(str.c_str(),"r");

	fgets(buff, sizeof(buff), fp);
	
	pclose(fp);
	buff[strlen(buff)] = '\0';
	buff[strlen(buff) - 1] = '\0';
	return buff;
}

bool Object::isFileExist( String file)
{
	fstream _file;
    _file.open(file.c_str(),ios::in);
    if(!_file)
    {
    	return false;
    }
    else
    {
    	_file.close();
		return true;
    }
	
}

void Object::setAuthData(String data)
{
	authData = data;
}

bool Object::checkRight( String id )
{
	//cout<<"authData:"<<authData<<endl;
	if( authData == "-1|-1" )
		return true;
	return authData.hasStr( id );
}


