/*
    FreeRTOS V6.0.2 - Copyright (C) 2010 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS eBook                                  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "../System/sysConfig.h"
#include "../System/lpc214x.h"
/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				1
#define configUSE_TICK_HOOK				0
#define configUSE_MALLOC_FAILED_HOOK	1
#define configCPU_CLOCK_HZ				( PCLK )	/* Should be the clock of the timer not CPU */
#define configTICK_RATE_HZ				( 1000 )	/* OS Interrupt rate in Hertz */
#define configMAX_PRIORITIES			( 5 )
/* Assuming there are 4 priorities set above */
#define PRIORITY_IDLE		0
#define PRIORITY_LOW		1
#define PRIORITY_MEDIUM		2
#define PRIORITY_HIGH		3
#if (configMAX_PRIORITIES > 4)
#define PRIORITY_CRITICAL 	4
#endif

#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 128 )	 /* Do not change this */
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 24 * 1024 ) ) /* Only needed when using heap_1.c and heap_2.c */
#define STACK_BYTES(x)					((x)/4)	/* freeRTOS allocates 4-times the size given due to 32-bit ARM */
#define MS_PER_TICK()					( 1000 / configTICK_RATE_HZ)
#define OS_MS(x)						( x / MS_PER_TICK() )

#define configMAX_TASK_NAME_LEN				16	/* Maximum characters to describe a task */
#define configUSE_TRACE_FACILITY			0	/* This is for offline traces, usually run-time status is better option */
#define configGENERATE_RUN_TIME_STATS		1	/* Use for debugging: Uses some overhead along and TIMER1 */
#define configCHECK_FOR_STACK_OVERFLOW  	2	/* Use for debugging initially until you are sure you have set correct stack sizes, then set to 0 */
#define INCLUDE_uxTaskGetStackHighWaterMark 1	/* You can uxTaskGetStackHighWaterMark() to get tasks remaining stack to make sure it won't overflow */

#define configUSE_16_BIT_TICKS				0	/* LEAVE TO 0 since ARM7 is 32-bit */
#define configIDLE_SHOULD_YIELD				0	/* If tasks share idle priority, idle task will yield to them */
#define configQUEUE_REGISTRY_SIZE			0	/* This is for debugging only */
#define configUSE_MUTEXES					1 	/* If you use mutexes, set to 1 */

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 				0
#define configMAX_CO_ROUTINE_PRIORITIES 	( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				0
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1

#if (configGENERATE_RUN_TIME_STATS != 0)
	#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()	\
									 TIMER1_PR = 32;	\
									 TIMER1_PC = 0;		\
									TIMER1_MR0 = 0;		\
									TIMER1_MCR = 0;		\
									TIMER1_TCR = 1;

	#define portGET_RUN_TIME_COUNTER_VALUE()		TIMER1_TC
	#define portRESET_RUN_TIME_COUNTER()			TIMER1_PC = 0; TIMER1_TC = 0

#endif

#endif /* FREERTOS_CONFIG_H */
