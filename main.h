#define _XTAL_FREQ        8000000L
#define FCY             _XTAL_FREQ/4 /* 0.5us operation cycle */

// User configuration fields.
#define CCCHANNEL 0x0
#define GP1CCNUMBER 67
#define GP3CCNUMBER 0x40
#define GP1ONVAL 127
#define GP1OFFVAL 0
#define GP3ONVAL 127
#define GP3OFFVAL 0

#define AN0CHANNEL (uint8_t)(0xB0|CCCHANNEL)
	// analog input AN0 is used as CC#11 - expression
#define AN0CCNUM (uint8_t)11

#define GP1NOTEON0 (uint8_t)0x81
#define GP1NOTEON1 (uint8_t)48
#define GP1NOTEON2 (uint8_t)100
#define GP1NOTEOFF0 (uint8_t)0x91
#define GP1NOTEOFF1 (uint8_t)48
#define GP1NOTEOFF2 (uint8_t)0

// user configuration fields above.

#define GP1ONSTAT 0x0
#define GP1ON0 (uint8_t)(0xB0|CCCHANNEL)
#define GP1ON1 (uint8_t)GP1CCNUMBER
#define GP1ON2 (uint8_t)GP1ONVAL

#define GP1OFFSTAT 0x1
#define GP1OFF0 (uint8_t)(0xB0|CCCHANNEL)
#define GP1OFF1 (uint8_t)GP1CCNUMBER
#define GP1OFF2 (uint8_t)GP1OFFVAL

#define GP3ONSTAT 0x2
#define GP3ON0 (uint8_t)(0xB0|CCCHANNEL)
#define GP3ON1 (uint8_t)GP3CCNUMBER
#define GP3ON2 (uint8_t)GP3ONVAL

#define GP3OFFSTAT 0x3
#define GP3OFF0 (uint8_t)(0xB0|CCCHANNEL)
#define GP3OFF1 (uint8_t)GP3CCNUMBER
#define GP3OFF2 (uint8_t)GP3OFFVAL

#define AN0STAT 0x4

#define ALLSOUNDOFFVALUE 120

uint8_t GP1bitHistory;
uint8_t GP3bitHistory;
uint8_t TMR0roundUpper;
uint8_t TMR0roundLower;
uint8_t ringBufUpper;
uint8_t ringBufLower;
/*
 * Ringbuffer message length is enough in 3bits, but 4bits are suitable for reducing program space.
 * 
 *  0000: GP1 pedal released(pulled up by internal resistor), 0001: GP1 stomped, 
 *  0010: GP3 released, 0011: GP3 stomped.
 *  0100: analog value is to be sent because it has been changed from last sending.
 * 
 * It means ring buffer totally accomodate 6 messages.
*/

void push(uint8_t pushVal);
uint8_t pop(void);
void TMRcarry(); // Carry operation between TMR0roundCountUpper and lower are done in this function.

uint8_t head;
uint8_t tail;
bool lastGp1;
bool lastGp3;
uint8_t an0lastVal;
bool an0inUse;
