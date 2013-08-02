import java.util.*;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.sql.*;

public class TrafficMatricGenerator {
	
	public static String localIP;
    // MySQL配置时的用户名
    private static String user = "root"; 
    private static String configFilename = "nccfg.cfg";
    // MySQL配置时的密码
    private static String password = "123456";

	private Connection conn;

	
	public TrafficMatricGenerator()
	{
		conn = getMysqlConn();
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
	 * 获取所有的group id 列表
	 * @return
	 */
	public List<Integer> getGroupIDList()
	{
		String sql="select ID from Group ";
		List<Integer> group_id_list = new LinkedList<Integer>();
		try{
            // statement用来执行SQL语句
            Statement statement = conn.createStatement();
            ResultSet rs = statement.executeQuery(sql);
            String temp = null;
            int group_id;
            while(rs.next()) {
    
             // 选择sname这列数据
             temp = rs.getString("id");
             group_id = Integer.parseInt(temp);
             group_id_list.add(group_id);
             System.out.println("Group(id) "+group_id);
            }
            return group_id_list;
		}catch(Exception e)
		{
			e.printStackTrace();
			//出错，则返回空列表
			return group_id_list;
		}
	}
	/**
	 * 根据GroupID获取所属的所有VMID
	 */
	public List<Integer> getGruopPartnerID(int group_id)
	{
		
		List<Integer> id_list = new LinkedList<Integer>();
		try{
            
			String sql="select ID from VMDetail where GroupID = "+group_id ;
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
	 * 根据vm_id获取其通信流量,若流量为空，则返回空串
	 * @param vm_id
	 * @return
	 */
	public String getTrafficDataFromVMID(int vm_id)
	{
		String vm_ip = getIPfromID(vm_id);
		try{
            
			String sql="select Data from TrafficData where IP = '"+vm_ip+"'" ;
            // statement用来执行SQL语句
			Statement statement = conn.createStatement();
            ResultSet rs = statement.executeQuery(sql);
            String s_data = null;
            int id;
            if(rs.next()) {
             // 选择sname这列数据
            	s_data = rs.getString("IP");
            	System.out.println(vm_ip+" traffic data: "+s_data);
            	return s_data;
            }
            return "";
		

		}catch(Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}
	/**
	 * 根据一组虚拟机的IP,从数据库中的记录生成TrafficMatrix文件
	 * 文件名为Traffic_Matrix_<group id>.txt
	 */
	public void generateTrafficMatrixFile(int group_id,List<Integer> id_list)
	{	
		for(int vm_id:id_list)
		{
			String trafficData =getTrafficDataFromVMID(vm_id);
			
		}
	}
	
	/**
	 * 根据数据库中的记录生成TrafficMatrix文件
	 * 文件名为Traffic_Matrix_<group id>.txt
	 */
	public void generateTrafficMatrix()
	{
		List<Integer> group_id_list = getGroupIDList();
		
		for(int group_id:group_id_list)
		{
			List<Integer> id_list =getGruopPartnerID(group_id);
			generateTrafficMatrixFile(group_id,id_list);
		}
	}
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub

	}

}
