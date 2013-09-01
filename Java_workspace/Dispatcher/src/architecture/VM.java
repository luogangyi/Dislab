package architecture;
import model.*;


public class VM {

	private static int counter=0;
	public int vm_id;//=counter++;
	//public int group_id;
	public int job_id;//or userid
	public Job job;
	public int req_cpu;
	public int req_mem;
	public String virtType; 
	public int group_id;
	public int inner_vm_id; //group inner id					//every vm in a job has a inner_vm_id, which starts from 0
	public Node node;							//the physical machine it uses
	
	
	public VM(int cpu, int mem, int job_id, Job job,int vm_id, int inner_vm_id)
	{
		req_cpu = cpu;
		req_mem = mem;
		this.vm_id = vm_id;
		this.job_id = job_id;
		this.job = job;
		this.inner_vm_id = inner_vm_id;
	}
	
	public VM(int cpu, int mem, int group_id, int vm_id,int inner_vm_id,String virtType)
	{
		req_cpu = cpu;
		req_mem = mem;
		this.vm_id = vm_id;
		this.group_id = group_id;
		this.inner_vm_id = inner_vm_id;
		this.virtType = virtType;
	}
	
	public VM(VMDetail vmd)
	{
		req_cpu = vmd.getVCPU();
		req_mem = vmd.getRAM();
		this.vm_id = vmd.getID();
		this.group_id = vmd.getGroupID();
		this.inner_vm_id = vmd.getGroupInnerID();
		this.virtType = vmd.getVirtType();
	}
	
	
	public Node getNode() {
		return node;
	}

	public void setNode(Node node) {
		this.node = node;
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "VM [vm_id=" + vm_id + ", req_cpu=" + req_cpu + ", req_mem="
				+ req_mem + ", virtType=" + virtType + ", group_id=" + group_id
				+ ", inner_vm_id=" + inner_vm_id + "]";
	}
}