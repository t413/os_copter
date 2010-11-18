/*
//  $Id: lpc2148-rom.ld 20 2008-10-02 19:38:42Z jcw $
//  $Revision: 20 $
//  $Author: jcw $
//  $Date: 2008-10-02 15:38:42 -0400 (Thu, 02 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/lpc2148-rom.ld $
*/

MEMORY 
{
	flash	(rx) : ORIGIN = 4k, LENGTH = 496k
	ram		(rw) : ORIGIN = 0x40000000, LENGTH = 32K - 32
}

__stack_end__ = 0x40000000 + 32K - 36;

SECTIONS 
{
	. = 0;
	startup : 
  { 
    __start_of_startup__ = .;
    *(.startup)
    __end_of_startup__ = .;
  } >flash

	.ctors :
	{
		PROVIDE(__ctors_start__ = .);
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		PROVIDE(__ctors_end__ = .);
	} >flash
	
	.dtors :
	{
		PROVIDE(__dtors_start__ = .); 
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		PROVIDE(__dtors_end__ = .);
	} >flash
    . = ALIGN(4);
    
	prog : 
	{
    __start_of_text__ = .;
    __start_of_prog__ = .;
		*(.text)
    __end_of_prog__ = .;
    __start_of_rodata__ = .;
		*(.rodata)
		*(.rodata*)
    __end_of_rodata__ = .;
    __start_of_glue7__ = .;
		*(.glue_7)
		*(.glue_7t)
    __end_of_glue7__ = .;
	} >flash

	__end_of_text__ = .;

  .intvects :
  {
    __intvects_beg__ = .;
		__intvects_beg_src__ = __end_of_text__;
    *(.intvects)
    __intvects_end__ = .;
  } >ram AT>flash

	__end_of_intvects__ = __end_of_text__ + (__intvects_end__ - __intvects_beg__);

  .protected :
  {
    __protected_beg__ = .;
    *(.protected)
    __protected_end__ = .;
  } >ram
  
	.data : 
	{
		__data_beg__ = .;
		__data_beg_src__ = __end_of_intvects__;
		*(.data)
		__data_end__ = .;
	} >ram AT>flash

	.bss : 
	{
		__bss_beg__ = .;
		*(.bss)
	} >ram

	/* Align here to ensure that the .bss section occupies space up to
	_end.  Align after .bss to ensure correct alignment even if the
	.bss section disappears because there are no input sections.  */
	. = ALIGN(32 / 8);
}
	. = ALIGN(32 / 8);
	_end = .;
	_bss_end__ = .; 
  __bss_end__ = .; 
  __heap_beg__ = .;
  __heap_end__ = .;
  __end__ = . ;
	PROVIDE (end = .);
