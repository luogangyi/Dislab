#ifndef NC_PREPARE
#define NC_PREPARE

#include "VM.h"

class PrepareDir : public VM
{
public:
	PrepareDir(string cmd);
	virtual ~PrepareDir();

public:
	virtual void Do();
	bool getPara();

	int mountNFS();	
	bool mount(string src, string dest);
	string getCurrentDir();
	bool isFileExist( string file);

private:
	string nfsip;
	string nfsDir;
};

#endif

