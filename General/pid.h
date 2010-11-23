/*
 * pid.h
 *
 *  Created on: Nov 18, 2010
 *      Author: timo
 */

#ifndef PID_H_
#define PID_H_


// this is the struct to save the pid calculation data in.
typedef struct {
	float p, i, d;
	float error;
	float prev_val;
}PID_DATA;


float calculate_pid(float incoming_val, float goal_val, PID_DATA*pid);


#endif /* PID_H_ */
