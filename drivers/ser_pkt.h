/*
 *  ser_pkt.h
 *  Created by Tim O'Brien
 *  
 *  used to encode, send, and decode serilized data
 *  created for UAVs and RC projects
 *  
 *  ideas and some code from microgear.googlecode.com
 */


/*  Packets look like this:
 *  
 *  /------------------------Header--------------------------/ /--data--/ /-------checksum-------/
 *  | Start byte0 | Start1 | packet ID | packet Type | length | **data** | Checksum0 | Checksum1 |
 *  
 *  the data section is variable length is specified (in __ bytes long).
 *  all bytes are sent as uint8_t (unsigned 8 bit ints)
 */

#ifndef SER_PKT_H
#define SER_PKT_H


//datatype definitions
#ifndef int8_t
#define int8_t char
#endif
#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifndef int16_t
#define int16_t short
#endif
#ifndef uint16_t
#define uint16_t unsigned short
#endif


/*****************************************/
/**********   datatypes/macros  **********/
/*****************************************/
#define START_OF_MSG0 0x93
#define START_OF_MSG1 0xE0

// packet IDs (first byte of two)
#define BACKEND_CONTROL	0
#define USER_CONTROL	1
	#define ACC_DATA		0
	#define FULL_REMOTE		1
#define TELEM_FEEDBACK	2
	#define STATUS			0
	#define PITCH_ROLL		1
	#define MOTOR_OUTPUT	2
	#define DEBUG_OUTPUT	3
#define SETTINGS_COMM	3
	#define REQUEST_SETTING	0
	#define RECIEVE_PIDS	1
	#define SEND_PIDS		2

// packet types

typedef struct {
	double x;
	double y;
} ACCtelem;

typedef struct {
	uint16_t d0;
	uint16_t d1;
	uint16_t d2;
	uint16_t d3;
} FourU16;

/*****************************************/
/**********		declarations	**********/
/*****************************************/
void send_int16_packet(uint8_t,uint8_t,int16_t,int16_t,int16_t,int16_t);
void send_float_packet(uint8_t,uint8_t,float,float);
void send_byte_packet(uint8_t,uint8_t,uint8_t);

uint8_t* getSerialPacket();
ACCtelem decode_acc_data( uint8_t *buf );
FourU16 decode_4xint16( uint8_t *buf );
void decode_4xint16_alt( uint8_t * buf, unsigned int* d0, unsigned int* d1, unsigned int* d2, unsigned int*d3);

#endif
