
#include<iostream>
#include<cstdio>
#include"../common/String.h"
using namespace std;
class SCnode
{
public:

String IP;

String Dir;

int CAPACITY;

SCnode();

int applyforspace(int ID,int applysize,bool isdisk,int VMID);

int releasespace(int ID,bool type);

int redirect(String IP1,String Dir1,String IP2,String Dir2);

public:
	 String dbServerIP;
	 String dbUserName;
	 String dbUserPass;

};

