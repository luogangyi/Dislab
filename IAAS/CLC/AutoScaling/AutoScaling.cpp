#include "AutoScaling.h"
#include "VMMigrate.h"

extern Log log;
extern int clcServerPort;
extern String clcServerAddr;
extern String admin,passwd;
extern int ncPort;


int vmExpandCPULimit = 0;
int groupExpandCPULimit = 0;

AutoScaling::AutoScaling()
{

}



AutoScaling::~AutoScaling()
{

}


void AutoScaling::Do()
{

	//////////////从数据库中取得虚拟机扩展的上限

	String sql = "SELECT VMExpandCPULimit,GroupExpandCPULimit FROM GlobalCFG";
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return ;
	
	if( query.numRow() == 1 )
	{
		vmExpandCPULimit = query.getIntField( "VMExpandCPULimit" );
		groupExpandCPULimit = query.getIntField( "GroupExpandCPULimit" );
	}

	/////////////////


	sql = "UPDATE ServerArray SET STATUS=1 WHERE VMID IN (SELECT VMDetail.ID FROM VMDetail INNER JOIN OnlineVMS ON VMDetail.ID=OnlineVMS.VMID)";
	executeDBSQL( sql );

	sql = "UPDATE ServerArray SET STATUS=0 WHERE VMID NOT IN (SELECT VMDetail.ID FROM VMDetail INNER JOIN OnlineVMS ON VMDetail.ID=OnlineVMS.VMID)";
	executeDBSQL( sql );

	sql = "select ID,Name,MaxSize,MinSize,VMID,Step from ScalingGroup where Flag=1";

	//CppMySQLQuery query ;
	CppMySQLQuery query2 ;
	if( executeDBQuery( sql, query) == false )
		return ;
	

	String groupID,groupName,vmID;
	int maxSize,minSize,step;
	
	int activeVMCnt = 0;
	while( !query.eof() )
	{
		groupID = query.getStringField("ID");
		groupName = query.getStringField("Name");
		vmID = query.getStringField("VMID");
		maxSize = query.getIntField("MaxSize");
		minSize = query.getIntField("MinSize");
		step = query.getIntField("Step");


		sql = "SELECT COUNT(*) AS CNT FROM ServerArray WHERE ";
		sql += "GroupID=" + groupID + " AND STATUS=1";
		
		if( executeDBQuery( sql, query2) == false )
		{
			query.nextRow();
			continue;		
		}
		activeVMCnt = query2.getIntField( "CNT");


		if( activeVMCnt == 0 && minSize == 0 )
		{
			log.write( "group " + groupID + "(" + groupName + ") MinSize and active vm are 0.", Log::INFO);
			query.nextRow();
			continue;
		}
		if( activeVMCnt < minSize )
		{
			log.write( "group " + groupID + "(" + groupName + ") activeVMCnt<MinSize.", Log::INFO);
			processStartPolicy(groupID, maxSize, minSize, step, vmID, activeVMCnt);
			query.nextRow();
			continue;
		}

		//begin:对虚拟机CPU进行扩展，扩展时不启动新虚拟机
		if( isGroupOverLoad( groupID ) && isGroupCanExpand( groupID ))
		{
			log.write("group " + groupName + " is overload,and can be expand",Log::INFO);
			expandGroup( groupID );
			query.nextRow();
			continue;
		}

		/////////////

		
		if( isTriggerStartPolicy( groupID, activeVMCnt ) )
		{
			log.write( "group " + groupID + "(" + groupName + ") trigger START policy.", Log::INFO);
			processStartPolicy(groupID, maxSize, minSize, step, vmID, activeVMCnt);
		}
		else if( activeVMCnt != 0 && isTriggerClosePolicy( groupID, activeVMCnt ) )
		{
			log.write( "group " + groupID + "(" + groupName + ") trigger CLOSE policy.", Log::INFO);
			processClosePolicy( groupID, maxSize, minSize, step, vmID, activeVMCnt);
		}
		else
			log.write( "group " + groupID + "(" + groupName + ") performance is normal.", Log::INFO);
		
		query.nextRow();
	}


	//处理不在组内的虚拟机的扩展
	processSingleVMExpand();

}

//返回值：
//0--OK
//1--system error
//2--未触发Start策略
//参数：
//groupID--组ID
//maxSize--组内最大虚拟机个数
//minSize--组内最小虚拟机个数
//step每次启动关闭虚拟机个数
int AutoScaling::processStartPolicy(String groupID, int maxSize, int minSize, int step, String vmID, int activeVMCnt)
{	
	
	if( activeVMCnt == maxSize )
	{
		log.write("AutoScaling group online vms reach the MaxSize.", Log::INFO);
		return 0;
	}

	//启动一个组内虚拟机
	String sql = "SELECT ID FROM VMDetail WHERE ID=";
	sql += "(SELECT VMID FROM ServerArray WHERE GroupID=";	
	sql += groupID + " AND STATUS=0 ORDER BY ID  LIMIT 0,1)";
	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return 1;

	if( query.numRow() != 1 )
		return 1;

	String beStartedID = query.getStringField( "ID" );
	//String vmName = query.getStringField( "vmname" );

	if( startVM( beStartedID ) )
	{
		sql = "update ServerArray set Status=1,CPUFlag=0,CPUValue=0,OPTime=0 where VMID=" + beStartedID;
		executeDBSQL( sql );

		//更新AutoScalingVMOP表
		sql = "select ID from AutoScalingVMOP where VMID=" + beStartedID;
		executeDBQuery( sql, query);
		//cout<<"num:"<<query.numRow()<<endl;
		if( query.numRow() == 1 )
			sql = "delete from AutoScalingVMOP where VMID=" + beStartedID;
		else
		{
			sql = "insert into AutoScalingVMOP (VMID,MotherVMID,OP) values (";
			sql += beStartedID + "," + vmID + ",1)";
		}
		executeDBSQL( sql );
		

	}
	
	return 0;
}


//Function:判断是否触发了Start策略
bool AutoScaling::isTriggerStartPolicy(String groupID, int activeVMCnt)
{

	String sql = "select vmid,cpuflag from ServerArray where status=1 and GroupID=" + groupID;

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() == 0 )
		return false;

	String vmid;
	int cpuflag;
	while( !query.eof() )
	{
		vmid = query.getStringField( "vmid" );
		cpuflag = query.getIntField( "cpuflag" );
		if( isVMOverLoad( vmid ) == false )
			return false;
		
		query.nextRow();		
	}

	return true;

/*
	String sql = "SELECT AverageCPU,Duration FROM ScalingPolicy WHERE TYPE=1 AND GroupID=";
	sql += groupID;
	
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 )
	{
		log.write("AutoScaling Group Policy Error:no start policy", Log::ERROR);
		return false;
	}

	int avgCPU;
	int duration;
	avgCPU = query.getIntField( "AverageCPU" );
	duration = query.getIntField( "Duration" );

	
	int lineCnt = duration*activeVMCnt;
	

	////////////////////////////////////////
	char cntBuf[10];
	memset(cntBuf, 0, sizeof(cntBuf));
	sprintf(cntBuf,"%d", lineCnt);


	sql = "SELECT CPU FROM VMPerfData WHERE vmname IN ";
	sql += "(SELECT VMName FROM VMDetail INNER JOIN ServerArray ON VMDetail.ID=ServerArray.VMID ";
	sql += " WHERE STATUS=1 AND GroupID=";
	sql += groupID + ") ORDER BY TIME DESC LIMIT 0,";
	sql += cntBuf;

	

	if( executeDBQuery( sql, query) == false )
		throw "execute db query error";

	if( query.numRow() < lineCnt )
		return false;

	float sum=0.0;
	
	while( !query.eof() )
	{
		sum += query.getFloatField( "CPU" );
		
		query.nextRow();		
	}
	
	cout<<sum<<" "<<sum/lineCnt<<endl;
	if( sum/lineCnt > avgCPU )
		return true;

	return false;

*/
		
}



bool AutoScaling::isVMOverLoad(String vmid)
{
	String sql = "SELECT UNIX_TIMESTAMP(starttime) AS SEC FROM OnlineVMS INNER JOIN VMDetail ON OnlineVMS.VMName=VMDetail.VMNAME WHERE id=" + vmid;

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 )			
		return false;
	

	int startSec = query.getIntField( "SEC" );

	sql = "SELECT TIME FROM VMStatus WHERE cpustatus='high' AND vmid=" + vmid;

	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 )			
		return false;

	int reportSEC = query.getIntField( "TIME" );

	return reportSEC > startSec;
}

bool AutoScaling::startVM(String vmID)
{
	String request = "START VM:VMID=" + vmID;
	String ack;
	int iRetn;
	
	int sock = connect2Server( clcServerAddr.c_str(), clcServerPort);
	if( sock == -1 )
	{
		log.write("connect clc vm server error.", Log::ERROR);		
		return false;
	}

	//发送登陆消息
	String loginRequest = "LOGIN:USER=";
	loginRequest += admin + ",PASSWD=" + passwd;
	if( sendData2CLC( loginRequest.c_str(), loginRequest.size(), sock) == false )
	{
		log.write("send login request to clc vm server error.", Log::ERROR);		
		return false;
	}

	if( recvDataFromCLC( sock, ack) == false )
	{
		log.write("recv login ack message from clc vm server error.", Log::ERROR);		
		return false;
	}

	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("system user or passwd error,login failed.", Log::ERROR);
		
		return false;
	}	

	////////////////////发送启动虚拟机消息

	if( sendData2CLC( request.c_str(), request.size(), sock) == false )
	{
		log.write("send start vm request to clc vm server error.", Log::ERROR);		
		return false;
	}
	

	if( recvDataFromCLC( sock, ack) == false )
	{
		log.write("recv start vm ack message from clc vm server error.", Log::ERROR);		
		return false;
	}
	log.write("recv start vm ack message:" + ack + " from clc vm server.", Log::INFO);

	
	//分析结果
	
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);
		
		return false;
	}	
	
	closeSock( sock );
		
	
	return iRetn == 0;
}



//Function:判断是否触发了Close策略
bool AutoScaling::isTriggerClosePolicy(String groupID, int activeVMCnt)
{

	String sql = "select vmid from ServerArray where status=1 and GroupID=" + groupID;

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() == 0 )
		return false;

	String vmid;
	while( !query.eof() )
	{
		vmid = query.getStringField( "vmid" );

		if( isVMLowLoad( vmid ) == false)
			return false;
		
		query.nextRow();		
	}

	return true;

	/*String sql = "SELECT AverageCPU,Duration FROM ScalingPolicy WHERE TYPE=0 AND GroupID=";
	sql += groupID;
	
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 )
	{
		log.write("AutoScaling Group Policy Error:no close policy", Log::ERROR);
		return false;
	}

	int avgCPU;
	int duration;
	avgCPU = query.getIntField( "AverageCPU" );
	duration = query.getIntField( "Duration" );

	int lineCnt = duration*activeVMCnt;
	

	////////////////////////////////////////
	char cntBuf[10];
	memset(cntBuf, 0, sizeof(cntBuf));
	sprintf(cntBuf,"%d", lineCnt);


	sql = "SELECT CPU FROM VMPerfData WHERE vmname IN ";
	sql += "(SELECT VMName FROM VMDetail INNER JOIN ServerArray ON VMDetail.ID=ServerArray.VMID ";
	sql += " WHERE STATUS=1 AND GroupID=";
	sql += groupID + ") ORDER BY TIME DESC LIMIT 0,";	
	sql += cntBuf;

	

	if( executeDBQuery( sql, query) == false )
		throw "execute db query error";

	if( query.numRow() < lineCnt )
		return false;

	float sum=0.0;
	
	while( !query.eof() )
	{
		sum += query.getFloatField( "CPU" );
		
		query.nextRow();		
	}
	
	cout<<sum<<" "<<sum/lineCnt<<endl;
	if( sum/lineCnt <= avgCPU )
		return true;

	return false;

*/
		
}


bool AutoScaling::isVMLowLoad(String vmid)
{
	String sql = "SELECT UNIX_TIMESTAMP(starttime) AS SEC FROM OnlineVMS INNER JOIN VMDetail ON OnlineVMS.VMName=VMDetail.VMNAME WHERE id=" + vmid;

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 )			
		return false;
	

	int startSec = query.getIntField( "SEC" );

	sql = "SELECT TIME FROM VMStatus WHERE cpustatus='low' AND vmid=" + vmid;

	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 )			
		return false;

	int reportSEC = query.getIntField( "TIME" );

	return reportSEC > startSec;
}

//返回值：
//0--OK
//1--system error
//2--未触发Close策略
//参数：
//groupID--组ID
//maxSize--组内最大虚拟机个数
//minSize--组内最小虚拟机个数
//step每次启动关闭虚拟机个数
int AutoScaling::processClosePolicy(String groupID, int maxSize, int minSize, int step, String vmID, int activeVMCnt)
{
	
	if( activeVMCnt <= minSize )
	{

		log.write("AutoScaling group online vms reach the MinSize.", Log::INFO);
		return 0;	
	}

	//关闭一个组内虚拟机
	String sql = "SELECT ID,vmname FROM VMDetail WHERE ID=";
	sql += "(SELECT VMID FROM ServerArray WHERE GroupID=";	
	sql += groupID + " AND STATUS=1 ORDER BY ID DESC LIMIT 0,1)";
	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return 1;

	if( query.numRow() != 1 )
		return 1;

	String beClosedID = query.getStringField( "ID" );
	String vmName = query.getStringField( "vmname" );
	cout<<vmName<<" will be closed."<<endl;
	if( closeVM( beClosedID ) )
	{
		sql = "update ServerArray set Status=0 where VMID=" + beClosedID;
		executeDBSQL( sql );

		//更新AutoScalingVMOP表
		sql = "select ID from AutoScalingVMOP where VMID=" + beClosedID;
		executeDBQuery( sql, query);
		
		if( query.numRow() == 1 )
			sql = "delete from AutoScalingVMOP where VMID=" + beClosedID;
		else
		{
			sql = "insert into AutoScalingVMOP (VMID,MotherVMID,OP) values (";
			sql += beClosedID + "," + vmID + ",0)";
		}
		executeDBSQL( sql );
	}
	
	return 0;
}


bool AutoScaling::closeVM(String vmID)
{
	String request = "CLOSE VM:VMID=" + vmID;
	String ack;
	int iRetn;
	
	int sock = connect2Server( clcServerAddr.c_str(), clcServerPort);
	if( sock == -1 )
	{
		log.write("connect clc vm server error.", Log::ERROR);		
		return false;
	}

	//发送登陆消息
	String loginRequest = "LOGIN:USER=";
	loginRequest += admin + ",PASSWD=" + passwd;
	
	if( sendData2CLC( loginRequest.c_str(), loginRequest.size(), sock) == false )
	{
		log.write("send login request to clc vm server error.", Log::ERROR);		
		return false;
	}

	if( recvDataFromCLC( sock, ack) == false )
	{
		log.write("recv login ack message from clc vm server error.", Log::ERROR);		
		return false;
	}

	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("system user or passwd error,login failed.", Log::ERROR);
		
		return false;
	}	

	////////////////////发送关闭虚拟机消息

	if( sendData2CLC( request.c_str(), request.size(), sock) == false )
	{
		log.write("send close vm request to clc vm server error.", Log::ERROR);		
		return false;
	}
	

	if( recvDataFromCLC( sock, ack) == false )
	{
		log.write("recv close vm ack message from clc vm server error.", Log::ERROR);		
		return false;
	}
	log.write("recv close vm ack message:" + ack + " from clc vm server.", Log::INFO);

	
	//分析结果
	
	if( getIntParaValue( ack, "retn", iRetn) == false )
	{
		log.write("get para retn error from ack message.", Log::ERROR);
		
		return false;
	}	
	
	closeSock( sock );
		
	
	return iRetn == 0;
}


bool AutoScaling::isGroupOverLoad(String groupID)
{

	String sql = "select vmid,cpuflag from ServerArray where status=1 and GroupID=" + groupID;

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() == 0 )
		return false;

	String vmid;
	int cpuflag;
	while( !query.eof() )
	{
		vmid = query.getStringField( "vmid" );
		cpuflag = query.getIntField( "cpuflag" );
		if( isVMOverLoad( vmid ) == false )
			return false;
		
		query.nextRow();		
	}

	return true;
}


bool AutoScaling::isGroupCanExpand(String groupID)
{
	//String sql = "SELECT vmid FROM ServerArray WHERE (CPUFlag=0 OR CPUFlag=1) AND STATUS=1 AND groupid=";
	//sql += groupID;	

	//判断NC节点的CPU是否已全部使用完，如果已使用完则不能
	//扩展

	String sql = "SELECT ncip FROM OnlineVMS WHERE vmname IN ";
	sql += "(SELECT vmname FROM VMDetail INNER JOIN ServerArray ";
	sql += " ON VMDetail.ID=ServerArray.VMID  WHERE groupid=" + groupID + ")";

	CppMySQLQuery query,query2 ;
	if( executeDBQuery( sql, query) == false )
		return false;
	String ncip;
	int freeCPU;
	bool bFlag = false;
	while( !query.eof() )
	{
		ncip = query.getStringField( "ncip" );
		sql = "SELECT CPU-cpu_used AS free_cpu FROM onlineNC WHERE ip='";
		sql += ncip + "'";
		
		if( executeDBQuery( sql, query2) == false )
			return false;
		if( query2.numRow() != 1 )
			return false;

		freeCPU = query2.getIntField( "free_cpu" );
		
		if( freeCPU > 0 )
		{
			bFlag = true;
			break;
		}

		query.nextRow();
	}


	if( bFlag == false )
		return false;

	///判断组内的虚拟机是否已到扩展的上限
	sql = "SELECT VCPU FROM VMDetail WHERE id IN (SELECT VMID FROM ServerArray WHERE STATUS =1 AND groupid=";
	sql += groupID + ")";

	if( executeDBQuery( sql, query) == false )
		return false;
	int vcpu;
	while( !query.eof() )
	{
		vcpu = query.getIntField( "VCPU" );
		if( vcpu < groupExpandCPULimit )
			return true;
		query.nextRow();
	}

	return false;

	
}


bool AutoScaling::expandGroup(String groupID)
{
	String sql = "SELECT vmid FROM ServerArray WHERE STATUS=1 AND groupid=";
	sql += groupID;

	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	String vmid;

	while( !query.eof() )
	{
		vmid = query.getStringField( "vmid" );
		if( isGroupVMCanExpand( vmid ) == 0 )
			expandVM(vmid);
		query.nextRow();
	}

	return true;
}

bool AutoScaling::expandVM(String vmid)
{
	String sql = "SELECT uuid,host,vcpu FROM VMDetail WHERE status=1 and ID=" + vmid;
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() == 0 )
	{
		sql = "update ServerArray set Status=0,CPUFlag=0,CPUValue=0,OPTime=0 where VMID=" + vmid;
		executeDBSQL( sql );
		return false;
	}

	String ncip;

	ncip = query.getStringField( "host" );
	String uuid = query.getStringField( "uuid" );

	int vcpus = query.getIntField( "vcpu" );

	
	int sock = connect2Server( ncip.c_str(), ncPort);
	if( sock == -1 )
	{
		log.write("connect nc node error.", Log::ERROR);		
		return false;
	}

	String request = "Expand VM:uuid=" + uuid;

	if( sendData2NC( request.c_str(), request.size(), sock) == false )
	{
		log.write("send Expand VM request to nc node error.", Log::ERROR);		
		return false;
	}

	String ack;

	if( recvDataFromNC( sock, ack) == false )
	{
		log.write("recv Expand VM ack message from nc node error.", Log::ERROR);		
		return false;
	}

	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )		
		return false;
	
	
	return iRetn == 0;
	
}

bool AutoScaling::shrinkVM(String vmuuid, String vmid)
{
	String sql = "SELECT host,vcpu FROM VMDetail WHERE status=1 and uuid='";
	sql += vmuuid + "'";
	CppMySQLQuery query ;
	
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() == 0 )
	{
		sql = "update ServerArray set Status=0,CPUFlag=0,CPUValue=0,OPTime=0 where VMID=" + vmid;
		executeDBSQL( sql );
		return false;
	}

	String ncip;

	ncip = query.getStringField( "host" );
	

	int vcpus = query.getIntField( "vcpu" );

	
	int sock = connect2Server( ncip.c_str(), ncPort);
	if( sock == -1 )
	{
		log.write("connect nc node error.", Log::ERROR);		
		return false;
	}

	String request = "Expand VM:uuid=" + vmuuid;
	request += ",flag=1";

	if( sendData2NC( request.c_str(), request.size(), sock) == false )
	{
		log.write("send Expand VM request to nc node error.", Log::ERROR);		
		return false;
	}

	String ack;

	if( recvDataFromNC( sock, ack) == false )
	{
		log.write("recv Expand VM ack message from nc node error.", Log::ERROR);		
		return false;
	}

	int iRetn;
	if( getIntParaValue( ack, "retn", iRetn) == false )		
		return false;
	

	
	return iRetn == 0;
	
}

//不在组内的虚拟机的扩展
void AutoScaling::processSingleVMExpand()
{
	

	//收缩虚拟机
	String sql = "SELECT ID,uuid FROM VMDetail WHERE IsParaVirt=1 and IsAutoScaling=1";
	CppMySQLQuery query ;
	String vmid,uuid;
	
	if( executeDBQuery( sql, query) == false )
		return ;

	while( !query.eof() )
	{
		vmid = query.getStringField( "ID" );
		uuid = query.getStringField( "uuid" );
		
		if( isVMLowLoad( vmid ) && isVMCanShrink( uuid ))		
			shrinkVM( uuid, vmid );					

		query.nextRow();
	}
	sleep(2);
	//扩展虚拟机
	
	sql = "SELECT ID,VMName,VCPU FROM VMDetail WHERE IsParaVirt=1 and IsAutoScaling=1 AND ID NOT IN (SELECT VMId FROM ServerArray)";
	
	if( executeDBQuery( sql, query) == false )
		return ;

	int cpuCnt = 0;
	while( !query.eof() )
	{
		vmid = query.getStringField( "ID" );
		
		cpuCnt = query.getIntField( "VCPU" );

		if( vmExpandCPULimit != 0 && cpuCnt >= vmExpandCPULimit )
		{
			//cout<<"vm reach the vmExpandCPULimit"<<endl;
			query.nextRow();
			continue;
		}
		
		if( isVMOverLoad( vmid ) )
		{
			int iRet = isVMCanExpand( vmid );
			if( iRet == -1 )
			{
				query.nextRow();
				continue;
			}
			else if( iRet == 0 )
				expandVM( vmid );
			else
			{
				
				//cout<<"do vm migrate"<<endl;
				VMMigrate m;
				//m.init();
				//if( m.migrate( vmname ) )
				//	expandVM( vmid );
			}
			
		}
		

		query.nextRow();
	}	
	

}


//返回值：
// 0--可以扩展
//1--不可以扩展
int AutoScaling::isVMCanExpand(String vmID)
{
	
	
	//判断NC节点的CPU是否已全部使用完，如果已使用完则不能
	//扩展

	String sql = "SELECT ncip FROM OnlineVMS WHERE vmname=";
	sql += "(SELECT vmname FROM VMDetail WHERE id=" + vmID + ")";
	
	CppMySQLQuery query ;
	 
	if( executeDBQuery( sql, query) == false )
		return -1;


	if( query.numRow() != 1 )
		return -1;
	
	String ncip;
	ncip = query.getStringField( "ncip" );
	
	int freeCPU;
	
	
	sql = "SELECT CPU-cpu_used AS free_cpu FROM onlineNC WHERE ip='";
	sql += ncip + "'";
		
	if( executeDBQuery( sql, query) == false )
		return -1;
	if( query.numRow() != 1 )
		return -1;

	freeCPU = query.getIntField( "free_cpu" );
		
	if( freeCPU > 0 )
		return 0;
	return 1;


}



int AutoScaling::isGroupVMCanExpand(String vmID)
{
	
	
	//判断NC节点的CPU是否已全部使用完，如果已使用完则不能
	//扩展

	String sql = "SELECT VCPU,ncip FROM VMDetail INNER JOIN OnlineVMS ON VMDetail.VMNAME=OnlineVMS.VMName WHERE VMDetail.ID=";
	sql += vmID;
	
	CppMySQLQuery query ;
	 
	if( executeDBQuery( sql, query) == false )
		return -1;


	if( query.numRow() != 1 )
		return -1;

	int vcpu = query.getIntField( "VCPU" );
	
	if( groupExpandCPULimit != 0 && vcpu >= groupExpandCPULimit ) 
		return 1;
	
	String ncip;
	ncip = query.getStringField( "ncip" );
	
	int freeCPU;
	
	
	sql = "SELECT CPU-cpu_used AS free_cpu FROM onlineNC WHERE ip='";
	sql += ncip + "'";
	
	if( executeDBQuery( sql, query) == false )
		return -1;
	if( query.numRow() != 1 )
		return -1;

	freeCPU = query.getIntField( "free_cpu" );
	
	if( freeCPU > 0 )
		return 0;
	return 1;


}

bool AutoScaling::isVMCanShrink(String vmuuid)
{
	
	String sql = "select VCPU from VMDetail where uuid='" + vmuuid + "'";
	CppMySQLQuery query ;
	if( executeDBQuery( sql, query) == false )
		return false;

	if( query.numRow() != 1 ) 
		return false;

	int cnt = query.getIntField( "VCPU" );

	return cnt > 1;
}



