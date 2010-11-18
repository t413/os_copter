/*
 * pid.c
 *
 *  Created on: Nov 18, 2010
 *      Author: timo
 */

#include "pid.h"

int calculate_pid(int incoming_val, int goal_val, PID_DATA*pid){
	int delta = goal_val - incoming_val;

	//integral calculation.
	pid->error += delta;

	//derivative calculation.
	int this_d = (incoming_val - pid->prev_val);
	pid->prev_val = incoming_val;

	return (delta * pid->p) + (pid->error * pid->i) + (this_d * pid->d);
}
