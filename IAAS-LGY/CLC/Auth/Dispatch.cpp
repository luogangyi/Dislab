#include "Dispatch.h"
#include "../../common/Log.h"
#include "CppMysql.h"
#include "Config.h"


extern Log log;

String Dispatch::dbServerIP;
String Dispatch::dbUserName;
String Dispatch::dbUserPass;

Dispatch::Dispatch(String cmd)
{
	strCmd = cmd;	
	iRamSize = 0;
}

Dispatch::~Dispatch()
{

}


void Dispatch::Do()
{
	log.write("in StartVM::Do", Log::INFO);	
	
	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:GET BEST NC:RETN=4,DESC=参数错误";
		return;
	}

	if( getVMRamSize() == false )
	{
		ackCode =  "ACK:GET BEST NC:RETN=8,DESC=数据库内部错误";
		return;
	}

	if( iRamSize == 0 )
	{
		ackCode =  "ACK:GET BEST NC:RETN=1002,DESC=没有找到此虚拟机";
		return;
	}	

	if( findFirstCapableNC() == false )
	{
		ackCode =  "ACK:GET BEST NC:RETN=8,DESC=数据库内部错误";
		return;
	}
	//cout<<"IP:"<<capableNCIP<<endl;
	if( capableNCIP.size() != 0 )
		ackCode =  "ACK:GET BEST NC:RETN=0,DESC=成功,IP=" + capableNCIP;
	else
		ackCode =  "ACK:GET BEST NC:RETN=1001,DESC=未找到合适的NC节点";
}

bool Dispatch::findFirstCapableNC()
{
	CppMySQL3DB db;
	String logContent;
	int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return false;
	}

	CppMySQLQuery query ;
	String sql = "select ip,ram,cpu from onlineNC";
	try
	{
		query = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		return false;
	}	
	String ip;
	int iRam;
	CppMySQLQuery query2 ;
	int iUsedRam;
	int iCpu;
	double dUpTime;
	
	while( !query.eof() )
	{
		ip = query.getStringField( "ip" );
		iRam = query.getIntField( "ram" );
		iCpu = query.getIntField( "cpu" );
		
		sql = "select ram,uptime from PerfData where ip='" + ip + "' order by time desc limit 0,1";
		try
		{
			query2 = db.querySQL( sql.c_str() );
		}
		catch(const char* sErr)
		{
			logContent = "execute " + sql + " failed:" + sErr;
			log.write( logContent, Log::ERROR);
			query.nextRow();
			continue;
		}	

		iUsedRam = query2.getIntField( "ram" );
		dUpTime = query2.getFloatField("uptime");

		if( ( (iRam - iUsedRam) > iRamSize ) && dUpTime < iCpu * 3 )
		{
			capableNCIP = ip;
			return true;
		}
		query.nextRow();
	}
	return true;

	
}


bool Dispatch::getVMRamSize()
{
	
	CppMySQL3DB db;
	String logContent;
	int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	
	if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return false;
	}
	
	
	CppMySQLQuery query ;
	String sql = "select ram from VMDetail where id=" + sVMID;
	try
	{
		query = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		return false;
	}	
	
	int rowCnt = query.numRow();
	iRamSize = 0;
	if( rowCnt != 1 )
	{
		char buf[10];
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		return true;
	}

	iRamSize = query.getIntField("ram");
	return true;
	
}


bool Dispatch::getPara()
{
	processPara();


	if( paraMap.size() != 1 )
		return false;
	
	map<String,String>::iterator it;

	//获取NFS服务器IP地址信息
	it = paraMap.find("vmid");
	if( it == paraMap.end() )		
		return false;
	

	sVMID = it->second;
	
	if( sVMID.size() == 0 )
		return false;	

	
	return true;
}









