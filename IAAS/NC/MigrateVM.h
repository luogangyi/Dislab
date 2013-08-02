#ifndef NC_MIGRATE_VM	
#define NC_MIGRATE_VM	

#include "VM.h"

class MigrateVM : public VM
{
public:
	MigrateVM(string cmd);
	virtual ~MigrateVM();

public:
	virtual void Do();
	bool getPara();
	bool migrateVM2IP();

private:
	
	string toip;

};







#endif


