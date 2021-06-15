// PIC10F222 Configuration Bit Settings
// 'C' source line config statements
// CONFIG
#pragma config IOSCFS = 8MHZ    // Internal Oscillator Frequency Select bit (8 MHz)
#pragma config MCPU = OFF       // Master Clear Pull-up Enable bit (Pull-up disabled)
#pragma config WDTE = ON       // Watchdog Timer Enable bit (WDT enabled)
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
	uint8_t tmpReg;
	
	if( head%2 ){
		switch(head){
			case 1:
				tmpReg = (uint8_t)(ringBufLower & 0xf0);
				break;
			case 3:
				tmpReg = (uint8_t)(ringBufMiddle & 0xf0);
				break;
			case 5:
				tmpReg = (uint8_t)(ringBufUpper & 0xf0);
				break;
		}
		tmpReg >>=4;
	}else{
		switch(head){
			case 0:
				retVal = (uint8_t)(ringBufLower & 0x0f);
				break;
			case 2:
				retVal = (uint8_t)(ringBufMiddle & 0x0f);
				break;
			case 4:
				retVal = (uint8_t)(ringBufUpper & 0x0f);
				break;
		}
	}
	
	// moving head cursor.
	head++;
	//head %= 6;
	if( head >5) head -= 6;
	return retVal;
}

void push(uint8_t pushVal)
{
	if( tail%2 ){
		switch(tail){
			case 1:
				ringBufLower &= 0xf0;
				ringBufLower |= (uint8_t)pushVal;
				break;
			case 3:
				ringBufMiddle &= 0xf0;
				ringBufMiddle|= (uint8_t)pushVal;
				break;
			case 5:
				ringBufUpper &= 0xf0;
				ringBufUpper |= (uint8_t)pushVal;
				break;
		}
	}else{
		pushVal <<=4;
		switch(tail){
			case 0:
				ringBufLower &= 0xf0;
				ringBufLower |= pushVal;
				break;
			case 2:
				ringBufMiddle &= 0xf0;
				ringBufMiddle |= pushVal;
				break;
			case 4:
				ringBufUpper &= 0xf0;
				ringBufUpper |= pushVal;
				break;
		}
	}
	
	// moving tail cursor
	tail++;
	//tail %= 6;
	if(tail>5) tail -=6;
}
			
void main(void) {
	asm("MOVWF OSCCAL");

	/* initial chip configuration */
	GPIObits.GP2 = 1; // for preventing unintended MIDI message output.
	OPTION = 0b11000010; // PSA=010 for TMR0, then TMR0 counts every 4us.
	TRISGPIO = 0b00001011; // <3> is actually don't be cared, because it is always input by hardware limitation.
	ADCON0 = 0b01000001; // ANS<0> (GPIO0's pin) is used as analog input.
	
	if(GPIObits.GP1 == 0) an0inUse |= 1;
	else an0inUse = 0;

	// send all-sound-off to the channel where coming CC messages will be.
	send1byte(0xB0|CCCHANNEL);
	send1byte(ALLSOUNDOFFVALUE);
	send1byte(0x0);

	head = 0;
	tail = 0;
	lastGp1 |= 1; // "lastGp1=1;" is equivalent but waste of operations.
	lastGp3 |= 1;
	GP1bitHistory = 0xff;
	GP3bitHistory = 0xff;
	TMR0 = 0;
	an0lastVal = 0;
	
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
					case 0b0000:
						send1byte(CCCHANNEL);
						send1byte(GP1ON1);
						send1byte(GP1ON2);
						break;
					case 0b0001:
						send1byte(CCCHANNEL);
						send1byte(GP1OFF1);
						send1byte(GP1OFF2);
						break;
					case 0b0010:
						send1byte(CCCHANNEL);
						send1byte(GP3ON1);
						send1byte(GP3ON2);
						break;
					case 0b0011:
						send1byte(CCCHANNEL);
						send1byte(GP3OFF1);
						send1byte(GP3OFF2);
						break;
					case 0b0100:
						send1byte(CCCHANNEL);
						send1byte(AN0CCNUM);
						send1byte(an0lastVal);
						break;
				}
			}
			TMR0roundUpper = 0;
			TMR0roundLower = 0;
		}
		CLRWDT();
	}
}

void TMRcarry(void){
	STATUSbits.CARRY = 0;
	TMR0roundLower += TMR0;
	TMR0 = 0;
	if( STATUSbits.CARRY )TMR0roundUpper++;
}
