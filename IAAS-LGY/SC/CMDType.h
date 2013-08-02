#include<iostream>
#include<cstdio>
#include"../common/String.h"

class CMDType
{
	public:

	enum Type
	{
	ADSP,//添加节点
	RVSP,//删除节点
	APST,//申请空间
	DEST,//删除空间
	QUSP,//查询存储点
	QUDE,//查询存储点内容
	TEST,//测试节点
	MVST,//内容转存
	ERROR// 无法识别
	};


public:
	CMDType::Type processcmd(const char* cmd);
	String  processack(const char* cmd,CMDType::Type type);
	String  processadsp(const char* cmd);
	String  processrvsp(const  char* cmd);
	String  processapst(const char* cmd);
	String  processdest(const char* cmd);
	String  processqusp(const char* cmd);
	String  processqude(const char* cmd);
	String  processtest(const char* cmd);
	String  processmvst(const char* cmd);
	String  processerror();

};
