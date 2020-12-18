/*
 * SysTick.h
 *
 *  Created on: Dec 17, 2020
 *      Author: marcelobaez
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_
#include "inc/tm4c123gh6pm.h"
//#include "inc/hw_ints.h"
//#include "inc/hw_memmap.h"


void SysTick_Init_ms( unsigned long baseTempo_ms );
void SysTick_Init_us( unsigned long baseTempo_us );
unsigned char SysTickRun( void );
void SysTickWaitBusy( unsigned long time );



#endif /* SYSTICK_H_ */
