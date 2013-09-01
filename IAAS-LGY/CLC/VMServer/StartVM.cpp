#include "StartVM.h"
#include <time.h> 
#include <vector>
#include<string>

extern Log log;
extern String g_dispatchServerIP;
extern int g_dispatchServerPort;
extern int g_ncVMServerPort;
StartVM::StartVM()
{
	strCmd = "";
	bestNCIP = "";
	vmID = "";
	nfsIP = "";
	nfsDir = "";
	groupStartFlag=false;
	
}

StartVM::~StartVM()
{

}

void StartVM::Do()
{
	String logContent;

	log.write("begin StartVM::Do().", Log::INFO);	
	
	//参数检查
	log.write("begin process parameter.", Log::INFO);
	if( getPara() == false)
	{
		ackCode =  "ACK:START VM:RETN=4,DESC=parameters error";
		return;
	}
	log.write("end process parameter.", Log::INFO);

	//按组启动虚拟机
	if(groupStartFlag)
	{
		if(groupVMStart() == true)
		{
			ackCode =  "ACK:START VM:RETN=0,DESC=Group Start Success!";
		}
		else
		{
			ackCode +=  "Group Start Failed!";
		}
	}
	else
	{
		String VMID = vmID;
		if(singleVMStart(VMID) == true)
		{
			ackCode =  "ACK:START VM:RETN=0,DESC=Single VM Start Success!";
		}
		else
		{
			return;
		}
	}
	cout<<ackCode<<endl;
	log.write("end StartVM::Do().", Log::INFO);
}

bool StartVM::singleVMStart(String VMID)
{
	//检测虚拟机是否存在
	if( isVMExist(VMID) == false)
	{
		ackCode =  "ACK:START VM:RETN=1001,DESC=vm is not exist";
		return false;
	}

	// check whether user can access this vm(id)
	if( checkRight( VMID ) == false )
	{		
		ackCode =  "ACK:START VM:RETN=14,DESC=cannot access this vm";
		return false;
	}

	if( isVMStarted(VMID) == true )
	{
		ackCode =  "ACK:START VM:RETN=1001,DESC=vm is already started";
		return false;
	}

	//find the nc to apply this VM
	// and try to start it.
	if( applyNC() == false )
	{
		ackCode = "ACK:START VM:RETN=8,DESC=system is busy,please wait";
		return false;
	}

	//获取虚拟机所在的NFS
	/*if( getNFSInfo() == false )
	{
		log.write("get the nfs info failed of the vm.", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=system internal error";		
		return;
	}*/
	return true;
}

bool StartVM::checkVMSatus(String VMID)//检查VM状态是否合法
{
	//检测虚拟机是否存在
	if( isVMExist(VMID) == false)
	{
		ackCode =  "ACK:START VM:RETN=1001,DESC=vm is not exist";
		return false;
	}

	// check whether user can access this vm(id)
	if( checkRight( VMID ) == false )
	{
		ackCode =  "ACK:START VM:RETN=14,DESC=cannot access this vm";
		return false;
	}

	if( isVMStarted(VMID) == true )
	{
		ackCode =  "ACK:START VM:RETN=1001,DESC=vm is already started";
		return false;
	}
	return true;
}


bool StartVM::groupVMStart()
{
	//获取group中的VMID
	//****为了不改变原有系统结构，这里直接更新vmid这个类变量***
	String sql = "select ID,UUID from VMDetail where GroupID=" + groupID;
	CppMySQLQuery query ;

	if( executeDBQuery( sql, query) == false )
	{
		log.write("Execute SQL: " + sql + " failed .", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=Error In Query Database, cannot use group start! ";
		return false;
	}
	String vmIDList="";
	while( !query.eof() )
	{
		vmID = query.getStringField( "ID" );
		//只要组中有一台虚拟机状态检测不通过，则不能通过“组”方式启动虚拟机
		if(checkVMSatus(vmID)==false)
		{
			ackCode = "ACK:START VM:RETN=8,DESC=VM "+vmID+" Status Error,cannot use group start! ";
			log.write("Group Start: check VM status Error.", Log::ERROR);
			return false;
		}
		query.nextRow();
	}
	//状态检查通过，则通过“组”方式，选择NC
	if( groupApplyNC(groupID) == false )
	{
		log.write("Group Apply NC Failed! ", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=Group Apply NC Failed! please try later!";
		return false;
	}

	return true;
//	if( startVM(VMID) == false )
//	{
//		log.write("start vm " + vmID + " failed .", Log::ERROR);
//		ackCode = "ACK:START VM:RETN=8,DESC=start VM failed";
//		return false;
//	}

}



bool StartVM::updateDB(String vm_id,String nc_ip)
{
	time_t t = time(0); 
    char tmp[64]; 
    strftime( tmp, sizeof(tmp), "%Y/%m/%d %X",localtime(&t) ); 
	String localTime = tmp;

	String sql = "update VMDetail set status=1,host='";
	sql += nc_ip + "' where id=" + vm_id;
	executeDBSQL( sql );

	if( executeDBSQL( sql ) != 1 )
	{
		log.write("update vm status to VMDetail table failed.", Log::ERROR);
		return false;
	}
	return true;

}

bool StartVM::getNFSInfo()
{	
	String sql = "select SID from SCStorDetail where vmid=" + vmID;
	CppMySQLQuery query ;
	

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();
	if( rowCnt != 1 ) 
		return false;
	
	String sid = query.getStringField("SID");

	sql = "SELECT SCSP.IP,SCSP.DIR FROM SCSP INNER JOIN ";
	sql += "SCStorDetail ON SCSP.ID=SCStorDetail.SID WHERE SCStorDetail.SID=" + sid;
	if( executeDBQuery( sql, query) == false )
		return false;	
	
	rowCnt = query.numRow();
	if( rowCnt < 1 ) 
		return false;

	nfsIP = query.getStringField("IP");
	nfsDir = query.getStringField("DIR");
	return true;
}

bool StartVM::startVM(String VMID,String NCIP)
{
	cout<<"Execute Start VM CMD, VMID = "<<VMID.c_str()<<" ;NCIP = "<< NCIP.c_str()<<endl;
	// for test!
	//	ackCode = "ACK:START VM:RETN=0,DESC=Emulator Success!";
	//	return true;
	String request = "START VM:VMID=" + VMID;

	int sock = connect2Server( NCIP.c_str(), g_ncVMServerPort);
	if( sock == -1 )
	{
		log.write("Connect NC VM Server error.", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=Connect NC VM server error";
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to nc vm server error.", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=Send request to NC VM server error";
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("Receive ack message from nc vm server error.", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=Receive ack message from NC VM server error.";
		return false;
	}
	log.write("recv ack message:" + ack + " from nc vm server.", Log::INFO);

	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=get para retn error from ack message.";
		return false;
	}

	closeSock( sock );


	ackCode = ack;
	//启动成功，则iRetn应为0
	return iRetn == 0;
	
}


bool StartVM::isVMExist(String VMID)
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select id from VMDetail where id=" + VMID;

	if( executeDBQuery( sql, query) == false )
		return false;

	int rowCnt = query.numRow();

	if( rowCnt >= 1 )		
		return true;
	
	return false;
	
}

bool StartVM::isVMStarted(String VMID)
{
	
	String logContent;	
	
	CppMySQLQuery query ;
	String sql = "select id from VMDetail where status=1 and id=" + VMID;

	if( executeDBQuery( sql, query) == false )
		return false;	
	

	int rowCnt = query.numRow();

	if( rowCnt >= 1 ) 
		return true;
	return false;
	
}



void StartVM::setUserName(String user)
{
	userName = user;
}
bool StartVM::getPara()
{
	processPara();

	if( paraMap.size() != 1 )
		return false;
	
	map<String,String>::iterator it;

	
	it = paraMap.find("groupid");
	if( it != paraMap.end() )
	{
		groupID = it->second;
		if( groupID.size() == 0 )
			return false;
		else
		{
			groupStartFlag = true;
			return true;
		}
	}



	it = paraMap.find("vmid");
	if( it == paraMap.end() )		
		return false;
	
	vmID = it->second;
	if( vmID.size() == 0 )
		return false;


	return true;
}


bool StartVM::applyNC()
{
	/*if( getVMIDByName() == false )
	{
		log.write("Get vmid from database failed.", Log::ERROR);
		return false;
	}*/

	String request = "GET BEST NC:VMID=" + vmID;

	int sock = connect2Server( g_dispatchServerIP.c_str(), g_dispatchServerPort);
	if( sock == -1 )
	{
		log.write("connect dispatch server error.", Log::ERROR);
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to dispatch server error.", Log::ERROR);
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from dispatch server error.", Log::ERROR);
		return false;
	}
	log.write("recv ack message:" + ack + " from dispatch server.", Log::INFO);
	//分析结果，得出IP
	String NCIP;
	cout<<"Receive from dispatch server, ack code = "<<ack.c_str()<<endl;
	if( getStrParaValue( ack, "ip", NCIP) == false )
	{
		log.write("get para ip error from ack message.", Log::ERROR);
		return false;
	}
	closeSock( sock );
	
	//尝试启动VM
	bool flag = startVM(vmID,NCIP);
	//若启动失败
	if(flag == false)
	{
		log.write("start vm " + vmID + " failed .", Log::ERROR);
		ackCode = "ACK:START VM:RETN=8,DESC=start VM failed";
		return false;
	}
	//启动成功，则更新数据库
	//若VM启动成功，则更新数据库
	updateDB(vmID,NCIP);
	ackCode = "ACK:START VM:RETN=0,DESC=success";
	return true;
}

bool StartVM::groupApplyNC(String GroupID)
{

	String request = "GET BEST NC:GroupID=" + GroupID;

	int sock = connect2Server( g_dispatchServerIP.c_str(), g_dispatchServerPort);
	if( sock == -1 )
	{
		log.write("connect dispatch server error.", Log::ERROR);
		return false;
	}

	if( sendData( request.c_str(), request.size(), sock) == false )
	{
		log.write("send request to dispatch server error.", Log::ERROR);
		return false;
	}
	String ack;

	if( recvData( sock, ack) == false )
	{
		log.write("recv ack message from dispatch server error.", Log::ERROR);
		return false;
	}
	closeSock( sock );
	log.write("recv ack message:" + ack + " from dispatch server.", Log::INFO);
	//分析结果，得出VMID/NCIP组
	//*********************待修改！！！*************************
	String vm_nc_map;
	if( getStrParaValue( ack, "vm_nc_map", vm_nc_map) == false )
	{
		log.write("get para ip error from ack message.", Log::ERROR);
		return false;
	}


	//分解格式为 vm_id1/nc_ip1|vm_id2/nc_ip2|...的字符串
	char para[4096],vm_id[128],nc_ip[128];
	const char * split = "|";
	char *temp,*p;
	strcpy(para,vm_nc_map.c_str() );
	temp=strtok (para,split);
	while(temp!=NULL)
	{
		//temp的格式为vm_id/nc_ip，因此需要将temp再次分解
		p = strstr(temp,"/");
		if(p==NULL)
		{
			log.write("get para error from ack message.", Log::ERROR);
			return false;
		}
		//截断字符串
		*p = '\0';
		strcpy(vm_id,temp);
		strcpy(nc_ip,(p+1));
		cout<<"try to start VM："<<vm_id<<" on NC: "<<nc_ip<<endl;
		bool flag = startVM(String(vm_id),String(nc_ip));

		if(flag == false)
		{
			log.write("Start VM Error from NC_VM's ack message.", Log::ERROR);
			return false;
		}
		else//启动成功
		{
			updateDB(String(vm_id),String(nc_ip));
		}
		temp=strtok (NULL,split);
	}

	return true;
}










