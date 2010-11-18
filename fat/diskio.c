/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module  (C)ChaN, 2007             */
/*-----------------------------------------------------------------------*/
/* are platform dependent.                                               */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#define DEBUG_SD_CARD		0	// Set to 1 to printf debug data.
#define OPTIMIZE_SSP_SPI	0	// Set to 1 and fill in Optimized code yourself!

volatile DSTATUS Stat = STA_NOINIT; /* Disk status */
volatile BYTE Timer1, Timer2; 		/* 100Hz decrement timers */
BYTE CardType;						/* Card type flags */

xSemaphoreHandle *SPIAccessSemaphore;
void diskio_initializeSPIMutex(xSemaphoreHandle *sema)
{
	SPIAccessSemaphore = sema;
}

char get_spi(void)
{
	const unsigned int spiAccessTimeout = 3000;
	if(xSemaphoreTake( *SPIAccessSemaphore, spiAccessTimeout)){
		SELECT();
		return 1;
	}

	return 0;
}
void release_spi(void)
{
	DESELECT();
	rcvr_spi();
	xSemaphoreGive(*SPIAccessSemaphore);
}

BYTE wait_ready(void)
{
	BYTE res;

	Timer2 = 50; /* Wait for ready in timeout of 500ms */
	rcvr_spi();
	do {
		res = rcvr_spi();
	}while ((res != 0xFF) && Timer2);

	return res;
}

void initialize_SdCardSignals(void)
{
	SET_CSPIN_OUT(); // Set the CS pin of SD card as output and deselect
	CARD_SIGNAL_INIT(); // Set the write-protect and card-detect signals as input
}

void power_on(void)
{
	// Power on the SD-Card Socket if hardware allows
}

void power_off(void)
{
	if (!get_spi())
		return;

	wait_ready();
	release_spi();

	// Power off the SD-Card Socket if hardware allows
	Stat |= STA_NOINIT; // Set STA_NOINIT
}

int chk_power(void)
{
	return (!(SOCKPORT & SOCKINS)); // LOW means inserted
}

BOOL rcvr_datablock(BYTE *buff, /* Data buffer to store received data */
UINT btr /* Byte count (must be multiple of 4) */
)
{
	BYTE token;

	Timer1 = 10;
	do
	{ /* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);

	if (token != 0xFE)
		return FALSE; /* If not valid data token, return with error */

#if OPTIMIZE_SSP_SPI

#else
	do {
		*buff++ = rcvr_spi();
		*buff++ = rcvr_spi();
		*buff++ = rcvr_spi();
		*buff++ = rcvr_spi();
	} while (btr -= 4);
	rcvr_spi(); /* Discard CRC */
	rcvr_spi();
#endif


	return TRUE; /* Return with success */
}

#if _READONLY == 0
BOOL xmit_datablock(const BYTE *buff, /* 512 byte data block to be transmitted */
BYTE token /* Data/Stop token */
)
{
	BYTE resp;

	if (wait_ready() != 0xFF)
		return FALSE;

	xmit_spi(token) ; /* Xmit data token */
	if (token != 0xFD)
	{ /* Is data token */
#if OPTIMIZE_SSP_SPI

#else
		unsigned char wc = 0;
		do
		{ /* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
		xmit_spi(0xFF) ; /* CRC (Dummy) */
		xmit_spi(0xFF) ;
		resp = rcvr_spi(); /* Reveive data response */
		if ((resp & 0x1F) != 0x05) /* If not accepted, return with error */
			return FALSE;
#endif
	}

	return TRUE;
}
#endif /* _READONLY */

BYTE send_cmd(BYTE cmd, /* Command byte */
DWORD arg /* Argument */
)
{
	BYTE n, res;

	if (cmd & 0x80)
	{ /* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1)
			return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();				// Wait for card ready

	if (wait_ready() != 0xFF)
	{
		#if(DEBUG_SD_CARD)
		rprintf("send_cmd: Error, wait_ready() did not return 0xFF\n");
		#endif
		return 0xFF;
	}

	/* Send command packet */
#if OPTIMIZE_SSP_SPI

#else
	xmit_spi(cmd) ; 				/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24)) ;	/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16)) ;	/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8))  ;	/* Argument[15..8] */
	xmit_spi((BYTE)arg) ; 			/* Argument[7..0] */
	n = 0x01; /* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95; /* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87; /* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi(); /* Skip a stuff byte when stop reading */
	n = 10; /* Wait for a valid response in timeout of 10 attempts */
#endif

	do
	{
		res = rcvr_spi();
	} while ((res & 0x80) && --n);

#if(DEBUG_SD_CARD)
	if (n == 0) rprintf("send_cmd: Timeout during card read\n");
#endif

	return res; /* Return with the response value */
}

DSTATUS disk_initialize(BYTE drv /* Physical drive nmuber (0) */
)
{
	BYTE n, cmd, ty, ocr[4];

	if (drv) {
		return STA_NOINIT; /* Supports only single drive */
	}
	if (Stat & STA_NODISK) {
		return Stat; /* No card in the socket */
	}

	power_on(); /* Force socket power on */
	FCLK_SLOW();
	for (n = 10; n; n--)
		rcvr_spi(); /* 80 dummy clocks */

	if (!get_spi())
		return RES_ERROR;
	ty = 0;
	if (send_cmd(CMD0, 0) == 1)
	{ /* Enter Idle state */
		Timer1 = 100; /* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1)
		{ /* SDHC */
			#if(DEBUG_SD_CARD)
			rprintf("disk_initialize: CMD8 succeeded...\n");
			#endif

			for (n = 0; n < 4; n++)
				ocr[n] = rcvr_spi(); /* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
			{ /* The card can work at vdd range of 2.7-3.6V */
				#if(DEBUG_SD_CARD)
				rprintf("disk_initialize: SD-HC Card detected!\n");
				#endif

				while (Timer1 && send_cmd(ACMD41, 1UL << 30))
					; /* Wait for leaving idle state (ACMD41 with HCS bit) */

				if (Timer1 && send_cmd(CMD58, 0) == 0)
				{ /* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++)
						ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
				else {
					#if(DEBUG_SD_CARD)
					rprintf("disk_initialize: CMD58 FAILED!\n");
					#endif
				}
			}
		}
		else
		{ /* SDSC or MMC */
			#if(DEBUG_SD_CARD)
			rprintf("disk_initialize: Card is not SD-HC!\n");
			#endif
			if (send_cmd(ACMD41, 0) <= 1) {
				ty = CT_SD1;
				cmd = ACMD41; /* SDSC */
			}
			else {
				ty = CT_MMC;
				cmd = CMD1; /* MMC */
			}
			while (Timer1 && send_cmd(cmd, 0)) ; /* Wait for leaving idle state */

			if (!Timer1 || send_cmd(CMD16, 512) != 0) /* Set R/W block length to 512 */
			{
				#if(DEBUG_SD_CARD)
				rprintf("disk_initialize: Could not set block length to 512\n");
				ty = 0;
				#endif
			}
		}
	}
	else
	{
		#if(DEBUG_SD_CARD)
		rprintf("disk_initialize: CMD0 did not respond...\n");
		#endif
	}
	CardType = ty;
	release_spi();

	if (ty) { /* Initialization succeded */
		Stat &= ~STA_NOINIT; /* Clear STA_NOINIT */
		FCLK_FAST();
	}
	else { /* Initialization failed */
		power_off();
	}

	return Stat;
}

DSTATUS disk_status(BYTE drv /* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT; /* Supports only single drive */
	return Stat;
}

DRESULT disk_read(BYTE drv, /* Physical drive nmuber (0) */
BYTE *buff, /* Pointer to the data buffer to store read data */
DWORD sector, /* Start sector number (LBA) */
BYTE count /* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (!get_spi()) return RES_ERROR;

	if (!(CardType & CT_BLOCK)) sector *= 512; /* Convert to byte address if needed */

	if (count == 1)
	{ /* Single block read */
		if ((send_cmd(CMD17, sector) == 0) /* READ_SINGLE_BLOCK */
		&& rcvr_datablock(buff, 512)) count = 0;
	}
	else
	{ /* Multiple block read */
		if (send_cmd(CMD18, sector) == 0)
		{ /* READ_MULTIPLE_BLOCK */
			do
			{
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write(BYTE drv, /* Physical drive nmuber (0) */
const BYTE *buff, /* Pointer to the data to be written */
DWORD sector, /* Start sector number (LBA) */
BYTE count /* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;
	if (!get_spi()) return RES_ERROR;

	if (!(CardType & CT_BLOCK))
		sector *= 512; /* Convert to byte address if needed */

	if (count == 1)
	{ /* Single block write */
		if ((send_cmd(CMD24, sector) == 0) && xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else{
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0)
		{ /* WRITE_MULTIPLE_BLOCK */
			do
			{
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD)) /* STOP_TRAN token */
			count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */

#if _USE_IOCTL != 0
DRESULT disk_ioctl(BYTE drv, /* Physical drive nmuber (0) */
BYTE ctrl, /* Control code */
void *buff /* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = (BYTE*)buff;
	WORD csize;

	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER)
	{
		switch (*ptr)
		{
			case 0: /* Sub control code == 0 (POWER_OFF) */
				if (chk_power()) power_off(); /* Power off */
				res = RES_OK;
				break;
			case 1: /* Sub control code == 1 (POWER_ON) */
				power_on(); /* Power on */
				res = RES_OK;
				break;
			case 2: /* Sub control code == 2 (POWER_GET) */
				*(ptr + 1) = (BYTE) chk_power();
				res = RES_OK;
				break;
			default:
				res = RES_PARERR;
		}
	}
	else
	{
		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl)
		{
			case CTRL_SYNC: /* Make sure that no pending write process */
				//SELECT();			// Wait for card ready
				if (!get_spi()) return RES_ERROR;
				if (wait_ready() == 0xFF) res = RES_OK;
				break;

			case GET_SECTOR_COUNT: /* Get number of sectors on the disk (DWORD) */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
				{
					if ((csd[0] >> 6) == 1)
					{ /* SDC ver 2.00 */
						csize = csd[9] + ((WORD) csd[8] << 8) + 1;
						*(DWORD*) buff = (DWORD) csize << 10;
					}
					else
					{ /* SDC ver 1.XX or MMC*/
						n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
						csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10)
								+ 1;
						*(DWORD*) buff = (DWORD) csize << (n - 9);
					}
					res = RES_OK;
				}
				break;

			case GET_SECTOR_SIZE: /* Get R/W sector size (WORD) */
				*(WORD*) buff = 512;
				res = RES_OK;
				break;

			case GET_BLOCK_SIZE: /* Get erase block size in unit of sector (DWORD) */
				if (CardType & CT_SD2)
				{ /* SDC ver 2.00 */
					if (send_cmd(ACMD13, 0) == 0)
					{ /* Read SD status */
						rcvr_spi();
						if (rcvr_datablock(csd, 16))
						{ /* Read partial block */
							for (n = 64 - 16; n; n--)
								rcvr_spi(); /* Purge trailing data */
							*(DWORD*) buff = 16UL << (csd[10] >> 4);
							res = RES_OK;
						}
					}
				}
				else
				{ /* SDC ver 1.XX or MMC */
					if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
					{ /* Read CSD */
						if (CardType & CT_SD1)
						{ /* SDC ver 1.XX */
							*(DWORD*) buff = (((csd[10] & 63) << 1) + ((WORD) (csd[11] & 128) >> 7)
									+ 1) << ((csd[13] >> 6) - 1);
						}
						else
						{ /* MMC */
							*(DWORD*) buff = ((WORD) ((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3)
									<< 3) + ((csd[11] & 224) >> 5) + 1);
						}
						res = RES_OK;
					}
				}
				break;

			case MMC_GET_TYPE: /* Get card type flags (1 byte) */
				*ptr = CardType;
				res = RES_OK;
				break;

			case MMC_GET_CSD: /* Receive CSD as a data block (16 bytes) */
				if (send_cmd(CMD9, 0) == 0 /* READ_CSD */
				&& rcvr_datablock(ptr, 16)) res = RES_OK;
				break;

			case MMC_GET_CID: /* Receive CID as a data block (16 bytes) */
				if (send_cmd(CMD10, 0) == 0 /* READ_CID */
				&& rcvr_datablock(ptr, 16)) res = RES_OK;
				break;

			case MMC_GET_OCR: /* Receive OCR as an R3 resp (4 bytes) */
				if (send_cmd(CMD58, 0) == 0)
				{ /* READ_OCR */
					for (n = 4; n; n--)
						*ptr++ = rcvr_spi();
					res = RES_OK;
				}
				break;

			case MMC_GET_SDSTAT: /* Receive SD statsu as a data block (64 bytes) */
				if (send_cmd(ACMD13, 0) == 0)
				{ /* SD_STATUS */
					rcvr_spi();
					if (rcvr_datablock(ptr, 64)) res = RES_OK;
				}
				break;

			default:
				res = RES_PARERR;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */

void disk_timerproc(void)
{
	volatile static BYTE pv;
	BYTE n, s;

	n = Timer1; /* 100Hz decrement timer */
	if (n)
		Timer1 = --n;
	n = Timer2;
	if (n)
		Timer2 = --n;

	n = pv;
	pv = SOCKPORT & (SOCKWP | SOCKINS); /* Sample socket switch */

	if (n == pv) { /* Have contacts stabled? */
		s = Stat;

		if (pv & SOCKWP) 		/* WP is H (write protected) */
			s |= STA_PROTECT;
		else					/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (pv & SOCKINS) 		/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else					/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}

BYTE getSDCardInfo(SDINFO* info)
{
	BYTE i;

	if (!get_spi())
		return 0;

	/* read cid register */
	if (send_cmd(CMD10, 0)) {
		release_spi();
		return 0;
	}
	while (rcvr_spi() != 0xfe);

	for (i = 0; i < 18; ++i){
		BYTE b = rcvr_spi();

		switch (i)
		{
			case 0:
				info->manufacturer = b;
				break;
			case 1:
			case 2:
				info->oem[i - 1] = b;
				break;
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				info->product[i - 3] = b;
				break;
			case 8:
				info->revision = b;
				break;
			case 9:
			case 10:
			case 11:
			case 12:
				info->serial |= (DWORD) b << ((12 - i) * 8);
				break;
			case 13:
				info->manufacturing_year = b << 4;
				break;
			case 14:
				info->manufacturing_year |= b >> 4;
				info->manufacturing_month = b & 0x0f;
				break;
		}
	}
	release_spi();

	return 1;
}
