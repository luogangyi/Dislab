/*
 * main.cpp
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */
#include <stdlib.h>
#include "InfoManager.h"
#include "Log.h"
#include "RealTimeInfo.h"
#include "stdio.h"
extern Log log;

bool setRedirect(const char* fileName)//重定向至文件
{
FILE *stream ;
   if((stream = freopen(fileName, "w", stdout)) == NULL)
      return false;



  // stream = freopen("CON", "w", stdout);
return true;

}

int main()
{
	
	setRedirect("NCPerformance.log");

	InfoManager im;

	if(im.ini()==false)
	{
		log.write("NC initial failed!",Log::ERROR);
		exit(1);
	}
	im.thresholdIni();//初始化门限值

/**********************************************************
	if(im.sendHardwareInfo()<1)//首先发送硬件信息
	{
		log.write("Error in send hardwareinfo",Log::ERROR);
		exit(1);
	}
*************************************************************/

	im.sendRealTimeInfo();//发送实时信息

	return 1;
}



