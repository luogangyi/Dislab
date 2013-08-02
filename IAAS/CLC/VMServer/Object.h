#ifndef __VM__
#define __VM__

#include <iostream>
#include <map>
#include "../../common/String.h"
#include "../../common/Log.h"
#include "../../common/CppMysql.h"


class Object
{
public:
	Object();
	virtual ~Object();

public:
	virtual void Do();
	virtual bool sendAck();
	void setSock(int s);
	void processPara();
	String getAck();
	void setPara(String para);
	bool sendData(const char* data, int len, int sock);
	bool recvData(int sock, String& data);
	int connect2Server(const char* ip, int port);
	bool getStrParaValue(String cmdAck, String item, String& sValue);
	bool getIntParaValue(String cmdAck, String item, int& iValue);
	void closeSock(int sock);
	String getCurrentDir();
	bool isFileExist( String file);
	void setAuthData(String data );
	bool checkRight( String name );

public:
	bool executeDBQuery( String sql, CppMySQLQuery& result);
	int executeDBSQL(String sql);
protected:
	String strCmd;
	String ackCode;
	int sock;
	std::map<String,String> paraMap;
	
	String authData;
public:
	

};

#endif 
