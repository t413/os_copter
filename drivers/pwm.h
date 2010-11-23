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


#ifndef __pwm_h
#define __pwm_h

/* 
 * setup_all_PWM  -  setup the timers to work with PWM.
 *  Inputs: 
 *   -resolution of the timer (you may later write a value from 0 to <this> value)
 *   -prescale value (scales down the peripheral clock) 
 */
void setup_all_PWM(unsigned int resolution, unsigned int prescale);

/* 
 * setupPWM  -  setup specific timer for PWM output.
 *  Inputs: 
 *   -which timer to setup (1-6)
 */
void setupPWM(unsigned int whichTimer);


/*  servo functionality!!
 *
 *    |
 *    |__ __
 *    | |  |
 *    | |  |                        |repeat here
 *   _|_|__|______________________________
 *   0  1  2  3  4  5  6  7  8  9  10 ms
 *
 *   the first argument, 10000, sets how long (in µs) the period of the
 */
#define SERVO_PERIOD 20000
#define setup_PWM_servo() setup_all_PWM(SERVO_PERIOD,PCLK/1000000)  //PCLK/1000000 => µs scale
#define write_PWM_servo(chan,val) write_PWM(chan, (val>2000)?2000:(val<1000)?1000:val );


/*
 * write_PWM  -  write a value to modulate.
 *  Inputs:
 *   -which timer to use, only 1-6 are valid.
 *   -value to use (must be between 0 and the resolution specified in setup_all_PWM)
 */
void write_PWM(unsigned char whichTimer, unsigned int val);


/* 
 * writePWM (1-6)  -  write a value to modulate.
 *  Inputs: 
 *   -value to use (must be between 0 and the resolution specified in setup_all_PWM)
 */
#define writePWM1(val) write_PWM(1,val)
#define writePWM2(val) write_PWM(1,val)
#define writePWM3(val) write_PWM(1,val)
#define writePWM4(val) write_PWM(1,val)
#define writePWM5(val) write_PWM(1,val)
#define writePWM6(val) write_PWM(1,val)

#endif  // __pwm_h
