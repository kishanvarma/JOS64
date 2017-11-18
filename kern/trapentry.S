/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/macro.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;			/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;				/* align function definition */		\
	name:					/* function starts here */		\
	pushq $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushq $0;							\
	pushq $(num);							\
	jmp _alltraps


.text


/*
 * Lab 3: Your code here for generating entry points for the different traps.
   Find the PUSHA, POPA macros in kern/macro.h. Since amd64 doesn't support
   pusha, popa so you will find these macros handy.
 */


TRAPHANDLER_NOEC(MY_divide ,0 )
TRAPHANDLER_NOEC(MY_debug ,1 )
TRAPHANDLER_NOEC(MY_nmi ,2 )
TRAPHANDLER_NOEC(MY_brkpnt ,3 )
TRAPHANDLER_NOEC(MY_oflow ,4 )
TRAPHANDLER_NOEC(MY_bound ,5 )
TRAPHANDLER_NOEC(MY_illop ,6 )
TRAPHANDLER_NOEC(MY_device ,7 )
TRAPHANDLER(MY_dblflt,8 )
TRAPHANDLER(MY_tss ,10 )
TRAPHANDLER(MY_segnp ,11 )
TRAPHANDLER(MY_stack ,12 )
TRAPHANDLER(MY_gpflt ,13 )
TRAPHANDLER(MY_pgflt ,14 )
TRAPHANDLER_NOEC(MY_fperr ,16 )
TRAPHANDLER(MY_align ,17 )
TRAPHANDLER_NOEC(MY_mchk ,18 )
TRAPHANDLER_NOEC(MY_simderr ,19 )

TRAPHANDLER_NOEC(MY_syscall ,48 )
TRAPHANDLER_NOEC(MY_default ,500 )


TRAPHANDLER_NOEC(MY_IRQ0 ,32 )
TRAPHANDLER_NOEC(MY_IRQ1 ,33 )
TRAPHANDLER_NOEC(MY_IRQ2 ,34 )
TRAPHANDLER_NOEC(MY_IRQ3 ,35 )
TRAPHANDLER_NOEC(MY_IRQ4 ,36 )
TRAPHANDLER_NOEC(MY_IRQ5 ,37 )
TRAPHANDLER_NOEC(MY_IRQ6 ,38 )
TRAPHANDLER_NOEC(MY_IRQ7 ,39 )
TRAPHANDLER_NOEC(MY_IRQ8 ,40 )
TRAPHANDLER_NOEC(MY_IRQ9 ,41 )
TRAPHANDLER_NOEC(MY_IRQ10 ,42 )
TRAPHANDLER_NOEC(MY_IRQ11 ,43 )
TRAPHANDLER_NOEC(MY_IRQ12 ,44 )
TRAPHANDLER_NOEC(MY_IRQ13 ,45 )
TRAPHANDLER_NOEC(MY_IRQ14 ,46 )
TRAPHANDLER_NOEC(MY_IRQ15 ,47 )

/*
 * Lab 3: Your code here for _alltraps
 *
 * Hint: Be sure to review the x64 calling convention from lab1
 *       for how to pass a pointer to the trapframe.
 */

_alltraps :

	sub $16,%rsp  
    	movw %ds,8(%rsp)
	movw %es,0(%rsp)	
	PUSHA
	
	movw $GD_KD,%rax
	movw %rax,%ds
	movw %rax,%es
	movq %rsp,%rdi
	call trap
	
	POPA_
	movw 0(%rsp),%es
	movw 8(%rsp),%ds
	add $16,%rsp

	iret    //to return : returns from a system call back to a user program
