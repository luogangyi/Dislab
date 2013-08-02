import java.sql.*;
import java.util.*;
import java.util.Date;
import java.util.regex.*;
import java.io.*;

public class TrafficMonitor {

	public static String interface_monitor = "br0";
	public static String localIP;
    // MySQL配置时的用户名
    private static String user = "root"; 
    private static String configFilename = "nccfg.cfg";
    // MySQL配置时的密码
    private static String password = "123456";
    //监控时间间隔，单位为 分钟(min)
    private static int inteval = 1;
	private Connection conn;

	
	public TrafficMonitor()
	{
		setLocalIP();
		conn = getMysqlConn();
	}
	/**
	 * 从配置文件中获取本机IP
	 * @return
	 */
	public static boolean setLocalIP()
	{
		try{
			BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(configFilename)));
			String line = null;
			while((line = br.readLine())!=null)
			{
				//System.out.println(line);
				if(line.contains("NCLocalIP"))
					localIP = line.substring(line.indexOf('=')+1);
			}
			if(localIP!=null)
			{
				System.out.println("Local IP :"+localIP);
				return true;
			}
			return false;
		}catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	//获取MySQL连接
	public Connection getMysqlConn()
	{
        // 驱动程序名
        String driver = "com.mysql.jdbc.Driver";

        // URL指向要访问的数据库名scutcs
        String url = "jdbc:mysql://127.0.0.1:3306/cloud";

        try { 
         // 加载驱动程序
         Class.forName(driver);

         // 连续数据库
         Connection conn = DriverManager.getConnection(url, user, password);
         return conn;
        }
        catch(Exception e)
        {
        	e.printStackTrace();
        	return null;
        }
	}
	
	/**
	 * 根据物理机IP获取物理机上所运行的虚拟机的ID
	 * @param localIP
	 * @return
	 */
	public List<Integer> getLocalVMID(String localIP)
	{
		String sql="select ID from VMDetail where Host = '"+localIP + "'";
		List<Integer> id_list = new LinkedList<Integer>();
		try{
            // statement用来执行SQL语句
            Statement statement = conn.createStatement();
            ResultSet rs = statement.executeQuery(sql);
            String name = null;
            int id;
            while(rs.next()) {
    
             // 选择sname这列数据
             name = rs.getString("ID");
             id = Integer.parseInt(name);
             id_list.add(id);
             System.out.println("VM(id) on local"+localIP+" :" +name);
            }
            return id_list;
		}catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	/**
	 * 根据VMID获取VM所属的GroupID
	 * 根据GroupID获取所属的所有VMID
	 */
	public List<Integer> getGruopPartnerID(int vmid)
	{
		
		
		String sql="select GroupID from VMDetail where ID = "+vmid + "";
		List<Integer> id_list = new LinkedList<Integer>();
		try{
            // statement用来执行SQL语句
            Statement statement = conn.createStatement();
            ResultSet rs = statement.executeQuery(sql);
            String GruopID = null;
            int group_id;
            if(rs.next())
            {
            	GruopID = rs.getString("GroupID");
                group_id = Integer.parseInt(GruopID);
                System.out.println("Group ID on local"+group_id);
            }
            else
            {
            	System.out.println("Error VMID!");
            	return null;
            }

            
    		sql="select ID from VMDetail where GroupID = "+group_id ;
            // statement用来执行SQL语句
            statement = conn.createStatement();
            rs = statement.executeQuery(sql);
            String name = null;
            int id;
            while(rs.next()) {
    
             // 选择sname这列数据
             name = rs.getString("ID");
             id = Integer.parseInt(name);
             id_list.add(id);
             System.out.println(group_id+" has parter VM(id) :"+id);
            }
            return id_list;
		

		}catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	/**
	 * 生成产生监控traffic命令的脚本
	 * 返回监控结果所保存的文件名
	 * @param ip_arr
	 * @return
	 */
	public String  generateMonitorCMD()
	{
		try{
			File file = new File("start_monitor.sh");
			FileOutputStream fos = new FileOutputStream(file);
			OutputStreamWriter osw = new OutputStreamWriter(fos,"UTF-8");
			BufferedWriter bw = new BufferedWriter(osw);
			Date date = new Date();
			bw.write("#!bin/sh");
			bw.newLine();
			bw.write("#written by lgy!");
			bw.newLine();

			String outputFileName = "traffic"+"_"+date+".txt";
			//将空格全部替换为下划线，否则命令执行会出错！
			outputFileName = outputFileName.replaceAll("\\s+", "_");

			String cmdString="/usr/sbin/tcpdump " +" -i "+interface_monitor+" >"+outputFileName;
			bw.write(cmdString);
			bw.newLine();
			bw.flush();

			bw.close();
			return outputFileName;
		}catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}

	}
	/**
	 * 根据VM id 获取 ip
	 * @param id
	 * @return
	 */	
	public String getIPfromID(int id)
	{
		String sql="select IP from MACIP where VMID = "+id;
		List<Integer> id_list = new LinkedList<Integer>();
		try{
            // statement用来执行SQL语句
            Statement statement = conn.createStatement();
            ResultSet rs = statement.executeQuery(sql);
            String IP = null;
            int group_id;
            if(rs.next())
            {
            	IP = rs.getString("IP");
                System.out.println("VM(id) : "+id +", IP:"+IP);
                return IP;
            }
            else
            {
            	System.out.println("No IP belongs to this VMID!");
            	return null;
            }

		}catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}

	/**
	 * 每隔inteval时间实行一次TCPdump命令
	 * @return
	 */
	public  int excecuteTCPDumpCMD()
	{
		
	    try
	    {
	    
	      String[] cmd = new String[2];
	      cmd[0] = "sh";
	      cmd[1] = "start_monitor.sh";

	      Process p = Runtime.getRuntime().exec(cmd);
	      Thread.sleep(inteval*60*1000);
	      cmd[0] = "sh";
	      cmd[1] = "stop_monitor.sh";

	      Runtime.getRuntime().exec(cmd);
	    }

	    catch (Exception ioe)
	    {
	    	ioe.printStackTrace();
	    	System.out.println(
	        "An error occured while getting node information ("
	          + ioe.getMessage()
	          + ")");
	    }
		return 0;
	}
	/**
	 * 结果以JSON格式存入数据库,即 key为ip 值为ip1:data1,ip2:data2,...
	 * @param ip
	 * @param result
	 */
	public void storeResultToDB(String ip,String result)
	{
		String sql="delete from TrafficData where IP = '"+ip+"'";
		String sql2 = "insert into TrafficData (IP,Data) values ('"+ip+"','"+result+"')";
		try{
            // statement用来执行SQL语句
            Statement statement = conn.createStatement();
            statement.execute(sql);
            statement.execute(sql2);
		}catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	/**
	 * 
	 * @param fileName
	 * @param m_ip 要监控的IP
	 * @param ip_arr 与监控IP所属同一群组中的IP
	 * 结果以JSON格式存入数据库,即 {ip:{ip1:data1,ip2:data2,...}}
	 */
	public void processData(String fileName,String m_ip, List<String> ip_arr)
	{

		Long[] data_arr = new Long[ip_arr.size()]; 
		Pattern[] pattern_arr = new Pattern[ip_arr.size()];
		
		String regex_ip = m_ip.replaceAll("\\.", "\\\\.");
		String[] regex_ip_arr = new String[ip_arr.size()];
		for(int i=0;i < ip_arr.size();i++)
		{
			regex_ip_arr[i] = ip_arr.get(i).replaceAll("\\.", "\\\\.");
			//System.out.println(ip_arr[i]);

			String regex = "(("+regex_ip_arr[i]+")|("+regex_ip+"))\\s+>"
					+ "\\s+(("+regex_ip_arr[i]+")|("+regex_ip+")).*length\\s+(\\d+)";
			pattern_arr[i] = Pattern.compile(regex);
		}
		
		
		//System.out.println(m_ip);

		for(int i = 0;i <data_arr.length; i++)
		{
			data_arr[i] = 0L ;
		}
		try{
			String line;
			BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(fileName)));
			while((line = br.readLine())!=null)
			{
				for(int i = 0;i<data_arr.length;i++)
				{

					Matcher m = pattern_arr[i].matcher(line);
					if(m.find())
					{
						data_arr[i] += Long.parseLong(m.group(7));
					}			
				}
			}
		}
		catch(Exception e)
		{
			System.out.println("Process Monitored Data Error!");
			e.printStackTrace();
		}
		for(int i = 0;i <data_arr.length; i++)
		{
			System.out.println(data_arr[i]);
		}
		
		String result_data="";
		for(int i=0;i<ip_arr.size();i++)
		{
			result_data+=ip_arr.get(i);
			result_data+=":";
			result_data+=data_arr[i];
			if(i != ip_arr.size()-1)
				result_data+=",";
		}
		//将结果存入数据库中
		storeResultToDB(m_ip,result_data);
	}
	/**
	 * 开始监控虚拟机通信
	 * 每隔Inteval更新一次
	 */
	public void startMonitor()
	{
		while(true)
		{
			String monitored_filename;
			monitored_filename = generateMonitorCMD();
			excecuteTCPDumpCMD();

			
			List<Integer> local_vm_id_list = getLocalVMID(localIP);
			for(int vm_id:local_vm_id_list)
			{
				String local_vm_ip = getIPfromID(vm_id);
				//替换为正则表达式所需要的格式
				System.out.println("local_vm_ip :"+local_vm_ip);
				List<Integer> partner_id_list = getGruopPartnerID(vm_id);
				List<String> parter_ip_list = new LinkedList<String>();
				for(int partner_id:partner_id_list)
				{
					String partner_ip = getIPfromID(partner_id);
					System.out.println("parter_ip:"+partner_ip);
					parter_ip_list.add(partner_ip);
				}
				
				//处理监控到的数据
				processData(monitored_filename,local_vm_ip,parter_ip_list);
				
			}
		}
	}
	public static void main(String[] args) {

		TrafficMonitor tm = new TrafficMonitor();

		tm.startMonitor();

	}

}
