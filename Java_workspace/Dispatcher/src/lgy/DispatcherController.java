package lgy;

import java.io.*;
import java.sql.*;
import java.util.*;
import java.util.concurrent.Executors;
import java.net.*;

import common.*;
import model.*;
import architecture.*;

public class DispatcherController  implements Runnable{

	
	private static String config_file_name = "clccfg.cfg";

	private static String db_address;
	private static String db_user_name;
	private static String db_user_pass;
	private static String db_name;
	private static Map<String,String> config_map;
	private static Log log;
	private Socket client_socket;
	
	
	public DispatcherController(Socket clientSocket)
	{
		client_socket = clientSocket;
	}
	
	/**
	 * 初始化，初始化失败，则返回false
	 * @return
	 */
	public static boolean init(Log l) throws Exception
	{
		//初始化日志
		log = l;

		//读配置文件
		readConfigFile();
		
		
		if(!config_map.containsKey("DBServerAddr".toLowerCase()))
		{
			System.out.println("Initiate db_address Failed!");
			return false;
		}
		else
		{	
			db_address = config_map.get("DBServerAddr".toLowerCase());
		}
		
		
		if(!config_map.containsKey("DBUserName".toLowerCase()))
		{
			System.out.println("Initiate db_user_name Failed!");
			return false;
		}
		else
		{	
			db_user_name = config_map.get("DBUserName".toLowerCase());
		}
		
		if(!config_map.containsKey("DBUserPass".toLowerCase()))
		{
			System.out.println("Initiate db_user_pass Failed!");
			return false;
		}
		else
		{	
			db_user_pass = config_map.get("DBUserPass".toLowerCase());
		}
		
		
		if(!config_map.containsKey("DBName".toLowerCase()))
		{
			System.out.println("Initiate db_name Failed!");
			return false;
		}
		else
		{	
			db_name = config_map.get("DBName".toLowerCase());
		}
		
		
		return true;
		
	}
	
	public static Connection getMysqlConn()
	{
        // 驱动程序名
        String driver = "com.mysql.jdbc.Driver";

        // URL指向要访问的数据库名scutcs
        String url = "jdbc:mysql://"+db_address+":3306/"+db_name;

        try { 
         // 加载驱动程序
         Class.forName(driver);

         // 连续数据库
         Connection conn = DriverManager.getConnection(url, db_user_name, db_user_pass);
         return conn;

        }
        catch(Exception e)
        {
        	e.printStackTrace();
        	return null;
        }
	}
	
	public static void readConfigFile() throws Exception
	{
		config_map = new HashMap<String,String>();
		
		BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(config_file_name)));
		String line = null;
		String [] key_value;
		while((line = br.readLine())!=null)
		{
			if(line.startsWith("#"))//跳过注释行
				continue;
			
			key_value = line.split("=");
			
			if(key_value.length != 2)
				continue;
			
			//key 全部转化为小写
			config_map.put(key_value[0].toLowerCase(), key_value[1]);		
		}
		br.close();
	}
	
	
	//根据VM_ID获取VM
	private static VMDetail getVMDetail(String vm_id)
	{
		String sql = "select * from VMDetail where id="+vm_id;
		List<VMDetail> vm_list = new LinkedList<VMDetail>();
		try{
			Connection conn = DatabaseManager.getMysqlConn();
			Statement statement = conn.createStatement();
			ResultSet rs = statement.executeQuery(sql);
			if(rs.next())
			{
				//| ID | VMNAME| OS| DISK | RAM | VCPU | USERID | DiskType | IsParaVirt | 
				//IsAutoScaling | VirtType | UUID  | Status | Host | GroupID | GroupInnerID |

				VMDetail vmd = new VMDetail();
				vmd.setID(rs.getInt("ID"));
				vmd.setVMNAME(rs.getString("VMName"));
				vmd.setOS(rs.getString("OS"));
				vmd.setDISK(rs.getInt("DISK"));
				vmd.setRAM(rs.getInt("RAM"));
				vmd.setVCPU(rs.getInt("VCPU"));
				vmd.setUSERID(rs.getInt("USERID"));
				vmd.setDiskType(rs.getInt("DiskType"));
				vmd.setIsParaVirt(rs.getInt("IsParaVirt"));
				vmd.setIsAutoScaling(rs.getInt("IsAutoScaling"));
				vmd.setVirtType(rs.getString("VirtType"));
				vmd.setUUID(rs.getString("UUID"));
				vmd.setStatus(rs.getInt("Status"));
				vmd.setHostIP(rs.getString("Host"));
				vmd.setGroupID(rs.getInt("GroupID"));
				vmd.setGroupInnerID(rs.getInt("GroupInnerID"));
				
				return vmd;
			}
			
		}catch(Exception e)
		{
			e.printStackTrace();
			log.write("Error in get VMDetail by vm_id from Datebase.", TYPE.ERROR);
			return null;
		}
		return null;
	}
	//根据Group_ID获取VM list
	private static List<VMDetail> getVMDetailList(String group_id)
	{
		String sql = "select * from VMDetail where GroupID="+group_id;
		List<VMDetail> vm_list = new LinkedList<VMDetail>();
		try{
			Connection conn = DatabaseManager.getMysqlConn();
			Statement statement = conn.createStatement();
			ResultSet rs = statement.executeQuery(sql);
			while(rs.next())
			{
				//| ID | VMNAME| OS| DISK | RAM | VCPU | USERID | DiskType | IsParaVirt | 
				//IsAutoScaling | VirtType | UUID  | Status | Host | GroupID | GroupInnerID |

				VMDetail vmd = new VMDetail();
				vmd.setID(rs.getInt("ID"));
				vmd.setVMNAME(rs.getString("VMName"));
				vmd.setOS(rs.getString("OS"));
				vmd.setDISK(rs.getInt("DISK"));
				vmd.setRAM(rs.getInt("RAM"));
				vmd.setVCPU(rs.getInt("VCPU"));
				vmd.setUSERID(rs.getInt("USERID"));
				vmd.setDiskType(rs.getInt("DiskType"));
				vmd.setIsParaVirt(rs.getInt("IsParaVirt"));
				vmd.setIsAutoScaling(rs.getInt("IsAutoScaling"));
				vmd.setVirtType(rs.getString("VirtType"));
				vmd.setUUID(rs.getString("UUID"));
				vmd.setStatus(rs.getInt("Status"));
				vmd.setHostIP(rs.getString("Host"));
				vmd.setGroupID(rs.getInt("GroupID"));
				vmd.setGroupInnerID(rs.getInt("GroupInnerID"));
				
				vm_list.add(vmd);
			}
			
		}catch(Exception e)
		{
			e.printStackTrace();
			log.write("Error in get VMDetail by vm_id from Datebase.", TYPE.ERROR);
			return null;
		}
		return vm_list;
	}
	//将 "0021" 转化成 int 型的 21
	private int convertRecLen(byte[] len_buff)
	{
		String result="";
		boolean begin = false;

		for(int i=0;i<4;i++)
		{
			if(len_buff[i]!='0')
			{
				result += (char)len_buff[i];
				begin = true;
			}
			else
			{
				if(begin)
					result += (char)len_buff[i];
			}
		}
		if(result != "")
		{
			int  len = Integer.parseInt(result);
			return len;
		}
		return -1;
	}
	
	//将byte[4]转成int
	private int convertRecLen2(byte[] len_buff)
	{
		int result =0;

		for(int i=0;i<4;i++)
		{
			result+= len_buff[i]<<8*i;
		}
		return result;
	}
	
	//模拟 协议中的sprinf（buff,%04d,len）
	//注意TCP是big endian!所以高字节在低地址，即byte[0]要放最高位！
	byte[] getMsgLenBytes(int len) throws Exception
	{
		if(len > 9999 || len <0)
		{
			System.out.println("Length should between 0,9999");
			throw new Exception ("Length should between 0,9999");
		}
		
		byte [] lenbyte = new byte[4];
		for(int i= 3 ;i>=0;i--)
		{
			lenbyte[i] = (byte)(len%10 +'0');
			//System.out.println(lenbyte[i]);
			len = len/10;
		}
		return lenbyte;
	}
	
	byte[] getMsgLenBytesToCPP(int len) throws Exception
	{
		
		byte [] lenbyte = new byte[4];
		for(int i= 0 ;i<4;i++)
		{
			lenbyte[i] = (byte)(len & 0x000000FF);
			len = len>>8;

		}
		return lenbyte;
	}
	/**
	 * 接收指令，分析指令，并放回结果
	 */
	public void run()
	{
		byte[] len_buff = new byte[512];  
		byte[] cmd_buff = new byte[512];
		int offset;
		try{
			InputStream in = client_socket.getInputStream();
			OutputStream out = client_socket.getOutputStream();
			
			offset = in.read(len_buff,0,4);
			
			len_buff[4]='\0';
			//int cmd_len = convertRecLen(len_buff);
			int cmd_len = convertRecLen2(len_buff);
			//System.out.println("Cmd length: " + new String(len_buff));
			offset = in.read(cmd_buff,0,512);
			//System.out.println(cmd_len+" " +offset);
			if(offset!=cmd_len)
			{
				log.write("Error In Reading Socket Data!", TYPE.ERROR);
				System.out.println("Error In Reading Socket Data!");
				return;
			}
			System.out.println("Receive CMD: "+new String(cmd_buff));
			String cmd = new String(cmd_buff,0,offset);
			
			//System.out.println(cmd);
			log.write("Receive CMD: "+cmd, TYPE.INFO);
			//分析指令，返回指令执行结果
			String result = analyzeCMD(cmd);
			byte[] msgbuff =result.getBytes();
			//byte[] msglen = getMsgLenBytes(msgbuff.length);
			byte[] msglen = getMsgLenBytesToCPP(msgbuff.length);
			log.write("Return CMD: "+cmd, TYPE.INFO);
			out.write(msglen);
			out.write(msgbuff);
			out.flush();
			
			client_socket.close();
			
		}catch(Exception e)
		{
			log.write("Error In Socket!", TYPE.ERROR);
			e.printStackTrace();
		}
		
	}
	String analyzeCMD(String cmd)
	{
		String ackCode;
		String []cmd_para = cmd.split(":");
		if(cmd_para.length!=2)
		{
			log.write("Error CMD Format!", TYPE.ERROR);
			return "ACK:GET BEST NC:RETN=4,DESC=Error CMD Format!";
		}
		
		if(!cmd_para[0].toLowerCase().equals("GET BEST NC".toLowerCase()))
		{
			log.write("Error CMD Type!", TYPE.ERROR);
			return "ACK:GET BEST NC:RETN=4,DESC=Error CMD Type!";
		}
		
		String [] params = cmd_para[1].split(",");
		if(params.length!=1)
		{
			log.write("Error Parameter!", TYPE.ERROR);
			return "ACK:GET BEST NC:RETN=4,DESC=Error Parameter!";
		}
		
		String[] key_value = params[0].split("=");
		if( key_value.length != 2 )
		{
			log.write("Error Parameter!", TYPE.ERROR);
			return "ACK:GET BEST NC:RETN=4,DESC=Error Parameter!";
		}
		
		if( key_value[0].toLowerCase().equals("VMID".toLowerCase()))
		{
			System.out.println("Process allocate VM:"+key_value[1]);
			ackCode = findNCForSingleVM(key_value[1]);
		}
		else if(key_value[0].toLowerCase().equals("GroupID".toLowerCase()))
		{
			ackCode = findNCForGroupVM(key_value[1]);
		}
		else
		{
			log.write("Error Parameter!", TYPE.ERROR);
			return "ACK:GET BEST NC:RETN=4,DESC=Error Parameter!";
		}
		System.out.println("Send ACK Code From Dispatcher Server :" + ackCode);
		return ackCode;
		
	}
	
	//返回值为 由最佳 NC的IP组成的Ack Code
	String findNCForSingleVM(String VMID)
	{
		String ackCode = "";
		VMDetail vmd = getVMDetail(VMID);
		FirstFit ff = new FirstFit();
		String nc_ip = ff.findBestNC(vmd);
		System.out.println("findNCForSingleVM: NCIP= "+nc_ip);
		if(nc_ip == null)
		{
			ackCode += "ACK:GET BEST NC:RETN=8,DESC=No Capable NC now !";
		}else
		{
			ackCode += "ACK:GET BEST NC:RETN=0,DESC=SUCCESS,IP="+nc_ip;
		}
		
		return ackCode;
	}
	
	String findNCForGroupVM(String GroupID)
	{
		String ackCode = "";
		List<VMDetail> vmd_list = getVMDetailList(GroupID);
		FirstFit ff = new FirstFit();
		Map<Integer,String> vmid_ncip_map = ff.findBestNCForGroup(vmd_list);
		
		if(vmid_ncip_map == null)
		{
			ackCode += "ACK:GET BEST NC:RETN=8,DESC=No Capable NCs for Group Start now !";
		}else
		{
			String result = "";
			//生成的格式为 vm_id1/nc_ip1|vm_id2/nc_ip2|...
			for(Map.Entry<Integer,String> entry:vmid_ncip_map.entrySet())
			{
				result +=entry.getKey()+"/"+entry.getValue()+"|";
			}
			//去除最后的"|"
			result  = result.substring(0, result.length()-1);
			ackCode += "ACK:GET BEST NC:RETN=0,DESC=SUCCESS,VM_NC_MAP="+result;
		}
		
		return ackCode;
	}
	
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub

	}

}
