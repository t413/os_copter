/* 
 * PWM library
 * Created by Tim O'Brien, t413.com
 * September 24, 2010
 *
 * * Released under the Creative Commons Attribution-ShareAlike 3.0
 * * You are free 
 * *  - to Share, copy, distribute and transmit the work
 * *  - to Remix -- to adapt the work
 * * Provided 
 * *  - Attribution (my name and website in the comments in your source)
 * *  - Share Alike - If you alter, transform, or build upon this work, 
 * *      you may distribute the resulting work only under 
 * *      the same or similar license to this one
 *
 * if you are unfamiliar with the terms of this license,
 *  would like to see the full legal code of the license,
 *  or have any questions please email me (timo@t413.com) or visit:
 *  http://creativecommons.org/licenses/by-sa/3.0/us/   for details
 */

#include "pwm.h"
#include "../System/lpc214x.h"

void setup_all_PWM(unsigned int resolution, unsigned int prescale){
	PWMPR = prescale;  //PWM prescale register - divide peripheral clock by 4
	PWMMR0 = resolution;  //make pwm 10bit
	PWMMCR = 0x02;  //reset on PWMMR0
}

void setupPWM(unsigned int whichTimer){
	switch (whichTimer) {
		case 1: //set up P0.0 as PWM1
			PINSEL0 &= ~(3<<0); //clear bit 0 and 1
			PINSEL0 |= (1<<1); //bit 0 = 1
			break;
		case 2: //set up P0.1 as PWM2
			PINSEL0 &= ~(3<<14); //SET 14 and 15 to 0
			PINSEL0 |= (1<<15); //SET 15 to 1
			break;
		case 3: //set up P0.7 as PWM3
			PINSEL0 &= ~(3<<2);
			PINSEL0 |= (1<<3);
			break;
		case 4: //set up P0.8 as PWM4
			PINSEL0 &= ~(3<<16);
			PINSEL0 |= (1<<17);
			break;
		case 5: //set up P0.21 as PWM5
			PINSEL1 &= ~(3<<10);
			PINSEL1 |= (1<<10);
			break;
		case 6: //set up P0.9 as PWM6
			PINSEL0 &= ~(3<<18);
			PINSEL0 |= (1<<19);
			break;
	}
	PWMMCR |= (1<<(whichTimer*3+1));  // Reset PWMTC if PWMMR_ matches it.
	if (whichTimer > 1)
		PWMPCR  &= ~(1<<(whichTimer));  // single edge controlled mode. (only for PWM2-6)
	PWMPCR |= (1<<(whichTimer+8)); // The PWM2 output enabled.
	PWMMCR = 0x02;  //reset on PWMMR0

	PWMTCR = 0x09;//Go! PWM Timer Control Register (bit0=Counter Enable, 3=PWM Enable)
}


/* these are the memory offsets to go from &PWMMR1 to &PWMMR_ */
#define PWM2_O ((&PWMMR2)-(&PWMMR1))
#define PWM3_O ((&PWMMR3)-(&PWMMR1))
#define PWM4_O ((&PWMMR4)-(&PWMMR1))
#define PWM5_O ((&PWMMR5)-(&PWMMR1))
#define PWM6_O ((&PWMMR6)-(&PWMMR1))

void write_PWM(unsigned char chan, unsigned int val){
	if ((chan>6)||(chan<1)) return; //error, which timer to use is out of range.
	unsigned long * pwm_mr_to_use = (unsigned long*)(&PWMMR1 + ( (chan==1)?0:(chan==2)?PWM2_O:(chan==3)?PWM3_O:(chan==4)?PWM4_O:(chan==5)?PWM5_O:PWM6_O  ));
	if (*pwm_mr_to_use != val){
		*pwm_mr_to_use = val; //set the match register for the PWM output.
		PWMLER |= (1 << chan);// Latch PWM registers
	}
}

