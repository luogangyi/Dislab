#ifndef __CONFIG__H
#define __CONFIG__H
#include <iostream>
#include "String.h"

class ConfigFile
{

public:
	static bool getStrValue(String sItem, String& sValue);
	static bool getIntValue(String sItem, int& iValue);
public:
	static String strFileName;

private:
	static bool compare(String, char* );
};



#endif

