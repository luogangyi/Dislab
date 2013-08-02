#include "MigrateVM.h"

extern Log log;


MigrateVM::MigrateVM(string cmd)
{
	strCmd = cmd;
	toip = "";
	vmName = "";
}

MigrateVM::~MigrateVM()
{
	
}


void MigrateVM::Do()
{
	log.write("begin MigrateVM::Do.", Log::INFO);

	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:MIGRATE VM:RETN=4,DESC=参数错误";
		return;
	}

	int iRet = isAlreadyStart();
	if(  iRet == 0 )
	{
		ackCode =  "ACK:MIGRATE VM:RETN=1001,DESC=虚拟机未启动";
		return;
	}	
	else if( iRet == 2 )
	{
		ackCode =  "ACK:MIGRATE VM:RETN=8,DESC=系统内部错误";
		return;
	}

	if( migrateVM2IP() == false )
	{
		ackCode =  "ACK:MIGRATE VM:RETN=1002,DESC=虚拟机迁移失败";
		return;
	}

	ackCode =  "ACK:MIGRATE VM:RETN=0,DESC=成功";
	log.write("end MigrateVM::Do.", Log::INFO);
}


bool MigrateVM::migrateVM2IP()
{
	string cmd = "xm migrate --live " + vmName + " " + toip;
	cout<<cmd<<endl;
	int iRet = system(cmd.c_str());
	return iRet == 0;
}


bool MigrateVM::getPara()
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
	toip = it->second;

	if( toip.size() == 0 )
		return false;


	//获取NFS上虚拟机所在目录
	it = paraMap.find("vmname");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	vmName = it->second;
	if( vmName.size() == 0 )
		return false;
	
		
	return true;
}


