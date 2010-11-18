/*-----------------------------------------------------------------------
 /  Low level disk interface modlue include file  R0.05   (C)ChaN, 2007
 /-----------------------------------------------------------------------*/

#ifndef _DISKIO
#ifdef __cplusplus
extern "C" {
#endif

#define _READONLY	0	    /* 1: Read-only mode */
#define _USE_IOCTL	1

#include "./diskioStructs.h"
#include "./diskioDefines.h"	// Platform dependent calls should be in this file!

#include "../FreeRTOS/FreeRTOS.h"
#include "../FreeRTOS/semphr.h"	// SPI-Access Semaphore

/// This Semaphore is used to lock/unlock SPI Access.
/// Valid semaphore pointer must be passed otherwise system will crash!
void diskio_initializeSPIMutex(xSemaphoreHandle *sema);

void initialize_SdCardSignals(void);	///< Initializes the SD Card Signals using CARD_SIGNAL_INIT() and SET_CSPIN_OUT()
DSTATUS disk_initialize(BYTE drv);		///< Initializes SD Card by putting it into SPI Mode
DSTATUS disk_status(BYTE drv);			///< Returns status of card (if its been initialized or not)
BYTE getSDCardInfo(SDINFO* info);		///< Gets low-level (manufacturer info) from SD Card

DRESULT disk_read (BYTE drv,       BYTE *buff, DWORD sector, BYTE count);	///< Reads a sector from the SD Card
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count);	///< Writes a sector to the SD-Card
DRESULT disk_ioctl(BYTE drv, BYTE ctrl,void *buff);							///< Low level function used by FAT File System Layer
void disk_timerproc(void); 													///< Timeout function MUST BE CALLED AT 100Hz (every 10ms)









/* Definitions for MMC/SDC command */
#define CMD0			(0x40+0)		/* GO_IDLE_STATE */
#define CMD1			(0x40+1)		/* SEND_OP_COND (MMC) */
#define	ACMD41			(0xC0+41)		/* SEND_OP_COND (SDC) */
#define CMD8			(0x40+8)		/* SEND_IF_COND */
#define CMD9			(0x40+9)		/* SEND_CSD */
#define CMD10			(0x40+10)		/* SEND_CID */
#define CMD12			(0x40+12)		/* STOP_TRANSMISSION */
#define ACMD13			(0xC0+13)		/* SD_STATUS (SDC) */
#define CMD16			(0x40+16)		/* SET_BLOCKLEN */
#define CMD17			(0x40+17)		/* READ_SINGLE_BLOCK */
#define CMD18			(0x40+18)		/* READ_MULTIPLE_BLOCK */
#define CMD23			(0x40+23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23			(0xC0+23)		/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24			(0x40+24)		/* WRITE_BLOCK */
#define CMD25			(0x40+25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD55			(0x40+55)		/* APP_CMD */
#define CMD58			(0x40+58)		/* READ_OCR */

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* Command code for disk_ioctrl() */
/* Generic command */
#define CTRL_SYNC			0	/* Mandatory for write functions */
#define GET_SECTOR_COUNT	1	/* Mandatory for only f_mkfs() */
#define GET_SECTOR_SIZE		2
#define GET_BLOCK_SIZE		3	/* Mandatory for only f_mkfs() */
#define CTRL_POWER			4
#define CTRL_LOCK			5
#define CTRL_EJECT		  	6
/* MMC/SDC command */
#define MMC_GET_TYPE      	10
#define MMC_GET_CSD       	11
#define MMC_GET_CID       	12
#define MMC_GET_OCR			13
#define MMC_GET_SDSTAT    	14
/* ATA/CF command */
#define ATA_GET_REV		  	20
#define ATA_GET_MODEL     	21
#define ATA_GET_SN        	22

/* Card type flags (CardType) */
#define CT_MMC				0x01
#define CT_SD1				0x02
#define CT_SD2				0x04
#define CT_SDC				(CT_SD1|CT_SD2)
#define CT_BLOCK        	0x08

#ifdef __cplusplus
}
#endif

#define _DISKIO
#endif
