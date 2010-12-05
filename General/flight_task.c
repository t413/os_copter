/*
 * flight_task.c
 *
 *  Created on: Dec 5, 2010
 *      Author: timo
 */
#include "flight_task.h"
#include "../osHandles.h"
#include "../drivers/uart/uart0.h"  // uart0GetChar()
#include "../System/rprintf.h"      // rprintf
#include "../System/lpc214x.h"
#include <math.h>                   // strtok()
#include "../drivers/pwm.h"
#include "../drivers/pid.h"
#include "../drivers/wii_sensors.h"


void flight_task(void *pvParameters)
{
	OSHANDLES *osHandles = (OSHANDLES*)pvParameters;

	setup_PWM_servo();
	setupPWM(2); //set up P0.7 as PWM2  //setupPWM(5);setupPWM(4);setupPWM(6);
	write_PWM_servo(2, 1500 );

	init_wii_sensors();
	SENSOR_DATA zero_vals = { 0 };
	zero_wii_sensors(&zero_vals);

	memset(&(osHandles->flight_settings), 0, sizeof(osHandles->flight_settings));
	osHandles->flight_settings.pid_roll.p = 5.5;
	osHandles->flight_settings.pid_pitch.p = 5.5;
	osHandles->flight_settings.pid_yaw.p = 8.5;

	int pitch_integral = 0;
	unsigned int loop_count = 0;
    unsigned int last_ms = xTaskGetTickCount();
	SENSOR_DATA sensor_vals;

	for(;;)  //this is a continuous task.
	{
		unsigned char data_type = update_wii_data(&sensor_vals, &zero_vals);

		if (data_type == 1){
			unsigned int this_ms = xTaskGetTickCount();
			pitch_integral += sensor_vals.pitch*(this_ms-last_ms); //deg/s * ms = degrees*1000
			last_ms = this_ms;
		}
#if 0
		if (!(loop_count%50))
			rprintf("decoded : %i %i %i [%i], %i %i %i\n",
					sensor_vals.yaw*595/8192,
					sensor_vals.roll*595/8192,
					sensor_vals.pitch*595/8192,
					pitch_integral*595/8192/1000,
					sensor_vals.x,sensor_vals.y,sensor_vals.z
					//(int)(atan2(z,y)/ M_PI * 180.0)
					);
#endif
		//(float)(atan2(z,y)/ M_PI * 180.0) (float)(atan2(z,x)/ M_	PI * 180.0)
		// (data[4]&1)?"connected":"")
		//else rprintf("wm+ error\n");

		//if ((loop_count*3/1000)%2)
		//	write_PWM(2, ((loop_count*3)%1000)+1000 );
		//else
		//	write_PWM(2, (2000 - (loop_count*3)%1000) );
		//write_PWM_servo(2, pitch_integral/1000+1500 );
		write_PWM_servo(2, calculate_pid(pitch_integral/1000, 0, &(osHandles->flight_settings.pid_roll)) + 1500 );

		vTaskDelay(3);
		loop_count++;
	}
}
