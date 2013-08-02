#ifndef CLC_DELETE_VM
#define CLC_DELETE_VM

#include "Object.h"

class DeleteVM : public Object
{
public:
	DeleteVM();
	virtual ~DeleteVM();

public:
	virtual void Do();
	void setUserName(String user);
	bool getPara();
	bool isVMExist();
	bool isVMStarted();
	bool delVMFromDB();
	
	bool delVMFile();
	bool getNFSInfo();
	int mountNFS();
	bool mount( String src, String dest);
	bool applySC2FreeDisk();
	bool deleteNATRule();
	
private:

	String userName;
	String vmName;
	String vmID;
	String vmUUID;
	String ncNodeIP;

	String nfsIP;
	String nfsDir;
	String localNFSDir;
};







#endif

