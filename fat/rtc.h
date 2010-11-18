#ifndef RTC_H
#define RTC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"
#include "../System/lpc214x.h"


typedef struct {
	WORD	year;	/* 1..4095 */
	BYTE	month;	/* 1..12 */
	BYTE	mday;	/* 1.. 31 */
	BYTE	wday;	/* 1..7 */
	BYTE	hour;	/* 0..23 */
	BYTE	min;	/* 0..59 */
	BYTE	sec;	/* 0..59 */
} RTC;

void initialize_RTC (void);					/* Initialize RTC */
void rtc_gettime (RTC*);					/* Get time */
void rtc_settime (const RTC*);				/* Set time */

DWORD get_fattime ();


#ifdef __cplusplus
}
#endif
#endif
