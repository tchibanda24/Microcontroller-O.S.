/*
 * myMem.h
 *
 *  Created on: Sept 24, 2015
 *      Author: Thabani Chibanda
 */
#ifndef myMem_H

#define myMem_H 1
#include <stdlib.h>
#include "myInterrupt.h"
#include "lcdcConsole.h"
#include "pushbutton.h"
#include "led.h"
#include "delay.h"
#include "mcg.h"
#include "sdram.h"
#include "uart.h"
#include "lcdc.h"

//total Mbytes
#define MByte 1500

typedef uint32_t pid_t;

//necessary functions
void *myMalloc(unsigned int sz);
void myFree(void *ptr);
void myMemoryMap();

#if !(myMem_DISABLE)
#define malloc(sz)             	 myMalloc(sz)
#define free(ptr)              		 myFree(ptr)
#define mmap()                 	 myMemoryMap()
#endif

#endif
