#ifndef TEENSY_H
#define TEENSY_H

#define TEENSY_CMD     0
#define TEENSY_INT     1
#define TEENSY_FLOAT   2

/* cmd_type */
#define TEENSY_CMD_BEGIN 4
#define TEENSY_CMD_END   5
#define TEENSY_CMD_ONCE  6

int32_t teensy_register_dataref(uint8_t ident, char *str, uint8_t type, void (*cb)(uint8_t, uint32_t));
int teensy_send_int(uint16_t ident, uint32_t d);
void teensy_send_float(uint16_t ident, float d);
void teensy_send_command(uint16_t id, uint8_t cmd_type);
void teensy_usb_callback(uint8_t *hbuf);
uint32_t teensy_get_last_request_time(void);
float teensy_from_float(uint32_t data);

#endif /* TEENSY_H */
