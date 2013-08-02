/*
 * RealTimeInfo.cpp
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#include "RealTimeInfo.h"
#include "infostruct.h"
#include "String.h"
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
#include "Log.h"
#include "Config.h"
#define MAX_DOMAIN_NUM 24
using namespace std;
extern Log log;

RealTimeInfo::RealTimeInfo()
{
//Date:		2012-11-05
//Author:	WuHao
//Description:	Add the initialization of connection to the hypervisor
/************************************************/
	conn=virConnectOpenReadOnly("");
	if(NULL==conn)
	{
		log.write("RealTimeInfo::RealTimeInfo() failed to connect to hypervisor!",Log::ERROR);
		return;
	}
/************************************************/

	iniMemtotal();
	bool bRet1 = ConfigFile::getStrValue( "NetCard", netcard);
	if(bRet1==false)
	{
		log.write("read NetCard error", Log::ERROR);
		return;
	}
}
float RealTimeInfo::getUptime()//获取uptime命令对第一个值，返回-1出错
{
	char buffer[64];
	int rs;
	float load=-1;
	rs=system("uptime>uptime_result.txt");
	FILE *f1=fopen("uptime_result.txt","r");
	if(f1==NULL)
	{
		log.write("open /proc/stat failed !",Log::ERROR);
		return -1;
	}
	fgets(buffer,64,f1);
	load=uptime_anylyse(buffer);
	fclose(f1);
	//cout<<"load="<<load<<endl;
	return load;
	//cout<<buffer<<endl;
}
bool RealTimeInfo::iniMemtotal()
{
//Date:		2012-11-04
//Author:	wuhao
//Discription:	Change the way to get memtoal
/*	system("xentop -i 1 -b|grep Mem |awk '{print $2}' >memtotal.txt");
	FILE *f1=fopen("memtotal.txt","r");
	if(f1==NULL)
	{
		log.write("open /proc/stat failed !",Log::ERROR);
		return false;
	}
	char buff[32];
	fscanf(f1,"%s",buff);
	int total=0;
	for(int i=0;i<32;i++)
	{
		if(buff[i]=='k'||buff[i]=='K')
		{
			break;
		}
		total=total*10+buff[i]-'0';
	}
	fclose(f1);
	if(total>0)
	{
		memtotal=total;
		return true;
	}
	else
		return false;*/
	
	system("dmidecode -t memory|grep Size |awk '{print $2}' >memtotal.txt");
	FILE *fpMemtotal=fopen("memtotal.txt","r");
	if(fpMemtotal==NULL)
	{
		log.write("RealTimeInfo::iniMemtotal() Failed to get memtotal!",Log::ERROR);
		return false;
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
		memtotal=1024*iTempTotal;
		return true;
	}
	else
		return false;
}

float RealTimeInfo::uptime_anylyse(char *s)//解析uptime的返回值
{
	int rs=0;
	float load;
	int decimalcount=0;
	bool decimalbegin=false;
	int i=0;
	bool isfind=false;
	for(i=0;i<strlen(s);i++)
	{
		if(s[i]=='a' &&s[i+1]=='g' &&s[i+2]=='e')
		{

			isfind=true;
			break;
		}

	}
	if(isfind==false)
		return 0;
	i=i+4;
	//cout<<"begin anylyser ";
	for(;s[i]!=',';i++)
	{
		if(s[i]<='9'&&s[i]>='0')
		{
		//	cout<<s[i]<<" ";
			rs=(rs*10+s[i]-'0');
			if(decimalbegin==true)
			{
				decimalcount++;
			}
		}
		else if(s[i]=='.')
		{
			decimalbegin=true;
		}

	}
	//cout<<decimalcount<<" "<<endl;
	//cout<<rs<<endl;
	load=rs;
	for(int j=0;j<decimalcount;j++)
	{
		load=load/10.0;
	}
	//cout<<load<<endl;
	return load;
}
cputime RealTimeInfo::getCPUtime()//获取cpu使用时间,返回-1失败
{
	cputime t;
	t.used=-1;
	t.total=-1;
	long long  u1,u2,u3,u4;
	char c[10];
	FILE *f1=fopen("/proc/stat","r");
	if(f1==NULL)
	{
		log.write("open /proc/stat failed !",Log::ERROR);
		return t;
	}
	fscanf(f1,"%s\t%lld\t%lld\t%lld\t%lld",c,&u1,&u2,&u3,&u4);
//	printf("%s %d %d %d %d \n",c,u1,u2,u3,u4);
	t.used=u1+u2+u3;
	t.total=u1+u2+u3+u4;
	//printf("%lld\n",t.total);
	fclose(f1);
	return t;
}

ram RealTimeInfo::getRam()
{
//Date:		2012-11-05
//Author:	WuHao
//Description:	change to use virDomainGetInfo() to get ram
/*		ram t;
		t.used=-1;
		t.total=-1;
		int ramfree=0;
	//	int cached;
		int u1;
		char c1[10],c2[10];
		FILE *f1=fopen("/proc/meminfo","r");
		if(f1==NULL)
		{
			log.write("open /proc/meminfo failed !",Log::ERROR);
			return t;
		}


		fscanf(f1,"%s\t%d\t%s\n",c1,&u1,c2);//取total
		t.total=u1;
		fscanf(f1,"%s\t%d\t%s\n",c1,&u1,c2);//取free
		ramfree+=u1;
		fscanf(f1,"%s\t%d\t%s\n",c1,&u1,c2);//取buffer
		ramfree+=u1;
		fscanf(f1,"%s\t%d\t%s\n",c1,&u1,c2);//取cache
		ramfree+=u1;

	//	printf("%s\t%d\t%s\n%s\t%d\t%s",c1,u1,c2,c3,u2,c4);
		t.used=t.total-ramfree;
		fclose(f1);

		int memtemp;
		system("xm list | grep -v Domain-0 | sed '1d' | awk '{print $3}' >domainmem.txt");
		FILE *f2=fopen("domainmem.txt","r");
		if(f2==NULL)
		{
			cout<<"open domainmem.txt failed !";
			return t;
		}
		int totalused=0;
		while(!feof(f2))
		{
			memtemp=0;
			fscanf(f2,"%d\n",&memtemp);
			//cout<<memtemp<<endl;
			totalused+=memtemp;
		}
		fclose(f2);
		//cout<<"t.used before"<<t.used<<endl;
		t.used+=totalused*1024;//domain-0的已使用内存＋分配给虚拟机的内存(换算）
		t.total=memtotal;//替换为总内存

		return t;*/

	ram currentRam;
	currentRam.used=-1;
	currentRam.total=-1;

        int array_iDomains[MAX_DOMAIN_NUM];
        int iDomain=virConnectListDomains(conn,array_iDomains,MAX_DOMAIN_NUM);
        if(-1==iDomain)
        {
                log.write("RealTimeInfo::getRam() Fail to get the Id of active Domain!",Log::ERROR);
                return currentRam;
        }

        long int iTempUsedRam=0;
        virDomainInfo aDomainInfo;
	int i=0;
	if(strcmp("Xen",virConnectGetType(conn))==0)
		i=1;
        for(;i<iDomain;i++)
        {
                virDomainPtr aDomainPtr=virDomainLookupByID(conn,array_iDomains[i]);
                if(NULL==aDomainPtr)
                {
                       log.write("RealTimeInfo::getRam() Fail to get the pointer of Domain!",Log::ERROR);
                        return currentRam;
                }
                int iRet=virDomainGetInfo(aDomainPtr,&aDomainInfo);
                if(iRet<0)
                {
                        log.write("RealTimeInfo::getRam() Fail to get the memory of Domain!",Log::ERROR);
                        return currentRam;
		}
                iTempUsedRam += aDomainInfo.memory;

		virDomainFree(aDomainPtr);
        }

	currentRam.used=iTempUsedRam;
	currentRam.total=memtotal;

        return currentRam;
}

int RealTimeInfo::getCPU_Used_Count()//获取已使用cpu个数
{
//Date:         2012-11-05
//Author:       WuHao
//Description:  change to use virDomainGetInfo() to get the num of CPU
/*	system("xm list | grep -v Domain-0 | sed '1d' | awk '{print $4}' >cpucount.txt");
	FILE *f2=fopen("cpucount.txt","r");
	if(f2==NULL)
	{
		cout<<"open cpucount.txt failed !";
		return 0;
	}
	int cpu_used=0;
	int count_temp=0;
	while(!feof(f2))
	{
		count_temp=0;
		fscanf(f2,"%d\n",&count_temp);
		//cout<<memtemp<<endl;
		cpu_used+=count_temp;
	}
	fclose(f2);
	return cpu_used;*/

	int iUsedCPU=0;

        int array_iDomains[MAX_DOMAIN_NUM];
        int iDomain=virConnectListDomains(conn,array_iDomains,MAX_DOMAIN_NUM);
        if(-1==iDomain)
        {
                log.write("RealTimeInfo::getCPU_Used_Count() Fail to get the Id of active Domain!",Log::ERROR);
                return iUsedCPU;
        }

        int iTempUsedCPU=0;
        virDomainInfo aDomainInfo={'0',0,0,0,0};
	int i=0;
	if(strcmp("Xen",virConnectGetType(conn))==0)
		i=1;
        for(;i<iDomain;i++)
        {
                virDomainPtr aDomainPtr=virDomainLookupByID(conn,array_iDomains[i]);
                if(NULL==aDomainPtr)
                {
                       log.write("RealTimeInfo::getCPU_Used_Count() Fail to get the pointer of Domain!",Log::ERROR);
			return iUsedCPU;
                }
                
		int iRet=virDomainGetInfo(aDomainPtr,&aDomainInfo);
                if(iRet<0)
                {
                        log.write("RealTimeInfo::getCPU_Used_Count() Fail to get the num of CPUs!",Log::ERROR);
                        return iUsedCPU;
                }
                iTempUsedCPU += aDomainInfo.nrVirtCpu;

		virDomainFree(aDomainPtr);
        }

        iUsedCPU=iTempUsedCPU;

        return iUsedCPU;
}
netio RealTimeInfo::getNetio()
{
	netio ni;
	ni.receives=-1;
	ni.sends=-1;

	long long re=0,se=0;
	int t;
	String cmd="cat /proc/net/dev|grep ' "+netcard+":'|awk -F':' '{print $2}' >netinfo.txt";
	//system(cmd.c_str());
	FILE *f1=fopen("netinfo.txt","r");
	if(f1==NULL)
	{
		log.write("open netinfo.txt failed !",Log::ERROR);
		return ni;
	}

	fscanf(f1,"\t%lld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%lld",&re,&t,&t,&t,&t,&t,&t,&t,&se);
	//printf("receive=%d\tsend=%d\n",re/8192,se/8192);//换算成KB

	ni.receives=re/8192;//换算成KB
	ni.sends=se/8192;

	fclose(f1);
	return ni;
}



