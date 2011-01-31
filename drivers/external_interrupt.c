/*
 * external_interrupt.c
 *
 *  Created on: Jan 25, 2011
 *      Author: timo
 */

#include "external_interrupt.h"

#include "../System/lpc214x.h"
#include "../System/sysConfig.h"
#include "../FreeRTOS/FreeRTOS.h"


/*----- private variables -----*/
volatile unsigned int last_triggered, delta_t;


/*----- private declarations -----*/
void EINT0_Handler( void );// __attribute__ ((interrupt));
void setupTimer1();


/*----- function definitions -----*/
void enable_interupt_ext0( void )
{
	/* ---- setup P0.16 as INT0 ----*/
	PINSEL1 &= ~(3<<0); //clear bit 0 and 1
	PINSEL1 |= (1<<0); //bit 0 = 1
    EXTMODE |= 1; //EXTMODE0 as edge sensitive
    EXTPOLAR |= 1; //make EXT0 rising edge sensitive
    //VICIntEnClear = (1<<14); //disable EXT0 interrupt.
    //VICIntSelect &= ~(1<<14); //select IRQ mode for EXT0
    VICVectAddr9 = ( long ) EINT0_Handler;
    VICVectCntl9  = (1<<5) | 14; //enable vector 6 for EINT0 (see table 57)
    VICIntEnable |= (1<<14); //enable EXT0 as an interrupt.

    last_triggered = 0;
    delta_t = 0;

    setupTimer1();
}

unsigned int pwm_in_delta_t(){
	return delta_t;
}

void EINT0_Handler( void ) {
	portSAVE_CONTEXT();

	unsigned int current_time = T1TC;
	delta_t = current_time - last_triggered;
	last_triggered = current_time;

	EXTINT |= 0; //clear EINT0
	VICVectAddr = 0;	//Acknowledge interrupt
	portRESTORE_CONTEXT();
}

void setupTimer1(){
        T1TCR = 0x0; //disable counter

        T1TC  = 0x0;
        T1PR  = PCLK/1000; //  /1000000;
        T1PC  = 0x0;
        T1TCR = 0x1; //Counter Enable for timer0
}
