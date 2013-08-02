#include<iostream>
#include<cstdio>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "../common/CppMysql.h"
#include "../common/Log.h"
#include"SCnode.h"
#include<stdlib.h>
#include"CMDreceiver.h"
#include"CMDsender.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include"../common/Config.h"
using namespace std;
extern Log log;
SCnode::SCnode()
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
int SCnode::applyforspace(int ID,int applysize,bool isdisk,int VMID)
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
     CppMySQLQuery exec1;
     CppMySQLQuery exec2;





	 String sql1="update SCSP set Remain=Remain-"+String(applysize)+" where ID="+String(ID);



	 try
			 		{
		               db.execSQL( sql1.c_str() );
			 		}
			 	catch(const char* sErr1)
			 		{
			 			logContent = "execute " + sql1 + " failed:" + sErr1;
			 			log.write( logContent, Log::ERROR);
			 			return -1;
			 		//	query.nextRow();
			 			//continue;
			 		}

				 String sql2="insert into  SCStorDetail (SID,Size,Isdisk,VMID) values("+String(ID)+","+String(applysize)+","+String(isdisk)+","+String(VMID)+")";


			 	try
			 				 		{
			 			             db.execSQL( sql2.c_str() );
			 				 		}
			 				 	catch(const char* sErr2)
			 				 		{
			 				 			logContent = "execute " + sql2 + " failed:" + sErr2;
			 				 			log.write( logContent, Log::ERROR);
			 				 			return -1;
			 				 		//	query.nextRow();
			 				 		//	continue;
			 				 		}
			 	                 //返回更新成功

return 0;

}
int SCnode::releasespace(int ID,bool type)
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
		 String sql1="select * from SCStorDetail where VMID="+String(ID);
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
		 	int rowCnt = query1.numRow();

		 	     if( rowCnt == 0 )
		 			{
		 			//返回没有内容
		 	    	 return 1;
		 			}
     int SID=query1.getIntField("SID");
		 	    int Size = query1.getIntField("Size");
     CppMySQLQuery exec1;
     CppMySQLQuery exec2;
	 String sql2="update SCSP set Remain=Remain+"+String(Size)+" where ID=" +String(SID);
	 String sql3="delete from SCStorDetail where VMID="+String(ID);
	 try
			 		{
		               db.execSQL( sql2.c_str() );

			 		}
			 	catch(const char* sErr2)
			 		{
			 			logContent = "execute " + sql2 + " failed:" + sErr2;
			 			log.write( logContent, Log::ERROR);
			 			return -1;
			 		//	query.nextRow();
			 		//	continue;
			 		}
                 //返回更新成功
			   try	{
			 		db.execSQL( sql3.c_str() );
			 	}
			 catch(const char* sErr3)
			{
			 		logContent = "execute " + sql3 + " failed:" + sErr3;
			 		log.write( logContent, Log::ERROR);
			 		return -1;
			 		//	query.nextRow();
			 	//	continue;
			 		}
			 	                 //返回更新成功

return 0;

}

int SCnode::redirect(String IP1,String Dir1,String IP2,String Dir2)
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
		String sql1="select * from SCSP where IP='"+IP1+"'and Dir='"+Dir1+"'";
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
	    CppMySQLQuery query2;
		String sql2="select * from SCSP where IP='"+IP2+"'and Dir='"+Dir2+"'";
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

	 	int rowCnt1 = query1.numRow();
	 	int rowCnt2 = query2.numRow();



	 	     if( rowCnt1 == 0||rowCnt2 ==0)
	 			{
	 			//返回源或者目的节点不存在
	 	    	 return 1;
	 			}

	 		int APPSPACE=query1.getIntField("Capacity")-query1.getIntField("Remain");
	 		int Remain=query2.getIntField("Remain");
	 		if(APPSPACE>=Remain)
	 			//返回无法转存
	 			return 2;

	 		int ID1=query1.getIntField("ID");
	 		//cout<<"ID1="<<ID1<<endl;
	 		int ID2=query2.getIntField("ID");
	 		//cout<<"ID2="<<ID2<<endl;

	 	    CppMySQLQuery query3;
	 	  		String sql3="select * from SCStorDetail where SID="+String(ID1);
	 	  		try
	 	  			{
	 	  				query3 = db.querySQL( sql3.c_str() );
	 	  			}
	 	  		catch(const char* sErr3)
	 	  			{
	 	  				logContent = "execute " + sql3 + " failed:" + sErr3;
	 	  				log.write( logContent, Log::ERROR);
	 	  				return -1;
	 	  			//	continue;
	 	  			}
	 	  		int rowCnt3 = query3.numRow();



	 	  			  SCnode a;
	 	  for(int i=0;i<rowCnt3;i++)
	 	  {
   		   int Size = query3.getIntField("Size");
   		    int VMID=query3.getIntField("VMID");
   		  	bool ISDisk = query3.getIntField("Isdisk");

   		//  	cout<<ID1<<" "<<Size<<" "<<ISDisk<<" "<<VMID<<endl;
   		  	query3.nextRow();
   		 a.releasespace(VMID,ISDisk);
   		 a.applyforspace(ID2,Size,ISDisk,VMID);
 }

	  return 0;
}

