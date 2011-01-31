#include "userInterface.h"

#include "../osHandles.h"
#include "../drivers/uart/uart0.h"  // uart0GetChar()
#include "../System/rprintf.h"      // rprintf
#include "../System/crash.h"        // To crash the system :)
#include "../System/lpc214x.h"

#include <stdlib.h>                 // atoi()
#include <stdio.h>                  // printf() or iprintf()
#include <string.h>                 // strtok()
#include <math.h>                   // strtok()

#include "../drivers/i2c.h"
#include "../drivers/ssp_spi.h"
#include "../drivers/motors.h"
#include "../drivers/pid.h"
#include "../drivers/ser_pkt.h"

#include "../drivers/external_interrupt.h"

#define MINIMUM(a,b)		(((a)>(b))? (b):(a))


/* ---- private declarations ---- */
void getUartLine(char* uartInput);
#define MATCH(a,b)		(strcmp(a,b) == 0)
unsigned char send_zero(unsigned char address, unsigned char loc);
void scan_i2c();


void uartUI(void *pvParameters)
{
	OSHANDLES *osHandles = (OSHANDLES*)pvParameters;
	vTaskDelay(OS_MS(100));
#if 0 //testing sonar
	enable_interupt_ext0();
	unsigned int last = 0;
	for (;;)
	{
		unsigned int current = pwm_in_delta_t();
		if (current != last) {
			rprintf("change: %i\n",current);
			vTaskDelay(OS_MS(50));
		}
		last = current;
	}
#endif
#if 1 //flight task

	for (;;){
		unsigned char packet[128] = "";
		if (getSerialPacket(packet) != NULL)
		{
			if (packet[2] == USER_CONTROL){
				if (packet[3] == ACC_DATA){
					//ACCtelem recievedA = decode_acc_data(packet+5);
				}
				else if (packet[3] == FULL_REMOTE){
					FourU16 recieved = decode_4xint16(packet+5);

					osHandles->flight_control.tx_roll  =   (recieved.d0-1500)/16+1500;  //scale down this stick action.
					osHandles->flight_control.tx_pitch = (-(recieved.d1-1500))/16+1500;  //scale down this stick action.
					osHandles->flight_control.tx_yaw   =   (recieved.d2-1500)/16+1500;  //scale down this stick action.
					osHandles->flight_control.tx_throttle = recieved.d3;

					osHandles->flight_control.command_used_number = 0;

					if (osHandles->flight_control.tx_throttle < MIN_SAFETY) {
						osHandles->flight_settings.pid_roll->error = 0; //zero the integral error
						osHandles->flight_settings.pid_pitch->error = 0; //zero the integral error
						osHandles->flight_settings.pid_yaw->error = 0; //zero the integral error

						// enable flying (arm it) when yaw-> throttle==min.
						if (recieved.d2 > MAX_SAFETY && osHandles->flight_control.armed == 1) {
							osHandles->flight_control.armed = 3; //armed=3 means ready to fly
							if (osHandles->flight_control.telem_mode) rprintf("armed\n");
						}
						//armed=1 means we've gotten one packet with yaw>MINCHECK
						if (recieved.d2 > MAX_SAFETY) osHandles->flight_control.armed |= 1;

						if (recieved.d2 < MIN_SAFETY) { osHandles->flight_control.armed = 0; write_motors_zero(); } //disarm when yaw

						if ((recieved.d2 < MIN_SAFETY) && (recieved.d0 > MAX_SAFETY) && (recieved.d1 < MIN_SAFETY))
							{ osHandles->flight_control.please_update_sensors = 1; } //zero sensors.
					}
					if (osHandles->flight_control.telem_mode) {
						rprintf("got data! : r:%i p:%i y:%i t:%i\n",
								osHandles->flight_control.tx_roll,
								osHandles->flight_control.tx_pitch,
								osHandles->flight_control.tx_yaw,
								osHandles->flight_control.tx_throttle);
					}
				}
			}
			else if (packet[2] == SETTINGS_COMM)
			{
				switch (packet[3])
				{
					case 'X':	//kill signal
					{
						osHandles->flight_control.armed = 0;
						write_motors_zero();
						break;
					}
					case 'z':	// Zero sensors
					{
						if (! osHandles->flight_control.armed)
							osHandles->flight_control.please_update_sensors = 1;
						break;
					}
					case 'm':	// pulse single motor
					{
						//FRONTMOTORPIN,REARMOTORPIN, LEFTMOTORPIN, RIGHTMOTORPIN
						uint8_t whichm = *(uint8_t *) (packet+5);
						pulse_single_motor(whichm,150);
						break;
					}
					case REMOTE_2_QUAD_SETTINGS:// update PID values
					{
						int16_t values[9] = {0};
						decode_some_int16s(packet+5, values, packet[4]/2 ); //should be 9.

						osHandles->flight_settings.pid_pitch->p = values[0];
						osHandles->flight_settings.pid_pitch->i = values[1];
						osHandles->flight_settings.pid_pitch->d = values[2];
						osHandles->flight_settings.pid_roll->p = values[3];
						osHandles->flight_settings.pid_roll->i = values[4];
						osHandles->flight_settings.pid_roll->d = values[5];
						osHandles->flight_settings.pid_yaw->p = values[6];
						osHandles->flight_settings.pid_yaw->i = values[7];
						osHandles->flight_settings.pid_yaw->d = values[8];

						osHandles->flight_settings.flying_mode = values[9]>>8;
						osHandles->flight_settings.led_mode = values[9] & 0xFF;

						if (osHandles->flight_control.telem_mode) {
							rprintf("\n updated pids to(*10): %i,%i,%i,%i,%i,%i,%i,%i,%i",
									(osHandles->flight_settings.pid_pitch->p),
									(osHandles->flight_settings.pid_pitch->i),
									(osHandles->flight_settings.pid_pitch->d),
									(osHandles->flight_settings.pid_roll->p),
									(osHandles->flight_settings.pid_roll->i),
									(osHandles->flight_settings.pid_roll->d),
									(osHandles->flight_settings.pid_yaw->p),
									(osHandles->flight_settings.pid_yaw->i),
									(osHandles->flight_settings.pid_yaw->d)
									       );
							rprintf("\nflying_mode=%i led_mode=%i \n\n",osHandles->flight_settings.flying_mode, osHandles->flight_settings.led_mode );
						}

						if (! osHandles->flight_control.armed) {  // motor feedback if not armed
							for (unsigned char i = 0; i<4; i++){
								pulse_single_motor(i%4,100);
								vTaskDelay(50*configTICK_RATE_HZ/1000);
							}
						}
						break; // <- NO BREAK, I want it to send the PIDs back.
					case 'p':// request for PID values
					{
						int16_t values[] = {
								osHandles->flight_settings.pid_pitch->p, osHandles->flight_settings.pid_pitch->i, osHandles->flight_settings.pid_pitch->d,
								osHandles->flight_settings.pid_roll->p, osHandles->flight_settings.pid_roll->i, osHandles->flight_settings.pid_roll->d,
								osHandles->flight_settings.pid_yaw->p, osHandles->flight_settings.pid_yaw->i, osHandles->flight_settings.pid_yaw->d,
								(osHandles->flight_settings.flying_mode<<8) + osHandles->flight_settings.led_mode
						};
						send_some_int16s(SETTINGS_COMM,QUAD_2_REMOTE_SETTINGS,values, sizeof(values));
						break;
					}
					}
					case 'r':{	//TEMPORARY!!! telemetry toggle (for debug mode)
						if (! osHandles->flight_control.armed)
							pulse_single_motor(0,50);
						osHandles->flight_control.telem_mode = !osHandles->flight_control.telem_mode;
						break;
					}
					case '$':{	//TEMPORARY!!! pulse pattern to test pin direction.
						pulse_single_motor(0,150);vTaskDelay(OS_MS(150));
						pulse_single_motor(1,150);vTaskDelay(OS_MS(150));
						pulse_single_motor(2,150);vTaskDelay(OS_MS(150));
						pulse_single_motor(3,150);
						break;
					}
					/* not implemented yet.
					case 't':{	//telemetry ON / OFF
						// Implementation:
						//   send_byte_packet(SETTINGS_COMM,(uint8_t) 't',1);
						osHandles->flight_control.telem_mode = *(uint8_t *) (packet+5);
						break;
					}
					case 'r':{  // altitude control.
					if (altitudeAdjustTo){
							altitudeAdjustTo = 0;
							if (!armed) motors.pulseMotorsTimes(1);
						}
						else {
							altitudeAdjustTo = 24;
							if (!armed) motors.pulseMotorsTimes(2);
						}
						}
						break;
					}
					case '$':{	//swich flight mode
						if (flyingMode == ACROBATICMODE){
							flyingMode = STABLEMODE;
							if (!armed){
								motors.pulseSingleMotor(2,150);
								motors.pulseSingleMotor(3,150);
							}
						}
						else {
							flyingMode = ACROBATICMODE;
							if (!armed)
								motors.pulseSingleMotor(1,150);
						}
						break;
					}
					case '@':{	//heading hold on/off
						headingHoldOn = !headingHoldOn; //toggle value
						if (!armed)
							motors.pulseSingleMotor(!headingHoldOn,150);
						break;
					}
					case 'b':{	//beep out the PID values
						// implimentation:
						//   send_byte_packet(SETTINGS_COMM,(uint8_t) 'b',1);
						//ROLL 0, PITCH 1, YAW 2, LEVELROLL 3, LEVELPITCH 4, HEADING 5, ALTITUDE 6
						uint8_t thispid = *(uint8_t *) (packet+5);
						if (!armed)
							beepFloat( PID[thispid].P );
						break;
					}
					case 's':{	//show or illustrate what pid value is about to be adjusted with the motors
						// implimentation:
						//   send_byte_packet(SETTINGS_COMM,(uint8_t) 's',1);
						//ROLL 0, PITCH 1, YAW 2, LEVELROLL 3, LEVELPITCH 4, HEADING 5, ALTITUDE 6
						uint8_t thispid = *(uint8_t *) (packet+5);
						if (!armed){
							if (thispid == ROLL) //pulse left-right, left-right
								for (uint8_t i = 0; i<4; i++)
									motors.pulseSingleMotor(i%2+2,150);
							else if (thispid == PITCH) //pulse front-back, front-back
								for (uint8_t i = 0; i<4; i++)
									motors.pulseSingleMotor(i%2,150);
							else if (thispid == YAW){ //pulse front-back then sides
								motors.pulseMotors(150,0,2,3);
								delay(200);
								motors.pulseMotors(150,1,2,3);
							}
							else if (thispid == LEVELROLL) //pulse front-back, front-back
								motors.pulseMotors(200,2,3);
							else if (thispid == LEVELPITCH) //pulse front-back, front-back
								motors.pulseMotors(200,0,1);
							else if (thispid == HEADING) { //pulse front-back, front-back
								motors.pulseMotors(100,0);
								delay(200);
								motors.pulseMotors(100,0);
								delay(200);
								motors.pulseMotors(100,0);
							}
							else if (thispid == ALTITUDE) { //pulse front-back, front-back
								motors.pulseMotors(200,1);
								delay(250);
								motors.pulseMotors(200,1);
								delay(100);
								motors.pulseMotors(200,0);
							}
						}
						break;
					}
					case 'U':{      //increment the P value of the given axis by 0.1
						//ROLL 0, PITCH 1, YAW 2, LEVELROLL 3, LEVELPITCH 4, HEADING 5, ALTITUDE 6
						uint8_t thispid = *(uint8_t *) (packet+5);
						if ((!armed) && (thispid>=0) && (thispid < 7)){
							uint16_t temp = PID[thispid].P * 10;
							PID[thispid].P = (float)(temp+5)/10;
							motors.pulseSingleMotor(0,150);
						}
						break;
					}
					case 'D':{      //decrement the P value of the given axis by 0.1
						uint8_t thispid = *(uint8_t *) (packet+5);
						if ((!armed) && (thispid>=0) && (thispid < 7)){
							uint16_t temp = PID[thispid].P * 10;
							PID[thispid].P = (float)(temp-5)/10;
							motors.pulseSingleMotor(1,150);
						}
						break;
					}
					case 'S':{	//save PID.D values to EEPROM
						writeFloat(PID[ROLL].P, PGAIN_ADR);
						writeFloat(PID[PITCH].P, PITCH_PGAIN_ADR);
						writeFloat(PID[LEVELROLL].P, LEVEL_PGAIN_ADR);
						writeFloat(PID[LEVELPITCH].P, LEVEL_PITCH_PGAIN_ADR);
						writeFloat(PID[YAW].P, YAW_PGAIN_ADR);
						writeFloat(PID[HEADING].P, HEADING_PGAIN_ADR);
						writeFloat(PID[ALTITUDE].P, ALTITUDE_PGAIN_ADR);
						motors.pulseMotors(250,0,2);
						motors.pulseMotors(250,1,3);
						break;
					}
					*/

				}
			}
		}
	}
#endif
#if 0 //terminal
	char uartInput[128];
	for (;;)
	{
		rprintf("tim: "); //print prompt
		memset(uartInput, 0, sizeof(uartInput));  //clear buffer
		getUartLine(uartInput);  //waits for return
		if (strlen(uartInput) <= 0) continue; //be sure there's a command here.

		char *command = strtok(uartInput, " ");

		if (MATCH(command, "echo")){
			char *echoBack = strtok(NULL, "");
			rprintf("Echo: %s\n", echoBack);
		}
		else if (MATCH(command, "p")){
			rprintf("c=%i, line is %s\n", pwm_in_delta_t(), (IOPIN0 & (1<<16))?"high":"low");

		}
		else if (MATCH(command, "scan")){
			scan_i2c();
		}
		/*else if (MATCH(command, "led")){
			int led = atoi(strtok(NULL, " "));
			if ((led < 7)&&(led > 0)){
				int val = atoi(strtok(NULL, ""));
				write_PWM(led, val );
				rprintf("wrote %i to LED %i\n",val,led);
			}
		}
		else if (MATCH(command, "servo")){
			int val2 = atoi(strtok(NULL, ""));
			write_PWM_servo(5, val2 );
			rprintf("wrote %i to servo 5\n",val2);
		}*/
		else if (MATCH(command, "t")){
			osHandles->flight_control.tx_throttle = atoi(strtok(NULL, ""));
			rprintf("throttle = %i\n",osHandles->flight_control.tx_throttle);
		}
		else if (MATCH(command, "telem")){
			osHandles->flight_control.telem_mode = !osHandles->flight_control.telem_mode;
			rprintf("flight mode=%i\n",osHandles->flight_control.telem_mode);
		}


		/*---------cpu info opperations--------*/
		else if (MATCH(command, "CRASH")) {
			char *crashType = strtok(NULL, "");

			if(MATCH(crashType, "UNDEF"))		performUndefinedInstructionCrash();
			else if(MATCH(crashType, "PABORT"))	performPABORTCrash();
			else if(MATCH(crashType, "DABORT"))	performDABORTCrash();
			else rprintf("Define the crash type as either UNDEF, PABORT, or DABORT\n");
		}
		#if configGENERATE_RUN_TIME_STATS
		else if(MATCH(command, "CPU")) {
			 vTaskGetRunTimeStats(uartInput, 0);
			rprintf("%s", uartInput);
		}
		else if(MATCH(command, "CPUr")) {
			vTaskGetRunTimeStats(uartInput, 1);
			rprintf("%s", uartInput);
		}
		else if(MATCH(command, "CPUn")) {
			int delayTimeMs = atoi(strtok(NULL, ""));
			if(delayTimeMs > 0) {
				rprintf("Delaying for %ims.  CPU Usage will be reported afterwards...\n", delayTimeMs);
				vTaskGetRunTimeStats(uartInput, 1);
				vTaskDelay(OS_MS(delayTimeMs));
				vTaskGetRunTimeStats(uartInput, 1);
				rprintf("%s", uartInput);
			}
			else {
				rprintf("You must define a delay time in milliseconds as parameter.\n");
			}
		}
		#endif
		#if INCLUDE_uxTaskGetStackHighWaterMark
		else if(MATCH(command, "watermark")) {
			rprintf("High watermarks (closer to zero = bad)\n");
			rprintf("userInterface : % 5i\n", 4*uxTaskGetStackHighWaterMark(osHandles->task.userInterface));
		}
		#endif
		else if (MATCH("HELP", command)) {
			rprintf("Command list:\n fix me!");
		}

		else {
			rprintf("Command not recognized.\n");
		}
	}
#endif
}

void getUartLine(char* uartInput)
{
	char data;
	unsigned int uartInputPtr = 0;

	while (1)
	{
		while(!uart0GetChar(&data, portMAX_DELAY));

		if (data == '\n'){
			uartInput[uartInputPtr] = 0;
			uart0PutChar('\n', 100);
			break;
		}
		else if (data == '\r'){
			// Ignore it
		}
		else if (data == '\b')
		{
			if (uartInputPtr > 0){
				uartInputPtr--;
				rprintf("\b \b");
			}
			else {					// Nothing to backspace
				uart0PutChar(7, 0);	// ASCII Char 7 is for bell
			}
		}
		/*
		// If 1st char is +/- then call it quit
		else if(uartInputPtr == 0 && (data == '+' || data == '-')) {
			uartInput[0] = data;
			uartInput[1] = '\0';
			break;
		}
		*/
		else{
			uart0PutChar(data, 100);
			uartInput[uartInputPtr++] = data;
		}
	}
}

unsigned char send_zero(unsigned char address, unsigned char loc){
	unsigned char dummy[1];
	return i2c_send(address, loc, dummy, 0);
}

void scan_i2c(){
	rprintf("Scanning i2c bus:\n");
	int k=0x10;
	while (k < 0xF1){
		if (send_zero(k,0x00))
			rprintf("\nfound at 0x%x\n",k);
		else rprintf("\t %x ",k);
		if (!(k%16)) rprintf("\n");
		k += 2;
	}
}


