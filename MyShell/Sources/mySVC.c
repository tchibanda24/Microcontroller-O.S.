/*
 * mySVC.c
 *
 *   Routines for supervisor calls
 *
 *  Created on: Nov 10, 2015
 *      Author: Thabani Chibanda
 */

/*
 * Size of the supervisor call stack frame (no FP extension):
 *   No alignment => 32 (0x20) bytes
 *   With alignment => 36 (0x24) bytes
 *
 * Format of the supervisor call stack frame (no FP extension):
 *   SP Offset   Contents
 *   +0			 R0
 *   +4			 R1
 *   +8			 R2
 *   +12		 R3
 *   +16		 R12
 *   +20		 LR (R14)
 *   +24		 Return Address
 *   +28		 xPSR (bit 9 indicates the presence of a reserved alignment
 *   				   word at offset +32)
 *   +32		 Possible Reserved Word for Alignment on 8 Byte Boundary
 *
 * Size of the supervisor call stack frame (with FP extension):
 *   No alignment => 104 (0x68) bytes
 *   With alignment => 108 (0x6C) bytes
 *
 * Format of the supervisor call stack frame (with FP extension):
 *   SP Offset   Contents
 *   +0			 R0
 *   +4			 R1
 *   +8			 R2
 *   +12		 R3
 *   +16		 R12
 *   +20		 LR (R14)
 *   +24		 Return Address
 *   +28		 xPSR (bit 9 indicates the presence of a reserved alignment
 *   				   word at offset +104)
 *   +32		 S0
 *   +36		 S1
 *   +40		 S2
 *   +44		 S3
 *   +48		 S4
 *   +52		 S5
 *   +56		 S6
 *   +60		 S7
 *   +64		 S8
 *   +68		 S9
 *   +72		 S10
 *   +76		 S11
 *   +80		 S12
 *   +84		 S13
 *   +88		 S14
 *   +92		 S15
 *   +96		 FPSCR
 *   +100		 Reserved Word for 8 Byte Boundary of Extended Frame
 *   +104		 Possible Reserved Word for Alignment on 8 Byte Boundary
 */

#define myMem_Disable 1
#include "mySVC.h"
#include "myMem.h"
#include "myIO.h"
#include <stdio.h>
#include <derivative.h>

extern int cmd_echo(int argc, char *argv[]);
extern int cmd_date(int argc, char *argv[]);
extern int cmd_help(int argc, char *argv[]);
extern int cmd_exit(int argc, char *argv[]);

#define XPSR_FRAME_ALIGNED_BIT 9
#define XPSR_FRAME_ALIGNED_MASK (1<<XPSR_FRAME_ALIGNED_BIT)

struct frame {
	union {
		int r0;
		int arg0;
		int returnVal;
		void* ptrreturnVal;
	};
	union {
		int r1;
		int arg1;
	};
	union {
		int r2;
		int arg2;
	};
	union {
		int r3;
		int arg3;
	};
	int r12;

	union {
		int r14;
		int lr;
	};

	int returnAddr;
	int xPSR;
};

/* Issue the SVC (Supervisor Call) instruction (See A7.7.175 on page A7-503 of the
 * ARM�v7-M Architecture Reference Manual, ARM DDI 0403Derrata 2010_Q3 (ID100710)) */

//supervisor mcreate instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCCreate(char *name) {
	__asm("svc %0" : : "I" (SVC_CREATE));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) SVCCreate(char* name) {
	__asm("svc %0" : : "I" (SVC_CREATE));
}
#endif

//supervisor mdelete instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCDelete(char *name) {
	__asm("svc %0" : : "I" (SVC_DELETE));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) SVCDelete(char *name) {
	__asm("svc %0" : : "I" (SVC_DELETE));
}
#endif

//supervisor mopen instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
stream* __attribute__((naked)) __attribute__((noinline)) SVCOpen(char *name, char *mode_char) {
	__asm("svc %0" : : "I" (SVC_OPEN));
	__asm("bx lr");
}
#else
stream* __attribute__((never_inline)) SVCOpen(char *name, char *mode_char) {
	__asm("svc %0" : : "I" (SVC_OPEN));
}
#endif

//supervisor mclose instruction
#ifdef __GNUC__
void __attribute__((naked)) __attribute__((noinline)) SVCClose(stream *strm) {
	__asm("svc %0" : : "I" (SVC_CLOSE));
	__asm("bx lr");
}
#else
void __attribute__((never_inline)) SVCClose(stream *strm) {
	__asm("svc %0" : : "I" (SVC_CLOSE));
}
#endif

//supervisor mget instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
char __attribute__((naked)) __attribute__((noinline)) SVCGet(stream *strm) {
	__asm("svc %0" : : "I" (SVC_GET));
	__asm("bx lr");
}
#else
char __attribute__((never_inline)) SVCGet(stream *strm) {
	__asm("svc %0" : : "I" (SVC_GET));
}
#endif

//supervisor mput instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCPut(stream *strm, char c) {
	__asm("svc %0" : : "I" (SVC_PUT));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) SVCPut(stream *strm, char c) {
	__asm("svc %0" : : "I" (SVC_PUT));
}
#endif

//supervisor mseek instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCSeek(stream *strm, int offset) {
	__asm("svc %0" : : "I" (SVC_SEEK));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) SVCSeek(stream *strm, int offset) {
	__asm("svc %0" : : "I" (SVC_SEEK));
}
#endif

//supervisor mget_desc instruction
#ifdef __GNUC__
void __attribute__((naked)) __attribute__((noinline)) SVCGet_Desc(stream *strm, int *fd, int *dd) {
	__asm("svc %0" : : "I" (SVC_GETDESC));
	__asm("bx lr");
}
#else
void __attribute__((never_inline)) SVCGet_Desc(stream *strm, int *fd, int *dd) {
	__asm("svc %0" : : "I" (SVC_GETDESC));
}
#endif

//supervisor mlist instruction
#ifdef __GNUC__
void __attribute__((naked)) __attribute__((noinline)) SVCList(void) {
	__asm("svc %0" : : "I" (SVC_LIST));
	__asm("bx lr");
}
#else
void __attribute__((never_inline)) SVCList(void) {
	__asm("svc %0" : : "I" (SVC_LIST));
}
#endif

//supervisor myMalloc instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
void* __attribute__((naked)) __attribute__((noinline)) SVCMalloc(unsigned int sz) {
	__asm("svc %0" : : "I" (SVC_MALLOC));
	__asm("bx lr");
}
#else
void* __attribute__((never_inline)) SVCMalloc(unsigned int sz) {
	__asm("svc %0" : : "I" (SVC_MALLOC));
}
#endif

//supervisor myFree instruction
#ifdef __GNUC__
void __attribute__((naked)) __attribute__((noinline)) SVCFree(void *ptr) {
	__asm("svc %0" : : "I" (SVC_FREE));
	__asm("bx lr");
}
#else
void __attribute__((never_inline)) SVCFree(void *ptr) {
	__asm("svc %0" : : "I" (SVC_FREE));
}
#endif

//supervisor myMemoryMap instruction
#ifdef __GNUC__
void __attribute__((naked)) __attribute__((noinline)) SVCMMAP() {
	__asm("svc %0" : : "I" (SVC_MMAP));
	__asm("bx lr");
}
#else
void __attribute__((never_inline)) SVCMMAP() {
	__asm("svc %0" : : "I" (SVC_MMAP));
}
#endif

//supervisor echo instruction
#ifdef __GNUC__
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCEcho(int argc, char *argv[]) {
	__asm("svc %0" : : "I" (SVC_ECHO));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) mlist(void) {
	__asm("svc %0" : : "I" (SVC_ECHO));
}
#endif


//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
void __attribute__((naked)) __attribute__((noinline)) SVCDate(int argc, char *argv[]) {
	__asm("svc %0" : : "I" (SVC_DATE));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) myMalloc(int argc, char *argv[]) {
	__asm("svc %0" : : "I" (SVC_DATE));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
uint32_t __attribute__((naked)) __attribute__((noinline)) SVCSpawn(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize) {
	__asm("svc %0" : : "I" (SVC_SPAWN));
	__asm("bx lr");
}
#else
int __attribute__((never_inline))  SVCSpawn(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize) {
	__asm("svc %0" : : "I" (SVC_SPAWN));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
void __attribute__((naked)) __attribute__((noinline)) SVCYield(void) {
	__asm("svc %0" : : "I" (SVC_YIELD));
	__asm("bx lr");
}
#else
void __attribute__((never_inline)) SVCYield(void); {
	__asm("svc %0" : : "I" (SVC_YIELD));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
void __attribute__((naked)) __attribute__((noinline))  SVCBlock(void) {
	__asm("svc %0" : : "I" (SVC_BLOCK));
	__asm("bx lr");
}
#else
void __attribute__((never_inline))  SVCYield(void); {
	__asm("svc %0" : : "I" (SVC_BLOCK));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCBlockPid(pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_BLOCKPID));
	__asm("bx lr");
}
#else
int __attribute__((never_inline))  SVCBlockPid(pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_BLOCKPID));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline))  SVCWake(pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_WAKE));
	__asm("bx lr");
}
#else
int __attribute__((never_inline))  SVCWake(pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_WAKE));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline))  SVCKill (pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_KILL));
	__asm("bx lr");
}
#else
int __attribute__((never_inline))  SVCKill(pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_KILL));
}
#endif

//supervisor date instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
void __attribute__((naked)) __attribute__((noinline))  SVCWait (pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_WAIT));
	__asm("bx lr");
}
#else
void __attribute__((never_inline))  SVCWait(pid_t targetPid) {
	__asm("svc %0" : : "I" (SVC_WAIT));
}
#endif

//supervisor help instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCHelp(int argc, char *argv[]) {
	__asm("svc %0" : : "I" (SVC_HELP));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) myFree(void *ptr]) {
	__asm("svc %0" : : "I" (SVC_HELP));
}
#endif

//supervisor exit instruction
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCExit(int argc, char *argv[]) {
	__asm("svc %0" : : "I" (SVC_EXIT));
	__asm("bx lr");
}
#else
int __attribute__((never_inline)) myMemoryMap() {
	__asm("svc %0" : : "I" (SVC_EXIT));
}
#endif

/*

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) SVCArtichoke(int arg0, int arg1, int arg2, int arg3) {
	__asm("svc %0" : : "I" (SVC_ARTICHOKE));
	__asm("bx lr");
}
#pragma GCC diagnostic pop
#else
int __attribute__((never_inline)) SVCArtichoke(int arg0, int arg1, int arg2, int arg3) {
	__asm("svc %0" : : "I" (SVC_ARTICHOKE));
}
#endif

 */

/* This function sets the priority at which the SVCall handler runs (See
 * B3.2.11, System Handler Priority Register 2, SHPR2 on page B3-723 of
 * the ARM�v7-M Architecture Reference Manual, ARM DDI 0403Derrata
 * 2010_Q3 (ID100710)).
 *
 * If priority parameter is invalid, this function performs no action.
 *
 * The ARMv7-M Architecture Reference Manual in section "B1.5.4 Exception
 * priorities and preemption" on page B1-635 states, "The number of
 * supported priority values is an IMPLEMENTATION DEFINED power of two in
 * the range 8 to 256, and the minimum supported priority value is always 0.
 * All priority value fields are 8-bits, and if an implementation supports
 * fewer than 256 priority levels then low-order bits of these fields are RAZ."
 *
 * In the K70 Sub-Family Reference Manual in section "3.2.2.1 Interrupt
 * priority levels" on page 85, it states, "This device supports 16 priority
 * levels for interrupts.  Therefore, in the NVIC each source in the IPR
 * registers contains 4 bits."  The diagram that follows goes on to confirm
 * that only the high-order 4 bits of each 8 bit field is used.  It doesn't
 * explicitly mention the System Handler (like the SVC handler) priorities,
 * but they should be handled consistently with the interrupt priorities.
 *
 * It is important to set the SVCall priority appropriately to allow
 * or disallow interrupts while the SVCall handler is running. */

void svcInit_SetSVCPriority(unsigned char priority) {
	if(priority > SVC_MaxPriority)
		return;

	SCB_SHPR2 = (SCB_SHPR2 & ~SCB_SHPR2_PRI_11_MASK) | SCB_SHPR2_PRI_11(priority << SVC_PriorityShift);
}

void svcHandlerInC(struct frame *framePtr);

/* Exception return behavior is detailed in B1.5.8 on page B1-652 of the
 * ARM�v7-M Architecture Reference Manual, ARM DDI 0403Derrata 2010_Q3
 * (ID100710) */

/* When an SVC instruction is executed, the following steps take place:
 * (1) A stack frame is pushed on the current stack (either the main or
 *     the process stack, depending on how the system is configured)
 *     containing the current values of R0-R3, R12, LR, the return
 *     address for the SVC instruction, xPSR, and, if appropriate, the
 *     floating point registers,
 * (2) An appropriate value is loaded into the LR register indicating
 *     that a stack frame is present on the stack (this will cause a
 *     special return sequence later when this value is loaded into
 *     the PC),
 * (3) Values of R0-R3 and R12 are no longer valid,
 * (4) The PC is set to the address in the interrupt vector table for
 * 	   the interrupt service routine for the SVC instruction,
 * (5) The processor switches to Handler mode (code execution in
 *     Handler mode is always privileged),
 * (6) The xPSR is set to indicate appropriate SVC state,
 * (7) The processor switches to the main stack by now using the main
 * 	   stack pointer.
 *
 * These steps are discussed in detail in the pseudo-code given for
 * processor action ExceptionEntry() on page B1-643 of the ARM�v7-M
 * Architecture Reference Manual, ARM DDI 0403Derrata 2010_Q3
 * (ID100710).  ExceptionEntry() invokes PushStack() and
 * ExceptionTaken() on page B1-643. */

/* The following assembler function makes R0 point to the top-of-stack
 * for the stack that was current (either the main or the process stack)
 * before the SVC interrupt service routine began running.  This is
 * where the stack frame was stored by the SVC instruction.  Then, this
 * function calls the svcHandlerInC function.  Note that when a C
 * function is called, R0 contains the first parameter.  Therefore, the
 * first parameter passed to svcHandlerInC is a pointer to the
 * top-of-stack of the stack containing the stack frame. */

#ifdef __GNUC__
void __attribute__((naked)) svcHandler(void) {
	__asm("\n\
            tst		lr, #4\n\
			ite		eq\n\
			mrseq	r0, msp\n\
			mrsne	r0, psp\n\
			push	{lr}\n\
			bl		svcHandlerInC\n\
			pop		{pc}\n\
			");
}
#else
__asm void svcHandler(void) {
	tst		lr, #4
	mrseq	r0, msp
	mrsne	r0, psp
	push	lr
	bl		svcHandlerInC
	pop		pc
}
#endif

void svcHandlerInC(struct frame *framePtr) {
	//printf("Entering svcHandlerInC\n");

	//printf("framePtr = 0x%08x\n", (unsigned int)framePtr);

	/* framePtr->returnAddr is the return address for the SVC interrupt
	 * service routine.  ((unsigned char *)framePtr->returnAddr)[-2]
	 * is the operand specified for the SVC instruction. */
	//printf("SVC operand = %d\n", ((unsigned char *)framePtr->returnAddr)[-2]);

	switch(((unsigned char *)framePtr->returnAddr)[-2]) {
	case SVC_CREATE:
		framePtr->returnVal = mcreate((char*)(framePtr->arg0));
		break;
	case SVC_DELETE:
		framePtr->returnVal = mdelete((char*)(framePtr->arg0));
		break;
	case SVC_OPEN:
		(framePtr->returnVal) = (unsigned int)(mopen((char*)(framePtr->arg0), (char*)(framePtr->arg1)));
		break;
	case SVC_CLOSE:
		mclose((stream*)framePtr->arg0);
		break;
	case SVC_GET:
		framePtr->returnVal = mget((stream*)(framePtr->arg0));
		break;
	case SVC_PUT:
		framePtr->returnVal = mput((stream*)(framePtr->arg0), (framePtr->arg1));
		break;
	case SVC_SEEK:
		framePtr->returnVal = mseek((stream*)(framePtr->arg0), (framePtr->arg1));
		break;
	case SVC_GETDESC:
		mget_desc((stream*)(framePtr->arg0), (int*)(framePtr->arg1), (int*)(framePtr->arg2));
		break;
	case SVC_LIST:
		mlist();
		break;
	case SVC_MALLOC:
		framePtr->ptrreturnVal = myMalloc((unsigned int)(framePtr->arg0));
		break;
	case SVC_FREE:
		myFree((void*)framePtr->arg0);
		break;
	case SVC_MMAP:
		myMemoryMap();
		break;
	case SVC_ECHO:
		framePtr->returnVal = cmd_echo((framePtr->arg0), (char**)(framePtr->arg1));
		break;
	case SVC_DATE:
		framePtr->returnVal = cmd_date((framePtr->arg0), (char**)(framePtr->arg1));
		break;
	case SVC_SPAWN:
		framePtr->returnVal = spawn(framePtr->arg0, framePtr->arg1, (char**)(framePtr->arg2), (uint32_t)(framePtr->arg3));
		break;
	case SVC_YIELD:
		yield();
		break;
	case SVC_BLOCK:
		block();
		break;
	case SVC_BLOCKPID:
		framePtr->returnVal = blockPid((uintptr_t)framePtr->arg0);
		break;
	case SVC_WAKE:
		framePtr->returnVal = wake((uintptr_t)framePtr->arg0);
		break;
	case SVC_KILL:
		framePtr->returnVal = kill((uintptr_t)framePtr->arg0);
		break;
	case SVC_WAIT:
		wait((uintptr_t)framePtr->arg0);
		break;
	case SVC_HELP:
		framePtr->returnVal = cmd_help((framePtr->arg0), (char**)(framePtr->arg1));
		break;
	case SVC_EXIT:
		framePtr->returnVal = cmd_exit((framePtr->arg0), (char**)(framePtr->arg1));
		break;
	default:
		printf("Unknown SVC has been called\n");
	}

	//printf("Exiting svcHandlerInC\n");
}
