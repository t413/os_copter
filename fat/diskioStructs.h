#ifndef DISKIOSTRUCTS_H_
#define DISKIOSTRUCTS_H_

#include "./integer.h"
typedef unsigned char DSTATUS;					///< Status of Disk Functions

typedef enum							///< Results of Disk Functions
{
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

typedef struct SDINFO
{
	BYTE manufacturer; // A manufacturer code globally assigned by the SD card organization.
	BYTE oem[3]; // A string describing the card's OEM or content, globally assigned by the SD card organization.
	BYTE product[6]; // SD-Card product name
	BYTE revision; // The card's revision, coded in packed BCD. For example, the revision value \c 0x32 means "3.2".
	DWORD serial; // A serial number assigned by the manufacturer.
	BYTE manufacturing_year; // The year of manufacturing.  A value of zero means year 2000.
	BYTE manufacturing_month; // The month of manufacturing.
	QWORD capacity; // The card's total capacity in bytes.
	BYTE flag_copy; // Defines whether the card's content is original(0) or copied(1)
	BYTE flag_write_protect; // Defines whether the card's content is write-protected.
	BYTE flag_write_protect_temp; // Defines whether the card's content is temporarily write-protected.
	// These are internal flags and do not represent the state of the card's mechanical write-protect switch.
	BYTE format; //  The card's data layout. See the \c SD_RAW_FORMAT_* constants for details.
// note This value is not guaranteed to match reality.
} SDINFO;




#endif /* DISKIOSTRUCTS_H_ */
