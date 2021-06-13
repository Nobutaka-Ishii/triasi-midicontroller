#define _XTAL_FREQ        8000000L
#define FCY             _XTAL_FREQ/4 /* 0.5us operation cycle */

// User configuration fields.
#define CCCHANNEL 0x0
#define GP1CCNUMBER 0x40
#define GP3CCNUMBER 67
#define GP1ONVAL 127
#define GP1OFFVAL 0
#define GP3ONVAL 127
#define GP3OFFVAL 0

// user configuration fields above.

#define GP1ONSTAT 0b000
#define GP1ON0 (uint8_t)(0xB0|CCCHANNEL)
#define GP1ON1 (uint8_t)GP1CCNUMBER
#define GP1ON2 (uint8_t)GP1ONVAL

#define GP1OFFSTAT 0b001
#define GP1OFF0 (uint8_t)(0xB0|CCCHANNEL)
#define GP1OFF1 (uint8_t)GP1CCNUMBER
#define GP1OFF2 (uint8_t)GP1OFFVAL

#define GP3ONSTAT 0b010
#define GP3ON0 (uint8_t)(0xB0|CCCHANNEL)
#define GP3ON1 (uint8_t)GP3CCNUMBER
#define GP3ON2 (uint8_t)GP3ONVAL

#define GP3OFFSTAT 0b011
#define GP3OFF0 (uint8_t)(0xB0|CCCHANNEL)
#define GP3OFF1 (uint8_t)GP3CCNUMBER
#define GP3OFF2 (uint8_t)GP3OFFVAL

#define AN0STAT 0b100
#define AN0CCNUM (uint8_t)11
// analog input AN) is used as CC#11 - expression

#define ALLSOUNDOFFVALUE 120


uint8_t GP1bitHistory;
uint8_t GP3bitHistory;
uint8_t TMR0roundUpper;
uint8_t TMR0roundLower;
uint8_t ringBufUpper;
uint8_t ringBufLower;
/*
 *  000: CC#64-OFF, 001: CC#64-ON, 
 *  010: NOTE-OFF, 011: NOTE-ON
 *  100: analog value is to be sent.
 * 
 * It means this buffer has depth of 4.
*/

/*
 * uint8_t cursor 
 * <5>: last GP3 stat
 * <4>: last GP1 stat
 * <3:2>: ring buffer's tail position
 * <1:0>: ring buffer's head position
 */
//uint8_t cursor = 0b00110000; //

void push(uint8_t pushVal);
uint8_t pop(void);
void TMRcarry(); // Carry operation between TMR0roundCountUpper and lower are done in this function.

uint8_t head;
uint8_t tail;
uint8_t lastGp1;
uint8_t lastGp3;
uint8_t an0lastVal;
