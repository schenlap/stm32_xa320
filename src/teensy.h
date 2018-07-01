#ifndef TEENSY_H
#define TEENSY_H

int32_t teensy_register_dataref(uint8_t ident, char *str, uint8_t type, void (*cb)(uint8_t, uint32_t));
int teensy_send_int(uint16_t ident, uint32_t d);
void teensy_send_float(uint16_t ident, float d);
void teensy_send_command_once(uint16_t id);
void teensy_usb_callback(uint8_t *hbuf);
uint32_t teensy_get_last_request_time(void);
float teensy_from_float(uint32_t data);

#endif /* TEENSY_H */
