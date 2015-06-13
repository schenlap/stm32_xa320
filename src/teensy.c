#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "usb.h"
#include "teensy.h"

static uint16_t ident = 0;

/*
 * type: 0 .. command, 1 .. integer, 2 .. float
 */
uint8_t teenys_register_dataref(char *str, uint8_t type)
{
	uint8_t buf[64];
	ident++;

	buf[0] = strlen(str) + 6; // expect len of string only
	buf[1] = 0x01;        // Register command
	buf[2] = (uint8_t)(ident & 0xFF);
	buf[3] = (uint8_t)(ident >> 8);
	buf[4] = type;
	buf[5] = 0;           // reserved
	strncpy((char *)&buf[6], (char *)str, 64 - 5);
	
	usb_send_packet(buf, 64);
	return ident;
}
