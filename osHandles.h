#ifndef OSHANDLES_H
#define OSHANDLES_H

#include "./FreeRTOS/FreeRTOS.h"	// Includes for RTOS & Other services provided by it:
#include "./FreeRTOS/task.h"
#include "./FreeRTOS/queue.h"
#include "./FreeRTOS/semphr.h"
#include "./drivers/pid.h"

//#include <stdint.h>
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

//used for osHandles->flight_settings.flying_mode
#define X_MODE 0
#define PLUS_MODE 1

// A pointer to OSHANDLES is all that is needed for any file/task to access
// Semaphores, task handles, queue handles etc...
// Pointer can be passed to task as pvParameter
typedef struct
{
	struct {
		//xQueueHandle playback_playlist;
	}queue;

	struct {
		xTaskHandle userInterface;
		xTaskHandle flight_task;
		xTaskHandle led_task;
	}task;

	struct {
		xSemaphoreHandle SPI;  // spi lock
	}lock;

	struct {
		PID_DATA * pid_roll;
		PID_DATA * pid_pitch;
		PID_DATA * pid_yaw;
		uint8_t flying_mode; //X_MODE or PLUS_MODE
		uint8_t led_mode;
		uint16_t pitch_roll_tx_scale;
		uint16_t yaw_tx_scale;
	}flight_settings;

	struct {
		unsigned char armed;
		unsigned char telem_mode;
		int tx_throttle, tx_yaw, tx_pitch, tx_roll;
		unsigned int command_used_number;
		unsigned char please_update_sensors;
	}flight_control;


}OSHANDLES;

#endif /* COMMONDEFS_H_ */
