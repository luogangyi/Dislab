#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "libvirtEventHandle.h"
#include <sys/time.h>

extern string localip;

void* heartBeatThreadFunc(void* arg);
LibVirtEventHandle::LibVirtEventHandle()
{
	dconn = NULL;
}

LibVirtEventHandle::~LibVirtEventHandle()
{

}

bool LibVirtEventHandle::init()
{
	int callbackret = -1;
    int ret = virEventRegisterDefaultImpl();
	if( ret < 0 )
	{
		cout<<"virEventRegisterDefaultImpl:"<<strerror(errno)<<endl;		
		return false;
	}
    dconn = virConnectOpen("");

	if( NULL == dconn )
	{		
		cout<<"virConnectOpen:"<<strerror(errno)<<endl;
		return false;
	}

	callbackret = virConnectDomainEventRegisterAny(dconn,
                                                    NULL,
                                                    VIR_DOMAIN_EVENT_ID_LIFECYCLE,
                                                    VIR_DOMAIN_EVENT_CALLBACK(myDomainEventCallback),
                                                    NULL, NULL);
    if (callbackret == -1)
    {
    	cout<<"virConnectDomainEventRegisterAny:"<<strerror(errno)<<endl;		
    	return false;
    }
	return true;
}

bool LibVirtEventHandle::begin()
{	
	//更新已关闭虚拟机的状态
	char **inactiveDom;  
	char buff[40]={0};	
	int numOfInactiveDom = virConnectNumOfDefinedDomains(dconn);  
	inactiveDom = (char**)malloc(sizeof(char*) * (numOfInactiveDom)); 
	
	int domCnt = virConnectListDefinedDomains(dconn, inactiveDom, numOfInactiveDom);

	if( domCnt == -1 )
		return false;
	virDomainPtr dom;
	for( int i=0; i < domCnt; i++)
	{
		memset(buff, 0, sizeof(buff));
		
		dom = virDomainLookupByName(dconn, inactiveDom[i]);
	
		if( dom != NULL )
		{
			if( virDomainGetUUIDString( dom, buff) != -1 )
			{
				
				string sql = "update VMDetail set status=0,host='' where uuid='" + string(buff) + "'";
				if( VM::executeDBSQL( sql ) == -1 )
				{
					free(inactiveDom);
					return false;
				}
			}
		}
		else
		{
			free(inactiveDom);
			return false;
		}
		
	}
	free(inactiveDom);
	////////////////////////
	//更新活动虚拟机的状态
	int numDomains;
	int *activeDomains;

	numDomains = virConnectNumOfDomains(dconn);
	activeDomains = (int*)malloc(sizeof(int*) * numDomains);
	numDomains = virConnectListDomains(dconn, activeDomains, numDomains);
	
	for (int i = 0 ; i < numDomains ; i++) 
	{
		memset(buff, 0, sizeof(buff));
		
		dom = virDomainLookupByID(dconn, activeDomains[i]);
	
		if( dom != NULL )
		{
			if( virDomainGetUUIDString( dom, buff) != -1 )
			{
				
				string sql = "update VMDetail set status=1,host='";
				sql += localip + "' where uuid='" + string(buff) + "'";
				if( VM::executeDBSQL( sql ) == -1 )
				{
					free(activeDomains);
					return false;
				}
			}
		}
		else
		{
			free(activeDomains);
			return false;
		}
	}
	
	free(activeDomains);
	
	///////////////
	pthread_t pid;
	int iRet = pthread_create(&pid, NULL, libvirtEventHandleThreadFunc, NULL);
	if ( iRet )
	{			
		cout<<"pthread_create:"<<strerror(errno)<<endl;
		return false;
	}

		
	return true;
}



int LibVirtEventHandle::myDomainEventCallback(virConnectPtr conn , virDomainPtr dom,int event,int detail,void *opaque )
{
	char buff[40]={0};	
	
	if( event == VIR_DOMAIN_EVENT_STOPPED )
	{
		virDomainGetUUIDString(dom, buff);		
		string sql = "update VMDetail set status=0,host='' where uuid='" + string(buff) + "'";
		if( VM::executeDBSQL( sql ) > 0 )
			virDomainUndefine(dom);
	}
    return 0;
}

void* LibVirtEventHandle::libvirtEventHandleThreadFunc(void* arg)
{    
    while(virEventRunDefaultImpl()!=-1)
		;
	
    return NULL;    	
 }






