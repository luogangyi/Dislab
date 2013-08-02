package lgy;

import java.io.*;
import java.util.*;

public class Emulator {

	static DataCenter datacenter = new DataCenter();
	static BufferedReader br ;
	static BufferedWriter bw;
	static BufferedWriter bw2;
	static BufferedWriter bw_benefit;
	static BufferedWriter bw2_benefit;
	static Node max_node;
	static List<Job> job_execute_list = new LinkedList<Job>();//a job list which is actually running.
	static Stack <MigrationRecord> m_stack = new Stack<MigrationRecord>();
	static double threshold = 0.99;
	static double recon_threshold = 0.95;
	static int migration_counter=0;
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try{
			DataFilter.initiateDataCenter(0,datacenter);	
		}
		catch(Exception e)
		{
			System.out.println("datacenter initailizaion failed!");
			return;
		}
		
		try{
			br= new  BufferedReader(
					new FileReader("TestDataGenerator/Modified_Log_49.txt"));
			
			bw = new BufferedWriter(new FileWriter("Job_Status_"+new Date()+".txt"));
			bw.write("Time\t"+"Total\t"+"Dispersed\t"+"Machine_Used\n"+"Descirption: First-fit,greedy-recon/600s,threshold = 0.99; recon_threshold = 0.95;");
			bw.flush();
			
			
			bw_benefit = new BufferedWriter(new FileWriter("Job_Status_benefit_"+new Date()+".txt"));
			bw_benefit.write("Time\t"+"Current\n"+"Descirption: First-fit,greedy-recon/600s,threshold = 0.99; recon_threshold = 0.95;");
			bw_benefit.flush();
			
		}
		catch(Exception e)
		{
			System.out.println("Job Log Open failed!");
			return;
		}
		//Network_aware();
		First_fit();
		try{
			br.close();
			
		}
		catch(Exception e)
		{
			System.out.println("Job Log Close failed!");
			return;
		}

	}
	
	public static void Network_aware()
	{
		long timer_counter = 0;
		
		Job job = initializeJobFromLog();
		while(job!=null ||job_execute_list.size()>0)
		{
			if(job!=null &&timer_counter >= job.start_time)
			{
				//System.out.println("asigning..");
				if(assignJob_network_aware(job)==true)
				{
					job_execute_list.add(job);
					//printJobStatus(job);
					//System.out.println("Assign Job:"+job.job_id+" success!");
				}
				else
				{
					System.out.println("Assign Job:"+job.job_id+" failed!");
				}
				job = initializeJobFromLog();
			}
			timer_counter++;
			executeJob();
			
			if(timer_counter%60==0)
			{
				//printInfo();//print current status every 30min;
				//reconsolidate_greedy(timer_counter);
				if(timer_counter%600==0)
					searchDispersedJob(timer_counter);
				//printTrafficInfo();
			}
		}
	}
	
	public static void First_fit()
	{
		long timer_counter = 0;
		
		Job job = initializeJobFromLog();
		while(job!=null ||job_execute_list.size()>0)
		{
			if(job!=null &&timer_counter >= job.start_time)
			{
				//System.out.println("asigning..");
				if(assignJob_first_fit(job)==true)
				{
					job_execute_list.add(job);
					//printJobStatus(job);
					//System.out.println("Assign Job:"+job.job_id+" success!");
				}
				else
				{
					System.out.println("Assign Job:"+job.job_id+" failed!");
				}
				job = initializeJobFromLog();
			}
			timer_counter++;
			executeJob();
			
			if(timer_counter%600==0)
			{
				//printInfo();//print current status every 30min;
				reconsolidate_greedy(timer_counter);
				//reconsolidate_random(timer_counter);
				if(timer_counter%600==0)
					searchDispersedJob(timer_counter);
				//printTrafficInfo();
			}
		}
	}
	
	public static String[] getJobInfo()
	{
		try{
			String templine=br.readLine();
			while(templine!=null && templine.startsWith(";"))
			{
				templine = br.readLine();
			}
			if(templine==null)
				return null;
			else
			{
				return templine.split("\\s+");
			}
		}
		catch(Exception e)
		{
			System.out.println("Read Job Log Error!");
			return null;
		}

	}
	public static Job initializeJobFromLog()
	{
		//System.out.println("initialize..");
		String[] jobInfo = getJobInfo();
		if(jobInfo==null)
			return null;
		else
		{
			int job_id = Integer.parseInt(jobInfo[0]);
			int start_time = Integer.parseInt(jobInfo[1]);
			int time_used = Integer.parseInt(jobInfo[2]);
			int cpu_used = Integer.parseInt(jobInfo[3]);
			int mem_total = Integer.parseInt(jobInfo[4]);
			int parted = Integer.parseInt(jobInfo[6]);
			int mem_parted = Integer.parseInt(jobInfo[5]); ;
			Job job = new Job(job_id, start_time, time_used, cpu_used,
					mem_total, parted, mem_parted);
			return job;
		}
	}
	/**
	 * assign VM to job,
	 * if success, then return true;
	 * else return false;
	 * @param job
	 * @return
	 */
	public static boolean assignJob_network_aware(Job job)
	{
		VM []vm_arr = new VM[job.parted];
		VM temp;
		for(int i=0; i<job.parted;i++)//apply 'parted' VM 
		{
			temp = new VM(job.cpu_parted, job.mem_parted, job.job_id,job, i); //note that "i" equals inner_vm_id
			vm_arr[i] = temp;			
		}
		if(minHeightTree(vm_arr,job.parted))
		{
			for(int i=0; i<job.parted;i++)//apply 'parted' VM 
			{
				job.vm_list.add(vm_arr[i]);			
			}
			return true;	
		}
		return false;
	}
	
	public static boolean assignJob_first_fit(Job job)
	{
		VM temp;
		for(int i=0; i<job.parted;i++)//apply 'parted' VM 
		{
			temp = new VM(job.cpu_parted, job.mem_parted, job.job_id,job, i); //note that "i" equals inner_vm_id
			if(findNode(temp)==false)
				return false;
			//System.out.println("Job "+job.job_id+" Time begin: "+job.start_time+"; Time total: " + job.time_left);
			job.vm_list.add(temp);			
		}
		return true;
	}
	
	public static boolean minHeightTree(VM[]vm_arr,int parted)
	{
		if(try_allocate_in_rack(vm_arr))
			return true;
		else if(try_allocate_in_group(vm_arr))
			return true;
		else if(try_allocate_in_datacenter(vm_arr))
			return true;
		return false;
	}
	/**
	 * 
	 * @param vm_arr
	 * @return
	 */
	public static boolean try_allocate_in_rack(VM[] vm_arr)
	{
		for(Group g:datacenter.group_list)
		{
			for(Rack r:g.rack_list)
			{
				if(allocate_in_specific_rack(vm_arr,r))
					return true;
			}
		}
		//全部尝试失败，则返回失败
		return false;
	}
	
	private static boolean allocate_in_specific_rack(VM[] vm_arr,Rack r)
	{
		VM vm;
		int success_counter = 0;
		boolean flag = false;
		for(int i=0;i<vm_arr.length;i++)
		{
			vm = vm_arr[i];
			flag = false;
			for(Node n:r.node_list)
			{
				if(flag ==true)
					break;
				if(n.getStatus()==false)
				{
					if(datacenter.total_cpu_rate>threshold||datacenter.total_mem_rate>threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
						n.setStatus(true);
					else
						continue;
				}
				//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
				if(n.getCpu_left()>= vm.req_cpu && n.getMem_left() >= vm.req_mem)
				{
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					n.requireCpu(vm.req_cpu);
					n.requireMem(vm.req_mem);
					vm.node = n;
					
					//put this VM into the node's vm_list!
					n.addVM(vm);
					
					success_counter++;
					flag = true;
				}
			}
		}
		if(success_counter==vm_arr.length)
			return true;
		else//尝试失败，则释放刚才分配的资源
			retreat_allocated_vms(vm_arr,success_counter);

		return false;
	}
	/**
	 * 
	 * @param vm_arr
	 * @return
	 */
	public static boolean try_allocate_in_group(VM[] vm_arr)
	{
		for(Group g:datacenter.group_list)
		{
			if(allocate_in_specific_group(vm_arr,g))
				return true;
		}
		//全部尝试失败，则返回失败
		return false;
	}
	
	private static boolean allocate_in_specific_group(VM[] vm_arr,Group g)
	{
		VM vm;
		int success_counter = 0;
		boolean flag = false;
		for(int i=0;i<vm_arr.length;i++)
		{
			flag =false;
			vm = vm_arr[i];
			for(Rack r:g.rack_list)
			{
				if(flag == true)
					break;
				for(Node n:r.node_list)
				{
					if(flag == true)
						break;
					if(n.getStatus()==false)
					{
						if(datacenter.total_cpu_rate>threshold||datacenter.total_mem_rate>threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
							n.setStatus(true);
						else
							continue;
					}
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					if(n.getCpu_left()>= vm.req_cpu && n.getMem_left() >= vm.req_mem)
					{
						//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
						n.requireCpu(vm.req_cpu);
						n.requireMem(vm.req_mem);
						vm.node = n;
						
						//put this VM into the node's vm_list!
						n.addVM(vm);
						
						success_counter++;
						flag = true;
					}
				}	
			}
		}
		if(success_counter==vm_arr.length)
			return true;
		else//尝试失败，则释放刚才分配的资源
			retreat_allocated_vms(vm_arr,success_counter);
		return false;
	}
	public static boolean try_allocate_in_datacenter(VM[] vm_arr)
	{

		VM vm;
		int success_counter = 0;
		boolean flag = false;
		for(int i=0;i<vm_arr.length;i++)
		{
			flag =false;
			vm = vm_arr[i];
			for(Group g:datacenter.group_list)
			{
				for(Rack r:g.rack_list)
				{
					if(flag == true)
						break;
					for(Node n:r.node_list)
					{
						if(flag == true)
							break;
						if(n.getStatus()==false)
						{
							if(datacenter.total_cpu_rate>threshold||datacenter.total_mem_rate>threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
								n.setStatus(true);
							else
								continue;
						}
						//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
						if(n.getCpu_left()>= vm.req_cpu && n.getMem_left() >= vm.req_mem)
						{
							//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
							n.requireCpu(vm.req_cpu);
							n.requireMem(vm.req_mem);
							vm.node = n;
							
							//put this VM into the node's vm_list!
							n.addVM(vm);
							
							success_counter++;
							flag = true;
						}
					}	
				}
			}

		}
		if(success_counter==vm_arr.length)
			return true;
		else//尝试失败，则释放刚才分配的资源
			retreat_allocated_vms(vm_arr,success_counter);
		return false;
	
	}
	
	//配分未成功时，释放已分配的资源。
	public static void retreat_allocated_vms(VM[]vm_arr,int success_counter)
	{
		for(int i=0;i<success_counter;i++)
		{
			VM vm = vm_arr[i];
			Node n = vm_arr[i].node;
			//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
			n.releaseCpu(vm.req_cpu);
			n.releaseMem(vm.req_mem);

			vm.node = null;
			
			//put this VM into the node's vm_list!
			n.removeVM(vm);
		}
	}
	/**
	 * assign a VM to physical Node,
	 * if success, return true;
	 * else return false; 
	 * @param vm
	 * @return
	 */
	public static boolean findNode(VM vm)
	{
		//System.out.println("searching..");
		for(Group g:datacenter.group_list)
		{
			for(Rack r:g.rack_list)
			{
				for(Node n:r.node_list)
				{
					if(n.getStatus()==false)
					{
						if(datacenter.total_cpu_rate>threshold||datacenter.total_mem_rate>threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
							n.setStatus(true);
						else
							continue;
					}
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					if(n.getCpu_left()>=vm.req_cpu && n.getMem_left() >= vm.req_mem)
					{
						//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
						n.requireCpu(vm.req_cpu);
						n.requireMem(vm.req_mem);

						vm.node = n;
						
						if(max_node==null)
							max_node = n;
						else if(max_node.node_id<n.node_id)
							max_node = n;
						//put this VM into the node's vm_list!
						n.addVM(vm);
						
						//if(g.group_id>8)
						//System.out.println("VM "+vm.vm_id+" is located on: Group "+g.group_id+" Rack "
						//		+r.rack_id+" Node "+n.node_id);
						return true;
					}
				}
			}
		}
		return false;
	}
	/**
	 * 
	 */
	public static void executeJob()
	{
		List<Job> finished_job_list = new LinkedList<Job> ();
	
		for(Job job:job_execute_list)
		{
			job.time_left--;
			if(job.time_left<=0)
			{
				for(VM vm:job.vm_list)
				{
					//return the resource it consumes;
					vm.node.releaseCpu(vm.req_cpu);
					vm.node.releaseMem(vm.req_mem);
					
					//remove this vm from its physical server!
					vm.node.removeVM(vm);
				}
				//System.out.println("Job "+job.job_id+" is finished!");
				
				finished_job_list.add(job);
				//job_execute_list.remove(job);
			}
		}
		
		job_execute_list.removeAll(finished_job_list);//remove all finished jobs.
	}
	
	public static void printInfo()
	{
		for(Job job:job_execute_list)
		{
			int [][] route_table = new int[job.parted][job.parted];
			
			int i=0,j=0;
			
			
			//compute route table of all the VMs a job has.
			//if both VM on a same node, then distance=0;
			//else if both VM on a same rack, then distance=1
			//else if both VM on a same group, then distance=2
			//else if both VM on a same datacenter, then distance=3;
			for(VM vm:job.vm_list)
			{
				j=0;
				for(VM vm_l:job.vm_list)
				{
					Node n1,n2;
					n1 = vm.node;
					n2 = vm_l.node;
					
					if(n1.node_id == n2.node_id)
						route_table[i][j]=0;
					else if(n1.rack_id == n2.rack_id)
						route_table[i][j]=0;
					else if(n1.group_id == n2.group_id)
						route_table[i][j]=1;
					else
						route_table[i][j]=2;
					j++;
				}
				i++;
			}
			
			double result = 0;
			double best = 0;
			for(int i2=0; i2<job.parted; i2++)
			{
				for(int j2=0; j2<job.parted; j2++)
				{
					best += job.traffic_matrix[i2][j2];
					result += job.traffic_matrix[i2][j2] /Math.pow(5, route_table[i2][j2]);
				}
			}
			if(best > result*3)
				System.out.println("Job "+job.job_id+": benefit = "+ result+" best: "+best);
		}
	}
	
	public static void printJobStatus(Job job)
	{
		int [][] route_table = new int[job.parted][job.parted];
		
		int i=0,j=0;
		
		
		//compute route table of all the VMs a job has.
		//if both VM on a same node, then distance=0;
		//else if both VM on a same rack, then distance=1
		//else if both VM on a same group, then distance=2
		//else if both VM on a same datacenter, then distance=3;
		for(VM vm:job.vm_list)
		{
			j=0;
			for(VM vm_l:job.vm_list)
			{
				Node n1,n2;
				n1 = vm.node;
				n2 = vm_l.node;
				
				if(n1.node_id == n2.node_id)
					route_table[i][j]=0;
				else if(n1.rack_id == n2.rack_id)
					route_table[i][j]=0;
				else if(n1.group_id == n2.group_id)
					route_table[i][j]=1;
				else
					route_table[i][j]=2;
				j++;
			}
			i++;
		}
		
		double result = 0;
		double best = 0;
		for(int i2=0; i2<job.parted; i2++)
		{
			for(int j2=0; j2<job.parted; j2++)
			{
				best += job.traffic_matrix[i2][j2];
				result += job.traffic_matrix[i2][j2] /Math.pow(5, route_table[i2][j2]);
			}
		}
		if(best > result*3)
			System.out.println("Job "+job.job_id+": benefit = "+ result+" best: "+best);
	}
	
	public static void reconsolidate_greedy(long current_time)
	{
		migration_counter=0;
		//searchLowUsageNode();
		//searchDispersedJob(current_time);
		moveDispersedJob(current_time);
		//searchDispersedJob2(current_time);
		System.out.println("Mirgration Counter: "+ migration_counter);
	}
	
	public static void reconsolidate_random(long current_time)
	{
		List<Job> job_list;
		List<Job> best_job_list = new LinkedList<Job>();
		int dispersed_counter;
		int best_dispersed_counter = -1;
		job_list = random_shuffle_job(job_execute_list);
		best_job_list.addAll(job_list);
		for(int i=0;i<100;i++)
		{		
			m_stack.clear();
			dispersed_counter=do_random_recon_once(current_time,job_list);
			if(best_dispersed_counter == -1)
			{
				best_dispersed_counter = dispersed_counter;
			}
			else if(dispersed_counter < best_dispersed_counter)
			{
				best_dispersed_counter = dispersed_counter;
				best_job_list.clear();
				best_job_list.addAll(job_list);
			}
			retreat_migration();
			job_list = random_shuffle_job(job_execute_list);
		}
		do_random_recon_once(current_time,best_job_list);
	}
	/**
	 * 
	 * @return disperse value
	 */
	public static int do_random_recon_once(long current_time, List<Job> job_list)
	{
		m_stack.clear();
		int dispersed_job_counter =  0;
		double total_result0=0;
		double total_result1=0;
		
		//static List<Job> job_execute_randomed = job_execute_list.
		for(Job job:job_list)
		{
			int [][] route_table = new int[job.parted][job.parted];
			
			int i=0,j=0;
			
			int dispersed_counter = 0;
			
			dispersed_counter = computeRouteTable(job, route_table);

			
			double result = 0;
			double best = 0;
			for(int i2=0; i2<job.parted; i2++)
			{
				for(int j2=0; j2<job.parted; j2++)
				{
					best += job.traffic_matrix[i2][j2];
					result += job.traffic_matrix[i2][j2] /Math.pow(5, route_table[i2][j2]);
				}
			}
			
			dispersed_counter = dispersed_counter/2;
			int pair = job.vm_list.size()*job.vm_list.size()-job.vm_list.size();
			
			if(dispersed_counter >=1)
			{
				if(best>=1)
					total_result0+=result/best;
				else total_result0++;
				
				if(shuffle_in_node_2(job)==false)
				{
					if(shuffle_in_rack_2(job)==false)
					{
						//System.out.println("Shuffle job "+job.job_id+ "failed!");
						//return -1;
					}
				}
				//shuffle_in_group(job);
			}
		}
		return searchDispersedJob_2(current_time);
	}
	
	//撤销所有迁移操作
	public static void retreat_migration()
	{
		MigrationRecord m;
		while(!m_stack.empty())
		{
			m  = m_stack.pop();
			m.reverseMigratoin();
		}
	}
	public static List<Job> random_shuffle_job(List<Job> original_job_list )
	{
		int size = original_job_list.size();
		List<Job> shuffled_list = new LinkedList<Job>();
		
		int []arr = new int [size];
		for(int i=0; i<size; i++)
			arr[i]=i;
		
		Random r = new Random();
		int a,b,t;
		for(int i=0; i<size; i++)
		{
			a = r.nextInt(size);
			b = size -a-1;
			
			t = arr[a];
			arr[a] = arr[b];
			arr[b] = t;
		}
		
		for(int i=0; i<size; i++)
		{
			shuffled_list.add(original_job_list.get(arr[i]));
		}
		return shuffled_list;
		
		
	}
	
	public static void searchLowUsageNode()
	{

		//System.out.println("searching..");
		for(Group g:datacenter.group_list)
		{
			for(Rack r:g.rack_list)
			{
				for(Node n:r.node_list)
				{
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					//if(n.cpu_left>n.cpu_total/4*3 && n.mem_left > n.mem_total/4*3 &&n.vm_list.size()>0)
					{
						System.out.println("Gruop "+n.group_id+ " Rack "+ n.rack_id+ " Node "+n.node_id+" is in low usage!");
					}
				}
			}
		}
	}
	
	public static int searchDispersedJob(long current_time)
	{
		int dispersed_job_counter =  0;
		double total_result=0;
		for(Job job:job_execute_list)
		{
			int [][] route_table = new int[job.parted][job.parted];
			
			int i=0,j=0;
			
			int dispersed_counter = 0;
			
			//compute route table of all the VMs a job has.
			//if both VM on a same node, then distance=0;
			//else if both VM on a same rack, then distance=1
			//else if both VM on a same group, then distance=2
			//else if both VM on a same datacenter, then distance=3;
			for(VM vm:job.vm_list)
			{
				j=0;
				for(VM vm_l:job.vm_list)
				{
					Node n1,n2;
					n1 = vm.node;
					n2 = vm_l.node;
					
					if(n1.node_id == n2.node_id)
						route_table[i][j]=0;
					else if(n1.rack_id == n2.rack_id)
						route_table[i][j]=0;
					else if(n1.group_id == n2.group_id)
						route_table[i][j]=1;
					else
						route_table[i][j]=2;
					
					if(route_table[i][j]>=1)
					{
						dispersed_counter++;
					}
					
					j++;
				}
				i++;
			}
			
			double result = 0;
			double best = 0;
			for(int i2=0; i2<job.parted; i2++)
			{
				for(int j2=0; j2<job.parted; j2++)
				{
					best += job.traffic_matrix[i2][j2];
					result += job.traffic_matrix[i2][j2] /Math.pow(5, route_table[i2][j2]);
				}
			}

			
			dispersed_counter = dispersed_counter/2;
			int pair = job.vm_list.size()*job.vm_list.size()-job.vm_list.size();
			if(dispersed_counter >=1)
			{
				dispersed_job_counter++;

//				System.out.println("Job "+job.job_id+" has "+pair+" pairs in total, and "
//						+dispersed_counter+" of them are dispersed!"
//						+" ;benefit = "+ result+" best: "+best);
			}
			
			if(best>=1)
				total_result+=result/best;
			else total_result++;
		}
		System.out.println("Total Job "+job_execute_list.size()+" ,dispersed counter:"+ dispersed_job_counter+" Machine used:" + Node.running_counter);
		try{
			bw.write(current_time+"\t"+job_execute_list.size()+"\t"+dispersed_job_counter+"\t"+Node.running_counter+"\n");
			bw.flush();
			
			bw_benefit.write(current_time+"\t"+job_execute_list.size()+"\t"+total_result+"\n");
			bw_benefit.flush();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return dispersed_job_counter;
	}
	
	public static int searchDispersedJob_2(long current_time)
	{
		int dispersed_job_counter =  0;
		double total_result=0;
		for(Job job:job_execute_list)
		{
			int [][] route_table = new int[job.parted][job.parted];
			
			int i=0,j=0;
			
			int dispersed_counter = 0;
			
			//compute route table of all the VMs a job has.
			//if both VM on a same node, then distance=0;
			//else if both VM on a same rack, then distance=1
			//else if both VM on a same group, then distance=2
			//else if both VM on a same datacenter, then distance=3;
			for(VM vm:job.vm_list)
			{
				j=0;
				for(VM vm_l:job.vm_list)
				{
					Node n1,n2;
					n1 = vm.node;
					n2 = vm_l.node;
					
					if(n1.node_id == n2.node_id)
						route_table[i][j]=0;
					else if(n1.rack_id == n2.rack_id)
						route_table[i][j]=0;
					else if(n1.group_id == n2.group_id)
						route_table[i][j]=1;
					else
						route_table[i][j]=2;
					
					if(route_table[i][j]>=1)
					{
						dispersed_counter++;
					}
					
					j++;
				}
				i++;
			}
			
			double result = 0;
			double best = 0;
			for(int i2=0; i2<job.parted; i2++)
			{
				for(int j2=0; j2<job.parted; j2++)
				{
					best += job.traffic_matrix[i2][j2];
					result += job.traffic_matrix[i2][j2] /Math.pow(5, route_table[i2][j2]);
				}
			}

			
			dispersed_counter = dispersed_counter/2;
			int pair = job.vm_list.size()*job.vm_list.size()-job.vm_list.size();
			if(dispersed_counter >=1)
			{
				dispersed_job_counter++;

//				System.out.println("Job "+job.job_id+" has "+pair+" pairs in total, and "
//						+dispersed_counter+" of them are dispersed!"
//						+" ;benefit = "+ result+" best: "+best);
			}
			
			if(best>=1)
				total_result+=result/best;
			else total_result++;
		}
		//System.out.println("Total Job "+job_execute_list.size()+" ,dispersed counter:"+ dispersed_job_counter+" Machine used:" + Node.running_counter);
		try{
			//bw.write(current_time+"\t"+job_execute_list.size()+"\t"+dispersed_job_counter+"\t"+Node.running_counter+"\n");
			//bw.flush();
			
//			bw_benefit.write(total_result+"\n");
//			bw_benefit.flush();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return dispersed_job_counter;
	}
	
	public static void moveDispersedJob(long current_time)
	{
		int dispersed_job_counter =  0;
		double total_result0=0;
		double total_result1=0;
		for(Job job:job_execute_list)
		{
			int [][] route_table = new int[job.parted][job.parted];
			
			int i=0,j=0;
			
			int dispersed_counter = 0;
			
			dispersed_counter = computeRouteTable(job, route_table);

			
			double result = 0;
			double best = 0;
			for(int i2=0; i2<job.parted; i2++)
			{
				for(int j2=0; j2<job.parted; j2++)
				{
					best += job.traffic_matrix[i2][j2];
					result += job.traffic_matrix[i2][j2] /Math.pow(5, route_table[i2][j2]);
				}
			}
			
			dispersed_counter = dispersed_counter/2;
			int pair = job.vm_list.size()*job.vm_list.size()-job.vm_list.size();
			
			if(dispersed_counter >=1)
			{
				if(best>=1)
					total_result0+=result/best;
				else total_result0++;
				
				if(shuffle_in_node(job)==false)
				{
					if(shuffle_in_rack(job)==false)
					{
						//System.out.println("Shuffle job "+job.job_id+ "failed!");
					}
				}
				//shuffle_in_group(job);
			}
		}
	}
	
	
	//fill the route_table and return the dispersed job counter
	public static int computeRouteTable(Job job, int [][] route_table)
	{
		int i=0,j=0;
		int dispersed_counter = 0;
		//compute route table of all the VMs a job has.
		//if both VM on a same node, then distance=0;
		//else if both VM on a same rack, then distance=1
		//else if both VM on a same group, then distance=2
		//else if both VM on a same datacenter, then distance=3;
		for(VM vm:job.vm_list)
		{
			j=0;
			for(VM vm_l:job.vm_list)
			{
				Node n1,n2;
				n1 = vm.node;
				n2 = vm_l.node;
				
				if(n1.node_id == n2.node_id)
					route_table[i][j]=0;
				else if(n1.rack_id == n2.rack_id)
					route_table[i][j]=0;
				else if(n1.group_id == n2.group_id)
					route_table[i][j]=1;
				else
					route_table[i][j]=2;
				
				//only care cross group
				if(route_table[i][j]>=1)
				{
					dispersed_counter++;
				}
				
				j++;
			}
			i++;
		}
		return dispersed_counter;
	}
	
	public static void shuffle_in_group(Job job)
	{
		Map<Integer,Integer> group_id_map = new HashMap<Integer,Integer>();
		int group_id;
		int value;
		for(VM vm:job.vm_list)
		{
			group_id = vm.node.group_id;
			if(!group_id_map.containsKey(group_id))
				group_id_map.put(group_id, 1);
			else
			{
				value = group_id_map.get(group_id)+1;
				group_id_map.remove(group_id);
				group_id_map.put(group_id, value);
			}
		}
		
		//sort by value
		ArrayList<Map.Entry<Integer,Integer>> group_map_list = new ArrayList<Map.Entry<Integer,Integer>>(group_id_map.entrySet());
		Collections.sort(group_map_list,new Comparator<Map.Entry<Integer, Integer>>(){
			public int compare(Map.Entry<Integer, Integer> o1, Map.Entry<Integer,Integer>o2){
				return (o1.getValue()-o2.getValue());
			}
		});
		
		
//		if(group_id_map.size()>3)
//		{
//			System.out.print("Job "+job.job_id+" is too large, can't be reshuffle right now!");
//			System.out.println(group_map_list .toString());
//		}
//		else
		{
			//try to put the vm in small group into a large group
			List<VM> large_group = new LinkedList<VM>();
			List<VM> small_group = new LinkedList<VM>();
			
			int large_group_id=0;
			int small_group_id=0;
			
			Group lgroup=null;
			
			int first=0;
			
			large_group_id = group_map_list.get(group_map_list.size()-1).getKey();
			
//			for(Map.Entry<Integer, Integer> entry:group_map_list)
//			{
//				int i=entry.getKey();
//				if(first==0)
//				{
//					small_group_id = i;
//				}
//				else if(first==1 && group_id_map.size()==2)
//				{
//					large_group_id = i;
//				}
//				else if(first==2 && group_id_map.size()==3)
//				{
//					large_group_id = i;
//				}
//				first++;
//			}
			
			//separate VMs
			for(VM vm:job.vm_list)
			{
				group_id = vm.node.group_id;
				if(group_id != large_group_id)
					small_group.add(vm);
				else
				{
					lgroup = vm.node.group;
					large_group.add(vm);
				}
			}
			
			//try to allocate small group into large group
			
			boolean vms_relocate =true;
			for(VM vm:small_group)
			{
				boolean vm_relocate =false;

				for(Rack r:lgroup.rack_list)
				{
					for(Node n:r.node_list)
					{

						//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
						if(n.getCpu_left()>= vm.req_cpu && n.getMem_left() >= vm.req_mem)
						{
							
							//return the original space
							vm.node.releaseCpu(vm.req_cpu);
							vm.node.releaseMem(vm.req_mem);

							vm.node.removeVM(vm);
							
							//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
							n.requireCpu(vm.req_cpu);
							n.requireMem(vm.req_mem);
							vm.node = n;
							
							//put this VM into the node's vm_list!
							n.addVM(vm);
							
							//if(g.group_id>8)
							//System.out.println("VM "+vm.vm_id+" is located on: Group "+g.group_id+" Rack "
							//		+r.rack_id+" Node "+n.node_id);
							vm_relocate = true;
							
							break;
						}
						
					}
					if(vm_relocate==true)
						break;
				}
				
				vms_relocate = vms_relocate && vm_relocate;
			}
			if(vms_relocate==true)
				System.out.println("Job "+job.job_id+" reallocate success!");
			else
				System.out.println("Job "+job.job_id+" reallocate failed!"+group_map_list.toString());
			
			
		}
	}
	
	//只考虑node的情况下，要把所有虚拟机放到同一个rack下，如果尝试失败，则返回false，开始尝试将虚拟机部署在同一个rack的尽量少的node上
	public static boolean shuffle_in_node(Job job)
	{
		Node node;
		int value;
		Map<Node,Integer> node_vm_map = new HashMap<Node,Integer>();
		for(VM vm:job.vm_list)
		{
			node = vm.node;
			if(!node_vm_map.containsKey(vm.node))
				node_vm_map.put(node, 1);
			else
			{
				value = node_vm_map.get(node)+1;
				node_vm_map.remove(node);
				node_vm_map.put(node, value);
			}
		}
		//sort by value
		ArrayList<Map.Entry<Node,Integer>> node_vm_list = new ArrayList<Map.Entry<Node,Integer>>(node_vm_map.entrySet());
		Collections.sort(node_vm_list,new Comparator<Map.Entry<Node, Integer>>(){
			public int compare(Map.Entry<Node, Integer> o1, Map.Entry<Node,Integer>o2){
				return (o1.getValue()-o2.getValue());
			}
		});
		

		//try to put the vm in small group into a large group
		List<VM> large_node = new LinkedList<VM>();
		List<VM> small_node = new LinkedList<VM>();
		
		Node lnode = null;
		Node temp_node;
		lnode = node_vm_list.get(node_vm_list.size()-1).getKey();
					
		//separate VMs
		for(VM vm:job.vm_list)
		{
			temp_node = vm.node;
			if(temp_node != lnode)
				small_node.add(vm);
			else
			{
				large_node.add(vm);
			}
		}
		
		//firstly, try to allocate all other vms into a same node
		boolean vms_node_relocate = false;
		
		
		int total_cpu_needed = 0;
		int total_mem_needed = 0;
		for(VM vm:small_node)
		{
			total_cpu_needed += vm.req_cpu;
			total_mem_needed += vm.req_mem;
		}
		
		if(lnode.getCpu_left() >= total_cpu_needed &&lnode.getMem_left() >= total_mem_needed)
		{
			for(VM vm:small_node)
			{
				vm.node.releaseCpu(vm.req_cpu);
				vm.node.releaseMem(vm.req_mem);

				vm.node.removeVM(vm);
				
				//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
				lnode.requireCpu(vm.req_cpu);
				lnode.releaseMem(vm.req_mem);
;
				vm.node = lnode;
				
				//put this VM into the node's vm_list!
				lnode.addVM(vm);
				
				migration_counter++;
			}
			return true;
		}
		return false;
	}
	
	//只考虑node的情况下，要把所有虚拟机放到同一个rack下，如果尝试失败，则返回false，开始尝试将虚拟机部署在同一个rack的尽量少的node上
	//要记录下所有的操作，以便回溯
	public static boolean shuffle_in_node_2(Job job)
	{
		Node node;
		int value;
		Map<Node,Integer> node_vm_map = new HashMap<Node,Integer>();
		for(VM vm:job.vm_list)
		{
			node = vm.node;
			if(!node_vm_map.containsKey(vm.node))
				node_vm_map.put(node, 1);
			else
			{
				value = node_vm_map.get(node)+1;
				node_vm_map.remove(node);
				node_vm_map.put(node, value);
			}
		}
		//sort by value
		ArrayList<Map.Entry<Node,Integer>> node_vm_list = new ArrayList<Map.Entry<Node,Integer>>(node_vm_map.entrySet());
		Collections.sort(node_vm_list,new Comparator<Map.Entry<Node, Integer>>(){
			public int compare(Map.Entry<Node, Integer> o1, Map.Entry<Node,Integer>o2){
				return (o1.getValue()-o2.getValue());
			}
		});
		

		//try to put the vm in small group into a large group
		List<VM> large_node = new LinkedList<VM>();
		List<VM> small_node = new LinkedList<VM>();
		
		Node lnode = null;
		Node temp_node;
		lnode = node_vm_list.get(node_vm_list.size()-1).getKey();
					
		//separate VMs
		for(VM vm:job.vm_list)
		{
			temp_node = vm.node;
			if(temp_node != lnode)
				small_node.add(vm);
			else
			{
				large_node.add(vm);
			}
		}
		
		//firstly, try to allocate all other vms into a same node
		boolean vms_node_relocate = false;
		
		
		int total_cpu_needed = 0;
		int total_mem_needed = 0;
		for(VM vm:small_node)
		{
			total_cpu_needed += vm.req_cpu;
			total_mem_needed += vm.req_mem;
		}
		
		if(lnode.getCpu_left() >= total_cpu_needed &&lnode.getMem_left() >= total_mem_needed)
		{
			for(VM vm:small_node)
			{
				//record this migration
				MigrationRecord m=new MigrationRecord(vm,vm.node,lnode);
				m_stack.push(m);
				
				vm.node.releaseCpu(vm.req_cpu);
				vm.node.releaseMem(vm.req_mem);

				vm.node.removeVM(vm);
				
				//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
				lnode.requireCpu(vm.req_cpu);
				lnode.releaseMem(vm.req_mem);
;
				vm.node = lnode;
				
				//put this VM into the node's vm_list!
				lnode.addVM(vm);
				

			}
			return true;
		}
		return false;
	}
	
	
	//只考虑rack的情况下，要把所有虚拟机放到同一个rack下，如果尝试失败，则返回false，开始尝试将虚拟机部署在同一个group的尽量少的rack上
	public static boolean shuffle_in_rack(Job job)
	{
		int rack_id;
		int value;
		
		Map<Integer,Integer> rack_id_map = new HashMap<Integer,Integer>();

		for(VM vm:job.vm_list)
		{
			rack_id = vm.node.rack_id;
			if(!rack_id_map.containsKey(rack_id))
				rack_id_map.put(rack_id, 1);
			else
			{
				value = rack_id_map.get(rack_id)+1;
				rack_id_map.remove(rack_id);
				rack_id_map.put(rack_id, value);
			}
		}
		
		//sort by value
		ArrayList<Map.Entry<Integer,Integer>> rack_map_list = new ArrayList<Map.Entry<Integer,Integer>>(rack_id_map.entrySet());
		Collections.sort(rack_map_list,new Comparator<Map.Entry<Integer, Integer>>(){
			public int compare(Map.Entry<Integer, Integer> o1, Map.Entry<Integer,Integer>o2){
				return (o1.getValue()-o2.getValue());
			}
		});
		
		
//		if(group_id_map.size()>3)
//		{
//			System.out.print("Job "+job.job_id+" is too large, can't be reshuffle right now!");
//			System.out.println(group_map_list .toString());
//		}
//		else
		{
			//try to put the vm in small group into a large group
			List<VM> large_rack = new LinkedList<VM>();
			List<VM> small_rack = new LinkedList<VM>();
			
			int large_rack_id=0;
			int small_rack_id=0;
			
			Rack lrack=null;
			
			int first=0;
			
			large_rack_id = rack_map_list.get(rack_map_list.size()-1).getKey();
						
			//separate VMs
			for(VM vm:job.vm_list)
			{
				rack_id = vm.node.rack_id;
				if(rack_id != large_rack_id)
					small_rack.add(vm);
				else
				{
					lrack = vm.node.rack;
					large_rack.add(vm);
				}
			}
			
			//firstly, try to allocate all other vms into a same node
			boolean vms_node_relocate = false;
					
			int total_cpu_needed = 0;
			int total_mem_needed = 0;
			
			for(VM vm:small_rack)
			{
				total_cpu_needed += vm.req_cpu;
				total_mem_needed += vm.req_mem;
			}
			
			int total_cpu_left = 0;
			int total_mem_left = 0;
			
			for(Node node:lrack.node_list)
			{
				total_cpu_left += node.getCpu_left();
				total_mem_left += node.getMem_left();
			}
			
			if(total_cpu_left >=total_cpu_needed && total_mem_left >= total_mem_needed)
			{
				//reallocate vm in small rack into large rack
				List<VM> left = null;
				if(try_reallocate_in_rack(small_rack, large_rack, lrack, left)==true)
					return true;
				else return false;
			}
		}
			return false;
	}
	
	//只考虑rack的情况下，要把所有虚拟机放到同一个rack下，如果尝试失败，则返回false，开始尝试将虚拟机部署在同一个group的尽量少的rack上
	public static boolean shuffle_in_rack_2(Job job)
	{
		int rack_id;
		int value;
		
		Map<Integer,Integer> rack_id_map = new HashMap<Integer,Integer>();

		for(VM vm:job.vm_list)
		{
			rack_id = vm.node.rack_id;
			if(!rack_id_map.containsKey(rack_id))
				rack_id_map.put(rack_id, 1);
			else
			{
				value = rack_id_map.get(rack_id)+1;
				rack_id_map.remove(rack_id);
				rack_id_map.put(rack_id, value);
			}
		}
		
		//sort by value
		ArrayList<Map.Entry<Integer,Integer>> rack_map_list = new ArrayList<Map.Entry<Integer,Integer>>(rack_id_map.entrySet());
		Collections.sort(rack_map_list,new Comparator<Map.Entry<Integer, Integer>>(){
			public int compare(Map.Entry<Integer, Integer> o1, Map.Entry<Integer,Integer>o2){
				return (o1.getValue()-o2.getValue());
			}
		});
		
		
//		if(group_id_map.size()>3)
//		{
//			System.out.print("Job "+job.job_id+" is too large, can't be reshuffle right now!");
//			System.out.println(group_map_list .toString());
//		}
//		else
		{
			//try to put the vm in small group into a large group
			List<VM> large_rack = new LinkedList<VM>();
			List<VM> small_rack = new LinkedList<VM>();
			
			int large_rack_id=0;
			int small_rack_id=0;
			
			Rack lrack=null;
			
			int first=0;
			
			large_rack_id = rack_map_list.get(rack_map_list.size()-1).getKey();
						
			//separate VMs
			for(VM vm:job.vm_list)
			{
				rack_id = vm.node.rack_id;
				if(rack_id != large_rack_id)
					small_rack.add(vm);
				else
				{
					lrack = vm.node.rack;
					large_rack.add(vm);
				}
			}
			
			//firstly, try to allocate all other vms into a same node
			boolean vms_node_relocate = false;
					
			int total_cpu_needed = 0;
			int total_mem_needed = 0;
			
			for(VM vm:small_rack)
			{
				total_cpu_needed += vm.req_cpu;
				total_mem_needed += vm.req_mem;
			}
			
			int total_cpu_left = 0;
			int total_mem_left = 0;
			
			for(Node node:lrack.node_list)
			{
				total_cpu_left += node.getCpu_left();
				total_mem_left += node.getMem_left();
			}
			
			if(total_cpu_left >=total_cpu_needed && total_mem_left >= total_mem_needed)
			{
				//reallocate vm in small rack into large rack
				List<VM> left = null;
				if(try_reallocate_in_rack_2(small_rack, large_rack, lrack, left)==true)
					return true;
				else return false;
			}
		}
			return false;
	}
	/**
	 * 
	 * @param small_rack
	 * @param large_rack
	 * @param lrack
	 * @param left 返回没有完成reallocate的虚拟机list
	 */
	public static boolean try_reallocate_in_rack(List<VM> small_rack, List<VM> large_rack,Rack lrack, List<VM> left)
	{
		//store the node which vm in large_rack belongs
		Set<Node> node_set = new HashSet<Node>();
		for(VM vm:large_rack)
		{
			node_set.add(vm.node);
		}
		
		//优先部署到同一个node
		List<VM> success_list = new LinkedList<VM>();
		for(VM vm:small_rack)
		{
			for(Node node:node_set)
			{
				if(node.getStatus()==false)
				{
					if(datacenter.total_cpu_rate>recon_threshold||datacenter.total_mem_rate>recon_threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
						node.setStatus(true);
					else
						continue;
				}
				if(node.getCpu_left()>= vm.req_cpu && node.getMem_left()>= vm.req_mem)
				{
					vm.node.releaseCpu(vm.req_cpu);
					vm.node.releaseMem(vm.req_mem);;
					vm.node.removeVM(vm);
					
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					node.requireCpu(vm.req_cpu);
					node.requireMem(vm.req_mem);
;
					vm.node = node;
					
					//put this VM into the node's vm_list!
					node.addVM(vm);
					migration_counter++;
					success_list.add(vm);
				}
			}
		}
		
		small_rack.removeAll(success_list);
		if(small_rack.size()==0) //reallocate 到同一个rack的一组node成功
		{
			left =null;
			return true;
		}
		
		
		success_list.clear();
		for(VM vm:small_rack)
		{
			for(Node node:lrack.node_list)
			{
				if(node.getCpu_left()>= vm.req_cpu && node.getMem_left()>= vm.req_mem)
				{
					vm.node.releaseCpu(vm.req_cpu);
					vm.node.releaseMem(vm.req_mem);;
					vm.node.removeVM(vm);
					
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					node.requireCpu(vm.req_cpu);
					node.requireMem(vm.req_mem);
;
					vm.node = node;
					
					//put this VM into the node's vm_list!
					node.addVM(vm);
					
					success_list.add(vm);
				}
			}
		}
		small_rack.removeAll(success_list);
		if(small_rack.size()==0) //reallocate 到同一个rack的一组node成功
		{
			left =null;
			return true;
		}
		else
		{
			left = small_rack;
			return false;
		}
		
		
	}
	
	public static boolean try_reallocate_in_rack_2(List<VM> small_rack, List<VM> large_rack,Rack lrack, List<VM> left)
	{
		//store the node which vm in large_rack belongs
		Set<Node> node_set = new HashSet<Node>();
		for(VM vm:large_rack)
		{
			node_set.add(vm.node);
		}
		
		//优先部署到同一个node
		List<VM> success_list = new LinkedList<VM>();
		for(VM vm:small_rack)
		{
			for(Node node:node_set)
			{
				if(node.getStatus()==false)
				{
					if(datacenter.total_cpu_rate>recon_threshold||datacenter.total_mem_rate>recon_threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
						node.setStatus(true);
					else
						continue;
				}
				if(node.getCpu_left()>= vm.req_cpu && node.getMem_left()>= vm.req_mem)
				{
					MigrationRecord m=new MigrationRecord(vm,vm.node,node);
					m_stack.push(m);
					
					vm.node.releaseCpu(vm.req_cpu);
					vm.node.releaseMem(vm.req_mem);;
					vm.node.removeVM(vm);
					
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					node.requireCpu(vm.req_cpu);
					node.requireMem(vm.req_mem);
					vm.node = node;
					
					//put this VM into the node's vm_list!
					node.addVM(vm);
					
					success_list.add(vm);
				}
			}
		}
		
		small_rack.removeAll(success_list);
		if(small_rack.size()==0) //reallocate 到同一个rack的一组node成功
		{
			left =null;
			return true;
		}
		
		
		success_list.clear();
		for(VM vm:small_rack)
		{
			for(Node node:lrack.node_list)
			{
				if(node.getCpu_left()>= vm.req_cpu && node.getMem_left()>= vm.req_mem)
				{
					vm.node.releaseCpu(vm.req_cpu);
					vm.node.releaseMem(vm.req_mem);;
					vm.node.removeVM(vm);
					
					//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
					node.requireCpu(vm.req_cpu);
					node.requireMem(vm.req_mem);
					vm.node = node;
					
					//put this VM into the node's vm_list!
					node.addVM(vm);
					
					success_list.add(vm);
				}
			}
		}
		small_rack.removeAll(success_list);
		if(small_rack.size()==0) //reallocate 到同一个rack的一组node成功
		{
			left =null;
			return true;
		}
		else
		{
			left = small_rack;
			return false;
		}
		
		
	}
	
	public static void printTrafficInfo()//print current traffic info
	{
		int datacenter_traffic = 0;
		for(Group group:datacenter.group_list)
		{
			int group_traffic = 0;
			for(Rack rack:group.rack_list)
			{
				int rack_traffic = 0;
				for(Node node:rack.node_list)
				{
					int node_traffic = 0;
					for(VM vm:node.get_vm_list())
					{
						for(int i=0;i<vm.job.traffic_matrix.length;i++)
						{
							VM vm2 = vm.job.vm_list.get(i);
							if(vm.node.group_id != vm2.node.group_id)
								datacenter_traffic += vm.job.traffic_matrix[vm.inner_vm_id][i];//统计所有通过核心交换机的通信量
							if(vm.node.rack_id !=vm2.node.rack_id)
								group_traffic += vm.job.traffic_matrix[vm.inner_vm_id][i];//统计所有通过group交换机的通信量
							if(vm.node.node_id != vm2.node.node_id)
								rack_traffic += vm.job.traffic_matrix[vm.inner_vm_id][i];//统计所有通过rack交换机的通信量
						 
								node_traffic += vm.job.traffic_matrix[vm.inner_vm_id][i];
						}
								
					}
					node.node_traffic = node_traffic;
				}
				rack.rack_traffic = rack_traffic;
			}
			group.group_traffic = group_traffic;
		}
		datacenter.datacenter_traffic = datacenter_traffic;
		
		//重复计算了一遍，因此全部除2
		datacenter.datacenter_traffic = datacenter.datacenter_traffic/2;
		int datacenter_total_traffic= 0;
		
		for(Group group:datacenter.group_list)
		{
			int group_total_traffic = 0;
			
			group.group_traffic = group.group_traffic / 2;
			for(Rack rack:group.rack_list)
			{
				int rack_total_traffic = 0;
				
				rack.rack_traffic = rack.rack_traffic / 2;
				for(Node node:rack.node_list)
				{
					node.node_traffic = node.node_traffic /2;
					rack_total_traffic += node.node_traffic;
				}
				
				group_total_traffic +=rack.rack_traffic;
				
				System.out.println("Rack "+rack.rack_id +" total inner traffic:" + rack_total_traffic + "; real traffic "+ rack.rack_traffic );
			}
			System.out.println("Group "+group.group_id +" total inner traffic:" + group_total_traffic + "; real traffic "+ group.group_traffic );
		}
		
		System.out.println();
		System.out.println();
	}



}
