#ifndef XML_H
#define XML_H
#include <cstring>
#include <fstream>


using namespace std;

class XML
{
public:
	XML();
	~XML();
	bool begin(string vmName, int ram, int vcpu, string disk, string bridge, string mac);
	bool end();
	string getXenHeaderXML(string vmName, int ram, int vcpu, string uuid);
	string getXenDiskXML(string disk);
	string getXenNetworkXML(string mac);
	string getXenTailXML();

	//kvm
	string getKvmHeaderXML(string vmName, int ram, int vcpu, string uuid);
	string getKvmDiskXML(string disk);
	string getKvmNetworkXML(string mac);
	string getKvmTailXML();
public:
	
};






#endif

