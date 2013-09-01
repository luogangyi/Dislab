package lgy;

import java.util.*;

import model.*;
import architecture.*;

public class FirstFit {

	String findBestNC(VMDetail vmd)
	{
		VM vm =new VM(vmd);
		try{
			DatacenterBuilder.updateNC();
			DataCenter datacenter =DatacenterBuilder.getDataCenter();
			System.out.println(vm);
			//System.out.println("searching..");
			for(RackGroup g:datacenter.group_map.values())
			{
				for(Rack r:g.rack_map.values())
				{
					for(Node n:r.node_map.values())
					{
						System.out.println(n);
						//首先判断虚拟化类型
						if(!n.virtType.equalsIgnoreCase(vm.virtType))
						{
							continue;
						}
//						if(n.getStatus()==false)
//						{
//							if(datacenter.total_cpu_rate>threshold||datacenter.total_mem_rate>threshold||datacenter.getTotal_mem_left()<10240||datacenter.getTotal_cpu_left()<10)
//								n.setStatus(true);
//							else
//								continue;
//						}
						//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
						if(n.getCpu_left()>=vm.req_cpu && n.getMem_left() >= vm.req_mem)
						{
							//System.out.println(n.cpu_left+"\t"+n.mem_left+"\n");
							n.requireCpu(vm.req_cpu);
							n.requireMem(vm.req_mem);

							vm.node = n;
							
//							if(max_node==null)
//								max_node = n;
//							else if(max_node.node_id<n.node_id)
//								max_node = n;
							//put this VM into the node's vm_list!
							n.addVM(vm);
							
							//if(g.group_id>8)
							//System.out.println("VM "+vm.vm_id+" is located on: Group "+g.group_id+" Rack "
							//		+r.rack_id+" Node "+n.node_id);
							return n.IP;
						}
					}
				}
			}
			//查找失败则返回null
			return null;
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}
	Map<Integer,String> findBestNCForGroup(List<VMDetail> vmd_list)
	{
		Map<Integer,String> vmid_ncip_map = new HashMap<Integer,String>();
		boolean success = true;
		for(VMDetail vmd:vmd_list)
		{
			String nc_ip = findBestNC(vmd);
			if(nc_ip != null)
			{
				vmid_ncip_map.put(vmd.getID(), nc_ip);
			}
			else
			{
				//出现失败时，则不向后继续查找
				success = false;
				break;
			}	
		}
		if(success)
			return vmid_ncip_map;
		else
			return null;
	}
	public static void main(String[] args) {
		// TODO Auto-generated method stub

	}

}
