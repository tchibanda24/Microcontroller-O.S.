/*
 * myShell.h
 *
 *  Created on: Sept 9, 2015
 *      Author: Thabani Chibanda
 */

#ifndef _MYSHELL_H
#define _MYSHELL_H

#define myMem_DISABLE 1
#include "myMem.h"
#include "MyIO.h"
#include "mySVC.h"
#include "myInterrupt.h"
#include "process.h"
#include "uart.h"
#include "pushbutton.h"
#include "lcdc.h"
#include "led.h"
#include "switchCmd.h"
#include "lcdcConsole.h"
#include "sdram.h"
#include "priv.h"
#include "nvic.h"
#include "mcg.h"
#include "flexTimer.h"
#include "PDB.h"
#include "delay.h"
#include "util.h"
#include "profont.h"

#define LINE_MAX 256
#define NUM_CMDS 23
#define FALSE 0
#define TRUE 1
#define CHAR_EOF 4

//make all systme calls run Supervisor Control Instruction
#define mcreate SVCCreate
#define mdelete SVCDelete
#define mopen SVCOpen
#define mclose SVCClose
#define mput SVCPut
#define mget SVCGet
#define mseek SVCSeek
#define mlist SVCList
#define mget_desc SVCGet_Desc
#define malloc myMalloc
#define free SVCFree
#define mmap SVCMMAP

struct console console;
const int IRC;					/* Internal Reference Clock */
const int FLL_Factor;
const int KHzInHz;
const int peripheralClock;
const int baud;
stream *open_f[MAX_STREAMS];
stream *open_d[NUM_DEVS];

//initializers
void shell_Init(const int moduleClock);

//commands
int cmd_create(int argc, char *argv[]);
int cmd_delete(int argc, char *argv[]);
int cmd_open(int argc, char *argv[]);
int cmd_close(int argc, char *argv[]);
int cmd_read(int argc, char *argv[]);
int cmd_write(int argc, char *argv[]);
int cmd_seek(int argc, char *argv[]);
int cmd_list(int argc, char *argv[]);
int cmd_malloc(int argc, char *argv[]);
int cmd_free(int argc, char *argv[]);
int cmd_mmap(int argc, char *argv[]);
int cmd_echo(int argc, char *argv[]);
int cmd_setDate(int argc, char *argv[]);
int cmd_getDate(int argc, char *argv[]);
int cmd_date(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);
int cmd_exit(int argc, char *argv[]);

//test commands
int cmd_ser2lcd(int argc, char *argv[]);
int cmd_touch2led(int argc, char *argv[]);
int cmd_pot2ser(int argc, char *argv[]);
int cmd_therm2ser(int argc, char *argv[]);
int cmd_pb2led(int argc, char *argv[]);
int cmd_intTest1(int argc, char *argv[]);
int cmd_intTest2(int argc, char *argv[]);
int cmd_intTest3(int argc, char *argv[]);

#endif
