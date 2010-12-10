/*
 * Motor library
 * Created by Tim O'Brien, t413.com
 * December 9, 2010
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

#ifndef MOTORS_H_
#define MOTORS_H_

/*----public macros----*/
#define MIN_CONTROL 1000
#define MID_CONTROL 1500
#define MAX_CONTROL 2000
#define MIN_SAFETY MIN_CONTROL + 100
#define MAX_SAFETY MAX_CONTROL - 100
#define MIN_THROTTLE MIN_CONTROL + 100

/*----public functions----*/
void setup_motors();
void write_motors(unsigned short front,unsigned short back,unsigned short left,unsigned short right);
void pulse_motors(unsigned char times,unsigned short delay);
void pulse_single_motor(unsigned char which_motor, unsigned short delay);

/*----public function macros----*/
#define write_motors_zero() write_motors(MIN_CONTROL, MIN_CONTROL, MIN_CONTROL, MIN_CONTROL)


#endif /* MOTORS_H_ */
