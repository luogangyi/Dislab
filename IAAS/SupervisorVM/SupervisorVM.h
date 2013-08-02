/*
 * SupervisorVM.h
 *
 *  Created on: 2011-11-15
 *      Author: lgy
 */

#ifndef SUPERVISORVM_H_
#define SUPERVISORVM_H_
#include "../common/String.h"
#include "VMInfoStruct.h"
#include "../common/CppMysql.h"
#include <libvirt/libvirt.h>

class SupervisorVM
{
protected:

	VMInfo *head;//头节点
	VMName *vmnameHead;//存储名字的头节点
	String databaseServer;//数据库服务器
	String username;//用户名
	String password;//密码
	String dbname;//数据库名
	String localIP;//本机IP
	CppMySQL3DB db;//数据库连接
	void clearOldInfo();//清除过时的数据
	bool deleteOnlineVM(char vmname[50]);
	bool insertOnlineVM(VMInfo *temp);
	bool updateVMPerformance();//更新vm信息
	bool openDB();//打开数据库
	virConnectPtr conn;

public:
	SupervisorVM();
	bool ini();//初始化
	~SupervisorVM();
	bool getLocalVM();
	bool getServerVM();//获取在数据库里注册的VM
	void checkOnlineVM();//比较数据库中的vm和本机的是否相同
};


#endif /* SUPERVISORVM_H_ */
