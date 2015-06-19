#ifndef TEENSY_H
#define TEENSY_H

uint8_t teensy_register_dataref(uint8_t ident, char *str, uint8_t type, void (*cb)(uint8_t, uint32_t));
void teensy_send_int(uint16_t ident, uint32_t d);
void teensy_usb_callback(uint8_t *hbuf);
uint32_t teensy_get_last_request_time(void);

#endif /* TEENSY_H */
