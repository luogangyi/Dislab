/*
 * RealTimeInfo.h
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#ifndef REALTIMEINFO_H_
#define REALTIMEINFO_H_
#include <libvirt/libvirt.h>
#include "infostruct.h"
#include "String.h"

class RealTimeInfo //获取各种系统实时信息
{
protected:
	float uptime_anylyse(char *);//分析uptime的字符串
	virConnectPtr conn;
	int memtotal;
	String netcard;//要监控的网卡端口
public:
	RealTimeInfo();
	bool iniMemtotal();
	cputime getCPUtime();//获取cpu使用时间,返回-1失败
	ram getRam();//获取ram使用率
	netio getNetio();//获取网络流量，KB/s
	float getUptime();//获取uptime命令的第一个负载值
	int getCPU_Used_Count();//获取已使用cpu个数
};


#endif /* REALTIMEINFO_H_ */
