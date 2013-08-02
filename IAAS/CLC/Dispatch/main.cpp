#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "Process.h"
#include "../../common/Log.h"

using namespace std;
Log log;


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
	
	if(process.Init() == false)
	{
		cout<<"Init Process error"<<endl;
		return 0;
	}

	
	process.Do();

	return 0;
}
