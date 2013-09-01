package architecture;

import java.util.*;
import java.io.*;

public class Job {
	public int job_id;
	public int start_time;
	public int time_used;
	public int time_left;
	public int cpu_used;
	public int mem_total;
	public int parted;
	public int cpu_parted;
	public int mem_parted;
	
	public int[][] traffic_matrix;
	
	public List<VM> vm_list = new LinkedList<VM>();
	public Job(int job_id, int start_time, int time_used, int cpu_used,
			int mem_total, int parted, int mem_parted) {
		super();
		this.job_id = job_id;
		this.start_time = start_time;
		this.time_used = time_used;
		this.cpu_used = cpu_used;
		this.mem_total = mem_total;
		this.parted = parted;
		this.mem_parted = mem_parted;
		
		cpu_parted = cpu_used/parted;
		time_left = time_used;
		
		initiateTrafficMatrix();
	}
	
	public void initiateTrafficMatrix()
	{
		BufferedReader br ;
		try{
			br= new  BufferedReader(
					new FileReader("TestDataGenerator/TrafficData/Traffic_Matrix_"+job_id+".txt"));
			
			br.readLine();
			String s_dim = br.readLine();
			int dim = Integer.parseInt(s_dim);
			
			traffic_matrix = new int[dim][dim];
			
			for(int i=0; i<dim; i++)
			{
				String templine = br.readLine();
				String []data = templine.split("\\s+");
				for(int j=0; j<dim; j++)
				{
					traffic_matrix[i][j] = Integer.parseInt(data[j]);
				}
			}
			
			//System.out.println("initiate traffic matrix success! test data ="+traffic_matrix[dim-1][dim-1]);
			
			br.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			//System.out.println("TestDataGenerator/TrafficData/Traffic_Matrix_"+job_id+".txt"+" Open failed!");
			return;
		}
	}
	
}
