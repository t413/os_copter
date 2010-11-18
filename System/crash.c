#include "./crash.h"
#include "./rprintf.h"
#include <string.h>		// memset function

int printCrashInfo ()
{
  abortDat_t *ad = (abortDat_t *) &__abort_dat;

  rprintf ("\nReport Valid? :%s, sigil=0x%08X, count=%u\n", (ad->sigil == 0xdeadc0de) ? "LIKELY" : "INVALID", ad->sigil, ad->count);
  rprintf ("Abort type: %s\n", (ad->type == 0) ? "UNDEFINED INSTRUCTION" : (ad->type == 1) ? "PREFETCH ABORT" : (ad->type == 2) ? "DATA ABORT" : "unknown");
  rprintf ("** PC=0x%08X, opcode=0x%08X **\n", ad->pc, ad->opcode);
  rprintf ("** Locate value of PC in your *.lst file to find the root cause. **\n\n");

  // wait until previous output is done.  Otherwise system will keep resetting.
  rprintf ("cpsr=0x%08X, sp=0x%08X, lr=0x%08X\n", ad->cpsr, ad->sp, ad->lr);
  rprintf ("r0=0x%08X, r1=0x%08X,  r2=0x%08X,  r3=0x%08X\n", ad->r0, ad->r1, ad->r2, ad->r3);
  rprintf ("r4=0x%08X, r5=0x%08X,  r6=0x%08X,  r7=0x%08X\n", ad->r4, ad->r5, ad->r6, ad->r7);
  rprintf ("r8=0x%08X, r9=0x%08X, r10=0x%08X, r11=0x%08X\n", ad->r8, ad->r9, ad->r10, ad->r11);
  rprintf ("r12=0x%08X\n\n", ad->r12);

  rprintf ("sp[0]=0x%08X, sp[1]=0x%08X, sp[2]=0x%08X, sp[3]=0x%08X\n", ad->stack [0], ad->stack [1], ad->stack [2], ad->stack [3]);
  rprintf ("sp[4]=0x%08X, sp[5]=0x%08X, sp[6]=0x%08X, sp[7]=0x%08X\n\n", ad->stack [4], ad->stack [5], ad->stack [6], ad->stack [7]);

  return 0;
}

int clearCrashInfo ()
{
	// Clear everything but COUNT, which should only reset upon power-loss
	abortDat_t *ad = (abortDat_t *) &__abort_dat;
	unsigned int numberOfResets = ad->count;
	memset (ad, 0, sizeof (* ad));
	ad->count = numberOfResets;
	return 0;
}

int didSystemCrash()
{
	abortDat_t *ad = (abortDat_t *) &__abort_dat;
	return (ad->sigil == 0xdeadc0de) ? 1:0;
}

void performUndefinedInstructionCrash()
{
	asm volatile (" .word 0x06000010" : /* no output */ : /* no inputs */ );
}

void performPABORTCrash()
{
	asm volatile (" ldr r0, =0x00080000" : /* no output */ : /* no inputs */ );
	asm volatile (" mov pc, r0" : /* no output */ : /* no inputs */ );
}

void performDABORTCrash()
{
	unsigned char c;
	volatile unsigned char *ptr = (unsigned char *) 0x40008000;
	c = *ptr;
}
