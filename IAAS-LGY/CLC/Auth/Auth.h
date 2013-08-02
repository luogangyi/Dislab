#ifndef _CLC_AUTH_
#define _CLC_AUTH_
#include "Object.h"


class Auth : public Object
{
public:
	Auth(String cmd);
	virtual ~Auth();

public:
	virtual void Do();
	bool getPara();
	bool checkPass();
	bool getPermissionData();

private:
	String userName;
	String userPass;
	int userType;
	String userID;
	String permissionData;
public:
	static String dbServerIP;
	static String dbUserName;
	static String dbUserPass;

};











#endif


