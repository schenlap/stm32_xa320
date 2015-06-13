#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "usb.h"
#include "teensy.h"

static uint16_t ident = 0;
uint8_t buf[64];

/*
 * type: 0 .. command, 1 .. integer, 2 .. float
 */
uint8_t teensy_register_dataref(char *str, uint8_t type)
{
	uint8_t len = strlen(str);
	ident++;

	buf[0] = len + 6;
	buf[1] = 0x01;        // Register command
	buf[2] = (uint8_t)(ident & 0xFF);
	buf[3] = (uint8_t)(ident >> 8);
	buf[4] = type;
	buf[5] = 0;           // reserved
	strncpy((char *)&buf[6], (char *)str, 64 - 5);
	buf[len + 6] = 0; // len of next command
	
	usb_send_packet(buf, 64);
	return ident;
}

void teensy_send_int(uint16_t id, uint32_t d) {
	buf[0] = 10;
	buf[1] = 0x02;        // Write command
	buf[2] = (uint8_t)(id & 0xFF);
	buf[3] = (uint8_t)(id >> 8);
	buf[4] = 1;           // Integer
	buf[5] = 0;           // reserved
	buf[6] = (uint8_t) d;
	buf[7] = (uint8_t) (d >> 8);
	buf[8] = (uint8_t) (d >> 16);
	buf[9] = (uint8_t) (d >> 24);
	buf[10] = 0; // len of next command
	
	usb_send_packet(buf, 64);
}
