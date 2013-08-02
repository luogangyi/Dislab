#include "StartVM.h"
#include <map>
#include <sys/mount.h>
#include <fstream>
#include <libvirt/libvirt.h>
#include <sys/file.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "XML.h"
extern Log log;
extern CppMySQL3DB db;


StartVM::StartVM(string cmd)
{
	strCmd = cmd;
	localVMPath = "";
	retCode = -1;	
	
	nfsIP = "";
	nfsDir = "";
	vmName = "";
	vmMac = "";
	vmIP = "";
	virtType = "";
	vmUUID = "";
}


StartVM::~StartVM()
{
	
}

bool StartVM::startVM()
{

	if( virtType == "xen" )
		return startXenVM();
	else if( virtType == "kvm" )
		return startKvmVM();

	return false;
}

bool StartVM::startXenVM()
{
	string dir = localVMPath;	
	
	string disk = dir + "/" + vmUUID + ".img";
	
	XML xml;	
	
	string headXML = xml.getXenHeaderXML( vmName, iRam, iCPU, vmUUID);
	
	string diskXML = xml.getXenDiskXML( disk );
	string netXML = xml.getXenNetworkXML( vmMac );

	string tailXML = xml.getXenTailXML();

	string domXML = headXML + diskXML + netXML + tailXML;

	
	virConnectPtr conn = NULL;
    virDomainPtr dom = NULL;

	
    conn = virConnectOpen("");
	
	if (conn == NULL)
	{		
		log.write("StartVM::isAlreadyStart() Failed to connect to hypervisor.", Log::ERROR);
		return false;
	}

	
	dom = virDomainDefineXML(conn, domXML.c_str());
	if( dom == NULL )
	{
    	virConnectClose(conn);
		return false;
	}

	if( virDomainCreateWithFlags(dom, 0) != 0 )
	{
		virDomainUndefine(dom);
    	virDomainFree(dom);
    	virConnectClose(conn);
		return false;
	}


	virDomainFree(dom);    
    virConnectClose(conn);
	
	return true;

}


bool StartVM::startKvmVM()
{
	string dir = localVMPath;	
	
	string disk = dir + "/" + vmUUID + ".img";
	
	XML xml;	
	
	string headXML = xml.getKvmHeaderXML( vmName, iRam, iCPU, vmUUID);
	
	string diskXML = xml.getKvmDiskXML( disk );
	string netXML = xml.getKvmNetworkXML( vmMac );

	string tailXML = xml.getKvmTailXML();

	string domXML = headXML + diskXML + netXML + tailXML;
	
	virConnectPtr conn = NULL;
    virDomainPtr dom = NULL;

	
    conn = virConnectOpen("");
	
	if (conn == NULL)
	{		
		log.write("StartVM::isAlreadyStart() Failed to connect to hypervisor.", Log::ERROR);
		return false;
	}

	
	dom = virDomainDefineXML(conn, domXML.c_str());
	if( dom == NULL )
	{
    	virConnectClose(conn);
		return false;
	}

	if( virDomainCreateWithFlags(dom, 0) != 0 )
	{
		virDomainUndefine(dom);
    	virDomainFree(dom);
    	virConnectClose(conn);
		return false;
	}


	virDomainFree(dom);    
    virConnectClose(conn);
	
	return true;

}
	


void StartVM::Do()
{
	log.write("in StartVM::Do", Log::INFO);

	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:START VM:RETN=4,DESC=parameters error";
		return;
	}

	if( getVMInfoFromDB() == false)
	{
		ackCode =  "ACK:START VM:RETN=8,DESC=system internal error";
		return;
	}
	
	//检查虚拟机是否已启动
	int iRet = isAlreadyStart();
	if( iRet == 1)
	{
		ackCode = "ACK:START VM:RETN=1002,DESC=vm is running";
		return;
	}
	else if( iRet == 2)
	{
		ackCode = "ACK:START VM:RETN=8,DESC=system internal error";
		return;
	}
	
	
	//挂载NFS
	iRet = mountNFS();
	if( iRet == 1)
	{
		ackCode =  "ACK:START VM:RETN=1002,DESC=mount NFS error";
		return;
	}

	
	iRet = startVM();
	if( iRet == false)
	{
		ackCode =  "ACK:START VM:RETN=1002,DESC=start vm failed";		
		return;
	}	
	
	ackCode =  "ACK:START VM:RETN=0,DESC=success";

}


//挂载NFS服务器上的目录
//返回值：0-成功1-失败
int StartVM::mountNFS()
{
	////////////
	//对传入的参数dir进行处理，将'/'替换成'-'
	char dirName[1024];
	memset(dirName, 0, sizeof(dirName));
	strcpy(dirName, nfsDir.c_str());
	for(int i=0; i<strlen(dirName); i++)
	{
		if( dirName[i] == '/' )
			dirName[i] = '-';
	}
	string temp = dirName + 1;
	
	/////////	

	string dir = getCurrentDir() + "/nfs/" + nfsIP + "/" + temp;
	localVMPath = dir;
	
	string mountPoint = "mountpoint " + dir;
	bool bMounted = false;
	int iRet = system( mountPoint.c_str());
	if( iRet == 0 )
		return 0;
	
	
	
	string logContent = "vm directory " + dir + " do not exist,first try to make it.";
	if( !isFileExist(dir) )
	{
		log.write(logContent.c_str(), Log::WARN);

		string mkdir = "mkdir -p " + dir;
		system(mkdir.c_str());
	}
	
	
	string mountSrc = nfsIP + ":" + nfsDir;
	logContent = "begin to mount " + mountSrc + " to " + dir;
	log.write(logContent.c_str(), Log::INFO);
	if( mount(mountSrc, dir) == false )
	{
		logContent = "mount " + mountSrc + " to " + dir + " failed.";
		log.write(logContent.c_str(), Log::ERROR);
		
		return 1;
	}
	logContent = "mount " + mountSrc + " to " + dir + " success.";
	log.write(logContent.c_str(), Log::INFO);
	return 0;
}

bool StartVM::mount( string src, string dest)
{
	
	string cmd = "mount " + src + " " + dest;
	
	int iRet = system( cmd.c_str() );

	return iRet == 0;
}

bool StartVM::umount( string dir)
{
	string str = "umount " + dir;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	FILE* fp = popen(str.c_str(),"r");

	fgets(buff, sizeof(buff), fp);
	
	
	pclose(fp);

	if( strlen(buff) == 0 )
		return true;
	
	buff[strlen(buff)] = '\0';
	buff[strlen(buff) - 1] = '\0';

	str = buff;
	
	return false;

}

bool StartVM::isFileExist( string file)
{
	//access(path,F_OK)
	fstream _file;
    _file.open(file.c_str(),ios::in);
    if(!_file)
    {
    	return false;
    }
    else
    {
    	_file.close();
		return true;
    }
	
}

string StartVM::getCurrentDir()
{
	char cwd[1024];
	memset(cwd, 0, sizeof(cwd));
	getcwd(cwd, sizeof(cwd));

	string pwd(cwd);
	return pwd;
}


bool StartVM::getPara()
{
	processPara();


	if( paraMap.size() != 1 )
		return false;
	
	map<string,string>::iterator it;

	//获取NFS服务器IP地址信息
	it = paraMap.find("vmid");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	vmID = it->second;
	
	
	return true;
}


bool StartVM::getVMInfoFromDB()
{
	
	string sql = "select SID from SCStorDetail where vmid=" + vmID;
	CppMySQLQuery query ;
	

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;
	
	string sid = query.getStringField("SID");

	sql = "SELECT SCSP.IP,SCSP.DIR FROM SCSP INNER JOIN ";
	sql += "SCStorDetail ON SCSP.ID=SCStorDetail.SID WHERE SCStorDetail.SID=" + sid;
	if( executeDBQuery( sql, query) == false )
		return false;	
	
	rowCnt = query.numRow();
	if( rowCnt < 1 ) 
		return false;

	nfsIP = query.getStringField("IP");
	nfsDir = query.getStringField("DIR");

	//获取虚拟机名称
	sql = "select vmname,ram,vcpu,virttype,uuid from VMDetail where id=" + vmID;
	if( executeDBQuery( sql, query) == false )
		return false;

	rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;

	vmName = query.getStringField("vmname");
	virtType = query.getStringField("virttype");
	
	iRam = query.getIntField( "ram" );
	iCPU = query.getIntField( "vcpu" );
	vmUUID = query.getStringField("uuid");


	//获取虚拟机网卡地址
	sql = "select mac,ip from MACIP where vmid=" + vmID;
	if( executeDBQuery( sql, query) == false )
		return false;

	rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;

	vmMac = query.getStringField("mac");
	vmIP = query.getStringField("ip");

	
	return true;
}



