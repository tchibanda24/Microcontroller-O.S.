/**
 * pushbutton.c
 * routines to manipulate the pushbuttons
 *
 * ARM-based K70F120M microcontroller board
 *   for educational purposes only
 * CSCI E-92 Spring 2014, Professor James L. Frankel, Harvard Extension School
 *
 * Written by James L. Frankel (frankel@seas.harvard.edu)
 */

#include "derivative.h"
#include "pushbutton.h"

/**
 * Routine to initialize both of the pushbuttons
 * 
 * Note: This procedure *does* enable the appropriate port clocks
 */
void pushbuttonInitAll(void) {
	/* Enable the clocks for PORTD & PORTE */
	SIM_SCGC5 |= (SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);

	pushbuttonSW1Config();
	pushbuttonSW2Config();
}

/**
 * Routine to configure pushbutton SW1
 * 
 * Note: This procedure does not enable the port clock
 */
void pushbuttonSW1Config(void) {
	/* Configure bit PUSHBUTTON_SW1_PORTD_BIT of PORTD using the Pin Control
	 * Register (PORTD_PCR) to be a GPIO pin.  This sets the MUX field (Pin Mux
	 * Control) to GPIO mode (Alternative 1).  Also, by setting the PE bit,
	 * enable the internal pull-up or pull-down resistor.  And, by setting the
	 * PS bit, enable the internal pull-up resistor -- not the pull-down
	 * resistor.  (See 11.4.1 on page 309 of the K70 Sub-Family Reference
	 * Manual, Rev. 2, Dec 2011) */
	PORT_PCR_REG(PORTD_BASE_PTR, PUSHBUTTON_SW1_PORTD_BIT) =
			PORT_PCR_MUX(PORT_PCR_MUX_GPIO) |
			PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	/* By not setting the Port Data Direction Register (GPIOx_PDDR) to be a GPIO
	 * output, we have configured bit PUSHBUTTON_SW1_PORTD_BIT of PORTD to be a
	 * GPIO input.  (See 60.2.6 on page 2155 of the K70 Sub-Family Reference
	 * Manual, Rev. 2, Dec 2011) */
}

/**
 * Routine to configure pushbutton SW2
 * 
 * Note: This procedure does not enable the port clock
 */
void pushbuttonSW2Config(void) {
	/* Configure bit PUSHBUTTON_SW2_PORTE_BIT of PORTE to be a GPIO pin with an
	 * internal pull-up resistor. */
	PORT_PCR_REG(PORTE_BASE_PTR, PUSHBUTTON_SW2_PORTE_BIT) =
			PORT_PCR_MUX(PORT_PCR_MUX_GPIO) |
			PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	/* Configure bit PUSHBUTTON_SW2_PORTE_BIT of PORTE to be a GPIO input. */
}

/**
 * Routine to read state of pushbutton SW1
 */
int sw1In(void) {
	/* Returns 1 when pushbutton SW1 is depressed and 0 otherwise */
	int pushbuttonState;

	/* Read the state of bit PUSHBUTTON_SW1_PORTD_BIT of PORTD using the Port Data
	 * Input Register (GPIOx_PDIR).  (See 60.2.5 on page 2155 of the K70
	 * Sub-Family Reference Manual, Rev. 2, Dec 2011) */
	pushbuttonState = PTD_BASE_PTR->PDIR & (1 << PUSHBUTTON_SW1_PORTD_BIT);
	return !pushbuttonState;
}

/**
 * Routine to read state of pushbutton SW2
 */
int sw2In(void) {
	/* Returns 1 when pushbutton SW2 is depressed and 0 otherwise */
	int pushbuttonState;

	/* Read the state of bit PUSHBUTTON_SW2_PORTE_BIT of PORTE */
	pushbuttonState = PTE_BASE_PTR->PDIR & (1 << PUSHBUTTON_SW2_PORTE_BIT);
	return !pushbuttonState;
}

