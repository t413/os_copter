/*
 * watchdog.h
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "./lpc214x.h"
#include "./sysConfig.h"

void watchdog_Reset();
void watchdog_Feed();
void watchdog_Start();
void watchdog_Stop();

void watchdogDelayUs(unsigned int delayUs);


#ifdef __cplusplus
}
#endif
#endif /* WATCHDOG_H_ */
