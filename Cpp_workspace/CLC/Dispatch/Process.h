#ifndef __PROCESS_H
#define __PROCESS_H
#include <iostream>
#include "../../common/Log.h"
#include "../../common/String.h"

class Process
{
public:
	Process();
	~Process();

public:
	bool Init();	
	void Do();

private:
	int serverPort;
	int listenSocket;
	
};

#endif
