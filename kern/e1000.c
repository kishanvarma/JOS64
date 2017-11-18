#include <kern/e1000.h>
#include <inc/string.h>

// LAB 6: Your driver code here

volatile uint32_t *E1000_base;

void init_ring()
{
	int i;	
	cprintf("\n Initialising the TX RING.\n");
	for(i = 0; i < TXD_MAX; i++){
		TXD_RING[i].addr = PADDR(&TXP_BUF[i]);
		TXD_RING[i].length = 0;
		TXD_RING[i].status |= E1000_TXD_STAT_DD; //condition
	}
	cprintf("\n TX descriptors are initialised.\n");
	
	cprintf("\n Initialising the RX RING.\n");
	for(i = 0; i <RXD_MAX; i++){
                RXD_RING[i].addr = PADDR(&RXP_BUF[i]);
                RXD_RING[i].status = 0;
        }
        cprintf("\n RX descriptors are initialised.\n");
}

int pci_attach_E1000(struct pci_func *p_f){
	//L6 Ex3 :just add the enable function.
	pci_func_enable(p_f);
	
	init_ring();
		
	E1000_base = (uint32_t*)mmio_map_region(p_f->reg_base[0],(size_t)p_f->reg_size[0]);
	cprintf("DEvice status Register : %x \n",E1000_base[2]);
	
	//14.5-Transmit Initialization
	E1000_base[E1000_TDBAL/4] = PADDR(TXD_RING);
	E1000_base[E1000_TDBAH/4] = 0; 
	
	E1000_base[E1000_TDLEN/4] = TXD_MAX * sizeof(struct tx_desc); 
 
	E1000_base[E1000_TDH/4] = 0;
	E1000_base[E1000_TDT/4] = 0;
 	//Given Bit DEscription of Transmit Control Register
	E1000_base[E1000_TCTL/4] = E1000_TCTL_EN | E1000_TCTL_PSP | (E1000_TCTL_CT & (0x10<<4)) | (E1000_TCTL_COLD & (0x40<<12)) ;
	 //Given Bit DEscription of Transmit IPG Register
	E1000_base[E1000_TIPG/4] = 10 | (0x8<<10) | (0x6<<20);
	//Ex10 Explanation.
	E1000_base[E1000_RAL/4] = 0x12005452 ; 
	E1000_base[E1000_RAH/4] = 0x5634 | E1000_RAH_AV ;

	E1000_base[E1000_RDBAL/4] = PADDR(RXD_RING);
	E1000_base[E1000_RDBAH/4] = 0;
 	
	E1000_base[E1000_RDLEN/4] = RXD_MAX * sizeof(struct rx_desc);
 	
	E1000_base[E1000_RDH/4] = 0;
	E1000_base[E1000_RDT/4] = 127;
	//Bit descriptions 
	E1000_base[E1000_RCTL/4] = E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SECRC;

	return 0;
}

int transmit(void *addr,int n){
//	cprintf("\n inside transmit\n");
	if(n > TXD_BUFFER_SIZE) 		//1518
		n = TXD_BUFFER_SIZE; 		//truncate to the max  size when larger than that.

	uint32_t tail = E1000_base[E1000_TDT/4]; //where the tail is now.
						 //condition that the buffer us full.
	if(!(TXD_RING[tail].status & E1000_TXD_STAT_DD)){
		cprintf("\n the transmit condition is not meeting\n");
		return -1;
		}

	
	memmove(&TXP_BUF[tail],addr,n);   //copy the packet in TXP_BUF.
	TXD_RING[tail].length = n;        //length 
	TXD_RING[tail].status = 0;	  
	TXD_RING[tail].cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
		
	E1000_base[E1000_TDT/4] = (tail + 1)% TXD_MAX ; //increment the tail in circular queue fashion.
//		cprintf("\n returnng from transnit after copying.\n");
		return 0;	
}


int receive(void *addr,int n){
	//similar to the recieve function written above
	uint32_t tail = E1000_base[E1000_RDT/4]; 
	tail = (tail + 1)% RXD_MAX ;
 	E1000_base[E1000_RDT/4] = tail ;

//	cprintf("\n%d\t%d\t%d",tail,RXD_RING[tail].status ,RXD_RING[tail].length);

	if(!(RXD_RING[tail].status & E1000_RXD_STAT_DD)){
//               cprintf("\n the receive condition is not meeting\n");
                return -1;
                }
		
	n = RXD_RING[tail].length;
	//copy the packet from rxp buf to the addr buffer.
        memcpy(addr,&RXP_BUF[tail],n);   
        RXD_RING[tail].status = 0;
//        E1000_base[E1000_RDT/4] = (tail + 1)% RXD_MAX ; 
	E1000_base[E1000_RDT/4] = tail ;
//	cprintf("\n %d ",n);
	return n;
}
