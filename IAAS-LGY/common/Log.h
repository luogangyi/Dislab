#ifndef __LOG__H
#define __LOG__H

#include <iostream>
#include "String.h"
#include <string>
#include <cstdio>
#include <cstdlib>

class Log
{
public:
	Log();
	~Log();

public:
	//void write(String content, int type);
	void write( String content, int type);
	//void write( string content, int type);
	bool Init(char* fileName);
	enum contentType{INFO,WARN,ERROR};

private:
	String logFileName;
	String programName;
	
};


#endif

