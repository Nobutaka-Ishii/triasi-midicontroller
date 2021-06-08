#define _XTAL_FREQ        8000000L
#define FCY             _XTAL_FREQ/4 /* 0.5us operation cycle */

// User configuration fields.
#define CCCHANNEL 0x0
#define CCNUMBER 0x40
#define CCONVAL 127
#define CCOFFVAL 0

#define NOTECHANNEL 1
#define NOTENUM 48
#define NOTEVEL 100
// user configuration fields above.

#define CCONSTAT 0b00
#define CCON0 (uint8_t)(0xB0|CCCHANNEL)
#define CCON1 (uint8_t)CCNUMBER
#define CCON2 (uint8_t)CCONVAL

#define CCOFFSTAT 0b01
#define CCOFF0 (uint8_t)(0xB0|CCCHANNEL)
#define CCOFF1 (uint8_t)CCNUMBER
#define CCOFF2 (uint8_t)CCOFFVAL

#define NOTEONSTAT 0b10
#define NOTEON0 (uint8_t)(0x90|NOTECHANNEL)
#define NOTEON1 (uint8_t)NOTENUM
#define NOTEON2 (uint8_t)NOTEVEL

#define NOTEOFFSTAT 0b11
#define NOTEOFF0 (uint8_t)(0x80|NOTECHANNEL)
#define NOTEOFF1 (uint8_t)NOTENUM
#define NOTEOFF2 (uint8_t)0

uint8_t GP1bitHistory;
uint8_t GP3bitHistory;
uint8_t TMR0roundUpper;
uint8_t TMR0roundLower;
uint8_t ringBuf;
/*
 *  00: CC#64-OFF, 001: CC#64-ON, 
 *  10: NOTE-OFF, 011: NOTE-ON
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
uint8_t head;
uint8_t tail;
uint8_t lastGp1;
uint8_t lastGp3;
