/*
 * infostruct.h
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#ifndef INFOSTRUCT_H_
#define INFOSTRUCT_H_
#include "String.h"

struct realtimeinfo
{
	long reporttime;//报告的时间
	int cpuratio;//cpu利用率*1000
	int ramratio;//ram利用率*1000
	int ramused;
//	int diskio;//
	int cpu_used_count;
	int netin;//网络接收流量
	int netout;//网络发送流量
	float uptime;//uptime命令的第一个负载值
};

struct performanceThreshold//性能门限
{
	int maxCpu;//最大cpu使用率
	float maxLoad;//最大负载
	int maxNetIn;//最大网络接受带宽
	int maxNetOut;//最大网络发送带宽
	int maxRam;//最大内存使用率
};

struct hardwareinfo
{
	long time;
	int cpucount;//cpu数量
	int ramsize;//内存大小
	int disksize;//磁盘大小
	int netwidth;//网卡带宽，单位Mb/s

//Date:		2013-03-06
//Author:	WuHao
//Description:	add VM type
	String virtType;
};

struct cputime
{
	long long  used;
	long long  total;
};

struct ram
{
	int used;
	int total;
};

struct netio
{
	long sends;
	long receives;
};

#endif /* INFOSTRUCT_H_ */
