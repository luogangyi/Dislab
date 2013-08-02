#include "UnknownCmd.h"
using namespace std;

UnknownCmd::UnknownCmd()
{

}

UnknownCmd::~UnknownCmd()
{
}

void UnknownCmd::Do()
{
	ackCode =  "ACK:UNKNOWN CMD:RETN=6,DESC=Î´Ê¶±ðµÄÃüÁî";
}
