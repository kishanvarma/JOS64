#include "ns.h"

extern union Nsipc nsipcbuf;

    void
input(envid_t ns_envid)
{
    binaryname = "ns_input";

    // LAB 6: Your code here:
    // 	- read a packet from the device driver
    //	- send it to the network server
    // Hint: When you IPC a page to the network server, it will be
    // reading from it for a while, so don't immediately receive
    // another packet in to the same physical page.

	//this is similar to the idea from testoutput.c
	int n,r;
	uint8_t packet[2048]; //this is the maximum size of packet i have taken in my e1000.h

	while(1){
		//alocate new pages as per the hint.	
		sys_page_alloc(0, &nsipcbuf, PTE_P|PTE_W|PTE_U) ;
		 
		//read a packet from the device driver and if does not do it properly,dont waste cpu - sys_yield() it,
		while((n = sys_receive(packet, n)) < 0)
			sys_yield();
		//my sys_receive returns size of packet read
		nsipcbuf.pkt.jp_len = n ;
		//now copy the data in packet in
		memmove(nsipcbuf.pkt.jp_data,packet,n);
		//send it to the network server.
		ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_U|PTE_P|PTE_W);
		//so that same page is not used again.
		sys_page_unmap(0, &nsipcbuf);
		
	}
//ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
}
