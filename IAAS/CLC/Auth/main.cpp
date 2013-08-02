#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "Process.h"
#include "../../common/Log.h"
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>

using namespace std;
Log log;



bool setupLog()
{
	string pattern = "%d: [%p] %c %x: %m%n";
	log4cpp::RollingFileAppender* fileAppender = new log4cpp::RollingFileAppender("fileAppender","mylog.log",10*1024*1024,10);
	log4cpp::PatternLayout* patternLayout = new log4cpp::PatternLayout();
	patternLayout->setConversionPattern(pattern);

	fileAppender->setLayout(patternLayout);
	log4cpp::Category& LOG = log4cpp::Category::getRoot();
	
	LOG.setAppender(fileAppender);
	LOG.setPriority(log4cpp::Priority::DEBUG);

	
}
int main(int argc, char* argv[])
{
	Process process;
	
	char* pos = NULL;
	pos = strrchr(argv[0],'/');	
	if( pos != NULL )
	{
		pos++;
		log.Init(pos);	
	}
	else 
		log.Init(argv[0]);

	setupLog();
	if(process.Init() == false)
	{
		cout<<"Init Process error"<<endl;
		return 0;
	}

	
	
	process.Do();

	return 0;
}
