#include "XML.h"

extern string bridge;



XML::XML()
{
	
}


XML::~XML()
{
}


bool XML::begin(string vmName, int ram, int vcpu, string disk, string bridge, string mac)
{
	/*outfile.open(fileName.c_str(), ios::out);
	if( outfile.is_open() == false)
		return false;
	outfile<<"<domain type='kvm'>"<<endl;
	outfile<<"\t<name>"<<vmName<<"</name>"<<endl;
	outfile<<"\t<memory>"<<ram*1024<<"</memory>"<<endl;
	outfile<<"\t<os>\n\t\t<type>hvm</type>\n\t\t<boot dev=\"hd\" />\n\t</os>"<<endl;
	outfile<<"\t<features>\n\t\t<acpi/>\n\t\t<apic/>\n\t\t<pae/>\n\t</features>"<<endl;
	outfile<<"\t<vcpu>"<<vcpu<<"</vcpu>"<<endl;
	
	//begin write device
	outfile<<"\t<devices>"<<endl;
	outfile<<"\t\t<disk type='file' device='disk'>"<<endl;
	outfile<<"\t\t\t<driver type='raw' cache='none'/>"<<endl;
	outfile<<"\t\t\t<source file='"<<disk<<"'/>"<<endl;
	outfile<<"\t\t\t<target dev='hda' bus='ide'/>"<<endl;
	outfile<<"\t\t</disk>"<<endl;
	
	outfile<<"\t\t<interface type='bridge'>"<<endl;
	outfile<<"\t\t\t<source bridge='"<<bridge<<"'/>"<<endl;
	outfile<<"\t\t\t<mac address='"<<mac<<"'/>"<<endl;
	outfile<<"\t\t</interface>"<<endl;
	outfile<<"\t\t<serial type='pty'/>"<<endl;
	outfile<<"\t\t<input type='tablet' bus='usb'/>"<<endl;
	outfile<<"\t\t<graphics type='vnc' port='-1' autoport='yes' keymap='en-us' />"<<endl;
	outfile<<"\t\t<video>"<<endl;
	outfile<<"\t\t\t<model type='vmvga' vram='9216' heads='1'/>"<<endl;
	outfile<<"\t\t\t<address type='pci' domain='0x0000' bus='0x00' slot='0x02' function='0x0'/>"<<endl;
	outfile<<"\t\t</video>"<<endl;
	
	
	outfile<<"\t</devices>"<<endl;*/
	return true;
}


bool XML::end()
{
	
	//outfile<<"</domain>"<<endl;
	//outfile.close();
	return true;
}

string XML::getXenHeaderXML(string vmName, int ram, int vcpu, string uuid)
{
	string xml;

	char buf[10];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",ram*1024);
	
	xml = "<domain type='xen' >";
	xml += "<name>" + vmName + "</name>";
	xml += "<uuid>" + uuid + "</uuid>";
	xml += "<memory>" + string(buf);
	xml += "</memory>";
	
	//xml += "<os><type>linux</type></os>";
	xml += "<features><acpi/></features>";

	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",vcpu);
	xml += "<vcpu>" + string(buf);
	xml += "</vcpu>";
	xml += "<bootloader>/usr/bin/pygrub</bootloader>";
	xml += "<devices>";
	
	return xml;
	
}


string XML::getKvmHeaderXML(string vmName, int ram, int vcpu, string uuid)
{
	string xml;

	char buf[10];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",ram*1024);
	
	xml = "<domain type='kvm' >";
	xml += "<name>" + vmName + "</name>";
	xml += "<uuid>" + uuid + "</uuid>";
	xml += "<memory>" + string(buf);
	xml += "</memory>";
	
	xml += "<os><type>hvm</type></os><boot dev='hd' />";
	xml += "<features><acpi/></features>";

	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",vcpu);
	xml += "<vcpu>" + string(buf);
	xml += "</vcpu>";	
	xml += "<devices>";
	
	return xml;
	
}

string XML::getXenDiskXML(string disk)
{
	string xml = "<disk type='file' >";
	xml += "<driver type='raw' cache='none'/>";
	xml += "<source file='" + disk + "'/>";
	xml += "<target dev='hda' bus='ide'/>";
	xml += "</disk>";
	
	return xml;
}


string XML::getKvmDiskXML(string disk)
{
	string xml = "<disk type='file' >";
	xml += "<driver type='qcow2' cache='none'/>";
	xml += "<source file='" + disk + "'/>";
	xml += "<target dev='vda' bus='virtio'/>";
	xml += "</disk>";
	
	return xml;
}


string XML::getXenNetworkXML(string mac)
{
	string xml = "<interface type='bridge'>";
	xml += "<source bridge='" + bridge + "'/>";
	xml += "<mac address='" + mac + "'/>";
	xml += "</interface>";
	
	return xml;
	
}

string XML::getKvmNetworkXML(string mac)
{
	string xml = "<interface type='bridge'>";
	xml += "<source bridge='" + bridge + "'/>";
	xml += "<mac address='" + mac + "'/>";
	xml += "</interface>";
	
	return xml;
	
}


string XML::getXenTailXML()
{
	string xml = "<serial type='pty'/>";	
	xml += "<graphics type='vnc' port='5900' autoport='yes' keymap='en-us' listen='0.0.0.0' />";
	
	
	xml += "</devices>";
	xml += "</domain>";
	xml += "\n";
	
	return xml;
	
}


string XML::getKvmTailXML()
{
	string xml = "<serial type='pty'/>";
	xml += "<input type='tablet' bus='usb'/>";
	xml += "<graphics type='vnc' port='-1' autoport='yes' keymap='en-us' listen='0.0.0.0' />";
	xml += "<video>";
	xml += "<model type='vga' vram='9216' heads='1'/>";
	xml += "</video>";
	
	xml += "</devices>";
	xml += "</domain>";
	
	return xml;
	
}






