/*
 * pid.c
 *
 *  Created on: Nov 18, 2010
 *      Author: timo
 */

#include "pid.h"

float calculate_pid(float incoming_val, float goal_val, PID_DATA*pid){
	float delta = goal_val - incoming_val;

	//integral calculation.
	pid->error += delta;

	//derivative calculation.
	float this_d = (incoming_val - pid->prev_val);
	pid->prev_val = incoming_val;

	return (delta * pid->p) + (pid->error * pid->i) + (this_d * pid->d);
}
