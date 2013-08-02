#include "Auth.h"

extern Log log;


String Auth::dbServerIP;
String Auth::dbUserName;
String Auth::dbUserPass;

Auth::Auth(String cmd)
{
	strCmd = cmd;
	userName = "";
	userPass = "";
}

Auth::~Auth()
{
	
}


void Auth::Do()
{
	log.write("begin Auth::Do.", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:LOGIN:RETN=4,DESC=parameter error";
		return;
	}
	log.write("end process parameter.", Log::INFO);
	

	//检查用户名密码是否正确

	log.write("begin check user name and password.", Log::INFO);
	if( checkPass() == false)
	{
		ackCode =  "ACK:LOGIN:RETN=1001,DESC=user name or pass is error";
		return;
	}
	log.write("end check user name and password.", Log::INFO);

	if( userType == 0 )
	{
		ackCode =  "ACK:LOGIN:RETN=0,DESC=success,RESULT=-1|-1";
		return;
	}
	//////////////////////获取权限数据
	log.write("begin get permission data.", Log::INFO);
	if( getPermissionData() == false)
	{
		ackCode =  "ACK:LOGIN:RETN=8,DESC=system internal error";
		return;
	}
	log.write("end get permission data.", Log::INFO);



	ackCode =  "ACK:LOGIN:RETN=0,DESC=success,RESULT=" + permissionData;

	log.write("end Auth::Do.", Log::INFO);	
}

bool Auth::getPermissionData()
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
	CppMySQLQuery query2 ;
	String sql = "select id from VMDetail where userid=" + userID;
	
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
	
	permissionData = "";
	
	
	String tmp;
	String vmID;
	String diskName;
	while( !query.eof() )
	{
		vmID = query.getStringField( "id" );		
		permissionData += vmID + "|0&";


		sql = "select DiskName from VDiskDetail where vmid=" + vmID;
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

		while( !query2.eof() )
		{
			diskName = query2.getStringField( "DiskName" );
			permissionData += diskName + "|1&";
			query2.nextRow();
		}
		
		query.nextRow();
	}
	
	
	int iSize = permissionData.size();
	if( iSize > 0 )	
		permissionData[iSize-1]='\0';
	
	return true;
}

bool Auth::checkPass()
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
	String sql = "select id,type from  User where user='" + userName + "'";
	sql += " and passwd='" + userPass + "'";
	try
	{
		query = db.querySQL( sql.c_str() );
	}
	catch(const char* sErr)
	{
		logContent = "execute " + sql + " failed:" + sErr;
		log.write( logContent, Log::ERROR);
		db.close();
		return false;
	}	
	
	int rowCnt = query.numRow();
	
	if( rowCnt != 1 )
	{
		char buf[10];
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		db.close();
		return false;
	}

	userID = query.getStringField("id");
	userType = query.getIntField("type");
	db.close();
	return true;

}


bool Auth::getPara()
{
	
	processPara();


	if( paraMap.size() != 2 )
		return false;
	
	map<String,String>::iterator it;

	
	it = paraMap.find("user");
	if( it == paraMap.end() )		
		return false;
	

	userName = it->second;
	
	if( userName.size() == 0 )
		return false;	

	it = paraMap.find("passwd");
	if( it == paraMap.end() )		
		return false;
	

	userPass= it->second;
	
	if( userPass.size() == 0 )
		return false;	

	
	return true;
}

