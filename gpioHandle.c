#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "gpioHandle.h"

void send1byte(uint8_t msgByte){
	uint8_t itr;
	uint8_t gpStatStore;
	uint8_t outputBit; // for precise calculation of GPIO output timing.
	uint8_t TMR0store;
	
	// start bit
	GPIObits.GP2 = 0;
	__delay_us(29.5);
	
	for( itr = 0; itr < 8; itr++){
		//GPIObits.GP2 = (uint8_t)(msgByte & 0x1);
		outputBit = (uint8_t)( (msgByte) & 0x1);
		GPIObits.GP2 = outputBit;
		msgByte >>= 1;

		constructHistory();
		__delay_us(6);
	}
	__delay_us(2); // required for precise timing.
	
	// stop bit
	GPIObits.GP2 |= 1;
	
	// Room for 31.5us operations for end of sending stop bit.
	//TMR0store = TMR0;
	//TMR0 = 0;

	REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS

	// wait for 31us elapse by TMR0
	// This elapsing time depends on GPIO status processing operation in REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS.
	// So elastic waiting duration approach is mandatory.
	//while(TMR0 < 2){} // approx 32us is consumed from TMR0=0 statement.
			// removed the waiting operation, because following instruction and coming MIDI send timing,
			// time enouh for 31us is always consumed.
	TMRcarry();
}

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
	uint8_t tmpReg0; // iterator and calculation use.
	
	ADCON0 = 0b01000011; // "ADCON0 |= 0b00000010" is equivalent but wastes program space.
	for( tmpReg0 = 0 ; tmpReg0 < 8; tmpReg0++){
		constructHistory();
		__delay_us(20);
	} // in the 2nd loop, AD conversion gets always finished.
	
	// so no check of ADC process status bit.
	tmpReg0 = ADRES;
	tmpReg0 >>= 1;
	
	// in the operations below, [2] and [0] are used for calculation.
	an0oldVal[2] += (uint8_t)(0x7f & an0oldVal[1]);
	an0oldVal[2] >>= 1; // Now [2] has the average value of [2] and [1]
	an0oldVal[1] &= 0x80;
	an0oldVal[1] |= an0oldVal[0]; // Not substitution but OR operation, because [1]'s MSB is used as an0 use/unuse flag.
	an0oldVal[0] += tmpReg0;
	an0oldVal[0] >>= 1; // Now [0] has the average value of [0] and current ADC value.
	an0oldVal[2] += an0oldVal[0];
	an0oldVal[2] >>= 1; // Now [2] has the average of last 4 AD conversion value.
	
	if ( (an0oldVal[2] != an0lastSentVal) && ( (an0oldVal[1] & 0x80) == 0x80 ) ){
		an0lastSentVal = an0oldVal[2]; // to be sent later when an0 operation is popped from the buffer.
		push(AN0STAT);
	}
	
	// renew the ADC values history array.
	an0oldVal[2] = (0x7f & an0oldVal[1]);
	// an0oldVal[1] is already renewed by the value of [0].
	an0oldVal[0] = tmpReg0;
}
 


