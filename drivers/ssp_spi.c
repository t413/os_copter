
#include "ssp_spi.h"

unsigned char sspSpiDummyByte;

void initialize_SSPSPI()
{
	PINSEL1 &= ~((3<<2) | (3<<4) | (3<<6) | (3<<8));	// Clear SCK, MISO, MOSI, and SSEL configuration
	PINSEL1 |=  ((2<<2) | (2<<4) | (2<<6) | (2<<8));	// Use SCK, MISO, MOSI, and SSEL option

	// Warning: Do not need to SSEL1 HIGH for LPC2148, other CPUs may need it though
	// IODIR0 |= (1<<20);
	// IOSET0 =  (1<<20);

	SSPCPSR = 32;		// Large divider, someone should lower the divider later.
	SSPCR0 = (0x07 | (0<<6) | (0<<7) | 0UL<<8);	// 8-bit format, SPI MODE0
	SSPCR1 = (0x02);	// Enable SSP
}

// char rxTxByteSSPSPI(char data) __attribute__ ((always_inline));
char rxTxByteSSPSPI(char data)
{
	SSPDR = data;
	//while(!(SSPSR & 0x01));	// BIT1 is 1 if FIFO is empty
	while((SSPSR & 0x10));		// Wait for BSY bit to be 0
	return SSPDR;
}

void setSSPSPI_Divider(unsigned int divider)
{
	// Divider must be an even number
	SSPCPSR = divider & 0xFFFFFFFE;
}
