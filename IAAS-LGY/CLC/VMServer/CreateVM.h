#ifndef _CREATE_VM_H
#define _CREATE_VM_H

#include "Object.h"

class CreateVM : public Object
{
public:
	CreateVM();
	virtual ~CreateVM();
public:
	virtual void Do();
	void setUserName( String user);
	bool checkUserVMCnt();
	bool getPara();
	bool isVMExist();
	bool getNFSAddr();
	bool apply2SC(String request);
	bool copyVMImageFile();
	int mountNFS();
	bool mount( String src, String dest);
	bool getFreeMac();
	
	bool createVMCfgFile();
	bool addVM2DB();
	bool getUserIDByName();

	void freeData();
	bool applySC2FreeDisk();
	bool isSuccess();
	String getVMName();

private:
	String userName;
	String vmName;
	String vmID;
	String vmUUID;
	String tid;
	String virtType;
	String nfsIP;
	String nfsDir;
	String diskSize;
	String ramSize;
	String cpuCnt;
	String osDesc;
	String templateImageFileName;
	String localNFSDir;
	String mac;
	String userID;
	String changedTemplateFileName;
	bool bSuccess;
public:
	static int UserMaxVM;

};





#endif

