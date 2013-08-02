/*
 * main.cpp
 *
 *  Created on: 2011-11-15
 *      Author: lgy
 */
#include "SupervisorVM.h"
#include "../common/Log.h"
#include "../common/CppMysql.h"
#include <stdlib.h>
extern Log log;

int main()
{
	SupervisorVM svm;
	if(svm.ini()==false)
	{
		log.write("initial error!",Log::ERROR);
		exit(0);
	}

	svm.checkOnlineVM();
}

