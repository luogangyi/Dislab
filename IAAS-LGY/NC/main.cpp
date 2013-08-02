#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "Process.h"
#include "../common/Log.h"
#include "../common/CppMysql.h"
#include <cstring>
#include "libvirtEventHandle.h"
#include "NCHeartBeat.h"

using namespace std;
Log log;


String dbServerIP;
String dbUserName;
String dbUserPass;
string bridge;

string localip;
int heartbeatinterval = 20;


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
	Process process;
	
	char* pos = NULL;
	pos = strrchr(argv[0],'/');
	
	
	if( pos != NULL )
	{
		pos++;
		log.Init(pos);	
	}
	else 
		log.Init(argv[0]);

	
	
	if(process.Init() == false)
	{
		cout<<"Init Process error"<<endl;
		return 0;
	}
	
	if( openDB() == false )
		return 1;

	//////
	////
	LibVirtEventHandle handle;
	if( handle.init() == false )
	{
		cout<<"Init LibVirtEventHandle error"<<endl;
		return 0;
	}
	if( handle.begin() == false )
		return 0;

	/////////
	NCHeartBeat nh;
	if( nh.init() == false)
		return 0;
	
	if( nh.begin() == false)
		return 0;
	
	process.Do();

	return 0;
}
