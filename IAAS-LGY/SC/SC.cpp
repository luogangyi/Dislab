#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/statvfs.h>
#include<iostream>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<cstdio>
#include<stdlib.h>
#include <sys/vfs.h>

#include"SC.h"
#include "../common/CppMysql.h"
#include "../common/Log.h"
#include "SCnode.h"
#include<ctime>
#include"CMDsender.h"
#include"CMDreceiver.h"
#include"../common/Config.h"
extern Log log;
extern String result1;
extern String result2;
extern String ip;
extern String dirname;
using namespace std;



 SC::SC()
{
	bool bRet1 = ConfigFile::getStrValue( "DBServerIP",dbServerIP);
	bool bRet2 = ConfigFile::getStrValue( "DBUserPass",dbUserPass );
	bool bRet3 = ConfigFile::getStrValue( "DBUserName", dbUserName );
	if(!bRet1||!bRet2||!bRet3)
	{
		cout<<"ini failed!";
		exit(-1);
	}


}

int SC::addnode(String IP,String Dir,int Capacity,int TID)
{
int Remain=10;


CppMySQL3DB db;
String logContent;

char buf[10]={0};
sprintf(buf,"%d",TID);

int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
if ( iRet != 0 )
{
	logContent = "open mysql db failed";
	log.write(logContent, Log::ERROR);
	return -1;
}
CppMySQLQuery query1;
String sql1="select * from SCSP where IP='"+IP+"'and Dir='"+Dir+"'and TID="+buf;
	try
		{
			query1 = db.querySQL( sql1.c_str() );
		}
	catch(const char* sErr1)
		{
			logContent = "execute " + sql1 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);

		//	continue;
		}
	int rowCnt1 = query1.numRow();

	//socket编程得出DiskRemain

    struct statfs diskStatfs;
    statfs("/", &diskStatfs);
   // unsigned long long totalBlocks = diskStatfs.f_blocks;
  //  unsigned long long totalSize = totalBlocks * diskStatfs.f_bsize;
   // cout << "Total Size: " << (totalSize >> 30) << " GB" << endl;
    unsigned long long freeBlocks = diskStatfs.f_bfree;
    unsigned long long freeSize = freeBlocks * diskStatfs.f_bsize;
    int DiskRemain=(freeSize >> 30);
  //  cout << "Free Size: " << (freeSize >> 30) << " GB" << endl;
   // cout <<"used ratio:"  <<100-((freeSize*100) /totalSize)<<"%"<<endl;
    if (rowCnt1!=0)
    {
    	for(int j=0;j<rowCnt1;j++)
    	{
    		int usedCapacity=query1.getIntField("Capacity");
       DiskRemain-=usedCapacity;
                query1.nextRow();
    	}
    }


	CMDsender cmdsender=CMDsender("114.212.83.222",10000);
	String cmd="add";//+Dir;
	//cmd+=" "+Capacity;
	char buff[64];
	strcpy(buff,cmdsender.sendCMD(cmd));


			    String sql2="insert into SCSP (IP,Dir,Capacity,Remain)values('"+IP+"','"+Dir+"',"+String(Capacity)+","+String(Remain)+")";
			    cout<<sql2<<endl;
			   try
			    		{
			    			db.querySQL( sql2.c_str() );
			    		}
			    	catch(const char* sErr2)
			    		{
			    			logContent = "execute " + sql2+ " failed:" + sErr2;
			    			log.write( logContent, Log::ERROR);
			    			return -1;
			    			//query2.nextRow();
			    			//continue;
			    		}




//Remain=-Capacity;

return 0;
}


int SC::deletenode(String IP,String Dir)
{
CppMySQL3DB db;
String logContent;
int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return -1;
	}
    CppMySQLQuery query1;
	String sql1="select * from SCSP where IP='"+IP+"'and Dir='"+Dir+"'";
	try
		{
			query1 = db.querySQL( sql1.c_str() );
		}
	catch(const char* sErr1)
		{
			logContent = "execute " + sql1 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);
			return -1;
		//	continue;
		}
	int rowCnt1 = query1.numRow();

		if( rowCnt1 != 1 )
		{
			char buf[10];
			logContent = "execute " + sql1 + ",the result row count is not 1 " + " ,we desire 1 row.";
			log.write( logContent, Log::ERROR);

			return 1;
		}
		String ID=query1.getStringField("ID");

		 CppMySQLQuery query2;
		 String sql2="select * from SCStorDetail where SID="+ID;

		 try
		 		{
		 			query2 = db.querySQL( sql2.c_str() );
		 		}
		 	catch(const char* sErr2)
		 		{

		 		}
		 	int rowCnt2 = query2.numRow();

		 	     if( rowCnt2 != 0 )
		 			{  //返回该存储点上存在内容，不允许删除的信息
		 				return 2;
		 			}
		 CppMySQLQuery exec;
		 String sql3="delete from SCSP where IP='"+IP+"'and Dir='"+Dir+"'";
		 try
				 		{
			               db.execSQL(sql3.c_str());
				 		}
				 	catch(const char* sErr3)
				 		{
				 			logContent = "execute " + sql3 + " failed:" + sErr3;
				 			log.write( logContent, Log::ERROR);
				 		//	query3.nextRow();
				 		// continue;
				 		}
                      //返回删除成功

return 0;
}


int SC::querynode()
{
	CppMySQL3DB db;
	String logContent;
	int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
	if ( iRet != 0 )
		{
			logContent = "open mysql db failed";
			log.write(logContent, Log::ERROR);
			return -1;
		}
	    CppMySQLQuery query1;
		String sql1="select * from SCSP";
		try
			{
				query1 = db.querySQL( sql1.c_str() );
			}
		catch(const char* sErr1)
			{
				logContent = "execute " + sql1 + " failed:" + sErr1;
				log.write( logContent, Log::ERROR);
				return -1;
			//	continue;
			}
		int rowCnt1 = query1.numRow();

			if( rowCnt1 == 0 )
			{
				char buf[10];
				logContent = "execute " + sql1 + ",the result row count is 0 ";
				log.write( logContent, Log::ERROR);
				//节点数目为0
	               return 0;
			}
String m[500];
SC a;
int i=0;
			while(i<rowCnt1-1)
		{
		String IP=query1.getStringField("IP");
		String Dir=query1.getStringField("Dir");
		 m[i]=IP+"|"+Dir;
		 cout<<m[i]<<" "<<endl;
		 result1+=m[i]+"&";
		 i++;
		query1.nextRow();
		}
			String IP=query1.getStringField("IP");
			String Dir=query1.getStringField("Dir");
			m[i]=IP+"|"+Dir;
			 result1+=m[i];


	//	cout<<"results:"<<result<<endl;
	//返回具体信息
    return 0;
}

int SC::querynodedetails(String IP,String Dir)

{CppMySQL3DB db;
String logContent;
int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return -1;
	}
    CppMySQLQuery query1;
	String sql1="select * from SCSP where IP='" +IP+"'and Dir='"+Dir+"'";

	try
		{
			query1 = db.querySQL( sql1.c_str() );
		}
	catch(const char* sErr1)
		{
			logContent = "execute " + sql1 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);
			return -1;
		//	continue;
		}
	int rowCnt1 = query1.numRow();

		if( rowCnt1 != 1 )
		{
			char buf[10];
			logContent = "execute " + sql1 + ",the result row count is not 1 " + " ,we desire 1 row.";
			log.write( logContent, Log::ERROR);
                //返回不存在该存储点的信息
			return 1;
		}
		 int ID=query1.getIntField("ID");

		 CppMySQLQuery query2;
		 String sql2="select * from SCStorDetail where SID="+String(ID);


		 try
		 		{
		 			query2 = db.querySQL( sql2.c_str() );
		 		}
		 	catch(const char* sErr2)
		 		{
		 			logContent = "execute " + sql2 + " failed:" + sErr2;
		 			log.write( logContent, Log::ERROR);
		 			return -1;
		 		//	continue;
		 		}
		 	int rowCnt2 = query2.numRow();
		 	     if( rowCnt2 == 0 )
		 			{
		 			//返回没有内容
		 	    	 return 1;
		 			}
		 	    String m[500];
		 	    int i=0;
		 	    			while(i<rowCnt2-1)
		 	    		{
		 	    		String SID = query2.getStringField("SID");
		 	    		String Size = query2.getStringField("Size");
		 	    		String VMID = query2.getStringField("VMID");
		 	    		String ISDisk = String(query2.getIntField("Isdisk"));
                           m[i]=SID+"|"+Size+"|"+VMID+"|"+ISDisk;
		 	    		result2+=m[i]+"&";
		 	    		 i++;
		 	    		query2.nextRow();
		 	    		}
		 	    			String SID = query2.getStringField("SID");
		 	    			String Size = query2.getStringField("Size");
		 	    			String VMID = query2.getStringField("VMID");
		 	    			String ISDisk = String(query2.getIntField("Isdisk"));
		 	    			m[i]=SID+"|"+Size+"|"+VMID+"|"+ISDisk;
		 	    			result2+=m[i];
		 	    	//返回具体信息
		 	        return 0;

}



int SC::applyforspace(int applysize,bool isdisk,int VMID,int VMTID,String VirtType)
{  CppMySQL3DB db;
String logContent;
int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return -1;
	}
    CppMySQLQuery query1;
	String sql1="select * from SCSP where VirtType='";
	sql1 += VirtType + "' order by Remain";
	
	try
		{
			query1 = db.querySQL( sql1.c_str() );
		}
	catch(const char* sErr1)
		{
			logContent = "execute " + sql1 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);
			return -1;
		//	continue;
		}
	int rowCnt1 = query1.numRow();
    int TID=query1.getIntField("TID");

	
        //rowCnt1为行数即返回节点数目
		if( rowCnt1 == 0 )
		{
			char buf[10];
			logContent = "execute " + sql1 + ",the result row count is 0 ";
			log.write( logContent, Log::ERROR);
			return 1;
			//节点数目为0
        }
		int i=0;

    while (i<rowCnt1)
    {    	
    	if(query1.getIntField("Remain")>applysize && VMTID==TID )
			break;
   		else
    	{
	    	i++;
	        query1.nextRow();
	        TID=query1.getIntField("TID");
	        
    	}
    }

	
if(i>rowCnt1)
	{
	logContent = "NOT support OS or NOT sufficient space";
	log.write( logContent, Log::ERROR);
	return 1;
//	continue;
}
int ID=query1.getIntField("ID");

if(ID==0) return 1;

	ip=query1.getStringField("IP");
	dirname=query1.getStringField("Dir");
	SCnode a;

   return a.applyforspace(ID,applysize,isdisk,VMID);
}


 int SC:: testnode(String IP,String Dir)
{


CppMySQL3DB db;
String logContent;
int iRet = db.open(dbServerIP.c_str(), dbUserName.c_str(), dbUserPass.c_str(), "cloud");
if ( iRet != 0 )
	{
		logContent = "open mysql db failed";
		log.write(logContent, Log::ERROR);
		return -1;
	}
    CppMySQLQuery query1;
	String sql1="select * from SCSP where IP='"+IP+"'and Dir='"+Dir+"'";
	try
		{
			query1 = db.querySQL( sql1.c_str() );
		}
	catch(const char* sErr1)
		{
			logContent = "execute " + sql1 + " failed:" + sErr1;
			log.write( logContent, Log::ERROR);
			return -1;
		//	continue;
		}
	int rowCnt1 = query1.numRow();

		if( rowCnt1 != 1 )
		{
			char buf[10];
			logContent = "execute " + sql1 + ",the result row count is not 1 " + " ,we desire 1 row.";
			log.write( logContent, Log::ERROR);
       //返回不存在该节点错误
			return 2;
		}

		//建立socket测试

        access(Dir.c_str(),06);
		CMDsender cmdsender=CMDsender("114.212.83.222",10000);
		String cmd="test"+Dir;
		char buff[64];
		strcpy(buff,cmdsender.sendCMD(cmd));
		return 0;

}

