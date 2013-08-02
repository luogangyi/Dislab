#ifndef __VM__
#define __VM__

#include <iostream>
#include <map>
#include <string>
#include "../common/Log.h"
#include "../common/CppMysql.h"
#include "../common/String.h"

using namespace std;
class VM
{
public:
	VM();
	virtual ~VM();

public:
	virtual void Do();
	virtual bool sendAck();
	void setSock(int s);
	void processPara();
	int isAlreadyStart();
	static int executeDBSQL(string sql);
	static int executeDBSQL(String sql);
	static bool executeDBQuery( string sql, CppMySQLQuery & result);
	static bool executeDBQuery( String sql, CppMySQLQuery & result);
	

protected:
	string strCmd;
	string ackCode;
	int sock;
	map<string,string> paraMap;
	string vmID;
	string vmName;
	string vmUUID;
	int retCode;
	string retDesc;
	

};

#endif 
