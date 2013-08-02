/*
¹¦ÄÜ£ºÇ¨ÒÆÐéÄâ»ú
*/
#ifndef _CLC_VM_MIGRATE
#define  _CLC_VM_MIGRATE

#include "Object.h"

class VMMigrate : public Object
{
public:
	VMMigrate();
	virtual ~VMMigrate();

public:
	bool init();
	virtual void Do();
	bool isNCOverLoad(String ncip);
	String findNC(String vmName);
	bool migrateVM(String oldNCIP, String newNCIP, String vmName);
	bool prepareDir(String ncIP,String vmName);
	bool isVMOverLoad(String vmName);
	int getNCFreeMemory(String ncip);
	int getVMMemory(String vmname);
	bool expandVMMem(String vmName, String ncip);
	bool migrate(String vmName);

private:
	int ncServerPort;
	int ncUsedMem;
};








#endif


