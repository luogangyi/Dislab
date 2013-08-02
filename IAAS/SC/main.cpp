
#include<iostream>
#include<cstdio>
#include<stdlib.h>
#include"../common/String.h"
#include"SC.h"
#include "../common/CppMysql.h"
#include "../common/Log.h"
#include "SCnode.h"
#include"CMDreceiver.h"
#include"Object.h"
#include"CMDType.h"
#include"../common/Config.h"
using namespace std;
Log log;
int main(){
	int port;
	ConfigFile::strFileName = "scfg.cfg";
	bool i = ConfigFile::getIntValue("Port",port);
	if(!i){
		cout<<"get port failed!";
		exit(-1);
	}
	CMDreceiver *re=new CMDreceiver(port);
	re->receiver();


return 1;
}
/*ADD NFS SP:IP=,DIR=,CAPACITY=    ACK: ADD NFS SP:RETN=,DESC=
 *     RMV NFS SP: IP=,DIR=,      ACK: RMV NFS SP:RETN=,DESC=
 *     APPLY NFS STOR:VMID=,CAPACITY=,TYPE=,VMTID=   ACK: APPLY NFS STORE:RETN=,DESC=，ip=,dirname=
 *     DEL NFS STOR:ID=,TYPE=     	ACK: DEL NFS STOR:RETN=,DESC=
 *     QRY NFS SP:                ACK: QRY NFS SP:RETN=,DESC=,TOTAL=,NUM=,RESULT=
 *     QRY SP DETAIL:IP=,DIR=  ACK: QRY SP DETAIL: RETN=,DESC=,RESULT=
 *     TEST NFS SP:IP=,DIRNAME=    ACK: TEST NFS SP: RETN=,DESC=[,used=][,remain=]
 *     MV NFS STOR:SIP=,SSPDIRNAME=,DIP=,DSPDIRNAME=     ACK: MV NFS STOR: RETN=,DESC=
*/
