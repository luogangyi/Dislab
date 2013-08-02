#include "ExpandVMMem.h"
#include <fstream>

extern Log log;


ExpandVMMem::ExpandVMMem(string cmd)
{
	strCmd = cmd;	
	expandFlag = 0;
}

ExpandVMMem::~ExpandVMMem()
{
	
}


void ExpandVMMem::Do()
{
	

	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:EXPAND VM:RETN=4,DESC=para error";
		return;
	}

	/*
	int vmmem = getVMMemory();
	if( vmmem == 0 )
	{
		ackCode =  "ACK:EXPAND VM:RETN=1001,DESC=get vm memory info failed";
		return;
	}

	vmmem = (int)(vmmem * 1.2);*/

	int cpuCnt = getVMCPUCnt();

	if( cpuCnt == 0 )
	{
		ackCode =  "ACK:EXPAND VM:RETN=1001,DESC=get vm cpu info failed";
		return;
	}

	if( expandFlag == 0 )
		cpuCnt++;
	else 
		cpuCnt--;
	
	char buff[1024];
	memset(buff,0,sizeof(buff));

	sprintf(buff,"xm vcpu-set %s %d",vmName.c_str(),cpuCnt);
	
	system(buff);

	if( getVMCPUCnt() == cpuCnt )
		ackCode =  "ACK:EXPAND VM:RETN=0,DESC=success";
	else
		ackCode =  "ACK:EXPAND VM:RETN=8,DESC=error";
	
}


int ExpandVMMem::getVMMemory()
{

	string cmd = "xm list | grep ";
	cmd += vmName + " | awk '{print $3}' > vmmem.txt";

	system(cmd.c_str());

	int vmmem;
	ifstream infile;
	infile.open("vmmem.txt", ios::in);
	if( infile.is_open() == false)
	{		
		return 0;
	}
	infile>>vmmem;
	infile.close();
	
	return vmmem;

}


int ExpandVMMem::getVMCPUCnt()
{

	string cmd = "xm list | grep ";
	cmd += vmName + " | awk '{print $4}' > vmcpu.txt";

	system(cmd.c_str());

	int cpuCnt;
	ifstream infile;
	infile.open("vmcpu.txt", ios::in);
	if( infile.is_open() == false)
	{		
		return 0;
	}
	infile>>cpuCnt;
	infile.close();
	
	return cpuCnt;

}



bool ExpandVMMem::getPara()
{
	processPara();
		
	map<string,string>::iterator it;

	it = paraMap.find("vmname");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	vmName = it->second;
	if( vmName.size() == 0 )
		return false;


	//flag=0表示增加CPU，其它值表示减少CPU
	it = paraMap.find("flag");

	if( it != paraMap.end() )
		expandFlag=1;
		
	return true;
}




