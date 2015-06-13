#ifndef TEENSY_H
#define TEENSY_H

uint8_t teensy_register_dataref(char *str, uint8_t type);
void teensy_send_int(uint16_t ident, uint32_t d);

#endif /* TEENSY_H */
