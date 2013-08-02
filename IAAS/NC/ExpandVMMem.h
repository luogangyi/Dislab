#ifndef __EXPAND_VMMEM_
#define __EXPAND_VMMEM_
#include "VM.h"
#include <iostream>
#include "../common/String.h"



class ExpandVMMem : public VM
{
public:
	ExpandVMMem(string cmd);
	virtual ~ExpandVMMem();

public:
	virtual void Do();	
	bool getPara();
	int getVMMemory();
	int getVMCPUCnt();
	

private:

	bool expandFlag;
	
};


#endif
