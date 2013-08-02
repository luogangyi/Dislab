#ifndef _UNKNOW_CMD	
#define _UNKNOW_CMD	

#include "VM.h"
#include <iostream>
class UnknownCmd : public VM
{
public:
	UnknownCmd();
	virtual ~UnknownCmd();
public:
	virtual void Do();
	
};



#endif
