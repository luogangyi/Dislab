#include<iostream>
#include<cstdio>

#include"CMDType.h"
#include"Object.h"
#include "../common/CppMysql.h"
#include "../common/Log.h"
#include"Object.h"
#include"SC.h"
#include"SCnode.h"
#include"stdlib.h"

extern String result1="";
extern String result2="";
extern String ip="";
extern String dirname="";
 CMDType::Type CMDType::processcmd(const char* cmd)
{
	String cmd1="ADD NFS SP:";
	String cmd2="RMV NFS SP:";
	String cmd3="APPLY NFS STOR:";
	String cmd4="DEL NFS STOR:";
	String cmd5="QRY NFS SP:";
	String cmd6="QRY SP DETAIL:";
	String cmd7="TEST NFS SP:";
	String cmd8="MV NFS STOR:";

	    if(strstr(cmd,cmd1.c_str())!=NULL)
		   return ADSP;
  	    else if (strstr(cmd,cmd2.c_str())!=NULL)
		    return RVSP;
	    else if (strstr(cmd,cmd3.c_str())!=NULL)
	    	return APST;
	    else if (strstr(cmd,cmd4.c_str())!=NULL)
	    	return DEST;
	    else if (strstr(cmd,cmd5.c_str())!=NULL)
	    	return QUSP;
	    else if (strstr(cmd,cmd6.c_str())!=NULL)
	    	return QUDE;
	    else if (strstr(cmd,cmd7.c_str())!=NULL)
	    	return TEST;
	    else if (strstr(cmd,cmd8.c_str())!=NULL)
	    	return MVST;
	    else
	    	return ERROR;


}

String CMDType:: processack(const char* cmd,Type type)

{
if(type==ADSP) {cout<<"ADSP"<<endl;return processadsp(cmd);}
else if(type==RVSP){cout<<"RVSP"<<endl;return processrvsp(cmd);}
else if(type==APST){cout<<"APST"<<endl;return processapst(cmd);}
else if(type==DEST){cout<<"DEST"<<endl;return processdest(cmd);}
else if(type==QUSP){cout<<"QUSP"<<endl;return processqusp(cmd);}
else if(type==QUDE){cout<<"QUDE"<<endl;return processqude(cmd);}
else if(type==TEST){cout<<"TEST"<<endl;return processtest(cmd);}
else if(type==MVST){cout<<"MVST"<<endl;return processmvst(cmd);}
else {return processerror();}

}
String CMDType:: processadsp(const char* cmd)

{cmd=strstr(cmd,"IP=");
char buff[100];
strcpy(buff, cmd);
char *para=buff;
char *q1;
char *q2;

q1=strtok(para,",");
q2 = strtok(NULL, ",");

String cmd1=String(q1);
String cmd2=String(q2);

  String IP;
	Object ob1;
	ob1.strCmd=cmd1;

	ob1.processPara();
	if(ob1.paraMap.size() != 1 )
		return processerror();;
	map<String,String>::iterator it1;
	it1 = ob1.paraMap.find("IP");
	if( it1 == ob1.paraMap.end() )
		return processerror();;

	IP = it1->second;
	//cout<<"IP:"<<IP<<endl;
	if( IP.size() == 0 )
	return processerror();;

	        String Dir;
			Object ob2;
			ob2.strCmd=cmd2;
			ob2.processPara();
			if(ob2.paraMap.size() != 1 )
				return processerror();;
			map<String,String>::iterator it2;
			it2 = ob2.paraMap.find("DIR");
			if( it2 == ob2.paraMap.end() )
				return processerror();;

			Dir = it2->second;

		//	cout<<"Dir:"<<Dir<<endl;
			if( Dir.size() == 0 )
			return processerror();;
			String cmd3;
			String cmd4;
			cmd=strstr(cmd,"CAPACITY=");
			char buff1[100];
			strcpy(buff1, cmd);
			char *para1=buff1;
			char *q3;
			char *q4;
			q3=strtok(para1,",");
			q4 = strtok(NULL, ",");

			cmd3=String(q3);
			cmd4=String(q4);
			String Capacity;
			Object ob3;
			ob3.strCmd=cmd3;
			ob3.processPara();
			if(ob3.paraMap.size() != 1 )
				return processerror();;
			map<String,String>::iterator it3;
			it3 = ob3.paraMap.find("CAPACITY");
			if( it3 == ob3.paraMap.end() )
				return processerror();;

			Capacity = it3->second;


			if( Capacity.size() == 0 )
			return processerror();

			String TID;
						Object ob4;
						ob4.strCmd=cmd4;
						ob4.processPara();
						if(ob4.paraMap.size() != 1 )
							return processerror();;
						map<String,String>::iterator it4;
						it4 = ob4.paraMap.find("TID");
						if( it4 == ob4.paraMap.end() )
							return processerror();;

						TID = it4->second;
						cout<<"IP:"<<IP<<endl;
						cout<<"Dir:"<<Dir<<endl;
						cout<<"CAPACITY:"<<Capacity<<endl;
						cout<<"TID:"<<TID<<endl;
						if( TID.size() == 0 )
						return processerror();;
SC sc;
int i=sc.addnode(IP,Dir,atoi(Capacity.c_str()),atoi(TID.c_str()));
String retn;
	  String desc;
	 if(i==-1){
		    	  retn="-1";
		         desc="exec sql error";}
	 else  if(i==0){
	  retn="0";
	desc="succeed";

}
else if(i==1){
	 retn="1";
	 desc="not existed";


}
else{retn="2";
desc="not sufficient";
}
String msg="ACK:ADD NFS SP:RETN="+retn+",DESC="+desc;
	      		return msg;

}
String CMDType:: processrvsp(const char* cmd)
{
	cmd=strstr(cmd,"IP=");
		      char buff[100];
		      strcpy(buff, cmd);
		     char *para=buff;
		      char *q1;
		      char *q2;
		      q1=strtok(para,",");
		      q2 = strtok(NULL, ",");
		String cmd1=String(q1);
		String cmd2=String(q2);
		String IP;
			Object ob1;
			ob1.strCmd=cmd1;

			ob1.processPara();
			if(ob1.paraMap.size() != 1 )
				return processerror();;
			map<String,String>::iterator it1;
			it1 = ob1.paraMap.find("IP");
			if( it1 == ob1.paraMap.end() )
				return processerror();;

			IP = it1->second;
		//	cout<<"IP:"<<IP<<endl;
			if( IP.size() == 0 )
			return processerror();;

			        String Dir;
					Object ob2;
					ob2.strCmd=cmd2;
					ob2.processPara();
					if(ob2.paraMap.size() != 1 )
						return processerror();;
					map<String,String>::iterator it2;
					it2 = ob2.paraMap.find("DIR");
					if( it2 == ob2.paraMap.end() )
						return processerror();;

					Dir = it2->second;
					if(Dir.size()==0)
						return processerror();
			//		cout<<"Dir:"<<Dir<<endl;
    	      SC sc;

    	  int i=sc.deletenode(IP,Dir);
    	      String retn;
    	   	  String desc;
    	  	 if(i==-1){
    	   		    	  retn="-1";
    	   		         desc="exec sql error";}
    	  	 else if(i==0){
    	    	  retn="0";
	      		desc="succeed";

    	    }
    	     else if(i==1){
    	    	 retn="1";
    	    	 desc="not existed";


    	     }
    	     else{retn="2";
	    	  desc="not valid";
    	     }
         String msg="ACK:RMV NFS SP:RETN="+retn+",DESC="+desc;
    	     	      		return msg;
    }
	String  CMDType::processapst(const char* cmd)
	{
		 ip="";
		 dirname="";
	String applysize;
	String isdisk;
	String VMID;
	String VMTID;
	String VirtType;
	cmd=strstr(cmd,"VMID=");
	char buff[100];
	strcpy(buff, cmd);
	char *para=buff;
	char *q1;
	char *q2;

	q1=strtok(para,",");
	q2=strtok(NULL,",");

	String cmd1=String(q1);
	String cmd2345=strstr(cmd,"CAPACITY=");

	char buff1[100];
			strcpy(buff1, cmd2345.c_str());
		char *para1=buff1;
		char *q3;
		char *q4;
		q3=strtok(para1,",");
		q4=strtok(NULL, ",");
	//	cout<<"cmd23 is "<<cmd23.c_str()<<endl;
		String cmd2=String(q3);
	//	cout<<"cmd2 is "<<cmd2<<endl;
		//String cmd34=String(q4);
	//	cout<<"cmd3 is "<<cmd3<<endl;

		String cmd345=strstr(cmd,"TYPE=");

							char buff2[100];
									strcpy(buff2, cmd345.c_str());
								char *para2=buff2;
								char *q5;
								char *q6;
								q5=strtok(para2,",");
								//q6=strtok(NULL, ",");
								String cmd3=String(q5);
								//String cmd45=String(q6);

								String cmd45=strstr(cmd,"VMTID=");
								char buff3[100];
									strcpy(buff3, cmd45.c_str());
								char *para3=buff3;
//cout<<"cmd56 is "<<cmd56<<endl;
								char *q7;
								char *q8;
								q7=strtok(para3,",");
								q8=strtok(NULL, ",");
								String cmd4=String(q7);
								String cmd5=String(q8);

		Object ob1;
		ob1.strCmd=cmd1;

		ob1.processPara();
		if(ob1.paraMap.size() != 1 )
			return processerror();;
		map<String,String>::iterator it1;
		it1 = ob1.paraMap.find("VMID");
		if( it1 == ob1.paraMap.end() )
			return processerror();

		VMID = it1->second;
		//cout<<"IP:"<<IP<<endl;
		if( VMID.size()==0 )
		return processerror();;


				Object ob2;
				ob2.strCmd=cmd2;
				ob2.processPara();
				if(ob2.paraMap.size() != 1 )
					return processerror();;
				map<String,String>::iterator it2;
				it2 = ob2.paraMap.find("CAPACITY");
				if( it2 == ob2.paraMap.end() )
					return processerror();;

				applysize = it2->second;

			//	cout<<"Dir:"<<Dir<<endl;
				if( applysize.size() ==0 )
				return processerror();;


				Object ob3;
				ob3.strCmd=cmd3;
			//	cout<<"cmd3"<<cmd3<<endl;
				ob3.processPara();
				if(ob3.paraMap.size() != 1 )
					return processerror();;
				map<String,String>::iterator it3;
				it3 = ob3.paraMap.find("TYPE");
				if( it3 == ob3.paraMap.end() )
					return processerror();;

				isdisk = it3->second;

			//	cout<<"Dir:"<<Dir<<endl;
				if( isdisk.size()==0 )
				return processerror();;

				Object ob4;
								ob4.strCmd=cmd4;
							//	cout<<"cmd3"<<cmd3<<endl;
								ob4.processPara();
								if(ob4.paraMap.size() != 1 )
									return processerror();;
								map<String,String>::iterator it4;
								it4 = ob4.paraMap.find("VMTID");
								if( it4 == ob4.paraMap.end() )
									return processerror();;

								VMTID = it4->second;

							//	cout<<"Dir:"<<Dir<<endl;
								if( VMTID.size()==0 )
								return processerror();;

				Object ob5;
								ob5.strCmd=cmd5;
							//	cout<<"cmd3"<<cmd3<<endl;
								ob5.processPara();
								if(ob5.paraMap.size() != 1 )
									return processerror();;
								map<String,String>::iterator it5;
								it5 = ob5.paraMap.find("VirtType");
								if( it5 == ob5.paraMap.end() )
									return processerror();;

								VirtType = it5->second;

							//	cout<<"Dir:"<<Dir<<endl;
								if( VirtType.size()==0 )
								return processerror();;


				   SC sc;
				  int i=sc.applyforspace(atoi(applysize.c_str()),atoi(isdisk.c_str()),atoi(VMID.c_str()),atoi(VMTID.c_str()),VirtType);
				    	      String retn;
				    	   	  String desc;
				    	   	 if(i==-1){
				    	   		    	  retn="-1";
				    	   		         desc="exec sql error";}
				    	   	 else    if(i==0){
				    	    	  retn="0";
					      		desc="succeed";

				    	    }
				    	     else if(i==1){
				    	    	 retn="1";
				    	    	 desc="not succeed";


				    	     }

	if(i==0)	{ String msg="ACK:APPLY NFS STOR:RETN="+retn+",DESC="+desc+",IP="+ip+",dirname="+dirname;
				    	     	      		return msg;}

	else{ String msg="ACK:APPLY NFS STOR:RETN="+retn+",DESC="+desc;
		return msg;}

}



	String  CMDType::processdest(const char* cmd)
	{
		String ID;
		String type;
		cmd=strstr(cmd,"ID=");
				      char buff[100];
				      strcpy(buff, cmd);
				     char *para=buff;
				      char *q1;
				      char *q2;
				      q1=strtok(para,",");
				      q2=strtok(NULL, ",");
				String cmd1=String(q1);
				String cmd2=String(q2);

					Object ob1;
					ob1.strCmd=cmd1;

					ob1.processPara();
					if(ob1.paraMap.size() != 1 )
						return processerror();;
					map<String,String>::iterator it1;
					it1 = ob1.paraMap.find("ID");
					if( it1 == ob1.paraMap.end() )
						return processerror();;

					ID = it1->second;
					//cout<<"IP:"<<IP<<endl;
					if( ID.size() == 0 )
					return processerror();;


							Object ob2;
							ob2.strCmd=cmd2;
							ob2.processPara();
							if(ob2.paraMap.size() != 1 )
								return processerror();;
							map<String,String>::iterator it2;
							it2 = ob2.paraMap.find("TYPE");
							if( it2 == ob2.paraMap.end() )
								return processerror();;

							type = it2->second;
							if(type.size()==0)
								return processerror();;
		    	      SCnode scnode;
		   int i=scnode.releasespace(atoi(ID.c_str()),atoi(type.c_str()));
		    	      String retn;
		    	   	  String desc;
		    	   	 if(i==-1){
		    	   					 retn="-1";
		    	   					   desc="exec sql error";}
		    	   	 else   if(i==0){
		    	   					    	    	  retn="0";
		    	   						      		desc="succeed";

		    	   					    	    }
		    	   					    	     else if(i==1){
		    	   					    	    	 retn="1";
		    	   					    	    	 desc="not existed";


		    	   					    	     }

		    	 String msg="ACK:DEL NFS STOR:RETN="+retn+",DESC="+desc;
		    	   		return msg;


	}
	String  CMDType::processqusp(const char* cmd)
	{result1="";
		SC sc;
		int i=sc.querynode();
		String retn;
		String desc;
	 	 if(i==-1){
			    	retn="-1";
			        desc="exec sql error";}
	 	 else 	  if(i==0){
			    	   	retn="0";
			    	  desc="succeed";

			    	   		}
			    	   	else if(i==1){
			    	   		retn="1";
			    	   		desc="not succeed";

			    	   					}

	 	//cout<<"results:"<<result1<<endl;
	 	 if(i==0){String msg="ACK:QRY NFS SP:RETN="+retn+",DESC="+desc+",RESULT="+result1;
			return msg;}
	 	 else{String msg="ACK:QRY NFS SP:RETN="+retn+",DESC="+desc;
			return msg;
		}

	}
	String  CMDType::processqude(const char* cmd)
	{
		result2="";
		String IP;
		String Dir;
		cmd=strstr(cmd,"IP=");
				      char buff[100];
				      strcpy(buff, cmd);
				     char *para=buff;
				      char *q1;
				      char *q2;
				      q1=strtok(para,",");
				      q2 = strtok(NULL, ",");
				String cmd1=String(q1);
				String cmd2=String(q2);

					Object ob1;
					ob1.strCmd=cmd1;

					ob1.processPara();
					if(ob1.paraMap.size() != 1 )
						return processerror();;
					map<String,String>::iterator it1;
					it1 = ob1.paraMap.find("IP");
					if( it1 == ob1.paraMap.end() )
						return processerror();;

					IP = it1->second;
					//cout<<"IP:"<<IP<<endl;
					if( IP.size() == 0 )
					return processerror();;


							Object ob2;
							ob2.strCmd=cmd2;
							ob2.processPara();
							if(ob2.paraMap.size() != 1 )
								return processerror();
							map<String,String>::iterator it2;
							it2 = ob2.paraMap.find("DIR");
							if( it2 == ob2.paraMap.end() )
								return processerror();;

							Dir = it2->second;
							if(Dir.size()==0)
								return processerror();;
							SC sc;
							int i=sc.querynodedetails(IP,Dir);
							 String retn;
							String desc;
								if(i==-1){
									retn="-1";
								desc="exec sql error";}
								 else 	if(i==0){
									retn="0";
									desc="succeed";
								 }
							else if(i==1){
							retn="1";
						    desc="not succeed";
							    }


								//cout<<"results:"<<result2<<endl;
				if(i==0)
			{String msg="ACK:QRY SP DETAIL:RETN="+retn+",DESC="+desc+",RESULT="+result2;
			return msg;}
								else
			{String msg="ACK:QRY SP DETAIL:RETN="+retn+",DESC="+desc;
			return msg;}

	}
	String  CMDType::processtest(const char* cmd)
	{
		String IP;
		String Dir;


				cmd=strstr(cmd,"IP=");
						      char buff[100];
						      strcpy(buff, cmd);
						     char *para=buff;
						      char *q1;
						      char *q2;
						      q1=strtok(para,",");
						      q2 = strtok(NULL, ",");
						String cmd1=String(q1);
						String cmd2=String(q2);

							Object ob1;
							ob1.strCmd=cmd1;

							ob1.processPara();
							if(ob1.paraMap.size() != 1 )
								return processerror();;
							map<String,String>::iterator it1;
							it1 = ob1.paraMap.find("IP");
							if( it1 == ob1.paraMap.end() )
								return processerror();

							IP = it1->second;
							//cout<<"IP:"<<IP<<endl;
							if( IP.size() == 0 )
							return processerror();


									Object ob2;
									ob2.strCmd=cmd2;
									ob2.processPara();
									if(ob2.paraMap.size() != 1 )
										return processerror();
									map<String,String>::iterator it2;
									it2 = ob2.paraMap.find("DIR");
									if( it2 == ob2.paraMap.end() )
										return processerror();

									Dir = it2->second;
									if(Dir.size()==0)
										return processerror();
									SC sc;
									int i=sc.querynodedetails(IP,Dir);
									 String retn;
									String desc;
										if(i==-1){
											retn="-1";
										desc="exec sql error";}
										 else 	if(i==0){
											retn="0";
											desc="succeed";

													 }
									else if(i==1){
									retn="1";
								desc="not existed";


											 }
								else{retn="2";
									desc="not valid";
										}
										String msg="ACK:TEST NFS SP:RETN="+retn+",DESC="+desc;
													    	     	      		return msg;

	}
	String  CMDType::processmvst(const char* cmd)
	{
		String IP1;
		String Dir1;
		String IP2;
		String Dir2;
		cmd=strstr(cmd,"SIP=");
							      char buff[100];
							      strcpy(buff, cmd);
							     char *para=buff;
							      char *q1;
							      char *q2;
							      q1=strtok(para,",");
							      q2 = strtok(NULL, ",");
							      String cmd34=strstr(cmd,"DIP=");
							      char buff1[100];
							      strcpy(buff1, cmd34.c_str());
							      char *para1=buff1;
							      char *q3;
							     char *q4;
							     q3=strtok(para1,",");
							     q4 = strtok(NULL, ",");
							String cmd1=String(q1);
							String cmd2=String(q2);
							String cmd3=String(q3);
							String cmd4=String(q4);
					    //	cout<<"cmd1 is"<<cmd1<<endl;
						//	cout<<"cmd2 is"<<cmd2<<endl;
						//	cout<<"cmd3 is"<<cmd3<<endl;
						//	cout<<"cmd4 is"<<cmd4<<endl;
								Object ob1;
								ob1.strCmd=cmd1;

								ob1.processPara();
								if(ob1.paraMap.size() != 1 )
									return processerror();
								map<String,String>::iterator it1;
								it1 = ob1.paraMap.find("SIP");
								if( it1 == ob1.paraMap.end() )
									return processerror();

								IP1 = it1->second;
								//cout<<"IP:"<<IP<<endl;
								if( IP1.size() == 0 )
								return processerror();


										Object ob2;
										ob2.strCmd=cmd2;
										ob2.processPara();
										if(ob2.paraMap.size() != 1 )
											return processerror();
										map<String,String>::iterator it2;
										it2 = ob2.paraMap.find("SSPDIRNAME");
										if( it2 == ob2.paraMap.end() )
											return processerror();

										Dir1 = it2->second;
										if(Dir1.size()==0)
											return processerror();
										Object ob3;
				             ob3.strCmd=cmd3;
                               ob3.processPara();
                             if(ob3.paraMap.size() != 1 )
		                     return processerror();
							map<String,String>::iterator it3;
							it3 = ob3.paraMap.find("DIP");
							if( it3 == ob3.paraMap.end() )
							return processerror();

							IP2 = it3->second;
							//cout<<"IP:"<<IP<<endl;
							if( IP2.size() == 0 )
							return processerror();


							Object ob4;
							ob4.strCmd=cmd4;
							ob4.processPara();
							if(ob4.paraMap.size() != 1 )
								return processerror();
							map<String,String>::iterator it4;
							it4 = ob4.paraMap.find("DSPDIRNAME");
							if( it4 == ob4.paraMap.end() )
							return processerror();
                 //    cout<<"Dir2!!!!"<<Dir2<<endl;
					       Dir2 = it4->second;
		          if(Dir2.size()==0)
						return processerror();

		      	                         SCnode scnode;
		      									int i=scnode.redirect(IP1,Dir1,IP2,Dir2);
		      									 String retn;
		      									String desc;
		      										if(i==-1){
		      											retn="-1";
		      										desc="exec sql error";}
		      										 else 	if(i==0){
		      											retn="0";
		      											desc="succeed";

		      													 }
		      									else if(i==1){
		      									retn="1";
		      								desc="source or destination not existed";


		      											 }
		      								else{retn="2";
		      											 desc="not sufficient space to redirect";
		      										}
		      										String msg="ACK:MV NFS STOR: RETN="+retn+",DESC="+desc;
		      													    	     	      		return msg;
	}
	String  CMDType::processerror()
	{  String msg="invalid cmd!";
		return msg; }


