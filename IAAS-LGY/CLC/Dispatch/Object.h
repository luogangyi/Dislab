#ifndef __VM__
#define __VM__

#include <iostream>
#include <map>
#include "../../common/String.h"
#include "../../common/Log.h"


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
	

protected:
	String strCmd;
	String ackCode;
	int sock;
	std::map<String,String> paraMap;
	String sVMID;
	
	

};

#endif 
