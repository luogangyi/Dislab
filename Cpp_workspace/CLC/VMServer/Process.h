#ifndef __PROCESS_H
#define __PROCESS_H
#include <iostream>
#include "../../common/Log.h"
#include "Login.h"
#include <map>

class Process
{
public:
	Process();
	~Process();

public:
	enum cmdType{LOGIN,CREATEVM,STARTVM,CLOSEVM,DELETEVM,QUERYVM,GETVNCCONSOLE,UNKNOWN};
	bool Init();	
	void Do();
	void setnonblocking(int sock);
	cmdType analyzeCmd(char * cmd);
	void processCmd(char* cmd, int sock);
	bool getStrParaValue(String para, String item, String& sValue);
	//getStrParaValue2 para参数直接是名称值对
	bool getStrParaValue2(String para, String item, String& sValue);
	bool getIntParaValue(String para, String item, int& iValue);
	bool getIntParaValue2(String para, String item, int& iValue);
	void deleteSockFromEpoll(int sock);
	bool sendAck( String ackCode, int sock);

	void saveUserPermissionData(String user, String data, int sock);
	void appendUserPermissionData( String data, int sock);
	String getUserPermissionDataBySock(int sock);
	String getUserNameBySock( int sock );

public:
	void processUnknownCmd(int sock);
	void processCreateVMCmd(int sock, String para);
	bool isUserLogin( int sock );
	void processStartVMCmd(int sock, String para);
	void processCloseVMCmd(int sock, String para);
	void processDeleteVMCmd(int sock, String para);
	void processQueryVMCmd(int sock, String para);
	void processGetVNCConsoleCmd(int sock, String para);
private:
	int serverPort;
	int listenSocket;
	int epfd;
	struct userPermissionData
	{
		String user;
		String data;
	};
	std::map<int, userPermissionData*> userPermissionDataMap;
	

private:
	Login login;
	
};

#endif
