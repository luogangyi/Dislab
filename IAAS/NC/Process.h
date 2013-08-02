#ifndef __PROCESS_H
#define __PROCESS_H
#include <iostream>
#include "VM.h"
#include "../common/Log.h"
class Process
{
public:
	Process();
	~Process();

public:
	bool Init();	
	void Do();
private:
	string getNetCardIP(string dev);

private:
	int serverPort;
	int listenSocket;
	
};

#endif
