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

#include "../osHandles.h"
#include "motors.h"
#include "pwm.h"

#define FRONT_TIMER 6
#define BACK_TIMER 5
#define LEFT_TIMER 2
#define RIGHT_TIMER 4


/*----public functions----*/
void setup_motors(){
	setup_PWM_servo();

	setupPWM(FRONT_TIMER); //front
	setupPWM(BACK_TIMER); //back
	setupPWM(LEFT_TIMER); //left
	setupPWM(RIGHT_TIMER); //right (set up P0.7 as PWM2)
}

void write_motors(unsigned short front,unsigned short back,unsigned short left,unsigned short right){
	write_PWM_servo(FRONT_TIMER, front ); //front
	write_PWM_servo(BACK_TIMER, back ); //back
	write_PWM_servo(LEFT_TIMER, left ); //left
	write_PWM_servo(RIGHT_TIMER, right ); //right
}

void pulse_motors(unsigned char times,unsigned short delay){
	for ( ;times; times--){
		write_motors(MIN_THROTTLE, MIN_THROTTLE, MIN_THROTTLE, MIN_THROTTLE);
		vTaskDelay(delay/2);
		write_motors(MIN_CONTROL, MIN_CONTROL, MIN_CONTROL, MIN_CONTROL);
		vTaskDelay(delay/2);
	}
}

void pulse_single_motor(unsigned char which_motor, unsigned short delay){
	unsigned char map_motor_to_timer[] = {FRONT_TIMER, BACK_TIMER, LEFT_TIMER, RIGHT_TIMER};
	write_PWM_servo(map_motor_to_timer[which_motor], MIN_THROTTLE );
	vTaskDelay(delay);
	write_motors_zero();
}
