#include "userInterface.h"

#include "../osHandles.h"
#include "../drivers/uart/uart0.h"  // uart0GetChar()
#include "../System/rprintf.h"      // rprintf
#include "../System/crash.h"        // To crash the system :)

#include <stdlib.h>                 // atoi()
#include <stdio.h>                  // printf() or iprintf()
#include <string.h>                 // strtok()

#include "../drivers/i2c.h"
#include "../drivers/ssp_spi.h"
#include "../drivers/pwm.h"
#include "pid.h"

#include "../System/lpc214x.h"


/* ---- private declarations ---- */
void getUartLine(char* uartInput);
#define MATCH(a,b)		(strcmp(a,b) == 0)
unsigned char send_zero(unsigned char address, unsigned char loc);
void scan_i2c();
unsigned char zero_wii_motion_plus(int*y,int*p,int*r);


void continuous(void *pvParameters)
{
	OSHANDLES *osHandles = (OSHANDLES*)pvParameters;

	setup_PWM_servo();
	setupPWM(2); //set up P0.7 as PWM2  //setupPWM(5);setupPWM(4);setupPWM(6);
	write_PWM_servo(2, 1500 );

	int yaw0=0,pitch0=0,roll0=0;
	unsigned char dummy[1] = "";
	if (i2c_send_byte(0xA6, 0xFE, 0x04) || i2c_send(0xA4,0x00,dummy,0)){ //activate wm+
		vTaskDelay(100);
		unsigned char res = zero_wii_motion_plus(&yaw0,&pitch0,&roll0);
		rprintf("first motion+ init success. 0values = %i-%i-%i (%i averages)\n",yaw0,pitch0,roll0,res);
	} else {rprintf("wm+ error, not init-ed.\n");for(;;){vTaskDelay(999999);}}

	int pitch_integral = 0;
	unsigned int loop_count = 0;
    unsigned int last_ms = xTaskGetTickCount();
    PID_DATA servo_yaw = {.p=0.5, .i=.01, .d=0, .error=0, .prev_val=0};
	for(;;){ //this is a continuous task.
		unsigned char data[6] = "";
		if (i2c_receive(0xA4,0x00,data,6)){
			int yaw = (((data[3]>>2)<<8)+data[0]-yaw0);
			int pitch = (((data[4]>>2)<<8)+data[1]-pitch0);
			int roll = (((data[5]>>2)<<8)+data[2]-roll0);
			unsigned int this_ms = xTaskGetTickCount();
			pitch_integral += pitch*(this_ms-last_ms); //deg/s * ms = degrees*1000
			last_ms = this_ms;
			//if (!(loop_count%50))
				//rprintf("decoded : %i %i %i [%d]%s\n", yaw*595/8192, roll*595/8192, pitch*595/8192, pitch_integral*595/8192/1000, (data[4]&1)?"connected":"");
		} //else rprintf("wm+ error\n");

		//if ((loop_count*3/1000)%2)
		//	write_PWM(2, ((loop_count*3)%1000)+1000 );
		//else
		//	write_PWM(2, (2000 - (loop_count*3)%1000) );
		//write_PWM_servo(2, pitch_integral/1000+1500 );
		write_PWM_servo(2, calculate_pid(pitch_integral/1000, 0, &servo_yaw) + 1500 );

		vTaskDelay(6);
		loop_count++;
	}
}

void uartUI(void *pvParameters)
{
	OSHANDLES *osHandles = (OSHANDLES*)pvParameters;
	char uartInput[128];
	
	vTaskDelay(100);
	setupPWM(5); write_PWM_servo(5, 1500 );
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
		}*/
		else if (MATCH(command, "servo")){
			int val2 = atoi(strtok(NULL, ""));
			write_PWM_servo(5, val2 );
			rprintf("wrote %i to servo 5\n",val2);
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

unsigned char zero_wii_motion_plus(int*y,int*p,int*r){
	unsigned char successful_reads = 0;
	unsigned char data[6];
	unsigned int calib_yaw0=0,calib_pitch0=0,calib_roll0=0;

	for (int i=0;i<10;i++){
		if (i2c_receive(0xA4,0x00,data,6)){
			if ( (data[5]&2) && !(data[5]&1) && (i>1)) {
				calib_yaw0+=(((data[3]>>2)<<8)+data[0]);
				calib_pitch0+=(((data[4]>>2)<<8)+data[1]);
				calib_roll0+=(((data[5]>>2)<<8)+data[2]);
				successful_reads++;
			}
		}
		vTaskDelay(10);
	}
	*y = calib_yaw0/successful_reads;
	*p = calib_pitch0/successful_reads;
	*r = calib_roll0/successful_reads;
	return successful_reads;
}


