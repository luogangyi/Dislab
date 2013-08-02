#include "GetVNCConsole.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libvirt/libvirt.h>


extern Log log;
extern string localip;
GetVNCConsole::GetVNCConsole(string cmd)
{
	port = "";
	strCmd= cmd;
}

GetVNCConsole::~GetVNCConsole()
{

}

void GetVNCConsole::Do()
{
	log.write("in GetVNCConsole::Do", Log::INFO);

	//参数检查
	if( getPara() == false)
	{
		ackCode =  "ACK:GET VNC CONSOLE:RETN=4,DESC=parameters error";
		return;
	}

	if( getVNCPort() )
	{
		ackCode =  "ACK:GET VNC CONSOLE:RETN=0,DESC=success,console=" + localip;
		ackCode += ":" + port;
		return;
	}
	ackCode =  "ACK:GET VNC CONSOLE:RETN=8,DESC=internal error";
	
}

bool GetVNCConsole::getVNCPort()
{
	virConnectPtr conn = NULL;
    virDomainPtr dom = NULL;

	
    conn = virConnectOpen("");
	
	if (conn == NULL)
		return false;
	
	dom = virDomainLookupByUUIDString(conn,vmUUID.c_str());

	if( dom == NULL )
	{		
		virConnectClose(conn);
		return false;
	}

	virDomainInfo info;
	
	char *xml = NULL;
	xml = virDomainGetXMLDesc(dom,0);	
	
	if( xml == NULL)			
		return false;
   
	virDomainFree(dom);    
	virConnectClose(conn);
	  
	  
	xmlDocPtr doc;
	xmlNodePtr cur;
	
	//doc = xmlReadMemory( xml, strlen(xml), "", NULL, 0);
	doc = xmlParseMemory(xml, strlen(xml));
	if (doc == NULL ) 
		return false;
	
	
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) 
	{
		xmlFreeDoc(doc);
		return false;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "domain")) 
	{
		xmlFreeDoc(doc);
		return false;
	}

	cur = cur->xmlChildrenNode;

	while (cur != NULL) 
	{

		if ((!xmlStrcmp(cur->name, (const xmlChar *)"devices")))
		{
			cur = cur->xmlChildrenNode;

			while (cur != NULL) 
			{
		
				if ((!xmlStrcmp(cur->name, (const xmlChar *)"graphics")))
				{
			
					xmlChar* portStr = xmlGetProp (cur, (const xmlChar *)"port");
					char buf[10]={0};
					sprintf(buf,"%s",portStr);
					port = buf;
					break;
			
				}
			
				cur = cur->next;

			}
			break;
		}

		cur = cur->next;

	}
	
	xmlFreeDoc(doc);

	return port.length() > 0;


}


bool GetVNCConsole::getPara()
{
	processPara();

	if( paraMap.size() != 1 )
		return false;
	
	map<string,string>::iterator it;

	//获取NFS服务器IP地址信息
	it = paraMap.find("vmuuid");
	if( it == paraMap.end() )
	{
		
		return false;
	}
	vmUUID = it->second;	
		
	return true;
}



