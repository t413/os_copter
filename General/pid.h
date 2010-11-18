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
        int p, i, d;
        int error;
        int prev_val;
}PID_DATA;


int calculate_pid(int incoming_val, int goal_val, PID_DATA*pid);


#endif /* PID_H_ */
