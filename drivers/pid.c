/*
 * pid.c
 *
 *  Created on: Nov 18, 2010
 *      Author: timo
 */

#include "../osHandles.h"
#include "pid.h"

#define INTEGRAL_SCALING_FACTOR (1<<10) //(1<<10) = 1,024.

void dual_clip(long * val, unsigned short cap);


int32_t calculate_pid(int16_t incoming_val, int16_t goal_val, PID_DATA*pid){
	int16_t delta = goal_val - incoming_val;

	//integral calculation.
	pid->error += delta;
	dual_clip( &(pid->error), (pid->i_limit) * INTEGRAL_SCALING_FACTOR/pid->i );  //limits how high or low the error can get

	//derivative calculation.
	int16_t this_d = (incoming_val - pid->prev_val);
	pid->prev_val = incoming_val;

	return ( (delta * pid->p) + (pid->error * pid->i)/100 + (this_d * pid->d)) / INTEGRAL_SCALING_FACTOR;
}


void dual_clip(long * val, unsigned short cap){
	if (*val > cap) *val = cap;
	else if (*val < -cap) *val = -cap;
}
