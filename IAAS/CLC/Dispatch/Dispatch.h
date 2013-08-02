#ifndef _CLC_DISPATCH_
#define _CLC_DISPATCH_
#include "Object.h"



struct NCNodeInfo
{
	int iRamSize;
	int iCPUCnt;
	String ipAddr;
};

class Dispatch : public Object
{

public:
	Dispatch(String cmd);
	virtual ~Dispatch();

public:
	virtual void Do();
	bool getPara();
	bool getVMInfo();
	bool findNC();

private:	
	int iRamSize;
	int iCPUCnt;
	String capableNCIP;
	String virtType;
public:
	static String dbServerIP;
	static String dbUserName;
	static String dbUserPass;



};





#endif
