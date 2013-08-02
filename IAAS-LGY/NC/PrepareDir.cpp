#include "PrepareDir.h"
#include <fstream>


extern Log log;
PrepareDir::PrepareDir( string cmd)
{
	strCmd = cmd;
	nfsip = "";
	nfsDir = "";
}

PrepareDir::~PrepareDir()
{

}

void PrepareDir::Do()
{
	log.write("begin PrepareDir::Do.", Log::INFO);

	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:PREPARE DIR:RETN=4,DESC=参数错误";
		return;
	}

	if( mountNFS() == 1 )
	{
		ackCode =  "ACK:PREPARE DIR:RETN=8,DESC=系统内部错误";
		return;
	}
	
	ackCode =  "ACK:PREPARE DIR:RETN=0,DESC=成功";
	
	log.write("end PrepareDir::Do.", Log::INFO);
	
}

bool PrepareDir::getPara()
{
	processPara();


	if( paraMap.size() != 2 )
		return false;
	
	map<string,string>::iterator it;

	//获取NFS服务器IP地址信息
	it = paraMap.find("ip");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	nfsip = it->second;

	if( nfsip.size() == 0 )
		return false;


	//获取NFS上虚拟机所在目录
	it = paraMap.find("dir");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	nfsDir = it->second;
	if( nfsDir.size() == 0 )
		return false;
	
		
	return true;
}


bool PrepareDir::isFileExist( string file)
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

//挂载NFS服务器上的目录
//返回值：0-成功1-失败
int PrepareDir::mountNFS()
{

	////////////
	//对传入的参数dir进行处理，将'/'替换成'-'
	char dirName[1024];
	memset(dirName, 0, sizeof(dirName));
	strcpy(dirName, nfsDir.c_str());
	for(int i=0; i<strlen(dirName); i++)
	{
		if( dirName[i] == '/' )
			dirName[i] = '-';
	}
	string temp = dirName + 1;
	
	/////////	

	string dir = getCurrentDir() + "/nfs/" + nfsip + "/" + temp;
	
	
	string mountPoint = "mountpoint " + dir;
	bool bMounted = false;
	int iRet = system( mountPoint.c_str());
	if( iRet == 0 )
		return 0;	
	
	
	
	string logContent = "vm directory " + dir + " do not exist,first try to make it.";
	
	log.write(logContent, Log::INFO);
	if( !isFileExist(dir) )
	{		
		string mkdir = "mkdir -p " + dir;
		system(mkdir.c_str());
		if( !isFileExist(dir) )
		{
			logContent = "create dir " + dir + " error.";
			log.write(logContent.c_str(), Log::ERROR);
			return false;
		}
	}
	
	string mountSrc = nfsip + ":" + nfsDir;
	logContent = "begin to mount " + mountSrc + " to " + dir;
	log.write(logContent.c_str(), Log::INFO);
	if( mount(mountSrc, dir) == false )
	{
		logContent = "mount " + mountSrc + " to " + dir + " failed.";
		log.write(logContent.c_str(), Log::ERROR);
		
		return 1;
	}
	logContent = "mount " + mountSrc + " to " + dir + " success.";
	log.write(logContent.c_str(), Log::INFO);
	return 0;
}

bool PrepareDir::mount( string src, string dest)
{	
	string cmd = "mount " + src + " " + dest;
	
	int iRet = system( cmd.c_str() );

	return iRet == 0;
}

string PrepareDir::getCurrentDir()
{
	

	char cwd[1024];
	memset(cwd, 0, sizeof(cwd));
	getcwd(cwd, sizeof(cwd));

	string pwd(cwd);
	return pwd;
}




