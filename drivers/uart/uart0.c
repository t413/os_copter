//
//  $Id: uart0.c 234 2008-10-28 23:56:14Z jcw $
//  $Revision: 234 $
//  $Author: jcw $
//  $Date: 2008-10-28 19:56:14 -0400 (Tue, 28 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0.c $
//

#include <stdlib.h>
#include "uart0.h"
#include "../../System/lpc214x.h"

//
//  Constants to determine the ISR source
#define serSOURCE_THRE				((unsigned portCHAR) 0x02)
#define serSOURCE_RX_TIMEOUT		((unsigned portCHAR) 0x0c)
#define serSOURCE_ERROR				((unsigned portCHAR) 0x06)
#define serSOURCE_RX				((unsigned portCHAR) 0x04)
#define serINTERRUPT_SOURCE_MASK	((unsigned portCHAR) 0x0f)

//  Queues used to hold received characters, and characters waiting to be transmitted
static xQueueHandle xRX0Queue;
static xQueueHandle xTX0Queue;
static volatile portLONG lTHREEmpty0;

static void uart0ISR_Handler(void)
{
	signed char cChar;
	portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

	long statusReg = UART0_IIR;
	switch (statusReg & serINTERRUPT_SOURCE_MASK)
	{
		//  Not handling this, but clear the interrupt
		case serSOURCE_ERROR:
		{
			cChar = UART0_LSR;
		}
			break;

			//  The THRE is empty.  If there is another character in the Tx queue, send it now,
			//  otherwise, no more characters, so indicate THRE is available
		case serSOURCE_THRE:
		{

			// Depending on if FIFO is enabled, we can send up to 16 chars because
			// THRE triggers when FIFO is empty.
			signed char bytesToSend = (statusReg & 0xC0) ? 16 : 1;
			while (bytesToSend-- && xQueueReceiveFromISR(xTX0Queue, &cChar,
					&higherPriorityTaskWoken))
			{
				U0THR = cChar;
			};
			// If we did not fill all the buffer, we have capacity to write to U0THR directly.
			if (bytesToSend > 0)
			{
				lTHREEmpty0 = 1;
			}
		}
			break;

			//  A character was received.  Place it in the queue of received characters
		case serSOURCE_RX_TIMEOUT:
		case serSOURCE_RX:
		{
			while ((UART0_LSR & 0x01))
			{ // read as long as there is unread data
				cChar = UART0_RBR;
				xQueueSendFromISR (xRX0Queue, &cChar, &higherPriorityTaskWoken);
			}
		}
			break;

		default:
			break;
	}

	VICVectAddr = (unsigned portLONG) 0;

	if (higherPriorityTaskWoken) portYIELD_FROM_ISR ();
}

void uart0ISR(void) __attribute__ ((naked));
void uart0ISR(void)
{
	portSAVE_CONTEXT ();
	runTimeStatISREntry();
	uart0ISR_Handler();
	runTimeStatISRExit();
	portRESTORE_CONTEXT ();
}

char uart0Init(unsigned long ulWantedBaud, unsigned long uxQueueLength)
{
	unsigned long ulDivisor;
	unsigned long ulWantedClock;

	if (0 == ulWantedBaud) ulWantedBaud = 38400;
	if (0 == uxQueueLength) uxQueueLength = 64;

	//  Create the queues used to hold Rx and Tx characters
	xRX0Queue = xQueueCreate(uxQueueLength, (unsigned portBASE_TYPE) sizeof(signed portCHAR));
	xTX0Queue = xQueueCreate(uxQueueLength + 1, (unsigned portBASE_TYPE) sizeof(signed portCHAR));

	lTHREEmpty0 = 1;

	if ((xRX0Queue == 0) || (xTX0Queue == 0)) return 0;

	portENTER_CRITICAL ();
	{
		//PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P00_MASK | PCB_PINSEL0_P01_MASK)) | (PCB_PINSEL0_P00_TXD0 | PCB_PINSEL0_P01_RXD0);
		PINSEL0 = (PINSEL0 & 0xfffffff0) | 0x00000005;

		// SCB_PCONP |= SCB_PCONP_PCUART0;
		PCONP |= 8; // Enable power to UART0

		ulWantedClock = ulWantedBaud * 16;
		ulDivisor = configCPU_CLOCK_HZ / ulWantedClock;

		// Set DLAB bit to access divisors
		UART0_LCR |= UART_LCR_DLAB;
		UART0_DLL = (unsigned char) (ulDivisor & (unsigned long) 0xff);
		ulDivisor >>= 8;
		UART0_DLM = (unsigned char) (ulDivisor & (unsigned long) 0xff);

		//  Turn on the FIFO's and clear the buffers
		UART0_FCR = UART_FCR_EN | UART_FCR_CLR;
		UART0_FCR |= (1 << 6); // 0=1char trigger, 1=4char, 2=8char, 3=14char trigger

		//  Setup transmission format
		UART0_LCR = UART_LCR_NOPAR | UART_LCR_1STOP | UART_LCR_8BITS;

		//  Setup the VIC for the UART
		VICIntSelect &= ~VIC_IntSelect_UART0;
		VICVectAddr2 = (long) uart0ISR;
		VICVectCntl2 = VIC_VectCntl_ENABLE | VIC_Channel_UART0;
		VICIntEnable |= VIC_IntEnable_UART0;

		//  Enable UART0 interrupts
		UART0_IER |= UART_IER_EI;
	}
	portEXIT_CRITICAL ();

	return 1;
}

void uart0Deinit(void)
{
	VICIntEnClr = 0x00000040;
	PCONP &= ~8;
	PINSEL0 &= 0xfffffff0;
}

unsigned long uart0GetChar(char *pcRxedChar, portTickType xBlockTime)
{
	return xQueueReceive (xRX0Queue, pcRxedChar, xBlockTime) ? pdTRUE : pdFALSE;
}

unsigned long uart0PutCharPolling(char cOutChar, unsigned long c)
{
	while (!(U0LSR & (1 << 5)))
		;
	UART0_THR = cOutChar;
	return 1;
}

unsigned long uart0PutChar(char cOutChar, portTickType xBlockTime)
{
	signed portBASE_TYPE xReturn = 0;

	portENTER_CRITICAL ();
	{
		//  Is there space to write directly to the UART?
		if (lTHREEmpty0)
		{
			lTHREEmpty0 = 0;
			UART0_THR = cOutChar;
			xReturn = pdPASS;
		}
		else
		{
			//  We cannot write directly to the UART, so queue the character.  Block for a maximum of
			//  xBlockTime if there is no space in the queue.
			xReturn = xQueueSend (xTX0Queue, &cOutChar, xBlockTime);

			//  Depending on queue sizing and task prioritisation:  While we were blocked waiting to post
			//  interrupts were not disabled.  It is possible that the serial ISR has emptied the Tx queue,
			//  in which case we need to start the Tx off again.
			if (lTHREEmpty0 && (xReturn == pdPASS))
			{
				xQueueReceive (xTX0Queue, &cOutChar, 0);
				lTHREEmpty0 = 0;
				UART0_THR = cOutChar;
			}
		}
	}
	portEXIT_CRITICAL ();

	return xReturn;
}

