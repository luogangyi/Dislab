/*
 * SupervisorVM.cpp
 *
 *  Created on: 2011-11-13
 *      Author: lgy
 */
#include<iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<ctime>
#include<sys/time.h>

#include "VMInfoStruct.h"
#include "../common/Log.h"
#include "../common/String.h"
#include "../common/CppMysql.h"
#include "../common/Config.h"
#include "SupervisorVM.h"
#include<iostream>

#define MAX_DOMAIN_NUM 24

Log log;

bool setRedirect(const char* fileName)
{
FILE *stream ;
   if((stream = freopen(fileName, "w", stdout)) == NULL)
      return false;



 	//stream = freopen("CON", "w", stdout);
	return true;

}

SupervisorVM::SupervisorVM()
{
	head=new VMInfo;
	head->next=NULL;
	vmnameHead=new VMName;
	vmnameHead->next=NULL;

	conn=NULL;
}

SupervisorVM::~SupervisorVM()
{
	VMInfo *temp=head;
	while(head!=NULL)
	{
		temp=head;
		head=head->next;
		delete temp;
	}

	VMName *temp2=vmnameHead;
	while(vmnameHead!=NULL)
	{
		temp2=vmnameHead;
		vmnameHead=vmnameHead->next;
		delete temp2;
	}

	virConnectClose(conn);
}

bool SupervisorVM::ini()
{
	String logServerIP = "";
	int logServerPort = -1;
	bool bRet;
	//¶ÁÈ¡ÊýŸÝ¿âÅäÖÃÐÅÏ¢
	bRet = ConfigFile::getStrValue( "DBServerAddr", databaseServer);
	if(!bRet)
	{
		log.write("read db config error, fail to find DBServerAddr", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue( "DBUserName", username);
	if(!bRet)
	{
		log.write("read db config error, fail to find DBUserName", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue( "DBUserPass", password );
	if(!bRet)
	{
		log.write("read db config error, fail to find DBUserPass", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue( "DBName", dbname);
	if(!bRet)
	{
		log.write("read db config error, fail to find DBName", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getStrValue("LogServerIP", logServerIP);
	if(!bRet)
	{
		log.write("read db config error, fail to find LogServerIP", Log::ERROR);
		return false;
	}

	bRet = ConfigFile::getIntValue("LogServerPort", logServerPort);
	if(!bRet)
	{
		log.write("read db config error, fail to find LogServerPort", Log::ERROR);
		return false;
	}

	bRet=ConfigFile::getStrValue("NCLocalIP",localIP);
	if(!bRet)
	{
		log.write("read db config error, fail to find NCLocalIP", Log::ERROR);
		return false;
	}

	if(openDB()==false)
	{
		return false;
	}
	setRedirect("SupervisorVM.log");

//Date:		2012-11-06
//Author:	WuHao
//Description:	Add the inialization of connection to the hypervisor
/********************************************************/
	conn=virConnectOpen("");
	if(NULL==conn)
	{
		log.write("Supervisor::ini() failed to connect the hypervisor",Log::ERROR);
		return false;
	}
/*********************************************************/

	return true;


}

bool SupervisorVM::openDB()
{
	String logContent;
	int iRet = db.open(databaseServer.c_str(), username.c_str(), password.c_str(), dbname.c_str());
	if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return false;
	}
	return true;
}
bool SupervisorVM::getLocalVM()
{
//Date:		2012-11-06
//Author:	WuHao
//Description:	change the way to update VMInfo list
/*	char buff[256];
	char nametemp[64];
	int a1;//缓存无用数据
	float af;
	char fchar1[64],fchar2[64];//保存char*的long行数据，将转换为long
	system("xentop -i 2 -d 1 -b>vminfo.txt");


	FILE *f1=fopen("vminfo.txt","r");
	if(f1==NULL)
	{
		log.write("open vminfo.txt failed !",Log::ERROR);
		return false;
	}
	//cout<<"xentop -i 2 -d 1 -b content is printing: \n";

	bool flag = false;
	int domain0count=0;
	while(!feof(f1))
	{
		if(domain0count==2)//读到2次NAME开始正式读取数据
		{
			VMInfo *temp=new VMInfo();
			fscanf(f1,"%s",temp->vmname);
			if(strcmp(temp->vmname,"Domain-0")==0)
			{
				fgets(buff,256,f1);//读完该行
				delete temp;
				continue;
			}
			if(strlen(temp->vmname)==0)
			{
				break;
			}
			fscanf(f1,"%s%d%f%d%f%d%f%d%d",temp->vmstate,&a1,&(temp->vmcputimeptg),&a1,&(temp->vmmemptg),&a1,&af,&a1,&a1);
			fscanf(f1,"%s%s%d%d%d%d%d \n",fchar1,fchar2,&a1,&a1,&a1,&a1,&a1);
			temp->netout=atol(fchar1);
			temp->netin=atol(fchar2);
			String *vnameptr=new String(temp->vmname);
			String cmd="xm list|grep "+*vnameptr+"|awk '{print $4}'>vcpuinfo.txt";
			system(cmd.c_str());
			FILE *f3=fopen("vcpuinfo.txt","r");
			if(f1==NULL)
			{
				log.write("open vcpuinfo.txt failed !",Log::ERROR);
				return false;
			}
			fscanf(f3,"%d",&(temp->vmvcpu));
			fclose(f3);
			time(&(temp->reporttimetime));
			//printf("%s\n",temp->vmname);
			//printf("%s %s %d %f %d %f %d %f %d %d\n",temp->vmname,temp->vmstate,a1,(temp->vmcputimeptg),a1,(temp->vmmemptg),a1,af,(temp->vmvcpu),a1);
			//cout<<temp->vmname<<" "<<"and so on"<<endl;
			temp->isFind=false;//初始为false
			//头插法

			//printf("%s %d %d %d %s %f\n",temp->vmname,(temp->vmid),(temp->vmmem),(temp->vmvcpu),temp->vmstate,(temp->vmtime));
			temp->next=head->next;
			head->next=temp;
			delete vnameptr;
		}
		else
		{
			fscanf(f1,"%s",nametemp);
			if(strcmp("NAME",nametemp)==0)
			{
					flag = true;
					domain0count++;
			}
			fgets(buff,256,f1);//读完该行
		}

	}
	if(flag==false)
	{
		cout<<"run xentop  error\n";
	}
	//cout<<"xentop content is finished! \n";
	fclose(f1);
	return flag;*/

	int array_iDomains[MAX_DOMAIN_NUM];
        int iDomain=virConnectListDomains(conn,array_iDomains,MAX_DOMAIN_NUM);
        if(-1==iDomain)
        {
                log.write("SupervisorVM::getLocalVM() failed to get the Id of active Domain!",Log::ERROR);
                return false;
        }

/**********************************************************************/
//			get total size of host memory
/**********************************************************************/
        system("dmidecode -t memory|grep Size |awk '{print $2}' >memtotal.txt");
        FILE *fpMemtotal=fopen("memtotal.txt","r");
        if(fpMemtotal==NULL)
        {
        	log.write("SupervisorVM::getLocalVM() failed to get memtotal!",Log::ERROR);
                return false;
        }
       	char szBuf[32];
        int iTempMemTotal=0;
        while(1)
        {
        	if(feof(fpMemtotal)) break;
                fscanf(fpMemtotal,"%s",szBuf);
		int iTempOneMem=0;
                for(int i=0;i<32 && szBuf[i]<='9' && szBuf[i]>='0';i++)
                {
                	iTempOneMem=iTempOneMem*10+szBuf[i]-'0';
                }
                iTempMemTotal += iTempOneMem;
       	}
        fclose(fpMemtotal);

        virDomainInfo aDomainInfo;
	int i=0;
	if(strcmp("Xen",virConnectGetType(conn))==0)
		i=1;
        for(;i<iDomain;i++)
        {
                virDomainPtr aDomainPtr=virDomainLookupByID(conn,array_iDomains[i]);
                if(NULL==aDomainPtr)
                {
                        log.write("SupervisorVM::getLocalVM() failed to get the pointer of Domain",Log::ERROR);
                        return false;
                }


//Date:		2013-03-25
//Description:	change vmname to uuid
/**********************************************************************
                VMInfo *pTempVMInfo=new VMInfo();
                strcpy(pTempVMInfo->vmname,virDomainGetName(aDomainPtr));
***********************************************************************/
		VMInfo *pTempVMInfo=new VMInfo();
		int iUUIDRet=virDomainGetUUIDString(aDomainPtr,pTempVMInfo->vmname);
		if(iUUIDRet<0)
		{
			log.write("SupervisorVM::getLocalVM() failed to get the uuid of the VM!",Log::ERROR);
                        delete pTempVMInfo;
                        return false;
		}

/**********************************************************************/
//			get the percentage of vCPU usage
/**********************************************************************/
                int iStartRet=virDomainGetInfo(aDomainPtr,&aDomainInfo);
                if(iStartRet<0)
                {
                        log.write("SupervisorVM::getLocalVM() failed to get start domain info!",Log::ERROR);
                        delete pTempVMInfo;
                        return false;
                }
                unsigned long long llStartCpuTime=aDomainInfo.cpuTime;
                struct timeval tempStartTime;
                if(-1==gettimeofday(&tempStartTime,NULL))
                {
                        log.write("SupervisorVM::getLocalVM() failed to get start time!",Log::ERROR);
                        delete pTempVMInfo;
                        return false;
		}

                sleep(2);

                int iEndRet=virDomainGetInfo(aDomainPtr,&aDomainInfo);
                if(iEndRet<0)
                {
                        log.write("SupervisorVM::getLocalVM() failed to get end domain info!",Log::ERROR);
                        delete pTempVMInfo;
                        return false;
                }
                unsigned long long llEndCpuTime=aDomainInfo.cpuTime;
                struct timeval tempEndTime;
                if(-1==gettimeofday(&tempEndTime,NULL))
                {
                        log.write("SupervisorVM::getLocalVM() failed to get end time!",Log::ERROR);
                        delete pTempVMInfo;
                        return false;
                }

                int iCpuTime=(llEndCpuTime-llStartCpuTime)/1000;
                int iRealTime=1000000*(tempEndTime.tv_sec-tempStartTime.tv_sec)+(tempEndTime.tv_usec-tempStartTime.tv_usec);

                pTempVMInfo->vmcputimeptg=100*iCpuTime/(float)iRealTime;

		pTempVMInfo->reporttimetime=tempEndTime.tv_sec;


//Date:		2013-03-25
//Description:	change vmname to uuid
/**********************************************************************
                unsigned long int uliMemTotal=1024*iTempMemTotal;
                unsigned long int uliMemUsed=aDomainInfo.memory;
                pTempVMInfo->vmmemptg=100*uliMemUsed/(float)uliMemTotal;
***********************************************************************/
		unsigned long int uliMemUsed=aDomainInfo.memory;
		pTempVMInfo->vmmemptg=uliMemUsed/1024;


/**********************************************************************/
//			get the state of vCPU
/**********************************************************************/
                pTempVMInfo->vmvcpu=aDomainInfo.nrVirtCpu;
                unsigned char ucDomainState=aDomainInfo.state;
                switch(ucDomainState)
                {
                        case 1:
                                strcpy(pTempVMInfo->vmstate,"-----r");
                                break;
                        case 2:
                                strcpy(pTempVMInfo->vmstate,"--b---");
                                break;
                        default:
                                strcpy(pTempVMInfo->vmstate,"------");
                }
                pTempVMInfo->isFind=false;


/**********************************************************************/
//			get the I/O of Network
/**********************************************************************/
                string szNetPath="";
		char *szDomainXML=virDomainGetXMLDesc(aDomainPtr,1);
		bool isFound=false;
		if(szDomainXML!=NULL)
		{
		for(int iOffset=0;szDomainXML[iOffset]!='\0' && !isFound;iOffset++)
		{
			string szLine="";

			while(szDomainXML[iOffset]!='\n' && szDomainXML[iOffset]!='\0')
			{
				szLine+=szDomainXML[iOffset];
				iOffset++;
			}
		
			if(strcmp(szLine.c_str(),"    <interface type='network'>")==0 || strcmp(szLine.c_str(),"    <interface type='bridge'>")==0)
			{
				iOffset++;
				while(szDomainXML[iOffset]!='\0' && !isFound)
				{
					szLine="";
					int j=iOffset;

					while(szDomainXML[iOffset]!='\n' && szDomainXML[iOffset]!='\0')
					{
					szLine+=szDomainXML[iOffset];
					iOffset++;
					}
				
					if(strncmp(szLine.c_str(),"      <target dev",15)==0)
					{
						for(j+=19;szDomainXML[j]!='\'';j++)
						{
							szNetPath +=szDomainXML[j];
							isFound=true;
//							String aLog="net card:";
//							aLog += szNetPath;
//							log.write(aLog, Log::ERROR);
						}
					}
					iOffset++;
				}
			}
			if(szDomainXML[iOffset]=='\0')
				iOffset--;
		}
		}
		free(szDomainXML);

		if(isFound)
		{
                virDomainInterfaceStatsStruct tempNetStats;
		int iNetRet=virDomainInterfaceStats(aDomainPtr,szNetPath.c_str(),&tempNetStats,sizeof(tempNetStats));
                if(iNetRet<0)
                {
                        pTempVMInfo->netout=0;
                        pTempVMInfo->netin=0;
                }
                else
                {
                        pTempVMInfo->netout=tempNetStats.tx_bytes/1024;
                        pTempVMInfo->netin=tempNetStats.rx_bytes/1024;
                }
		}
		else
		{
			pTempVMInfo->netout=0;
			pTempVMInfo->netin=0;
		}

		pTempVMInfo->next=head->next;
		head->next=pTempVMInfo;

		virDomainFree(aDomainPtr);
        }

        return true;
}

/*********************************************************************
bool SupervisorVM::getServerVM()
{

	String logContent;
	CppMySQLQuery query1;
	String sql1="select VMNAME from OnlineVMS where NCIP='"+localIP+"'";
	try
	{

		query1 = db.querySQL( sql1.c_str() );
	}
	catch(const char* sErr1)
	{
		logContent = "execute " + sql1 + " failed:" + sErr1;
		log.write( logContent, Log::ERROR);
		//db.close();
		sleep(10);
		openDB();//出错时尝试重新连接db
		return false;
			//	continue;
	}
	int rowCnt1 = query1.numRow();

	if( rowCnt1 == 0 )
	{
		logContent = "execute " + sql1 + ",the result row count is 0 ";
		log.write( logContent, Log::WARN);
		//节点数目为0
	    return true;
	}
	int i=0;
	VMName *temp;
	while(i<rowCnt1)
	{
		temp=new VMName;
		strcpy(temp->vmname,query1.getStringField("VMNAME"));//获取数据库中的在线VMname，存入链表
		temp->next=vmnameHead->next;
		vmnameHead->next=temp;
		//cout<<vmnameHead->next->vmname<<endl;
		i++;
		query1.nextRow();
	}
	cout<<endl;
	return true;
}
************************************************/

void SupervisorVM::clearOldInfo()//清除过时的数据
{
	VMInfo *ph=head->next;
	VMInfo *temp;
	while(ph!=NULL)
	{
		temp=ph;
		ph=ph->next;
		delete temp;
	}
	head->next=NULL;

	VMName *ph2=vmnameHead->next;
	VMName *temp2;
	while(ph2!=NULL)
	{
		temp2=ph2;
		ph2=ph2->next;
		delete temp2;
	}
	vmnameHead->next=NULL;
}
void SupervisorVM::checkOnlineVM()
{
	String logContent;
	int count=0;
	while(1)
	{
		clearOldInfo();//清除过时的数据
		if(getLocalVM()==false)
		{
			sleep(1);
			continue;
		}
/*		if(getServerVM()==false)
		{
			sleep(1);
			continue;
		}*/
		updateVMPerformance();//时时性能每2秒一次，vm状态每6秒一次
		count++;
		if(count<3)
		{
			sleep(1);
			continue;
		}
		count=0;

/*		VMName *p1=vmnameHead->next;//数据库中在线vm
		VMInfo *p2=head->next;//本地在线vm（真实在线的）
		bool isfind=false;
		while(p1!=NULL)
		{
			//cout<<p1->vmname<<endl;
			logContent="xm list= ";
			while(p2!=NULL)
			{
				logContent+=p2->vmname;
				logContent+=" ";
				if((strstr(p1->vmname,p2->vmname)!=NULL)&&(strstr(p2->vmname,p1->vmname)!=NULL))
				{
					//cout<<"find "<<p1->vmname<<endl;
					p2->isFind=true;
					isfind=true;
					break;
				}
				p2=p2->next;
			}
			if(isfind==false)
			{
				//cout<<"in delete \n";
				log.write( logContent, Log::INFO);
				deleteOnlineVM(p1->vmname);
			}
			p2=head->next;
			p1=p1->next;
			isfind=false;
		}
		p2=head->next;
		while(p2!=NULL)
		{
			if(p2->isFind==false)
			{
				insertOnlineVM(p2);
			}
			p2=p2->next;
		}*/
		sleep(1);
	}
}
bool SupervisorVM::updateVMPerformance()
{
	String logContent;
	int res;
	VMInfo *p1=head->next;
	while(p1!=NULL)
	{
		String sname(p1->vmname);
		String scpu((p1->vmcputimeptg)/(float)(p1->vmvcpu));
		String smem(p1->vmmemptg);
		String snetout(p1->netout);
		String snetin(p1->netin);
		String stime(p1->reporttimetime);
		String sql1="insert into VMPerfData (UUID,NCIP,CPU,MEM,NETOUT,NETIN,TIME) values('"+sname+"','"+localIP+"',"+scpu+","+smem+","+snetout+","+snetin+","+stime+")";
		

		try
		{
			res = db.execSQL( sql1.c_str() );
			//logContent = sql1;
			//log.write(logContent, Log::INFO);
		}
		catch(const char* sErr1)
		{
			logContent = "execute " + sql1 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);
			return false;
				//	continue;
		}
		String sql2="update VMDetail set VCPU="+String(p1->vmvcpu)+ " where VMNAME='"+sname+"'";
		try
		{
			res = db.execSQL( sql2.c_str() );
			//logContent = sql1;
			//log.write(logContent, Log::INFO);
		}
		catch(const char* sErr1)
		{
			logContent = "execute " + sql2 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);
			return false;
				//	continue;
		}

		p1=p1->next;
	}
	return true;
}

//Date:		2013-03-25
//Description:	delete table OnlineVMS
/*******************************************************************
bool SupervisorVM::insertOnlineVM(VMInfo *temp)
{

	int res;
	String logContent;
	String sname(temp->vmname);
	String stime(ctime(&(temp->reporttimetime)));
	//cout<<vmname<<" "<<sname.c_str()<<endl;
	String sql1="insert into OnlineVMS (VMNAME,NCIP,STARTTIME) values('"+sname+"','"+localIP+"',CURRENT_TIMESTAMP())";
	try
	{
		res = db.execSQL( sql1.c_str() );
		logContent = sql1;
		log.write(logContent, Log::INFO);
	}
	catch(const char* sErr1)
	{
		logContent = "execute " + sql1 + " failed:" + sErr1;
		log.write( logContent, Log::ERROR);
		return false;
			//	continue;
	}
	return true;
}
bool SupervisorVM::deleteOnlineVM(char vmname[50])
{
	if(strlen(vmname)==0)
		return true;

	String logContent;
	int res;
	String sname(vmname);
	//cout<<vmname<<" "<<sname.c_str()<<endl;
	String sql1="delete from OnlineVMS where VMNAME='"+sname+"'";
	try
	{
		res = db.execSQL( sql1.c_str() );
		logContent = "delete online vm,vmname="+sname;
		log.write(logContent, Log::INFO);
	}
	catch(const char* sErr1)
	{
		logContent = "execute " + sql1 + " failed:" + sErr1;
		log.write( logContent, Log::ERROR);
		return false;
			//	continue;
	}
	return true;
}
****************************************************************/


