#ifndef _SYSCONFIG_H_
#define _SYSCONFIG_H_

#define FOSC 			12000000UL	///<  Crystal Frequency
#define USE_PLL			1			///<  1: Use PLL to multiply crystal frequency.  0: System Clock = Crystal Frequency
#define PLL_MUL			4			///<  PLL multiplication factor (1 to 32). This results in CPU Frequency.
#define PLL_DIV			2			///<  PLL division factor (1, 2, 4, or 8).
#define PBSD			1			///<  Peripheral bus speed divider (1, 2, or 4).

#define stackSize_UND     16		///< UND Interrupt Stack Size
#define stackSize_ABT     16		///< ABT Interrupt Stack Size
#define stackSize_FIQ     32		///< FIQ Interrupt Stack Size
#define stackSize_IRQ     512		///< IRQ Interrupt Stack Size
#define stackSize_SVC     512		///< Supervisor Mode Stack size


///////////////////////////////////////////////
/// Do not change anything below this line! ///
///////////////////////////////////////////////
















#if (USE_PLL == 1)
#if PLL_MUL > 32
#error PLL_MUL must be in the range 1-32
#elif PLL_MUL < 1
#error PLL_MUL must be in the range 1-32
#endif

#if PLL_DIV == 1
#elif PLL_DIV == 2
#elif PLL_DIV == 4
#elif PLL_DIV == 8
#else
#error PLL_DIV must be 1, 2, 4, or 8
#endif

#define CCLK (FOSC * PLL_MUL)          		///< CPU core clock frequency
#if ((CCLK < 10000000) || (CCLK > 60000000))
#error CCLK is out of range (Valid Range is between 10MHz-60MHz)
#endif

#define FCCO (FOSC * PLL_MUL * 2 * PLL_DIV)	///< CC Osc. Freq.
#if ((FCCO < 156000000))
#error FCCO is out of range (valid range is: 156MHz-320MHz). Try increasing PLL_DIV or PLL_MUL.
#elif(FCCO > 320000000)
#error FCCO is out of range (valid range is: 156MHz-320MHz). Try decreasing PLL_DIV or PLL_MUL.
#endif

#else
#define CCLK (FOSC)
#endif

#if PBSD == 1
#elif PBSD == 2
#elif PBSD == 4
#else
#error PBSD must be 1, 2, or 4
#endif
#define PCLK (CCLK / PBSD)





#if USE_PLL == 1
#define CORE_FREQ (FOSC * PLL_MUL)
#define PERI_FREQ (CORE_FREQ/PBSD)
#else
#define CORE_FREQ (FOSC)
#define PERI_FREQ (FOSC/PBSD)
#endif

/* MAM (Memory Settings Accelerator Module)
 * Number of CLOCKS to read from the FLASH
 */
#if CORE_FREQ < 20000000
#define MAM_TIMING   1
#elif CORE_FREQ < 40000000
#define MAM_TIMING   2
#else
#define MAM_TIMING   3
#endif

#define MAM_SETTING  2                /* 0=disabled,
                                         1=partly enabled (enabled for code prefetch, but not for data),
                                         2=fully enabled */

/* initialize the exception vector mapping */
#ifndef RAM_EXEC
#define MAM_MAP      1                  /* 1 = exception vectors are in FLASH at 0x0000 0000,
                                           2 = exception vectors are in SRAM at 0x4000 0000   */
#else
#define MAM_MAP 2                       /* When executing from RAM, MAM_MAP should always be 2 */
#endif

#endif  /* #ifdef _SYSCONFIG_H */
