package lgy;
import java.io.*;

public class DataFilter {

	static void initiateDataCenter(int datacenter_id, DataCenter datacenter) throws Exception
	{
		String temp_line;
		String []info;
		BufferedReader br = new  BufferedReader(
				new FileReader("emulational_datacenter_architecture.swf"));
			
		temp_line = br.readLine();
		while(temp_line != null)
		{
			if(!temp_line.startsWith(";"))
			{
				System.out.println("#"+temp_line);
				info = temp_line.split("\\s+");
				
				Group g = new Group(info,datacenter_id,datacenter);
				datacenter.addGroup(g);
				/**********for test***************
				for(int i=0;i<info.length;i++)
				{
					System.out.print(info[i]+"#");
				}
				System.out.println();
				***********************************/
			}
			temp_line = br.readLine();
		}
	}
	/**for test
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		//System.out.println("Hello World!");
		
		String temp_line;
		String []info;
		try{
			BufferedReader br = new  BufferedReader(
					new FileReader("emulational_datacenter_architecture.swf"));
			
			
			
			temp_line = br.readLine();
			while(temp_line != null)
			{
				if(!temp_line.startsWith(";"))
				{
					System.out.println("#"+temp_line);
					info = temp_line.split("\\s+");

					for(int i=0;i<info.length;i++)
					{
						System.out.print(info[i]+"#");
					}
					System.out.println();
				}
				temp_line = br.readLine();
			}
			
			
		}catch(Exception e)
		{
			System.out.println();
		}
	}

}
