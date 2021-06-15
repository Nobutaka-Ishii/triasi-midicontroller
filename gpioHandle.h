/*
 if( GP1bitHistory == 0xff && lastGp1 == 0x0){ 
 
current GP1 indicate Hi(contiguous 1: 0b11111111).
It means chatterings are successfully removed.
And comparing it with last GP1 status, it turned out that last=low, current=Hi.
This means pedal was released. 
 */

// These operation are both in gpioHandle and main, but cannot functionalize it because of stack depth limitation.
#define REMOVECHATTERINGANDFIXCURRENTPEDALSTATUS \
	if( GP1bitHistory == 0xff && lastGp1 == 0x0){ \
		push(GP3OFFSTAT); \
		lastGp1 |= 1; \
	} \
	if( GP1bitHistory == 0x00 && lastGp1 == 0x1){ \
		push(GP3ONSTAT); \
		lastGp1 = 0; \
	} \
	if( GP3bitHistory == 0xff && lastGp3 == 0x0){ \
		push(GP1OFFSTAT); \
		lastGp3 |= 1; \
	} \
	if( GP3bitHistory == 0x00 && lastGp3 == 0x1){ \
		push(GP1ONSTAT); \
		lastGp3 = 0; \
	} // set of operations end here.

void send1byte(uint8_t msgByte);
void collectGpioStat(void);
void constructHistory(void);
