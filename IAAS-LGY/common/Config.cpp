#include <cstdio>
#include <cstdlib>
#include "Config.h"
#include <iostream>
#include <fstream>
#include "String.h"




using namespace std;
String ConfigFile::strFileName = "nccfg.cfg";
bool ConfigFile::compare(String item1, char* item2)
{
	char buff[50];

	memset(buff, 0, sizeof(buff));
	strcpy(buff,item1.c_str());
	
	char* p = item2;
	for(int i=0;i<strlen(item2);i++)
	{
		item2[i] = tolower(*p);
		p++;
	}
	p = buff;
	for(int i=0;i<strlen(buff);i++)
	{
		buff[i] = tolower(*p);
		p++;
	}
	
	
	return strcmp(buff, item2) == 0;
}

bool ConfigFile::getStrValue(String sItem, String& sValue)
{	
	ifstream infile;
	char buff[256];
	infile.open(strFileName.c_str(), ios::in);
	
		
	while(!infile.eof())
	{
		memset(buff, 0 ,sizeof(buff));
		
		infile.getline(buff,256,'\n');
		if( *buff == '#' )		
			continue;
		for(int i = 0; i<256 && buff[i] != '\0'; i++)
		{
			if (buff[i] == '\r' )			
				buff[i] = '\0';	
		}
		
		for(int i = 0; i<256 && buff[i] != '\0'; i++)
		{
			if (buff[i] == '=' )
			{
				buff[i] = '\0';			

				if( compare(sItem, buff) == true)
				{

					sValue = &buff[i + 1];
					return true;
				}
			}
		}

	}
	
	infile.close();
	
	return false;
}

bool ConfigFile::getIntValue(String sItem, int& iValue)
{
	String sValue;
	if( getStrValue(sItem, sValue))
	{
		iValue = atoi(sValue.c_str());
		return true;
	}
	return false;
}
