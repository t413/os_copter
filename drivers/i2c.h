/* 
 * i2c library
 * Created by Tim O'Brien, t413.com
 * September 26, 2010
 *
 * * Releaced under the Creative Commons Attribution-ShareAlike 3.0
 * * You are free 
 * *  - to Share, copy, distribute and transmit the work
 * *  - to Remix — to adapt the work
 * * Provided 
 * *  - Attribution (my name and website in the comments in your source)
 * *  - Share Alike - If you alter, transform, or build upon this work, 
 * *      you may distribute the resulting work only under 
 * *      the same or similar license to this one
 *
 * if you are unfamiliar with the terms of this lisence,
 *  would like to see the full legal code of the license,
 *  or have any questions please email me (timo@t413.com) or visit:
 *  http://creativecommons.org/licenses/by-sa/3.0/us/   for details
 */

#ifndef __i2c_h
#define __i2c_h


/*
 * i2c_init - initialize/set up the i2c bus
 *  Input: 
 *   -frequency (in MHz) to run the bus at.
 */
void i2c_init(unsigned int freq);


/* 
 * i2c_send  -  send data array to an i2c device.
 *  Input: 
 *   -the address of the slave to talk to
 *   -the register of the slave to read/write to
 *   -and array to read from/write to.
 *   -the legnth of that array
 */
unsigned char i2c_send(unsigned char slaveAddr, unsigned char slaveReg, volatile unsigned char * data, unsigned int legnth);

/* 
 * i2c_send_byte  -  start talking to an i2c device at an address.
 *  Input: 
 *   -the address of the slave to talk to
 *   -the register of the slave to read/write to
 *   -the single byte to write to the register
 */
unsigned char i2c_send_byte(unsigned char slaveAddr, unsigned char slaveReg, unsigned char data);

/* 
 * i2c_receive  -  receive an array of data.
 *  Input: 
 *   -the address of the slave to talk to
 *   -the register of the slave to read/write to
 *   -and array to read from/write to.
 *   -how many bytes to read (likely also the legnth of that array)
 */
unsigned char i2c_receive(unsigned char slaveAddr, unsigned char slaveReg, volatile unsigned char * data, unsigned int legnth);

/* 
 * i2c_receive_byte  -  receive a single byte.
 *  Input: 
 *   -the address of the slave to talk to
 *   -the register of the slave to read/write to
 *  Output: 
 *   -the unsigned byte coming from the slave.
 */
unsigned char i2c_receive_byte(unsigned char slaveAddr, unsigned char slaveReg);

#endif 
