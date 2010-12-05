/*
 * wii_sensors.c
 *
 *  Created on: Dec 5, 2010
 *      Author: timo
 */

#include "../osHandles.h"
#include "../System/lpc214x.h"
#include "../drivers/i2c.h"
#include "./wii_sensors.h"


unsigned char init_wii_sensors(){
	unsigned char dummy[1] = "";

	// cold-initialize wii motion+ with nunchuck attatched
	if ( i2c_send_byte(0xA6, 0xF0, 0x55) && i2c_send_byte(0xA6, 0xFE, 0x05) )
	{
		return 1;
	}
	//already initialized motion plus is ready to go.
	else if (i2c_send(0xA4,0x00,dummy,0))
	{
		return 2;
	}

	else return 0; //error
}


unsigned char zero_wii_sensors(SENSOR_DATA *zero_data){
	unsigned char successful_reads = 0;
	SENSOR_DATA vals;
	SENSOR_DATA dummy_zero = { 0 };
	//memset(zero_data, 0, sizeof(SENSOR_DATA)); //zero the data.

	for (int i=0;i<15;i++)
	{
		if (update_wii_data(&vals, &dummy_zero) == 1)
		{
			zero_data->yaw   += vals.yaw;
			zero_data->pitch += vals.pitch;
			zero_data->roll  += vals.roll;
			successful_reads++;
		}
		vTaskDelay(10);
	}
	zero_data->yaw   = zero_data->yaw   / successful_reads;
	zero_data->pitch = zero_data->pitch / successful_reads;
	zero_data->roll  = zero_data->roll  / successful_reads;
	return successful_reads;
}

unsigned char update_wii_data(SENSOR_DATA *vals, SENSOR_DATA *zer0){
	unsigned char data[6] = "";
	if (i2c_receive(0xA4,0x00,data,6))
	{
		if ( data[5]&0x02 ) //wm+ data
		{
			vals->yaw = (((data[3]>>2)<<8)+data[0]);
			vals->pitch = (((data[4]>>2)<<8)+data[1]);
			vals->roll = (((data[5]>>2)<<8)+data[2]);
			//use the slow/fast mode data
			vals->yaw   = ((data[3]&0x02)>>1  ? vals->yaw/5   : vals->yaw) - zer0->yaw;
			vals->pitch = ((data[4]&0x02)>>1  ? vals->pitch/5 : vals->pitch) - zer0->pitch;
			vals->roll  = ((data[3]&0x01)     ? vals->roll/5  : vals->roll) - zer0->roll;
			return 1; //got wm+ data successfully
		}
		else //this is nunchuck data
		{
			vals->x = ( (data[2]<<2)        + ((data[5]>>3)&0x2) ) - zer0->x;
			vals->y = ( (data[3]<<2)        + ((data[5]>>4)&0x2) ) - zer0->y;
			vals->z = ( ((data[4]&0xFE)<<2) + ((data[5]>>5)&0x6) ) - zer0->z;
			return 2; //got nunchuck data successfully
		}
	}
	return 0; //data not received successfully.
}
