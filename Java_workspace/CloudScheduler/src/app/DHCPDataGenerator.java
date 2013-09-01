package app;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.*;
import java.io.*;

public class DHCPDataGenerator {
	
	public static String interface_monitor = "br0";
	public static String localIP;
    // MySQL配置时的用户名
    private static String user = "root"; 
    private static String configFilename = "nccfg.cfg";
    // MySQL配置时的密码
    private static String password = "123456";
    //监控时间间隔，单位为 分钟(min)
    private static int inteval = 1;
	private static Connection conn;

	public static Connection getMysqlConn()
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
	
	public static String convertIntToHex(int i)
	{
		if(i>=256)
		{
			System.out.println("ERROR!");
			return "";
		}
		if(i<16)
		{
			return "0"+Integer.toHexString(i);
		}
		else
			return Integer.toHexString(i);
	}

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try
		{
			File file = new File("DHCP_DATA.txt");
			FileOutputStream fos = new FileOutputStream(file);
			OutputStreamWriter osw = new OutputStreamWriter(fos,"UTF-8");
			BufferedWriter bw = new BufferedWriter(osw);
			
			conn = getMysqlConn();
			//host dislab {
			//  hardware ethernet 00:16:3e:00:00:0a;
			//  fixed-address 192.168.101.110;
			//}
            Statement statement = conn.createStatement();
            //ResultSet rs = statement.executeQuery(sql);
			for(int i=1;i<140;i++)
			{
				String s = "host dislab_VM_" +i+" {\n";
				s += "\t hardware ethernet 00:16:3e:00:00:";
				s += convertIntToHex(i)+";\n";
				s += "\t fixed-address 192.168.101.";
				s += (100+i);
				s += ";\n";
				s +="}\n";
				System.out.print(s);
				String sql = "insert into MACIP (MAC,IP,VMID) values ('00:16:3e:00:00:"+convertIntToHex(i)+
						"','192.168.101."+(100+i)+"',0)";
				statement.execute(sql);

			}
			
//			bw.write(result.toString());
//			bw.flush();
//			bw.close();
		}catch(Exception e)
		{
			e.printStackTrace();
		}
	}

}
