/***********************************************************************/
/*                                                                     */
/*  SYSCALLS.C:  System Calls for the newlib                           */
/*  most of this is from newlib-lpc and a Keil-demo                    */
/*                                                                     */
/*  These are "reentrant functions" as needed by                       */
/*  the WinARM-newlib-config, see newlib-manual.                       */
/*  Collected and modified by Martin Thomas                            */
/*                                                                     */
/***********************************************************************/

#include "sysConfig.h"
#include "../drivers/uart/uart0.h"
#include "./rprintf.h"

#include <stdlib.h>
#include <reent.h>
#include <sys/stat.h>

static inline int inputDeviceGetChar()
{
	// Put the function here that will return a char that standard C/C++ (scanf) will use
	char ch;
	if(uart0GetChar(&ch, 0xffffffff)) {
		return ch;
	}
	return 0;
}
static inline int outputDevicePutChar(char c)
{
	// Put the function here that will get a char that standard C/C++ (printf) will use
	uart0PutChar((signed char)c, 0xffffffff);
	return 1;
}

	#ifdef __cplusplus
	void* operator new(size_t s)
	{
		return malloc(s);
	}
	void operator delete(void* p)
	{
		free(p);
	}
	void *operator new[] (size_t s)
	{
		return malloc(s);
	}

	void operator delete[] (void* p)
	{
		free(p);
	}
	#endif

_ssize_t _getpid_r(struct _reent *r)
{
	rprintf("Get PId R\n");
	return 0;
}
int _kill_r _PARAMS ((struct _reent *a, int b, int c))
{
	rprintf("Kill R?\n");
	return 0;
}


_ssize_t _read_r(
    struct _reent *r, 
    int file, 
    void *ptr, 
    size_t len)
{
	int c;
	unsigned char *p;
	
	while ( ( c = inputDeviceGetChar() ) == 0 ) {
		;
	}
	
	p = (unsigned char*)ptr;
	*p = c;
	
	return (_ssize_t)(1);
}


_ssize_t _write_r (
    struct _reent *r, 
    int file, 
    const void *ptr, 
    size_t len)
{
	size_t todo;
	const unsigned char *p;
	
	todo = len;
	p = (const unsigned char*)ptr;
	
	for( ; todo != 0; todo--) {
		outputDevicePutChar(*p++);
	}
	
	return (_ssize_t)len;			/* Number of bytes written.	*/
}


int _close_r(
    struct _reent *r, 
    int file)
{
	return 0;
}


_off_t _lseek_r(
    struct _reent *r, 
    int file, 
    _off_t ptr, 
    int dir)
{
	return (_off_t)0;	/*  Always indicate we are at file beginning.  */
}


int _fstat_r(
    struct _reent *r, 
    int file, 
    struct stat *st)
{
	/*  Always set as character device.				*/
	st->st_mode = S_IFCHR;
	/* assigned to strong type with implicit 	*/
	/* signed/unsigned conversion.  Required by 	*/
	/* newlib.					*/

	return 0;
}

#if 1
int isatty(int file); /* avoid warning */

int isatty(int file)
{
	return 1;
}
#endif


#if 0
static void _exit (int n) {
label:  goto label; /* endless loop */
}
#endif 


/* "malloc clue function" from newlib-lpc/Keil-Demo/"generic" */

#if 1
register char *stack_ptr asm ("sp");
void * _sbrk_r(
		struct _reent *_s_r,
		ptrdiff_t incr)
{
	extern char end asm ("end");  /* Defined by the linker.  */
	extern unsigned long __heap_max;
	static char *heap_end;
	char *prev_heap_end;
	char *eom;

	if (!heap_end)
		heap_end = & end;

	prev_heap_end = heap_end;

	//
	//  If FreeRTOS is not running, the stack pointer will be in the SVC region,
	//  and therefore greater than the start of the heap (end of .bss).  In this
	//  case, we see if the heap allocation will run past the current stack
	//  pointer, and if so, return ENOMEM (this does not guarantee subsequent
	//  stack operations won't overflow into the heap!).
	//
	//  If FreeRTOS is running, then we define the end of the heap to be the
	//  start of end of the supervisor stack (since FreeRTOS is running, the
	//  system stack, and very little of the supervisor stack space is used).
	//
	eom = (stack_ptr > & end) ? stack_ptr : (char *) __heap_max;

	if (heap_end + incr > eom)
	{
		//errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}
#else
/**** Locally used variables. ****/
// mt: "cleaner": extern char* end;
extern char end[];              /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static char *heap_ptr;		/* Points to current end of the heap.	*/

/************************** _sbrk_r *************************************
 * Support function. Adjusts end of heap to provide more memory to
 * memory allocator. Simple and dumb with no sanity checks.

 *  struct _reent *r -- re-entrancy structure, used by newlib to
 *                      support multiple threads of operation.
 *  ptrdiff_t nbytes -- number of bytes to add.
 *                      Returns pointer to start of new heap area.
 *
 *  Note:  This implementation is not thread safe (despite taking a
 *         _reent structure as a parameter).
 *         Since _s_r is not used in the current implementation, 
 *         the following messages must be suppressed.
 */
void * _sbrk_r(
    struct _reent *_s_r, 
    ptrdiff_t nbytes)
{
	char  *base;		/*  errno should be set to  ENOMEM on error  */

	if (!heap_ptr) {	/*  Initialize if first time through.  */
		heap_ptr = end;
	}
	base = heap_ptr;	/*  Point to end of heap.  */
	heap_ptr += nbytes;	/*  Increase heap.  */
	
	return base;		/*  Return pointer to start of new heap area.  */
}
#endif
