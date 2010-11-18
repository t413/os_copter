#include "hooks.h"
#include "../../drivers/uart/uart0.h"
#include "../../System/rprintf.h"

void vApplicationIdleHook(void)
{
	// THIS FUNCTION MUST NOT BLOCK
	// Put CPU to IDLE here. RTOS will wake up CPU from OS timer interrupt.
	PCON |= 1; // Puts the CPU in IDLE mode.
}

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	rprintf_devopen(uart0PutCharPolling);
	rprintf("HALTING SYSTEM: Stack overflow by task: %s\n", pcTaskName);
	rprintf("Try increasing stack memory of this task.\n");
	while(1);
}

#if configUSE_TICK_HOOK
void vApplicationTickHook( void )
{
	// This function is called at every OS Tick
	// DO NOT PUT A LOT OF CODE HERE.  KEEP IT SHORT AND SIMPLE (if you really need it)
}
#endif

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
void vApplicationMallocFailedHook( void )
{
	rprintf_devopen(uart0PutCharPolling);
	rprintf("HALTING SYSTEM: Your system ran out of memory (RAM)!\n");
	while(1);
}
#endif

