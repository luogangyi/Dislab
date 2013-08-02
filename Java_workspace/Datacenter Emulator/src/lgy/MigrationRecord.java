package lgy;

public class MigrationRecord {


	VM vm;
	Node origin;
	Node target;
	
	public MigrationRecord(VM vm, Node origin, Node target) {
		super();
		this.vm = vm;
		this.origin = origin;
		this.target = target;
	}
	
	//一条记录代表一次迁移，从origin迁移到target
	//reverse 既从target迁移回origin
	public void reverseMigratoin()
	{
		target.releaseCpu(vm.req_cpu);
		target.releaseMem(vm.req_mem);
		target.removeVM(vm);
		
		origin.requireCpu(vm.req_cpu);
		origin.requireMem(vm.req_mem);
		origin.addVM(vm);
		
		vm.node = origin;
	}

}
