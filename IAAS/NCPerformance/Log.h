#ifndef __LOG__H
#define __LOG__H

#include <iostream>
#include "String.h"

class Log
{
public:
	Log();
	~Log();

public:
	void write(String content, int type);
	bool Init(String ip, int port);
	enum contentType{INFO,WARN,ERROR};

private:
	String serverIP;
	int serverPort;
	int sock;
};


#endif
