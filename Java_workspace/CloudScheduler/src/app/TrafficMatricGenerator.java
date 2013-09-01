package app;
import java.util.*;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.sql.*;

import model.*;


public class TrafficMatricGenerator {
	
	public static String localIP;
    // MySQL配置时的用户名
    private static String user = "root"; 
    private static String configFilename = "nccfg.cfg";
    // MySQL配置时的密码
    private static String password = "123456";
    private static String outputFilePath = "TrafficData/";

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
	 * 获取所有的Group id 返回List<group id>
	 * @return
	 */
	public List<Integer> getGroupIDList()
	{
		String sql="select * from Group ";
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
	 * 根据group id 获取该group中的所有VM，结果按GroupInnerID升序排序
	 * @param group_id
	 * @return
	 */
	public List<VMDetail> getVMListFromGroupID(int group_id)
	{

		List<VMDetail> vm_list = new LinkedList<VMDetail>();
		VMDetail temp;
		try{

			String sql="select * from VMDetail where GroupID = "+group_id +" order by GroupInnerID ASC" ;
			// statement用来执行SQL语句
			Statement statement = conn.createStatement();
			ResultSet rs = statement.executeQuery(sql);
			while(rs.next()) {

				temp = new  VMDetail();
				temp.setID(Integer.parseInt(rs.getString("ID")));
				temp.setGroupID(Integer.parseInt(rs.getString("GroupID")));
				temp.setGroupInnerID(Integer.parseInt(rs.getString("GroupInnerID")));
				vm_list.add(temp);
				System.out.println("GroupID: "+temp.getGroupID()+" ;GroupInnerID: "+ temp.getGroupInnerID()+
						"VMID: "+ temp.getID());
			}
			return vm_list;


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
		//String vm_ip = getIPfromID(vm_id);
		try{
            
			String sql="select Data from TrafficData where VMID = '"+vm_id+"'" ;
            // statement用来执行SQL语句
			Statement statement = conn.createStatement();
            ResultSet rs = statement.executeQuery(sql);
            String s_data = null;
            if(rs.next()) {
             // 选择sname这列数据
            	s_data = rs.getString("Data");
            	System.out.println(vm_id+" traffic data: "+s_data);
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
	 * ***********注意！！！vm_list必须是按GroupInnerID排序**********
	 * ****************！！！Data也必须是按GroupInnerID排序**********
	 * **********************************************************
	 * 根据一组虚拟机的ID,从数据库中的记录生成TrafficMatrix文件
	 * 文件名为Traffic_Matrix_<group id>.txt
	 */
	public void generateTrafficMatrixFile(int group_id,List<VMDetail> vm_list)
	{	
		StringBuffer result= new StringBuffer();
		for(VMDetail vm :vm_list)
		{
			String trafficData =getTrafficDataFromVMID(vm.getID());
			String[] temp_arr = trafficData.split(",");
			String[] key_value;
			for(int i=0;i<temp_arr.length;i++)
			{
				key_value = temp_arr[i].split(":");
				if(key_value.length != 2)
				{
					System.out.println("Traffic Data Format Error!");
					continue;
				}
				result.append(key_value[1]+"\t");
			}
			result.append("\n");
		}
		
		//将result写入文件中
		try
		{
			File file = new File(outputFilePath+"Traffic_Matrix_"+group_id+".txt");
			FileOutputStream fos = new FileOutputStream(file);
			OutputStreamWriter osw = new OutputStreamWriter(fos,"UTF-8");
			BufferedWriter bw = new BufferedWriter(osw);
			bw.write(result.toString());
			bw.flush();
			bw.close();
		}catch(Exception e)
		{
			e.printStackTrace();
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
			List<VMDetail> vm_list = getVMListFromGroupID(group_id);
			//注意！！！vm_list必须是按GroupInnerID排序
			generateTrafficMatrixFile(group_id,vm_list);
		}
	}
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub

		TrafficMatricGenerator tmg = new TrafficMatricGenerator();
		//每30分钟更新一次traffic矩阵
		while(true)
		{
			try{
				tmg.generateTrafficMatrix();
				Thread.sleep(1000*60*30);
			}catch(Exception e)
			{
				e.printStackTrace();
			}
		}
		
	}

}
