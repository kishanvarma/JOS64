#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H


#include <kern/pci.h>
#include <kern/pmap.h>


//used /* e1000_hw.h   Structures, enums, and macros for the MAC */

//From 14.5 
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */

//from 3.3.3 and 3.4 : required definitions.
#define E1000_TXD_CMD_EOP    0x01 /* End of Packet */
#define E1000_TXD_CMD_RS     0x08 /* Report Status */
#define E1000_TXD_STAT_DD    0x01 /* Descriptor Done */

//14.5
#define E1000_RDBAL    0x02800  /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH    0x02804  /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN    0x02808  /* RX Descriptor Length - RW */
#define E1000_RDH      0x02810  /* RX Descriptor Head - RW */
#define E1000_RDT      0x02818  /* RX Descriptor Tail - RW */
#define E1000_RCTL     0x00100  /* RX Control - RW */
#define E1000_RCTL_EN             0x00000002    /* enable */
#define E1000_RCTL_BAM            0x00008000    /* broadcast enable */
#define E1000_RCTL_SZ_2048        0x00000000    /* rx buffer size 2048 */
#define E1000_RCTL_SECRC          0x04000000    /* Strip Ethernet CRC */
//3.3
#define E1000_RXD_STAT_DD       0x01    /* Descriptor Done */
#define E1000_RXD_STAT_EOP      0x02    /* End of Packet */

#define E1000_RAL       0x05400  /* Receive Address - RW Array */
#define E1000_RAH      0x05404  /* Receive Address - RW Array */
#define E1000_RAH_AV  0x80000000        /* Receive descriptor valid */


struct tx_desc {
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
}__attribute__((packed));

struct packet{
	uint8_t packet_data[2048]; //Ill maintain max size packet here and truncate to 1518 in the tranmsit function. 
}__attribute__((packed));

struct rx_desc {
        uint64_t addr;
        uint16_t length;
        uint16_t cs;
        uint8_t status;
        uint8_t css;
        uint16_t special;
}__attribute__((packed));

#define TXD_MAX 64
#define TXD_BUFFER_SIZE 1518

#define RXD_MAX 128
#define RXD_BUFFER_SIZE 2048

struct tx_desc TXD_RING[TXD_MAX];
struct packet TXP_BUF[TXD_MAX];

struct rx_desc RXD_RING[RXD_MAX];
struct packet RXP_BUF[RXD_MAX];

int transmit(void *addr,int n);
int receive(void *addr,int n);
int pci_attach_E1000(struct pci_func *p_f);

#endif	// JOS_KERN_E1000_H
