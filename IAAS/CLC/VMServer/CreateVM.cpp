#include "CreateVM.h"

#include <fstream>
#include <vector>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

extern String dbServerIP;
extern String dbUserName;
extern String dbUserPass;
extern Log log;
extern String scServerIP;
extern int scServerPort;
extern int g_templateCnt;

int CreateVM::UserMaxVM = 0;
void* copyFileThreadFunc(void * arg);


struct fileNameStruct
{
	String templateFileName;
	String changedFileName;
};


CreateVM::CreateVM()
{
	userName = "";
	vmName = "";
	tid = "";
	virtType = "";
	nfsIP = "";
	nfsDir = "";
	templateImageFileName = "";
	localNFSDir = "";
	mac = "";
	osDesc = "";
	userID = "";
	changedTemplateFileName = "";
	vmID = "";
	bSuccess = false;
}


CreateVM::~CreateVM()
{

}


String CreateVM::getVMName()
{
	return vmName;
}
void CreateVM::Do()
{

	String logContent;

	
	log.write("begin CreateVM::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:LOGIN:RETN=4,DESC=parameters error";
		return;
	}
	log.write("end process parameter.", Log::INFO);

	if( getUserIDByName() == false )
	{
		ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error,can not get user info";
		return;
	}


	//虚拟机是否已存在
	if( isVMExist() == true )
	{
		ackCode = "ACK:CRE VM:RETN=1001,DESC=vm is alread exist,please use a different name";
		return;
	}
	
	//用户虚拟机数量是否已达最大
	if( checkUserVMCnt() == false )
	{
		ackCode = "ACK:CRE VM:RETN=1001,DESC=can not create new vm,reach the max vm limit";
		return;
	}

	
	

	//获取一个空闲的MAC/IP地址对
	if( getFreeMac() == false )
	{
		ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error,no free ip address";
		return;
	}
	
	if( addVM2DB() == false )
	{
		ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error";
		freeData();
		return;
	}

	
	
	if( getNFSAddr() == false )
	{
		ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error,apply disk space error";
		freeData();
		return;
	}	

	if( mountNFS() != 0 )
	{
		ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error";
		return ;
	}

	//产生配置文件
	//if( createVMCfgFile() == false )
	//{
	//	ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error,create vm config file error";
	//	freeData();
	//	return;
	//}
	//创建虚拟硬盘文件
	if( copyVMImageFile() == false )
	{
		ackCode = "ACK:CRE VM:RETN=8,DESC=system internal error,prepare virtual disk error,please retry later";
		freeData();
		return;
	}
	

	
	log.write("end CreateVM::Do().", Log::INFO);
	bSuccess = true;
	ackCode = "ACK:CRE VM:RETN=0,DESC=success";
}


void* copyFileThreadFunc(void * arg)
{
	log.write("begin to copy a new template img file.", Log::INFO);
	fileNameStruct* pFile = (fileNameStruct*)arg;
	String destFile = pFile->changedFileName + ".bak";
	String srcFile = pFile->templateFileName + "0";
	String cmd = "cp " + srcFile + " " + destFile;
	system( cmd.c_str() );
	
	rename( destFile.c_str(), pFile->changedFileName.c_str() );
	delete pFile;
	return NULL;
}

//在创建虚拟机失败时，清除数据
void CreateVM::freeData()
{
	log.write("create vm failed,clear data.", Log::INFO);
	String sql = "delete from VMDetail where vmname='" + vmName + "'";
	executeDBSQL( sql );
	
	sql = "update MACIP set vmid=0 where mac='" + mac + "'";
	executeDBSQL( sql );
	String imgFileName = vmName + ".img";

	//将改名的文件改回来
	//rename( imgFileName.c_str(), changedTemplateFileName.c_str());

	String cfgFile = localNFSDir + "/" + vmName;
	remove( cfgFile.c_str() ); //删除配置文件
	applySC2FreeDisk();
}


bool CreateVM::createVMCfgFile()
{
	log.write( "begin create new vm cfg file .", Log::INFO );
	char dirName[1024];
	memset(dirName, 0, sizeof(dirName));
	strcpy(dirName, nfsDir.c_str());
	for(int i=0; i<strlen(dirName); i++)
	{
		if( dirName[i] == '/' )
			dirName[i] = '-';
	}
	String temp = dirName + 1;
	temp = nfsIP + "_" + temp;
	
	String dir = getCurrentDir() + "/nfs/" + temp;
	localNFSDir = dir;
	
	
	String templateCfgFile = localNFSDir + "/" + templateImageFileName + ".cfg";
	String destCfgFile = localNFSDir + "/" + vmName;
	String logContent;
	String macAddr;
	
	if( isFileExist(templateCfgFile) == false)
	{
		logContent = "template vm cfg file not exist.";
		log.write(logContent, Log::ERROR);
		return false;
	}

	char buff[1024];
	memset(buff, 0, sizeof(buff));
	sprintf(buff,"vif = [ \"mac=%s,bridge=xenbr0,script=vif-bridge\" ]", mac.c_str());
	macAddr = buff;
	
	ifstream infile;
	ofstream outfile;
	char fileBuff[1024];
	String line;
	infile.open(templateCfgFile.c_str(), ios::in);

	if( infile.is_open() == false)
	{
		logContent = "open template vm cfg file " + templateCfgFile + " for read error.";
		log.write(logContent, Log::ERROR);
		return false;
	}
	
	vector<String> lines;
	String tmp;
	while(!infile.eof())
	{
		memset(fileBuff, 0 ,sizeof(fileBuff));		
		infile.getline(fileBuff, sizeof(fileBuff), '\n');
		String tmp = fileBuff;
		if( strncmp(fileBuff, "vif", 3) == 0 )
		{			
			lines.push_back( macAddr );
			continue;
		}
		else if( strncmp(fileBuff, "name", 4) == 0 )
		{		
			continue;
		}
		else if( strncmp(fileBuff, "maxmem", 6) == 0 )
		{		
			
			continue;
		}
		else if( strncmp(fileBuff, "memory", 6) == 0 )
		{			
			
			continue;
		}
		else if( strncmp(fileBuff, "vcpus", 5) == 0 )
		{			
			continue;
		}
		lines.push_back(tmp);
	}
	infile.close();

	outfile.open(destCfgFile.c_str(), ios::out);
	if( outfile.is_open() == false)
	{
		logContent = "open vm cfg file " + destCfgFile + " for write error.";
		log.write(logContent, Log::WARN);
		return false;
	}

	int maxMem = atoi(ramSize.c_str());
	maxMem = maxMem;
	outfile<<"name = \""<<vmName<<"\""<<endl;
	outfile<<"maxmem = "<<maxMem<<endl;
	outfile<<"memory = "<<ramSize<<endl;
	outfile<<"vcpus = "<<cpuCnt<<endl;
	for(int i=0; i < lines.size(); i++)
	{
		outfile<<lines[i]<<endl;		
	}	
	
	outfile.close();
	logContent = " modify the cfg file of vm " + vmName + " success.";
	log.write(logContent, Log::INFO);
	
	log.write( "end create new vm cfg file .", Log::INFO );
	return true;
}

bool CreateVM::addVM2DB()
{
	log.write( "begin insert new vm data to db.", Log::INFO );
	String logContent;

	///////////
	CppMySQLQuery query ;
	String sql = "select DiskSize,FileName,RamSize,CPU,OS,IsParaVirt from VMTemplate where id=" + tid;
	
	if( executeDBQuery( sql, query) == false )
		return false;
	

	int rowCnt = query.numRow();

	if( rowCnt != 1 ) 
	{
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		return false;
	}

	String isParaVirt = query.getStringField("IsParaVirt");

	diskSize = query.getStringField("DiskSize");
	templateImageFileName = query.getStringField("FileName");
	//ramSize = query.getStringField("RamSize");
	//cpuCnt = query.getStringField("CPU");
	osDesc = query.getStringField( "OS" );


	////////////////
	sql = "insert into VMDetail (VMNAME,OS,DISK,RAM,VCPU,USERID,IsParaVirt,VirtType,uuid) values('";
	sql += vmName + "','" + osDesc + "'," + diskSize + "," + ramSize;
	sql += ",'" + cpuCnt + "'," + userID + "," + isParaVirt + ",'" + virtType + "',UUID())";

	if( executeDBSQL( sql ) == false )
	{
		log.write("execute " + sql + " failed.", Log::ERROR);
		return false;
	}	
	

	//CppMySQLQuery query ;
	sql = "select id,uuid from VMDetail where VMName='" + vmName + "' and userid=" + userID;
	if( executeDBQuery( sql, query ) == false )
	{
		log.write("execute " + sql + " failed.", Log::ERROR);
		return false;
	}

	vmID = query.getStringField("id");
	vmUUID = query.getStringField("uuid");

	sql = "update MACIP set vmid=" + vmID;
	sql += " where mac='" + mac + "'";
	if( executeDBSQL( sql ) == false )
	{
		log.write("execute " + sql + " failed.", Log::ERROR);
		return false;
	}
	log.write( "end insert new vm data to db.", Log::INFO );
	return true;

}

bool CreateVM::isSuccess()
{
	return bSuccess;
}

bool CreateVM::applySC2FreeDisk()
{
	String request = "DEL NFS STOR:ID=" + vmID + ",TYPE=0";
	int sock = connect2Server( scServerIP.c_str(), scServerPort);
	if( sock == -1 )
	{
		log.write("connect sc node server error.", Log::ERROR);
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to sc node server error.", Log::ERROR);
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from sc node server error.", Log::ERROR);
		return false;
	}
	log.write("recv ack message:" + ack + " from sc node server.", Log::INFO);

	
	//分析结果
	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para ip error from ack message.", Log::ERROR);
		return false;
	}
	closeSock( sock );
	
	if( iRetn == 0 )
	{
		log.write("apply sc server to free disk success.", Log::INFO);
		return true;
	}
	

	log.write("apply sc server to free disk failed.", Log::ERROR);
	return false;
}


bool CreateVM::getUserIDByName()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select id from User where user='" + userName + "'";

	if( executeDBQuery( sql, query) == false )
		return false;	
	
	int rowCnt = query.numRow();
	
	if( rowCnt != 1 )
	{
		
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		return false;
	}

	userID = query.getStringField("id");

	
	return true;

}
bool CreateVM::copyVMImageFile()
{
	//挂载NFS
	//if( mountNFS() != 0 )
		//return false;
	
	//复制文件
	String templateName = localNFSDir + "/" + templateImageFileName;
	String tmp;
	char buff[10];
	String destFileName = localNFSDir + "/" + vmUUID + ".img";
	
	for(int i=1; i <= g_templateCnt; i++)
	{		
		memset(buff, 0, sizeof( buff ));
		sprintf(buff, "%d", i);
		tmp = templateName + buff;
		if( isFileExist( tmp ) )
		{
			if( rename(tmp.c_str(), destFileName.c_str()) == 0 )
			{
				changedTemplateFileName = tmp;
				String resFileName = changedTemplateFileName + ".res";				
				return true;
			}
		}
	}
	
	return false;
}


bool CreateVM::getFreeMac()
{
	
	log.write( "begin get a free mac/ip address for new vm .", Log::INFO );
	CppMySQLQuery query ;
	String logContent;

	//先锁MACIP表
	String sql = "lock table MACIP write;";
	if( executeDBSQL( sql ) != 0 )
	{
		log.write("lock table MACIP failed.", Log::ERROR);
		return false;
	}
	

	sql = "select mac from MACIP where vmid=0 LIMIT 0,1";

	if( executeDBQuery( sql, query) == false )
	{
		sql = "UNLOCK TABLE ;";
		executeDBSQL( sql );
		return false;
	}
	
	int rowCnt = query.numRow();
	
	if( rowCnt != 1 )
	{
		sql = "UNLOCK TABLE ;";
		executeDBSQL( sql );
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		return false;
	}

	mac = query.getStringField("mac");

	sql = "update MACIP set vmid=-1 where mac='" + mac + "'";
	executeDBSQL( sql );

	//解锁MACIP表
	sql = "UNLOCK TABLE ;";
	executeDBSQL( sql );
	
	log.write( "end get a free mac/ip address for new vm .", Log::INFO );	
	return true;
}



int CreateVM::mountNFS()
{
	String logContent;
	char dirName[1024];
	memset(dirName, 0, sizeof(dirName));
	strcpy(dirName, nfsDir.c_str());
	for(int i=0; i<strlen(dirName); i++)
	{
		if( dirName[i] == '/' )
			dirName[i] = '-';
	}
	String temp = dirName + 1;
	temp = nfsIP + "_" + temp;
	
	String dir = getCurrentDir() + "/nfs/" + temp;
	localNFSDir = dir;
	if( !isFileExist(dir) )
	{		
		String mkdir = "mkdir -p " + dir;
		system(mkdir.c_str());
		if( !isFileExist(dir) )
		{
			log.write("create dir " + dir + " error.", Log::ERROR);
			return false;
		}
	}
	
	/////////	
	
	String mountPoint = "mountpoint " + dir;
	bool bMounted = false;
	int iRet = system( mountPoint.c_str());
	if( iRet == 0 )
		return 0;	
	
	
	
	String mountSrc = nfsIP+ ":" + nfsDir;
	logContent = "begin to mount " + mountSrc + " to " + dir;
	log.write(logContent, Log::INFO);
	if( mount(mountSrc, dir) == false )
	{
		logContent = "mount " + mountSrc + " to " + dir + " failed.";
		log.write(logContent, Log::ERROR);		
		return 1;
	}
	
	logContent = "mount " + mountSrc + " to " + dir + " success.";
	log.write(logContent, Log::INFO);
	return 0;
}


bool CreateVM::mount( String src, String dest)
{	
	
	String cmd = "mount " + src + " " + dest;
	
	int iRet = system( cmd.c_str() );

	return iRet == 0;
}


bool CreateVM::getNFSAddr()
{
	
	String logContent;	
	
	String request = "APPLY NFS STOR:VMID=" + vmID;
	request += ",CAPACITY=" + diskSize;
	request +=",TYPE=0";
	request +=",VMTID=" + tid;
	request += ",VirtType=" + virtType;
	
	if( apply2SC( request ) == false )
	{
		log.write(" apply capacity for vm error.", Log::ERROR);
		return false;
	}
	return true;
}



bool CreateVM::apply2SC(String request)
{
	int sock = connect2Server( scServerIP.c_str(), scServerPort);
	if( sock == -1 )
	{
		log.write("connect sc node server error.", Log::ERROR);
		return false;
	}
	
	
	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to sc node server error.", Log::ERROR);
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from sc node server error.", Log::ERROR);
		return false;
	}
	log.write("recv ack message:" + ack + " from sc node server.", Log::INFO);
	//分析结果，得出IP及DIR
	if( getStrParaValue( ack, "ip", nfsIP) == false )
	{
		log.write("get para ip error from ack message.", Log::ERROR);
		return false;
	}

	if( getStrParaValue( ack, "dirname", nfsDir) == false )
	{
		log.write("get para dir error from ack message.", Log::ERROR);
		return false;
	}

	closeSock( sock );
	return true;
}


bool CreateVM::getPara()
{	
	processPara();

	if( paraMap.size() != 5 )
		return false;
	
	map<String,String>::iterator it;

	
	it = paraMap.find("vmname");
	if( it == paraMap.end() )		
		return false;
	

	vmName = it->second;
	
	if( vmName.size() == 0 )
		return false;	

	it = paraMap.find("tid");
	if( it == paraMap.end() )
		return false;
	

	tid = it->second;
	
	if( tid.size() == 0 )
		return false;

	//获取内存大小
	it = paraMap.find("ram");
	if( it == paraMap.end() )		
		return false;

	ramSize = it->second;
	if( tid.size() == 0 )
		return false;

	//获取CPU个数
	it = paraMap.find("cpu");
	if( it == paraMap.end() )		
		return false;

	cpuCnt = it->second;
	if( tid.size() == 0 )
		return false;

	//获取虚拟化类型
	it = paraMap.find("virttype");
	if( it == paraMap.end() )		
		return false;

	virtType = it->second;
	virtType.toLower();
	if( virtType.size() == 0 )
		return false;

	
	
	return true;
}


//判断同一用户下是否存在同名的虚拟机
bool CreateVM::isVMExist()
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select id from VMDetail where vmname='";
	sql += vmName+ "' and userid=" + userID;

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();

	if( rowCnt >= 1 ) 
		return true;
	return false;
	
}


bool CreateVM::checkUserVMCnt()
{
	
	CppMySQLQuery query ;
	String logContent;

	String sql = "SELECT COUNT(*) AS VMCNT FROM VMDetail where userid=";
	sql += userID;

	if( executeDBQuery( sql, query) == false )
		return false;
	
	int rowCnt = query.numRow();
	
	if( rowCnt != 1 )
	{
		
		logContent = "execute " + sql + ",the result row count is not 1 " + " ,we desire 1 row.";
		log.write( logContent, Log::ERROR);
		return false;
	}

	int vmCnt = query.getIntField("VMCNT");
	
	if( vmCnt >= UserMaxVM )
		return false;
	
	return true;

	
}
void CreateVM::setUserName(String user)
{
	userName = user;
}

