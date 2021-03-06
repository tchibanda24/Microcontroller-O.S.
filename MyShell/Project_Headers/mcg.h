/*
 * mcg.h
 *
 *  Created on: Nov 6, 2015
 *      Author: Thabani Chibanda
 */

#ifndef _MCG_H
#define _MCG_H

/* Divider factors for clocks used by the SIM_CLKDIV1 register (System Clock
 * Divider Register 1) (See 12.2.16 on page 347 of the K70 Sub-Family Reference
 * Manual, Rev. 2, Dec 2011) */

#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_1 0
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_2 1
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_3 2
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_4 3
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_5 4
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_6 5
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_7 6
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_8 7
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_9 8
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_10 9
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_11 10
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_12 11
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_13 12
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_14 13
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_15 14
#define SIM_CLKDIV1_OUTDIV_DIVIDE_BY_16 15

void mcgInit(void);

#endif /* ifndef _MCG_H */
