package lgy;

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
	public final int node_id=counter++;			/*unique id for each physical server*/
	private int cpu_total;								/*cpu count*/
	private int cpu_left;								/*cpu used count*/
	private int mem_total;								/*total memory size*/
	private int mem_left;								/*used menory size*/
	public int rack_id;								/*the rack it belongs*/

	public int group_id;								/*the group it belongs*/
	public int datacenter_id;							/*the datacenter it belongs*/
	
	public int node_traffic;
	public Rack rack;
	public Group group;
	public DataCenter datacenter;
	
	private boolean status=false;					//initial status = false, meaning machine is turned off
	public double mem_rate = 0;
	public double cpu_rate = 0;
	private List<VM> vm_list = new LinkedList<VM>();
	
	public void addVM(VM vm)
	{
		vm_list.add(vm);
	}
	
	public void removeVM(VM vm)
	{
		vm_list.remove(vm);
		//所有虚拟机退出时
		if(vm_list.size()==0)
			setStatus(false);
	}
	
	public List<VM> get_vm_list()
	{
		return vm_list;
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

	public void setCpu_left(int cpu_left) {
		this.cpu_left = cpu_left;
		this.cpu_rate = (double)(cpu_total - cpu_left)/cpu_total;
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

	public void setMem_left(int mem_left) {
		this.mem_left = mem_left;
		this.mem_rate = (double)(mem_total-mem_left)/mem_total;
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

	public Node(int cpu_total, int mem_total,
			int rack_id, int group_id, int datacenter_id, Rack rack, Group group,DataCenter datacenter)
	{
		super();
		this.cpu_total = cpu_total;
		this.mem_total = mem_total;
		this.rack_id = rack_id;
		this.group_id = group_id;
		this.datacenter_id = datacenter_id;
		
		this.rack = rack;
		this.group = group;
		this.datacenter = datacenter;
		
		cpu_left = cpu_total;
		mem_left = mem_total;
	}
}
