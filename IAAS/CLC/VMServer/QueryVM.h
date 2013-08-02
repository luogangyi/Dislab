#ifndef _CLC_QUERY_VM
#define _CLC_QUERY_VM

#include "Object.h"

class QueryVM : public Object
{
public:
	QueryVM();
	~QueryVM();
public:

	virtual void Do();
	void setUserName(String user);
	bool getPara();	
	bool getVMInfo();

private:
	String userName;
	String vmName;
	String vmID;

};





#endif


