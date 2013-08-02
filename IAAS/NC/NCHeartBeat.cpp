#include "NCHeartBeat.h"
#include "VM.h"
#include "HardwareInfo.h"
#include <sstream>
#include <errno.h>

extern string localip;
extern String dbServerIP;
extern String dbUserName;
extern String dbUserPass;
extern int heartbeatinterval;

NCHeartBeat::NCHeartBeat()
{

}


NCHeartBeat::~NCHeartBeat()
{
}

bool NCHeartBeat::init()
{
	time_t t;
	time(&t);
	hardwareinfo hwinfo;
	HardwareInfo HI;
	hwinfo.cpucount=HI.getCPUcount();
	hwinfo.disksize=HI.getDiskSize();
	hwinfo.ramsize=HI.getRamSize();
	hwinfo.netwidth=HI.getNetWidth();
	hwinfo.virtType=HI.getVirtType();
	
	hwinfo.time=t;	
	
	
	String sql = "delete from onlineNC where IP='"+ String(localip) +"'";

	if( VM::executeDBSQL( sql ) == -1 )
		return false;

	ostringstream oss;
	oss<<"insert into onlineNC (IP,TIME,CPU,RAM,DISK,NET,VirtType,update_at) values ('";
	oss<<localip<<"',"<<hwinfo.time<<","<<hwinfo.cpucount<<",";
	oss<<hwinfo.ramsize<<","<<hwinfo.disksize<<","<<hwinfo.netwidth;
	oss<<",'"<<hwinfo.virtType<<"',"<<hwinfo.time<<")";

	sql = oss.str().c_str();

	if( VM::executeDBSQL( sql ) == -1 )
		return false;
	
	return true;
}



bool NCHeartBeat::begin()
{	
		
	pthread_t pid;
	int iRet = pthread_create(&pid, NULL, heartbeatThreadFunc, NULL);
	if ( iRet )
	{			
		cout<<"pthread_create:"<<strerror(errno)<<endl;
		return false;
	}
	
	return true;
}

void* NCHeartBeat::heartbeatThreadFunc(void* arg)
{    
	time_t now;	
	char buff[50];
	
	while(1)
	{
		time(&now);
		memset(buff,0,sizeof(buff));
		sprintf(buff,"%d",now);
		
		string sql = "update onlineNC set update_at=" + string(buff);
		sql += " where ip='" + localip + "'";
		VM::executeDBSQL( sql );
		sleep(heartbeatinterval);
	}
	
    return NULL;    	
 }
