/*
 * wii_sensors.h
 *
 *  Created on: Dec 5, 2010
 *      Author: timo
 */

#ifndef WII_SENSORS_H_
#define WII_SENSORS_H_

#define MAX_WII_SENSOR_POLLING_RATE (OS_MS(5)) //5ms

typedef struct {
	signed short roll, pitch, yaw;
	signed short x, y, z;
}SENSOR_DATA;

unsigned char init_wii_sensors();
unsigned char zero_wii_sensors(SENSOR_DATA *zero_data);
unsigned char update_wii_data(SENSOR_DATA *vals, SENSOR_DATA *zer0);


#endif /* WII_SENSORS_H_ */
