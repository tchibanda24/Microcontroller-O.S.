/**
 *
 * mySVC.h
 *
 *  Created on: Nov 10, 2015
 *      Author: Thabani Chibanda
 */

#ifndef MYSVC_H_
#define MYSVC_H_

#include "myIO.h"
#include "process.h"

#define SVC_MaxPriority 15
#define SVC_PriorityShift 4

// Implemented SVC numbers


#define SVC_CREATE 0
#define SVC_DELETE 1
#define SVC_OPEN 2
#define SVC_CLOSE 3
#define SVC_GET 4
#define SVC_PUT 5
#define SVC_SEEK 6
#define SVC_GETDESC 7
#define SVC_LIST 8
#define SVC_MALLOC 9
#define SVC_FREE 10
#define SVC_MMAP 11
#define SVC_DATE 12
#define SVC_ECHO 13
#define SVC_SPAWN 14
#define SVC_YIELD 15
#define SVC_BLOCK 16
#define SVC_BLOCKPID 17
#define SVC_WAKE 18
#define SVC_KILL 19
#define SVC_WAIT 20
#define SVC_HELP 21
#define SVC_EXIT 22


void svcInit_SetSVCPriority(unsigned char priority);
void svcHandler(void);

int SVCCreate(char *name);
int SVCDelete(char *name);
stream* SVCOpen(char *name, char *mode_char);
void  SVCClose(stream *strm);
char SVCGet(stream *strm);
int SVCPut(stream *strm, char c);
int SVCSeek(stream *strm, int offset);
void SVCGet_Desc(stream *strm, int *fd, int *dd);
void SVCList(void);
void* SVCMalloc(unsigned int sz);
void SVCFree(void *ptr);
void SVCMMAP();
void SVCDate(int argc, char *argv[]);
int SVCEcho(int argc, char *argv[]);
uint32_t SVCSpawn(int main(int argc, char *argv[]), int argc, char *argv[], uint32_t stackSize);
void SVCYield(void);
void SVCBlock(void);
int SVCBlockPid(pid_t targetPid);
int SVCWake(pid_t targetPid);
int SVCKill(pid_t targetPid);
void SVCWait(pid_t targetPid);

#endif
