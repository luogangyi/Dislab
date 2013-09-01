package architecture;

import java.util.*;
/**
 * Each Node represent a physical server.
 * Each Node belongs to a rack(which has a unique rack id)
 * @author lgy
 *
 */
public class Node {
	public static int counter=0;
	public static int running_counter = 0;
	public int node_id;			/*unique id for each physical server*/
	private int cpu_total;								/*cpu count*/
	private int cpu_left;								/*cpu used count*/
	private int mem_total;								/*total memory size*/
	private int mem_left;								/*used menory size*/
	public int rack_id;								/*the rack it belongs*/
	
	public int group_id;								/*the group it belongs*/
	public int datacenter_id;							/*the datacenter it belongs*/
	
	public String IP;
	public String virtType;
	public int node_traffic;
	public Rack rack;
	public RackGroup group;
	public DataCenter datacenter;
	
	String update_at;
	
	private boolean status=false;					//initial status = false, meaning machine is turned off
	public double mem_rate = 0;
	public double cpu_rate = 0;
	private Map<Integer,VM> vm_map = new HashMap<Integer,VM>();
	
	public void addVM(VM vm)
	{
		vm_map.put(vm.vm_id, vm);
	}
	
	
	public void removeVM(VM vm)
	{
		removeVM(vm.vm_id);
	}
	
	public void removeAllVM()
	{
		vm_map.clear();
	}
	public void removeVM(int vm_id)
	{
		vm_map.remove(vm_id);
		//所有虚拟机退出时
		if(vm_map.size()==0)
			setStatus(false);
	} 
	
	public boolean isExist(int vm_id)
	{
		return vm_map.containsKey(vm_id);
	}
	
	
	public Collection<VM> get_vm_list()
	{
		return vm_map.values();
	}
	
	public int getCpu_total() {
		return cpu_total;
	}

	public void setCpu_total(int cpu_total) {
		this.cpu_total = cpu_total;
	}

	public int getMem_total() {
		return mem_total;
	}

	public void setMem_total(int mem_total) {
		this.mem_total = mem_total;
	}

	public int getRack_id() {
		return rack_id;
	}

	public void setRack_id(int rack_id) {
		this.rack_id = rack_id;
	}

	public int getGroup_id() {
		return group_id;
	}

	public void setGroup_id(int group_id) {
		this.group_id = group_id;
	}

	public int getDatacenter_id() {
		return datacenter_id;
	}

	public void setDatacenter_id(int datacenter_id) {
		this.datacenter_id = datacenter_id;
	}
	
	public int getCpu_left() {
		return cpu_left;
	}

	//注意！！以行惨（数据库中的值）为准
	public void setCpu_left(int cpu_left) {
		if(cpu_left>this.cpu_left)
		{
			releaseCpu(cpu_left -this.cpu_left );
		}
		else if(cpu_left<this.cpu_left)
		{
			requireCpu(this.cpu_left - cpu_left);
		}
		else
		{
			//没有变化时，什么都不做。
			return;
		}
	}
	public void requireCpu(int cpu_require) {
		this.cpu_left = cpu_left - cpu_require;
		this.cpu_rate = (double)(cpu_total - cpu_left)/cpu_total;
		datacenter.setTotal_cpu_used(datacenter.getTotal_cpu_used()+cpu_require);
	}
	
	public void releaseCpu(int cpu_release) {
		this.cpu_left = cpu_left + cpu_release;
		this.cpu_rate = (double)(cpu_total - cpu_left)/cpu_total;
		datacenter.setTotal_cpu_used(datacenter.getTotal_cpu_used()-cpu_release);
	}
	
	public int getMem_left() {
		return mem_left;
	}

	//注意！！以行惨（数据库中的值）为准
	public void setMem_left(int mem_left) {
		if(mem_left >this.mem_left)
		{
			releaseMem(mem_left -this.mem_left );
		}
		else if(mem_left < this.mem_left)
		{
			requireMem(this.mem_left - mem_left);
		}
		else
		{
			return;
		}
	}
	public void requireMem(int mem_require) {
		this.mem_left = mem_left - mem_require;
		this.mem_rate = (double)(mem_total - mem_left)/mem_total;
		datacenter.setTotal_mem_used(datacenter.getTotal_mem_used()+mem_require);
	}
	
	public void releaseMem(int mem_release) {
		this.mem_left = mem_left + mem_release;
		this.mem_rate = (double)(mem_total - mem_left)/mem_total;
		datacenter.setTotal_mem_used(datacenter.getTotal_mem_used()- mem_release);
	}

	public boolean getStatus() {
		return status;
	}

	public void setStatus(boolean status) {

		//改变要反应在整个数据中心上
		if(status == true && this.status==false)
		{
			datacenter.setTotal_cpu(datacenter.getTotal_cpu()+this.cpu_total);
			datacenter.setTotal_mem(datacenter.getTotal_mem()+this.mem_total);
			running_counter++;
		}
		else if(status ==false && this.status==true)
		{
			datacenter.setTotal_cpu(datacenter.getTotal_cpu()-this.cpu_total);
			datacenter.setTotal_mem(datacenter.getTotal_mem()-this.mem_total);
			running_counter--;
		}
		this.status = status;
	}
	
	//兼容0,1
	public void setStatus(int status) {

		if(status == 1)
			setStatus(true);
		else
			setStatus(false);
	}

	public Node(int cpu_total, int mem_total,String IP,String virtType,
			int nc_id,int rack_id, int group_id, int datacenter_id, Rack rack, RackGroup group,DataCenter datacenter,int status,String update_at)
	{
		super();
		this.node_id = nc_id;
		this.cpu_total = cpu_total;
		this.mem_total = mem_total;
		this.rack_id = rack_id;
		this.group_id = group_id;
		this.datacenter_id = datacenter_id;
		this.virtType =virtType;
		this.IP = IP;
		this.rack = rack;
		this.group = group;
		this.datacenter = datacenter;
		
		if(status == 1)
			this.status = true;
		else
			this.status = false;
		
		this.update_at = update_at;
		
		cpu_left = cpu_total;
		mem_left = mem_total;
	}


	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "Node [node_id=" + node_id + ", cpu_total=" + cpu_total
				+ ", cpu_left=" + cpu_left + ", mem_total=" + mem_total
				+ ", mem_left=" + mem_left + ", rack_id=" + rack_id
				+ ", group_id=" + group_id + ", datacenter_id=" + datacenter_id
				+ ", IP=" + IP + ", virtType=" + virtType + ", status="
				+ status + "]";
	}
	
}