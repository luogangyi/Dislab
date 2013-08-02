#ifndef __GET_VNC_CONSOLE__
#define __GET_VNC_CONSOLE__

#include "Object.h"


class GetVNCConsole : public Object
{

public:
	GetVNCConsole();
	~GetVNCConsole();

	virtual void Do();	
	bool getPara();
	bool isVMStarted();
	bool isVMExist();	

private:
	bool getVNCPort();

	String ncNodeIP;
	String vmUUID;
	String vmID;
	String port;
	

};



#endif


