#ifndef _UNKNOW_CMD	
#define _UNKNOW_CMD	

#include "Object.h"
#include <iostream>
class UnknownCmd : public Object
{
public:
	UnknownCmd();
	virtual ~UnknownCmd();
public:
	virtual void Do();
	
};



#endif
