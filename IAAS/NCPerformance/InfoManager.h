/*
 * InfoManager.h
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#ifndef INFOMANAGER_H_
#define INFOMANAGER_H_
#include "infostruct.h"
#include "RealTimeInfo.h"
#include "String.h"

class InfoManager
{
protected:
	int timeperiod;
	RealTimeInfo RT;
	//int inimem;//初始内存
	String localIP;//本机ip
	performanceThreshold pt;//保存性能门限参数
	String databaseServer;//数据库服务器
	String username;//用户名
	String password;//密码
	String dbname;//数据库名
	String netcard;//要监控的网卡端口
	int cpucount;//cpu数量，方便计算性能瓶颈时使用
	String serverIp;//服务器ip
	int serverPort;//服务器端口
	void in_send(realtimeinfo *ri);
public:
	bool ini();//初始化参数
	InfoManager();
	void thresholdIni();//初始化门限值
	void sendRealTimeInfo();//发送NC实时信息
	void performanceTest(realtimeinfo rt);//性能检测
	int sendHardwareInfo();//发送硬件信息，失败则返回-1;

};


#endif /* INFOMANAGER_H_ */
