#ifndef __AUTO_SCALING__H
#define __AUTO_SCALING__H
#include <iostream>

#include "Object.h"


class AutoScaling : public Object
{

public:
	AutoScaling();
	virtual ~AutoScaling();

public:
	void Do();

	int processStartPolicy(String groupID,int maxSize, int minSize, int step, String vmID, int activeVMCnt);
	bool isTriggerStartPolicy(String groupID, int activeVMCnt);
	bool startVM(String vmID);

	int processClosePolicy(String groupID,int maxSize, int minSize, int step, String vmID, int activeVMCnt);
	bool isTriggerClosePolicy(String groupID, int activeVMCnt);
	bool closeVM(String vmID);
	bool isVMOverLoad(String vmid);
	bool isVMLowLoad(String vmid);

	bool isGroupOverLoad(String groupID);
	bool expandGroup(String groupID);
	bool expandVM(String vmid);
	bool shrinkVM(String vmuuid, String vmid);
	bool isGroupCanExpand(String groupID);

	void processSingleVMExpand();
	int isVMCanExpand(String vmID);
	int isGroupVMCanExpand(String vmID);
	
	bool isVMCanShrink(String vmuuid);
};



#endif


