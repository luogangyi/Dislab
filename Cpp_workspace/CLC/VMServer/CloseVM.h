#ifndef CLC_CLOSE_VM
#define CLC_CLOSE_VM

#include "Object.h"

class CloseVM : public Object
{
public:
	CloseVM();
	virtual ~CloseVM();
public:
	virtual void Do();
	void setUserName(String user);
	bool getPara();
	bool isVMExist();
	bool isVMStarted();
	bool closeVM();
	bool updateDB();
	//bool closeVMByName(String name, String ip);
	bool closeCopyVM();
	
	//bool getNCIPByVMName(String name, String& ip);
	
private:

	String userName;
	String vmID;
	String ncNodeIP;
};









#endif

