#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include<signal.h>

#include <stdlib.h>

#include "Process.h"
#include "../../common/Log.h"

using namespace std;
Log log;

String dbServerIP;
String dbUserName;
String dbUserPass;
String scServerIP;
int scServerPort;

//调度中收地址
String g_dispatchServerIP;
int g_dispatchServerPort;

int g_ncVMServerPort;
String g_gwAddr;

CppMySQL3DB db;


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
	
	Process process;

	if(process.Init() == false)
	{
		log.write("Init Process error", Log::ERROR);
		return 1;
	}

	

	if( openDB() == false )
		return 1;
	
	process.Do();

	return 0;
}
