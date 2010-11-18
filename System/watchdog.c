#include "./watchdog.h"

void watchdog_Reset()
{
	watchdog_Feed();
}
void watchdog_Feed()
{
	WDFEED = 0xAA;
	WDFEED = 0x55;
}
void watchdog_Start()
{
	WDTC = 0xFFFFFFFF;
	WDMOD = 1;
	watchdog_Feed();
}
void watchdog_Stop()
{
	WDMOD = 0;
	watchdog_Feed();
}

void watchdogDelayUs(unsigned int delayUs)
{
	// Get total count needed for this delay based on Peripheral Clock / 4
	// watchdog prescalar is 4 and add 1 since these calculations also take some time.
	unsigned int terminateCount = (delayUs * (PERI_FREQ/1000000/4));

	// Watchdog counts down so subtract from top value
	terminateCount = 0xFFFFFFFF - terminateCount;

	watchdog_Start();
	while(WDTV > terminateCount);
	watchdog_Stop();
}
