#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "../../common/Config.h"
#include <stdlib.h>
#include "../../common/CppMysql.h"
#include "../../common/Log.h"
#include "AutoScaling.h"

using namespace std;


Log log;
String dbServerIP;
String dbUserName;
String dbUserPass;
int clcServerPort;
String clcServerAddr;
String admin,passwd;
int ncPort;

CppMySQL3DB db;


bool init()
{
	//读取数据库配置信息
	bool bRet = ConfigFile::getStrValue( "DBServerAddr", dbServerIP );
	
	bool bRet2 = ConfigFile::getStrValue( "DBUserName", dbUserName );
	bool bRet3 = ConfigFile::getStrValue( "DBUserPass", dbUserPass );


	if( !bRet || !bRet2 || !bRet3 )
	{
		log.write("read db config error", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getIntValue( "CLCVMServerPort", clcServerPort );
	if( !bRet )
	{
		log.write("read CLCVMServerPort config error", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue( "CLCVMServerIP", clcServerAddr );
	if( !bRet )
	{
		log.write("read CLCVMServerIP config error", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue( "ADMIN", admin );
	if( !bRet )
	{
		log.write("read ADMIN config error", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue( "PASSWD", passwd);
	if( !bRet )
	{
		log.write("read PASSWD config error", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getIntValue( "NCVMServerPort", ncPort);
	if( !bRet )
	{
		log.write("read NCVMServerPort config error", Log::ERROR);
		return false;
	}
	
	return true;
}


bool openDB()
{
	
	int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	
	if ( iRet != 0 )
	{
		String logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return false;
	}	
	return true;
}


int main(int argc, char* argv[])
{
	//处理SIGPIPE信号，
	//不然如果对方套接字关闭会导致程序退出
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sa, 0 );

	

	char* pos = NULL;
	pos = strrchr(argv[0],'/');
	
	
	if( pos != NULL )
	{
		pos++;
		log.Init(pos);	
	}
	else 
		log.Init(argv[0]);	
	
	
	ConfigFile::strFileName = "clccfg.cfg";
	if( init() == false )
		return 1;
	

	if( openDB() == false )
		return 1;

	AutoScaling scal;
	while(1)
	{
		scal.Do();
		sleep(15);
	}

	db.close();
	
	
	return 0;
}
