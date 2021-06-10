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
	uint8_t tmpReg; // temporary value for operation reduction.
	switch( head ){
		case 0:
			retVal = (uint8_t)(ringBuf & 0b00000011);
			break;
		case 1:
			//retVal = (uint8_t)((ringBuf & 0b00001100) >> 2); // this style wates program count.
			tmpReg = (uint8_t)(ringBuf & 0b00001100);
			tmpReg >>= 2;
			retVal = tmpReg;
			break;
		case 2:
			//retVal = (uint8_t)((ringBuf & 0b00110000) >> 4);
			tmpReg = (uint8_t)(ringBuf & 0b00110000);
			tmpReg >>= 4;
			retVal = tmpReg;
			break;
		case 3:
			//retVal = (uint8_t)((ringBuf & 0b11000000) >> 6);
			tmpReg = (uint8_t)(ringBuf & 0b11000000);
			tmpReg >>= 6;
			retVal = tmpReg;
			break;
	}
	
	// moving head cursor.
	head++;
	head %= 4;
	return retVal;
}

void push(uint8_t pushVal)
{
	switch( tail ){
		case 0:
			ringBuf &= 0xfc;
			ringBuf |= (uint8_t)pushVal;
			break;
		case 1:
			ringBuf &= 0xf3;
			ringBuf |= (uint8_t)(pushVal <<2);
			break;
		case 2:
			ringBuf &= 0xcf;
			ringBuf |= (uint8_t)(pushVal <<4);
			break;
		case 3:
			ringBuf &= 0x3f;
			ringBuf |= (uint8_t)(pushVal <<6);
			break;			
	}
	
	// moving tail cursor
	tail++;
	tail %= 4;
}
			
void main(void) {
	asm("MOVWF OSCCAL");

	/* initial chip configuration */
	GPIObits.GP2 = 1; // for preventing unintended MIDI message output.
	OPTION = 0b10000010; // PSA=010 for TMR0, then TMR0 counts every 4us.
	TRISGPIO = 0b00000011;
	ADCON0 = 0b00000000;

	/* reduce program space
	send1byte(CCOFF0);
	send1byte(CCOFF1);
	send1byte(CCOFF2);
	send1byte(NOTEOFF0);
	send1byte(NOTEOFF1);
	send1byte(NOTEOFF2);
	*/
	
	// send all-sound-off to NOTE-control pedal channel
	send1byte(0xB0|NOTECHANNEL);
	send1byte(ALLSOUNDOFFVALUE);
	send1byte(0x0);

	head = 0;
	tail = 0;
	lastGp1 = 1;
	lastGp3 = 1;
	GP1bitHistory = 0xff;
	GP3bitHistory = 0xff;
	TMR0 = 0;
	
	/* main loop */
	while(1){
				
		if( tail == head ){
			collectGpioStat();
			
			REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS
	
			STATUSbits.CARRY = 0;
			TMR0roundLower += TMR0;
			TMR0 = 0;
			if( STATUSbits.CARRY ) TMR0roundUpper++;
			if( TMR0roundUpper > 252){ // 1-TMR0-round = 1.024ms. 1.024ms*252 ~258ms is fair for active sensing.
				send1byte(0xfe);
				TMR0roundUpper = 0;
			}
		}else{
			while( tail != head){
				switch( pop() ){
					case 0x00:
						send1byte(CCON0);
						send1byte(CCON1);
						send1byte(CCON2);
						break;
					case 0x01:
						send1byte(CCOFF0);
						send1byte(CCOFF1);
						send1byte(CCOFF2);
						break;
					case 0x2:
						send1byte(NOTEON0);
						send1byte(NOTEON1);
						send1byte(NOTEON2);
						break;
					case 0x3:
						send1byte(NOTEOFF0);
						send1byte(NOTEOFF1);
						send1byte(NOTEOFF2);
						break;
				}
			}
			TMR0roundUpper = 0;
			TMR0roundLower = 0;
		}
		CLRWDT();
	}
}
