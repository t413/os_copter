//
//  $Id: sysdefs.h 328 2008-11-09 05:00:23Z jcw $
//  $Revision: 328 $
//  $Author: jcw $
//  $Date: 2008-11-09 00:00:23 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/sysdefs.h $
//

#ifndef _SYSDEFS_H_
#define _SYSDEFS_H_

typedef unsigned char U8;
typedef signed char N8;
typedef unsigned short U16;
typedef signed short N16;
typedef unsigned int U32;
typedef signed int N32;

typedef volatile U8 REG8;
typedef volatile U16 REG16;
typedef volatile U32 REG32;

#define pREG8  (REG8 *)
#define pREG16 (REG16 *)
#define pREG32 (REG32 *)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define MINIMUM(x,y) ((x)<(y)?(x):(y))
#define MAXIMUM(x,y)((x)>(y)?(x):(y))
#define arrsizeof(x) ((sizeof (x))/(sizeof (x [0])))


#endif
