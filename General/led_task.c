/*
 * leds.c
 *
 *  Created on: Jan 30, 2011
 *      Author: timo
 */

#include "led_task.h"

#include "../osHandles.h"
#include "../System/lpc214x.h"


/* ---- private declarations ---- */

#define LED_PIN (1<<16) //P0.16

/* ---- public definitions ---- */
void led_task(void *pvParameters)
{
	OSHANDLES *osHandles = (OSHANDLES*)pvParameters;
	vTaskDelay(OS_MS(10));

	IODIR0 |= LED_PIN; //port 0.16 as output
	IOCLR0 = LED_PIN; //P0.16 off

	for (unsigned int i = 0; ;i++)
	{

		if (osHandles->flight_control.armed == 3 && (osHandles->flight_settings.led_mode & 1)){  //armed
			IOSET0 = LED_PIN;
			vTaskDelay(OS_MS(100));
		}
		else if (osHandles->flight_control.command_used_number < 75 && (osHandles->flight_settings.led_mode & 2)){  //transmitting, not armed
			if (!(i%6)) { IOSET0 = LED_PIN; }
			else { IOCLR0 = LED_PIN; }
		}
		else if (osHandles->flight_settings.led_mode & 4) { //standby
			if (!((i/500)%3)){ //every four seconds
				if (!(i%4)) { IOSET0 = LED_PIN; } //go on at 1/8th brightness
				else { IOCLR0 = LED_PIN; }
			}
			else { IOCLR0 = LED_PIN; }
		}
		else { IOCLR0 = LED_PIN; vTaskDelay(OS_MS(1000)); } //LEDs off.

		vTaskDelay(1);
		//unsigned int last_ms = xTaskGetTickCount();
	}
}
