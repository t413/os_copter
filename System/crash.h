
#ifndef CRASH_H_
#define CRASH_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct abortDat_s
{
  unsigned int dummy;
  unsigned int sigil;
  unsigned int count;
  unsigned int type;
  unsigned int pc;
  unsigned int opcode;
  unsigned int cpsr;
  unsigned int lr;
  unsigned int sp;
  unsigned int r0;
  unsigned int r1;
  unsigned int r2;
  unsigned int r3;
  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;
  unsigned int r12;
  unsigned int stack [8];
}
__attribute__ ((packed)) abortDat_t;
extern unsigned int __abort_dat;

int printCrashInfo ();
int clearCrashInfo ();
int didSystemCrash();

void performUndefinedInstructionCrash();
void performPABORTCrash();
void performDABORTCrash();

#ifdef __cplusplus
}
#endif

#endif /* CRASH_H_ */
