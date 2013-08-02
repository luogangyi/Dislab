#ifndef __LIBVIRT_EVENT_HANDLE__
#define __LIBVIRT_EVENT_HANDLE__

#include "VM.h"
#include <libvirt/libvirt.h>

class LibVirtEventHandle : public VM
{
public:
	LibVirtEventHandle();
	~LibVirtEventHandle();

	bool init();
	bool begin();
private:
	virConnectPtr dconn;
	static void* libvirtEventHandleThreadFunc(void* arg);
	static int myDomainEventCallback(virConnectPtr conn , virDomainPtr dom,int event,int detail,void *opaque );
	
};


#endif

