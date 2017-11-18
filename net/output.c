#include "ns.h"

extern union Nsipc nsipcbuf;

    void
output(envid_t ns_envid)
{
    binaryname = "ns_output";

    // LAB 6: Your code here:
    // 	- read a packet from the network server
	int from_ns;
	envid_t from_envid;
	int perm;
	while(1){
		perm = 0;
		from_ns =ipc_recv(&from_envid, &nsipcbuf, &perm);

		//can club these two conditions , but to know which error is being thrown, lets keep them separate.	
		if(from_ns != NSREQ_OUTPUT){
                        cprintf("\n not NSREQ_OUTPUT \n");
                        continue;
                }
		
		if(from_envid != ns_envid){
                        cprintf("\n Unexpected environment error in output \n");
                        continue;
                }	

		if((perm & PTE_P) == 0){
			cprintf("\n no page error \n");
			continue;
		}

	//
	while(sys_transmit(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len) < 0)
		sys_yield();

	}
    //	- send the packet to the device driver
	
}
