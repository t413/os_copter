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
#include "../drivers/pwm.h"



/* ---- private declarations ---- */
void getUartLine(char* uartInput);
#define MATCH(a,b)		(strcmp(a,b) == 0)
unsigned char send_zero(unsigned char address, unsigned char loc);
void scan_i2c();


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


