#ifndef _CLC_LOGIN	
#define _CLC_LOGIN	

#include "Object.h"


class Login : public Object
{

public:
	Login();
	virtual ~Login();
public:
	virtual void Do();
	bool init();
	//void setPara(String para);
	bool getPara();
	bool send2AuthServer(String cmd);
	String recvAckFromAuthServer();

private:
	int authSock;
	String userName;
	String userPass;

};









#endif


