#include<iostream>
#include<cstdio>
#include"../common/String.h"


using namespace std;


class SC{
public:

    SC();
int addnode(String IP,String Dir,int Capacity,int TID);

int deletenode(String IP,String Dir);

int querynode();

int querynodedetails(String IP,String Dir);

int testnode(String IP,String Dir);

int applyforspace(int applysize,bool isdisk,int VMID,int VMTID,String VirtType);

public:
	 String dbServerIP;
	 String dbUserName;
	 String dbUserPass;

};
