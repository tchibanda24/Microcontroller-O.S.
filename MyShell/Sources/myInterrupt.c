/*
 * myInterrupt.c
 *
 *  Routines for handling interrupts
 *
 *  Created on: Nov 21, 2015
 *      Author: Thabani Chibanda
 */

#include "myInterrupt.h"
#include "mySVC.h"
#include "myMem.h"

//serial port 2 input character buffer
char SerPort2InBuffer[IO_BUFFER_SIZE];
int s2InEnq = 0;
int s2InDeq = 0;
int s2InCt = 0;

//serial port 2 output character buffer
char SerPort2OutBuffer[IO_BUFFER_SIZE];
int s2OutEnq = 0;
int s2OutDeq = 0;
int s2OutCt = 0;

//interrupt counters
int intrrCt = 0;
int intrrTDRECt = 0;
int intrrRDRFCt = 0;
int intrrNotDRCt = 0;

/*****************************************************************************
  Name: interruptSerialPort2
  Parameters: None
  Return value: None
  Side effects:
    The SerPort2InBuffer and associated data structures may be
    updated.
*****************************************************************************/
void interruptSerialPort2(void) {
	uint32_t status;
	char ch;

	intrrCt++;
	status = UART2_S1;

	if(!(status & UART_S1_TDRE_MASK) && !(status & UART_S1_RDRF_MASK)) {
		intrrNotDRCt++;
	}

	if((UART2_C2 & UART_C2_TIE_MASK) && (status & UART_S1_TDRE_MASK)) {

		char ch;
		intrrTDRECt++;

		if(s2OutCt > 0) {
			/* There is a character in the output buffer to be transmitted */

			/* Dequeue the character */
			ch = SerPort2OutBuffer[s2OutDeq++];
			s2OutDeq = s2OutDeq % IO_BUFFER_SIZE;
			s2OutCt--;

			/* write the character to the UART */
			UART_D_REG(UART2_BASE_PTR) = ch;
		}

		/* If there are no more characters in the output buffer, disable the transmitter interrupt */
		if(s2OutCt <= 0) {
			UART2_C2 &= ~UART_C2_TIE_MASK;
		}
	}

	if((UART2_C2 & UART_C2_RIE_MASK) && (status & UART_S1_RDRF_MASK)) {
		intrrRDRFCt++;

		/* read the character that caused the interrupt */
		ch = UART_D_REG(UART2_BASE_PTR);

		if(s2InCt < IO_BUFFER_SIZE) {
			/* There is room in the input buffer for another character */
			SerPort2InBuffer[s2InEnq++] = ch;
			s2InEnq = s2InEnq % IO_BUFFER_SIZE;
			s2InCt++;
		}

		/* If there is no room in the input buffer for this character; discard it */
	}
}

/* ***************************************************************************
  Name: getcharFromBuffer
  Parameters: None
  Return value:
    Type    Description
    char    the next character input from serial port 2.  This character
            will be retrieved from the SerPort2InBuffer in FIFO
            fashion.
  Side effects:
    The SerPort2InBuffer and associated data structures may be
    updated.
    Interrupts will be disabled and re-enabled by this routine.
  **************************************************************************** */
char getcharFromBuffer(void) {
	char ch;

	/* Guarantee the following operations are atomic */
	/* Disable interrupts (PRIMASK is set) */
	__asm("cpsid i");

	while(s2InCt <= 0) {
		/* No chars in the buffer; let's wait for at least one char to arrive */
		/* Allows interrupts (PRIMASK is cleared) */
		__asm("cpsie i");

		/* This is when an interrupt could occur */
		delay(100);

		/* Disable interrupts (PRIMASK is set) */
		__asm("cpsid i");
	}

	/* A character should be in the buffer; remove the oldest one. */
	ch = SerPort2InBuffer[s2InDeq++];
	s2InDeq = s2InDeq % IO_BUFFER_SIZE;
	s2InCt--;

	/* Allows interrupts (PRIMASK is cleared) */
	__asm("cpsie i");
	return ch;
}

/****************************************************************************
  Name: putcharIntoBuffer
  Parameters:
    Type    Description
    char    the character to be output over serial port 2.  This character
            will be buffered in the SerPort2OutBuffer in FIFO
            fashion.
  Return value: None
  Side effects:
    The SerPort2OutBuffer and associated data structures may be
    updated.
    Interrupts will be disabled and re-enabled by this routine.
*****************************************************************************/
void putcharIntoBuffer(char ch) {
	/* Guarantee the following operations are atomic */

	/* Disable interrupts (PRIMASK is set) */
	__asm("cpsid i");

	while(s2OutCt >= IO_BUFFER_SIZE) {
		/* The buffer is full; let's wait for at least one char to be removed */
		/* Allows interrupts (PRIMASK is cleared) */
		__asm("cpsie i");

		/* This is when an interrupt could occur */
		delay(100);

		/* Disable interrupts (PRIMASK is set) */
		__asm("cpsid i");
	}

	/* There is room in the output buffer for another character */
	SerPort2OutBuffer[s2OutEnq++] = ch;
	s2OutEnq = s2OutEnq % IO_BUFFER_SIZE;
	s2OutCt++;

	/* Enable the transmitter interrupt for UART2 using the UART2_C2 register
	 * (UART Control Register 2) (See 57.3.4 on page 1909 of the K70 Sub-Family Reference
	 * Manual, Rev. 2, Dec 2011) */
	UART2_C2 |= UART_C2_TIE_MASK;

	/* Allows interrupts (PRIMASK is cleared) */
	__asm("cpsie i");
}

/*****************************************************************************
  Name: putsIntoBuffer
  Parameters:
    Type    Description
    s       pointer to the string to be output over serial port 2.
            This string will be buffered in the SerPort2OutBuffer
            in FIFO fashion.
  Return value: None
  Side effects:
    The SerPort2OutBuffer and associated data structures may be
    updated.
    Interrupts will be disabled and re-enabled by this routine.
*****************************************************************************/
void putsIntoBuffer(char *s) {
	while(*s) {
		putcharIntoBuffer(*s++);
	}
}

void intSerialIOInit(void) {
	/* On reset (i.e., before calling mcgInit), the processor clocking
	 * starts in FEI (FLL Engaged Internal) mode.  In FEI mode and with
	 * default settings (DRST_DRS = 00, DMX32 = 0), the MCGFLLCLK, the
	 * MCGOUTCLK (MCG (Multipurpose Clock Generator) clock), and the Bus
	 * (peripheral) clock are all set to 640 * IRC.  IRC is the Internal
	 * Reference Clock which runs at 32 KHz. [See K70 Sub-Family
	 * Reference Manual, Rev. 2, Section 25.4.1.1, Table 25-22 on
	 * page 657 and MCG Control 4 Register (MCG_C4) Section 25.3.4 on
	 * page 641] */

	/* After calling mcgInit, MCGOUTCLK is set to 120 MHz and the Bus
	 * (peripheral) clock is set to 60 MHz.*/

	/* Table 5-2 on page 221 indicates that the clock used by UART0 and
	 * UART1 is the System clock (i.e., MCGOUTCLK) and that the clock
	 * used by UART2-5 is the Bus clock. */
	//const int IRC = 32000;					/* Internal Reference Clock */
	//const int FLL_Factor = 640;
	//const int moduleClock = FLL_Factor*IRC;
	//const int KHzInHz = 1000;

	//const int baud = 9600;

	//uartInit(UART2_BASE_PTR, moduleClock/KHzInHz, baud);

	/* Enable the receiver full interrupt for UART2 using the UART2_C2 register
	 * (UART Control Register 2) (See 57.3.4 on page 1909 of the K70 Sub-Family Reference
	 * Manual, Rev. 2, Dec 2011) */
	//UART2_C2 |= UART_C2_RIE_MASK;

	/* Enable interrupts from UART2 status sources and set its interrupt priority */
	//NVICEnableIRQ(UART2_STATUS_IRQ_NUMBER, UART2_STATUS_INTERRUPT_PRIORITY);
}
