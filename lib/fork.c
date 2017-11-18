// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r,s,t;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
		
	//first check if its write , whether the page has pte_cow set.
	if( (err & FEC_WR) &&  (uvpt[VPN(addr)] & PTE_COW)) { 
		        r = sys_page_alloc(0,(void *)PFTEMP,PTE_W|PTE_U|PTE_P);
			
			if(r<0)
		  		panic("in lib/fork.c, pgfault error ,pagealloc");
			
			void *old_page_addr = ROUNDDOWN(addr,PGSIZE);
			memmove((void *)PFTEMP,old_page_addr,PGSIZE);
			
			s = sys_page_map(0,(void *)PFTEMP,0,old_page_addr,PTE_U|PTE_P|PTE_W);
			if(s < 0)
				panic("in lib/fork.c, pgfault error ,pagemap");
				
			t = sys_page_unmap(0,(void *)PFTEMP);
			if(t < 0)
				panic("in lib/fork.c, pgfault error ,pageunmap");

	}
	else{
//		cprintf(" \n(err & FEC_WR) :%d  \t\t (uvpt[VPN(addr)] & PTE_COW) : %d \n",(err & FEC_WR),(uvpt[VPN(addr)] & PTE_COW));
		panic("\n in lib/fork.c, pgfault error not for cow\n");
	}
//	panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r,s,t;
	pte_t ptentry = uvpt[pn];
	void *va_pn = (void *)((uintptr_t)pn * PGSIZE);
	int perm = ptentry & PTE_SYSCALL ;	

	if( perm & PTE_SHARE){
		r = sys_page_map(0,va_pn,envid,va_pn,perm);
        	if(r < 0)
              		panic("duppage : sys_page_map share error");

	}
	else if( ( perm & PTE_COW) ||   (perm & PTE_W)) {
		perm = perm & (~PTE_W); //remove the write perm so that it wont be re written by either processes
		perm = perm | PTE_COW; //making it cow

	//	Map our virtual page pn (address pn*PGSIZE) into the target envid at the same virtual address.
		r = sys_page_map(0,va_pn,envid,va_pn,perm);
		if(r < 0)
			panic("duppage : sys_page_map 1 error");

	// our mapping must be marked copy-on-write as well
		s = sys_page_map(0,va_pn,0,va_pn,perm);
		if(s < 0)
                        panic("duppage : sys_page_map 2 error");
	}
	else{
		//case when it s not cow, then map at same va.
		t = sys_page_map(0,va_pn,envid,va_pn,perm);
                if(t < 0)
                        panic("duppage : sys_page_map else part  error");
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//Set up our page fault handler
	set_pgfault_handler(pgfault);
	// Create a child
	envid_t child = sys_exofork();
	if(child < 0)
		panic("\nfork :child error");

	if(child == 0){
	//	cprintf("\n i am in the child");
		thisenv = &envs[ENVX(sys_getenvid())] ;
		return 0;
	}	
	
	//	cprintf("\n i am in the parent");
	int r = sys_page_alloc(child,(void *)(UXSTACKTOP - PGSIZE),PTE_P|PTE_W|PTE_U);
	if (r < 0 )
		panic("\n fork : sys_page_alloc \n");

	//copying the address space
	pml4e_t pml4e;
	pdpe_t pdpe,pdpe_entry = 0;
	pde_t pde,pde_entry = 0;
	pte_t pte,pte_entry = 0;

	for(pml4e = 0;pml4e<VPML4E(UTOP) ; pml4e++){
		if(uvpml4e[pml4e] & PTE_P){
 			for(pdpe = 0;pdpe < NPDPENTRIES ; pdpe++,pdpe_entry++){
				if(uvpde[pdpe_entry] & PTE_P){
				        for(pde = 0;pde < NPDENTRIES ; pde++,pde_entry++){
                                	    if(uvpd[pde_entry] & PTE_P){
						for(pte = 0;pte < NPTENTRIES ; pte++,pte_entry++){
                                        	   if(uvpt[pte_entry] & PTE_P){
							if(pte_entry != VPN(UXSTACKTOP - PGSIZE)){
							//	cprintf("\n will dup now");
								r = duppage(child,(unsigned)pte_entry);
								if(r < 0)
								  panic("\n fork : duppage error");
							     }
    						         }
					              }
				                  }
						else{ 
				   		     pte_entry  = (pde_entry + 1 ) * NPTENTRIES;
						}
				             }
					  }
			             else{  
		   			 pde_entry = (pdpe_entry + 1) * NPDENTRIES;	
					 }
				   }
    				}
    			   else{
				pdpe_entry =  (pml4e + 1) * NPDPENTRIES;	
			       }
		}
 

	extern void _pgfault_upcall(void);
	
	int s = sys_env_set_pgfault_upcall(child,_pgfault_upcall);
	if(s < 0)
		panic("\n fork : sys_env_set_pgfault_upcall error \n");

	int t = sys_env_set_status(child,ENV_RUNNABLE);
	if(t < 0)
                panic("\n fork : sys_env_set_status error \n");			
	
	return child;

	//	panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
