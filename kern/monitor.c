// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/pmap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/dwarf.h>
#include <kern/kdebug.h>
#include <kern/dwarf_api.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace","Prints Stack Back Trace",mon_backtrace},
	{ "showmappings", "Displays mappings in the given range", mon_display}};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	uint64_t *rbp = (uint64_t*)read_rbp();
	uint64_t rip;
	read_rip(rip);
	struct Ripdebuginfo DebugInfo;
	int i=0;	

	cprintf("Stack backtrace:\n");
	while(rbp){
		cprintf("  rbp %016x  rip %016x\n",rbp,rip);
	
		debuginfo_rip(rip,&DebugInfo);
		cprintf("       %s:%d: %s+%x  args:%d  ",DebugInfo.rip_file,DebugInfo.rip_line,DebugInfo.rip_fn_name,rip-DebugInfo.rip_fn_addr,DebugInfo.rip_fn_narg);
		for(i=1;i<=DebugInfo.rip_fn_narg;i++){
			cprintf("%016x ",*(rbp-i)>>32);
		}		
		cprintf("\n");
		rip = *(rbp + 1 );
		rbp = (uint64_t*)rbp[0];
	}
	return 0;
}

uint64_t 
atoi(char *s)
{
	uint64_t i = 0;
	//check the format is correct,.
	
	if( (*s != '0') || (*(s+1) != 'x')) {
		cprintf("\n wrong format\n");
		return -1;  //handle this negative number there,
	}
	//bypass 0 and x in the string to get the numeric value of the remaining

	for(s = s + 2; *s; s++){
		if(*s >= 'a' && *s <= 'f')
		   *s = *s - 'a' + 10 ;
		i = i*16 + *s ;;
	}
	return i;
}


int
mon_display(int argc, char **argv, struct Trapframe *tf)
{
        if(argc !=3){
                cprintf("\n Usage :showmappings start end \n start and end in Hex format 0x..\n");
                return 0;
        }
	uint64_t start = atoi(argv[1]);
	uint64_t end = atoi(argv[2]);
	if((start == -1) || (end == -1))
		return 0;
	uint64_t i = 0;	
	for( i = start; i <= end; i+=PGSIZE){
		pte_t *pte = pml4e_walk(boot_pml4,(void *)i,1);
		if(!(*pte & PTE_P))
			cprintf("\n page does not exist \n");
		
		cprintf("\n %x - PTE_P : %d\tPTE_W : %d\tPTE_U : %d\n",i,*pte & PTE_P ,*pte & PTE_W ,*pte & PTE_U);
	}
	
	return 0;
}
/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
