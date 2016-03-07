/*
 * myScheduler.h
 *
 *
 *  Created on: Dec 02, 2015
 *      Author: Thabani Chibanda
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_PROC 6
#define RUNNING 0
#define BLOCKED 1
#define READY 2
#define SLEEP 3

#define Q1 (periperhalclock/1000) / 10
#define Q2 (periperhalclock/1000) / 5
#define Q3 (periperhalclock/1000) / 4

typedef uint32_t uintptr_t;
typedef uint32_t pid_t;
typedef uint32_t quant_t;
typedef uint32_t addr_t;
typedef uint32_t state_t;
typedef struct proc proc;
typedef struct hw_frame hw_frame;
typedef struct sw_frame sw_frame;


pid_t pid(void);
uint32_t spawn(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize);
void yield(void);
void block(void);
int blockPid(pid_t targetPid);
int wake(pid_t targetPid);
int kill(pid_t targetPid);
void wait(pid_t targetPid);

#endif /* PROCESS_H_ */
