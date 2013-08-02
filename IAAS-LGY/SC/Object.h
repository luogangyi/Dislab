#ifndef __VM__
#define __VM__

#include <iostream>
#include <map>
#include "../common/String.h"

class Object
{
public:
	Object();
	virtual ~Object();

public:
	virtual void Do();
	virtual bool sendAck();
	void setSock(int s);
	void processPara();
	String strCmd;
	String ackCode;
	int sock;
	std::map<String,String> paraMap;
	
	
	

};

#endif 
