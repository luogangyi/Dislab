#ifndef __GET_VNC_CONSOLE__
#define __GET_VNC_CONSOLE__

#include "VM.h"

class GetVNCConsole : public VM
{

public:
	GetVNCConsole(string cmd);
	~GetVNCConsole();

	virtual void Do();	
	bool getPara();

private:
	bool getVNCPort();

	string port;

};



#endif

