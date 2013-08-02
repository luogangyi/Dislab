/*
 * InfoManager.cpp
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#include "InfoManager.h"
#include "infostruct.h"
#include "CppMysql.h"
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
#include "String.h"
#include "Log.h"
#include "Config.h"
using namespace std;

Log log;//全局日志类



InfoManager::InfoManager()
{

}
bool InfoManager::ini()
{

	String logServerIP = "";
	int logServerPort = -1;
	timeperiod=10;//默认值为10
	bool bRet;
	//¶ÁÈ¡ÊýŸÝ¿âÅäÖÃÐÅÏ¢
	bool bRet1 = ConfigFile::getStrValue( "SupervisorServerIP", serverIp);
	bool bRet2 = ConfigFile::getIntValue( "SupervisorServerPort",serverPort);
	bool bRet3 = ConfigFile::getIntValue( "SuperviosrTimePeriod",timeperiod);
	bool bRet4 = ConfigFile::getStrValue( "DBServerAddr", databaseServer);
	bool bRet5 = ConfigFile::getStrValue( "DBUserName", username);
	bool bRet6 = ConfigFile::getStrValue( "DBUserPass", password );
	bool bRet7 = ConfigFile::getStrValue( "DBName", dbname);
	bool bRet8 = ConfigFile::getStrValue( "NetCard", netcard);
	bool bRet9=ConfigFile::getStrValue("NCLocalIP",localIP);

	if(!bRet1||!bRet2||!bRet3||!bRet4||!bRet5||!bRet6||!bRet7||!bRet8||!bRet9)
	{
		log.write("read Supervisor server config error", Log::ERROR);
		return false;
	}

	return true;
}

void InfoManager::thresholdIni()//初始化门限值
{
	cpucount=1;
	//默认参数
	pt.maxCpu=800;
	pt.maxLoad=2*cpucount;
	pt.maxNetIn=10000;//KB/s
	pt.maxNetOut=10000;//kB/s
	pt.maxRam=800;

	//从配置文件中读取参数
	bool bRet1 = ConfigFile::getIntValue( "maxCpu", pt.maxCpu);
	bool bRet2 = ConfigFile::getFloatValue("maxLoad",pt.maxLoad);
	bool bRet3 = ConfigFile::getIntValue( "maxNetIn",pt.maxNetIn);
	bool bRet4 = ConfigFile::getIntValue( "maxNetOut",pt.maxNetOut);
	bool bRet5 = ConfigFile::getIntValue( "maxRam",pt.maxRam);
	pt.maxLoad=pt.maxLoad*cpucount;
	if(!bRet1||!bRet2||!bRet3||!bRet4||!bRet5)
	{
		log.write("Can't read threshold ,now use default value;",Log::WARN);
	}


}

void InfoManager::sendRealTimeInfo()
{
	/**********每秒获取一次内存，每period获取一次cpu和网络流量************************/
	cputime c_old,c_new;
	ram *r_old;
	netio n_old,n_new;
	int cpuratio;
	int ramratio;
	int netin;
	int netout;
	int truetimeperiod;
	float load;
	time_t t1,t2;

	r_old=new ram[timeperiod];

	//初始化
	c_old=RT.getCPUtime();
	n_old=RT.getNetio();
	for(int i=0;i<timeperiod;i++)
	{
		r_old[i]=RT.getRam();
		sleep(1);
	}

	while(1)//不断获取信息
	{
		time(&t1);
		c_new=RT.getCPUtime();
		n_new=RT.getNetio();
		for(int i=0;i<timeperiod;i++)
		{
			r_old[i]=RT.getRam();
			sleep(1);
		}
		time(&t2);
		truetimeperiod=t2-t1;//计算时间间隔
		//cout<<"c_new.used "<<c_new.used<<" c_old.used "<<c_old.used<<endl;
		//printf("%lld total=\n",c_old.total);
		//cout<<"c_new.total "<<c_new.total<<" c_old.total "<<c_old.total<<endl;
		//cout<<"c_new.used-c_old.used "<<c_new.used-c_old.used<<endl;
		//cout<<"c_new.total-c_old.total "<<c_new.total-c_old.total<<endl;
		cpuratio=(c_new.used-c_old.used)*1000.0/(c_new.total-c_old.total);
		netin=(n_new.receives-n_old.receives)/truetimeperiod;//单位 KB/s
		netout=(n_new.sends-n_old.sends)/truetimeperiod;//单位 KB/s
		int ramratio_total=0;
		int ramusedtotal=0;
		for(int i=0;i<timeperiod;i++)
		{
			//cout<<r_old[i].used<<" "<<r_old[i].total<<endl;
			ramusedtotal+=r_old[i].used;
			ramratio_total+=(r_old[i].used*1000.0/r_old[i].total);
		}
		int ramused=ramusedtotal/timeperiod;
		ramratio=ramratio_total/timeperiod;

		load=RT.getUptime();
		
		/**************保存信息****************/
		realtimeinfo rtinfo;
		rtinfo.cpuratio=cpuratio;
		rtinfo.netin=netin;
		rtinfo.netout=netout;
		rtinfo.ramratio=ramratio;
		rtinfo.reporttime=t2;
		rtinfo.uptime=load;
		rtinfo.cpu_used_count=RT.getCPU_Used_Count();
		//cout<<"load="<<load<<endl;
		rtinfo.ramused=ramused/1024;
		cout<<"Time:"<<ctime(&t2)<<" timeperiod="<<truetimeperiod<<" cpuratio="<<cpuratio/10.0<<"% ramused="<<rtinfo.ramused<<"MB"<<" load="<<load<<endl;
		/**************性能检测*******************/
		performanceTest(rtinfo);

		/***************传输****************/
		in_send(&rtinfo);//发送信能信息
		c_old=c_new;
		n_old=n_new;
	}
}

void InfoManager::performanceTest(realtimeinfo rt)//性能检测
{
	String msg;
	//若超过门限值，则发送警报
	if(rt.cpuratio>pt.maxCpu||rt.netin>pt.maxNetIn||rt.netout>pt.maxNetOut||rt.ramratio>pt.maxRam||rt.uptime>pt.maxLoad)
	{
		int res;
		CppMySQL3DB db;
		String logContent;
		int iRet = db.open(databaseServer.c_str(), username.c_str(), password.c_str(), dbname.c_str());
		if ( iRet != 0 )
		{
			log.write( "open mysql db failed",Log::ERROR);
				return ;
		}
		msg="NC is overloaded with (cpuratio,ramratio,netin,netout,uptime)=("+String(rt.cpuratio)+","+String(rt.ramratio)+","+String(rt.netin)+","+String(rt.netout)+","+String(rt.uptime)+")";
	//	char msg[]="NC is overloaded!";
		String *sip=&localIP;
		time_t t = time(0);
		char tmp[64];
		strftime( tmp, sizeof(tmp), "%Y/%m/%d %X %A ",localtime(&t) );
		String *stime=new String(tmp);

		String sql="insert into Warn (NCIP,TIME,WARNINFO) values ('"+*sip+"','"+*stime+"','"+msg+"')";
		try
		{
			res = db.execSQL( sql.c_str() );
		}
		catch(const char* sErr)
		{
			logContent = "execute " + sql + " failed:" + sErr;
			log.write( logContent, Log::ERROR);
			//cout<<sErr;
			return;
		}
		log.write(msg,Log::WARN);//写入警告信息
		delete stime;
		db.close();
		return ;
	}
}

void InfoManager::in_send(realtimeinfo *rtinfo)//发送性能信息
{
	//写入数据库
	int res;
	CppMySQL3DB db;
	String logContent;
	int iRet = db.open(databaseServer.c_str(), username.c_str(), password.c_str(), dbname.c_str());
	if ( iRet != 0 )
	{
		log.write( "open mysql db failed",Log::ERROR);
			return ;
	}
	String *sip=&localIP;
	String *stime=new String(rtinfo->reporttime);
	String *scpu=new String(rtinfo->cpuratio);
	String *sram=new String(rtinfo->ramused);
	String *sdisk=new String(0);
	String *snetin=new String(rtinfo->netin);
	String *snetout=new String(rtinfo->netout);
	String *suptime=new String(rtinfo->uptime);
	String *scpu_used_count=new String(rtinfo->cpu_used_count);
	//cout<<"uptime= "<<rtinfo.uptime<<endl;
	String sql="insert into PerfData (IP,TIME,CPU,RAM,DISKIO,NETIN,NETOUT,UPTIME) values ('"+*sip+"',"+*stime+","+*scpu+","+*sram+","+*sdisk+","+*snetin+","+*snetout+","+*suptime+")";
	try
	{
		res = db.execSQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		String logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		//cout<<sErr;
		return;
	}
	String sql2="update onlineNC set CPU_USED="+*scpu_used_count+" , MEMORY_USED="+*sram+" where IP='"+*sip+"'";
	try
	{
		res = db.execSQL( sql2.c_str() );
	}
	catch(const char* sErr)
	{
		String logContent = "execute " + sql2 + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		//cout<<sErr;
		return;
	}
	delete stime;
	delete scpu;
	delete sram;
	delete sdisk;
	delete snetin;
	delete snetout;
	delete suptime;
	delete scpu_used_count;
	db.close();
	//通信部分
	return;
}

//Date:		2013-03-25
//Author:	WuHao
//Description:	delete sending hardware info
/****************************************************************
int InfoManager::sendHardwareInfo()//发送硬件信息，失败则返回-1;
{
	time_t t;
	time(&t);
	hardwareinfo hwinfo;
	hwinfo.cpucount=HI.getCPUcount();
	hwinfo.disksize=HI.getDiskSize();
	hwinfo.ramsize=HI.getRamSize();
	hwinfo.netwidth=HI.getNetWidth();
	hwinfo.virtType=HI.getVirtType();
	//hwinfo.netwidth=-1;
	hwinfo.time=t;
	//写入数据库
	CppMySQL3DB db;
	String logContent;
	int iRet = db.open(databaseServer.c_str(), username.c_str(), password.c_str(), dbname.c_str());
	if ( iRet != 0 )
	{
		log.write( "open mysql db failed",Log::ERROR);
			return 0;
	}
	int res;
	String *sip=&localIP;
	String *stime=new String(hwinfo.time);
	String *scpu=new String(hwinfo.cpucount);
	String *sram=new String(hwinfo.ramsize);
	String *sdisk=new String(hwinfo.disksize);
	String *snet=new String(hwinfo.netwidth);
	String *sType=new String(hwinfo.virtType);
	String sql2="delete from onlineNC where IP='"+*sip+"'";
//Date:		2013-03-06
//Author:	WuHao
//Description:	add VM type
	String sql="insert into onlineNC (IP,TIME,CPU,RAM,DISK,NET,VirtType) values ('"+*sip+"',"+*stime+","+*scpu+","+*sram+","+*sdisk+","+*snet+",'"+*sType+"')";
	try
	{
		res= db.execSQL( sql2.c_str() );
		res = db.execSQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		//cout<<sErr;
		return 0;
	}
	delete stime;
	delete scpu;
	delete sram;
	delete sdisk;
	delete snet;
	db.close();
	return 1;

}*/
