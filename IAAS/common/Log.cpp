#include "Log.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h> 


using namespace std;

const int logFileSize = 10*1024*1024;

Log::Log()
{
	
}

Log::~Log()
{
	
}

bool Log::Init(char* fileName)
{
	logFileName = fileName ;
	logFileName += ".log";
	programName = fileName;
	
}

void Log::write( String content, int type)
{
	static int lineCnt = 0;
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

	String cmd =  buff;
	cmd = "echo '" + cmd;
	cmd += "' >> " + logFileName;	

	system(cmd.c_str());
	
	lineCnt++;

	if( lineCnt >= 100 )
	{
		struct stat info;  
		if( stat(logFileName.c_str(), &info) == 0 )
		{
			int size = info.st_size;  
			
			if( size >= logFileSize )
			{
				memset(tmp, 0, sizeof(tmp));
				String name = programName + "_%Y%m%d_%H%M%S.log";
				
				
				
				strftime( tmp, sizeof(tmp), name.c_str(), localtime(&t) );
				rename(logFileName.c_str(), tmp);
			}
		}
		lineCnt = 0;
	}
	
}

//void Log::write( string content, int type)
//{
//	String c = content;
	
	//write(c, type);
	
//}






