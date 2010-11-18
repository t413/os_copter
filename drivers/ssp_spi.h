

#ifndef SSP_SPI_H_
#define SSP_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../System/lpc214x.h"

/// Initializes the Pins for SSP Operation in SPI Mode
void initialize_SSPSPI();

/// Exchanges a byte over SPI
/// @param data the byte to send
/// @return the byte captured over SPI
char rxTxByteSSPSPI(char data);

/// Sets the SPI Clock Divider (must be at least 2)
void setSSPSPI_Divider(unsigned int divider);

extern unsigned char sspSpiDummyByte;
#define txByteSSPSPI_Fast(dat)	SSPDR = dat; while((SSPSR & 0x10)); sspSpiDummyByte = SSPDR;
#define rxByteSSPSPI_Fast(dst)	SSPDR = 0xFF; while((SSPSR & 0x10)); dst = SSPDR

// SSPSR.1 is 0 if Tx FIFO is not full
#define SPI_BUSY()				(SSPSR & (1<<4)))
#define txByteSSPSPI_Fifo(dat)	while((SSPSR & 0x02));	SSPDR = dat
#define flushRxFifoSSPSPI()		sspSpiDummyByte = SSPDR;	sspSpiDummyByte = SSPDR;	\
								sspSpiDummyByte = SSPDR;	sspSpiDummyByte = SSPDR;	\
								sspSpiDummyByte = SSPDR;	sspSpiDummyByte = SSPDR;	\
								sspSpiDummyByte = SSPDR;	sspSpiDummyByte = SSPDR

#ifdef __cplusplus
}
#endif
#endif /* SSP_SPI_H_ */
