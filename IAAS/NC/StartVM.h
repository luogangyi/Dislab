#ifndef __START_VM_
#define __START_VM_
#include "VM.h"
#include <iostream>




class StartVM : public VM
{
public:
	StartVM(string cmd);
	virtual ~StartVM();

public:
	virtual void Do();	
	bool getPara();
	
	int mountNFS();
	string getCurrentDir();
	bool isFileExist(string file);
	bool umount(string dir);
	bool mount(string src, string dest);
	bool startVM();
	bool getVMInfoFromDB();
	bool startXenVM();
	bool startKvmVM();
	
	

private:
	
	string nfsIP;
	string nfsDir;	
	
	string vmMac;
	string vmIP;
	
	int iRam,iCPU;
	string virtType;
	string localVMPath;
	
	

};


#endif
