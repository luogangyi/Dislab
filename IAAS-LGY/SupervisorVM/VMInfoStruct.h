/*
 * VMInfoStruct.h
 *
 *  Created on: 2011-11-14
 *      Author: lgy
 */
#include<ctime>
#ifndef VMINFOSTRUCT_H_
#define VMINFOSTRUCT_H_

struct VMInfo
{
	char vmname[50];
	float vmmemptg;//percentage
	int vmvcpu;//number of cpu
	char vmstate[16];
	//int cputime;//cputime used
	float vmcputimeptg;//percentage
	bool isFind;
	float vmtime;
	long netout;
	long netin;
	time_t reporttimetime;
	VMInfo *next;
//Date:		2013-03-06
//Author:	WuHao
//Description:	add type of VM
//	char vmType[10];
};

struct VMName
{
	char vmname[50];
	VMName *next;
};

#endif /* VMINFOSTRUCT_H_ */
