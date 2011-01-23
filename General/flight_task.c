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
#include <stdlib.h>                   // strtok()
#include "../drivers/motors.h"
#include "../drivers/pid.h"
#include "../drivers/wii_sensors.h"
#define MINIMUM(a,b)		(((a)>(b))? (b):(a))


void flight_task(void *pvParameters)
{
	OSHANDLES *osHandles = (OSHANDLES*)pvParameters;

	setup_motors();
	write_motors_zero();

	memset(&(osHandles->flight_settings), 0, sizeof(osHandles->flight_settings));
	osHandles->flight_settings.pid_roll.p = 7.0;
	osHandles->flight_settings.pid_pitch.p = 7.0;
	osHandles->flight_settings.pid_yaw.p = 12.0;
	osHandles->flight_settings.pid_yaw.i = 0.1;

	osHandles->flight_settings.tx_throttle = 1000;
	osHandles->flight_settings.tx_yaw = 1500;
	osHandles->flight_settings.tx_pitch = 1500;
	osHandles->flight_settings.tx_roll = 1500;
	osHandles->flight_settings.throttle_limit = 1600;

	SENSOR_DATA zero_vals = { 0 };
	for (;;){
		vTaskDelay(500);
		init_wii_sensors();
		unsigned int sensors_averages = zero_wii_sensors(&zero_vals);
		if (osHandles->flight_settings.telem_mode) {rprintf("read %i averages\n",sensors_averages);}
		if (sensors_averages < 1){ vTaskDelay(2000); }
		else break; //successfully initialized and zeroed.
	}

	//rprintf("yaw PID: %i %i %i\n",(int)osHandles->flight_settings.pid_yaw.p,(int)osHandles->flight_settings.pid_yaw.i,(int)osHandles->flight_settings.pid_yaw.d);

	int pitch_integral = 0;
	unsigned int loop_count = 0;
    unsigned int last_ms = xTaskGetTickCount();
	SENSOR_DATA sensor_vals;
	int pitch_offset = 0,roll_offset = 0,yaw_offset = 0;

	for(;;)  //this is a continuous task.
	{
		unsigned char data_type = update_wii_data(&sensor_vals, &zero_vals);
		if (data_type == 1){
			unsigned int this_ms = xTaskGetTickCount();
			pitch_integral += sensor_vals.pitch*(this_ms-last_ms); //deg/s * ms = degrees*1000
			last_ms = this_ms;

			pitch_offset = calculate_pid((sensor_vals.pitch)/(float)20+1500, osHandles->flight_settings.tx_pitch, &(osHandles->flight_settings.pid_pitch));
			roll_offset = calculate_pid((sensor_vals.roll)/(float)20+1500, osHandles->flight_settings.tx_roll, &(osHandles->flight_settings.pid_roll));
			yaw_offset = calculate_pid((sensor_vals.yaw)/(float)20+1500, osHandles->flight_settings.tx_yaw, &(osHandles->flight_settings.pid_yaw));
		}
		//osHandles->flight_settings.tx_throttle;
		if (osHandles->flight_settings.armed == 3){
			write_motors(
					osHandles->flight_settings.tx_throttle + pitch_offset - yaw_offset, //front
					osHandles->flight_settings.tx_throttle - pitch_offset - yaw_offset, //back
					osHandles->flight_settings.tx_throttle + roll_offset + yaw_offset,  //left
					osHandles->flight_settings.tx_throttle - roll_offset + yaw_offset   //right
					);

			//safety disarm if not updated command recently enough.
			if (osHandles->flight_settings.command_used_number++ > 50) { osHandles->flight_settings.armed = 0; write_motors_zero(); }
		}
		else {
			if (osHandles->flight_settings.please_update_sensors){ //allows the user interface task to zero the sensor values.
				osHandles->flight_settings.please_update_sensors = 0;
				zero_wii_sensors(&zero_vals);
				pulse_motors(3, 200);
				if (osHandles->flight_settings.telem_mode) {rprintf("zero-ed angles.\n");}
			}
		}


    	if (!(loop_count%100)&& osHandles->flight_settings.telem_mode) { rprintf("ofst[p,r,y]=[%i,%i,%i] <tx_p=%i> srs={%i-%i-%i}\n",  pitch_offset, roll_offset, yaw_offset, osHandles->flight_settings.tx_pitch ,sensor_vals.pitch,sensor_vals.roll,sensor_vals.yaw); }

		//write_PWM_servo(2, calculate_pid(pitch_integral/1000, 0, &(osHandles->flight_settings.pid_roll)) + 1500 );

		vTaskDelay(MAX_WII_SENSOR_POLLING_RATE);
		loop_count++;
	}
}
