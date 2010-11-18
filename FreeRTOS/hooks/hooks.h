#ifndef _FREERTOS_HOOKS
#define _FREERTOS_HOOKS

#include "../FreeRTOS.h"
#include "../task.h"

/// This function is called by the OS when it is IDLE (no tasks waiting for processing)
void vApplicationIdleHook(void);

/// This function is called by the OS when it detects stack overflow by a task
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );

/// This function is called by the OS upon every OS Tick (only if tick hook define is 1 at FreeRTOSconfig.h)
void vApplicationTickHook( void );

/// This function is called by the OS when memory allocation fails (only if it is defined at FreeRTOSconfig.h)
void vApplicationMallocFailedHook( void );

#endif
