/*
 * pid.c
 *
 *  Created on: Nov 18, 2010
 *      Author: timo
 */

#include "../osHandles.h"
#include "pid.h"

#define ISF (100) //INTEGRAL_SCALING_FACTOR

void dual_clip(int32_t * val, unsigned short cap);


int16_t calculate_pid(int16_t incoming_val, int16_t goal_val, PID_DATA*pid){
	int16_t delta = goal_val - incoming_val;

	//integral calculation.
	pid->error += delta;
	//dual_clip( &(pid->error), (pid->i_limit) * PID_INTEGRAL_SCALING_FACTOR / pid->i;  //limits how high or low the error can get

	//derivative calculation.
	int16_t this_d = (incoming_val - pid->prev_val);
	pid->prev_val = incoming_val;

	unsigned long result = ((delta*ISF * pid->p) + (pid->error*ISF * pid->i) + (this_d*ISF * pid->d)) / ISF;

	return result;
}


void dual_clip(int32_t * val, unsigned short cap){
	if (*val > cap) *val = cap;
	else if (*val < -cap) *val = -cap;
}
