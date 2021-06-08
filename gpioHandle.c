#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "gpioHandle.h"

#if 1
void send1byte(uint8_t msgByte){
	uint8_t itr;
	uint8_t tmpReg; // for precise calculation of GPIO output timing.
	// start bit
	GPIObits.GP2 = 0;
	__delay_us(29);
	NOP(); // required for precise timing.

	for( itr=0; itr<8; itr++){
		//GPIObits.GP2 = (uint8_t)(msgByte & 0x1);
		tmpReg = (uint8_t)( (msgByte) & 0x1);
		GPIObits.GP2 = tmpReg;
		msgByte >>= 1;
		__delay_us(20);
		NOP(); // required for precise timing.
	}

	NOP();NOP();NOP();NOP(); // required for precise timing.
	// stop bit
	GPIObits.GP2 = 1;
	__delay_us(32);
}
#else
void send1byte(uint8_t msgByte){
	uint8_t itr;
	uint8_t gpStatStore;
	uint8_t outputBit; // for precise calculation of GPIO output timing.
	uint8_t TMR0store;
	
	// start bit
	GPIObits.GP2 = 0;
	__delay_us(29);
	NOP(); // required for precise timing.
	
	for( itr = 0; itr < 8; itr++){
		//GPIObits.GP2 = (uint8_t)(msgByte & 0x1);
		outputBit = (uint8_t)( (msgByte) & 0x1);
		GPIObits.GP2 = outputBit;
		msgByte >>= 1;

		constructHistory();
		__delay_us(9);
	}
	GPIObits.GP2=1;
	__delay_us(2); // required for precise timing.
	
	
	// stop bit
	GPIObits.GP2 = 1;
	
	// Room for 31.5us operations for end of sending stop bit.
	TMR0store = TMR0;
	TMR0 = 0;

	REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS

	// wait for 31us elapse by TMR0
	// This elapsing time depends on GPIO status processing operation in REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS.
	// So elastic waiting duration approach is mandatory.
	while(TMR0 < 5){} // approx 32us is consumed from TMR0=0 statement.
	TMR0 = TMR0store;
}

#endif

void send1byte(uint8_t msgByte);
void constructHistory(void);
void collectGpioStat(void);

void constructHistory(void)
{
	uint8_t gpStatStore;
	gpStatStore = (uint8_t)(GPIO & 0x0a); // because GP1 and GP3 is the storing target.
	GP1bitHistory <<= 1;
	GP3bitHistory <<= 1;
	gpStatStore >>= 1; // make GP1 info to place lsb bit.
	GP1bitHistory |= (uint8_t)(gpStatStore & 0x01);
	gpStatStore >>= 2; // make GP3 info to place lsb bit.
	GP3bitHistory |= (uint8_t)(gpStatStore & 0x01);
}

void collectGpioStat(void)
{
	uint8_t itr;

	STATUSbits.CARRY = 0;
	TMR0roundLower += TMR0;
	TMR0 = 0;
	if( STATUSbits.CARRY )TMR0roundUpper++;
	
	for( itr = 0; itr < 8; itr++){
		constructHistory();
		__delay_us(10);
	}	
}

