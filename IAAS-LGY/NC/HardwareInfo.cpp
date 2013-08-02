/*
 * HardwareInfo.cpp
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#include "HardwareInfo.h"
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
#include<ctime>

#include "../common/Log.h"
#include "../common/Config.h"


using namespace std;
extern Log log;

int HardwareInfo::getCPUcount()
{
	char buffer[64];
	int count=0;
	FILE *f1=fopen("/proc/cpuinfo","r");
	if(f1==NULL)
	{
		log.write("open /proc/stat failed !",Log::ERROR);
		return -1;
	}
	while(!feof(f1))
	{
		fgets(buffer,64,f1);
		if(strstr(buffer,"processor"))
			count++;
	}

	fclose(f1);
	
	return count;
}
int HardwareInfo::getRamSize()
{
	
        system("dmidecode -t memory|grep Size |awk '{print $2}' >memtotal.txt");
        FILE *fpMemtotal=fopen("memtotal.txt","r");
        if(fpMemtotal==NULL)
        {
                log.write("HardwareInfo::getRamSize() Failed to get memtotal!",Log::ERROR);
                return -1;
        }
        char szBuf[32];
        int iTempTotal=0;
        while(1)
        {
                if(feof(fpMemtotal)) break;
                fscanf(fpMemtotal,"%s",szBuf);
                int iTempOneMem=0;
                for(int i=0;i<32 && szBuf[i]<='9' && szBuf[i]>='0';i++)
                {
                        iTempOneMem=iTempOneMem*10+szBuf[i]-'0';
                }
                iTempTotal += iTempOneMem;
        }
        fclose(fpMemtotal);
        if(iTempTotal>0)
        {
                return iTempTotal;
        }
        else
                return -1;
}

//Date:		2013-03-06
//Author:	WuHao
//Description:	add VM type
String HardwareInfo::getVirtType()
{
	virConnectPtr conn=virConnectOpenReadOnly("");
	if(NULL==conn)
	{
		log.write("RealTimeInfo::RealTimeInfo() failed to connect to hypervisor!",Log::ERROR);
		return "";
	}
/************************************************/
	const char *szPtr=virConnectGetType(conn);
	char szType[10];
	strcpy(szType,szPtr);
	String *szVirtType=new String(szType);
	String szOutput=*szVirtType;	
	return szOutput;
}

int HardwareInfo::getDiskSize()
{
	int u1,u2,u3;
	char buffer[64];
	char c[10];
	FILE *f1=fopen("/proc/partitions","r");
	if(f1==NULL)
	{
		log.write("open /proc/partitions failed !",Log::ERROR);
		return -1;
	}
	fgets(buffer,64,f1);
	fgets(buffer,64,f1);
	while(!feof(f1))
	{
		fscanf(f1,"%d\t%d\t%d\t%s\n",&u1,&u2,&u3,c);
		if(c[0]=='s'&&c[1]=='d'&&c[2]=='a'&&c[3]=='\0')
		{			
			break;
		}
	}
	fclose(f1);
	return u3/1024/1024;
}

int HardwareInfo::getNetWidth()
{
	char buffer[64];
	int rs;
	int width=-1;

	String szNetCard;
	bool bRet1 = ConfigFile::getStrValue( "NetCard", szNetCard);
	String cmd="ethtool "+szNetCard+">net_result.txt";
	rs=system(cmd.c_str());

	FILE *f1=fopen("net_result.txt","r");
	if(f1==NULL)
	{
		log.write("open net_result.txt failed !",Log::ERROR);
		return -1;
	}
	while(!feof(f1))
	{
		fgets(buffer,128,f1);
		if(strstr(buffer,"Speed"))
		{
			//cout<<"finded "<<buffer<<endl;
			width=width_analyse(buffer);
			break;
		}
	}
//	cout<<"width= "<<width<<endl;
	fclose(f1);

	return width;

}

int HardwareInfo::width_analyse(char*s)
{
	int i=0;
	int rs=0;
	for(i=0;s[i]!='\0';i++)
	{
		if(s[i]<='9'&&s[i]>='0')
		{
			rs=rs*10+s[i]-'0';
		}
		else if(s[i]=='G')
		{
			rs=rs*1000;
			break;
		}
		else if(s[i]=='M')
		{
			break;
		}
	}
	return rs;
}


