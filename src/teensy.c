#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "usb.h"
#include "teensy.h"

#define MAX_IDS    50

struct teensy_dat_t {
	void (*cb)(uint8_t, uint32_t);
};

struct teensy_dat_t datids[MAX_IDS];

uint8_t buf[64];
volatile uint32_t last_request_time;


uint32_t teensy_get_last_request_time(void) {
	return last_request_time;
}

/*
 * type: 0 .. command, 1 .. integer, 2 .. float
 */
uint8_t teensy_register_dataref(uint8_t ident, char *str, uint8_t type, void (*cb)(uint8_t, uint32_t))
{
	if (ident >= MAX_IDS)
		return 0;
	datids[ident].cb = cb;
	uint8_t len = strlen(str);

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
	if (!usb_ready)
		return;

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

void teensy_usb_callback(uint8_t *hbuf) {
	int i = 0;
	uint8_t len;
	uint8_t cmd;
	uint16_t id;
	uint32_t data;

	do {
		len = hbuf[i];
		if (len < 2 || len > 64 - i) break;
		cmd =hbuf[i + 1];
		switch(cmd) {
				case 0x02: /* Data from X-Plane */
						id = (hbuf[i + 3]) << 8 | hbuf[i + 2];
						data = hbuf[i + 6] | hbuf[i+7] << 8;
						data |= hbuf[i + 8] << 16 | hbuf[i+9] << 24;
						if (id < MAX_IDS) {
							if (datids[id].cb)
								datids[id].cb(id, data);
						}
					break;
				case 0x03: /* Enable / disable, is alway sent */
						data = hbuf[i + 2];
						last_request_time = systime_get();
					break;
		}
		i += len;
	} while (i < 64);
}
