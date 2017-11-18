// Fork a binary tree of processes and display their structure.

#include <inc/lib.h>

#define DEPTH 3

void forktree(const char *cur);

void
forkchild(const char *cur, char branch)
{
	char nxt[DEPTH+1];

	if (strlen(cur) >= DEPTH)
		return;

//	cprintf("\n in forkchild \n");
	
	snprintf(nxt, DEPTH+1, "%s%c", cur, branch);
	if (fork() == 0) {
		cprintf("\nthis is child \n");
		forktree(nxt);
		exit();
	}
//cprintf("\n in forkchild \n");
}

void
forktree(const char *cur)
{
	cprintf("%04x: I am '%s'\n", sys_getenvid(), cur);
	
	forkchild(cur, '0');
	
//	cprintf("\n after forkchild 1\n");	

	forkchild(cur, '1');
}

void
umain(int argc, char **argv)
{
	forktree("");
}

