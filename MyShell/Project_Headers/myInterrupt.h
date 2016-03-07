/*
 * myInterrupt.h
 *
 *  Created on: Nov 21, 2015
 *      Author: Thabani Chibanda
 */

#ifndef MYINTERRUPT_H_
#define MYINTERRUPT_H_

//#define myMem_DISABLE 1
#include "uart.h"
#include "sdram.h"
#include "nvic.h"
#include "priv.h"
#include "flexTimer.h"
#include "PDB.h"
#include "delay.h"
#include "util.h"


#define IO_BUFFER_SIZE 128


void interruptSerialPort2(void) ;
char getcharFromBuffer(void);
void putcharIntoBuffer(char ch);
void putsIntoBuffer(char *s);
void intSerialIOInit(void);

//serial port 2 input character buffer
char SerPort2InBuffer[IO_BUFFER_SIZE];
int s2InEnq;
int s2InDeq;
int s2InCt;

//serial port 2 output character buffer
char SerPort2OutBuffer[IO_BUFFER_SIZE];
int s2OutEnq;
int s2OutDeq;
int s2OutCt;

//interrupt counters
int intrrCt;
int intrrTDRECt;
int intrrRDRFCt;
int intrrNotDRCt;


#endif /* MYINTERRUPT_H_ */
