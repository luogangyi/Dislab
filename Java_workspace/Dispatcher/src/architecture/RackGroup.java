package architecture;
import java.util.*;
/**
 * Each Group represent a group of racks.
 * Each Group belongs to a datacenter(which has a unique datacenter id)
 * @author lgy
 *
 */
public class RackGroup {
	public static int counter=0;
	public int group_id;			/*unique id for each group*/
	public int datacenter_id;
	public DataCenter datacenter;
	public int group_traffic;
	public int status;//0-关，1-开
	public Map<Integer,Rack> rack_map=new HashMap<Integer,Rack>();
	
	/**
	 * 
	 * @param datacenter_id
	 */
	public RackGroup(int datacenter_id)
	{
		this.datacenter_id=datacenter_id;
	}
	/**
	 * 
	 * @param group_id
	 * @param datacenter_id
	 * @param datacenter
	 * @param status
	 */
	public RackGroup(int group_id,int datacenter_id ,DataCenter datacenter,int status)
	{
		this.datacenter = datacenter;
		this.datacenter_id=datacenter_id;
		this.group_id = group_id;
		this.status = status;
	}

	
	/**
	 * 
	 * @param rack_amount
	 * @param server_amount
	 * @param cpu_total
	 * @param mem_total
	 * @param datacenter_id
	 */
//	public RackGroup(int rack_amount,int server_amount, int cpu_total, int mem_total,
//			  	int datacenter_id,DataCenter datacenter)
//	{
//		this.datacenter_id=datacenter_id;
//		this.datacenter = datacenter;
//
//		this.group_id = counter++;
//		
//		Rack temp;
//		for(int i=0;i<rack_amount;i++)
//		{
//			temp= new Rack(server_amount, cpu_total, mem_total, group_id, datacenter_id,this,datacenter);
//			rack_map.put(temp.rack_id,temp);
//		}
//		System.out.println("rack amount: "+rack_map.size());
//	}	
	
	/**
	 * 
	 * @param info
	 */
//	public RackGroup(String []info, int datacenter_id,DataCenter datacenter)
//	{
//		this(Integer.parseInt(info[1]), Integer.parseInt(info[2]), Integer.parseInt(info[4]), Integer.parseInt(info[5]), datacenter_id,datacenter);		
//	}
	
	public void addRack(Rack r)
	{
		rack_map.put(r.rack_id, r);
	}
}
