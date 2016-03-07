/*
 * myScheduler.c
 *
 * Routines for handling processes(creation, deletion, blocking, scheduling etc.)
 *
 *  Created on: Dec 02, 2015
 *      Author: Thabani Chibanda
 */

#include "myShell.h"
#include "myScheduler.h"
#include "priv.h"
#include "derivative.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define PCB_MEM sizeof(proc) + sizeof(hw_frame) + sizeof(sw_frame)

extern int main(int argc, char *argv[]);

//function declarations
void SysTick_Init(void);
pid_t createProc(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize);
void SysTick_Handle(void);
void PendSV_Handler(void);
void context_switch(void);
static void *getSP(void);
static void setSP(void);
static inline void save_context(void);
static inline void load_context(void);

/*  *******************************             TYPEDEF DECLARATIONS             ***************************
    ********************************************************************************************************  */

typedef struct hw_frame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
} hw_frame;

typedef struct sw_frame {
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
} sw_frame;

typedef struct proc {
	addr_t sp;
	pid_t myPid;
	quant_t rem_q;
	state_t state;
	uint32_t prio;
	hw_frame *myFrame;
	sw_frame *myFrame2;

	proc *next;
	proc *prev;
} proc;

/*  ******************************                GLOBAL VARIABLES             ********************************
    ***********************************************************************************************************  */

static proc *p_first[] = {NULL, NULL, NULL}, *p_last[] = {NULL, NULL, NULL};
static proc *curr_proc = NULL;
static uint32_t quant[3];


static int curr_prio = 0;
static int num_proc = 0;
static uintptr_t curr_sp = 0;
static uintptr_t MAIN_STACK;

/*  *******************************                HELPER FUNCTIONS            ********************************
    ************************************************************************************************************  */

void processInit() {

	SysTick_Init();
	MAIN_STACK = (uintptr_t)getSP();
}

//setup the SysTick Timer
void SysTick_Init(void) {
	SYST_CSR = 0x7;
	SYST_RVR = (peripheralClock / 1000);
	SYST_CVR = SysTick_CVR_CURRENT(0x00);
	SYST_CSR = (SysTick_CSR_CLKSOURCE_MASK | SysTick_CSR_ENABLE_MASK);
}

//intialize a new process and retunr its PID
pid_t createProc(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize) {
	if(num_proc == MAX_PROC) return -1;

	num_proc++;
	proc *new_proc;
	hw_frame *proc_frame;
	sw_frame *proc_frame2;

	addr_t *my_mem = malloc(PCB_MEM);

	new_proc  = (proc*)my_mem;
	proc_frame = (hw_frame*)my_mem + sizeof(proc);
	proc_frame2 = (sw_frame*)proc_frame + sizeof(hw_frame);

	for(int i = 0; i < argc; i++) {
		if(i == 0)
			proc_frame->r0 = (uintptr_t)argv[i];
		if(i == 1)
			proc_frame->r1 = (uintptr_t)argv[i];
		if(i == 2)
			proc_frame->r2 = (uintptr_t)argv[i];
		if(i == 3)
			proc_frame->r3 = (uintptr_t)argv[i];
	}
	proc_frame->pc = ((uint32_t)main);
	proc_frame->lr = (uint32_t)0xFFFFFFFD;
	proc_frame->psr = 0x21000000; //default PSR value

	new_proc->sp = (uintptr_t)proc_frame2;
	new_proc->myPid = num_proc;
	new_proc->state = READY;
	new_proc->rem_q = (peripheralClock / 1000) / 5;
	new_proc->prio = 0;
	new_proc->myFrame = proc_frame;
	new_proc->myFrame2 = proc_frame2;
	new_proc->next = NULL;

	if(p_last[new_proc->prio] == NULL)
		p_last[new_proc->prio] = new_proc;
	else
		p_last[new_proc->prio]->next = new_proc;

	if(p_first[new_proc->prio] == NULL)
		p_first[new_proc->prio] = new_proc;
	else
		new_proc->prev = p_last[new_proc->prio];

	return num_proc;
}

//delete a process, and remove it from the linked list
int delete_proc(proc *p) {
	if(!p)  return -1;

	proc *prev, *next;

	if(p_first[p->prio] == p)
		p_first[p->prio] = p->next;

	if(p_last[p->prio] == p)
		p_last[p->prio] = p->prev;

	prev = p->prev;
	next = p->next;
	next->prev = prev;
	prev->next = next;

	free(p);

	return num_proc;
}

//The SysTick interrupt handler -- this grabs the main stack value then calls the context switcher
void SysTick_Handle(void) {

	if(SYS_CVR == 0) {
		if(curr_proc->prio > 3) {
			curr_proc->prio++;
			curr_proc->rem_q = Q[curr_proc->prio];
		}
	}

	save_context();  //The context is immediately saved
	curr_sp = (uintptr_t)getSP();
	context_switch();
	load_context(); //Since the PSP has been updated, this loads the last state of the new task
}

//This does the same thing as the SysTick handler -- it is just triggered in a different way
void PendSV_Handler(void) {

	save_context();  //The context is immediately saved
	curr_sp = (uintptr_t)getSP();
	context_switch();
	load_context(); //Since the PSP has been updated, this loads the last state of the new task
}

//This is the context switcher
void context_switch(void) {
	if(curr_proc == NULL) {
		curr_proc = p_first;
		curr_sp = curr_proc->sp;
		MAIN_STACK = getSP();
	}
	else {
		curr_proc = curr_proc->next;
	}

	curr_proc->sp = curr_sp;	//save current sp

	while(curr_proc) {
		if ( curr_proc == NULL) {
			//curr_proc = p_first;
			curr_sp = MAIN_RETURN;
			setSP();
			break;
		}
		if (curr_proc->state == READY) { //Check exec flag
			//change to unprivileged mode
			privUnprivileged();
			curr_sp = curr_proc->sp; //Use the thread stack upon handler return
			setSP();
			curr_proc->state == RUNNING;
			break;
		}
	}

	curr_sp = (uintptr_t)main;
	setSP();
}


//Scheduler for executing commands
void schedule(void) {

	curr_proc->sp = curr_sp;
	proc *curr = p_first[0];

	for(int i = 0; i < 3; i++) {
		while(curr) {
			if(curr->rem_q > 0) {
				privUnprivileged();
				curr_proc = curr;
				curr_sp = curr_proc->sp; //Use the thread stack upon handler return
				setSP();
				curr_proc->state == RUNNING;
				SysTickInit(curr_proc->prio);
				break;
			}
		}
	}

	SysTick_Init();
}

//Reads the main stack pointer
static void *getSP(void) {
	void* copy_sp = 0;

	__asm("mrs %[mspDest],msp" : [mspDest]"=r"(copy_sp));
	return copy_sp;
}

//Assigns the main stack pointer
static void setSP(void) {
	__asm("msr msp,%[mspSource]" : : [mspSource]"r"(curr_sp) : "sp");
}

//This saves the context on the PSP
static inline void save_context(void) {
	__asm("push {r4,r5,r6,r7,r8,r9,r10,r11}");

	__asm("ldr  r0, [%[shcsr]]"     "\n"
			"mov  r1, %[active]"      "\n"
			"orr  r1, r1, %[pended]"  "\n"
			"and  r0, r0, r1"         "\n"
			"push {r0}"
			:
			: [shcsr] "r" (&SCB_SHCSR),
			  [active] "I" (SCB_SHCSR_SVCALLACT_MASK),
			  [pended] "I" (SCB_SHCSR_SVCALLPENDED_MASK)
			  : "r0", "r1", "memory", "sp");
}

//This loads the context from the PSP
static inline void load_context(void) {
	__asm("pop {r4,r5,r6,r7,r8,r9,r10,r11}");

	__asm("pop {r0}"               "\n"
			"ldr r1, [%[shcsr]]"     "\n"
			"bic r1, r1, %[active]"  "\n"
			"bic r1, r1, %[pended]"  "\n"
			"orr r0, r0, r1"         "\n"
			"str r0, [%[shcsr]]"
			:
			: [shcsr] "r" (&SCB_SHCSR),
			  [active] "I" (SCB_SHCSR_SVCALLACT_MASK),
			  [pended] "I" (SCB_SHCSR_SVCALLPENDED_MASK)
			  : "r0", "r1", "sp", "memory");
}


/* ******************************               PROCESS FUNCTIONS            **********************************
   ************************************************************************************************************  */

// returns pid of current process
pid_t pid(void) {

	proc *p = getSP();
	return p->myPid;
}

/* Spawns a new process and schedules it. Main is the function to be run by the  newly created process
        argc is the # of arguments to be passed to main, with argv holding those arguments.
        StackSize is the size of the stack to be  allocated for the new process

    Returns indication of success w/ process id
*/
uint32_t spawn(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize) {

	pid_t new_pid = createProc(main, argc, argv, stackSize);
	schedule();
	return new_pid;
}

// yields remaining quantum
void yield(void) {

	context_switch();
	delProc();
}

// sets the current process to blocked state
void block(void) {

	addr_t *c_sp = getSP();
	proc *p = (proc*)(c_sp - 1);
	p->state = BLOCKED;
}

// sets the targetPid process to blocked state, does nothing if a that process id is not found
int blockPid(pid_t targetPid) {
	proc *curr = p_first;

	while(curr) {
		if(curr->myPid == targetPid) {
			curr->state = BLOCKED;
			return 1;
		}
		else {
			curr = curr->next;
		}
	}
	return 0;
}

// sets the targetPid process to ready state, does nothing if that process is not found
int wake(pid_t targetPid) {

	proc *curr = p_first;

	while(curr) {

		if(curr->myPid == targetPid) {
			curr->state = READY;
			return 1;
		}
		else {
			curr = curr->next;
		}
	}

	return 0;
}

// prematurely terminates the targetPid process, does nothing if that process is not found
int kill(pid_t targetPid) {

	proc *curr = p_first;

	while(curr) {

		if(curr->myPid == targetPid) {
			curr->state = -1;
			delete_proc;
			return 1;
		}
		else {
			curr = curr->next;
		}
	}

	return 0;
}

// waits for the targetPid process to end execution (naturally or prematurely)
void wait(pid_t targetPid) {

	proc *curr = p_first;

	while(curr) {
		if(curr->myPid == targetPid) {
			while(curr->state == RUNNING) {
			}
			break;
		}
		else {
			curr = curr->next;
		}
	}
}
