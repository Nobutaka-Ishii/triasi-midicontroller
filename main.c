// PIC10F222 Configuration Bit Settings
// 'C' source line config statements
// CONFIG
#pragma config IOSCFS = 8MHZ    // Internal Oscillator Frequency Select bit (8 MHz)
#pragma config MCPU = OFF       // Master Clear Pull-up Enable bit (Pull-up disabled)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)
#pragma config CP = OFF         // Code protection bit (Code protection off)
#pragma config MCLRE = OFF      // GP3/MCLR Pin Function Select bit (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "gpioHandle.h"

void push(uint8_t pushVal);
uint8_t pop(void);

uint8_t pop(void)
{
	uint8_t retVal;
	
	if( head%2 ){
		switch(head){
			case 1:
				retVal = (uint8_t)(ringBufLower & 0xf0);
				break;
			case 3:
				retVal = (uint8_t)(ringBufUpper & 0xf0);
				break;
		}
		retVal >>=4;
	}else{
		switch(head){
			case 0:
				retVal = (uint8_t)(ringBufLower & 0x0f);
				break;
			case 2:
				retVal = (uint8_t)(ringBufUpper & 0x0f);
				break;
		}
	}
	
	// moving head cursor.
	head++;
	head %= 4;
	return retVal;
}

void push(uint8_t pushVal)
{
	if( tail%2 ){
		pushVal <<=4;
		switch(tail){
			case 1:
				ringBufLower &= 0x0f;
				ringBufLower |= (uint8_t)pushVal;
				break;
			case 3:
				ringBufUpper &= 0x0f;
				ringBufUpper |= (uint8_t)pushVal;
				break;
		}
	}else{
		switch(tail){
			case 0:
				ringBufLower &= 0xf0;
				ringBufLower |= pushVal;
				break;
			case 2:
				ringBufUpper &= 0xf0;
				ringBufUpper |= pushVal;
				break;
		}
	}
	
	// moving tail cursor
	tail++;
	tail %= 4;
}
			
void main(void) {
	asm("MOVLW 0b00111110");
	asm("MOVWF OSCCAL");

	/* initial chip configuration */
	GPIObits.GP2 = 1; // for preventing unintended MIDI message output.
	OPTION = 0b10000010; // PSA=010 for TMR0, then TMR0 counts every 4us.
	TRISGPIO = 0b00001011; // <3> is actually don't be cared, because it is always input by hardware limitation.
	ADCON0 = 0b01000001; // ANS<0> (GPIO0's pin) is used as analog input.
	
	if(GPIObits.GP3 == 0) an0inUse = 0;
	else an0inUse |=1 ;
	
	if(GPIObits.GP1 == 0) gp1isNote = 1;
	else gp1isNote = 0;
	
	// send all-sound-off to the channel where coming CC messages will be.
	//send1byte(0xB0|CCCHANNEL);
	//send1byte(ALLSOUNDOFFVALUE);
	//send1byte(0x0);

	head = 0;
	tail = 0;
	/*
	lastGp1 = 1; // "lastGp1=1;" is equivalent but waste of operations.
	lastGp3 = 1;
	GP1bitHistory = 0xff;
	GP3bitHistory = 0xff;
	 */
	lastGp1 = 0;
	lastGp3 = 0;
	// Declaring as this, GP1bitHistory, GP3bitHistory are not cleared by 0xff.
	// Because even if being in the worst case that booting sequence had 0->1 transition,
	// they only sent NOTE-OFF. They will sound nothing.
	
	
	//TMR0 = 0; // not affect much
	//an0lastVal = 0;
	
	/* main loop */
	while(1){
				
		if( tail == head ){
			collectGpioStat();
			TMRcarry();

			REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS
			TMRcarry();
			
			if( TMR0roundUpper > 252){ // 1-TMR0-round = 1.024ms. 1.024ms*252 ~258ms is fair for active sensing.
				send1byte(0xfe);
				TMR0roundUpper = 0;
			}
		}else{
			while( tail != head){
				switch( pop() ){
					case 0x00:
						if(gp1isNote){
							send1byte(GP1NOTEON0);
							send1byte(GP1NOTEON1);
							send1byte(GP1NOTEON2);
						}else{
							send1byte(GP1ON0);
							send1byte(GP1ON1);
							send1byte(GP1ON2);
						}
						break;
					case 0x01:
						if(gp1isNote){
							send1byte(GP1NOTEOFF0);
							send1byte(GP1NOTEOFF1);
							send1byte(GP1NOTEOFF2);
						}else{
							send1byte(GP1OFF0);
							send1byte(GP1OFF1);
							send1byte(GP1OFF2);
						}
						break;
					case 0x02:
						send1byte(GP3ON0);
						send1byte(GP3ON1);
						send1byte(GP3ON2);
						break;
					case 0x03:
						send1byte(GP3OFF0);
						send1byte(GP3OFF1);
						send1byte(GP3OFF2);
						break;
					case 0x04:
						send1byte(AN0CHANNEL);
						send1byte(AN0CCNUM);
						send1byte(an0lastVal);
						break;
				}
			}
			TMR0roundUpper = 0;
			//TMR0roundLower = 0;
		}
		//CLRWDT();
	}
}

void TMRcarry(void){
	STATUSbits.CARRY = 0;
	TMR0roundLower += TMR0;
	TMR0 = 0;
	if( STATUSbits.CARRY )TMR0roundUpper++;
}
