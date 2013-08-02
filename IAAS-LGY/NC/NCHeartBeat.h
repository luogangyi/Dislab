#ifndef _NC_HEART_BEAT_
#define _NC_HEART_BEAT_
#include "../common/String.h"

class NCHeartBeat
{
public:
	NCHeartBeat();
	~NCHeartBeat();

	bool init();
	bool begin();
private:
	static void* heartbeatThreadFunc(void* arg);

};


struct hardwareinfo
{
	long time;
	int cpucount;
	int ramsize;
	int disksize;
	int netwidth;
	String virtType;
};



#endif

