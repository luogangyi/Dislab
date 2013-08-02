#ifndef _CLC_DISPATCH_
#define _CLC_DISPATCH_
#include "Object.h"
#include "String.h"

class Dispatch : public Object
{

public:
	Dispatch(String cmd);
	virtual ~Dispatch();

public:
	virtual void Do();
	bool getPara();
	bool getVMRamSize();
	bool findFirstCapableNC();

private:	
	int iRamSize;
	String capableNCIP;
public:
	static String dbServerIP;
	static String dbUserName;
	static String dbUserPass;



};





#endif
