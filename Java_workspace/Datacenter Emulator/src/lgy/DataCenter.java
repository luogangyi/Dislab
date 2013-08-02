package lgy;
import java.util.*;

public class DataCenter {
	public  List<Group> group_list = new LinkedList<Group>();
	public  int datacenter_traffic;
	public  double total_cpu_rate = 1;
	public  double total_mem_rate = 1;
	private long total_mem = 0;
	private long total_cpu = 0;
	private long total_mem_used = 0;
	private long total_cpu_used = 0;

	public long getTotal_cpu_left() {
		return total_cpu-total_cpu_used;
	}


	public long getTotal_mem_left() {
		return total_mem-total_mem_used;
	}


	public void addGroup(Group g)
	{
		group_list.add(g);
	}
	
	public Iterator<Group> getGroupList()
	{
		return group_list.iterator();
	}

	public long getTotal_mem() {
		return total_mem;
	}

	public void setTotal_mem(long total_mem) {
		this.total_mem = total_mem;
		this.total_mem_rate =(double) this.total_mem_used/this.total_mem;
	}

	public long getTotal_cpu() {
		return total_cpu;
	}

	public void setTotal_cpu(long total_cpu) {
		this.total_cpu = total_cpu;
		this.total_cpu_rate =(double) this.total_cpu_used/this.total_cpu;
	}

	public long getTotal_mem_used() {
		return total_mem_used;
	}

	public void setTotal_mem_used(long total_mem_used) {
		this.total_mem_used = total_mem_used;
		this.total_mem_rate =(double) this.total_mem_used/this.total_mem;
	}

	public long getTotal_cpu_used() {
		return total_cpu_used;
	}

	public void setTotal_cpu_used(long total_cpu_used) {
		this.total_cpu_used = total_cpu_used;
		this.total_cpu_rate =(double) this.total_cpu_used/this.total_cpu;
	}
	
}
