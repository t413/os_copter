#ifndef OSHANDLES_H
#define OSHANDLES_H

#include "./FreeRTOS/FreeRTOS.h"	// Includes for RTOS & Other services provided by it:
#include "./FreeRTOS/task.h"
#include "./FreeRTOS/queue.h"
#include "./FreeRTOS/semphr.h"
#include "./drivers/pid.h"

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
	}task;

	struct {
		xSemaphoreHandle SPI;  // spi lock
	}lock;

	struct {
		PID_DATA pid_roll;
		PID_DATA pid_pitch;
		PID_DATA pid_yaw;
		//unsigned char flying_mode;
		unsigned char armed;
		unsigned char telem_mode;
		unsigned int throttle_limit;
		int tx_throttle, tx_yaw, tx_pitch, tx_roll;
		unsigned int command_used_number;
		unsigned char please_update_sensors;
	}flight_settings;


}OSHANDLES;

#endif /* COMMONDEFS_H_ */
