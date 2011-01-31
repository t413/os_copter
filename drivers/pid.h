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
	signed short p, i, d;
	unsigned short i_limit; //total limit on the effect on I, set to 0 to disable.
	signed int error;
	signed short prev_val;
}PID_DATA;


//int16_t calculate_pid(int16_t incoming_val, int16_t goal_val, PID_DATA*pid);
signed short calculate_pid(signed short incoming_val, signed short goal_val, PID_DATA*pid);


#endif /* PID_H_ */
