#ifndef _CLOSE_VM_H
#define _CLOSE_VM_H

#include "VM.h"
#include <iostream>


class CloseVM : public VM
{
public:
	CloseVM(string cmd);
	virtual ~CloseVM();
public:
	virtual void Do();
	bool getPara();
	int closeVM();

	bool getVMInfoFromDB();

};




#endif
