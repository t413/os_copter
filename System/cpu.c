//
//  $Id: cpu.c 356 2009-01-12 04:11:31Z jcw $
//  $Revision: 356 $
//  $Author: jcw $
//  $Date: 2009-01-11 23:11:31 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/cpu/cpu.c $
//


#include "cpu.h"
#include "../System/sysConfig.h"
#include "../System/lpc214x.h"
#include "../System/rprintf.h"

typedef struct intVects_s
{
  unsigned int reset;
  unsigned int undefined;
  unsigned int swi;
  unsigned int pabt;
  unsigned int dabt;
  unsigned int reserved;
  unsigned int irq;
  unsigned int fiq;

  void *reset_handler;
  void *undefined_handler;
  void *swi_handler;
  void *pabt_handler;
  void *dabt_handler;
  void *reserved_handler;
  void *irq_handler;
  void *fiq_handler;
}
__attribute__ ((packed)) intVects_t;

extern unsigned int __intvects;

void cpuSetupHardware (void)
{
#ifdef RUN_FROM_RAM
  //
  //  Remap the interrupt vectors to RAM if we are are running from RAM
  //
  SCB_MEMMAP = SCB_MEMMAP_URM;
#endif

  //
  //  Configure pin functions.  All pins are set to GPIO, including the Debug
  //  port (P1.26) and the Trace port (P1.25..P1.16).
  //
  PINSEL0 = 0;
  PINSEL1 = 0;
  PINSEL2 = 0;
  IODIR0 = 0;
  IODIR1 = 0;

#if USE_PLL

	#if (PLL_DIV == 1)
	#define PLL_DIV_VALUE 0x00
			PLLCFG = ((PLL_MUL - 1) | (0x00<<5));
	#elif (PLL_DIV == 2)
			PLLCFG = ((PLL_MUL - 1) | (0x01<<5));
	#elif (PLL_DIV == 4)
			PLLCFG = ((PLL_MUL - 1) | (0x10<<5));
	#elif (PLL_DIV == 8)
			PLLCFG = ((PLL_MUL - 1) | (0x11<<5));
	#endif

	PLLCON = 1;

	PLLFEED = 0xaa;
	PLLFEED = 0x55;
	while((PLLSTAT & (1 << 10)) == 0);
	PLLCON = 3;			// Connect PLL clock to System Clock
	PLLFEED = 0xaa;
	PLLFEED = 0x55;
#else
	PLLCON = 0;
#endif

  //  Setup and turn on the MAM.  Three cycle access is used due to the fast
  //  PLL used.  It is possible faster overall performance could be obtained by
  //  tuning the MAM and PLL settings.
  MAMTIM = MAM_TIMING;
  MAMCR = MAM_SETTING;

  // Set the Peripheral Clock Divider
#if(PBSD == 4)
	VPBDIV = 0;
#else
	VPBDIV = PBSD;
#endif

  //  Make sure all interrupts disabled
  VICIntEnClr = (0x007ffffd);

  //
  //  Put FIQ handler in RAM.  If there wasn't enough space (because malloc()
  //  failed, and that would be *really* unusual), cpuSetupFIQISR() won't be
  //  called by fiqFIQISRCopyToRAM(), so the FIQ handler will remain in flash.
  //  If CFG_RAM_INTS is not defined, the FIQ handler be executed from flash,
  //  not RAM, because the FIQ vector will point to the version in flash, not
  //  in RAM.  It'll all still work, but just slower.
  //
#ifdef CFG_FIQ
  fiqFIQISRCopyToRAM ();
#endif

#ifdef CFG_RAM_INTS
  //
  //  Lastly, switch interrupt handlers to RAM vectors
  //
  SCB_MEMMAP = SCB_MEMMAP_URM;
#endif
}

//
//  If CFG_RAM_INTS is not defined, FIQ vector will be in the flash interrupt 
//  vector table (in boot.s), and the FIQ handler code copied to RAM will not
//  not be used.  The FIQ will still run, but will be slower.
//
void cpuSetupFIQISR (void *FIQInterrupt)
{
#ifdef CFG_RAM_INTS
  intVects_t *ivRam = (intVects_t *) &__intvects;

  ivRam->fiq_handler = FIQInterrupt;
#endif
}

void cpuPrintMemoryInfo(void)
{
	/* Extern variables are defined by the linker.  */
	extern unsigned int __bss_end__;

	const unsigned int stackSize = stackSize_UND +stackSize_ABT +stackSize_FIQ + stackSize_IRQ + stackSize_SVC;
	const unsigned int staticMemSize = ((unsigned)&__bss_end__ - (unsigned)0x40000000);
	const unsigned int freeRAM = (32*1024UL) - staticMemSize - stackSize - 32;

	rprintf("\n");
	rprintf("------------------------------\n");
	rprintf("----- Memory Information -----\n");
	rprintf("------------------------------\n");
	rprintf("--       Total Stack: % 5u --\n", stackSize);
	rprintf("--  Approx. Used Mem: % 5u --\n", staticMemSize);
	rprintf("--  Approx. Heap Mem: % 5u --\n", freeRAM);
	rprintf("------------------------------\n");
}

